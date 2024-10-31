
use warnings;
use strict;

my $count=285_000_000;
$count=10_000;
my $increment=$count/10;

print "    count: $count\n";
print "increment: $increment\n";

foreach (my $i=1; $i<=10; $i++ ) {
	my $low =  ($increment*$i) - $increment + 1 ;
	my $high = ($increment*$i);

	#printf( "range low: %10.0f  high: %10.0f size: %12s\n" , $low, $high, commify((($high - $low) +1 )) );
	#print  " --low-range $low --high-range $high \n";
	#print "clob2blob-$i:./clob-to-blob-range.pl --username flsplm --password flsplm --database bplmdev --table WfAssignment --clob SQLDEVELOPER_CLOB_1 --blob assignee --range-low $low --range-high $high --report-interval 50000\n"
	print "blobtest-$i:./clob-to-blob-range.pl --username jkstill --password grok --database 'lestrade2/pdb1.jks.com' --table blobtest --clob c1 --blob b1 --range-low $low --range-high $high --report-interval 100\n"
}

sub commify {
	# wn - working number
	my $wn = shift;
	unless ( $wn =~ /^\-{0,1}[0-9]+\.{0,1}[0-9]*$/ ) { return $wn; }
	1 while $wn =~ s/^(-?\d+)(\d{3})/$1,$2/;
	return $wn;
}

