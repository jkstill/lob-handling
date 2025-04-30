DECLARE
  enqueue_options     DBMS_AQ.ENQUEUE_OPTIONS_T;
  message_properties  DBMS_AQ.MESSAGE_PROPERTIES_T;
  message_handle      RAW(16);
  message             SYS.AQ$_JMS_TEXT_MESSAGE;
BEGIN
  message := SYS.AQ$_JMS_TEXT_MESSAGE.construct;
  message.set_text('WFASSIGNMENT:AAARkJAAEAAAMTEAAA');
  DBMS_AQ.ENQUEUE(queue_name => 'clob_to_blob_queue_001',
                  enqueue_options => enqueue_options,
                  message_properties => message_properties,
                  payload => message,
                  msgid => message_handle);
  COMMIT;
END;
/
