
#define _POSIX_C_SOURCE 200112L // for clock_gettime and timespec

#include <time.h>   // for clock_gettime
#include "ocilib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <getopt.h>

#include <emmintrin.h> // SSE2
#include <tmmintrin.h>  // for _mm_shuffle_epi8 (SSSE3)

#define MAX_BATCH_SIZE 100
#define MAX_COLUMNS 8
// some of the CLOBs are very large
#define DATA_SIZE_BUF 100 * 1048576
#define LOG_DIR "c2b-log"
#define CREDENTIAL_FILE "ora-creds.txt"

#define OCI_REBINDING_ENABLED TRUE

int VERBOSE = 0;
int DEBUG = 0;

char q_number[32];


OCI_Statement *stLoadColumnMetadata = NULL;

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


void usage(void) {
   printf ("\
Usage: getopt-long [OPTION] \
\n\
\n\
-d --debug\n\
-q --queue-id=QUEUE_ID\n\
-h --help\n\
\n");
}

int getopts(int argc, char **argv)
{
	int c;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"debug",     no_argument, 0,  'd' },
			{"queue-id",  required_argument,0,  'q' },
			{"help",    no_argument,       0, 'z'},
			{0,         0,                 0,  0 }
		};

		c = getopt_long(argc, argv, "?zdhq:", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {

			case 'q':
				printf("option q queue-id with value '%s'\n", optarg);
				// convert char padded to width of 3 with leading zeros
				snprintf(q_number, sizeof(q_number), "%03d", atoi(optarg));
				break;

			case 'd':
				DEBUG = 1;
				break;

			case '?':
				usage();
				return (EXIT_FAILURE);
			
			case 'z':
				usage();
				return (EXIT_FAILURE);

			case 'h':
				usage();
				return (EXIT_FAILURE);
				
			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
		printf("%s ", argv[optind++]);
		printf("\n");
		return(EXIT_SUCCESS);
	}

	// if any of the required options are missing, print usage and exit
	// if the length of the queue number is not 3, print usage and exit
	// if the queue number is not a number, print usage and exit
	if ( strlen(q_number) != 3 ) {
		fprintf(stderr, "Queue number must be between 0-999.\n");
		usage();
		return(EXIT_FAILURE);
	}

	return(EXIT_SUCCESS);
}

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
    for (i = 0; i + 32 <= n; i += 32) {
        // Load 32 hex characters into two registers
        __m128i block1 = _mm_loadu_si128((const __m128i *)(hex_char_data + i));
        __m128i block2 = _mm_loadu_si128((const __m128i *)(hex_char_data + i + 16));

        // Create shuffle masks for extracting even and odd indices
        // 0x80 indicates that byte is not used (masked out)
        __m128i idxEven = _mm_setr_epi8(
            0,  2,  4,  6,  8, 10, 12, 14,
            (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
        );
        __m128i idxOdd = _mm_setr_epi8(
            1,  3,  5,  7,  9, 11, 13, 15,
            (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
        );

        // Extract even and odd bytes from block1 and block2
        __m128i evens_block1 = _mm_shuffle_epi8(block1, idxEven); // even indices from first 16 chars
        __m128i odds_block1  = _mm_shuffle_epi8(block1, idxOdd);  // odd indices from first 16 chars
        __m128i evens_block2 = _mm_shuffle_epi8(block2, idxEven); // even indices from next 16 chars
        __m128i odds_block2  = _mm_shuffle_epi8(block2, idxOdd);  // odd indices from next 16 chars

        // Combine evens from block1 and block2 into one 128-bit register
        // evens_block1 has 8 valid bytes at the start
        // evens_block2 has 8 valid bytes at the start, we shift it left by 8 bytes and OR
        __m128i evens = _mm_or_si128(evens_block1, _mm_slli_si128(evens_block2, 8));

        // Combine odds similarly
        __m128i odds = _mm_or_si128(odds_block1, _mm_slli_si128(odds_block2, 8));

        // Now we have:
        // evens = [b0, b2, b4, b6, b8, b10, b12, b14, b16, b18, b20, b22, b24, b26, b28, b30]
        // odds  = [b1, b3, b5, b7, b9, b11, b13, b15, b17, b19, b21, b23, b25, b27, b29, b31]

        // Convert ASCII chars to nibbles
        __m128i zero = _mm_set1_epi8('0');

        // Subtract '0'
        evens = _mm_sub_epi8(evens, zero);
        odds  = _mm_sub_epi8(odds, zero);

        // Masks for uppercase and lowercase
        __m128i upperA = _mm_set1_epi8('A'-1);
        __m128i upperF = _mm_set1_epi8('F'+1);
        __m128i lowerA = _mm_set1_epi8('a'-1);
        __m128i lowerF = _mm_set1_epi8('f'+1);

        __m128i chars_evens = _mm_add_epi8(evens, zero); // convert back to ASCII space
        __m128i chars_odds  = _mm_add_epi8(odds, zero);

        __m128i ucase_mask_e = _mm_and_si128(_mm_cmpgt_epi8(chars_evens, upperA), _mm_cmplt_epi8(chars_evens, upperF));
        __m128i lcase_mask_e = _mm_and_si128(_mm_cmpgt_epi8(chars_evens, lowerA), _mm_cmplt_epi8(chars_evens, lowerF));
        __m128i ucase_mask_o = _mm_and_si128(_mm_cmpgt_epi8(chars_odds,  upperA), _mm_cmplt_epi8(chars_odds,  upperF));
        __m128i lcase_mask_o = _mm_and_si128(_mm_cmpgt_epi8(chars_odds,  lowerA), _mm_cmplt_epi8(chars_odds,  lowerF));

        // For uppercase: subtract additional 7
        evens = _mm_sub_epi8(evens, _mm_and_si128(ucase_mask_e, _mm_set1_epi8(7)));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(ucase_mask_o, _mm_set1_epi8(7)));

        // For lowercase: subtract additional 39
        evens = _mm_sub_epi8(evens, _mm_and_si128(lcase_mask_e, _mm_set1_epi8(39)));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(lcase_mask_o, _mm_set1_epi8(39)));

        // Now evens are high nibbles, odds are low nibbles
        __m128i high_shifted = _mm_slli_epi16(evens, 4);
        __m128i bytes = _mm_or_si128(high_shifted, odds);

        // Store result
        _mm_storeu_si128((__m128i *)(binary_char_data + i/2), bytes);
    }

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

	 // The SQL used to get the metadata for the CLOB/BLOB columns
	 // SELECT clob_column_name, blob_column_name FROM clob_to_blob_columns WHERE tablename = :tbl ORDER BY column_id

	 // get PID and cast to string
	 // this is a hack to get a unique identifier for the log file
	 // it is not used in the SQL
	 int pid = getpid();
	 char pid_str[16];
	 snprintf(pid_str, sizeof(pid_str), "%d", pid);

	 snprintf(select_buf, sizeof(select_buf), "SELECT /*+ PID: %s */ ", pid_str);
	 snprintf(update_buf, sizeof(update_buf), "UPDATE /*+ PID: %s */ ", pid_str);

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
    snprintf(sql_log_file, sizeof(sql_log_file), LOG_DIR "/sqlgen_%s-%s.log", pid_str, tablename);
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

