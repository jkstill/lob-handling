

-- clear some c2 to make sure that clob-to-blob-aq correctly skips that column

update 
	blobdest2 set 
	b1 = empty_blob()
	,  c2 = c1
	,  b2 = empty_blob()
/

update blobdest2
set c2 = empty_clob()
where id in (15, 27)
/

commit;

