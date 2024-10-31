
col character format a2
col charlen format 999
col cloblen format 999
col rawstr format a8
col rawlen format 999

select 
	'≥' character
	, length('≥') charlen
	, dbms_lob.getlength('≥') cloblen
	, UTL_RAW.CAST_TO_RAW(RPAD('≥',1,'≥')) rawstr
	, length(UTL_RAW.CAST_TO_RAW(RPAD('≥',1,'≥')))  rawlen
from dual
/
