
set linesize 200 trimspool on
set pagesize 100
col name format a30
col method_name format a30
col return_class format a30

SELECT  name, method_name, return_class
FROM user_java_methods
WHERE name = 'HexConverter'
/
