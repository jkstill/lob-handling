// hex2bin_dispatch.c
// Runtime-dispatched hex(ASCII) -> bytes with AVX-512 / AVX2 / SSE2 / scalar fallbacks.
// Public API:
//   ptrdiff_t hex_to_bytes(const char *src, size_t len, uint8_t *dst, bool strict);
//
// Build (linux/mac):
//   gcc -O3 -Wall -Wextra -march=x86-64 -o hex2bin hex2bin-dispatch.o
//   gcc -O3 -Wall -Wextra -mavx2 -mavx512bw -mavx512vl -march=x86-64 -o hex2bin hex2bin-dispatch.o
//   # optionally add: -mavx2 -mavx512bw -mavx512vl   (these enable building those paths)
//   // create static lib
//   ar rcs libhexsimd.a hex2bin-dispatch.o
// Build (MSVC):
//   cl /O2 /W4 /EHsc hex2bin_dispatch.c
//
// Optional test main:
//   gcc -O3 -DTEST_HEX hex2bin-dispatch.c && ./a.out

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#if defined(_MSC_VER)
  #include <intrin.h>
#else
  #include <immintrin.h>
#endif

// -------------------------
// CPUID helpers (x86/x64)
// -------------------------
static void cpuid_x86(unsigned leaf, unsigned subleaf, unsigned regs[4]) {
#if defined(_MSC_VER)
    int cpuInfo[4];
    __cpuidex(cpuInfo, (int)leaf, (int)subleaf);
    regs[0]=(unsigned)cpuInfo[0]; regs[1]=(unsigned)cpuInfo[1];
    regs[2]=(unsigned)cpuInfo[2]; regs[3]=(unsigned)cpuInfo[3];
#elif defined(__GNUC__) || defined(__clang__)
    unsigned a,b,c,d;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
                               : "a"(leaf), "c"(subleaf));
    regs[0]=a; regs[1]=b; regs[2]=c; regs[3]=d;
#else
    regs[0]=regs[1]=regs[2]=regs[3]=0;
#endif
}

static unsigned long long xgetbv_x86(unsigned idx) {
#if defined(_MSC_VER)
    return _xgetbv(idx);
#elif defined(__GNUC__) || defined(__clang__)
    unsigned eax, edx;
    __asm__ volatile (".byte 0x0f, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c"(idx));
    return ((unsigned long long)edx << 32) | eax;
#else
    return 0ULL;
#endif
}

typedef struct {
    bool has_sse2;
    bool has_avx;
    bool has_avx2;
    bool has_avx512bw;
    bool has_avx512vl;
} isa_features_t;

static isa_features_t detect_isa(void) {
    isa_features_t f = {0};
    unsigned r[4] = {0};

    // Basic features
    cpuid_x86(1, 0, r);
    bool osxsave = (r[2] & (1u<<27)) != 0;
    f.has_sse2   = (r[3] & (1u<<26)) != 0;

    // AVX OS support check
    if (osxsave) {
        unsigned long long xcr0 = xgetbv_x86(0);
        bool os_avx = ((xcr0 & 0x6) == 0x6); // XMM (bit1) and YMM (bit2)
        if (os_avx && (r[2] & (1u<<28))) f.has_avx = true;
    }

    // Extended features (leaf 7)
    cpuid_x86(7, 0, r);
    if (f.has_avx) {
        f.has_avx2 = (r[1] & (1u<<5)) != 0; // EBX bit5
    }

    // AVX-512 requires OS support for ZMM/OPMASK
    if (osxsave) {
        unsigned long long xcr0 = xgetbv_x86(0);
        bool os_avx512 = ((xcr0 & 0xE0) == 0xE0); // Opmask(K) bit5, ZMM hi256 bit6, ZMM bit7
        if (os_avx512) {
            f.has_avx512bw = (r[1] & (1u<<30)) != 0; // EBX bit30
            f.has_avx512vl = (r[1] & (1u<<31)) != 0; // EBX bit31
        }
    }
    return f;
}

// -------------------------
// Scalar fallback
// -------------------------
static inline int to_nibble_scalar(unsigned char c, unsigned char *out) {
    if (c >= '0' && c <= '9') { *out = (unsigned char)(c - '0'); return 1; }
    unsigned char u = (unsigned char)(c & ~0x20u); // fold to upper
    if (u >= 'A' && u <= 'F') { *out = (unsigned char)(10 + (u - 'A')); return 1; }
    return 0;
}

