
set serveroutput on size unlimited

/* PL/SQL Test Block */
/*
DECLARE
    l_table_name VARCHAR2(100) := 'BLOBDEST';
    l_column_name VARCHAR2(100) := 'C1';
    l_row_id VARCHAR2(18) := 'AABecuAAOAACEmDAAF';
    l_blob BLOB;
	dummy integer;
BEGIN
    l_blob := clob_to_blob_java_02(l_table_name, l_column_name, l_row_id);

	update BLOBDEST set b1 =  l_blob where rowid = l_row_id;

    IF l_blob IS NOT NULL THEN
        DBMS_OUTPUT.PUT_LINE('BLOB conversion successful. Length: ' || DBMS_LOB.getlength(l_blob));
    ELSE
        DBMS_OUTPUT.PUT_LINE('BLOB conversion failed or returned NULL.');
    END IF;
END;
/

*/




/* PL/SQL Test Block */
DECLARE
    l_table_name VARCHAR2(100) := 'BLOBDEST';
    l_clob_column_name VARCHAR2(100) := 'C1';
    l_blob_column_name VARCHAR2(100) := 'B1';
    l_row_id VARCHAR2(18) := 'AABedVAAOAACEmQAAb';
BEGIN
    clob_to_blob_java(l_table_name, l_clob_column_name, l_blob_column_name, l_row_id);
    DBMS_OUTPUT.PUT_LINE('BLOB column updated successfully.');
END;
/
