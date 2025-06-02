#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

echo home: $scriptHome

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

source ./setenv.sh

#./insert-photo.pl --insert-as-clob --truncate --database $ezName --username jkstill --password grok --input-file $kittyPic

for imageFile in ../images/thumb/*.jpg 
do
	#imageName="$(basename $imageFile | cut -f1 -d\.)-${imageNumber}.jpg"
	#imageName="$(basename $imageFile | cut -f1 -d\.).jpg"
	imageName="$(basename $imageFile)"
	echo "imageName: $imageName"
	./insert-photo.pl --database $ezName --username c2btest --password c2btest -insert-as-clob --input-file $imageFile --image-name $imageName
done


