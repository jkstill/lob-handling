// hex-to-bin-to-hex-tests.c
// Add bin->hex for each hex->bin method: scalar, SSE2, SSSE3, AVX2, AVX2_opt
// Uppercase hex output (0-9 A-F)

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <immintrin.h>    // AVX2 / SSSE3 / SSE2 intrinsics

// ---------------------- Config / Test Data ----------------------

#ifndef TESTDATALEN
#define TESTDATALEN 66
#endif

static unsigned char testdata[TESTDATALEN + 1] =
    "0123456789ABCDEFabcdef0123456789ABCDEFabcdef0123456789ABCDEFabcdef";

// Buffers for hex->bin (n/2) and bin->hex (n)
static unsigned char out_bin_scalar [TESTDATALEN/2];
static unsigned char out_bin_sse2   [TESTDATALEN/2];
static unsigned char out_bin_ssse3  [TESTDATALEN/2];
static unsigned char out_bin_avx2   [TESTDATALEN/2];
static unsigned char out_bin_avx2op [TESTDATALEN/2];

static unsigned char out_hex_scalar [TESTDATALEN];
static unsigned char out_hex_sse2   [TESTDATALEN];
static unsigned char out_hex_ssse3  [TESTDATALEN];
static unsigned char out_hex_avx2   [TESTDATALEN];
static unsigned char out_hex_avx2op [TESTDATALEN];
unsigned char ref_upper[TESTDATALEN];

// ---------------------- Helpers ----------------------

static inline unsigned char toNib_branchless(unsigned char c) {
    // Branchless ASCII hex -> nibble (0..15). Accepts '0'..'9','A'..'F','a'..'f'.
    unsigned char lower = (unsigned char)(c | 0x20);
    unsigned char isLet = (unsigned)((unsigned)lower - 'a') <= ('f' - 'a');
    return (unsigned char)((c & 0x0Fu) + (isLet ? 9u : 0u));
}

static inline unsigned char nib_to_hex_upper(unsigned char v) {
    // 0..15 -> '0'..'9','A'..'F' (uppercase)
    unsigned char ch = (unsigned char)('0' + v);
    if (v > 9) ch = (unsigned char)(ch + 7);  // '0'+10 + 7 == 'A'
    return ch;
}

static void to_upper_hex_inplace(unsigned char *s, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = s[i];
        if ((unsigned)(c - 'a') <= ('f' - 'a')) s[i] = (unsigned char)(c - 32);
    }
}


// ---------------------- Scalar ----------------------

static void scalar_to_bin(const unsigned char *hex, unsigned char *bin, size_t n) {
    // n must be even
    for (size_t i = 0; i < n; i += 2) {
        unsigned char hi = toNib_branchless(hex[i]);
        unsigned char lo = toNib_branchless(hex[i+1]);
        bin[i/2] = (unsigned char)((hi << 4) | lo);
    }
}

static void scalar_to_hex(const unsigned char *bin, unsigned char *hex, size_t nbytes) {
    for (size_t i = 0; i < nbytes; ++i) {
        unsigned char b = bin[i];
        hex[2*i+0] = nib_to_hex_upper((unsigned char)(b >> 4));
        hex[2*i+1] = nib_to_hex_upper((unsigned char)(b & 0x0F));
    }
}

// ---------------------- SSE2 ----------------------
#ifdef __SSE2__
#include <emmintrin.h>

