
@@to-blob-chunk4k.sql

alter function to_blob_chunk4k compile plsql_code_type=native reuse settings;

set serveroutput on size unlimited

DECLARE
  l_hex   CLOB;
  l_blob  BLOB;
  t0      NUMBER;
  t1      NUMBER;
  iterations  INTEGER := 100;
BEGIN
  ---------------------------------------------------------------
  -- Build 2 000 000-character CLOB in 16-byte chunks
  ---------------------------------------------------------------
  --DBMS_LOB.createtemporary(l_hex, TRUE);

  select hexdata into l_hex from hexdata where id = 1;

  ---------------------------------------------------------------
  -- Time the conversion
  ---------------------------------------------------------------
  t0 := DBMS_UTILITY.get_time;
  for i in 1..iterations
  loop
    l_blob := to_blob_chunk4k(l_hex);
  end loop;
  t1 := DBMS_UTILITY.get_time;

  DBMS_OUTPUT.put_line('Convert to blob elapsed: ' || ROUND((t1 - t0) / 100, 3));
  DBMS_OUTPUT.put_line('Convert to blob average: ' || ROUND(((t1 - t0) / 100)/iterations, 3));
  DBMS_OUTPUT.put_line('Blob bytes : ' || DBMS_LOB.getlength(l_blob));
  --DBMS_OUTPUT.put_line('t0 : ' || to_char(t0));
  --DBMS_OUTPUT.put_line('t1 : ' || to_char(t1));

	update blobdata set blobdata = l_blob where id = 1;

  ---------------------------------------------------------------
  -- Clean up
  ---------------------------------------------------------------
  --DBMS_LOB.freetemporary(l_hex);
  DBMS_LOB.freetemporary(l_blob);
  
  COMMIT;

END;
/


