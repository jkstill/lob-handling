
Create hex dump

 xxd -p ../kitty-from-blob.jpg | tr -d '\n' > kitty.hex

Revert hex dump to binary

 xxd -r -p kitty.hex > kitty-from-hex.jpg

Get the first 32 bytes

 xxd -p kitty-from-clob.jpg | tr -d '\n'  |  perl -p -e '$_=substr($_,0,32)'

Compile 

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

