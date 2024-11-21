


logDir='logs';

mkdir -p $logDir

logFile=$logDir/clob-to-blob-$(date +%Y-%m-%d_%H-%M-%S).log

# Redirect stdout ( > ) into a named pipe ( >() ) running "tee"
# process substitution
# clear/recreate the logfile
> $logFile
exec 1> >(tee -ia $logFile)
exec 2> >(tee -ia $logFile >&2)


banner () {
	 echo
	 echo "#################################################"
	 echo "# $@"
	 echo "#################################################"
	 echo
}


banner "CLOB to BLOB conversion with PL/SQL function"
./reset.sh
./clob-to-blob-func.sh hex_to_blob

banner "CLOB to BLOB conversion with inline function"
./reset.sh
./clob-to-blob-inline.sh

banner "CLOB to BLOB conversion with perl pack() function"
./reset.sh
./clob-to-blob.sh

