#!/usr/bin/env perl

use warnings;
use strict;

use Encode qw( encode );


binmode *STDIN; #, ':raw';
binmode *STDOUT; #, ':raw';

my $hex='';

while (<STDIN>) {
	$hex .= uc unpack 'H*', encode 'UTF-8',$_;
	#tr/A-Fa-f0-9//dc;
	#tr/A-Fa-f0-9//c;
	#print unpack "H*";
}

print $hex;

