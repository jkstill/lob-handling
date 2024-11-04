CREATE OR REPLACE PROCEDURE hex_to_blob_proc(clob IN CLOB, blob OUT BLOB) AS
    EXTERNAL
    LIBRARY hex_lib
    NAME "hex_to_blob"
    LANGUAGE C;
/

show errors procedure hex_to_blob_proc


