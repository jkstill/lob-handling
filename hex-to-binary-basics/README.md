
Simple demos to illustrate converting hex strings to binary data, and vice versa.


./b2h.pl kinglet-small.jpg > kinglet-small.hex
./h2b.pl kinglet-small.hex  kinglet-small-copy.jpg

if cmp kinglet-small.jpg kinglet-small-copy.jpg ; then echo "Files are identical"; else echo "Files differ"; fi


