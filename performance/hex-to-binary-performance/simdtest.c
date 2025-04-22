#include <stdio.h>
#include <string.h>
#include <emmintrin.h>   // SSE2
#include <tmmintrin.h>   // SSSE3

/*

A couple hours spent with ChatGPT to try and get the SSE2 code working.

Gave up on sse2, but the SSSE3 code is working well.

Going to stick with SSE3.
 
 
*/

//#define TESTDATALEN 66  // 32 bytes hex = 64 characters
#define TESTDATALEN 48  // 24 bytes hex = 48 characters
//unsigned char testdata[TESTDATALEN + 1] = "0123456789ABCDEFabcdef0123456789ABCDEFabcdef0123456789ABCDEFabcdef";
unsigned char testdata[TESTDATALEN + 1] = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
unsigned char result_ssse3[TESTDATALEN / 2];
unsigned char result_sse2[TESTDATALEN / 2];

void superScalarSSSE3(void);
void superScalarSSE2(void);

// this SIMD code is much faster than the  lookup table code (test4)
// this is 10x faster than the hex lookup table
void superScalarSSE2(void)
{
    const __m128i ascii0 = _mm_set1_epi8('0');
    const __m128i nine   = _mm_set1_epi8(9);
    const __m128i adj    = _mm_set1_epi8(7);

    size_t i;
    for (i = 0; i + 32 <= TESTDATALEN; i += 32) {
        const unsigned char* src = testdata + i;

        // Load 32 characters into two 128-bit registers
        __m128i chunk1 = _mm_loadu_si128((const __m128i*)(src));
        __m128i chunk2 = _mm_loadu_si128((const __m128i*)(src + 16));

        // Interleave manually to extract even and odd nibbles
        // Step 1: unpack to words to isolate bytes
        __m128i lo1 = _mm_unpacklo_epi8(chunk1, _mm_setzero_si128());
        __m128i hi1 = _mm_unpackhi_epi8(chunk1, _mm_setzero_si128());
        __m128i lo2 = _mm_unpacklo_epi8(chunk2, _mm_setzero_si128());
        __m128i hi2 = _mm_unpackhi_epi8(chunk2, _mm_setzero_si128());

        // Extract even bytes: byte 0,2,4,...30
        __m128i even_bytes = _mm_setzero_si128();
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 0), 0);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 2), 1);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 4), 2);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 6), 3);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 0), 4);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 2), 5);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 4), 6);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 6), 7);

        // Repeat for second 16 bytes
        __m128i even_bytes2 = _mm_setzero_si128();
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 0), 0);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 2), 1);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 4), 2);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 6), 3);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 0), 4);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 2), 5);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 4), 6);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 6), 7);

        // Merge to get full even-byte vector
        __m128i evens = _mm_castps_si128(
            _mm_shuffle_ps(_mm_castsi128_ps(even_bytes), _mm_castsi128_ps(even_bytes2), _MM_SHUFFLE(1, 0, 1, 0))
        );

        // Now do the same for odd-indexed characters
        __m128i odd_bytes = _mm_setzero_si128();
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(lo1, 1), 0);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(lo1, 3), 1);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(lo1, 5), 2);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(lo1, 7), 3);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(hi1, 1), 4);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(hi1, 3), 5);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(hi1, 5), 6);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(hi1, 7), 7);

        __m128i odd_bytes2 = _mm_setzero_si128();
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(lo2, 1), 0);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(lo2, 3), 1);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(lo2, 5), 2);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(lo2, 7), 3);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(hi2, 1), 4);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(hi2, 3), 5);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(hi2, 5), 6);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(hi2, 7), 7);

        __m128i odds = _mm_castps_si128(
            _mm_shuffle_ps(_mm_castsi128_ps(odd_bytes), _mm_castsi128_ps(odd_bytes2), _MM_SHUFFLE(1, 0, 1, 0))
        );

        // Now decode ASCII to nibbles
        evens = _mm_sub_epi8(evens, ascii0);
        odds  = _mm_sub_epi8(odds,  ascii0);

        __m128i mask_e = _mm_cmpgt_epi8(evens, nine);
        __m128i mask_o = _mm_cmpgt_epi8(odds,  nine);

        evens = _mm_add_epi8(evens, _mm_and_si128(mask_e, adj));
        odds  = _mm_add_epi8(odds,  _mm_and_si128(mask_o, adj));

        // Combine nibbles
        __m128i high_nibbles = _mm_slli_epi16(evens, 4);
        __m128i bytes = _mm_or_si128(high_nibbles, odds);

        _mm_storeu_si128((__m128i *)(result_sse2 + i / 2), bytes);
    }

    // Scalar fallback for remaining
    for (; i + 1 < TESTDATALEN; i += 2) {
        unsigned char high = testdata[i];
        unsigned char low  = testdata[i + 1];

        high = (high >= '0' && high <= '9') ? high - '0' :
               (high >= 'A' && high <= 'F') ? high - 'A' + 10 :
               (high >= 'a' && high <= 'f') ? high - 'a' + 10 : 0;

        low = (low >= '0' && low <= '9') ? low - '0' :
              (low >= 'A' && low <= 'F') ? low - 'A' + 10 :
              (low >= 'a' && low <= 'f') ? low - 'a' + 10 : 0;

        result_sse2[i / 2] = (high << 4) | low;
    }
}



// this SIMD code is much faster than the  lookup table code (lookup64k)
// however, this does not speed up the clob converion much, as most time is spent in the database
void superScalarSSSE3(void)
{
    strcpy((char *)result_ssse3, "\0");

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
        _mm_storeu_si128((__m128i *)(result_ssse3 + i/2), bytes);
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

        result_ssse3[i / 2] = (high << 4) | low;
    }
}

void print_result(const char *label, const unsigned char *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

int main(void) {
    // Initialize output buffers
    memset(result_ssse3, 0, sizeof(result_ssse3));
    memset(result_sse2, 0, sizeof(result_sse2));

    // Convert
    superScalarSSSE3();
    superScalarSSE2();

    // Show output
    print_result("SSSE3", result_ssse3, TESTDATALEN / 2);
    print_result(" SSE2", result_sse2, TESTDATALEN / 2);

    // Compare
    int diff = 0;
    for (size_t i = 0; i < TESTDATALEN / 2; ++i) {
        if (result_ssse3[i] != result_sse2[i]) {
            printf("Mismatch at byte %zu: SSSE3=0x%02X SSE2=0x%02X\n",
                   i, result_ssse3[i], result_sse2[i]);
            diff++;
        }
    }

    if (!diff) {
        printf("✅ Outputs match!\n");
    } else {
        printf("❌ %d mismatches found\n", diff);
    }

    return diff;
}
