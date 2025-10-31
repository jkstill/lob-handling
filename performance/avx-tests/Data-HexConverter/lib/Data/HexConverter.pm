package Data::HexConverter;

use 5.008008;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

our @EXPORT_OK = qw(
    hex_to_bytes
    bytes_to_hex
    hexsimd_hex2bin_impl_name
    hexsimd_bin2hex_impl_name
);

our $VERSION = '0.01';

require XSLoader;
XSLoader::load('Data::HexConverter', $VERSION);

1;
__END__

=head1 NAME

Data::HexConverter - Perl extension for fast hex/binary conversion

=head1 SYNOPSIS

  use Data::HexConverter qw(hex_to_bytes bytes_to_hex);

  my $bytes = hex_to_bytes("DEADBEEF");
  my $hex = bytes_to_hex($bytes);

=head1 DESCRIPTION

This module provides a simple and fast way to convert between hexadecimal and binary data.

=head1 AUTHOR

Jared Still, E<lt>jkstill@gmail.com<gt>

=head1 SEE ALSO

L<perl(1)>.

=cut
