
prompt
prompt Sanity Check 2
prompt
prompt check the md5 sum of the blob data created by the C, Python and Perl tests
propmt  
prompt "md5sum data/*.dat"
prompt
prompt The output of dbms_crypto.hash should match the output of the md5sum command

-- 2 is the type for md5
select lower(dbms_crypto.hash(src => blobdata,typ => 2)) from blobdata;

