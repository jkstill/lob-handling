# Portable Makefile for hexsimd: static + shared + pkg-config + test
CC      ?= gcc
AR      ?= ar
RANLIB  ?= ranlib
INSTALL ?= install

# ----- outputs -----
LIB_NAME   := hexsimd
MAJOR      := 1
MINOR      := 0
PATCH      := 0
SOVER      := $(MAJOR)
FULLVER    := $(MAJOR).$(MINOR).$(PATCH)

LIB_STATIC := lib$(LIB_NAME).a
LIB_SHARED := lib$(LIB_NAME).so.$(FULLVER)
SONAME     := lib$(LIB_NAME).so.$(SOVER)

BIN_TEST   := tests/test_shared
HEXSIMD    := hexsimd

# ----- dirs -----
SRC := hexsimd.c
HDR := hexsimd.h

PREFIX  ?= /usr/local
LIBDIR  := $(PREFIX)/lib
INCDIR  := $(PREFIX)/include
PKGDIR  := $(LIBDIR)/pkgconfig

# ----- flags -----
# Baseline ISA. DO NOT enable AVX globally.
CFLAGS_BASE ?= -O3 -Wall -Wextra -fPIC -fvisibility=hidden -march=x86-64 -mno-avx -mno-avx2 -mno-avx512f
LDFLAGS     ?=
# For the shared object: versioned SONAME and RUNPATH to $(LIBDIR) for convenience
LDFLAGS_SO  ?= -shared -Wl,-soname,$(SONAME)

# Optional knobs
ifdef NATIVE
  CFLAGS_BASE += -mtune=native
endif
ifdef NO_AVX2
  CFLAGS_BASE += -DHEXSIMD_NO_AVX2
endif
ifdef NO_AVX512
  CFLAGS_BASE += -DHEXSIMD_NO_AVX512
endif

# ----- rules -----
.PHONY: all
all: $(LIB_STATIC) $(LIB_SHARED) $(BIN_TEST) $(HEXSIMD)

# Static lib
$(LIB_STATIC): hexsimd.o
	$(AR) rcs $@ $^
	$(RANLIB) $@

# Shared lib (versioned)
$(LIB_SHARED): hexsimd.pic.o
	$(CC) $(LDFLAGS_SO) $^ -o $@ $(LDFLAGS)
	@ln -sf $(LIB_SHARED) lib$(LIB_NAME).so
	@ln -sf $(LIB_SHARED) $(SONAME)

$(HEXSIMD): $(SRC) $(HDR)
	$(CC) $(CFLAGS_BASE) -DTEST_HEX $(SRC) -o $@

# PIC objects for the shared lib
hexsimd.pic.o: $(SRC) $(HDR)
	$(CC) $(CFLAGS_BASE) -DHEXSIMD_BUILD -c $(SRC) -o $@

# non-PIC object is fine too (we already use -fPIC in base flags, but keep both)
hexsimd.o: $(SRC) $(HDR)
	$(CC) $(CFLAGS_BASE) -c $(SRC) -o $@

# Test program (links the shared lib via pkg-config once installed,
# but also link against local .so if not installed yet)
$(BIN_TEST): tests/test_shared.c $(LIB_SHARED) $(HDR)
	$(CC) -I. $(CFLAGS_BASE) $< -L. -l$(LIB_NAME) -Wl,-rpath,'$$ORIGIN/..' -o $@

# ----- pkg-config -----
PC := $(LIB_NAME).pc
$(PC): hexsimd.pc.in
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@VERSION@|$(FULLVER)|g' \
	    $< > $@

.PHONY: install uninstall
install: $(LIB_STATIC) $(LIB_SHARED) $(HDR) $(PC)
	$(INSTALL) -d $(LIBDIR) $(INCDIR) $(PKGDIR)
	$(INSTALL) -m 644 $(LIB_STATIC) $(LIBDIR)/
	$(INSTALL) -m 755 $(LIB_SHARED) $(LIBDIR)/
	cd $(LIBDIR) && ln -sf $(LIB_SHARED) $(SONAME) && ln -sf $(LIB_SHARED) lib$(LIB_NAME).so
	$(INSTALL) -m 644 $(HDR)       $(INCDIR)/
	$(INSTALL) -m 644 $(PC)        $(PKGDIR)/

uninstall:
	rm -f $(LIBDIR)/$(LIB_STATIC) \
	      $(LIBDIR)/$(LIB_SHARED) \
	      $(LIBDIR)/$(SONAME) \
	      $(LIBDIR)/lib$(LIB_NAME).so \
	      $(INCDIR)/$(HDR) \
	      $(PKGDIR)/$(PC)

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

.PHONY: selftest
selftest: $(BIN_TEST)
	@set -e; \
	for m in scalar sse2 avx2 avx512; do \
	  echo "== $$m =="; HEXSIMD_FORCE=$$m ./$(BIN_TEST) || exit $$?; \
	done

.PHONY: clean
clean:
	rm -f hexsimd hextest hextest.c *.o *.so* $(LIB_STATIC) $(LIB_SHARED) $(PC) $(BIN_TEST)

