
col library_name format a30
col file_spec format a60

set pagesize 100
set linesize 200 trimspool on 

select library_name, file_spec
from user_libraries
where library_name = 'CLOB_TO_LOB'
order by 1,2
/
