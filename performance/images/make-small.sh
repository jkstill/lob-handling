#!/usr/bin/env bash


cd /mnt/photos/images-proto

mkdir -p small

for f in *.jpg; do
	 echo "Processing $f"
	 name=$(basename $f | cut -f 1 -d '.')
	 echo " small: $name-small.jpg"
	 ffmpeg -i $f -vf scale="iw/2:ih/2" small/$name-small.jpg
done

