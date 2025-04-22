
set serveroutput on size unlimited

DECLARE
  dequeue_options     DBMS_AQ.DEQUEUE_OPTIONS_T;
  message_properties  DBMS_AQ.MESSAGE_PROPERTIES_T;
  msgid               RAW(16);
  payload             SYS.AQ$_JMS_TEXT_MESSAGE;
BEGIN
  DBMS_AQ.DEQUEUE(
    queue_name          => 'clob_to_blob_queue',
    dequeue_options     => dequeue_options,
    message_properties  => message_properties,
    payload             => payload,
    msgid               => msgid
  );

  DBMS_OUTPUT.PUT_LINE('Message text: ' || payload.text_vc);
END;
/


