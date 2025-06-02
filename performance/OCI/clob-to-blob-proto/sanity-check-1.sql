

col id format 99999
col name format a50
col md5hash_source format a32
col md5hash_dest format a32
set linesize 200 trimspool on
set pagesize 100


with data as (
select
	s.id
	, s.name
	, dbms_crypto.hash(s.b1,2) as md5hash_source
	, dbms_crypto.hash(d.b1,2) as md5hash_dest
from blobsource s
join blobdest d
on d.id = s.id
and  d.name = s.name
where dbms_lob.getlength(d.b1) > 0
)
select
	id
	, name
	, md5hash_source
	, md5hash_dest
	, case when md5hash_source = md5hash_dest then 'Match' else 'Mismatch' end as status
from data
order by id
/

