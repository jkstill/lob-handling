#!/usr/bin/env bash

# dump the first 1M of a bcp file to head
# dd is used because sed will read the entire file - it is only 1 line per file

file=$1
lineCount=$2
: ${lineCount:=20}

delimiter=$3
: ${delimiter:=','}

#dd if=./flsplm_flsplm/data/flsplm_flsplm.WTPartUsageLink.dat bs=1M count=1 |  sed -r -e  's/<EORD>/\n/g'  -e 's/<EOFD>/,/g' | head
dd if="$1" bs=5M count=1 2>/dev/null |  sed -r -e  's/<EORD>/\n/g'  -e "s/<EOFD>/${delimiter}/g" | head -$lineCount

