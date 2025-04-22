select dbms_lob.getlength(b1), dbms_lob.getlength(c1) , dbms_lob.getlength(c1)/2 clobdestsize 
from blobdest
where dbms_lob.getlength(b1) > 0
--where rownum < 10
/
