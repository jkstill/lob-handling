
col q_object format a40
set linesize 200 trimspool on
set pagesize 100

with queue_data as (
	select substr(q.user_data.text_vc,1,instr(q.user_data.text_vc,':')-1)  q_object -- tablename in this case
	from clob_to_blob_qtab q 
	where state = 0 -- ready
	--where rownum < 5
)
select q_object
	, count(*) q_count
from queue_data q
group by q_object
order by q_object
/
