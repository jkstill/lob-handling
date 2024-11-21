#!/usr/bin/env bash


rm -f clob-to-blob clob-to-blob.o

for i in {1..10}
do
	echo
done

# shared
#gcc -o clob-to-blob clob-to-blob.c -O2 -L$ORACLE_HOME/lib -lclntsh  -L/usr/local/lib -locilib -std=c99
# static - no need to install ocilib
gcc -msse2 -O3 -o clob-to-blob clob-to-blob.c -O2 -l:libocilib.a -L$ORACLE_HOME/lib -lclntsh  -L/usr/local/lib -std=c99


