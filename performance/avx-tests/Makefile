# Makefile for hexsimd
# Usage:
#   make universal    # ship-one, run-anywhere (runtime CPUID picks path)
#   make native       # build only what this machine supports
#   make clean

CC      ?= gcc
AR      ?= ar
CFLAGS  ?= -O3 -Wall -Wextra -fPIC
LDFLAGS ?=
LIB     := libhexsimd.a
HDR     := hexsimd.h
SRC     := hexsimd.c

# Probe program to detect runtime-supported ISAs for 'native' build.
# It prints: FEATURES: sse2 avx avx2 avx512bw avx512vl (whatever applies)
PROBE := probe_isa
PROBE_SRC := $(PROBE).c
$(PROBE_SRC):
	@printf '%s\n' '#include <stdio.h>' > $(PROBE_SRC)
	@printf '%s\n' '#include <immintrin.h>' >> $(PROBE_SRC)
	@printf '%s\n' 'int main(){' >> $(PROBE_SRC)
	@printf '%s\n' '  unsigned a,b,c,d; int sse2=0,avx=0,avx2=0,avx512bw=0,avx512vl=0; ' >> $(PROBE_SRC)
	@printf '%s\n' '  __asm__ __volatile__("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(1),"c"(0));' >> $(PROBE_SRC)
	@printf '%s\n' '  sse2 = !!(d & (1u<<26));' >> $(PROBE_SRC)
	@printf '%s\n' '  int osxsave = !!(c & (1u<<27));' >> $(PROBE_SRC)
	@printf '%s\n' '  unsigned long long xcr0=0; if(osxsave){ unsigned eax,edx; __asm__ __volatile__(".byte 0x0f,0x01,0xd0":"=a"(eax),"=d"(edx):"c"(0)); xcr0=((unsigned long long)edx<<32)|eax; }' >> $(PROBE_SRC)
	@printf '%s\n' '  int os_avx = osxsave && ((xcr0 & 0x6)==0x6); avx = os_avx && !!(c & (1u<<28));' >> $(PROBE_SRC)
	@printf '%s\n' '  __asm__ __volatile__("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(7),"c"(0));' >> $(PROBE_SRC)
	@printf '%s\n' '  avx2 = avx && !!(b & (1u<<5));' >> $(PROBE_SRC)
	@printf '%s\n' '  int os_avx512 = osxsave && ((xcr0 & 0xE0)==0xE0);' >> $(PROBE_SRC)
	@printf '%s\n' '  avx512bw = os_avx512 && !!(b & (1u<<30));' >> $(PROBE_SRC)
	@printf '%s\n' '  avx512vl = os_avx512 && !!(b & (1u<<31));' >> $(PROBE_SRC)
	@printf '%s\n' '  printf("FEATURES:"); if(sse2)printf(" sse2"); if(avx)printf(" avx"); if(avx2)printf(" avx2"); if(avx512bw)printf(" avx512bw"); if(avx512vl)printf(" avx512vl"); printf("\n"); return 0; }' >> $(PROBE_SRC)

.PHONY: universal native clean

# =========
# Universal
# =========
universal: CFLAGS += -DTEST_HEX
universal: CFLAGS += -DHEXSIMD_UNIVERSAL
universal: $(LIB) hextest hexsimd

# For universal, we rely on per-function #pragma GCC target inside hexsimd.c,
# so a single object is fine.
$(LIB): hexsimd.o
	$(AR) rcs $@ $^

hexsimd.o: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -mavx512bw -mavx512vl -c $(SRC) -o $@

hexsimd: $(SRC) $(HDR)
	$(CC) -O3 -Wall -Wextra -mavx512bw -mavx512vl -DTEST_HEX hexsimd.c -o hexsimd

hextest: hextest.c $(LIB) $(HDR)
	$(CC) $(CFLAGS) $< -L. -lhexsimd -o $@

# =========
# Native
# =========
native: $(PROBE_SRC)
	@$(CC) $(CFLAGS) -o $(PROBE) $(PROBE_SRC)
	@feat=`./$(PROBE)`; \
	echo "$$feat"; \
	rm -f $(PROBE) $(PROBE_SRC); \
	case "$$feat" in \
	  *avx512bw*avx512vl*) echo "Building with AVX-512BW/VL"; \
	    $(CC) $(CFLAGS) -mavx512bw -mavx512vl -DHEXSIMD_NATIVE -c $(SRC) -o hexsimd.o ;; \
	  *avx2*) echo "Building with AVX2"; \
	    $(CC) $(CFLAGS) -mavx2 -DHEXSIMD_NATIVE -c $(SRC) -o hexsimd.o ;; \
	  *sse2*) echo "Building with SSE2"; \
	    $(CC) $(CFLAGS) -msse2 -DHEXSIMD_NATIVE -c $(SRC) -o hexsimd.o ;; \
	  *) echo "Building scalar only"; \
	    $(CC) $(CFLAGS) -DHEXSIMD_NATIVE -c $(SRC) -o hexsimd.o ;; \
	esac
	$(AR) rcs $(LIB) hexsimd.o
	$(CC) $(CFLAGS) hextest.c -L. -lhexsimd -o hextest

# =========
# Helpers
# =========
hextest.c:
	@printf '%s\n' '#include "hexsimd.h"' > $@
	@printf '%s\n' '#include <stdio.h>' >> $@
	@printf '%s\n' 'int main(void){' >> $@
	@printf '%s\n' '  const char *hx="F2C78D5A3E91B4C0F8D4730AB9E6254D";' >> $@
	@printf '%s\n' '  unsigned char out[64]; char back[128];' >> $@
	@printf '%s\n' '  ptrdiff_t n=hex_to_bytes(hx, 32, out, 1);' >> $@
	@printf '%s\n' '  if(n<0){puts("hex_to_bytes failed");return 1;}' >> $@
	@printf '%s\n' '  ptrdiff_t m=bytes_to_hex(out,(size_t)n,back);' >> $@
	@printf '%s\n' '  back[m]=0;' >> $@
	@printf '%s\n' '  printf("hex2bin impl: %s\n", hexsimd_hex2bin_impl_name());' >> $@
	@printf '%s\n' '  printf("bin2hex impl: %s\n", hexsimd_bin2hex_impl_name());' >> $@
	@printf '%s\n' '  printf("roundtrip: %s\n", back);' >> $@
	@printf '%s\n' '  return 0; }' >> $@

clean:
	rm -f *.o $(LIB) hexsimd hextest hextest.c $(PROBE) $(PROBE_SRC)