static void sse2_to_bin(const unsigned char *hex, unsigned char *bin, size_t n) {
    const __m128i ascii0 = _mm_set1_epi8('0');
    const __m128i adj_uc = _mm_set1_epi8(7);    // 'A'..'F'
    const __m128i adj_lc = _mm_set1_epi8(39);   // 'a'..'f'
    const __m128i A_m1 = _mm_set1_epi8('A' - 1);
    const __m128i F_p1 = _mm_set1_epi8('F' + 1);
    const __m128i a_m1 = _mm_set1_epi8('a' - 1);
    const __m128i f_p1 = _mm_set1_epi8('f' + 1);

    size_t i = 0;
    for (; i + 32 <= n; i += 32) {
        __m128i b1 = _mm_loadu_si128((const __m128i*)(hex + i));
        __m128i b2 = _mm_loadu_si128((const __m128i*)(hex + i + 16));

        // Unpack to words
        __m128i z = _mm_setzero_si128();
        __m128i b1_lo = _mm_unpacklo_epi8(b1, z), b1_hi = _mm_unpackhi_epi8(b1, z);
        __m128i b2_lo = _mm_unpacklo_epi8(b2, z), b2_hi = _mm_unpackhi_epi8(b2, z);

        // Gather evens (0,2,4,..,30) from b1, then b2
        __m128i e1 = _mm_setzero_si128();
        e1 = _mm_insert_epi16(e1, _mm_extract_epi16(b1_lo, 0), 0);
        e1 = _mm_insert_epi16(e1, _mm_extract_epi16(b1_lo, 2), 1);
        e1 = _mm_insert_epi16(e1, _mm_extract_epi16(b1_lo, 4), 2);
        e1 = _mm_insert_epi16(e1, _mm_extract_epi16(b1_lo, 6), 3);
        e1 = _mm_insert_epi16(e1, _mm_extract_epi16(b1_hi, 0), 4);
        e1 = _mm_insert_epi16(e1, _mm_extract_epi16(b1_hi, 2), 5);
        e1 = _mm_insert_epi16(e1, _mm_extract_epi16(b1_hi, 4), 6);
        e1 = _mm_insert_epi16(e1, _mm_extract_epi16(b1_hi, 6), 7);

        __m128i e2 = _mm_setzero_si128();
        e2 = _mm_insert_epi16(e2, _mm_extract_epi16(b2_lo, 0), 0);
        e2 = _mm_insert_epi16(e2, _mm_extract_epi16(b2_lo, 2), 1);
        e2 = _mm_insert_epi16(e2, _mm_extract_epi16(b2_lo, 4), 2);
        e2 = _mm_insert_epi16(e2, _mm_extract_epi16(b2_lo, 6), 3);
        e2 = _mm_insert_epi16(e2, _mm_extract_epi16(b2_hi, 0), 4);
        e2 = _mm_insert_epi16(e2, _mm_extract_epi16(b2_hi, 2), 5);
        e2 = _mm_insert_epi16(e2, _mm_extract_epi16(b2_hi, 4), 6);
        e2 = _mm_insert_epi16(e2, _mm_extract_epi16(b2_hi, 6), 7);

        __m128i evens = _mm_packus_epi16(e1, e2);

        // Gather odds (1,3,5,..,31) from b1, then b2
        __m128i o1 = _mm_setzero_si128();
        o1 = _mm_insert_epi16(o1, _mm_extract_epi16(b1_lo, 1), 0);
        o1 = _mm_insert_epi16(o1, _mm_extract_epi16(b1_lo, 3), 1);
        o1 = _mm_insert_epi16(o1, _mm_extract_epi16(b1_lo, 5), 2);
        o1 = _mm_insert_epi16(o1, _mm_extract_epi16(b1_lo, 7), 3);
        o1 = _mm_insert_epi16(o1, _mm_extract_epi16(b1_hi, 1), 4);
        o1 = _mm_insert_epi16(o1, _mm_extract_epi16(b1_hi, 3), 5);
        o1 = _mm_insert_epi16(o1, _mm_extract_epi16(b1_hi, 5), 6);
        o1 = _mm_insert_epi16(o1, _mm_extract_epi16(b1_hi, 7), 7);

        __m128i o2 = _mm_setzero_si128();
        o2 = _mm_insert_epi16(o2, _mm_extract_epi16(b2_lo, 1), 0);
        o2 = _mm_insert_epi16(o2, _mm_extract_epi16(b2_lo, 3), 1);
        o2 = _mm_insert_epi16(o2, _mm_extract_epi16(b2_lo, 5), 2);
        o2 = _mm_insert_epi16(o2, _mm_extract_epi16(b2_lo, 7), 3);
        o2 = _mm_insert_epi16(o2, _mm_extract_epi16(b2_hi, 1), 4);
        o2 = _mm_insert_epi16(o2, _mm_extract_epi16(b2_hi, 3), 5);
        o2 = _mm_insert_epi16(o2, _mm_extract_epi16(b2_hi, 5), 6);
        o2 = _mm_insert_epi16(o2, _mm_extract_epi16(b2_hi, 7), 7);

        __m128i odds = _mm_packus_epi16(o1, o2);

        // Decode
        __m128i ce = evens, co = odds;
        evens = _mm_sub_epi8(evens, ascii0);
        odds  = _mm_sub_epi8(odds,  ascii0);

        __m128i u_e = _mm_and_si128(_mm_cmpgt_epi8(ce, A_m1), _mm_cmplt_epi8(ce, F_p1));
        __m128i u_o = _mm_and_si128(_mm_cmpgt_epi8(co, A_m1), _mm_cmplt_epi8(co, F_p1));
        __m128i l_e = _mm_and_si128(_mm_cmpgt_epi8(ce, a_m1), _mm_cmplt_epi8(ce, f_p1));
        __m128i l_o = _mm_and_si128(_mm_cmpgt_epi8(co, a_m1), _mm_cmplt_epi8(co, f_p1));

        evens = _mm_sub_epi8(evens, _mm_and_si128(u_e, adj_uc));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(u_o, adj_uc));
        evens = _mm_sub_epi8(evens, _mm_and_si128(l_e, adj_lc));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(l_o, adj_lc));

        __m128i hi = _mm_slli_epi16(evens, 4);
        __m128i out = _mm_or_si128(hi, odds);
        _mm_storeu_si128((__m128i*)(bin + i/2), out);
    }

    // tail
    for (; i + 1 < n; i += 2) {
        bin[i/2] = (unsigned char)((toNib_branchless(hex[i]) << 4) | toNib_branchless(hex[i+1]));
    }
}

