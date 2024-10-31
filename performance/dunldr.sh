#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

source ./setenv.sh

./dunldr \
	--longlen 2500000 \
	--username jkstill \
	--password grok \
	--database $ezName \
	--owner jkstill \
	--table blobsource \
	--file-suffix '.dat' \
	--bincol blobsource=b1 \
	--load-as-clob \
	--load-direct \
	--record-prefix 'NA' \
	--record-suffix 'NA' \
	--field-delimiter '<EOFD>' \
	--record-delimiter '<EORD>'



