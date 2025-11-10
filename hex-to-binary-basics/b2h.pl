#!/usr/bin/env perl
# read 1 byte binary byte at a time from a file and output as hex
# usage: b2h.pl <binary file>
# example: b2h.pl myfile.bin
# output: hex representation of the binary file

use strict;
use warnings;

my $filename = shift or die "Usage: $0 <binary file>\n";

open(my $fh, '<:raw', $filename) or die "Could not open file '$filename' $!";

while (read($fh, my $byte, 1)) {
	 printf "%02X", ord($byte);
}


