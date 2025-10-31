#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <immintrin.h> // For SIMD intrinsics

// CPUID / XGETBV helpers (copied from hexsimd.c)
static void cpuid_x86(unsigned leaf, unsigned subleaf, unsigned regs[4]) {
#if defined(_MSC_VER)
    int cpuInfo[4];
    __cpuidex(cpuInfo, (int)leaf, (int)subleaf);
    regs[0]=(unsigned)cpuInfo[0]; regs[1]=(unsigned)cpuInfo[1];
    regs[2]=(unsigned)cpuInfo[2]; regs[3]=(unsigned)cpuInfo[3];
#else
    unsigned a,b,c,d;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
                               : "a"(leaf), "c"(subleaf));
    regs[0]=a; regs[1]=b; regs[2]=c; regs[3]=d;
#endif
}

static unsigned long long xgetbv_x86(unsigned idx) {
#if defined(_MSC_VER)
    return _xgetbv(idx);
#else
    unsigned eax, edx;
    __asm__ volatile (".byte 0x0f, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c"(idx));
    return ((unsigned long long)edx << 32) | eax;
#endif
}

typedef struct {
    int sse2, avx, avx2, avx512bw, avx512vl;
} isa_t;

static isa_t detect_isa_runtime(void) {
    isa_t f = {0};
    unsigned r[4] = {0};
    cpuid_x86(1,0,r);
    int osxsave = (r[2] & (1u<<27)) != 0;
    f.sse2 = (r[3] & (1u<<26)) != 0;

    if (osxsave) {
        unsigned long long xcr0 = xgetbv_x86(0);
        int os_avx = ((xcr0 & 0x6) == 0x6);
        if (os_avx && (r[2] & (1u<<28))) f.avx = 1;

        cpuid_x86(7,0,r);
        if (f.avx) f.avx2 = (r[1] & (1u<<5)) != 0;

        int os_avx512 = ((xcr0 & 0xE0) == 0xE0);
        if (os_avx512) {
            f.avx512bw = (r[1] & (1u<<30)) != 0;
            f.avx512vl = (r[1] & (1u<<31)) != 0;
        }
    }
    return f;
}

// Forward declarations of the functions to be benchmarked, as they are not in the header
ptrdiff_t hex_to_bytes_scalar_impl(const char* src, size_t len, uint8_t* dst, bool strict);
ptrdiff_t hex_to_bytes_sse2_impl(const char* src, size_t len, uint8_t* dst, bool strict);
ptrdiff_t hex_to_bytes_avx2_impl(const char* src, size_t len, uint8_t* dst, bool strict);
ptrdiff_t hex_to_bytes_avx512_impl(const char* src, size_t len, uint8_t* dst, bool strict);

#define HEX_STRING_LENGTH 2097152
#define NUM_ITERATIONS 1000

// Function pointer type for the hex_to_byte functions
typedef ptrdiff_t (*hex_to_byte_func)(const char*, size_t, uint8_t*, bool);

// Struct to hold function info
typedef struct {
    hex_to_byte_func func;
    const char* name;
} benchmark_func;

int main() {
    // A 2048 character hex string
    char hex_string[HEX_STRING_LENGTH + 1];
    for (int i = 0; i < HEX_STRING_LENGTH; i++) {
        hex_string[i] = "0123456789abcdef"[i % 16];
    }
    hex_string[HEX_STRING_LENGTH] = '\0';

    uint8_t* output_buffer = (uint8_t*)malloc(HEX_STRING_LENGTH / 2);
    if (!output_buffer) {
        perror("Failed to allocate memory");
        return 1;
    }

    isa_t isa_features = detect_isa_runtime();

    benchmark_func functions[4]; // Max 4 functions
    int num_functions = 0;

    functions[num_functions++] = (benchmark_func){hex_to_bytes_scalar_impl, "scalar"};

    if (isa_features.sse2) {
        functions[num_functions++] = (benchmark_func){hex_to_bytes_sse2_impl, "sse2"};
    }
    if (isa_features.avx2) {
        functions[num_functions++] = (benchmark_func){hex_to_bytes_avx2_impl, "avx2"};
    }
    if (isa_features.avx512bw && isa_features.avx512vl) {
        functions[num_functions++] = (benchmark_func){hex_to_bytes_avx512_impl, "avx512"};
    }

    for (int i = 0; i < num_functions; i++) {
        struct timespec start, end;
        double total_time = 0;

        for (int j = 0; j < NUM_ITERATIONS; j++) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            functions[i].func(hex_string, HEX_STRING_LENGTH, output_buffer, true);
            clock_gettime(CLOCK_MONOTONIC, &end);
            total_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        }

        double avg_time = total_time / NUM_ITERATIONS;
        printf("Average time for %s: %f seconds\n", functions[i].name, avg_time);
    }

    free(output_buffer);
    return 0;
}