static void sse2_to_hex(const unsigned char *bin, unsigned char *hex, size_t nbytes) {
    const __m128i nine = _mm_set1_epi8(9);
    const __m128i zero = _mm_set1_epi8('0');
    const __m128i add7 = _mm_set1_epi8(7); // for letters

    size_t i = 0;
    for (; i + 16 <= nbytes; i += 16) {
        __m128i v = _mm_loadu_si128((const __m128i*)(bin + i));

        // Extract high/low nibbles into separate vectors (as bytes)
        __m128i hi = _mm_and_si128(_mm_srli_epi16(v, 4), _mm_set1_epi8(0x0F));
        __m128i lo = _mm_and_si128(v, _mm_set1_epi8(0x0F));

        // Map 0..15 -> '0'..'9','A'..'F': add '0', then +7 where >9
        __m128i hi_ch = _mm_add_epi8(hi, zero);
        __m128i lo_ch = _mm_add_epi8(lo, zero);

        __m128i hi_gt9 = _mm_cmpgt_epi8(hi, nine);
        __m128i lo_gt9 = _mm_cmpgt_epi8(lo, nine);

        hi_ch = _mm_add_epi8(hi_ch, _mm_and_si128(hi_gt9, add7));
        lo_ch = _mm_add_epi8(lo_ch, _mm_and_si128(lo_gt9, add7));

        // Interleave into hex string: H0,L0,H1,L1,...
        __m128i lo16 = _mm_unpacklo_epi8(hi_ch, lo_ch);
        __m128i hi16 = _mm_unpackhi_epi8(hi_ch, lo_ch);

        _mm_storeu_si128((__m128i*)(hex + 2*i + 0), lo16);
        _mm_storeu_si128((__m128i*)(hex + 2*i + 16), hi16);
    }

    for (; i < nbytes; ++i) {
        unsigned char b = bin[i];
        hex[2*i+0] = nib_to_hex_upper((unsigned char)(b >> 4));
        hex[2*i+1] = nib_to_hex_upper((unsigned char)(b & 0x0F));
    }
}
#endif // __SSE2__

