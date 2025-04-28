Generate JPG files from test data
=================================

The `get-photo.pl` script is used to retrieve and generate JPG files from a database
where the BLOB data was generated from CLOB data.

The CLOB date was created from original JPG files using `insert-photo.pl`. (found elsewhere)

This script is just a sanity check.

The CLOB and BLOB are both used to create JPG files.

Teh BLOB data was converted from CLOB data, and the is the one we are most interested in.

### get-photos.sh

Run this to generate JPG files from the BLOBDEST2 table.

## Sample Commanmds

```text
./get-photo.pl --database $dest_oracle_ezName --username $dest_oracle_user --password $dest_oracle_password --list-photos
./get-photo.pl --database $dest_oracle_ezName --username $dest_oracle_user --password $dest_oracle_password --list-photos
./get-photo.pl --database $dest_oracle_ezName --username $dest_oracle_user --password $dest_oracle_password --list-photos | head -16
./get-photo.pl  --database lestrade2/pdb1.jks.com --username jkstill --password grok
./get-photo.pl --database lestrade2/pdb1.jks.com --username jkstill --password grok
./get-photo.pl --database lestrade2/pdb1.jks.com --username jkstill --password grok > /mnt/zips/tmp/oracle/blobtest/dv.jpg
./get-photo.pl  --database lestrade/pdb01 --username jkstill --password grok
./get-photo.pl --database lestrade/pdb01 --username jkstill --password grok
./get-photo.pl --database lestrade/pdb01 --username jkstill --password grok --list-photos
./get-photo.pl --database lestrade/pdb01 --username jkstill --password grok --photo-name  cat-on-a-hot-sunroof-01.jpg
./get-photo.pl --database lestrade/pdb01 --username jkstill --password grok --photo-name  spider-small-01.jpg
./get-photo.pl  --database lestrade/pdb01 --username jkstill --password grok > test-kitty.jpg
./get-photo.pl  --database ora192rac-scan/pdb1.jks.com --username jkstill --password grok
./get-photo.pl --database ora192rac-scan/pdb1.jks.com  --username jkstill --password grok
./get-photo.pl --database ora192rac-scan/pdb1.jks.com --username jkstill --password grok
./get-photo.pl --database ora192rac-scan/pdb1.jks.com --username jkstill --password grok
./get-photo.pl --username jkstill --database lestrade/pdb01 --password grok
./get-photo.pl --username jkstill --database ora192rac02/pdb1.jks.com --password grok
./get-photo.pl  --username jkstill --password grok --database 'ora192rac-scan/pdb1.jks.com'
```



