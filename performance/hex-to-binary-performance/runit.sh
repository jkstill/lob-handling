#!/usr/bin/env bash

banner () {
	echo
	echo "==>> $@"
}

banner "remove old data"
rm data/*

banner "compiling hex-to-bin-tests-new.c"
./build.sh

banner "running hex-to-bin-tests-new"
./hex-to-bin-tests-new

banner "convert binary to hex and compare with chksum"
./dumphex.sh


