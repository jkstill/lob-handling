
# Read the hex file and convert it to a JPG file
def hex_to_jpg(hex_file_path, jpg_file_path):
   with open(hex_file_path, 'r') as hex_file:
      hex_data = hex_file.read()

   # Convert hex to binary
   binary_data = bytes.fromhex(hex_data)

   # Write binary data to a JPG file
   with open(jpg_file_path, 'wb') as jpg_file:
      jpg_file.write(binary_data)

# Example usage
hex_to_jpg('data/testdata.hex', 'data/python.dat')

