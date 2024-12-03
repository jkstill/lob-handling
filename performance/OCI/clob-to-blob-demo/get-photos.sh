#!/usr/bin/env bash


PERLEXE=$ORACLE_HOME/perl/bin/perl

[[ -x $PERLEXE ]] || {
	 PERLEXE=$(which perl)
}

export PERLEXE

photoList=$(mktemp)


$PERLEXE ./get-photo.pl --database $dest_oracle_ezName --username $dest_oracle_user --password $dest_oracle_password --list-photos > $photoList

mkdir -p photos

while read photo
do
	echo $photo
	$PERLEXE ./get-photo.pl --database $dest_oracle_ezName --username $dest_oracle_user --password $dest_oracle_password --photo-name $photo 
done < <(head -16 $photoList | awk '{ print $4 }' )

rm -f $photoList



