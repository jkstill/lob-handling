#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <emmintrin.h> // SSE2 header

#include "testdata.h"
#include "base16_decoding_table.h"

//#define TESTDATALEN 104857600
#define TESTDATALEN 10485780

/* the resulting binary string is half the size of the input hex string
 * because every two hex characters map to one byte */
unsigned char result[TESTDATALEN/2];


// this SIMD code is much faster than the  lookup table code (test4)
// however, this does not speed up the clob converion much, as most time is spent in the database
void test6(void)
{
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

    size_t i;
    for (i = 0; i + 32 <= TESTDATALEN; i += 32) {
        // Load 32 hex characters
        __m128i chars_high = _mm_loadu_si128((const __m128i *)(testdata + i));
        __m128i chars_low = _mm_loadu_si128((const __m128i *)(testdata + i + 16));

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
        _mm_storeu_si128((__m128i *)(result + i / 2), bytes);
    }

    // Process any remaining characters
    for (; i < TESTDATALEN; i += 2) {
        unsigned char high = testdata[i];
        unsigned char low = testdata[i + 1];

        high = (high >= '0' && high <= '9') ? high - '0' :
               (high >= 'A' && high <= 'F') ? high - 'A' + 10 :
               (high >= 'a' && high <= 'f') ? high - 'a' + 10 : 0;

        low = (low >= '0' && low <= '9') ? low - '0' :
              (low >= 'A' && low <= 'F') ? low - 'A' + 10 :
              (low >= 'a' && low <= 'f') ? low - 'a' + 10 : 0;

        result[i / 2] = (high << 4) | low;
    }
}

void test1()
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

void test2()
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

void test3()
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

void test4()
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

void test5()
{
    static const unsigned char lookup[256] = {
        ['0'] = 0, ['1'] = 1, ['2'] = 2, ['3'] = 3,
        ['4'] = 4, ['5'] = 5, ['6'] = 6, ['7'] = 7,
        ['8'] = 8, ['9'] = 9,
        ['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13,
        ['E'] = 14, ['F'] = 15,
        ['a'] = 10, ['b'] = 11, ['c'] = 12, ['d'] = 13,
        ['e'] = 14, ['f'] = 15,
    };
    size_t i;
    unsigned char val;
    for (i = 0; i < TESTDATALEN; i++) {
        val = lookup[(unsigned char)testdata[i]];

        if (i % 2 == 0) {
            result[i/2] = val << 4;
        } else {
            result[i/2] |= val;
        }
    }
}

#define NUMTESTS 1000

int main() {
    struct timespec before, after;
    unsigned long long checksum;
    int i;
    double elapsed;

	 fprintf(stderr, "TESTDATALEN: %d\n", TESTDATALEN);

    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        test6();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup test6() took %f seconds\n", elapsed);

	 // -----

    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        test5();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup test5() took %f seconds\n", elapsed);

	 // -----

    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        test1();
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
        test2();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("256 entries table took %f seconds\n", elapsed);

	 // -----
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        test3();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("32768 entries table took %f seconds\n", elapsed);
	 // -----

    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        test4();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("65536 entries table took %f seconds\n", elapsed);
	 // -----

    return 0;
}
