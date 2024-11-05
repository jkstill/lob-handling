select id, dbms_lob.getlength(c1), dbms_lob.getlength(b1), rowid
from blobdest
order by id
/
