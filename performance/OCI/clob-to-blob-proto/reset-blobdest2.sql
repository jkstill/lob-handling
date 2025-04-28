

-- clear some c2 to make sure that clob-to-blob-aq correctly skips that column

@@create-blobdest2

update 
	blobdest2 set 
	b1 = empty_blob()
	,  c2 = c1
	,  b2 = empty_blob()
/

update blobdest2
set c2 = empty_clob()
where id in (3, 7,15, 27, 32, 45, 61, 72, 85, 97, 112,  125, 173, 215, 292, 327, 383, 415, 427, 815, 827, 1215, 1227, 1815, 1827, 2215, 2227, 2615, 2627, 3015, 3027)
/

commit;

