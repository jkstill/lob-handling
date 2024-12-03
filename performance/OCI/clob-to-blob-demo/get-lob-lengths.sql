
-- get-lob-lengths.sql
-- a simple verification step
-- the CLOB data should be exactly 2x the size of the BLOB data

col name format a50
col clob_length format 99,999,999
col blob_length format 99,999,999

set feed on term on echo off verify off

set linesize 200 trimspool on
set pagesize 100


with data as (
	select id
		, dbms_lob.getlength(c1) clob_length
		, dbms_lob.getlength(b1) blob_length
		, name
	from blobdest
)
select id
	, clob_length
	, blob_length
	, case mod(clob_length,blob_length) 
		when 0 then 'OK'
		else 'Error'
	end clob_convert_status
	, name
from data
order by clob_convert_status, id
/
