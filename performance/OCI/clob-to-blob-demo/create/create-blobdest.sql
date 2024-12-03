
drop table blobdest cascade constraints purge;

create table blobdest (
	id number primary key not null,
	name varchar2(64) not null,
	c1 clob null,
	b1 blob null
)
tablespace users 
pctfree 10
lob ("B1") store as (tablespace "LOBS" DISABLE STORAGE IN ROW CHUNK 8192 PCTVERSION 10 NOCACHE LOGGING)
lob ("C1") store as (tablespace "LOBS" DISABLE STORAGE IN ROW CHUNK 8192 PCTVERSION 10 NOCACHE LOGGING)
/

create unique index blobdest_name on blobdest(name) tablespace users pctfree 10 initrans 50 maxtrans 255 storage (initial 64K next 1M pctincrease 0) nologging;


