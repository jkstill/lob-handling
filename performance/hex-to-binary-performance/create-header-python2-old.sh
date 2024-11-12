#!/home/jkstill/miniconda3/envs/oci/bin/python

import sys,struct

sys.stdout.write("unsigned char base16_decoding_table1[256] = {\n")

for i in xrange(256):
    try:
        j = str(int(chr(i), 16))
    except:
        j = '0'
    sys.stdout.write(j+',')
sys.stdout.write("};\n")

sys.stdout.write("\n")

l = 128*256*["0"]

for a in ['0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','A','B','C','D','E','F']:
    for b in ['0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','A','B','C','D','E','F']:
        l[struct.unpack("<H", a+b)[0]] = str(int(a+b, 16))

line = "unsigned char base16_decoding_table2[%d] = {"%(128*256)

for e in l:
    line += e+","
    if len(line) > 70:
        sys.stdout.write(line+"\n")
        line = ""
sys.stdout.write(line+"};\n")

sys.stdout.write("\n")

l = 256*256*["0"]

for a in ['0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','A','B','C','D','E','F']:
    for b in ['0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','A','B','C','D','E','F']:
        l[struct.unpack("<H", a+b)[0]] = str(int(a+b, 16))

line = "unsigned char base16_decoding_table3[%d] = {"%(256*256)

for e in l:
    line += e+","
    if len(line) > 70:
        sys.stdout.write(line+"\n")
        line = ""
sys.stdout.write(line+"};\n")
