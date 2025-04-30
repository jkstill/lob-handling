
/* 

grant executed on dbms_aqadm to user
grant executed on dbms_aq to user

*/

-- append '001' to the queue name
-- clob-to-blob-aq will be expected a queue id of 0-100

var max_rows number;
exec : max_rows := 5000;
exec : max_rows := 10;

DECLARE
   enqueue_options     DBMS_AQ.ENQUEUE_OPTIONS_T;
   message_properties  DBMS_AQ.MESSAGE_PROPERTIES_T;
   message_handle      RAW(16);
   message             AQ$_JMS_TEXT_MESSAGE; -- ✅ Use local wrapper type
BEGIN

   message := AQ$_JMS_TEXT_MESSAGE(NULL, NULL); -- or use the constructor with text later

   FOR trec IN (
		SELECT 'BLOBDEST' tablename, rowid row_id
			FROM blobdest 
			WHERE dbms_lob.getlength(b1) = 0
			and rownum <= :max_rows
   ) LOOP

      message.text_vc := trec.tablename || ':' || trec.row_id;

      DBMS_AQ.ENQUEUE(
         queue_name          => 'clob_to_blob_queue_001',
         enqueue_options     => enqueue_options,
         message_properties  => message_properties,
         payload             => message,
         msgid               => message_handle
      );

   END LOOP;

   COMMIT;
END;
/

