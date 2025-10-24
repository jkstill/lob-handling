// #include <immintrin.h>
#include <stdio.h>
#include <cpuid.h>

int main(void) {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        if (ebx & bit_SSE2)
            printf("SSE2 supported\n");
        else
            printf("No SSE2\n");
    }
}