int dequeue_batch(OCI_Connection *cn, QueueEntry *entries, int max_entries) {
	 OCI_TypeInfo *type = OCI_TypeInfoGet(cn, "AQ$_JMS_TEXT_MESSAGE", OCI_TIF_TYPE);

    if (!type) {
        fprintf(stderr, "Failed to get AQ type info.\n");
        return 0;
    }

	 // need to add queue number to the queue name
	 char queue_name[64];
	 snprintf(queue_name, sizeof(queue_name), "clob_to_blob_queue_%s", q_number);
    OCI_Dequeue *deq = OCI_DequeueCreate(type, queue_name);
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

void ferr_handler(OCI_Error *err,  FILE *logf)
{
    printf
    (
        "code  : ORA-%05i\n"
        "msg   : %s\n"
        "sql   : %s\n",
        OCI_ErrorGetOCICode(err),
        OCI_ErrorGetString(err),
        OCI_GetSql(OCI_ErrorGetStatement(err))
    );
	//fprintf(logf, "code  : ORA-%05i\n", OCI_ErrorGetOCICode(err));
}


void err_handler(OCI_Error *err)
{
    printf
    (
        "code  : ORA-%05i\n"
        "msg   : %s\n"
        "sql   : %s\n",
        OCI_ErrorGetOCICode(err),
        OCI_ErrorGetString(err),
        OCI_GetSql(OCI_ErrorGetStatement(err))
    );
	//fprintf(logf, "code  : ORA-%05i\n", OCI_ErrorGetOCICode(err));
}


int main(int argc, char **argv) {

	int getopts_success = getopts(argc, argv);
	if (getopts_success != 0) {
		exit(EXIT_FAILURE);
	}

    struct timespec batch_start, batch_end;

	 // per set of dequeued entries
    double elapsed_sec = 0.0; 
    double rows_per_sec = 0.0;

	 // entire batch
    double total_batch_time_sec = 0.0;
    unsigned long long total_rows_processed = 0;

	 int exit_code = 0;

    mkdir(LOG_DIR, 0777);
    FILE *logf = open_log_file();
    if (!logf || !read_credentials(CREDENTIAL_FILE, &creds)) return 1;
	 // flush log file after each write
	 setvbuf(logf, NULL, _IONBF, 0);

    //OCI_Initialize(oci_error_handler, NULL, OCI_ENV_DEFAULT|OCI_ENV_CONTEXT);
	 
	 // an attempt to automatically log errors to a file
	 // OCILib does not support this yet
	 //void (*errHandler)(OCI_Error *err, FILE *logf);
	 //errHandler = ferr_handler;
    // OCI_Initialize(errHandler, NULL, OCI_ENV_DEFAULT|OCI_ENV_CONTEXT);
    //OCI_Initialize(err_handler, NULL, OCI_ENV_DEFAULT|OCI_ENV_CONTEXT);
    OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT|OCI_ENV_CONTEXT);

    OCI_Connection *cn = OCI_ConnectionCreate(creds.db, creds.user, creds.pwd, OCI_SESSION_DEFAULT);
	 if (!cn) {
		  fprintf(logf, "Failed to connect to database: %s\n", OCI_ErrorGetString(OCI_GetLastError()));
		  OCI_Cleanup();
		  fclose(logf);
		  return 1;
	 }

    stLoadColumnMetadata = OCI_StatementCreate(cn);
    OCI_AllowRebinding(stLoadColumnMetadata, OCI_REBINDING_ENABLED);

    if(!OCI_Prepare(stLoadColumnMetadata, "SELECT clob_column_name, blob_column_name FROM clob_to_blob_columns WHERE tablename = :tbl ORDER BY column_id")) {
        OCI_Error *err = OCI_GetLastError();
        fprintf(logf, "[OCI_Prepare st] %s\n", OCI_ErrorGetString(err));
        return 1;
    }
	 

    //OCI_Statement *st = OCI_StatementCreate(cn);
    //OCI_ExecuteStmt(st, "alter session set tracefile_identifier = 'CLOBTOBLOB' ");
    //OCI_ExecuteStmt(st, "alter session set events '10046 trace name context forever, level 12'");

    OCI_Statement *stSel = OCI_StatementCreate(cn);
    OCI_Statement *stUpd = OCI_StatementCreate(cn);

    OCI_Lob **srcLobList = malloc(sizeof(OCI_Lob *) * MAX_COLUMNS);
    OCI_Lob **dstLobList = malloc(sizeof(OCI_Lob *) * MAX_COLUMNS);

    while (1) {

        clock_gettime(CLOCK_MONOTONIC, &batch_start);

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

        fprintf(logf, "Dequeued %d messages from the queue.\n", num_entries);

		  if (DEBUG) fprintf(logf, "DEBUG: top of main loop:\n");

        for (int i = 0; i < num_entries; ++i) {

			   // do not parse unless we have a new table
				// otherwise there is massive library lock contention with multiple clients parsing the same SQL
            if (strcmp(entries[i].tablename, last_tablename) != 0) {
					 if (DEBUG) fprintf(logf, "   DEBUG: Loading metadata for table: %s\n", entries[i].tablename);
					 if (DEBUG) fprintf(logf, "   DEBUG: Last table: %s Current table: %s \n", last_tablename, entries[i].tablename);

                if (!load_column_metadata(cn, stLoadColumnMetadata, entries[i].tablename, logf)) {
					     OCI_Error *err = OCI_GetLastError();
					     fprintf(logf, "[called load_column_metadata]  %s\n", OCI_ErrorGetString(err));
						  exit_code = 1;
						  break;
                }

                if (!generate_sql_templates(entries[i].tablename, logf)) {
					     OCI_Error *err = OCI_GetLastError();
					     fprintf(logf, "[generate_sql_templates] %s\n", OCI_ErrorGetString(err));
						  exit_code = 1;
						  break;
					 }

            }
                //strncpy(last_tablename, entries[i].tablename, sizeof(last_tablename));
				    strncpy(last_tablename, entries[i].tablename, sizeof(last_tablename) - 1);
                last_tablename[sizeof(last_tablename) - 1] = '\0';

					 /*
					    It is not genarally a good idea to reparse the same statement in a loop
						 However, if that is not done, the OCILib raises and error that the value is 'already binded to the statement'
						 There is no provision in OCILib to unbind a value from a statement
						 The OCI_AllowRebinding function can used to allow rebinding 
						 It has worked in simple test cases, and for load_column_metadata
						 But I have not had success with it in the main loop
					 */

                if (!OCI_Prepare(stSel, sql_template.select_sql)) {
					     OCI_Error *err = OCI_GetLastError();
					     fprintf(logf, "[OCI_Prepare stSel] %s:%s\n", OCI_ErrorGetString(err),sql_template.select_sql);
						  exit_code = 1;
						  break;
                }

		          if (DEBUG) fprintf(logf, "   DEBUG: RowId parse: %s|\n", entries[i].rowid);

		          if (!OCI_BindString(stSel, ":row_id", (otext *)entries[i].rowid, (unsigned int)(strlen(entries[i].rowid) + 1))) {
					     OCI_Error *err = OCI_GetLastError();
                    fprintf(logf, "[OCI Error stSel rowid] %s\n", OCI_ErrorGetString(err));
					     fprintf(logf, "[OCI_BindString stSel rowid] %s:%s\n", entries[i].rowid, sql_template.select_sql);
						  exit_code = 1;
						  break;
                }

		      if (DEBUG) {
					fprintf(logf, "   DEBUG: Executing SQL: %s|\n", sql_template.select_sql);
		         fprintf(logf, "   DEBUG: RowId execute: %s|\n", entries[i].rowid);
				}

            // an invalid rowid may be due to an message enqueued for a row that has been deleted
            if (!OCI_Execute(stSel)) {

					 char *rowIdErrorString = "ORA-01410";

                OCI_Error *err = OCI_GetLastError();
					 char *errString = (char *)OCI_ErrorGetString(err);

                fprintf(logf, "[OCI Error stSel Execute] %s\n", errString);
				    fprintf(logf, "[C2B Exec stSel Execute]:%s:%s:%s\n", entries[i].tablename,entries[i].rowid, sql_template.select_sql);

					 if (strstr(errString, rowIdErrorString) == NULL) {
	                 printf("\nlog file: %s/pid_%d.log\n\n", LOG_DIR, getpid());
						  exit_code = 1;
						  break;
						}
		      }

            OCI_Resultset *rs = OCI_GetResultset(stSel);
            if (!OCI_FetchNext(rs)) {
                OCI_Error *err = OCI_GetLastError();
                fprintf(logf, "[OCI Error stSel FetchNext] %s\n", OCI_ErrorGetString(err));
				    fprintf(logf, "[C2B Fetch stSel FetchNext]:%s:%s:%s\n", entries[i].tablename,entries[i].rowid, sql_template.select_sql);
				    exit_code = 1;
					 break;
				}


            char bind_blob_param[64];

            for (int col = 0; col < column_count; ++col) {
		          if (DEBUG) fprintf(logf, "      DEBUG: call hex_to_binary: %s\n", column_maps[col].clob_col);
                srcLobList[col] = OCI_GetLob(rs, col + 1);
                dstLobList[col] = OCI_LobCreate(cn, OCI_BLOB);
                unsigned int lobLen = OCI_LobGetLength(srcLobList[col]);
					 // log the size
					 if (DEBUG) fprintf(logf, "      DEBUG: CLOB size: %s:%d\n", column_maps[col].clob_col, lobLen);
                hex_to_binary_sse3(srcLobList[col], &lobLen, dstLobList[col]);
            }

		      if (DEBUG) fprintf(logf, "   DEBUG: Executing SQL: %s\n", sql_template.update_sql);

            if (!OCI_Prepare(stUpd, sql_template.update_sql)) {
					 OCI_Error *err = OCI_GetLastError();
					 fprintf(logf, "[OCI_Prepare stUpd] %s:%s\n", OCI_ErrorGetString(err),sql_template.update_sql);
				    exit_code = 1;
					 break;
		      }


            if (!OCI_BindString(stUpd, ":row_id", entries[i].rowid, (unsigned int)strlen(entries[i].rowid))) {

					 OCI_Error *err = OCI_GetLastError();
				    fprintf(logf, "[OCI Error stUpd] %s\n", OCI_ErrorGetString(err));
					 fprintf(logf, "[OCI_BindString stUpd] %s:%s\n", entries[i].rowid, sql_template.update_sql);
				    exit_code = 1;
					 break;
				}

            for (int col = 0; col < column_count; ++col) {

		          if (DEBUG) fprintf(logf, "      DEBUG: C2B bind column: %s\n", column_maps[col].blob_col);

                snprintf(bind_blob_param, sizeof(bind_blob_param), ":%s", column_maps[col].blob_col);

                if (! OCI_BindLob(stUpd, bind_blob_param, dstLobList[col]) ) {
						  OCI_Error *err = OCI_GetLastError();
						  fprintf(logf, "[OCI Error] %s\n", OCI_ErrorGetString(err));
				        fprintf(logf, "[C2B Bind]:%s:%s:%s\n", entries[i].tablename,entries[i].rowid, sql_template.update_sql);
				        exit_code = 1;
					     break;
					 }
            }

            if (!OCI_Execute(stUpd)) {
                OCI_Error *err = OCI_GetLastError();
                fprintf(logf, "[C2B Exec error stUpd] %s\n", OCI_ErrorGetString(err));
				    fprintf(logf, "[C2B Exec stUpd]:%s:%s:%s\n", entries[i].tablename,entries[i].rowid, sql_template.update_sql);
				    exit_code = 1;
					 break;
				}


            for (int i = 0; i < column_count; ++i) {
                if (dstLobList[i]) {
                    OCI_LobFree(dstLobList[i]);
                    dstLobList[i] = NULL;
                }
                srcLobList[i] = NULL; // makes sure you don't use stale pointers
            }

        }

        OCI_Commit(cn);
        free(entries);

        clock_gettime(CLOCK_MONOTONIC, &batch_end);
		  elapsed_sec = difftime(batch_end.tv_sec, batch_start.tv_sec) + (batch_end.tv_nsec - batch_start.tv_nsec)/1e9;

        total_batch_time_sec += elapsed_sec;
        total_rows_processed += num_entries;

        if (num_entries > 0 && elapsed_sec > 0.0) {
            rows_per_sec = num_entries / elapsed_sec;
        }

        fprintf(logf, "Batch timing: %.6f seconds for %d rows (%.2f rows/sec)\n", elapsed_sec, num_entries, rows_per_sec);

	 }

	 // free the statement handles
    if (stSel) OCI_StatementFree(stSel);
    if (stUpd) OCI_StatementFree(stUpd);
    if (stLoadColumnMetadata) OCI_StatementFree(stLoadColumnMetadata);

    for (int i = 0; i < column_count; ++i) {
        if (dstLobList[i]) OCI_LobFree(dstLobList[i]);
        dstLobList[i] = NULL;
        srcLobList[i] = NULL;
    }
    free(srcLobList);
    free(dstLobList);
    OCI_ConnectionFree(cn);
    OCI_Cleanup();

    if (total_rows_processed > 0) {
        fprintf(logf, "\n--- Summary ---\n");
        fprintf(logf, "Total rows processed: %llu\n", total_rows_processed);
        fprintf(logf, "Total time: %.6f seconds\n", total_batch_time_sec);
        fprintf(logf, "Average rows/sec: %.2f\n", total_rows_processed / total_batch_time_sec);
        fprintf(logf, "Average seconds/row: %.6f\n", total_batch_time_sec / total_rows_processed);
    }

    fclose(logf);

	 printf("\nlog file: %s/pid_%d.log\n\n", LOG_DIR, getpid());

    return exit_code;
}

