#!/usr/bin/env bash

echo 
echo create the TEMP table
echo 

source ./setenv.sh

time ./clob-to-blob-range.pl --database $ezName --username jkstill --password grok \
	--table blobdest \
	--clob c1 \
	--blob b1 \
	--create-temp
	

#: << 'COMMENT'

echo 
echo Run the conversion
echo 

time ./clob-to-blob-range.pl --database $ezName --username jkstill --password grok \
	--table blobdest \
	--clob c1 \
	--blob b1 \
	--range-low 1 \
	--range-high 1000 \
	--commit-interval 100 \
	--report-interval 200

	
#COMMENT

