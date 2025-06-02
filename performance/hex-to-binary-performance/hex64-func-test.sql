
-- using blobtest@lestrade/orcl1901
-- first run hex64k-test-data-table.sql

set timing on

update blobtest.hex64k_test_data
set b1 = to_blob_hextoraw( c1 )
where c1 is not null;

commit;


