
CLOB to BLOB Conversion Performance Tests
=========================================

This project contains performance tests for the CLOB to BLOB conversion in the Oracle Database.

Currently the data is read in from sqlldr using direct path and stored in a CLOB column. 

The following line was inserted by CoPilot - interesting
-- The CLOB to BLOB conversion is done using the `DBMS_LOB.CONVERTTOBLOB` function.

There are multiple methods to test.

## Possible Methods

### Baseline - CLOB to BLOB

Originally this was in PL/SQL.  I have rewritten it in Perl to make it easier to divide the work among multiple threads.

This might be better written in Java.

### Method 1 - PL/SQL

Note: this cannot work easily with sqlldr.

sqlldr does not have native parallel processing.  

It  can use multiple files, and will create multiple processes, but it will not use multiple threads to process a single file.

The time required to break up the file into multiple files may be a bit much.

Troubleshooting errors, etc will be more difficult.

Use the `DBMS_LOB.CONVERTTOBLOB` function in PL/SQL.

A wrapper function is created to convert the CLOB to BLOB.

The PL/SQL function may look like this:

```sql
CREATE OR REPLACE FUNCTION clob_to_blob (p_clob IN CLOB) RETURN BLOB IS
  l_blob BLOB;
BEGIN
    DBMS_LOB.CREATETEMPORARY(l_blob, TRUE);
    DBMS_LOB.CONVERTTOBLOB(l_blob, p_clob, DBMS_LOB.LOBMAXSIZE);
    RETURN l_blob;
    END clob_to_blob;
    ``` 

Test if this can be used in a sqllldr control file.  

It will not be possible to use direct path, but if we can avoid the clob-to-blob conversion post-processing, it may be faster.


### Method 2 - Java

Use Java to convert the CLOB to BLOB.  

It may be faster than the Perl implementation, but I am not sure.

### Method 3 - use `DBMS_LOB.CONVERTTOBLOB` in Perl

Modify the perl to use the `DBMS_LOB.CONVERTTOBLOB` procedure.

Currently the Perl script reads the CLOB into a variable and converts it to to binary with  pack(), then saves to the database.

### Method to use

The most likely method to use is the PL/SQL wrapper function that calls `DBMS_LOB.CONVERTTOBLOB`.

It can be used from Perl.

## Testing

### Create a test tables

### Source Data

run the following script to create the source and destination tables.

`cd create`

`create.sql`

Load data into the source table.
The following script creates 100 rows with the same photo.

`./insert-photo.sh`


Now dump the table to a hex sqlldr file with dunldr.

`./dunldr.sh`


Copy the blobsource.par file to blobtest.par

Copy the blobsource.ctl file to blobtest.ctl

Edit the blobtest.par file to point to the blobtest.ctl file.

Edit the blobtest.ctl file to load data into the BLOBDEST table.

#### blobdest.par

```
userid = jkstill/XXX@server//db_service
control = blobdest.ctl
log = blobdest.log
bad = blobdest.bad
```

#### blobdest.ctl

```
options(direct=true,readsize=5000000)
load data
infile 'blobsource.dat'
   "str '<EORD>'"
truncate
into table BLOBDEST
fields terminated by '<EOFD>'
trailing nullcols
(
        ID,
        C1 CHAR(2500000)
)
```

## Test Results

For best results, the BLOBDEST table should be dropped and recreated before each test.

1000 rows loaded into BLOBSOURCE.
Each row has an ID and a BLOB column B1, with a 1M photo inserted into each row.


BLOBSOURCE was then dumped with `dunldr.sh` to create the data file for sqlldr.


The `sqlldr.sh` script was used to load the data into a CLOB in the BLOBDEST table.

### clob-to-blob.sh

The clob-to-blob.pl script reads the CLOB into a variable and converts it to to binary with  pack(), then saves to the database.

This calls `clob-to-blob-range.pl` twice:

The first time truncates the table and creates the temp table used to track rowids.
(The temp table step is not really necessary for testing, but I did not want to rewrite the script)


RAC 19.12:
```
$  ./clob-to-blob.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.147s
user    0m0.037s
sys     0m0.009s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    0m46.384s
user    0m17.615s
sys     0m5.747s

```

Standalone 21.3:
```
$  ./clob-to-blob.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.108s
user    0m0.025s
sys     0m0.019s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    0m56.130s
user    0m19.557s
sys     0m6.198s
```

### clob-to-blob-func.sh

The clob-to-blob-func.pl script uses the clob2blob function to convert the CLOB to BLOB.

RAC:
```
$  ./clob-to-blob-func.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.163s
user    0m0.047s
sys     0m0.000s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    1m5.765s
user    0m9.062s
sys     0m4.513s

```


Standalone:
```
$  ./clob-to-blob-func.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.095s
user    0m0.039s
sys     0m0.004s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    0m47.887s
user    0m10.398s
sys     0m4.799s
```

These results are mixed, and a little disappointing.

The clob2blob function when used in the 21c database was abouit 17% faster than the original method.

In the RAC database, the clob2blob function was about 16% slower than the original method.

The best results processed 33 rows per second.

In the production database, the best results processed 200 rows per second.


## Conclusion

Run this test in the clients database to see which method is best.







