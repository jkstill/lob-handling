
col b1_check format a25
col b2_check format a25

with data as (
	select
		id
		, dbms_lob.getlength(b1) b1len
		, dbms_lob.getlength(c1) c1len
		, dbms_lob.getlength(c1)/2 c1destsize
		, dbms_lob.getlength(b2) b2len
		, dbms_lob.getlength(c2) c2len
		, dbms_lob.getlength(c2)/2 c2destsize
		, case dbms_lob.getlength(b1)
		  when 0 then 'ERROR in B1 BLOB Length'
		  when  NULL then 'B1 BLOB Length is NULL'
        else 'OK'
		end b1_check
		, case dbms_lob.getlength(b2)
		  when 0 then 'ERROR in B2 BLOB Length'
		  when  NULL then 'B2 BLOB Length is NULL'
        else 'OK'
		end b2_check
	from blobdest2
	where dbms_lob.getlength(b1) + dbms_lob.getlength(b2) > 0
	--where rownum < 10
	order by id
)
select d.*
from data d
--where id in (3, 7,15, 27, 32, 45, 61, 72, 85, 97, 112,  125, 173, 215, 292, 327, 383, 415, 427, 815, 827, 1215, 1227, 1815, 1827, 2215, 2227, 2615, 2627, 3015, 3027)
/
