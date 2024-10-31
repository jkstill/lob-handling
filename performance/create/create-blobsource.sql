
drop table blobsource cascade constraints purge;

create table blobsource (
  id number primary key not null,
  b1 blob not null
);


