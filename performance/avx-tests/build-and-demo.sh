#!/usr/bin/env bash

# Build everything
make -j

# Run the local shared-lib test
./tests/test_shared

# Try each path
HEXSIMD_FORCE=scalar ./tests/test_shared
HEXSIMD_FORCE=sse2   ./tests/test_shared
HEXSIMD_FORCE=avx2   ./tests/test_shared
HEXSIMD_FORCE=avx512 ./tests/test_shared

# Install (libs + header + pkg-config)
sudo make install

# Compile an external program with pkg-config
cat > demo.c <<'EOF'
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
EOF

export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

cc demo.c $(pkg-config --cflags --libs hexsimd) -o demo

# Make sure the runtime linker can find the .so (one of the following):
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# or add /usr/local/lib to /etc/ld.so.conf.d/custom.conf then:
# sudo ldconfig

./demo


