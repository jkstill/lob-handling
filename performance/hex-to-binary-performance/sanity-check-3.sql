
prompt
prompt Sanity Check 3
prompt
prompt run this after running hex64-func-test.sql
prompt  
prompt The output of dbms_crypto.hash should match between the two tables
prompt  
prompt Table blobtest.hex64k_test_data was created from the hex64k-test-data-table.sql
prompt The source table for the data is blobtest.blobsource, which has correct BLOB data
prompt 
prompt no rows should be returned
prompt 

col blob_source format a50
col blob_test format a50

-- 2 is the type for md5
with md5data as (
	select 
		lower(dbms_crypto.hash(src => s.b1,typ => 2))  as blob_source,
		lower(dbms_crypto.hash(src => t.b1,typ => 2)) as blob_test
	from blobtest.blobsource s
	join blobtest.hex64k_test_data t
		on t.id = s.id
	order by s.id
)
select blob_source, blob_test
from md5data
where blob_source != blob_test
/

