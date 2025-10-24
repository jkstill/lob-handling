#include <immintrin.h>
#include <stdio.h>
#include <cpuid.h>

int main(void) {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        if (ebx & bit_AVX512F)
            printf("AVX-512F supported\n");
        else
            printf("No AVX-512F\n");
    }
}
