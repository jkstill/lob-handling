#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

source ./setenv.sh

dunldr \
	--longlen 20000000 \
	--username $source_oracle_user \
	--password $source_oracle_password \
	--database $source_oracle_ezName \
	--owner jkstill \
	--table blobdest \
	--file-suffix '.dat' \
	--load-as-clob \
	--load-direct \
	--record-prefix 'NA' \
	--record-suffix 'NA' \
	--field-delimiter '<EOFD>' \
	--record-delimiter '<EORD>'



