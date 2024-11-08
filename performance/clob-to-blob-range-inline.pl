#!/usr/bin/env perl

use warnings;
use strict;
use FileHandle;
use DBI;
use DBD::Oracle qw(:ora_types);
use Getopt::Long;
use Data::Dumper;

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

my %optctl = ();

my($db, $username, $password);
my ($help, $sysdba, $connectionMode, $localSysdba, $sysOper) = (0,0,0,0,0);
my ($tableName, $blobColumn, $clobColumn);
my $createTemp = 0;
my ($rangeLow, $rangeHigh);
my $commitInterval=1000;
my $reportInterval=50000;
my $commitCounter=0;
my ($useFunction,$functionName)= (0,'');


Getopt::Long::GetOptions(
	\%optctl,
	"database=s"	=> \$db,
	"username=s"	=> \$username,
	"password=s"	=> \$password,
	"table=s"		=> \$tableName,
	"clob=s"			=> \$clobColumn,
	"blob=s"			=> \$blobColumn,
	"create-temp!" => \$createTemp,
	"range-low=i"  => \$rangeLow,
	"range-high=i" => \$rangeHigh,
	"use-function!" => \$useFunction,
	"function-name=s" => \$functionName,
	"commit-interval=i" => \$commitInterval,
	"report-interval=i" => \$reportInterval,
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


if ($useFunction) {
	if (!$functionName) {
		die "function-name must be specified when using --use-function\n";
	}
}

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

my $tempTable=q{MG_TMP_} . uc($tableName);

createTempTable() if $createTemp;

usage(1) unless defined($rangeLow) and defined($rangeHigh);

if ($rangeLow >= $rangeHigh ) { die usage(1); }

# get the rowids from the temp table - these are used to lookup the row in the target table
# cannot use GTT here, as multiple clients will use it
my $sql = qq{select id,r1 from $tempTable where id between $rangeLow and $rangeHigh};

my $rowidSTH = $dbh->prepare($sql);
$rowidSTH->execute();

# get the target table row
$sql=qq{select rowid, $clobColumn, nvl(length($blobColumn),0) blob_length from $tableName where rowid = ?};
print "CLOB SQL: $sql\n";

my $sth = $dbh->prepare($sql, {ora_piece_lob=>1,ora_piece_size=> $oraPieceSize});

my $blobUpdateSQL='';

# must first build hex_to_blob function with hex_to_blob.sql
if ($useFunction) {
	if ($functionName eq 'clob_to_blob_java' ) {
		$blobUpdateSQL=qq{begin $functionName(?,?,?,?); end;};
	} else {
		$blobUpdateSQL=qq{update $tableName set $blobColumn = ${functionName}($clobColumn)  where rowid = ?};
	}
} else {
   $blobUpdateSQL=qq{update $tableName set $blobColumn = ? where rowid = ?};
}

print "BLOB SQL: $blobUpdateSQL\n";

my $updateSTH = $dbh->prepare($blobUpdateSQL, {ora_piece_lob=>1,ora_piece_size=> $oraPieceSize});

# loop through rowids from the TEMP table
while ( my ($id,$rowid) = $rowidSTH->fetchrow_array() ) {

	#print "rowid: $rowid\n";
	$sth->execute($rowid);
	my ($id, $clob,$blobLength) = $sth->fetchrow_array();
	next if $blobLength;  # already converted - may have stopped and run again
	next unless $clob; # CLOB is empty

	if ($useFunction) {
		if ($functionName eq 'clob_to_blob_java') {
			$updateSTH->bind_param(1,$tableName,{ora_type=>SQLT_CHR});
			$updateSTH->bind_param(2,$clobColumn,{ora_type=>SQLT_CHR});
			$updateSTH->bind_param(3,$blobColumn,{ora_type=>SQLT_CHR});
			$updateSTH->bind_param(4,$rowid,{ora_type=>SQLT_CHR});
		} else {
			$updateSTH->bind_param(1,$rowid,{ora_type=>SQLT_CHR});
		}
	} else {
		#my $raw = pack"H*",$clob;
		my $raw = '';
		eval {
			hex_to_binary($clob, $raw);
		};
		if ($@) {
			warn "Error converting hex to binary: $@\n";
			warn "rowid: $rowid\n";
			warn "id: $id\n";
			next;
		}
		#print " raw len: " . length($raw) . "\n";
		$updateSTH->bind_param(1,$raw,{ora_type=>ORA_BLOB});
		$updateSTH->bind_param(2,$rowid,{ora_type=>SQLT_CHR});
	}

	eval {
		local $dbh->{RaiseError} = 1;
		local $dbh->{PrintError} = 0;
		$updateSTH->execute();
	};

	if ($@) {
		my($err,$errStr) = ($dbh->err, $dbh->errstr);
		warn "query died - $err - $errStr\n";
		warn "rowid: $rowid\n";
		warn "id: $id\n";
		warn "blobLength: $blobLength\n";
		warn "updateSQL: $blobUpdateSQL\n";
		warn "useFunction: $useFunction\n";
		next;
	}

	#$dbh->do(qq);

	$commitCounter++;
	unless ( $commitCounter%$commitInterval) { 
		$dbh->commit; 
	}

	unless ( $commitCounter%$reportInterval) { 
		print "rows committed: $commitCounter\n";
	}
}

$dbh->commit; 
$sth->finish;
$dbh->disconnect;


sub usage {
	my $exitVal = shift;
	$exitVal = 0 unless defined $exitVal;
	use File::Basename;
	my $basename = basename($0);
	print qq/

usage: $basename

  --database          target instance
  --username          target instance account name
  --password          target instance account password
  --table             table name
  --clob              CLOB column name
  --blob              BLOB column name

  --create-temp       just create the temp table and exit
                      the temp table is truncated if it exists

  --range-low         ID from the temp table to start with
  --range-high        ID from the temp table to end with

  --use-function      use the clob2blob function to convert CLOB to BLOB
  --nouse-function    use Perl to read CLOB and write BLOB (default)
  --function-name     name of the function to use
                      Examples: clob2blob, hex_to_blob, hex_to_blob_java

  --commit-interval   number of rows process per COMMIT
  --report-interval   number of rows process per report

  --sysdba            logon as sysdba
  --sysoper           logon as sysoper
  --local-sysdba      logon to local instance as sysdba. ORACLE_SID must be set
                      the following options will be ignored:
                         --database
                         --username
                         --password
  example:

  $basename --username scott --password tiger --database oraserver\/pdb1.yourdomain.com --table blobtest --clob c1 --blob b1 --create-temp

  $basename --username scott --password tiger --database oraserver\/pdb1.yourdomain.com --table blobtest --clob c1 --blob b1 --low-range 10001 --high-range 20000 --report-interval 1000


/;
   exit $exitVal;
};

sub createTempTable {

	print "Building Temp Table $tempTable\n";

	eval {
   	local $dbh->{RaiseError} = 1;
   	local $dbh->{PrintError} = 0;
		$dbh->do(qq{create table $tempTable (id number, r1 rowid, primary key(id))  organization index pctfree 1 initrans 20});
	};

	if ($@) {
   	my($err,$errStr) = ($dbh->err, $dbh->errstr);
   	if ($err == 955) {
      	; # ignore 'already exists'
   	} else {
      	die qq{query died - $err - $errStr\n};
   	}
	}


	$dbh->do(qq{truncate table $tempTable});
	$dbh->do(qq{insert /*+ append */ into $tempTable select rownum, rowid from $tableName where $blobColumn is null and $clobColumn is not null});
	$dbh->commit;

	print "Completed Temp Table $tempTable\n";
	$dbh->disconnect;
	exit;
}


