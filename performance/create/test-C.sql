


DECLARE
        clob_data CLOB := '48656C6C6F';  -- Example hex data
        blob_data BLOB;
BEGIN
        -- Call the function and get the result directly
        blob_data := hex_to_blob_func(clob_data);

        -- Confirm blob data length
        DBMS_OUTPUT.PUT_LINE('BLOB data length: ' || DBMS_LOB.GETLENGTH(blob_data));
END;
/

