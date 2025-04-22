
drop table clob_test purge;


create table clob_test (
  id number,
  description varchar2(20),
  clob_data clob
);

insert into clob_test (id, description, clob_data) values (1, 'some data', 'This is a test of the CLOB data type.');
insert into clob_test (id, description, clob_data) values (2, 'some data', 'This is another test of the CLOB data type. This one is a bit longer than the first one, just to see how it handles larger amounts of text. It should work just fine, but we will see.');
insert into clob_test (id, description, clob_data) values (3, 'some data', 'This is just more data');
insert into clob_test (id, description, clob_data) values (4, 'NULL', null);
insert into clob_test (id, description, clob_data) values (4, 'empty_clob()', empty_clob());
commit;

