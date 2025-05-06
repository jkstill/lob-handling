#!/usr/bin/env	bash

MEGABYTES=2

( 
	printf "char *testdata =\""
	cat /dev/urandom \
    | tr -d -c "0123456789abcdefABCDEF" \
    | tr '[A-Z]' '[a-z]' \
    | dd count=$MEGABYTES iflag=fullblock bs=1M; printf "\";\n" 
) > testdata.h

