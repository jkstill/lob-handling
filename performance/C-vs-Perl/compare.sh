#!/usr/bin/env bash

echo
echo "Running tests... 10240 iterations"
echo "================"
echo "Perl:"
echo
time ./kitty-perl.pl

echo
echo "C"
echo t
time ./kitty-C
echo

