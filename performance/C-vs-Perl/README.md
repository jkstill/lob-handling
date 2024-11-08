
Create hex dump

 xxd -p ../kitty-from-blob.jpg | tr -d '\n' > kitty.hex

Revert hex dump to binary

 xxd -r -p kitty.hex > kitty-from-hex.jpg

gcc -O2 -o kitty-C kitty-C.c

## Comparison


```
$  ./compare.sh

Running tests... 10240 iterations
================
Perl:

The C conversion was faster than the Perl conversion by 29.6%.

Conversion completed successfully.

real    1m23.498s
user    1m23.362s
sys     0m0.051s

C:

Conversion completed successfully.
real    0m59.395s
user    0m59.253s
sys     0m0.080s

Perl Inline:

real    0m9.428s
user    0m9.413s
sys     0m0.007s


```

