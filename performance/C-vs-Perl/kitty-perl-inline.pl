#!/usr/bin/env perl
use strict;
use warnings;

use Inline C => <<'END_OF_C_CODE';

// Function to convert a single hex character to its numerical value
unsigned char hex_char_to_value(char c) {
    if (c >= '0' && c <= '9')
        return (unsigned char)(c - '0');
    else if (c >= 'a' && c <= 'f')
        return (unsigned char)(c - 'a' + 10);
    else if (c >= 'A' && c <= 'F')
        return (unsigned char)(c - 'A' + 10);
    else {
        croak("Invalid hex character: %c", c);
    }
}

// Function to convert hex string to binary data
void hex_to_binary(SV *hex_sv, SV *binary_sv) {
    STRLEN hex_length;
    const char *hex_data = SvPV(hex_sv, hex_length);

    // Remove whitespace from hex_data
    char *clean_hex_data = malloc(hex_length + 1);
    if (clean_hex_data == NULL) {
        croak("Memory allocation failed");
    }
    const char *hex_ptr = hex_data;
    char *clean_ptr = clean_hex_data;
    while (*hex_ptr) {
        if (!isspace((unsigned char)*hex_ptr)) {
            *clean_ptr++ = *hex_ptr;
        }
        hex_ptr++;
    }
    *clean_ptr = '\0';

    hex_length = clean_ptr - clean_hex_data;

    if (hex_length % 2 != 0) {
        free(clean_hex_data);
        croak("Hex data length is not even");
    }

    size_t binary_size = hex_length / 2;
    unsigned char *binary_data = (unsigned char *)SvGROW(binary_sv, binary_size + 1);

    // Convert hex to binary
    for (size_t i = 0; i < binary_size; i++) {
        unsigned char high_nibble = hex_char_to_value(clean_hex_data[2 * i]);
        unsigned char low_nibble = hex_char_to_value(clean_hex_data[2 * i + 1]);
        binary_data[i] = (high_nibble << 4) | low_nibble;
    }

    SvCUR_set(binary_sv, binary_size);
    SvSETMAGIC(binary_sv);
    SvPOK_on(binary_sv);

    free(clean_hex_data);
}

END_OF_C_CODE

# Input and output file paths
my $input_file  = 'kitty.hex';
my $output_file = 'kitty-inline.jpg';

# Read the hex data from the input file
open(my $in_fh, '<', $input_file) or die "Cannot open $input_file: $!";
local $/;  # Enable 'slurp' mode to read the entire file
my $hex_data = <$in_fh>;
close($in_fh);

# Create a scalar for binary data
my $binary_data = '';

# Convert hex data to binary data using the C function
my $iterations=0;
for (my $i = 0; $i < 1024; $i++) {
	hex_to_binary($hex_data, $binary_data);
	$iterations++;
}


# Write the binary data to the output file
open(my $out_fh, '>', $output_file) or die "Cannot open $output_file: $!";
binmode($out_fh);  # Ensure we're writing binary data
print $out_fh $binary_data;
close($out_fh);

print "Conversion completed successfully.\n";
