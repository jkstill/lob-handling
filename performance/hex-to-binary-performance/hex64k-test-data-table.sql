
create table hex64k_test_data
as
select * from blobsource
where 1=0;

alter table hex64k_test_data modify (b1 null);

insert /*+ append */ into hex64k_test_data(id,name,c1)
select id,name,c1
from blobsource;

commit;

update hex64k_test_data set b1 = null;

commit;



