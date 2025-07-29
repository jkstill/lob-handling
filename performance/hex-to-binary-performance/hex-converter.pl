#!/usr/bin/env perl
#
use strict;
use warnings;
use Data::HexConverter qw(hex_to_binary binary_to_hex);
use Time::HiRes qw(time);

use IO::File;

my $iterations = 100;

my $hex_file = 'data/testdata.hex';
my $bin_file = 'data/perl-hex-converter.dat';

my $hex_fh = IO::File->new($hex_file, 'r') or die "Can't open $hex_file: $!";
my $bin_fh = IO::File->new($bin_file, 'w') or die "Can't open $bin_file: $!";
binmode $bin_fh;

my $hex_data;
while (my $line = $hex_fh->getline) {
	 chomp $line;
	 $line =~ s/^\s+//;
	 $line =~ s/\s+$//;
	 next if $line =~ /^#/;
	 $hex_data .= $line;
}

my $start_time = Time::HiRes::gettimeofday();

$| = 1;

my $bin_data;
for (my $i = 0; $i < $iterations; $i++) {
	#print '.';
	$bin_data = hex_to_binary(\$hex_data);
}

print "\n";

my $stop_time = Time::HiRes::gettimeofday();
printf("Elapsed: %.6f\n", $stop_time - $start_time);
printf("    Avg: %.6f\n", ($stop_time - $start_time)/$iterations);

$bin_fh->print($bin_data);



