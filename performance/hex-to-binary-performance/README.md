

https://stackoverflow.com/questions/8551383/how-to-convert-a-hexadecimal-string-to-a-binary-string-in-c#

```
./create-testdata.sh
./create-header.py

$  gcc -o hex-to-bin-tests hex-to-bin-tests.c

./hex-to-bin-tests

```

Additional optimized hex to binary conversions test6() using SIMD with help from ChatGTP

Compile:

* -msse2 for SSE2 emmintrin.h
* -mssse3 for SSSE3 tmmintrin.h

```text
$  gcc -mssse3 -O3 -o hex-to-bin-tests-new hex-to-bin-tests-new.c

$  gcc -msse2 -mssse3 -O3 -O2 -o hex-to-bin-tests-new hex-to-bin-tests-new.c
```

## Tests:

The sse2 tests are invalid as an as yet unresolved bug creates the wrong output.
I keep the test here to motivate me to fix the bug, as sse2 is so fast.

PL/SQL, Perl and Python results are shown as well.

C: hex-to-bin-tests-new.c
Python: hex-to-bin-test.py
Perl: hex-to-bin-test.pl
PL/SQL anonymous block: hex-to-bin-test.sql

### C

with various methods of converting hex to binary

```text
==>> running hex-to-bin-tests-new
TESTDATALEN: 2000000

arithmetic solution calcHex() took 0.666615 seconds avg: 0.006666153
optimized lookup superScalarSSE2() took 0.006210 seconds avg: 0.000062099
optimized lookup superScalarSSSE3() took 0.016884 seconds avg: 0.000168841
optimized lookup lookupBasic() took 0.088976 seconds avg: 0.000889763
256 entries table lookup256() took 0.089497 seconds avg: 0.000894965
32768 entries table lookup32k() took 0.038764 seconds avg: 0.000387644
65536 entries table lookup64k() took 0.037160 seconds avg: 0.000371600

```

### Python

The Bytes module is very fast compared to pack() in Perl.

Python is also very fast compared to the straight calculation in C.

The optimized C lookup methods with 32k and 64k lookups are about 2x faster than Python.

The C sse3 simd method is about 5x faster than Python.

```text
./hex-to-bin-test.py

Elapsed: 0.078338 seconds
   Avg : 0.000783 seconds
```

### Perl

10x slower than Python

```text
./hex-to-bin-test.pl

Elapsed: 0.776294
    Avg: 0.007763
```

### PL/SQL

Call the PL/SQL stored functions

#### PL/SQL anonymous block

#### PL/SQL Procedure

#### PL/SQL Compiled Procedure

Compile the functions and the calling procedure:






