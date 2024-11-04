
col c1_hash format a32
col b1_hash format a32

var hashMethod number;

set feed off term off
exec :hashMethod := sys.dbms_crypto.hash_md5;
set feed on term on

-- when using dbms_lob.converttoclob, all the function is doing is returning the clob


select 
	dbms_crypto.hash(c1, :hashMethod) c1_hash
	, dbms_crypto.hash(b1, :hashMethod) b1_hash
from blobdest
/
