
col tablename format a40
col column_id format 999
col clob_column_name format a30
col blob_column_name format a30
set linesize 200 trimspool on
set pagesize 100

select * 
from clob_to_blob_columns
order by tablename, column_id
/
