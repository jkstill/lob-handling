#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

echo home: $scriptHome

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }


kittyPic=../images/cat-on-a-hot-sunroof.jpg

source ./setenv.sh

#./insert-photo.pl --insert-as-clob --truncate --database $ezName --username jkstill --password grok --input-file $kittyPic
./insert-photo.pl --truncate --database $ezName --username jkstill --password grok --input-file $kittyPic

for i in {1..999}
do
	./insert-photo.pl --database $ezName --username jkstill --password grok --input-file $kittyPic
done