// ---------------------- SSSE3 ----------------------
#ifdef __SSSE3__
#include <tmmintrin.h>

static void ssse3_to_bin(const unsigned char *hex, unsigned char *bin, size_t n) {
    const __m128i idxEven = _mm_setr_epi8( 0,2,4,6,8,10,12,14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80 );
    const __m128i idxOdd  = _mm_setr_epi8( 1,3,5,7,9,11,13,15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80 );
    const __m128i ascii0  = _mm_set1_epi8('0');
    const __m128i A_m1 = _mm_set1_epi8('A'-1), F_p1 = _mm_set1_epi8('F'+1);
    const __m128i a_m1 = _mm_set1_epi8('a'-1), f_p1 = _mm_set1_epi8('f'+1);
    const __m128i adj_uc = _mm_set1_epi8(7), adj_lc = _mm_set1_epi8(39);

    size_t i = 0;
    for (; i + 32 <= n; i += 32) {
        __m128i b1 = _mm_loadu_si128((const __m128i*)(hex + i));
        __m128i b2 = _mm_loadu_si128((const __m128i*)(hex + i + 16));

        __m128i e1 = _mm_shuffle_epi8(b1, idxEven);
        __m128i o1 = _mm_shuffle_epi8(b1, idxOdd);
        __m128i e2 = _mm_shuffle_epi8(b2, idxEven);
        __m128i o2 = _mm_shuffle_epi8(b2, idxOdd);

        __m128i evens = _mm_or_si128(e1, _mm_slli_si128(e2, 8));
        __m128i odds  = _mm_or_si128(o1, _mm_slli_si128(o2, 8));

        __m128i ce = evens, co = odds;
        evens = _mm_sub_epi8(evens, ascii0);
        odds  = _mm_sub_epi8(odds,  ascii0);

        __m128i u_e = _mm_and_si128(_mm_cmpgt_epi8(ce, A_m1), _mm_cmplt_epi8(ce, F_p1));
        __m128i u_o = _mm_and_si128(_mm_cmpgt_epi8(co, A_m1), _mm_cmplt_epi8(co, F_p1));
        __m128i l_e = _mm_and_si128(_mm_cmpgt_epi8(ce, a_m1), _mm_cmplt_epi8(ce, f_p1));
        __m128i l_o = _mm_and_si128(_mm_cmpgt_epi8(co, a_m1), _mm_cmplt_epi8(co, f_p1));

        evens = _mm_sub_epi8(evens, _mm_and_si128(u_e, adj_uc));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(u_o, adj_uc));
        evens = _mm_sub_epi8(evens, _mm_and_si128(l_e, adj_lc));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(l_o, adj_lc));

        __m128i out = _mm_or_si128(_mm_slli_epi16(evens, 4), odds);
        _mm_storeu_si128((__m128i*)(bin + i/2), out);
    }

    for (; i + 1 < n; i += 2) {
        bin[i/2] = (unsigned char)((toNib_branchless(hex[i]) << 4) | toNib_branchless(hex[i+1]));
    }
}

static void ssse3_to_hex(const unsigned char *bin, unsigned char *hex, size_t nbytes) {
    // Build a 16-byte table "0123456789ABCDEF"
    const __m128i lut = _mm_setr_epi8('0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F');
    size_t i = 0;
    for (; i + 16 <= nbytes; i += 16) {
        __m128i v  = _mm_loadu_si128((const __m128i*)(bin + i));
        __m128i hi = _mm_and_si128(_mm_srli_epi16(v, 4), _mm_set1_epi8(0x0F));
        __m128i lo = _mm_and_si128(v, _mm_set1_epi8(0x0F));

        __m128i hi_ch = _mm_shuffle_epi8(lut, hi);
        __m128i lo_ch = _mm_shuffle_epi8(lut, lo);

        __m128i lo16 = _mm_unpacklo_epi8(hi_ch, lo_ch);
        __m128i hi16 = _mm_unpackhi_epi8(hi_ch, lo_ch);

        _mm_storeu_si128((__m128i*)(hex + 2*i + 0),  lo16);
        _mm_storeu_si128((__m128i*)(hex + 2*i + 16), hi16);
    }

    for (; i < nbytes; ++i) {
        unsigned char b = bin[i];
        hex[2*i+0] = nib_to_hex_upper((unsigned char)(b >> 4));
        hex[2*i+1] = nib_to_hex_upper((unsigned char)(b & 0x0F));
    }
}
#endif // __SSSE3__

