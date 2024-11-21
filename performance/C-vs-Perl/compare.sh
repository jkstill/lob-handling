#!/usr/bin/env bash



logDir='logs';

mkdir -p $logDir

logFile=$logDir/kitty-compare-$(date +%Y-%m-%d_%H-%M-%S).log

# Redirect stdout ( > ) into a named pipe ( >() ) running "tee"
# process substitution
# clear/recreate the logfile
> $logFile
exec 1> >(tee -ia $logFile)
exec 2> >(tee -ia $logFile >&2)


echo
echo "Running tests... 10240 iterations"
echo "================"
echo "Perl:"
echo
time ./kitty-perl.pl

echo "Perl Inline:"
time ./kitty-perl-inline.pl

echo
echo "C"
echo t
time ./kitty-C
echo

echo "Go:"
time ./kitty-go
echo


