
col b1_check format a25

select dbms_lob.getlength(b1) , dbms_lob.getlength(c1) 
      , case dbms_lob.getlength(b1)
        when 0 then 'ERROR in B1 BLOB Length'
        when  NULL then 'B1 BLOB Length is NULL'
        else 'OK'
      end b1_check
from blobdest
where dbms_lob.getlength(b1) > 0
--where rownum < 10
/
