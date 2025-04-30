BEGIN
  DBMS_AQADM.CREATE_QUEUE_TABLE(
    queue_table        => 'clob_to_blob_qtab',
    queue_payload_type => 'AQ$_JMS_TEXT_MESSAGE',
    multiple_consumers => FALSE);

  DBMS_AQADM.CREATE_QUEUE(
    queue_name         => 'clob_to_blob_queue_001',
    queue_table        => 'clob_to_blob_qtab');

  DBMS_AQADM.START_QUEUE(queue_name => 'clob_to_blob_queue_001');
END;
/
