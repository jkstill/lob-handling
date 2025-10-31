#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

#include "hexsimd.h"

MODULE = Data::HexConverter	PACKAGE = Data::HexConverter

PROTOTYPES: ENABLE

void
hex_to_bytes(const char* src, bool strict = true)
  CODE:
    STRLEN len = strlen(src);
    // Allocate buffer for the output. The result is half the length of the hex string.
    SV* retval_sv = newSV(len / 2);
    uint8_t* dst = (uint8_t*)SvPVX(retval_sv);

    ptrdiff_t res = hex_to_bytes(src, len, dst, strict);

    if (res < 0) {
        // On error (invalid input, odd length), return undef
        XSRETURN_EMPTY;
    } else {
        // Set the length of the SV and push it to the stack
        SvCUR_set(retval_sv, res);
        SvPOK_on(retval_sv);
        ST(0) = retval_sv;
        XSRETURN(1);
    }

void
bytes_to_hex(SV* bytes_sv)
  CODE:
    STRLEN len;
    const char* src = SvPV(bytes_sv, len);

    // Allocate buffer for the output. The result is twice the length of the byte string.
    SV* retval_sv = newSV(len * 2);
    char* dst = SvPVX(retval_sv);

    ptrdiff_t res = bytes_to_hex((const uint8_t*)src, len, dst);

    // Set the length of the SV and push it to the stack
    SvCUR_set(retval_sv, res);
    SvPOK_on(retval_sv);
    ST(0) = retval_sv;
    XSRETURN(1);

const char*
hexsimd_hex2bin_impl_name()

const char*
hexsimd_bin2hex_impl_name()
