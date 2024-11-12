#!/usr/bin/env python

import sys
import struct

# Generate base16_decoding_table1
sys.stdout.write("unsigned char base16_decoding_table1[256] = {\n")

for i in range(256):
    c = chr(i)
    if c in '0123456789abcdefABCDEF':
        j = str(int(c, 16))
    else:
        j = '0'
    sys.stdout.write(j + ',')
sys.stdout.write("};\n\n")

# Generate base16_decoding_table2
l = ["0"] * (128 * 256)
hex_digits = '0123456789abcdefABCDEF'

for a in hex_digits:
    for b in hex_digits:
        ab = (a + b).encode('ascii')
        index = struct.unpack("<H", ab)[0]
        l[index] = str(int(a + b, 16))

sys.stdout.write(f"unsigned char base16_decoding_table2[{128 * 256}] = {{\n")
line = ""
for e in l:
    line += e + ","
    if len(line) > 70:
        sys.stdout.write(line + "\n")
        line = ""
sys.stdout.write(line + "};\n\n")

# Generate base16_decoding_table3
l = ["0"] * (256 * 256)

for a in hex_digits:
    for b in hex_digits:
        ab = (a + b).encode('ascii')
        index = struct.unpack("<H", ab)[0]
        l[index] = str(int(a + b, 16))

sys.stdout.write(f"unsigned char base16_decoding_table3[{256 * 256}] = {{\n")
line = ""
for e in l:
    line += e + ","
    if len(line) > 70:
        sys.stdout.write(line + "\n")
        line = ""
sys.stdout.write(line + "};\n")