// ---------------------- AVX2 (fixed merge) ----------------------
#ifdef __AVX2__

static void avx2_to_bin(const unsigned char *hex, unsigned char *bin, size_t n) {
    const __m256i ascii0 = _mm256_set1_epi8('0');
    const __m256i A_m1 = _mm256_set1_epi8('A'-1), F_p1 = _mm256_set1_epi8('F'+1);
    const __m256i a_m1 = _mm256_set1_epi8('a'-1), f_p1 = _mm256_set1_epi8('f'+1);
    const __m256i adj_uc = _mm256_set1_epi8(7), adj_lc = _mm256_set1_epi8(39);

    const __m256i idxEven = _mm256_setr_epi8(
        0,2,4,6,8,10,12,14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
        0,2,4,6,8,10,12,14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
    );
    const __m256i idxOdd  = _mm256_setr_epi8(
        1,3,5,7,9,11,13,15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
        1,3,5,7,9,11,13,15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
    );

    size_t i = 0;
    for (; i + 64 <= n; i += 64) {
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(hex + i));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(hex + i + 32));

        __m256i e0 = _mm256_shuffle_epi8(v0, idxEven);
        __m256i o0 = _mm256_shuffle_epi8(v0, idxOdd);
        __m256i e1 = _mm256_shuffle_epi8(v1, idxEven);
        __m256i o1 = _mm256_shuffle_epi8(v1, idxOdd);

        // FIX: assemble within each load first, then stitch into 256b
        __m128i e0_lo = _mm256_castsi256_si128(e0), e0_hi = _mm256_extracti128_si256(e0, 1);
        __m128i o0_lo = _mm256_castsi256_si128(o0), o0_hi = _mm256_extracti128_si256(o0, 1);
        __m128i e1_lo = _mm256_castsi256_si128(e1), e1_hi = _mm256_extracti128_si256(e1, 1);
        __m128i o1_lo = _mm256_castsi256_si128(o1), o1_hi = _mm256_extracti128_si256(o1, 1);

        __m128i e0_128 = _mm_or_si128(e0_lo, _mm_slli_si128(e0_hi, 8));
        __m128i o0_128 = _mm_or_si128(o0_lo, _mm_slli_si128(o0_hi, 8));
        __m128i e1_128 = _mm_or_si128(e1_lo, _mm_slli_si128(e1_hi, 8));
        __m128i o1_128 = _mm_or_si128(o1_lo, _mm_slli_si128(o1_hi, 8));

        __m256i evens = _mm256_inserti128_si256(_mm256_castsi128_si256(e0_128), e1_128, 1);
        __m256i odds  = _mm256_inserti128_si256(_mm256_castsi128_si256(o0_128), o1_128, 1);

        __m256i ce = evens, co = odds;
        evens = _mm256_sub_epi8(evens, ascii0);
        odds  = _mm256_sub_epi8(odds,  ascii0);

        __m256i u_e = _mm256_and_si256(_mm256_cmpgt_epi8(ce, A_m1), _mm256_cmpgt_epi8(F_p1, ce));
        __m256i u_o = _mm256_and_si256(_mm256_cmpgt_epi8(co, A_m1), _mm256_cmpgt_epi8(F_p1, co));
        __m256i l_e = _mm256_and_si256(_mm256_cmpgt_epi8(ce, a_m1), _mm256_cmpgt_epi8(f_p1, ce));
        __m256i l_o = _mm256_and_si256(_mm256_cmpgt_epi8(co, a_m1), _mm256_cmpgt_epi8(f_p1, co));

        evens = _mm256_sub_epi8(evens, _mm256_and_si256(u_e, _mm256_set1_epi8(7)));
        odds  = _mm256_sub_epi8(odds,  _mm256_and_si256(u_o, _mm256_set1_epi8(7)));
        evens = _mm256_sub_epi8(evens, _mm256_and_si256(l_e, _mm256_set1_epi8(39)));
        odds  = _mm256_sub_epi8(odds,  _mm256_and_si256(l_o, _mm256_set1_epi8(39)));

        __m256i out = _mm256_or_si256(_mm256_slli_epi16(evens, 4), odds);
        _mm256_storeu_si256((__m256i*)(bin + i/2), out);
    }

    for (; i + 1 < n; i += 2) {
        bin[i/2] = (unsigned char)((toNib_branchless(hex[i]) << 4) | toNib_branchless(hex[i+1]));
    }
}

