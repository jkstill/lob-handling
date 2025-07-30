#!/usr/bin/env	bash

output_file="data/testdata.hex"
BLOCK_SIZE=1024
BLOCKS=$(( 2 * 1024  )) # 2M

( 
	cat /dev/urandom \
    | tr -d -c "0123456789abcdefABCDEF" \
	 | tr '[A-Z]' '[a-z]' 
) | dd count=$BLOCKS iflag=fullblock bs=$BLOCK_SIZE > $output_file


output_file="data/testdata-100M.hex"
BLOCKS=$(( 100 * 1024  )) # 100M

( 
	cat /dev/urandom \
    | tr -d -c "0123456789abcdefABCDEF" \
	 | tr '[A-Z]' '[a-z]' 
) | dd count=$BLOCKS iflag=fullblock bs=$BLOCK_SIZE > $output_file


