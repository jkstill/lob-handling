

drop table blobdata purge;

create table blobdata (
	 id number(10) not null,
	 blobdata blob null,
	 constraint blobdata_pk primary key (id)
);

-- insert a single row that will be updated later
insert into blobdata (id) values (1);

