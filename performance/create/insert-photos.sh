#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

echo home: $scriptHome

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

source ./setenv.sh

#./insert-photo.pl --insert-as-clob --truncate --database $ezName --username jkstill --password grok --input-file $kittyPic

#for imageNumber in '01'
for imageNumber in $(seq -w 1 25)
do

	for imageFile in ../images/*.jpg ../images/small/*.jpg
	do
		imageName="$(basename $imageFile | cut -f1 -d\.)-${imageNumber}.jpg"
		./insert-photo.pl --database $ezName --username blobtest --password blobtest -insert-as-clob --input-file $imageFile --image-name $imageName
	done
done


