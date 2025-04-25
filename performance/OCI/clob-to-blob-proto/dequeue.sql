
set serveroutput on size unlimited

var max_rows number;
exec :max_rows := 5000;

DECLARE
  dequeue_options     DBMS_AQ.DEQUEUE_OPTIONS_T;
  message_properties  DBMS_AQ.MESSAGE_PROPERTIES_T;
  msgid               RAW(16);
  payload             AQ$_JMS_TEXT_MESSAGE;
BEGIN

	dequeue_options.wait := DBMS_AQ.NO_WAIT;
	dequeue_options.visibility := DBMS_AQ.IMMEDIATE;

	for i in 1..:max_rows loop	
	--while true loop

		DBMS_AQ.DEQUEUE(
			queue_name          => 'clob_to_blob_queue',
			dequeue_options     => dequeue_options,
			message_properties  => message_properties,
			payload             => payload,
			msgid               => msgid
		);

  		--DBMS_OUTPUT.PUT_LINE('Message text: ' || payload.text_vc);

	end LOOP;

exception
when others then
	if sqlcode = -25228 then
		DBMS_OUTPUT.PUT_LINE('No messages in the queue');
	else
		DBMS_OUTPUT.PUT_LINE('Error: ' || sqlerrm(sqlcode));
	end if;
END;
/


