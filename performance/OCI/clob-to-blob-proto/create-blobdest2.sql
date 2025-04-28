
drop table blobdest2 cascade constraints purge;

create table blobdest2 
initrans 10
pctfree 10
pctused 50
lob ("B1") store as (tablespace "BLOBS" DISABLE STORAGE IN ROW CHUNK 8192 PCTVERSION 10 NOCACHE LOGGING)
lob ("B2") store as (tablespace "BLOBS" DISABLE STORAGE IN ROW CHUNK 8192 PCTVERSION 10 NOCACHE LOGGING)
lob ("C1") store as (tablespace "CLOBS" DISABLE STORAGE IN ROW CHUNK 8192 PCTVERSION 10 NOCACHE LOGGING)
lob ("C2") store as (tablespace "CLOBS" DISABLE STORAGE IN ROW CHUNK 8192 PCTVERSION 10 NOCACHE LOGGING)
as
select 
	id
	, name
	, c1
	, c1 as c2
	, b1
	, b1 as b2
from blobsource
--where id < 31
/



