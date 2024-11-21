
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

At first I thought the the PL/SQL function may look like this:

```sql
CREATE OR REPLACE FUNCTION clob_to_blob (p_clob IN CLOB) RETURN BLOB IS
  l_blob BLOB;
BEGIN
    DBMS_LOB.CREATETEMPORARY(l_blob, TRUE);
    DBMS_LOB.CONVERTTOBLOB(l_blob, p_clob, DBMS_LOB.LOBMAXSIZE);
    RETURN l_blob;
END clob_to_blob;
``` 

All that does however is convert the data type.  dbms_lob_converttoblob does not convert the data from hex to binary.

The actual function will look like this:

```sql
create or replace
function hex_to_blob (hex clob) return blob is
   b blob                 := null;
   s varchar2(32767 byte) := null;
   l number               := 8192;
begin

   if hex is not null then
      dbms_lob.createtemporary(b, true);

      for i in 0 .. length(Hex) / l loop
         dbms_lob.read(hex, l, i * l + 1, s);
         dbms_lob.append(b, to_blob(hextoraw(s)));
      end loop;

   end if;

   return b;

end hex_to_blob;
/

```

Test if this can be used in a sqllldr control file.  

It will not be possible to use direct path, but if we can avoid the clob-to-blob conversion post-processing, it may be faster.
Note: it is far too slow with conventional path.


### Method 2 - Java

Use Java to convert the CLOB to BLOB.  

It may be faster than the Perl implementation, but I am not sure.

### Method 3 - use `DBMS_LOB.CONVERTTOBLOB` in Perl

Modify the perl to use the `DBMS_LOB.CONVERTTOBLOB` procedure.

Currently the Perl script reads the CLOB into a variable and converts it to to binary with  pack(), then saves to the database.

### Method 4 - use C 

Use C to convert the CLOB to BLOB.
see blob.c

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

#### Create the hex_to_blob function

The hex_to_blob function is used to convert the hex data to binary.

```sql
@create/hex_to_blob.sql
```

#### Create the clob2clob function

This is a wrapper to dbms_lob.converttoblob and does not convert the CLOB to a binary BLOB.
Doing so would require additional code.
As it is not any faster than the hex_to_blob function, or Perl pack(), there is no point in extending it.

```sql
@create/clob2blob-func.sql
```

#### Create the Java function hexToBlob and PL/SQL wrapper function hex_to_blob_java

This works, but is quite a bit slower than both the PL/SQL function hex_to_blob and the Perl pack() method.

```sql
@create/java/HexConverter-java.sql
@create/java/hex-to-blob.sql
```

#### Create the Java function clobToBlob2 and PL/SQL wrapper function clob_to_blob_java

This Java Stored Procedure accepts tablename, clob and blob column names, and the rowid as arguments.

It then converts the CLOB to BLOB and updates the table.

Like the other Java function, it is too slow.


```sql
@create/java/HexConverter-java-02.sql
@create/java/hex-to-blob-02.sql
```


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

The sequence of script to run:

Prepare the data:

* create/insert-photo.sh
** this creates 1000 rows with a picture of a cat in each row
* dunldr.sh
** this creates the data file for sqlldr

Then for each test run the following scripts:

To test the Perl pack() method:

* sqlldr.sh
* clob-to-blob.sh

To test the PL/SQL hex_to_blob function:

* sqlldr.sh
* clob-to-blob-func.sh

The `sqlldr.sh` script will recreate the test table for each run, and then used to load the data into a CLOB in the BLOBDEST table.

### clob-to-blob.sh

The clob-to-blob.pl script reads the CLOB into a variable and converts it to to binary with  pack(), then saves to the database.

This calls `clob-to-blob-range.pl` twice:

The first time truncates the table and creates the temp table used to track rowids.
(The temp table step is not really necessary for testing, but I did not want to rewrite the script)


Testing is in a 2 Node RAC running Oracle 19.12.

Conversion with Perl pack() function:

```
$  ./clob-to-blob.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m3.375s
user    0m0.027s
sys     0m0.020s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
BLOB SQL: update blobdest set b1 = ? where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    0m54.411s
user    0m18.098s
sys     0m4.998s

```

