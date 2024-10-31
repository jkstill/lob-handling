#!/usr/bin/env bash

./dunldr --database lestrade/pdb01 --username jkstill --password grok \
	--owner jkstill --table blobtest --rowlimit 10 \
	--bincol blobtest=b1  \
	--longlen 2000000 \
	--record-prefix 'NA' --record-suffix 'NA' \
	--field-delimiter '<EOFD>' --record-delimiter '<EORD>'

