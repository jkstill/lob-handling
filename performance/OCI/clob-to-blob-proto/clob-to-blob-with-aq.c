
#include "ocilib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <emmintrin.h> // SSE3

#define MAX_BATCH_SIZE 100
#define MAX_COLUMNS 8
#define DATA_SIZE_BUF 100 * 1048576
#define LOG_DIR "c2b-log"
#define CREDENTIAL_FILE "ora-creds.txt"

typedef struct {
    char tablename[128];
    char rowid[64];
} QueueEntry;

typedef struct {
    char clob_col[128];
    char blob_col[128];
} ColumnMap;

typedef struct {
    char select_sql[2048];
    char lock_sql[1024];
    char update_sql[2048];
    int column_count;
} SQLTemplate;

typedef struct {
    char db[128], user[128], pwd[128];
} OraCredentials;

OraCredentials creds;
ColumnMap column_maps[MAX_COLUMNS];
int column_count = 0;
SQLTemplate sql_template;
char last_tablename[128] = "";

int hex_to_binary_sse3(OCI_Lob *hex_data, unsigned int *hex_length, OCI_Lob *binary_data) {
    size_t n;
    unsigned int hl = (*hex_length);

    unsigned char *binary_char_data = (unsigned char *)malloc(DATA_SIZE_BUF/2);
    unsigned char *hex_char_data  = (unsigned char *)malloc(DATA_SIZE_BUF+1);

    n = OCI_LobRead(hex_data, hex_char_data, hl);
    if (n % 2 != 0) {
        fprintf(stderr, "Hex data length is not even.\n");
        exit(EXIT_FAILURE);
    }

    size_t i;
    for (i = 0; i < n; i += 2) {
        unsigned char high = hex_char_data[i];
        unsigned char low = hex_char_data[i + 1];

        high = (high >= '0' && high <= '9') ? high - '0' :
               (high >= 'A' && high <= 'F') ? high - 'A' + 10 :
               (high >= 'a' && high <= 'f') ? high - 'a' + 10 : 0;

        low = (low >= '0' && low <= '9') ? low - '0' :
              (low >= 'A' && low <= 'F') ? low - 'A' + 10 :
              (low >= 'a' && low <= 'f') ? low - 'a' + 10 : 0;

        binary_char_data[i / 2] = (high << 4) | low;
    }

    OCI_LobTruncate(binary_data, 0);
    OCI_LobSeek(binary_data, 0, OCI_SEEK_SET);
    OCI_LobAppend(binary_data, binary_char_data, n / 2);

    free(binary_char_data);
    free(hex_char_data);
    return 1;
}

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

int load_column_metadata(OCI_Connection *cn, const char *tablename, FILE *logf) {
    OCI_Statement *st = OCI_StatementCreate(cn);
    OCI_Resultset *rs;
    column_count = 0;

    OCI_Prepare(st, "SELECT clob_column_name, blob_column_name FROM clob_to_blob_columns WHERE tablename = :tbl ORDER BY column_id");
    OCI_BindString(st, ":tbl", (otext *)tablename, (unsigned int)strlen(tablename));

    if (!OCI_Execute(st)) {
        fprintf(logf, "Column metadata query failed for table: %s\n", tablename);
        OCI_StatementFree(st);
        return 0;
    }

    rs = OCI_GetResultset(st);
    while (OCI_FetchNext(rs) && column_count < MAX_COLUMNS) {
        const char *clob_col = OCI_GetString(rs, 1);
        const char *blob_col = OCI_GetString(rs, 2);
        if (clob_col && blob_col) {
            strncpy(column_maps[column_count].clob_col, clob_col, sizeof(column_maps[column_count].clob_col));
            strncpy(column_maps[column_count].blob_col, blob_col, sizeof(column_maps[column_count].blob_col));
            column_count++;
        }
    }

    OCI_StatementFree(st);

    if (column_count == 0) {
        fprintf(logf, "No CLOB/BLOB column pairs found for table: %s\n", tablename);
        return 0;
    }

    return 1;
}

