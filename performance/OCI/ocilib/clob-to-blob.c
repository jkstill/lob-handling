
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#include "ocilib.h"
#include "clob-to-blob.h"

#define DATA_SIZE_BUF 8 * 1048576
#define DEBUG 0

// Function to convert a single hex character to its numerical value
unsigned char hex_char_to_value(char c) {
    if (c >= '0' && c <= '9')
        return (unsigned char)(c - '0');
    else if (c >= 'a' && c <= 'f')
        return (unsigned char)(c - 'a' + 10);
    else if (c >= 'A' && c <= 'F')
        return (unsigned char)(c - 'A' + 10);
    else {
        fprintf(stderr, "Invalid hex character: %c\n", c);
        //exit(EXIT_FAILURE);
		  return '~';
    }
}

// Function to convert hex string to binary data
//int hex_to_binary(const char *hex_data, size_t hex_length, unsigned char *binary_data) {
int hex_to_binary(OCI_Lob *hex_data, unsigned int *hex_length, OCI_Lob *binary_data) {
    size_t n;
    unsigned int hl = (*hex_length);

    unsigned char *binary_char_data = (unsigned char *)malloc(DATA_SIZE_BUF);
    char hex_char_data[DATA_SIZE_BUF+1];

    n = OCI_LobRead(hex_data, hex_char_data, hl);
    if (n % 2 != 0) {
        fprintf(stderr, "Hex data length is not even.\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < n / 2; i++) {
        unsigned char high_nibble = hex_char_to_value(hex_char_data[2 * i]);
        if (high_nibble == '~') {
            return '~';
        }

        unsigned char low_nibble = hex_char_to_value(hex_char_data[2 * i + 1]);
        if (low_nibble == '~') {
            return '~';
        }

        binary_char_data[i] = (high_nibble << 4) | low_nibble;
    }

    OCI_LobTruncate(binary_data, 0);
    OCI_LobSeek(binary_data, 0, OCI_SEEK_SET);
    OCI_LobAppend(binary_data, binary_char_data, n / 2);

    free(binary_char_data);
    return 1;
}

void err_handler(OCI_Error *err)
{
    printf("%s\n", OCI_ErrorGetString(err));
}

void usage (void) {

	printf("\nclob-to-blob: convert a CLOB column to a BLOB column\n");
	printf("   clob-to-blob database username password \n\n");


}

int main(int argc, oarg* argv[])
{
    otext dbs [SIZE_STR+1] = OTEXT("");
    otext usr [SIZE_STR+1] = OTEXT("");
    otext pwd [SIZE_STR+1] = OTEXT("");

    size_t i;

    /* CHECK COMMAND LINE --------------------------------------------------- */

    if (argc < (ARG_COUNT-1))
    {
		  usage();
        return EXIT_FAILURE;
    }

    /* GET ARGUMENTS ---------------------------------------------------------*/

    GET_ARG(dbs, ARG_DB);
    GET_ARG(usr, ARG_USER);
    GET_ARG(pwd, ARG_PWD);

    //if(argc == ARG_COUNT)
        //GET_ARG(home, ARG_HOME);

	fprintf(stderr,"starting\n");

	OCI_Connection *cn;
	OCI_Statement *st, *stBlob, *stUpd;
	OCI_Resultset *rs, *rsBlob;
	OCI_Lob *lob1, *lob2, *destLob;
	const char *rowIdStr;

	int id ;
	size_t n;

	if (!OCI_Initialize(err_handler, NULL, OCI_ENV_DEFAULT))
	{
		printf("something failed\n");
		return EXIT_FAILURE;
	}

	cn = OCI_ConnectionCreate(dbs, usr, pwd, OCI_SESSION_DEFAULT);
	st = OCI_StatementCreate(cn);
	stBlob = OCI_StatementCreate(cn);
	stUpd = OCI_StatementCreate(cn);

	OCI_ExecuteStmt(st, "select id, row_id from blobdest_rows where rownum < 11");

	rs = OCI_GetResultset(st);

	while (OCI_FetchNext(rs))
	{
		rowIdStr = OCI_GetString(rs,2);

		OCI_Prepare(stBlob, "select id, c1 from blobdest where rowid = :rowIdStr");
		OCI_BindString(stBlob,OTEXT(":rowIdStr"), (char *)rowIdStr,64);

		OCI_Execute(stBlob);

		rsBlob = OCI_GetResultset(stBlob);

		// convert this loop to "if" should be only 1 row by rowid
		// the "else" is an error condition
		
		if (OCI_FetchNext(rsBlob))
		{
			id = OCI_GetBigInt(rs,1);
			lob2 = OCI_LobCreate(cn, OCI_BLOB);
			lob1 = OCI_GetLob(rsBlob, 2);
			unsigned int srcLobLen = OCI_LobGetLength(lob1);

			OCI_LobSeek(lob1, 0, OCI_SEEK_SET );
			char retVal = hex_to_binary(lob1, &srcLobLen, lob2);
			if (retVal == '~' ) {
				fprintf( stderr, "Error converting Hex to Binary\n");
				break;
			}

			unsigned int lob1len = OCI_LobGetLength(lob1);
			unsigned int lob2len = OCI_LobGetLength(lob2);

			// best to use this only if using short test data
			if ( DEBUG == 1 ) {
				fprintf(stderr, "    id: %i\n", id);
				fprintf(stderr, "  blob id: %i\n",id);
				fprintf(stderr, "lob1 length %i\n", lob1len);
				fprintf(stderr, "lob2 length %i\n", lob2len);
			}


			OCI_Prepare(stUpd, "select 1  from blobdest where id = :id for update");
			OCI_BindInt(stUpd,OTEXT(":id"), &id);
			OCI_Execute(stUpd);

			OCI_Prepare(stUpd, "update blobdest set b1 = :blobData  where id = :id");

			OCI_BindInt(stUpd,OTEXT(":id"), &id);

			OCI_BindLob(stUpd,OTEXT(":blobData"), lob2);

			OCI_Execute(stUpd);

		} else {
			fprintf(stderr, "Error Fetching CLOB\n");
			fprintf(stderr, "  rowid: %s\n", rowIdStr);
			break;
		}

		OCI_LobFree(lob1);
		OCI_LobFree(lob2);

	}


	OCI_Commit(cn);

	fprintf(stderr, "\n%d row(s) updated\n", OCI_GetRowCount(rs));

	OCI_StatementFree(stBlob);
	OCI_StatementFree(st);
	OCI_StatementFree(stUpd);
	OCI_ConnectionFree(cn);
	OCI_Cleanup();

	return EXIT_SUCCESS;
}


