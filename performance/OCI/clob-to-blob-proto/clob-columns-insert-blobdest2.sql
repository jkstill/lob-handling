
delete from clob_to_blob_columns where tablename = 'BLOBDEST2';

insert into clob_to_blob_columns ( tablename, column_id, clob_column_name, blob_column_name)
values ( 'BLOBDEST2', 1, 'C1', 'B1' );

insert into clob_to_blob_columns ( tablename, column_id, clob_column_name, blob_column_name)
values ( 'BLOBDEST2', 2, 'C2', 'B2' );

--insert into clob_to_blob_columns ( tablename, column_id, clob_column_name, blob_column_name)
--values ( 'BLOBDEST2', 2, 'case dbms_lob.getlength(C2) when 0 then null else C2 end C2', 'B2' );

commit;


