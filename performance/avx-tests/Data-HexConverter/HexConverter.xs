#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

#include "hexsimd.h"

MODULE = Data::HexConverter	PACKAGE = Data::HexConverter

ptrdiff_t
hex_to_bytes(src, strict = 1)
    const char* src
    bool strict
  CODE:
    STRLEN len = strlen(src);
    uint8_t* dst = (uint8_t*)safemalloc(len / 2);
    RETVAL = hex_to_bytes(src, len, dst, strict);
    if (RETVAL < 0) {
        safefree(dst);
        XSRETURN_EMPTY;
    }
    ST(0) = sv_newmortal();
    sv_setpvn(ST(0), (const char*)dst, RETVAL);
    safefree(dst);

const char*
hexsimd_hex2bin_impl_name()
  CODE:
    RETVAL = hexsimd_hex2bin_impl_name();
  OUTPUT:
    RETVAL

const char*
hexsimd_bin2hex_impl_name()
  CODE:
    RETVAL = hexsimd_bin2hex_impl_name();
  OUTPUT:
    RETVAL

void
bytes_to_hex()
  CODE:
    STRLEN len;
    const char* s = SvPV(ST(0), len);
    char* dst = (char*)safemalloc(len * 2 + 1);
    ptrdiff_t ret = bytes_to_hex((const uint8_t*)s, len, dst);
    dst[ret] = '\0';
    ST(0) = sv_newmortal();
    sv_setpvn(ST(0), dst, ret);
    safefree(dst);
