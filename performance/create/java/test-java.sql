

set serveroutput on size unlimited

DECLARE
    clob_data CLOB := '48656C6C6F576F726C64';  -- Example hex data ("HelloWorld")
    blob_data BLOB;
BEGIN
    blob_data := hex_to_blob_java(clob_data);
    DBMS_OUTPUT.PUT_LINE('BLOB data length: ' || DBMS_LOB.GETLENGTH(blob_data));
END;
/


