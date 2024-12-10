#!/usr/bin/env bash

banner () {
	echo
	echo "==>> $@"
	echo
}

banner "remove old data"
rm data/*

banner "compiling hex-to-bin-tests-new.c"
gcc -msse2 -mssse3 -O3 -O2 -o hex-to-bin-tests-new hex-to-bin-tests-new.c

banner "running hex-to-bin-tests-new"
./hex-to-bin-tests-new

banner "convert binary to hex and compare with chksum"
./dumphex.sh


