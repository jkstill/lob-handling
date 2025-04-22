
BEGIN
  DBMS_AQADM.STOP_QUEUE(queue_name => 'clob_to_blob_queue', wait => TRUE);
END;
/

BEGIN
  DBMS_AQADM.STOP_QUEUE('clob_to_blob_queue', force => TRUE);
END;
/


BEGIN
  DBMS_AQADM.DROP_QUEUE('clob_to_blob_queue');
END;
/



BEGIN
  DBMS_AQADM.DROP_QUEUE_TABLE('clob_to_blob_qtab');
END;
/


