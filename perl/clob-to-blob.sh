#!/usr/bin/env bash


./clob-to-blob-range.pl --database ora192rac-scan/pdb1.jks.com --username jkstill --password grok \
	--table blobtest_load \
	--clob b1_clob \
	--blob b1 \
	--create-temp
	

#: << 'COMMENT'

./clob-to-blob-range.pl --database ora192rac-scan/pdb1.jks.com --username jkstill --password grok \
	--table blobtest_load \
	--clob b1_clob \
	--blob b1 \
	--range-low 1 \
	--range-high 1000 \
	--commit-interval 100 \
	--report-interval 200

	
#COMMENT

