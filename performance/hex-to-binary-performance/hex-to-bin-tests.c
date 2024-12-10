#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include "testdata.h"
#include "base16_decoding_table.h"

//#define TESTDATALEN 104857600
#define TESTDATALEN 10485780

/* the resulting binary string is half the size of the input hex string
 * because every two hex characters map to one byte */
unsigned char result[TESTDATALEN/2];

void calcHex2bin()
{
    size_t i;
    char cur;
    unsigned char val;
    for (i = 0; i < TESTDATALEN; i++) {
        cur = testdata[i];
        if (cur >= 97) {
            val = cur - 97 + 10;
        } else if (cur >= 65) {
            val = cur - 65 + 10;
        } else {
            val = cur - 48;
        }
        /* even characters are the first half, odd characters the second half
         * of the current output byte */
        if (i%2 == 0) {
            result[i/2] = val << 4;
        } else {
            result[i/2] |= val;
        }
    }
}

void lookup256()
{
    size_t i;
    char cur;
    unsigned char val;
    for (i = 0; i < TESTDATALEN; i++) {
        cur = testdata[i];
        val = base16_decoding_table1[(int)cur];
        /* even characters are the first half, odd characters the second half
         * of the current output byte */
        if (i%2 == 0) {
            result[i/2] = val << 4;
        } else {
            result[i/2] |= val;
        }
    }
}

void lookup32k()
{
    size_t i;
    uint16_t *cur;
    unsigned char val;
    for (i = 0; i < TESTDATALEN; i+=2) {
        cur = (uint16_t*)(testdata+i);
        // apply bitmask to make sure that the first bit is zero
        val = base16_decoding_table2[*cur & 0x7fff];
        result[i/2] = val;
    }
}

void lookup64k()
{
    size_t i;
    uint16_t *cur;
    unsigned char val;
    for (i = 0; i < TESTDATALEN; i+=2) {
        cur = (uint16_t*)(testdata+i);
        val = base16_decoding_table3[*cur];
        result[i/2] = val;
    }
}

#define NUMTESTS 10

int main() {
    struct timespec before, after;
    unsigned long long checksum;
    int i;
    double elapsed;

	 fprintf(stderr, "TESTDATALEN: %d\n", TESTDATALEN);

    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        calcHex2bin();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("arithmetic solution took %f seconds\n", elapsed);

    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        lookup256();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("256 entries table took %f seconds\n", elapsed);

    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        lookup32k();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("32768 entries table took %f seconds\n", elapsed);

    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        lookup64k();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("65536 entries table took %f seconds\n", elapsed);

    return 0;
}

