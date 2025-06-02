
drop table hexdata purge;

create table hexdata (
	 id number(10) not null,
	 hexdata clob not null,
	 constraint hexdata_pk primary key (id)
);

