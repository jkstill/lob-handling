#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Returns number of output bytes/chars written.
// hex_to_bytes:  len must be even; returns -1 on strict parse failure.
// bytes_to_hex:  out_len must be >= 2*len; returns -1 on failure.
ptrdiff_t hex_to_bytes(const char *src, size_t len, uint8_t *dst, bool strict);
ptrdiff_t bytes_to_hex(const uint8_t *src, size_t len, char *dst);

// Optional: return the chosen implementation names (for debugging).
const char* hexsimd_hex2bin_impl_name(void);
const char* hexsimd_bin2hex_impl_name(void);
