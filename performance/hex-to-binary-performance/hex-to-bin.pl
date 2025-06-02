#!/usr/bin/env perl

# convert hex string to binary
# usage: hex-to-bin.pl <file>
# this is just a POC script to read a file containing hex characters
# and convert them to binary
# it is necessary to read and convert 2 hex characters at a time

use strict;
use warnings;
use IO::File;

use Data::Dumper;

my $file = shift @ARGV or die "Usage: $0 <file>\n";

my $io = IO::File->new($file, 'r') or die "Cannot open file '$file': $!\n";

# read 1 byte at a time

# read 1 hex character at a time
my $c = '';
while ( $io->read($c,2) ) {
	# convert to binary
	print pack('H*',uc($c));
	#warn "$c";
}

#warn "\n";






