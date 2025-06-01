#!/usr/bin/env	bash

# unusual sizes, if used, are intentional
# the hex_to_bin() functions being tested work on 4 bytes at a time, and remaining bytes must be handled separately
#BLOCK_SIZE=144
#BLOCKS=3

BLOCK_SIZE=1024
BLOCKS=$(( 2 * 1024  )) # 2MB

( 
	printf "char *testdata =\""
	cat /dev/urandom \
    | tr -d -c "0123456789abcdefABCDEF" \
    | tr '[A-Z]' '[a-z]' \
    | dd count=$BLOCKS iflag=fullblock bs=$BLOCK_SIZE; printf "\";\n" 
) > testdata.h

