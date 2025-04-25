
/* 

grant executed on dbms_aqadm to user
grant executed on dbms_aq to user

*/

var max_rows number;
exec :max_rows := 5000;

DECLARE
   enqueue_options     DBMS_AQ.ENQUEUE_OPTIONS_T;
   message_properties  DBMS_AQ.MESSAGE_PROPERTIES_T;
   message_handle      RAW(16);
   message             AQ$_JMS_TEXT_MESSAGE; -- ✅ Use local wrapper type
BEGIN

   message := AQ$_JMS_TEXT_MESSAGE(NULL, NULL); -- or use the constructor with text later

   FOR trec IN (
		SELECT 'BLOBDEST2' tablename, rowid row_id
		FROM blobdest2
		WHERE dbms_lob.getlength(c1) + dbms_lob.getlength(c2) > 0
		and rownum <= :max_rows
	) LOOP

      message.text_vc := trec.tablename || ':' || trec.row_id;

      DBMS_AQ.ENQUEUE(
         queue_name          => 'clob_to_blob_queue',
         enqueue_options     => enqueue_options,
         message_properties  => message_properties,
         payload             => message,
         msgid               => message_handle
      );

   END LOOP;

   COMMIT;
END;
/