When run directly on the database server, the Perl pack() method is about 2x faster than when run on a local machine.

```
[oracle@ora192rac02 blob-performance]$ ./clob-to-blob.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.141s
user    0m0.046s
sys     0m0.010s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
BLOB SQL: update blobdest set b1 = ? where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    0m28.777s
user    0m5.749s
sys     0m0.558s
```


### clob-to-blob-func.sh

The clob-to-blob-func.pl script uses the hex_to_blob or clob2blob function to convert the CLOB to BLOB.
Note: that bit is hard coded in the script. Just uncomment the appropriate line.

hex_to_blob is a PL/SQL function that reads the CLOB in chunks, converts the hex to binary, and appends it to the BLOB.

clob2blob is a PL/SQL function that uses the DBMS_LOB.CONVERTTOBLOB procedure to convert the CLOB to BLOB.


Conversion with hex_to_blob function:

```
$  ./clob-to-blob-func.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.124s
user    0m0.040s
sys     0m0.004s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    2m59.027s
user    0m9.362s
sys     0m4.056s

```

Conversion with clob2blob function:

```
$  ./clob-to-blob-func.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.140s
user    0m0.026s
sys     0m0.019s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
BLOB SQL: update blobdest set b1 = clob2blob(c1)  where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    1m3.029s
user    0m9.306s
sys     0m3.600s

```

That is better than expected.

Run directly on the server:

```
$ ./clob-to-blob-func.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.199s
user    0m0.048s
sys     0m0.013s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
BLOB SQL: update blobdest set b1 = clob2blob(c1)  where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    0m58.458s
user    0m3.700s
sys     0m0.421s
```

Still not as fast as the Perl pack() method, but much better than the original PL/SQL hex_to_blob function.

I also tried a Java version of the function.

While it did work, it was much slower than PL/SQL:

```
$  ./clob-to-blob-func.sh

create the TEMP table

Building Temp Table MG_TMP_BLOBDEST
Completed Temp Table MG_TMP_BLOBDEST

real    0m0.141s
user    0m0.035s
sys     0m0.010s

Run the conversion

CLOB SQL: select rowid, c1, nvl(length(b1),0) blob_length from blobdest where rowid = ?
BLOB SQL: update blobdest set b1 = hex_to_blob(c1)  where rowid = ?
rows committed: 200
rows committed: 400
rows committed: 600
rows committed: 800
rows committed: 1000

real    10m16.904s
user    0m8.554s
sys     0m4.816s

```

When run on the server directly, it was much faster at 2m 25s.

That however is still too slow.

These results are pretty clear: the Perl pack() method is much faster than the PL/SQL hex_to_blob function.

While PL/SQL is very fast for database operations, it does not perform so well when it comes to data conversion performed in code.

The Perl pack() method is 3x faster than the PL/SQL hex_to_blob function.

This is partly becuase PL/SQL can only read the CLOB in chunks, and append them to the BLOB via the hextoraw() function.

In the RAC database, the clob2blob function was about 16% slower than the original method.

While the PL/SQL function converted 5.5 rows per second, the Perl pack() method converted 18.5 rows per second.

That is 0.17 seconds per row for the PL/SQL function, and 0.054 seconds per row for the Perl pack() method.

Wnen run on the database directly, 35.7 rows per second were processed, or about 0.028 seconds per row.


### clob-to-blob-inline.sh


### C clob to blob

The C program is about 2x faster than the Perl pack() method.

```
1000 row(s) fetched
Error occurred at OcilibEnvironmentCleanup: Found 999 non freed OCI descriptors
Error occurred at OcilibEnvironmentCleanup: Found 39960 non freed allocated bytes

real	0m27.456s
user	0m11.928s
sys	0m1.293s

[oracle@lestrade demo]$ time ./clob-to-blob lestrade/pdb01 jkstill grok
starting

1000 row(s) updated

real	0m19.567s
user	0m5.666s
sys	0m1.076s

```

## Conclusion

The Perl pack() method is the fastest method for converting a CLOB to a BLOB.








