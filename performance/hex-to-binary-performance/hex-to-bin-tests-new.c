#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>
#include <emmintrin.h> // SSE2 header
#include <immintrin.h>  // AVX2, SSE
#include <tmmintrin.h>  // for _mm_shuffle_epi8 (SSSE3)							  

#include "testdata.h"
#include "base16_decoding_table.h"

//#define TESTDATALEN 10000000
//
// 2M and 100 tests for large test data
// adjust create-testdata-header.sh to create 2M test data
#define TESTDATALEN 2097152
#define NUMTESTS 1000

// adjust create-testdata-header.sh to create 288 test data
// 288 and 1M tests for small test data
//#define TESTDATALEN 288
//#define NUMTESTS 1000000

/* the resulting binary string is half the size of the input hex string
 * because every two hex characters map to one byte */
unsigned char result[TESTDATALEN/2];

static inline unsigned char toNib(unsigned char c) {
    unsigned char lower = (unsigned char)(c | 0x20);                    // make letters lowercase
    unsigned char isLet = (unsigned)((unsigned)lower - 'a') <= ('f' - 'a'); // 'a'..'f'?
    return (unsigned char)((c & 0x0Fu) + (isLet ? 9u : 0u));            // digit: 0..9, letter: +9 => 10..15
}


void superScalarAVX2_opt(void)
{
    memset(result, 0, sizeof(result));

    const __m256i lower_mask = _mm256_set1_epi8(0x20);
    const __m256i ch_a_m1    = _mm256_set1_epi8('a' - 1);
    const __m256i ch_f_p1    = _mm256_set1_epi8('f' + 1);
    const __m256i add9       = _mm256_set1_epi8(9);
    const __m256i low_nib    = _mm256_set1_epi8(0x0F);

    const __m256i idxEven = _mm256_setr_epi8(
         0,  2,  4,  6,  8, 10, 12, 14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
         0,  2,  4,  6,  8, 10, 12, 14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
    );
    const __m256i idxOdd  = _mm256_setr_epi8(
         1,  3,  5,  7,  9, 11, 13, 15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
         1,  3,  5,  7,  9, 11, 13, 15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
    );

    size_t i = 0;
    for (; i + 64 <= TESTDATALEN; i += 64) {
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(testdata + i));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(testdata + i + 32));

        // Shuffle evens/odds per 128-bit lane (each has bytes in positions 0..7 of each lane)
        __m256i e0 = _mm256_shuffle_epi8(v0, idxEven);
        __m256i o0 = _mm256_shuffle_epi8(v0, idxOdd);
        __m256i e1 = _mm256_shuffle_epi8(v1, idxEven);
        __m256i o1 = _mm256_shuffle_epi8(v1, idxOdd);

        // --- Correctly assemble within each 256b load ---
        // From e0: low lane (0..7) then high lane (0..7) shifted into 8..15
        __m128i e0_lo = _mm256_castsi256_si128(e0);
        __m128i e0_hi = _mm256_extracti128_si256(e0, 1);
        __m128i e0_128 = _mm_or_si128(e0_lo, _mm_slli_si128(e0_hi, 8));

        __m128i o0_lo = _mm256_castsi256_si128(o0);
        __m128i o0_hi = _mm256_extracti128_si256(o0, 1);
        __m128i o0_128 = _mm_or_si128(o0_lo, _mm_slli_si128(o0_hi, 8));

        // From e1/o1: same pattern
        __m128i e1_lo = _mm256_castsi256_si128(e1);
        __m128i e1_hi = _mm256_extracti128_si256(e1, 1);
        __m128i e1_128 = _mm_or_si128(e1_lo, _mm_slli_si128(e1_hi, 8));

        __m128i o1_lo = _mm256_castsi256_si128(o1);
        __m128i o1_hi = _mm256_extracti128_si256(o1, 1);
        __m128i o1_128 = _mm_or_si128(o1_lo, _mm_slli_si128(o1_hi, 8));

        // Reassemble two 128b halves into one 256b: [v0-combined | v1-combined]
        __m256i evens = _mm256_inserti128_si256(_mm256_castsi128_si256(e0_128), e1_128, 1);
        __m256i odds  = _mm256_inserti128_si256(_mm256_castsi128_si256(o0_128), o1_128, 1);

        // --- branchless ASCII→nibble ---
        __m256i e_low = _mm256_or_si256(evens, lower_mask);
        __m256i o_low = _mm256_or_si256(odds,  lower_mask);

        __m256i e_is_letter = _mm256_and_si256(
            _mm256_cmpgt_epi8(e_low, ch_a_m1), _mm256_cmpgt_epi8(ch_f_p1, e_low));
        __m256i o_is_letter = _mm256_and_si256(
            _mm256_cmpgt_epi8(o_low, ch_a_m1), _mm256_cmpgt_epi8(ch_f_p1, o_low));

        evens = _mm256_add_epi8(_mm256_and_si256(evens, low_nib),
                                _mm256_and_si256(e_is_letter, add9));
        odds  = _mm256_add_epi8(_mm256_and_si256(odds,  low_nib),
                                _mm256_and_si256(o_is_letter, add9));

        __m256i high = _mm256_slli_epi16(evens, 4);
        __m256i out  = _mm256_or_si256(high, odds);

        _mm256_storeu_si256((__m256i*)(result + i/2), out);
    }

    // Scalar tail (remaining < 64 chars)
    for (; i + 1 < TESTDATALEN; i += 2) {
        result[i/2] = (unsigned char)((toNib(testdata[i]) << 4) | toNib(testdata[i+1]));
    }
}