static ptrdiff_t hex_to_bytes_scalar(const char *src, size_t len, uint8_t *dst, bool strict) {
    if (len & 1) return -1;
    size_t o = 0;
    for (size_t i = 0; i < len; i += 2) {
        unsigned char hi, lo;
	hi=' ';
	lo=' ';
        int v1 = to_nibble_scalar((unsigned char)src[i], &hi);
        int v2 = to_nibble_scalar((unsigned char)src[i+1], &lo);
        if (strict && (!v1 || !v2)) return -1;
        // If not strict, treat invalid as zeroed nibble
        if (!v1) hi = 0;
        if (!v2) lo = 0;
        dst[o++] = (uint8_t)((hi << 4) | lo);
    }
    return (ptrdiff_t)o;
}

// -------------------------
// SSE2 path (16 ASCII -> 8 bytes)
// -------------------------
#if defined(__SSE2__) || (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86)))
#include <emmintrin.h>

static inline __m128i sse2_toNib(__m128i x, __m128i *valid_mask_out, bool want_valid) {
    const __m128i c0 = _mm_set1_epi8('0');
    const __m128i c9p1 = _mm_set1_epi8('9'+1);
    const __m128i cA = _mm_set1_epi8('A');
    const __m128i cFp1 = _mm_set1_epi8('F'+1);
    const __m128i casebit = _mm_set1_epi8(0x20);
    const __m128i ten = _mm_set1_epi8(10);

    __m128i upper = _mm_andnot_si128(casebit, x);

    // digit: '0' <= x < '9'+1
    __m128i ge0 = _mm_cmpeq_epi8(_mm_max_epu8(x, c0), x);
    __m128i lt10= _mm_cmpeq_epi8(_mm_min_epu8(x, c9p1), x);
    __m128i is_digit = _mm_and_si128(ge0, lt10);

    // alpha: 'A' <= upper < 'F'+1
    __m128i geA = _mm_cmpeq_epi8(_mm_max_epu8(upper, cA), upper);
    __m128i ltG = _mm_cmpeq_epi8(_mm_min_epu8(upper, cFp1), upper);
    __m128i is_alpha = _mm_and_si128(geA, ltG);

    __m128i dval = _mm_sub_epi8(x, c0);
    __m128i lval = _mm_add_epi8(_mm_sub_epi8(upper, cA), ten);

    __m128i sel_d = _mm_and_si128(is_digit, dval);
    __m128i sel_l = _mm_andnot_si128(is_digit, lval);
    __m128i nib = _mm_or_si128(sel_d, sel_l);
    nib = _mm_and_si128(nib, _mm_set1_epi8(0x0F));

    if (want_valid && valid_mask_out) {
        __m128i valid = _mm_or_si128(is_digit, is_alpha);
        *valid_mask_out = valid;
    }
    return nib;
}

static inline __m128i sse2_pack_pairs(__m128i n) {
    // pairs: (even<<4)|odd
    __m128i even = _mm_and_si128(n, _mm_set1_epi16(0x00FF));
    __m128i odd  = _mm_and_si128(_mm_srli_epi16(n, 8), _mm_set1_epi16(0x00FF));
    __m128i hi = _mm_slli_epi16(even, 4);
    __m128i w16 = _mm_or_si128(hi, odd);
    return _mm_packus_epi16(w16, _mm_setzero_si128()); // low 8 bytes valid
}

static ptrdiff_t hex_to_bytes_sse2(const char *src, size_t len, uint8_t *dst, bool strict) {
    if (len & 1) return -1;
    size_t i = 0, o = 0;
    const size_t CH = 16;

    for (; i + CH <= len; i += CH, o += CH/2) {
        __m128i x = _mm_loadu_si128((const __m128i*)(src + i));
        __m128i valid;
        __m128i n = sse2_toNib(x, &valid, /*want_valid=*/strict);
        if (strict) {
            // valid lanes are 0xFF bytes; check all set
            // Fall back to scalar-ish check: move mask to bits
            unsigned mask = (unsigned)_mm_movemask_epi8(_mm_cmpeq_epi8(valid, _mm_set1_epi8((char)0xFF)));
            if (mask != 0xFFFF) return -1;
        }
        __m128i out = sse2_pack_pairs(n);
        _mm_storel_epi64((__m128i*)(dst + o), out);
    }

    // tail
    if (i < len)
        return hex_to_bytes_scalar(src + i, len - i, dst + o, strict) >= 0
               ? (ptrdiff_t)(o + ((len - i) >> 1)) : -1;
    return (ptrdiff_t)o;
}
#endif

// -------------------------
// AVX2/AVX (32 ASCII -> 16 bytes)
// -------------------------
#if defined(__AVX2__) || defined(__AVX__)
#include <immintrin.h>

