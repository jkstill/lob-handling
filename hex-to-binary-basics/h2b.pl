#!/usr/bin/env perl
# read hex data from a file and convert it to binary data
# usage: h2b.pl <input file> <output file>
# example: h2b.pl input.hex output.bin
# the input file should contain hex data in the format:
# 48656c6c6f20576f726c6421
# which represents the string "Hello World!"
use strict;
use warnings;

my ($inputFile, $outputFile) = @ARGV;
if (not defined $inputFile or not defined $outputFile) {
	 die "Usage: $0 <input file> <output file>\n";
 }

open my $in, '<', $inputFile or die "Cannot open '$inputFile': $!\n";
open my $out, '>:raw', $outputFile or die "Cannot open '$outputFile': $!\n";
# not using pack - make this as simple as possible

# read 2 characters at a time (equivalent to 1 byte)
# and convert to binary

while( 1 ) {
	my $hexByte;
	my $readBytes = read($in, $hexByte, 2);
	last if $readBytes == 0; # end of file
	if ($readBytes < 2) {
		die "Incomplete hex byte at end of file\n";
	}
	# convert hex to binary
	# do not use pack for this
	my $binaryData = chr( hex($hexByte) );
	# write binary data to output file
	print $out $binaryData;
}