static void avx2_to_hex(const unsigned char *bin, unsigned char *hex, size_t nbytes) {
    const __m256i lut = _mm256_setr_epi8(
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    );
    const __m256i low_f = _mm256_set1_epi8(0x0F);

    size_t i = 0;
    for (; i + 32 <= nbytes; i += 32) {
        __m256i v  = _mm256_loadu_si256((const __m256i*)(bin + i));
        __m256i hi = _mm256_and_si256(_mm256_srli_epi16(v, 4), low_f);
        __m256i lo = _mm256_and_si256(v, low_f);

        __m256i hi_ch = _mm256_shuffle_epi8(lut, hi);
        __m256i lo_ch = _mm256_shuffle_epi8(lut, lo);

        // Per lane: [H0,L0,H1,L1,...,H7,L7] and [H8..L8..H15..L15]
        __m256i lo16 = _mm256_unpacklo_epi8(hi_ch, lo_ch);
        __m256i hi16 = _mm256_unpackhi_epi8(hi_ch, lo_ch);

        // FIX: stitch lane0 of lo16 with lane0 of hi16 (bytes 0..31),
        //      and lane1 of lo16 with lane1 of hi16 (bytes 32..63).
        __m256i out0 = _mm256_permute2x128_si256(lo16, hi16, 0x20); // [lo16.low | hi16.low]
        __m256i out1 = _mm256_permute2x128_si256(lo16, hi16, 0x31); // [lo16.high| hi16.high]

        _mm256_storeu_si256((__m256i*)(hex + 2*i +  0), out0);
        _mm256_storeu_si256((__m256i*)(hex + 2*i + 32), out1);
    }

    for (; i < nbytes; ++i) {
        unsigned char b = bin[i];
        hex[2*i+0] = (unsigned char)("0123456789ABCDEF"[(b >> 4) & 0xF]);
        hex[2*i+1] = (unsigned char)("0123456789ABCDEF"[ b       & 0xF]);
    }
}



#endif // __AVX2__