static inline __m256i avx_toNib(__m256i x, __m256i *valid_out, bool want_valid) {
    const __m256i c0 = _mm256_set1_epi8('0');
    const __m256i c9p1 = _mm256_set1_epi8('9'+1);
    const __m256i cA = _mm256_set1_epi8('A');
    const __m256i cFp1 = _mm256_set1_epi8('F'+1);
    const __m256i casebit = _mm256_set1_epi8(0x20);
    const __m256i ten = _mm256_set1_epi8(10);

    __m256i upper = _mm256_andnot_si256(casebit, x);

    __m256i ge0 = _mm256_cmpeq_epi8(_mm256_max_epu8(x, c0), x);
    __m256i lt10= _mm256_cmpeq_epi8(_mm256_min_epu8(x, c9p1), x);
    __m256i is_digit = _mm256_and_si256(ge0, lt10);

    __m256i geA = _mm256_cmpeq_epi8(_mm256_max_epu8(upper, cA), upper);
    __m256i ltG = _mm256_cmpeq_epi8(_mm256_min_epu8(upper, cFp1), upper);
    __m256i is_alpha = _mm256_and_si256(geA, ltG);

    __m256i dval = _mm256_sub_epi8(x, c0);
    __m256i lval = _mm256_add_epi8(_mm256_sub_epi8(upper, cA), ten);

    __m256i sel_d = _mm256_and_si256(is_digit, dval);
    __m256i sel_l = _mm256_andnot_si256(is_digit, lval);
    __m256i nib = _mm256_or_si256(sel_d, sel_l);
    nib = _mm256_and_si256(nib, _mm256_set1_epi8(0x0F));

    if (want_valid && valid_out) *valid_out = _mm256_or_si256(is_digit, is_alpha);
    return nib;
}

static inline __m256i avx_pack_pairs(__m256i n) {
    __m256i even = _mm256_and_si256(n, _mm256_set1_epi16(0x00FF));
    __m256i odd  = _mm256_and_si256(_mm256_srli_epi16(n, 8), _mm256_set1_epi16(0x00FF));
    __m256i hi   = _mm256_slli_epi16(even, 4);
    __m256i w16  = _mm256_or_si256(hi, odd);
    // pack 128-bit halves separately and rejoin
    __m128i lo16 = _mm256_castsi256_si128(w16);
    __m128i hi16 = _mm256_extracti128_si256(w16, 1);
    __m128i lo8  = _mm_packus_epi16(lo16, _mm_setzero_si128());
    __m128i hi8  = _mm_packus_epi16(hi16, _mm_setzero_si128());
    return _mm256_set_m128i(hi8, lo8);
}

static ptrdiff_t hex_to_bytes_avx(const char *src, size_t len, uint8_t *dst, bool strict) {
    if (len & 1) return -1;
    size_t i = 0, o = 0;
    const size_t CH = 32;

    for (; i + CH <= len; i += CH, o += CH/2) {
        __m256i x = _mm256_loadu_si256((const __m256i*)(src + i));
        __m256i valid;
        __m256i n = avx_toNib(x, &valid, /*want_valid=*/strict);
        if (strict) {
            // Check validity across 32 lanes (bytes)
            __m256i allFF = _mm256_cmpeq_epi8(valid, _mm256_set1_epi8((char)0xFF));
            unsigned mask = (unsigned)_mm256_movemask_epi8(allFF);
            if (mask != 0xFFFFFFFFu) return -1;
        }
        __m256i out = avx_pack_pairs(n);
        // store 16 output bytes: the low 8 bytes of each 128-bit half
        __m128i lo = _mm256_castsi256_si128(out);
        __m128i hi = _mm256_extracti128_si256(out, 1);
        _mm_storel_epi64((__m128i*)(dst + o + 0), lo);
        _mm_storel_epi64((__m128i*)(dst + o + 8), hi);
    }

    // tail
    if (i < len)
        return hex_to_bytes_scalar(src + i, len - i, dst + o, strict) >= 0
               ? (ptrdiff_t)(o + ((len - i) >> 1)) : -1;
    return (ptrdiff_t)o;
}
#endif

// -------------------------
// AVX-512BW/VL path (64 ASCII -> 32 bytes)
// -------------------------
#if defined(__AVX512BW__) && defined(__AVX512VL__)
#include <immintrin.h>

