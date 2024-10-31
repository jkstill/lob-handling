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

$dbh->do('truncate table blobtest');

my $jpeg = 'death-valley.jpg';

-r $jpeg || die "could not find $jpeg - $!\n";

my $rawInputFH = IO::File->new();

$rawInputFH->open($jpeg,'r');
$rawInputFH->binmode(':raw');

my $photo;

while ( my $rawData = <$rawInputFH> ) {
	$photo .= $rawData;
}

print "len: " . length($photo) . "\n";

my $hex = uc unpack 'H*', $photo;
print "  hex len: " . length($hex) . "\n";
print "photo len: " . length($photo) . "\n";
#print "hex: $hex";

my $sql=q{insert into blobtest(id, c1) values(?,?)};
my $sth = $dbh->prepare($sql,{ora_check_sql => 0});
$sth->bind_param(1,1);
#$sth->bind_param(2,$hex,{ora_type=>SQLT_CHR});
$sth->bind_param(2,$hex,{ora_type=>ORA_CLOB});
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



