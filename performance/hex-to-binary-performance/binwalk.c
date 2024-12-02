#include <stdio.h>
#include <time.h>
#include <inttypes.h>

#include "testdata.h"
#include "base16_decoding_table.h"

//#define TESTDATALEN 104857600
#define TESTDATALEN 10485780

/* the resulting binary string is half the size of the input hex string
 * because every two hex characters map to one byte */
unsigned char result[TESTDATALEN/2];

void test1()
{
    size_t i;
    char cur;
    unsigned char val;
    for (i = 0; i < 32; i++) {
        cur = testdata[i];
		  printf("cur: %c %i\n", cur,cur);
        if (cur >= 97) {
            val = cur - 97 + 10;
        } else if (cur >= 65) {
            val = cur - 65 + 10;
        } else {
            val = cur - 48;
        }
		  printf("val: %d\n", val);
        /* even characters are the first half, odd characters the second half
         * of the current output byte */
        if (i%2 == 0) {
			  printf("1 even: result[%lu] = %d\n", i/2, val << 4);
            result[i/2] = val << 4;
        } else {
			  printf("2 odd: result[%lu] = %d\n", i/2, val);
            result[i/2] |= val;
			printf("result[%lu] = %d\n", i/2, result[i/2]);
		  	printf("============================================\n");
        }
    }
}

#define NUMTESTS 1

int main() {
    struct timespec before, after;
    unsigned long long checksum;
    int i;
    double elapsed;

	 fprintf(stderr, "TESTDATALEN: %d\n", TESTDATALEN);

    for (i = 0; i < NUMTESTS; i++) {
        test1();
    }

    return 0;
}