static inline __m512i toNib_avx512(__m512i x, __mmask64 *out_valid) {
    const __m512i c0   = _mm512_set1_epi8('0');
    const __m512i c9   = _mm512_set1_epi8('9');
    const __m512i cA   = _mm512_set1_epi8('A');
    const __m512i cF   = _mm512_set1_epi8('F');
    const __m512i loUp = _mm512_set1_epi8(0x20);
    const __m512i ten  = _mm512_set1_epi8(10);

    __m512i upper = _mm512_andnot_si512(loUp, x);

    __mmask64 m_digit = _mm512_cmp_epu8_mask(x, c0, _MM_CMPINT_GE)
                      & _mm512_cmp_epu8_mask(x, c9, _MM_CMPINT_LE);
    __mmask64 m_alpha = _mm512_cmp_epu8_mask(upper, cA, _MM_CMPINT_GE)
                      & _mm512_cmp_epu8_mask(upper, cF, _MM_CMPINT_LE);

    __m512i dval = _mm512_sub_epi8(x, c0);
    __m512i lval = _mm512_add_epi8(_mm512_sub_epi8(upper, cA), ten);

    __m512i nib  = _mm512_mask_blend_epi8(m_digit, lval, dval);
    nib = _mm512_and_si512(nib, _mm512_set1_epi8(0x0F));

    if (out_valid) *out_valid = (m_digit | m_alpha);
    return nib;
}

static inline __m512i pack_pairs_avx512(__m512i nib) {
    const __m512i mul = _mm512_set1_epi16(0x0110);
    __m512i sum16 = _mm512_maddubs_epi16(nib, mul);    // 32x u16
    __m512i zeros = _mm512_setzero_si512();
    return _mm512_packus_epi16(sum16, zeros);          // low 32 bytes are output
}

static ptrdiff_t hex_to_bytes_avx512(const char *src, size_t len, uint8_t *dst, bool strict) {
    if (len & 1) return -1;
    size_t i = 0, o = 0;
    const size_t CH = 64;

    for (; i + CH <= len; i += CH, o += CH/2) {
        __m512i x   = _mm512_loadu_si512((const void*)(src + i));
        __mmask64 valid;
        __m512i n   = toNib_avx512(x, &valid);
        if (strict && valid != ~(__mmask64)0) return -1;
        __m512i out = pack_pairs_avx512(n);
        // store low 32 bytes
        _mm256_storeu_si256((__m256i*)(dst + o), _mm512_castsi512_si256(out));
    }

    // tail with masked load/store
    size_t rem = len - i;
    if (rem) {
        __mmask64 imask = (rem == 64) ? ~(__mmask64)0 : (((__mmask64)1 << rem) - 1);
        __m512i x = _mm512_maskz_loadu_epi8(imask, (const void*)(src + i));

        __mmask64 valid;
        __m512i n = toNib_avx512(x, &valid);
        if (strict) {
            if ((valid & imask) != imask) return -1;
        }

        __m512i out = pack_pairs_avx512(n);
        size_t out_rem = rem >> 1;
        __mmask64 omask = (out_rem == 32) ? ~(__mmask64)0 : (((__mmask64)1 << out_rem) - 1);
        _mm512_mask_storeu_epi8((void*)(dst + o), omask, out);
        o += out_rem;
    }
    return (ptrdiff_t)o;
}
#endif

// -------------------------
// Public API: dispatcher
// -------------------------
typedef ptrdiff_t (*hex2bin_fn)(const char*, size_t, uint8_t*, bool);

static hex2bin_fn g_hex2bin_impl = NULL;

static hex2bin_fn pick_impl(void) {
    isa_features_t f = detect_isa();

#if defined(__AVX512BW__) && defined(__AVX512VL__)
    if (f.has_avx512bw && f.has_avx512vl) return &hex_to_bytes_avx512;
#endif
#if defined(__AVX2__) || defined(__AVX__)
    if (f.has_avx2 || f.has_avx)          return &hex_to_bytes_avx;
#endif
#if defined(__SSE2__) || (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86)))
    if (f.has_sse2)                        return &hex_to_bytes_sse2;
#endif
    return &hex_to_bytes_scalar;
}

static inline hex2bin_fn get_impl(void) {
    // Simple thread-safe-ish init for most apps; for strict MT, use call_once.
    if (!g_hex2bin_impl) g_hex2bin_impl = pick_impl();
    return g_hex2bin_impl;
}

ptrdiff_t hex_to_bytes(const char *src, size_t len, uint8_t *dst, bool strict) {
    return get_impl()(src, len, dst, strict);
}

// -------------------------
// Optional micro-test
// -------------------------
#ifdef TEST_HEX
static void print_hex(const uint8_t *b, size_t n) {
    for (size_t i=0;i<n;i++) printf("%02X", b[i]);
    putchar('\n');
}
int main(void) {
    const char *hx = "F2C78D5A3E91B4C0F8D4730AB9E6254D";
    size_t L = strlen(hx);
    uint8_t out[64]={0};
    ptrdiff_t n = hex_to_bytes(hx, L, out, true);
    if (n < 0) { puts("strict parse failed"); return 1; }
    printf("bytes: %td\n", n);
    print_hex(out, (size_t)n); // prints back as hex for sanity
    return 0;
}
#endif
