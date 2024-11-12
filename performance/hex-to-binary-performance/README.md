

https://stackoverflow.com/questions/8551383/how-to-convert-a-hexadecimal-string-to-a-binary-string-in-c#

```
./create-testdata.sh
./create-header.py

$  gcc -o hex-to-bin-tests hex-to-bin-tests.c

./hex-to-bin-tests

```

Tests:

```text
$  ./hex-to-bin-tests
TESTDATALEN: 10485780

checksum: 790195474
arithmetic solution took 78.156182 seconds
checksum: 790194559
256 entries table took 14.271829 seconds
checksum: 790193573
32768 entries table took 5.858738 seconds
checksum: 790193573
65536 entries table took 6.422559 seconds
```