// ---------------------- AVX2_opt (branchless nibble) ----------------------
#ifdef __AVX2__
static void avx2opt_to_bin(const unsigned char *hex, unsigned char *bin, size_t n) {
    const __m256i lower_mask = _mm256_set1_epi8(0x20);
    const __m256i ch_a_m1    = _mm256_set1_epi8('a' - 1);
    const __m256i ch_f_p1    = _mm256_set1_epi8('f' + 1);
    const __m256i add9       = _mm256_set1_epi8(9);
    const __m256i low_nib    = _mm256_set1_epi8(0x0F);

    const __m256i idxEven = _mm256_setr_epi8(
        0,2,4,6,8,10,12,14,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
        0,2,4,6,8,10,12,14,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80);
    const __m256i idxOdd  = _mm256_setr_epi8(
        1,3,5,7,9,11,13,15,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
        1,3,5,7,9,11,13,15,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80);

    size_t i = 0;
    for (; i + 64 <= n; i += 64) {
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(hex + i));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(hex + i + 32));

        __m256i e0 = _mm256_shuffle_epi8(v0, idxEven);
        __m256i o0 = _mm256_shuffle_epi8(v0, idxOdd);
        __m256i e1 = _mm256_shuffle_epi8(v1, idxEven);
        __m256i o1 = _mm256_shuffle_epi8(v1, idxOdd);

        // Correct lane assembly
        __m128i e0_lo = _mm256_castsi256_si128(e0), e0_hi = _mm256_extracti128_si256(e0, 1);
        __m128i o0_lo = _mm256_castsi256_si128(o0), o0_hi = _mm256_extracti128_si256(o0, 1);
        __m128i e1_lo = _mm256_castsi256_si128(e1), e1_hi = _mm256_extracti128_si256(e1, 1);
        __m128i o1_lo = _mm256_castsi256_si128(o1), o1_hi = _mm256_extracti128_si256(o1, 1);

        __m128i e0_128 = _mm_or_si128(e0_lo, _mm_slli_si128(e0_hi, 8));
        __m128i o0_128 = _mm_or_si128(o0_lo, _mm_slli_si128(o0_hi, 8));
        __m128i e1_128 = _mm_or_si128(e1_lo, _mm_slli_si128(e1_hi, 8));
        __m128i o1_128 = _mm_or_si128(o1_lo, _mm_slli_si128(o1_hi, 8));

        __m256i evens = _mm256_inserti128_si256(_mm256_castsi128_si256(e0_128), e1_128, 1);
        __m256i odds  = _mm256_inserti128_si256(_mm256_castsi128_si256(o0_128), o1_128, 1);

        // Branchless nibble: (c&0x0F) + (is_letter?9:0) with is_letter from lowercase compare
        __m256i e_low = _mm256_or_si256(evens, lower_mask);
        __m256i o_low = _mm256_or_si256(odds,  lower_mask);

        __m256i e_is_letter = _mm256_and_si256(_mm256_cmpgt_epi8(e_low, ch_a_m1), _mm256_cmpgt_epi8(ch_f_p1, e_low));
        __m256i o_is_letter = _mm256_and_si256(_mm256_cmpgt_epi8(o_low, ch_a_m1), _mm256_cmpgt_epi8(ch_f_p1, o_low));

        evens = _mm256_add_epi8(_mm256_and_si256(evens, low_nib), _mm256_and_si256(e_is_letter, add9));
        odds  = _mm256_add_epi8(_mm256_and_si256(odds,  low_nib), _mm256_and_si256(o_is_letter, add9));

        __m256i out = _mm256_or_si256(_mm256_slli_epi16(evens, 4), odds);
        _mm256_storeu_si256((__m256i*)(bin + i/2), out);
    }

    for (; i + 1 < n; i += 2) {
        bin[i/2] = (unsigned char)((toNib_branchless(hex[i]) << 4) | toNib_branchless(hex[i+1]));
    }
}

static void avx2opt_to_hex(const unsigned char *bin, unsigned char *hex, size_t nbytes) {
    // Same as avx2_to_hex (lookup is already optimal here)
    avx2_to_hex(bin, hex, nbytes);
}
#endif // __AVX2__

// ---------------------- Demo / Round-trip check ----------------------

static void print_hex_line(const char *tag, const unsigned char *p, size_t n) {
    printf("%s: ", tag);
    for (size_t i = 0; i < n; ++i) putchar(p[i]);
    putchar('\n');
}

