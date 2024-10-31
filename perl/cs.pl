#!/usr/bin/env perl

use warnings;
use strict;
use open qw( :std :encoding(UTF-8) );

#use Encode qw( encode );

#binmode *STDIN; #, ':raw';
#binmode *STDOUT; #, ':raw';

for ( my $i=35; $i < 128; $i++ ) {
	print(chr($i));
}

for ( my $i=163; $i < 984; $i++ ) {
	print(chr($i));
}