int generate_sql_templates(const char *tablename, FILE *logf) {
    char select_buf[2048] = {0};
    char update_buf[2048] = {0};
    char lock_buf[256] = {0};

    strcpy(select_buf, "SELECT id");
    strcpy(update_buf, "UPDATE ");
    strcat(update_buf, tablename);
    strcat(update_buf, " SET ");

    for (int i = 0; i < column_count; ++i) {
        char sel_snip[128];
        char upd_snip[256];
        snprintf(sel_snip, sizeof(sel_snip), ", %s", column_maps[i].clob_col);
        strncat(select_buf, sel_snip, sizeof(select_buf) - strlen(select_buf) - 1);
        snprintf(upd_snip, sizeof(upd_snip), "%s = :%s", column_maps[i].blob_col, column_maps[i].blob_col);
        strncat(update_buf, upd_snip, sizeof(update_buf) - strlen(update_buf) - 1);
        if (i < column_count - 1)
            strncat(update_buf, ", ", sizeof(update_buf) - strlen(update_buf) - 1);
    }

    snprintf(lock_buf, sizeof(lock_buf), "SELECT 1 FROM %s WHERE rowid = :row_id FOR UPDATE", tablename);
    strcat(select_buf, " FROM ");
    strcat(select_buf, tablename);
    strcat(select_buf, " WHERE rowid = :row_id");
    strcat(update_buf, " WHERE rowid = :row_id");

    strncpy(sql_template.select_sql, select_buf, sizeof(sql_template.select_sql));
    strncpy(sql_template.update_sql, update_buf, sizeof(sql_template.update_sql));
    strncpy(sql_template.lock_sql, lock_buf, sizeof(sql_template.lock_sql));
    sql_template.column_count = column_count;

    char sql_log_file[256];
    snprintf(sql_log_file, sizeof(sql_log_file), LOG_DIR "/sqlgen_%s.log", tablename);
    FILE *sqlf = fopen(sql_log_file, "w");
    if (sqlf) {
        fprintf(sqlf, "-- Auto-generated SQL for table: %s\n\n", tablename);
        fprintf(sqlf, "SELECT:\n%s\n\n", sql_template.select_sql);
        fprintf(sqlf, "LOCK:\n%s\n\n", sql_template.lock_sql);
        fprintf(sqlf, "UPDATE:\n%s\n\n", sql_template.update_sql);
        fclose(sqlf);
    } else {
        fprintf(logf, "⚠️ Could not write SQL log for table %s\n", tablename);
    }

    return 1;
}

int dequeue_batch(OCI_Connection *cn, QueueEntry *entries, int max_entries) {
	 OCI_TypeInfo *type = OCI_TypeInfoGet(cn, "AQ$_JMS_TEXT_MESSAGE", OCI_TIF_TYPE);

    if (!type) {
        fprintf(stderr, "Failed to get AQ type info.\n");
        return 0;
    }

    OCI_Dequeue *deq = OCI_DequeueCreate(type, "clob_to_blob_queue");
    if (!deq) {
        fprintf(stderr, "Failed to create AQ dequeue object.\n");
        return 0;
    }

    OCI_DequeueSetNavigation(deq, OCI_ADN_FIRST_MSG);
    OCI_DequeueSetMode(deq, OCI_ADM_REMOVE);              // Remove from queue on read
    OCI_DequeueSetVisibility(deq, OCI_AMV_IMMEDIATE);     // Immediate visibility
    OCI_DequeueSetWaitTime(deq, 1);                        // 1 second wait (non-blocking)

    int count = 0;

    for (int i = 0; i < max_entries; ++i) {
        OCI_Msg *msg = OCI_DequeueGet(deq);
        if (!msg) break;

        OCI_Object *payload = OCI_MsgGetObject(msg);
        const char *text = OCI_ObjectGetString(payload, "TEXT_VC");
        if (!text || strlen(text) == 0) continue;

        char *sep = strchr(text, ':');
        if (!sep) continue;

        size_t tbl_len = sep - text;
        if (tbl_len >= sizeof(entries[i].tablename)) tbl_len = sizeof(entries[i].tablename) - 1;

        strncpy(entries[i].tablename, text, tbl_len);
        entries[i].tablename[tbl_len] = '\0';

        strncpy(entries[i].rowid, sep + 1, sizeof(entries[i].rowid) - 1);
        entries[i].rowid[sizeof(entries[i].rowid) - 1] = '\0';

        count++;
    }

    OCI_DequeueFree(deq);
    return count;
}

void oci_error_handler(OCI_Error *err) {
    fprintf(stderr, "[OCILib Error] %s\n", OCI_ErrorGetString(err));
}

