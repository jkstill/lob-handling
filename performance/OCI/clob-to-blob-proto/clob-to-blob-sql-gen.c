#define _POSIX_C_SOURCE 200112L

#include <time.h>
#include "ocilib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define LOG_DIR "c2b-log"
#define CREDENTIAL_FILE "ora-creds.txt"
#define MAX_COLUMNS 8

#define OCI_REBINDING_ENABLED TRUE

typedef struct {
    char db[128], user[128], pwd[128];
} OraCredentials;

OraCredentials creds;
OCI_Statement *stLoadColumnMetadata = NULL;
int column_count = 0;

typedef struct {
    char clob_col[128];
    char blob_col[128];
} ColumnMap;

typedef struct {
    char select_sql[2048];
    char update_sql[2048];
    int column_count;
} SQLTemplate;

ColumnMap column_maps[MAX_COLUMNS];
SQLTemplate sql_template;

FILE* open_log_file() {
    char filename[256];
    snprintf(filename, sizeof(filename), LOG_DIR "/pid_%d.log", getpid());
    return fopen(filename, "a");
}

int read_credentials(const char *filename, OraCredentials *out) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "username:", 9) == 0)
            sscanf(line + 9, "%127s", out->user);
        else if (strncmp(line, "password:", 9) == 0)
            sscanf(line + 9, "%127s", out->pwd);
        else if (strncmp(line, "database:", 9) == 0)
            sscanf(line + 9, "%127s", out->db);
    }
    fclose(fp);
    return 1;
}

int load_column_metadata(OCI_Connection *cn, OCI_Statement *st, const char *tablename, FILE *logf) {

    OCI_Resultset *rs;
    column_count = 0;

    if (!OCI_BindString(st, ":tbl", (otext *)tablename, (unsigned int)strlen(tablename))){
        fprintf(logf, "[load_column_metadata] Column metadata bind failed for table: %s\n", tablename);
        return 0;
	 }

    if (!OCI_Execute(st)) {
        fprintf(logf, "[load_column_metadata] Column metadata query failed for table: %s\n", tablename);
        return 0;
    }

    rs = OCI_GetResultset(st);
    while (OCI_FetchNext(rs) && column_count < MAX_COLUMNS) {
        const char *clob_col = OCI_GetString(rs, 1);
        const char *blob_col = OCI_GetString(rs, 2);
		  // this needs an 'else' with a warning or error
        if (clob_col && blob_col) {
            strncpy(column_maps[column_count].clob_col, clob_col, sizeof(column_maps[column_count].clob_col));
            strncpy(column_maps[column_count].blob_col, blob_col, sizeof(column_maps[column_count].blob_col));
            column_count++;
        }
    }

    if (column_count == 0) {
        fprintf(logf, "No CLOB/BLOB column pairs found for table: %s\n", tablename);
        return 0;
    }

    return 1;
}

