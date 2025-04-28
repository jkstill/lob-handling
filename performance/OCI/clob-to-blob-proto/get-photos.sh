#!/usr/bin/env bash

source ./setenv.sh

PERLEXE=$ORACLE_HOME/perl/bin/perl

[[ -x $PERLEXE ]] || {
	 PERLEXE=$(which perl)
}

export PERLEXE

photoList=$(mktemp)

$PERLEXE ./get-photo.pl --database $dest_oracle_ezName --username $dest_oracle_user --password $dest_oracle_password --list-photos > $photoList

#echo $photoList
#exit

mkdir -p photos

while read photo
do
	echo $photo
	$PERLEXE ./get-photo.pl --database $dest_oracle_ezName --username $dest_oracle_user --password $dest_oracle_password --output-dir photos --photo-name $photo 
done < <(head -16 $photoList | awk '{ print $4 }' )

rm -f $photoList



