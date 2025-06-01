#!/usr/bin/env bash

input_file="$1"
# make the thumbnail file name match the input file name, but with '_thumb' following the name, but before the extension
output_file="${input_file%.*}_thumb.${input_file##*.}"

ffmpeg -i $input_file \
  -vf "scale=20:-1" \
  -q:v 2 \
  $output_file


