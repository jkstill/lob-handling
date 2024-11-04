
set pagesize 100
set linesize 200 trimspool on
col line format 99999
col position format 9999
col text format a80
col name format a30
col type format a20

SELECT name, type, sequence, line, position, text
FROM user_errors
WHERE name like  'HexConverter%' AND type like 'JAVA%'
ORDER BY name, type,  sequence
/
