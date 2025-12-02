

# Read the hex file and convert it to a JPG file
def jpg_to_hex(jpg_file_path, hex_file_path):
   with open(jpg_file_path, 'rb') as jpg_file:
      bin_data = jpg_file.read()

   # Convert hex to binary
   hex_data = bytes.hex(bin_data)

   # Write binary data to a JPG file
   with open(hex_file_path, 'w') as hex_file:
      hex_file.write(hex_data)

# Example usage
jpg_to_hex('images/kinglet-02.jpg', 'images/kinglet-02.hex')


