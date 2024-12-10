#!/usr/bin/env	bash


( 
	printf "char *testdata =\""
	cat /dev/urandom \
    | tr -d -c "0123456789abcdefABCDEF" \
    | tr '[A-Z]' '[a-z]' \
    | dd count=10 iflag=fullblock bs=1M; printf "\";\n" 
) > testdata.h

