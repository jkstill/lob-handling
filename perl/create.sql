
drop table blobtest cascade constraints purge;

create table blobtest(
	id number(12) not null,
	c1 clob,
	b1 blob
)
/



