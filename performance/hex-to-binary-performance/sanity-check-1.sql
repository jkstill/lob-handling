
-- a test script called a function to convert the hex data in hexdata.hexdata to binary blob data
-- and then back to hex data, checking that the results match
-- the first 64 bytes and the last 64 bytes of the hex data do match

select dbms_lob.substr(hexdata,64,1)  hexdata_head from hexdata;

select lower(rawtohex(dbms_lob.substr(blobdata,32,1))) hex_from_blob_head from blobdata;

select dbms_lob.substr(hexdata,64,dbms_lob.getlength(hexdata)-63) hexdata_tail from hexdata;

select lower(rawtohex(dbms_lob.substr(blobdata,32,dbms_lob.getlength(blobdata)-31))) hex_from_blob_tail from blobdata;


