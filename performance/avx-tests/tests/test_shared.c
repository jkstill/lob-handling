#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../hexsimd.h"

int main(void){
    const char *hx =
        "32D45FA2883337F16CAF523264E538D1AD89BD2924B67693AF1A7BCE7C6041AC"
        "96528A702C1FCAB51F75B14B6A5F20B1BAAFD93E9AC30769247EB6FAF408087F";
    uint8_t bin[256]; char back[512];
    ptrdiff_t n = hex_to_bytes(hx, strlen(hx), bin, true);
    if (n < 0) { puts("hex_to_bytes failed"); return 1; }
    ptrdiff_t m = bytes_to_hex(bin, (size_t)n, back);
    back[m] = 0;

    printf("impl hex2bin: %s\n", hexsimd_hex2bin_impl_name());
    printf("impl bin2hex: %s\n", hexsimd_bin2hex_impl_name());
    if (strcmp(hx, back) != 0) {
        puts("roundtrip mismatch"); return 2;
    }
    puts("roundtrip ok");
    return 0;
}
