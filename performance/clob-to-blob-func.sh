#!/usr/bin/env bash

echo 
echo create the TEMP table
echo 


usage () {
	cat <<EOF

valid function names

PL/SQL: clob2blob - wrapper for dbms_lob.converttoblob
        create with create/clob2blob-func.sql

  Note: this is a wrapper to dbms_lob.converttoblob
		  and does not convert the CLOB to a binary BLOB.
		  Doing so would require additional code.
		  As it is not any faster than the hex_to_blob function, or Perl pack(), there is no point in extending it.

PL/SQL: hex_to_blob -  reads clob to build blob
        create with create/hex2blob.sql
  
  Note: this is the only one of these functions that will convert the CLOB to a binary BLOB

Java: hex_to_blob_java - pass clob return blob

      Create with:
		  create/java/HexConverter-java.sql
		  create/java/hex-to-blob.sql

  Note: while this does work, it is significantly slower than both the PL/SQL hex_to_blob function, and the Perl pack() function.

EOF
}


declare functionName=$1

: ${functionName:?'You must provide a function name'}

case $functionName in
	clob2blob|hex_to_blob|hex_to_blob_java|clob_to_blob_java)
		;;
	*)
		echo "Invalid function name: $functionName"
		usage
		exit 1
		;;
esac


source ./setenv.sh


time ./clob-to-blob-range.pl --database $ezName --username jkstill --password grok \
	--use-function \
	--function-name $functionName \
	--table blobdest \
	--clob c1 \
	--blob b1 \
	--create-temp
	

#: << 'COMMENT'

echo 
echo Run the conversion
echo 

time ./clob-to-blob-range.pl --database $ezName --username jkstill --password grok \
	--use-function \
	--function-name $functionName \
	--table blobdest \
	--clob c1 \
	--blob b1 \
	--range-low 1 \
	--range-high 1000 \
	--commit-interval 100 \
	--report-interval 200

	
#COMMENT

