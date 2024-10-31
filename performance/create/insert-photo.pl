#!/usr/bin/env perl

use warnings;
use strict;
use FileHandle;
use DBI;
use DBD::Oracle qw(:ora_types);
use Getopt::Long;
use Data::Dumper;
use IO::File;

my %optctl = ();

my($db, $username, $password);
my ($help, $sysdba, $connectionMode, $localSysdba, $sysOper) = (0,0,0,0,0);
my $inputFile = '';
my $truncate = 0;
my $insertAsCLOB = 0;

Getopt::Long::GetOptions(
	\%optctl,
	"database=s"	=> \$db,
	"username=s"	=> \$username,
	"password=s"	=> \$password,
	"input-file=s"	=> \$inputFile,
	"insert-as-clob!"	=> \$insertAsCLOB,
	"truncate!"		=> \$truncate,
	"sysdba!"		=> \$sysdba,
	"local-sysdba!"=> \$localSysdba,
	"sysoper!"		=> \$sysOper,
	"z|h|help"		=> \$help
);

usage(0) if $help;

if (! $localSysdba) {

	$connectionMode = 0;
	if ( $sysOper ) { $connectionMode = 4 }
	if ( $sysdba ) { $connectionMode = 2 }

	usage(1) unless ($db and $username and $password);
}

die "cannot read $inputFile - $!\n" unless -r $inputFile;

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
			ora_session_mode => $connectionMode
		}
	);
}

die "Connect to  $db failed \n" unless $dbh;
$dbh->{RowCacheSize} = 100;
$dbh->{LongReadLen} = 4 * 2**30;


$dbh->do('truncate table blobsource') if $truncate;

my $rawInputFH = IO::File->new();

$rawInputFH->open($inputFile,'r');
$rawInputFH->binmode(':raw');

my $photo;

while ( my $rawData = <$rawInputFH> ) {
	$photo .= $rawData;
}

print "len: " . length($photo) . "\n";

# for insert as clob
#print "hex: $hex";

my $sql = q{select decode(max(id),null,0,max(id)) id from blobsource};
my $sth = $dbh->prepare($sql);
$sth->execute();

my ($id) = $sth->fetchrow_array();
$sth->finish;
$id++;

print "ID: $id\n";

$sql=q{insert into blobsource(id, b1) values(?,?)};
$sth = $dbh->prepare($sql,{ora_check_sql => 0});
$sth->bind_param(1,$id);
if ($insertAsCLOB) {
	my $hex = uc unpack 'H*', $photo;
	print "len: " . length($hex) . "\n";
	$sth->bind_param(2,$hex);
} else {
	$sth->bind_param(2,$photo,{ora_type=>SQLT_BIN});
}
$sth->execute();
$sth->finish;

$dbh->commit;
$dbh->disconnect;

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