int main(void) {
    mkdir(LOG_DIR, 0777);
    FILE *logf = open_log_file();
    if (!logf || !read_credentials(CREDENTIAL_FILE, &creds)) return 1;

    OCI_Initialize(oci_error_handler, NULL, OCI_ENV_DEFAULT|OCI_ENV_CONTEXT);
    OCI_Connection *cn = OCI_ConnectionCreate(creds.db, creds.user, creds.pwd, OCI_SESSION_DEFAULT);
	 if (!cn) {
		  fprintf(logf, "Failed to connect to database: %s\n", OCI_ErrorGetString(OCI_GetLastError()));
		  OCI_Cleanup();
		  fclose(logf);
		  return 1;
	 }


    //OCI_Statement *st = OCI_StatementCreate(cn);
    //OCI_ExecuteStmt(st, "alter session set tracefile_identifier = 'CLOBTOBLOB' ");
    //OCI_ExecuteStmt(st, "alter session set events '10046 trace name context forever, level 12'");

    // Placeholder logic: fill these with real dequeued values
    //QueueEntry entries[1] = {{"WFASSIGNMENT", "AAARkJAAEAAAMTEAAA"}};
    //int num_entries = 1;
	 //


    while (1) {

        QueueEntry *entries = malloc(sizeof(QueueEntry) * MAX_BATCH_SIZE);
        if (!entries) {
            fprintf(logf, "Memory allocation failed for QueueEntry list.\\n");
            break;
        }

        int num_entries = dequeue_batch(cn, entries, MAX_BATCH_SIZE);
        if (num_entries == 0) {
            fprintf(logf, "No messages dequeued. Exiting.\\n");
            free(entries);
            break;
        }

        fprintf(logf, "Dequeued %d messages from the queue.\\n", num_entries);

        OCI_Statement *stSel = OCI_StatementCreate(cn);
        OCI_Statement *stLock = OCI_StatementCreate(cn);
        OCI_Statement *stUpd = OCI_StatementCreate(cn);

        for (int i = 0; i < num_entries; ++i) {
            if (strcmp(entries[i].tablename, last_tablename) != 0) {
                if (!load_column_metadata(cn, entries[i].tablename, logf) || !generate_sql_templates(entries[i].tablename, logf)) continue;
                //strncpy(last_tablename, entries[i].tablename, sizeof(last_tablename));
				    strncpy(last_tablename, entries[i].tablename, sizeof(last_tablename) - 1);
                last_tablename[sizeof(last_tablename) - 1] = '\0';
            }

		      fprintf(logf, "DEBUG: Executing SQL: %s|\n", sql_template.select_sql);
            OCI_Prepare(stSel, sql_template.select_sql);

            //OCI_BindString(stSel, ":row_id", entries[i].rowid, (unsigned int)strlen(entries[i].rowid));
		      OCI_BindString(stSel, ":row_id", (otext *)entries[i].rowid, (unsigned int)(strlen(entries[i].rowid) + 1));
		      fprintf(logf, "DEBUG: RowId: %s|\n", entries[i].rowid);

            if (!OCI_Execute(stSel)) {
                OCI_Error *err = OCI_GetLastError();
                fprintf(logf, "[OCI Error] %s\n", OCI_ErrorGetString(err));
				    fprintf(logf, "Failed to execute select statement for table: %s\n", entries[i].tablename);
				    OCI_Rollback(cn);
                OCI_ConnectionFree(cn);
                OCI_Cleanup();
                fclose(logf);
				    return 1;
		      }

            OCI_Resultset *rs = OCI_GetResultset(stSel);
            if (!OCI_FetchNext(rs)) continue;

            //OCI_Lob *srcLobList[MAX_COLUMNS], *dstLobList[MAX_COLUMNS];
            OCI_Lob **srcLobList = malloc(sizeof(OCI_Lob *) * sql_template.column_count);
            OCI_Lob **dstLobList = malloc(sizeof(OCI_Lob *) * sql_template.column_count);

            char bind_blob_param[64];

            for (int col = 0; col < column_count; ++col) {
                srcLobList[col] = OCI_GetLob(rs, col + 2);
                dstLobList[col] = OCI_LobCreate(cn, OCI_BLOB);
                unsigned int lobLen = OCI_LobGetLength(srcLobList[col]);
                hex_to_binary_sse3(srcLobList[col], &lobLen, dstLobList[col]);
            }

		      fprintf(logf, "DEBUG: Executing SQL: %s\n", sql_template.lock_sql);
            OCI_Prepare(stLock, sql_template.lock_sql);
            OCI_BindString(stLock, ":row_id", entries[i].rowid, (unsigned int)strlen(entries[i].rowid));
            OCI_Execute(stLock);

		      fprintf(logf, "DEBUG: Executing SQL: %s\n", sql_template.update_sql);
            OCI_Prepare(stUpd, sql_template.update_sql);
            OCI_BindString(stUpd, ":row_id", entries[i].rowid, (unsigned int)strlen(entries[i].rowid));
            for (int col = 0; col < column_count; ++col) {
                snprintf(bind_blob_param, sizeof(bind_blob_param), ":%s", column_maps[col].blob_col);
                OCI_BindLob(stUpd, bind_blob_param, dstLobList[col]);
            }
            OCI_Execute(stUpd);

            for (int i = 0; i < column_count; ++i) {
                if (srcLobList[i]) OCI_LobFree(srcLobList[i]);
                if (dstLobList[i]) OCI_LobFree(dstLobList[i]);
            }
            free(srcLobList);
            free(dstLobList);

        }

        OCI_Commit(cn);

        if (stSel) OCI_StatementFree(stSel);
        if (stUpd) OCI_StatementFree(stUpd);
        if (stLock) OCI_StatementFree(stLock);

	 }

    OCI_ConnectionFree(cn);
    OCI_Cleanup();
    fclose(logf);
    return 0;
}

