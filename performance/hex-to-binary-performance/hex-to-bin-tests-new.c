#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>
#include <emmintrin.h> // SSE2 header
#include <tmmintrin.h>  // for _mm_shuffle_epi8 (SSSE3)							  

#include "testdata.h"
#include "base16_decoding_table.h"

#define TESTDATALEN 10000000

/* the resulting binary string is half the size of the input hex string
 * because every two hex characters map to one byte */
unsigned char result[TESTDATALEN/2];

// this SIMD code is much faster than the  lookup table code (test4)
// this is 10x faster than the hex lookup table
// cannot get sse2 code to work properly
// spent a couple hours on this with ChatGPT, no success
// stick with SSSE3
void superScalarSSE2(void)
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
        //__m128i high_nibbles_shifted = _mm_slli_epi16(high_nibbles, 4);
        //__m128i bytes = _mm_or_si128(high_nibbles_shifted, low_nibbles);

// Shift high nibbles into upper 4 bits of each byte
__m128i high_shifted = _mm_slli_epi16(high_nibbles, 4);

// OR high and low nibbles together (still 16-bit lanes)
__m128i combined = _mm_or_si128(high_shifted, low_nibbles);

// Pack 16-bit lanes down to 8-bit
__m128i bytes = _mm_packus_epi16(combined, _mm_setzero_si128());

// Store the lower 16 bytes (128 bits)
_mm_storeu_si128((__m128i *)(result + i / 2), bytes);

        // Store the result
        //_mm_storeu_si128((__m128i *)(result + i / 2), bytes);
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


// this SIMD code is much faster than the  lookup table code (lookup64k)
// however, this does not speed up the clob converion much, as most time is spent in the database
void superScalarSSSE3(void)
{
    strcpy((char *)result, "\0");

    size_t i;
    for (i = 0; i + 32 <= TESTDATALEN; i += 32) {
        // Load 32 hex characters into two registers
        __m128i block1 = _mm_loadu_si128((const __m128i *)(testdata + i));
        __m128i block2 = _mm_loadu_si128((const __m128i *)(testdata + i + 16));

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
        _mm_storeu_si128((__m128i *)(result + i/2), bytes);
    }

    // Process any remaining characters (less than 32)
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


void calcHex()
{
    size_t i;
    char cur;
    unsigned char val;
	 strcpy(result, "\0");
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
	 strcpy(result, "\0");
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
	 strcpy(result, "\0");
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
	 strcpy(result, "\0");
    for (i = 0; i < TESTDATALEN; i+=2) {
        cur = (uint16_t*)(testdata+i);
        val = base16_decoding_table3[*cur];
        result[i/2] = val;
    }
}

void lookupBasic()
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
	 strcpy(result, "\0");
    for (i = 0; i < TESTDATALEN; i++) {
        val = lookup[(unsigned char)testdata[i]];

        if (i % 2 == 0) {
            result[i/2] = val << 4;
        } else {
            result[i/2] |= val;
        }
    }
}

void writeResults(char *filename)
{
	 FILE *fp;
	 char fqnName[256] = "data/";
	 strcat(fqnName, filename);
	 //printf("writing to %s\n", fqnName);
	 fp = fopen(fqnName, "wb");
	 fwrite(result, 1, TESTDATALEN/2, fp);
	 fclose(fp);
}

void writeHex(char *filename)
{
	 FILE *fp;
	 char fqnName[256] = "data/";
	 long unsigned int bytesWritten;
	 strcat(fqnName, filename);
	 //printf("writing to %s\n", fqnName);
	 fp = fopen(fqnName, "w");
	 bytesWritten = fwrite(testdata, 1, TESTDATALEN, fp);
	 fclose(fp);

}



#define NUMTESTS 100

int main() {
    struct timespec before, after;
    unsigned long long checksum;
    int i;
    double elapsed;

	 //printf("TESTDATALEN: %d\n", TESTDATALEN);
	 writeHex("testdata.hex");

	 // basic
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        calcHex();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 writeResults("calcHex.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("arithmetic solution calcHex() took %f seconds\n", elapsed);


	 //-- SuperScalar SSE2
	 // SSSE2 code is not working
    //clock_gettime(CLOCK_MONOTONIC, &before);
    //for (i = 0; i < NUMTESTS; i++) {
        //superScalarSSE2();
    //}
    //clock_gettime(CLOCK_MONOTONIC, &after);

    //checksum = 0;
    //for (i = 0; i < TESTDATALEN/2; i++) {
        //checksum += result[i];
    //}
    //printf("checksum: %llu\n", checksum);
    //elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    //printf("optimized lookup superScalarSSE2() took %f seconds\n", elapsed);
	 //writeResults("superScalerSSE2.dat");

	 //-- SuperScalar SSSE3
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        superScalarSSSE3();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup superScalarSSSE3() took %f seconds\n", elapsed);
	 writeResults("superScalerSSSE3.dat");

	 // -----

	 // lookupBasic
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        lookupBasic();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("checksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup lookupBasic() took %f seconds\n", elapsed);
	 writeResults("lookupBasic.dat");

	 // -----
    // lookup256
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
    printf("256 entries table lookup256() took %f seconds\n", elapsed);
	 writeResults("lookup256.dat");

	 // -----
	 // lookup32k
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
    printf("32768 entries table lookup32k() took %f seconds\n", elapsed);
	 writeResults("lookup32k.dat");

	 // -----
    // lookup64k
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
    printf("65536 entries table lookup64k() took %f seconds\n", elapsed);
	 writeResults("lookup64k.dat");
	 // -----

    return 0;
}
