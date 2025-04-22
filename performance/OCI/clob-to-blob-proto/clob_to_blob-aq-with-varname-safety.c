
#include "ocilib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <emmintrin.h>

#define MAX_BATCH_SIZE 100
#define MAX_COLUMNS 8
#define DATA_SIZE_BUF 100 * 1048576
#define LOG_DIR "c2b-0log"
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

void oci_error_handler(OCI_Error *err) {
    fprintf(stderr, "[OCILib Error] %s
", OCI_ErrorGetString(err));
}

int hex_to_binary_sse3(OCI_Lob *hex_data, unsigned int *hex_length, OCI_Lob *binary_data) {
    unsigned char *binary_char_data = (unsigned char *)malloc(DATA_SIZE_BUF / 2);
    unsigned char *hex_char_data = (unsigned char *)malloc(DATA_SIZE_BUF + 1);
    size_t n = OCI_LobRead(hex_data, hex_char_data, *hex_length);
    if (n % 2 != 0) return 0;

    for (size_t i = 0; i < n; i += 2) {
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
        if (strncmp(line, "username:", 9) == 0) sscanf(line + 9, "%127s", out->user);
        else if (strncmp(line, "password:", 9) == 0) sscanf(line + 9, "%127s", out->pwd);
        else if (strncmp(line, "database:", 9) == 0) sscanf(line + 9, "%127s", out->db);
    }
    fclose(fp);
    return 1;
}

int dequeue_batch(OCI_Connection *cn, QueueEntry *entries, int max_entries) {
    OCI_TypeInfo *type = OCI_TypeInfoGet(cn, "AQ$_JMS_TEXT_MESSAGE", OCI_TIF_TYPE);
    OCI_Dequeue *deq = OCI_DequeueCreate(type, "clob_to_blob_queue");
    OCI_DequeueSetNavigation(deq, OCI_ADN_FIRST_MSG);
    OCI_DequeueSetMode(deq, OCI_ADM_REMOVE);
    OCI_DequeueSetVisibility(deq, OCI_AMV_IMMEDIATE);
    OCI_DequeueSetWaitTime(deq, 1);

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

int main(void) {
    mkdir(LOG_DIR, 0777);
    FILE *logf = open_log_file();
    if (!logf || !read_credentials(CREDENTIAL_FILE, &creds)) return 1;

    OCI_Initialize(oci_error_handler, NULL, OCI_ENV_DEFAULT);
    OCI_Connection *cn = OCI_ConnectionCreate(creds.db, creds.user, creds.pwd, OCI_SESSION_DEFAULT);

    QueueEntry entries[MAX_BATCH_SIZE];
    int num_entries = dequeue_batch(cn, entries, MAX_BATCH_SIZE);
    if (num_entries == 0) {
        fprintf(logf, "No messages dequeued.\n");
        return 0;
    }

    for (int i = 0; i < num_entries; ++i) {
        fprintf(logf, "Processing: %s %s\n", entries[i].tablename, entries[i].rowid);
        // Continue with actual CLOB/BLOB conversion here
    }

    OCI_Commit(cn);
    OCI_ConnectionFree(cn);
    OCI_Cleanup();
    fclose(logf);
    return 0;
}
