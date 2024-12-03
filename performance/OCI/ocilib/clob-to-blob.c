
#include "ocilib.h"
#include "clob-to-blob.h"
#include "base16_decoding_table.h"
#include <emmintrin.h> // SSE2 header SIMD code
#include <inttypes.h>

/*
compile:
# shared
#gcc -msse2 -O3 -o clob-to-blob clob-to-blob.c -O2 -L$ORACLE_HOME/lib -lclntsh  -L/usr/local/lib -locilib -std=c99

# static - no need to install ocilib
gcc -msse2 -O3 -o clob-to-blob clob-to-blob.c -O2 -l:libocilib.a -L$ORACLE_HOME/lib -lclntsh  -L/usr/local/lib -std=c99
*/

#define DATA_SIZE_BUF 100 * 1048576
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

	if ( DEBUG == 1) fprintf(stderr,"      hex_to_binary:  assign buffers\n");
	unsigned char *binary_char_data = (unsigned char *)malloc(DATA_SIZE_BUF/2);
	unsigned char *hex_char_data  = (unsigned char *)malloc(DATA_SIZE_BUF+1);
	if ( DEBUG == 1) fprintf(stderr,"      hex_to_binary:  buffers assigned\n");


	if ( DEBUG == 1) {
		fprintf(stderr,"      hex_to_binary - h:l %u\n",hl);;
	}

	if ( DEBUG == 1) fprintf(stderr,"      hex_to_binary: calling OCI_LobRead\n");
	n = OCI_LobRead(hex_data, hex_char_data, hl);

	if ( DEBUG == 1) fprintf(stderr,"      hex_to_binary: returned from OCI_LobRead\n");

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

int hex_to_binary_new(OCI_Lob *hex_data, unsigned int *hex_length, OCI_Lob *binary_data) {
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
    uint16_t *cur;
    unsigned char val;
    for (i = 0; i < n; i+=2) {
        cur = (uint16_t*)(hex_char_data+i);
        // apply bitmask to make sure that the first bit is zero
        val = base16_decoding_table2[*cur & 0x7fff];
        binary_char_data[i/2] = val;
    }


    OCI_LobTruncate(binary_data, 0);
    OCI_LobSeek(binary_data, 0, OCI_SEEK_SET);
    OCI_LobAppend(binary_data, binary_char_data, n / 2);

    free(binary_char_data);
    return 1;
}

// using SIMD instructions
// this SIMD code is much faster than the  lookup table code (test4)
// however, this does not speed up the clob converion much, as most time is spent in the database

int hex_to_binary_new2(OCI_Lob *hex_data, unsigned int *hex_length, OCI_Lob *binary_data) {
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
    uint16_t *cur;
    unsigned char val;
	 // SIMD code
	 
    // Constants stored in arrays
    static const unsigned char lookup_high_values[16] = {
        0x00, 0x10, 0x20, 0x30,
        0x40, 0x50, 0x60, 0x70,
        0x80, 0x90, 0xA0, 0xB0,
        0xC0, 0xD0, 0xE0, 0xF0
    };

    static const unsigned char lookup_low_values[16] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F
    };

    // Load constants into __m128i variables at runtime
    __m128i lookup_high = _mm_loadu_si128((const __m128i *)lookup_high_values);
    __m128i lookup_low = _mm_loadu_si128((const __m128i *)lookup_low_values);


    for (i = 0; i + 32 <= n; i += 32) {
        // Load 32 hex characters
        __m128i chars_high = _mm_loadu_si128((const __m128i *)(hex_char_data + i));
        __m128i chars_low = _mm_loadu_si128((const __m128i *)(hex_char_data + i + 16));

        // Convert hex characters to their numeric values
        // For simplicity, let's assume that all characters are valid hex digits (0-9, A-F, a-f)
        // You should add validation if necessary
    
        // Subtract '0' or 'A'-10 to get the numeric value
        __m128i mask_num = _mm_set1_epi8(0x0F);

        // Process high nibbles
        __m128i high_nibbles = _mm_sub_epi8(chars_high, _mm_set1_epi8('0'));
        __m128i high_mask = _mm_cmpgt_epi8(high_nibbles, _mm_set1_epi8(9));
        high_nibbles = _mm_add_epi8(high_nibbles, _mm_and_si128(high_mask, _mm_set1_epi8(39))); // Adjust for 'A'-'0'-10

        // Process low nibbles
        __m128i low_nibbles = _mm_sub_epi8(chars_low, _mm_set1_epi8('0'));
        __m128i low_mask = _mm_cmpgt_epi8(low_nibbles, _mm_set1_epi8(9));
        low_nibbles = _mm_add_epi8(low_nibbles, _mm_and_si128(low_mask, _mm_set1_epi8(39))); // Adjust for 'A'-'0'-10

        // Pack nibbles into bytes
        __m128i high_nibbles_shifted = _mm_slli_epi16(high_nibbles, 4);
        __m128i bytes = _mm_or_si128(high_nibbles_shifted, low_nibbles);

        // Store the result
        _mm_storeu_si128((__m128i *)(binary_char_data + i / 2), bytes);
    }

    // Process any remaining characters
    for (; i < n; i += 2) {
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

	OCI_ExecuteStmt(st, "select id, row_id from blobdest_rows -- where rownum < 101");
	if ( DEBUG == 1) fprintf(stderr,"blobdest_rows query executed\n");

	rs = OCI_GetResultset(st);

	while (OCI_FetchNext(rs))
	{
		rowIdStr = OCI_GetString(rs,2);

		OCI_Prepare(stBlob, "select id, c1 from blobdest where rowid = :rowIdStr");
		OCI_BindString(stBlob,OTEXT(":rowIdStr"), (char *)rowIdStr,64);

		OCI_Execute(stBlob);
		if ( DEBUG == 1) fprintf(stderr,"  blobdest query executed\n");

		rsBlob = OCI_GetResultset(stBlob);
		if ( DEBUG == 1) fprintf(stderr,"    blobdest query result\n");

		// convert this loop to "if" should be only 1 row by rowid
		// the "else" is an error condition
		
		if (OCI_FetchNext(rsBlob))
		{
			id = OCI_GetBigInt(rs,1);
			lob2 = OCI_LobCreate(cn, OCI_BLOB);
			lob1 = OCI_GetLob(rsBlob, 2);
			unsigned int srcLobLen = OCI_LobGetLength(lob1);

			if ( DEBUG == 1) fprintf(stderr,"    srcLobLen: %i\n",srcLobLen);

			if ( DEBUG == 1) fprintf(stderr,"    calling OCI_LobSeek\n");
			OCI_LobSeek(lob1, 0, OCI_SEEK_SET );
			if ( DEBUG == 1) fprintf(stderr,"    returned from OCI_LobSeek\n");

			if ( DEBUG == 1) fprintf(stderr,"    calling hex_to_binary\n");
			char retVal = hex_to_binary_new(lob1, &srcLobLen, lob2);
			if ( DEBUG == 1) fprintf(stderr,"    returned from hex_to_binary\n");

			if (retVal == '~' ) {
				fprintf( stderr, "Error converting Hex to Binary\n");
				break;
			}

			if ( DEBUG == 1) fprintf(stderr,"    calling OCI_LobGetLength\n");
			unsigned int lob1len = OCI_LobGetLength(lob1);
			unsigned int lob2len = OCI_LobGetLength(lob2);
			if ( DEBUG == 1) fprintf(stderr,"    returned from OCI_LobGetLength\n");

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

		if ( DEBUG == 1) fprintf(stderr,"===================================================\n");

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


