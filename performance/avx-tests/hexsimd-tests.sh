#!/usr/bin/env bash


echo '========== default =========================='
./hexsimd
echo '========== scalar =========================='
HEXSIMD_FORCE=scalar ./hexsimd
echo '========== sse2   =========================='
HEXSIMD_FORCE=sse2 ./hexsimd
echo '========== avx2   =========================='
HEXSIMD_FORCE=avx2 ./hexsimd
echo '========== avx512 =========================='
HEXSIMD_FORCE=avx512 ./hexsimd

