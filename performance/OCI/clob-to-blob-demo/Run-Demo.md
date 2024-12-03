

# Run Demo

## Prepare the Environment

This demo should be run on the Oracle Database server to avoid network latency.

Set the local oracle environment as per usual.

Edit the `demo-setenv.sh` script to set the correct values for your environment.

Then run the script to set the environment variables.

`. ./demo-setenv.sh`

## Create a User

Assume the user is blobtest.

Also assume that LOBs are stored in the LOBS tablespace.

Adjust accordingly if the LOBs are stored in a different tablespace.

```sql
create user blobtest identified by blobtest default tablespace users temporary tablespace temp;
grant connect, resource to blobtest;
alter user blobtest quota unlimited on users;
alter user blobtest quota unlimited on lobs;
```

## Create and Load The BLOBDEST Table

It is assumed that LOBs are stored in the LOBS tablespace.

Adjust the `create/create-blobdest.sql` script if the LOBs are stored in a different tablespace.

Now run the `sqlldr.sh` script to create the table and load it with data.

```text
$ ./sqlldr.sh

Table dropped.


Table created.


Index created.


SQL*Loader: Release 21.0.0.0.0 - Production on Tue Dec 3 14:30:40 2024
Version 21.15.0.0.0

Copyright (c) 1982, 2021, Oracle and/or its affiliates.  All rights reserved.

Path used:      Direct

Load completed - logical record count 400.

Table BLOBDEST:
  400 Rows successfully loaded.

Check the log file:
  blobdest.log
for more information about the load.

real    0m10.382s
user    0m8.721s
sys     0m0.797s

```

## Populate the Control Table

Run  `set-blogdest_rows.sh` script to create the table used to lookup the blob id.
(This table is not really necessary, but it is used in the demo to simulate what is used in production.)

`./set-blobdest_rows.sh`

The demo uses the blobdest_rows table for lookup of the blob id to be used in the update statement.

This table is created and populated in the `set-blobdest-rows.sh` script.


## Run the Demo

Please report the timing data from this step.

Run the 'clob-to-blob.sh' script to run the demo.

```text
$ ./clob-to-blob.sh
starting

400 row(s) updated

real    0m10.161s
user    0m3.622s
sys     0m0.932s
```

## Validate the data

Run the `./get-lob-lengths.sh` script to validate the data.

```text
$ ./get-lob-lengths.sh

...
        ID CLOB_LENGTH BLOB_LENGTH CLOB_CONVERT_ST NAME
---------- ----------- ----------- --------------- --------------------------------------------------
       389   8,741,322   4,370,661 OK              junco-nest-ladder-03-25.jpg
       390   2,340,078   1,170,039 OK              kinglet-01-25.jpg
       391   4,385,712   2,192,856 OK              kinglet-02-25.jpg
       392   2,900,254   1,450,127 OK              spider-25.jpg
       393     248,772     124,386 OK              cat-on-a-hot-sunroof-small-25.jpg
       394     355,186     177,593 OK              junco-nest-frog-01-small-25.jpg
       395     234,650     117,325 OK              junco-nest-ladder-01-small-25.jpg
       396     402,464     201,232 OK              junco-nest-ladder-02-small-25.jpg
       397     319,762     159,881 OK              junco-nest-ladder-03-small-25.jpg
       398     125,094      62,547 OK              kinglet-01-small-25.jpg
       399     177,014      88,507 OK              kinglet-02-small-25.jpg
       400     128,556      64,278 OK              spider-small-25.jpg

400 rows selected.
```

## Dump some photos

The test data consists of photos of animals.

There are several (16) photos.  There are 400 rows in the table due to duplication of the photos.

The `get-photos.sh` script will dump 16 photos.  Each is dumped twice, once from the CLOB data, once as from the BLOB data.

```text
$   ./get-photos.sh
cat-on-a-hot-sunroof-01.jpg
 raw len: 1090095
clob len: 2180190
blob len: 1090095
junco-nest-frog-01-01.jpg
 raw len: 4470089
clob len: 8940178
blob len: 4470089
...
spider-small-01.jpg
 raw len: 64278
clob len: 128556
blob len: 64278
```

Just copy the photos directory somewhere that the photos can be viewed.

If all appear to be valid, then the demo has been successful.




