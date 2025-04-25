
drop table blobsource cascade constraints purge;

create table blobsource (
	id number primary key not null,
	name varchar2(64) not null,	
	b1 blob not null,
	c1 clob
);


create unique index blobsource_name_idx on blobsource(name) tablespace users pctfree 10 initrans 50 maxtrans 255 storage (initial 64K next 1M pctincrease 0) nologging;


