
drop table blobdest cascade constraints purge;

create table blobdest (
  id number primary key not null,
  c1 clob null,
  b1 blob null
)
tablespace users 
pctfree 10
lob ("B1") store as (tablespace "LOBS" DISABLE STORAGE IN ROW CHUNK 8192 PCTVERSION 10 NOCACHE LOGGING)
lob ("C1") store as (tablespace "LOBS" DISABLE STORAGE IN ROW CHUNK 8192 PCTVERSION 10 NOCACHE LOGGING)
/


