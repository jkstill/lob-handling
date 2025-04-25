
-- requires 19.22

/*

With many consumers and a high transaction rate, 'buffer busy waits' can occur when the queue is being dequeued. 

These waits are consuming 65% of the execution time, so it is important to fix this.

The solution is to set the queue parameter CQ_SCRAMBLED_DEQ to 1.

  dbms_aqadm.set_queue_parameter('<schema name>.<queue name>', 'CQ_SCRAMBLED_DEQ', 1);

https://docs.oracle.com/en/database/oracle/oracle-database/19/arpls/DBMS_AQADM.html#GUID-9375E6C8-1BC0-4E45-8045-143927DD751C

This is only available as of 19.22.

https://docs.oracle.com/en/database/oracle/oracle-database/19/newft/new-features-19c-release-updates.html#GUID-1AF5692F-9415-4390-89D7-4DDE796FA1D5

Currently the bplmdev databases is at 19.03.

I tested this locally on a 19.22 database and it works.

It is a small server, and I ran 40 clients on 3200 rows.

Even on this small scale, there was a 16% improvement in throughput.

It should be even more significant on a larger server with millions of rows and 100 clients.

*/


begin
	dbms_aqadm.set_queue_parameter('BLOBTEST.CLOB_TO_BLOB_QUEUE', 'CQ_SCRAMBLED_DEQ', 1);
end;
/

