
col b1_check format a25

select dbms_lob.getlength(b1), dbms_lob.getlength(c1) , dbms_lob.getlength(c1)/2 clobdestsize 
        , case dbms_lob.getlength(b1) - dbms_lob.getlength(c1)/2
        when 0 then 'OK'
        else 'ERROR in B1 BLOB Length'
        end B1_check
from blobdest
where dbms_lob.getlength(b1) > 0
--where rownum < 10
/
