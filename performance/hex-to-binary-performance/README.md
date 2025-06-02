

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
PL/SQL anonymous block test of PL/SQL calculation functions: to-blob-chunk4k-test.sql
PL/SQL anonymous block test of PL/SQL hextoraw conversion function: hex64k-test.sql
PL/SQL test of hextoraw conversion function with data in a table: hex64-func-test.sql

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

Create the PL/SQL stored functions

```text
@hex-to-blob-plsql.sql
```

#### PL/SQL Procedures and Functions

Test a PL/SQL function that use dbms_lob.substr to get the hex data in 4k chunks, and uses hextoraw to convert the hex to binary.

This is similar to what is used in the SQL Developer Migration Workbench to convert hex data to binary.

```text
 @to-blob-chunk4k-test.sql
Convert to blob elapsed: 11.61
Convert to blob average: .116
Blob bytes : 1048576
```
nearly 12 seconds for 100 iterations.

This may not seem to slow, but it can be done faster than this.

Compile the function to native code:

```text
@plsql-compile-native
```

```text
 exec to_blob_chunk4k_test

only a very small improvement.

```text
@plsql-compile-native

Convert to blob elapsed: 11.47
Convert to blob average: .115
Blob bytes : 1048576

```

Now use an optimized PL/SQL function that does not use `dbms_lob.substr()`

```text
@hex64k-test.sql

Convert to blob elapsed: 3.31
Convert to blob average: .033
Blob bytes : 1048576
```
This is about 3x faster than the previous PL/SQL function.

## Rank of speed

Convert 2M of hex data to 1M binary blob data

1. C sse3 simd method
  - 0.000169 seconds avg
1. C 64k lookup method
  - 0.000372 seconds avg
1. C 32k lookup method
  - 0.000388 seconds avg
1. Python Bytes module
  - 0.000783 seconds avg
1. C basic lookup method
  - 0.000890 seconds avg
1. C 256 lookup method
  - 0.000895 seconds avg
1. C arithmetic method
  - 0.0066662 seconds avg
1. Perl pack() method
  - 0.007763 seconds avg
1. PL/SQL optimized function
  - 0.033000 seconds avg
1. PL/SQL chunked function
  - 0.115000 seconds avg



