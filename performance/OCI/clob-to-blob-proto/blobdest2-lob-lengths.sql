
col b1_check format a15
col b2_check format a15

select 
	id
	, dbms_lob.getlength(b1) b1len
	, dbms_lob.getlength(c1) c1len
	, dbms_lob.getlength(c1)/2 c1destsize
	, dbms_lob.getlength(b2) b2len
	, dbms_lob.getlength(c2) c2len
	, dbms_lob.getlength(c2)/2 c2destsize
	, case dbms_lob.getlength(b1) - dbms_lob.getlength(c1)/2
	when 0 then 'OK'
	else 'ERROR in B1 BLOB Length'
	end B1_check
	, case dbms_lob.getlength(b2) - dbms_lob.getlength(c2)/2
	when 0 then 'OK'
	else 'ERROR in B2 BLOB Length'
	end B1_check
from blobdest2
where 
	dbms_lob.getlength(b1) + dbms_lob.getlength(b2) > 0
--where rownum < 10
order by id
/
