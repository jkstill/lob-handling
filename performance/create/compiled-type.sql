
col name format a30
col type format a20
col plsql_code_type format a30

set pagesize 100
set linesize 200 trimspool on

select name,type,plsql_code_type
from all_plsql_object_settings
where owner = user
--where name like '%BENCH%'
--where name like 'DBMS_LOB'
--and owner = user
order by name,type
/