int generate_sql_templates(const char *tablename, FILE *logf) {
    char select_buf[2048] = {0};
    char update_buf[2048] = {0};

	// SELECT clob_column_name, blob_column_name FROM clob_to_blob_columns WHERE tablename = :tbl ORDER BY column_id
	
	 snprintf(select_buf, sizeof(select_buf), "SELECT ");
	 snprintf(update_buf, sizeof(update_buf), "UPDATE ");

    strcat(update_buf, tablename);
    strcat(update_buf, " SET ");

    for (int i = 0; i < column_count; ++i) {
        char sel_snip[128];
        char upd_snip[256];
		  if ( i == 0 ) {
				snprintf(sel_snip, sizeof(sel_snip), " %s", column_maps[i].clob_col);
		  } else {
				snprintf(sel_snip, sizeof(sel_snip), ", %s", column_maps[i].clob_col);
		  }

        strncat(select_buf, sel_snip, sizeof(select_buf) - strlen(select_buf) - 1);
		  // set BLOB to the :blob_col
        snprintf(upd_snip, sizeof(upd_snip), "%s = :%s", column_maps[i].blob_col, column_maps[i].blob_col);
        strncat(update_buf, upd_snip, sizeof(update_buf) - strlen(update_buf) - 1);
		  // set the source CLOB to empty_clob()
        snprintf(upd_snip, sizeof(upd_snip), ", %s = %s", column_maps[i].clob_col, "empty_clob()");
        strncat(update_buf, upd_snip, sizeof(update_buf) - strlen(update_buf) - 1);

        if (i < column_count - 1)
            strncat(update_buf, ", ", sizeof(update_buf) - strlen(update_buf) - 1);
    }

    strcat(select_buf, " FROM ");
    strcat(select_buf, tablename);
    strcat(select_buf, " WHERE rowid = :row_id");
    strncat(select_buf, " FOR UPDATE", sizeof(select_buf) - strlen(select_buf) - 1);

    strcat(update_buf, " WHERE rowid = :row_id");

    strncpy(sql_template.select_sql, select_buf, sizeof(sql_template.select_sql));
    strncpy(sql_template.update_sql, update_buf, sizeof(sql_template.update_sql));
    sql_template.column_count = column_count;

    char sql_log_file[256];
    snprintf(sql_log_file, sizeof(sql_log_file), LOG_DIR "/sqlgen_%s.log", tablename);
    FILE *sqlf = fopen(sql_log_file, "w");
    if (sqlf) {
        fprintf(sqlf, "-- Auto-generated SQL for table: %s\n\n", tablename);
        fprintf(sqlf, "SELECT:\n%s\n\n", sql_template.select_sql);
        fprintf(sqlf, "UPDATE:\n%s\n\n", sql_template.update_sql);
        fclose(sqlf);
    } else {
        fprintf(logf, "⚠️ Could not write SQL log for table %s\n", tablename);
    }

    return 1;
}

int main(void) {

    mkdir(LOG_DIR, 0777);
    FILE *logf = open_log_file();
    if (!logf || !read_credentials(CREDENTIAL_FILE, &creds)) return 1;
    setvbuf(logf, NULL, _IONBF, 0);

    OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT|OCI_ENV_CONTEXT);

    OCI_Connection *cn = OCI_ConnectionCreate(creds.db, creds.user, creds.pwd, OCI_SESSION_DEFAULT);
    if (!cn) {
        fprintf(logf, "Failed to connect to database\n");
        OCI_Cleanup();
        fclose(logf);
        return 1;
    }

    stLoadColumnMetadata = OCI_StatementCreate(cn);
    OCI_AllowRebinding(stLoadColumnMetadata, OCI_REBINDING_ENABLED);

    if (!OCI_Prepare(stLoadColumnMetadata,
        "SELECT clob_column_name, blob_column_name FROM clob_to_blob_columns WHERE tablename = :tbl ORDER BY column_id")) {
        fprintf(logf, "Failed to prepare load_column_metadata\n");
        return 1;
    }

    OCI_Statement *stListTables = OCI_StatementCreate(cn);
    if (!stListTables) {
        fprintf(logf, "Failed to create statement for listing tables\n");
        return 1;
    }

    if (!OCI_ExecuteStmt(stListTables, "SELECT DISTINCT tablename FROM clob_to_blob_columns ORDER BY tablename")) {
        fprintf(logf, "Failed to execute table listing\n");
        return 1;
    }

    OCI_Resultset *rsTables = OCI_GetResultset(stListTables);

    while (OCI_FetchNext(rsTables)) {
        const char *tablename = OCI_GetString(rsTables, 1);

        fprintf(logf, "Generating SQL for table: %s\n", tablename);

        if (!load_column_metadata(cn, stLoadColumnMetadata, tablename, logf)) {
            fprintf(logf, "Failed to load column metadata for table: %s\n", tablename);
            continue;
        }

        if (!generate_sql_templates(tablename, logf)) {
            fprintf(logf, "Failed to generate SQL for table: %s\n", tablename);
            continue;
        }
    }

    if (stListTables) OCI_StatementFree(stListTables);
    if (stLoadColumnMetadata) OCI_StatementFree(stLoadColumnMetadata);
    OCI_ConnectionFree(cn);
    OCI_Cleanup();
    fclose(logf);

    printf("\n✅ SQL generation complete. Log file in: %s/pid_%d.log\n\n", LOG_DIR, getpid());

    return 0;
}
