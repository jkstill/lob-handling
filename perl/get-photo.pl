#!/usr/bin/env perl

use warnings;
use strict;
use FileHandle;
use DBI;
use Getopt::Long;
use Data::Dumper;
use IO::File;

my %optctl = ();

my($db, $username, $password);
my ($help, $sysdba, $connectionMode, $localSysdba, $sysOper) = (0,0,0,0,0);

Getopt::Long::GetOptions(
	\%optctl,
	"database=s"	=> \$db,
	"username=s"	=> \$username,
	"password=s"	=> \$password,
	"sysdba!"		=> \$sysdba,
	"local-sysdba!"=> \$localSysdba,
	"sysoper!"		=> \$sysOper,
	"z|h|help"		=> \$help
);

if (! $localSysdba) {

	$connectionMode = 0;
	if ( $sysOper ) { $connectionMode = 4 }
	if ( $sysdba ) { $connectionMode = 2 }

	usage(1) unless ($db and $username and $password);
}


#print qq{
#
#USERNAME: $username
#DATABASE: $db
#PASSWORD: $password
    #MODE: $connectionMode
 #RPT LVL: @rptLevels
#};
#exit;


$|=1; # flush output immediately

my $longReadLen  = 20 * 2**20;
my $oraPieceSize = 5 * 2**20;

my $dbh ;

if ($localSysdba) {
	$dbh = DBI->connect(
		'dbi:Oracle:',undef,undef,
		{
			RaiseError => 1,
			AutoCommit => 0,
			ora_session_mode => 2
		}
	);
} else {
	$dbh = DBI->connect(
		'dbi:Oracle:' . $db,
		$username, $password,
		{
			RaiseError => 1,
			AutoCommit => 0,
			ora_session_mode => $connectionMode,
			LongReadLen => $longReadLen,
		}
	);
}

die "Connect to  $db failed \n" unless $dbh;
$dbh->{RowCacheSize} = 100;
#$dbh->{LongTruncOk}=1;

print "LongReadLen: $dbh->{LongReadLen}\n";
#$dbh->disconnect;
#exit;

my $jpegBlob = 'death-valley-from-blob.jpg';
my $jpegClob = 'death-valley-from-clob.jpg';

my $sql=q{select id, c1, b1 from blobtest_load where id = ?};

#my $sth = $dbh->prepare($sql,{ ora_pers_lob=>1,ora_check_sql => 0});
my $sth = $dbh->prepare($sql, {ora_piece_lob=>1,ora_piece_size=> $oraPieceSize});

$sth->execute(2);

my ($id, $clob, $blob) = $sth->fetchrow_array();

$sth->finish;
$dbh->disconnect;

my $raw = pack"H*",$clob;

print " raw len: " . length($raw) . "\n";
print "clob len: " . length($clob) . "\n";
print "blob len: " . length($blob) . "\n";

my $rawOutputFH = IO::File->new();
$rawOutputFH->open($jpegBlob,'w');
binmode $rawOutputFH, ':raw';
$rawOutputFH->print($blob);

$rawOutputFH = IO::File->new();
$rawOutputFH->open($jpegClob,'w');
binmode $rawOutputFH, ':raw';
$rawOutputFH->print($raw);


sub usage {
	my $exitVal = shift;
	$exitVal = 0 unless defined $exitVal;
	use File::Basename;
	my $basename = basename($0);
	print qq/

usage: $basename

  -database      target instance
  -username      target instance account name
  -password      target instance account password
  -sysdba        logon as sysdba
  -sysoper       logon as sysoper
  -local-sysdba  logon to local instance as sysdba. ORACLE_SID must be set
                 the following options will be ignored:
                   -database
                   -username
                   -password

  example:

  $basename -database dv07 -username scott -password tiger -sysdba  

  $basename -local-sysdba 

/;
   exit $exitVal;
};



