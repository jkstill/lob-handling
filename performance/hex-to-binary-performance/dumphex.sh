#!/usr/bin/env bash


for file in data/*.dat
do
	fileName=$(basename $file) 
	baseFile=$(echo $fileName | cut -d'.' -f1)	
	newFile="data/${baseFile}.hex"
	xxd -p $file | tr -d '\n' > $newFile
done

md5sum data/*.hex

