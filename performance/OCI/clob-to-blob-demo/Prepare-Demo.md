

# Prepare Demo

- [Prepare Demo](#prepare-demo)
  - [Prepare the Environment](#prepare-the-environment)
  - [Create and Populate the Source Tables](#create-and-populate-the-source-tables)
  - [Create and Populate the Control Tables](#create-and-populate-the-control-table)


## Prepare the Environment

Set the local oracle environment as per usual.

Edit the `setenv.sh` script used to set the environment variables for the database connection.

Call the `setenv.sh` script to set the environment variables.

## Create and Populate the Source Tables

Follow instructions at `/home/jkstill/oracle/lob-handling/performance/README.md` to create the blobsource and blobdest tables and populate them with data.

## Create a Dump of the schema

```bash
$ ./sqlldr.sh
$ mv jkstill.dump dump
```

Edit the `dump/*.{ctl,par}` files as needed.

Remove the 'userid' line from the `dump/blobdest.par` file.

## Create a Zip file for distribution

```bash
$  zip -r clob-to-blob-demo.zip create clob-to-blob clob-to-blob.sh demo-setenv.sh dump get-lob-lengths* get-photo* Run-Demo.md set-blobdest_rows* sqlldr.sh
```



The demo uses the blobdest_rows table for lookup of the blob id to be used in the update statement.

This table is created and populated in the `set-blobdest-rows.sh` script.