// try AVX2 again

void superScalarAVX2(void)
{
    const __m256i ascii0 = _mm256_set1_epi8('0');
    const __m256i adj_uc = _mm256_set1_epi8(7);   // 'A'..'F'
    const __m256i adj_lc = _mm256_set1_epi8(39);  // 'a'..'f'

    const __m256i A_m1 = _mm256_set1_epi8('A' - 1);
    const __m256i F_p1 = _mm256_set1_epi8('F' + 1);
    const __m256i a_m1 = _mm256_set1_epi8('a' - 1);
    const __m256i f_p1 = _mm256_set1_epi8('f' + 1);

    const __m256i idxEven = _mm256_setr_epi8(
         0,  2,  4,  6,  8, 10, 12, 14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
         0,  2,  4,  6,  8, 10, 12, 14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
    );
    const __m256i idxOdd  = _mm256_setr_epi8(
         1,  3,  5,  7,  9, 11, 13, 15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
         1,  3,  5,  7,  9, 11, 13, 15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
    );

    size_t i = 0;
    for (; i + 64 <= TESTDATALEN; i += 64) {
        __m256i v0 = _mm256_loadu_si256((const __m256i *)(testdata + i));
        __m256i v1 = _mm256_loadu_si256((const __m256i *)(testdata + i + 32));

        // Shuffle evens/odds per lane
        __m256i e0 = _mm256_shuffle_epi8(v0, idxEven);
        __m256i o0 = _mm256_shuffle_epi8(v0, idxOdd);
        __m256i e1 = _mm256_shuffle_epi8(v1, idxEven);
        __m256i o1 = _mm256_shuffle_epi8(v1, idxOdd);

        // --- FIX: assemble within each load (v0, v1) ---
        __m128i e0_lo = _mm256_castsi256_si128(e0);
        __m128i e0_hi = _mm256_extracti128_si256(e0, 1);
        __m128i e0_128 = _mm_or_si128(e0_lo, _mm_slli_si128(e0_hi, 8));

        __m128i o0_lo = _mm256_castsi256_si128(o0);
        __m128i o0_hi = _mm256_extracti128_si256(o0, 1);
        __m128i o0_128 = _mm_or_si128(o0_lo, _mm_slli_si128(o0_hi, 8));

        __m128i e1_lo = _mm256_castsi256_si128(e1);
        __m128i e1_hi = _mm256_extracti128_si256(e1, 1);
        __m128i e1_128 = _mm_or_si128(e1_lo, _mm_slli_si128(e1_hi, 8));

        __m128i o1_lo = _mm256_castsi256_si128(o1);
        __m128i o1_hi = _mm256_extracti128_si256(o1, 1);
        __m128i o1_128 = _mm_or_si128(o1_lo, _mm_slli_si128(o1_hi, 8));

        __m256i evens = _mm256_inserti128_si256(_mm256_castsi128_si256(e0_128), e1_128, 1);
        __m256i odds  = _mm256_inserti128_si256(_mm256_castsi128_si256(o0_128), o1_128, 1);
        // --- END FIX ---

        // Keep ASCII copies for case detection
        __m256i chars_e = evens;
        __m256i chars_o = odds;

        // ASCII → [0..]
        evens = _mm256_sub_epi8(evens, ascii0);
        odds  = _mm256_sub_epi8(odds,  ascii0);

        // Masks
        __m256i u_e = _mm256_and_si256(_mm256_cmpgt_epi8(chars_e, A_m1), _mm256_cmpgt_epi8(F_p1, chars_e));
        __m256i u_o = _mm256_and_si256(_mm256_cmpgt_epi8(chars_o, A_m1), _mm256_cmpgt_epi8(F_p1, chars_o));
        __m256i l_e = _mm256_and_si256(_mm256_cmpgt_epi8(chars_e, a_m1), _mm256_cmpgt_epi8(f_p1, chars_e));
        __m256i l_o = _mm256_and_si256(_mm256_cmpgt_epi8(chars_o, a_m1), _mm256_cmpgt_epi8(f_p1, chars_o));

        // Apply adjustments (subtract)
        evens = _mm256_sub_epi8(evens, _mm256_and_si256(u_e, adj_uc));
        odds  = _mm256_sub_epi8(odds,  _mm256_and_si256(u_o, adj_uc));
        evens = _mm256_sub_epi8(evens, _mm256_and_si256(l_e, adj_lc));
        odds  = _mm256_sub_epi8(odds,  _mm256_and_si256(l_o, adj_lc));

        // (high<<4) | low
        __m256i high = _mm256_slli_epi16(evens, 4);
        __m256i out  = _mm256_or_si256(high, odds);

        _mm256_storeu_si256((__m256i *)(result + i/2), out);
    }

    // tail (your scalar path is fine)
    for (; i + 1 < TESTDATALEN; i += 2) {
        unsigned char hi = testdata[i], lo = testdata[i + 1];
        hi = (hi >= '0' && hi <= '9') ? hi - '0' :
             (hi >= 'A' && hi <= 'F') ? hi - 'A' + 10 :
             (hi >= 'a' && hi <= 'f') ? hi - 'a' + 10 : 0;
        lo = (lo >= '0' && lo <= '9') ? lo - '0' :
             (lo >= 'A' && lo <= 'F') ? lo - 'A' + 10 :
             (lo >= 'a' && lo <= 'f') ? lo - 'a' + 10 : 0;
        result[i/2] = (hi << 4) | lo;
    }
}

