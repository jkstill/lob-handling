#!/usr/bin/env python3

import time


# Read the hex file and convert it to a JPG file
def get_hex_data(hex_file_path):
   with open(hex_file_path, 'r') as hex_file:
      hex_data = hex_file.read()
      return hex_data

def hex_to_jpg(hex_data):
   # Convert hex to binary
   binary_data = bytes.fromhex(hex_data)
   return binary_data


def main():
   hex_data = get_hex_data('data/testdata.hex')

   start = time.time()

   # convert hex to binary
   # loop 100 times
   for i in range(100):
      print('.', end='')
      binary_data = hex_to_jpg(hex_data)

   print()

   end = time.time()
   print(end - start)

   # Write binary data to a JPG file
   with open('data/testdata-py.bin', 'wb') as jpg_file:
      jpg_file.write(binary_data)


if __name__ == '__main__':
  main()


