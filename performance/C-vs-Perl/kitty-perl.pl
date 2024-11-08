#!/usr/bin/perl

use strict;
use warnings;

# Input and output file paths
my $input_file  = 'kitty.hex';
my $output_file = 'kitty-perl.jpg';

# Read the hex data from the input file
open(my $in_fh, '<', $input_file) or die "Cannot open $input_file: $!";
local $/;  # Enable 'slurp' mode to read the entire file
my $hex_data = <$in_fh>;
close($in_fh);

# Remove any whitespace (optional, if your hex data contains spaces or newlines)
#$hex_data =~ s/\s+//g;

# Convert hex data to binary data
my $binary_data;

for ( my $i = 0; $i < 10240; $i++ ) {
	$binary_data = pack("H*", $hex_data);
}


# Write the binary data to the output file
open(my $out_fh, '>', $output_file) or die "Cannot open $output_file: $!";
binmode($out_fh);  # Ensure we're writing binary data
print $out_fh $binary_data;
close($out_fh);

print "Conversion completed successfully.\n";

