#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <immintrin.h>  // AVX2, SSE

#define DATA_SIZE (1024 * 1024 * 10)  // 10MB hex = 5MB binary

void fill_hex_data(char *hex_data, size_t size) {
    const char hex_chars[] = "0123456789ABCDEF";
    for (size_t i = 0; i < size; ++i)
        hex_data[i] = hex_chars[rand() % 16];
}

double current_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// --- SIMD2 Function ---
void simd2_hex_to_bin(const char *hex_char_data, unsigned char *binary_char_data, size_t n) {
    size_t i;
    for (i = 0; i + 32 <= n; i += 32) {
        __m128i block1 = _mm_loadu_si128((const __m128i *)(hex_char_data + i));
        __m128i block2 = _mm_loadu_si128((const __m128i *)(hex_char_data + i + 16));

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

        _mm_storeu_si128((__m128i *)(binary_char_data + i / 2), bytes);
    }
}

// --- AVX2 Function ---
void simd_hex_to_bin_avx2(const char *hex_char_data, unsigned char *binary_char_data, size_t n) {
    for (size_t i = 0; i + 32 <= n; i += 32) {
        __m256i chars = _mm256_loadu_si256((const __m256i *)(hex_char_data + i));
        __m256i idx_even = _mm256_setr_epi8(0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1);
        __m256i idx_odd  = _mm256_setr_epi8(1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1);

        __m256i evens = _mm256_shuffle_epi8(chars, idx_even);
        __m256i odds  = _mm256_shuffle_epi8(chars, idx_odd);

        __m256i zero = _mm256_set1_epi8('0');
        __m256i evens_sub = _mm256_sub_epi8(evens, zero);
        __m256i odds_sub  = _mm256_sub_epi8(odds,  zero);

        __m256i evens_ascii = _mm256_add_epi8(evens_sub, zero);
        __m256i odds_ascii  = _mm256_add_epi8(odds_sub,  zero);

        __m256i upperA = _mm256_set1_epi8('A' - 1);
        __m256i upperF = _mm256_set1_epi8('F' + 1);
        __m256i lowerA = _mm256_set1_epi8('a' - 1);
        __m256i lowerF = _mm256_set1_epi8('f' + 1);

        //__m256i ucase_mask_e = _mm256_and_si256(_mm256_cmpgt_epi8(evens_ascii, upperA), _mm256_cmplt_epi8(evens_ascii, upperF));
        //__m256i lcase_mask_e = _mm256_and_si256(_mm256_cmpgt_epi8(evens_ascii, lowerA), _mm256_cmplt_epi8(evens_ascii, lowerF));
        //__m256i ucase_mask_o = _mm256_and_si256(_mm256_cmpgt_epi8(odds_ascii, upperA), _mm256_cmplt_epi8(odds_ascii, upperF));
        //__m256i lcase_mask_o = _mm256_and_si256(_mm256_cmpgt_epi8(odds_ascii, lowerA), _mm256_cmplt_epi8(odds_ascii, lowerF));
		  
__m256i ucase_mask_e = _mm256_and_si256(
    _mm256_cmpgt_epi8(evens_ascii, upperA),
    _mm256_cmpgt_epi8(upperF, evens_ascii));

__m256i lcase_mask_e = _mm256_and_si256(
    _mm256_cmpgt_epi8(evens_ascii, lowerA),
    _mm256_cmpgt_epi8(lowerF, evens_ascii));

__m256i ucase_mask_o = _mm256_and_si256(
    _mm256_cmpgt_epi8(odds_ascii, upperA),
    _mm256_cmpgt_epi8(upperF, odds_ascii));

__m256i lcase_mask_o = _mm256_and_si256(
    _mm256_cmpgt_epi8(odds_ascii, lowerA),
    _mm256_cmpgt_epi8(lowerF, odds_ascii));

        evens_sub = _mm256_sub_epi8(evens_sub, _mm256_or_si256(_mm256_and_si256(ucase_mask_e, _mm256_set1_epi8(7)), _mm256_and_si256(lcase_mask_e, _mm256_set1_epi8(39))));
        odds_sub  = _mm256_sub_epi8(odds_sub,  _mm256_or_si256(_mm256_and_si256(ucase_mask_o, _mm256_set1_epi8(7)), _mm256_and_si256(lcase_mask_o, _mm256_set1_epi8(39))));

        __m256i high_shifted = _mm256_slli_epi16(evens_sub, 4);
        __m256i combined = _mm256_or_si256(high_shifted, odds_sub);

        __m128i result = _mm256_castsi256_si128(_mm256_permute4x64_epi64(combined, 0xD8));
        _mm_storeu_si128((__m128i *)(binary_char_data + i / 2), result);
    }
}

// --- Main Benchmark ---
int main() {
    char *hex_data = malloc(DATA_SIZE);
    unsigned char *bin_data = malloc(DATA_SIZE / 2);
    fill_hex_data(hex_data, DATA_SIZE);

    double start = current_time();
    for (int i = 0; i < 100; ++i)
        simd_hex_to_bin_avx2(hex_data, bin_data, DATA_SIZE);
    double end = current_time();
    printf("AVX2 Time: %.6f seconds\n", end - start);

    start = current_time();
    for (int i = 0; i < 100; ++i)
        simd2_hex_to_bin(hex_data, bin_data, DATA_SIZE);
    end = current_time();
    printf("SIMD2 Time: %.6f seconds\n", end - start);

    free(hex_data);
    free(bin_data);
    return 0;
}

