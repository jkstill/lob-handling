#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

echo home: $scriptHome

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

source ./setenv-atp.sh

#./insert-photo.pl --insert-as-clob --truncate --database $ezName --username jkstill --password grok --input-file $kittyPic

#for imageNumber in '01'
for imageNumber in $(seq -w 1 25)
do

	for imageFile in ../images/*.jpg ../images/small/*.jpg
	do
		imageName="$(basename $imageFile | cut -f1 -d\.)-${imageNumber}.jpg"
		# username and password are hardcoded in the script to blank.  this due to autologin wallet
		./insert-photo-atp.pl --database atp23ai01 --username dummy --password dummy -insert-as-clob --input-file $imageFile --image-name $imageName
	done
done