int main(void) {
    const size_t H = TESTDATALEN;
    const size_t B = H/2;

    // SCALAR
    scalar_to_bin(testdata, out_bin_scalar, H);
    scalar_to_hex(out_bin_scalar, out_hex_scalar, B);

#ifdef __SSE2__
    sse2_to_bin(testdata, out_bin_sse2, H);
    sse2_to_hex(out_bin_sse2, out_hex_sse2, B);
#endif
#ifdef __SSSE3__
    ssse3_to_bin(testdata, out_bin_ssse3, H);
    ssse3_to_hex(out_bin_ssse3, out_hex_ssse3, B);
#endif
#ifdef __AVX2__
    avx2_to_bin(testdata, out_bin_avx2, H);
    avx2_to_hex(out_bin_avx2, out_hex_avx2, B);

    avx2opt_to_bin(testdata, out_bin_avx2op, H);
    avx2opt_to_hex(out_bin_avx2op, out_hex_avx2op, B);
#endif

    // Print a quick summary and equality checks
    print_hex_line("Input HEX      ", testdata, H);
    print_hex_line("Scalar->HEX    ", out_hex_scalar, H);

#ifdef __SSE2__


    print_hex_line("SSE2  ->HEX    ", out_hex_sse2, H);
    printf("Scalar==SSE2    : %s\n", memcmp(out_hex_scalar, out_hex_sse2, H)==0?"OK":"MISMATCH");
#endif
#ifdef __SSSE3__
    print_hex_line("SSSE3 ->HEX    ", out_hex_ssse3, H);
    printf("Scalar==SSSE3   : %s\n", memcmp(out_hex_scalar, out_hex_ssse3, H)==0?"OK":"MISMATCH");
#endif
#ifdef __AVX2__
    print_hex_line("AVX2  ->HEX    ", out_hex_avx2, H);
    print_hex_line("AVX2o ->HEX    ", out_hex_avx2op, H);
    printf("Scalar==AVX2    : %s\n", memcmp(out_hex_scalar, out_hex_avx2, H)==0?"OK":"MISMATCH");
    printf("Scalar==AVX2opt : %s\n", memcmp(out_hex_scalar, out_hex_avx2op, H)==0?"OK":"MISMATCH");
#endif

    // Also verify HEX→BIN equality between paths
#ifdef __SSE2__
    printf("BIN Scalar==SSE2  : %s\n", memcmp(out_bin_scalar, out_bin_sse2, B)==0?"OK":"MISMATCH");
#endif
#ifdef __SSSE3__
    printf("BIN Scalar==SSSE3 : %s\n", memcmp(out_bin_scalar, out_bin_ssse3, B)==0?"OK":"MISMATCH");
#endif
#ifdef __AVX2__
    printf("BIN Scalar==AVX2  : %s\n", memcmp(out_bin_scalar, out_bin_avx2, B)==0?"OK":"MISMATCH");
    printf("BIN Scalar==AVX2o : %s\n", memcmp(out_bin_scalar, out_bin_avx2op, B)==0?"OK":"MISMATCH");
#endif

	// compare output to uppercased input (round-trip check)
	memcpy(ref_upper, testdata, TESTDATALEN);
	to_upper_hex_inplace(ref_upper, TESTDATALEN);

	// Round-trip check vs. original input (uppercase/lowercase preserved only where input was valid hex).
	printf("Roundtrip Scalar : %s\n", memcmp(ref_upper, out_hex_scalar, H)==0?"OK":"MISMATCH");

#ifdef __SSE2__

	memcpy(ref_upper, testdata, TESTDATALEN);
	to_upper_hex_inplace(ref_upper, TESTDATALEN);

    printf("Roundtrip SSE2   : %s\n", memcmp(ref_upper, out_hex_sse2, H)==0?"OK":"MISMATCH");
#endif
#ifdef __SSSE3__
	memcpy(ref_upper, testdata, TESTDATALEN);
	to_upper_hex_inplace(ref_upper, TESTDATALEN);
    printf("Roundtrip SSSE3  : %s\n", memcmp(ref_upper, out_hex_ssse3, H)==0?"OK":"MISMATCH");
#endif
#ifdef __AVX2__
	memcpy(ref_upper, testdata, TESTDATALEN);
	to_upper_hex_inplace(ref_upper, TESTDATALEN);
    printf("Roundtrip AVX2   : %s\n", memcmp(ref_upper, out_hex_avx2, H)==0?"OK":"MISMATCH");
    printf("Roundtrip AVX2op : %s\n", memcmp(ref_upper, out_hex_avx2op, H)==0?"OK":"MISMATCH");
#endif

    return 0;
}