// this SIMD code is much faster than the  lookup table code (test4)
// this is 10x faster than the hex lookup table

void superScalarSSE2(void)
{
    memset(result, 0, sizeof(result));
    //strcpy((char *)result, "\0");
	 //unsigned char *result_sse2 = malloc(TESTDATALEN/2);

    const __m128i ascii0 = _mm_set1_epi8('0');
    const __m128i adj_uc = _mm_set1_epi8(7);   // 'A'..'F'  => -7 after '0' subtract
    const __m128i adj_lc = _mm_set1_epi8(39);  // 'a'..'f'  => -39 after '0' subtract

    const __m128i A_m1 = _mm_set1_epi8('A' - 1);
    const __m128i F_p1 = _mm_set1_epi8('F' + 1);
    const __m128i a_m1 = _mm_set1_epi8('a' - 1);
    const __m128i f_p1 = _mm_set1_epi8('f' + 1);

    size_t i;
    for (i = 0; i + 32 <= TESTDATALEN; i += 32) {
        const unsigned char* src = testdata + i;

        __m128i chunk1 = _mm_loadu_si128((const __m128i*)(src));
        __m128i chunk2 = _mm_loadu_si128((const __m128i*)(src + 16));

        __m128i lo1 = _mm_unpacklo_epi8(chunk1, _mm_setzero_si128());
        __m128i hi1 = _mm_unpackhi_epi8(chunk1, _mm_setzero_si128());
        __m128i lo2 = _mm_unpacklo_epi8(chunk2, _mm_setzero_si128());
        __m128i hi2 = _mm_unpackhi_epi8(chunk2, _mm_setzero_si128());

        // build evens (b0,b2,...,b30)
        __m128i even_bytes = _mm_setzero_si128();
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 0), 0);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 2), 1);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 4), 2);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 6), 3);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 0), 4);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 2), 5);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 4), 6);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 6), 7);

        __m128i even_bytes2 = _mm_setzero_si128();
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 0), 0);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 2), 1);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 4), 2);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 6), 3);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 0), 4);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 2), 5);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 4), 6);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 6), 7);

		  // Combine evens from both halves
		  // this line by ChatGPT
		  __m128i evens = _mm_packus_epi16(even_bytes, even_bytes2);

        // build odds (b1,b3,...,b31)
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


		  // Combine odds from both halves
		  __m128i odds = _mm_packus_epi16(odd_bytes, odd_bytes2);

        // --- FIXED DECODE STAGE ---
        // Keep ASCII copies for case detection
        __m128i chars_e = evens;
        __m128i chars_o = odds;

        // Convert ASCII → [0..] by subtracting '0'
        evens = _mm_sub_epi8(evens, ascii0);
        odds  = _mm_sub_epi8(odds,  ascii0);

        // Uppercase mask: 'A' <= char <= 'F'
        __m128i u_e = _mm_and_si128(_mm_cmpgt_epi8(chars_e, A_m1), _mm_cmplt_epi8(chars_e, F_p1));
        __m128i u_o = _mm_and_si128(_mm_cmpgt_epi8(chars_o, A_m1), _mm_cmplt_epi8(chars_o, F_p1));

        // Lowercase mask: 'a' <= char <= 'f'
        __m128i l_e = _mm_and_si128(_mm_cmpgt_epi8(chars_e, a_m1), _mm_cmplt_epi8(chars_e, f_p1));
        __m128i l_o = _mm_and_si128(_mm_cmpgt_epi8(chars_o, a_m1), _mm_cmplt_epi8(chars_o, f_p1));

        // Apply case-specific adjustments **by subtraction**
        evens = _mm_sub_epi8(evens, _mm_and_si128(u_e, adj_uc));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(u_o, adj_uc));

        evens = _mm_sub_epi8(evens, _mm_and_si128(l_e, adj_lc));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(l_o, adj_lc));
        // --- END FIX ---

        // Pack nibbles: (high<<4) | low
        __m128i high_nibbles = _mm_slli_epi16(evens, 4);
        __m128i bytes = _mm_or_si128(high_nibbles, odds);

        _mm_storeu_si128((__m128i *)(result + i / 2), bytes);
    }

    // Scalar tail
    for (; i + 1 < TESTDATALEN; i += 2) {
        unsigned char high = testdata[i];
        unsigned char low  = testdata[i + 1];

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
	//strcpy((char *)result, "\0");
    memset(result, 0, sizeof(result));

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

// --- SIMD2 Function ---
void simd2_hex_to_bin(void) {
	 
    memset(result, 0, sizeof(result));
	//strcpy((char *)result, "\0");
    size_t i;
    for (i = 0; i + 32 <= TESTDATALEN; i += 32) {
        __m128i block1 = _mm_loadu_si128((const __m128i *)(testdata + i));
        __m128i block2 = _mm_loadu_si128((const __m128i *)(testdata + i + 16));

        __m128i idxEven = _mm_setr_epi8(0, 2, 4, 6, 8,10,12,14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80);
        __m128i idxOdd  = _mm_setr_epi8(1, 3, 5, 7, 9,11,13,15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80);

        __m128i evens = _mm_or_si128(_mm_shuffle_epi8(block1, idxEven), _mm_slli_si128(_mm_shuffle_epi8(block2, idxEven), 8));
        __m128i odds  = _mm_or_si128(_mm_shuffle_epi8(block1, idxOdd),  _mm_slli_si128(_mm_shuffle_epi8(block2, idxOdd),  8));

        __m128i zero = _mm_set1_epi8('0');
        evens = _mm_sub_epi8(evens, zero);
        odds  = _mm_sub_epi8(odds,  zero);

        __m128i chars_evens = _mm_add_epi8(evens, zero);
        __m128i chars_odds  = _mm_add_epi8(odds, zero);

        __m128i upperA = _mm_set1_epi8('A'-1), upperF = _mm_set1_epi8('F'+1);
        __m128i lowerA = _mm_set1_epi8('a'-1), lowerF = _mm_set1_epi8('f'+1);

        __m128i ucase_mask_e = _mm_and_si128(_mm_cmpgt_epi8(chars_evens, upperA), _mm_cmplt_epi8(chars_evens, upperF));
        __m128i lcase_mask_e = _mm_and_si128(_mm_cmpgt_epi8(chars_evens, lowerA), _mm_cmplt_epi8(chars_evens, lowerF));
        __m128i ucase_mask_o = _mm_and_si128(_mm_cmpgt_epi8(chars_odds,  upperA), _mm_cmplt_epi8(chars_odds,  upperF));
        __m128i lcase_mask_o = _mm_and_si128(_mm_cmpgt_epi8(chars_odds,  lowerA), _mm_cmplt_epi8(chars_odds,  lowerF));

        evens = _mm_sub_epi8(evens, _mm_and_si128(ucase_mask_e, _mm_set1_epi8(7)));
        evens = _mm_sub_epi8(evens, _mm_and_si128(lcase_mask_e, _mm_set1_epi8(39)));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(ucase_mask_o, _mm_set1_epi8(7)));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(lcase_mask_o, _mm_set1_epi8(39)));

        __m128i high_shifted = _mm_slli_epi16(evens, 4);
        __m128i bytes = _mm_or_si128(high_shifted, odds);

        _mm_storeu_si128((__m128i *)(result + i / 2), bytes);
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
    memset(result, 0, sizeof(result));
	 //strcpy(result, "\0");
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
    memset(result, 0, sizeof(result));
	 //strcpy(result, "\0");
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
    memset(result, 0, sizeof(result));
	 //strcpy(result, "\0");
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
    memset(result, 0, sizeof(result));
	 //strcpy(result, "\0");
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
	 //strcpy(result, "\0");
    memset(result, 0, sizeof(result));
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




int main() {
    struct timespec before, after;
    unsigned long long checksum;
    int i,n;
    double elapsed;

	 printf("TESTDATALEN: %d\n", TESTDATALEN);
	 writeHex("testdata.hex");

	 // basic
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        calcHex();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("calcHex.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("arithmetic solution calcHex() took %3.6f for %d tests, seconds avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 // new SIMD2 
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        simd2_hex_to_bin();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("simd2_hex_to_bin.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup simd2_hex_to_bin() took %3.6f seconds for %d tests, avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 //-- SuperScalar SSE2
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        superScalarSSE2();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("superScalerSSE2.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup superScalarSSE2() took %3.6f for %d tests, seconds avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 //-- SuperScalar SSSE3
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        superScalarSSSE3();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("superScalerSSSE3.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup superScalarSSSE3() took %3.6f seconds for %d tests, avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 //-- SuperScalar AVX2
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        superScalarAVX2();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("superScalarAVX2.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup superScalarAVX2() took %3.6f seconds for %d tests, avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 //-- SuperScalar AVX2 Optimized
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        superScalarAVX2_opt();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("superScalarAVX2_opt.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup superScalarAVX2_opt() took %3.6f seconds for %d tests, avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 // -----

	 // lookupBasic
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        lookupBasic();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("lookupBasic.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("optimized lookup lookupBasic() took %3.6f seconds for %d tests,  avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 // -----
    // lookup256
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        lookup256();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("lookup256.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("256 entries table lookup256() took %3.6f seconds for %d tests, avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 // -----
	 // lookup32k
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        lookup32k();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("lookup32k.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("32768 entries table lookup32k() took %3.6f seconds for %d tests, avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);

	 // -----
    // lookup64k
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (i = 0; i < NUMTESTS; i++) {
        lookup64k();
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
	 n = i;
	 writeResults("lookup64k.dat");

    checksum = 0;
    for (i = 0; i < TESTDATALEN/2; i++) {
        checksum += result[i];
    }
    printf("\nchecksum: %llu\n", checksum);
    elapsed = difftime(after.tv_sec, before.tv_sec) + (after.tv_nsec - before.tv_nsec)/1.0e9;
    printf("65536 entries table lookup64k() took %3.6f seconds for %d tests, avg: %2.9f\n", elapsed, n, elapsed/NUMTESTS);
	 // -----

    return 0;
}
