#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "hexsimd.h"
int main(){
  const char *hx="F2C78D5A3E91B4C0F8D4730AB9E6254D";
  uint8_t b[64]; char out[128];
  ptrdiff_t n=hex_to_bytes(hx, strlen(hx), b, true);
  ptrdiff_t m=bytes_to_hex(b, (size_t)n, out); out[m]=0;
  puts(out); return 0;
}
