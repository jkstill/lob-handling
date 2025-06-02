
login as blobdest

@reset-blobdest
@reset-blobdest2

@@clob-to-blob-columns.sql

delete from clob_to_blob_columns;
commit;

@@clob-columns-insert.sql
@@clob-columns-insert-blobdest2.sql

@@enqueue-blobdest.sql
@@enqueue-blobdest2.sql

From cli:

./clob-to-blob-aq -q 1

