SELECT COUNT(*) AS ready_messages
FROM clob_to_blob_qtab
WHERE state = 0
/
