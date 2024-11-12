
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ocilib.h"

#define SIZE_BUF 8 * 1048576

#define DEBUG 1
#define OUTPUT_FILE "hextest.bin"
#define OUTPUT_FILE2 "hextest2.bin"

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

    unsigned char *binary_char_data = (unsigned char *)malloc(SIZE_BUF);
    char hex_char_data[SIZE_BUF+1];

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

// Function to convert hex string to binary data
int hex_to_bintest(unsigned char *hex_data, unsigned int hex_length, unsigned char *binary_data) {
	if (hex_length % 2 != 0) {
		fprintf(stderr, "hex_to_bintest: Hex data length is not even.\n");
		return '~';
	}

	if ( DEBUG == 1 )
		fprintf(stderr, "hex_to_bintest hex_length: %i\n",hex_length);

	for (unsigned int i = 0; i < hex_length / 2; i++) {
		unsigned char high_nibble = hex_char_to_value(hex_data[2 * i]);
		unsigned char low_nibble = hex_char_to_value(hex_data[2 * i + 1]);
		binary_data[i] = (high_nibble << 4) | low_nibble;
	}

	return 1;
}


void err_handler(OCI_Error *err)
{
    printf("%s\n", OCI_ErrorGetString(err));
}

int main(void)
{
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

	cn = OCI_ConnectionCreate("lestrade/pdb01", "jkstill", "grok", OCI_SESSION_DEFAULT);
	st = OCI_StatementCreate(cn);
	stBlob = OCI_StatementCreate(cn);
	stUpd = OCI_StatementCreate(cn);

	OCI_ExecuteStmt(st, "select id, row_id from blobdest_rows");

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
		
		while (OCI_FetchNext(rsBlob))
		{
			id = OCI_GetBigInt(rs,1);
			lob2 = OCI_LobCreate(cn, OCI_BLOB);
			lob1 = OCI_GetLob(rsBlob, 2);
			unsigned int srcLobLen = OCI_LobGetLength(lob1);

				
			// get the hex and convert to binary
			// a file hextest.bin is created
			// this is to verify that the hex_to_binary algorithm works correctly
			// hex_to_bintest has the same algorithm as hex_to_binary, without the LOB bits
			// easist to use with 1 row in table blobdest with c1 = "FFD8FFE000104A464946000101000001"
			if ( DEBUG == 1 ) {
				unsigned char *hex_data = (unsigned char *)malloc(SIZE_BUF);
				unsigned int max_lob_len = 2147483648;
				unsigned int lob_char_len = 0;
			
				OCI_LobSeek(lob1, 0, OCI_SEEK_SET );
				OCI_LobRead2(lob1, hex_data, &lob_char_len, &max_lob_len);

				unsigned int hex_length = strlen(hex_data) ;
				unsigned char *binary_data = (unsigned char *)malloc(hex_length / 2 );
				size_t binary_size = hex_length / 2;	

				int hexResult = hex_to_bintest(hex_data, hex_length, binary_data);
			
				// Write binary data to output file
				// should be FFD8FFE000104A464946000101000001 for the 32 char/16 byte test data
				// hexdump -C hextest.bin
				FILE *out_fp = fopen(OUTPUT_FILE, "wb");
				if (out_fp == NULL) {
					perror("Cannot open output file");
					free(binary_data);
					exit(EXIT_FAILURE);
				}

				fwrite(binary_data, 1, binary_size, out_fp);
				fclose(out_fp);

				OCI_LobSeek(lob2, 0, OCI_SEEK_SET );
				OCI_LobTruncate(lob2, 0 );
				//OCI_LobWrite(lob2,binary_data,(unsigned int) sizeof(binary_data));
				OCI_LobWrite(lob2,binary_data,(unsigned int) binary_size);

			}

			//free(binary_data);
			//free(hex_data);

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

			if ( DEBUG == 1 ) {
				unsigned char *lob_data = (unsigned char *)malloc(SIZE_BUF);
				unsigned int max_lob_len = 2147483648;
				unsigned int lob_char_len = 0;
			
				OCI_LobSeek(lob2, 0, OCI_SEEK_SET );
				//OCI_LobRead2(lob2, lob_data, &lob2len, &lob2len);
				OCI_LobRead(lob2, lob_data, lob2len);

				unsigned int lob_length = sizeof(lob_data) ;
				unsigned char *binary_data = (unsigned char *)malloc(lob_length / 2 );
				size_t binary_size = lob_length / 2;	

				fprintf(stderr, "main 2 lob_data: |%s|\n", lob_data);
				fprintf(stderr, "main 2 lob_data size: %u\n", sizeof(lob_data) );
				fprintf(stderr, "main 2 lob_length: %u\n", lob_length );

				// Write binary data to output file
				// should be FFD8FFE000104A464946000101000001 for the 32 char/16 byte test data
				// hexdump -C hextest.bin

				FILE *out_fp = fopen(OUTPUT_FILE2, "wb");
				if (out_fp == NULL) {
					perror("Cannot open output file");
					free(binary_data);
					exit(EXIT_FAILURE);
				}

				fwrite(lob_data, 1, lob2len, out_fp);
				fclose(out_fp);

			}


			OCI_Prepare(stUpd, "select 1  from blobdest where id = :id for update");
			OCI_BindInt(stUpd,OTEXT(":id"), &id);
			OCI_Execute(stUpd);

			OCI_Prepare(stUpd, "update blobdest set b1 = :blobData  where id = :id");

			OCI_BindInt(stUpd,OTEXT(":id"), &id);

			OCI_BindLob(stUpd,OTEXT(":blobData"), lob2);

			OCI_Execute(stUpd);

		}

		OCI_LobFree(lob1);
		OCI_LobFree(lob2);

	}


	OCI_ExecuteStmt(st, "commit");

	fprintf(stderr, "\n%d row(s) fetched\n", OCI_GetRowCount(rs));

	OCI_StatementFree(stBlob);
	OCI_StatementFree(st);
	OCI_StatementFree(stUpd);
	OCI_ConnectionFree(cn);
	OCI_Cleanup();

	return EXIT_SUCCESS;
}



