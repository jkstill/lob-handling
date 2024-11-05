

/* PL/SQL Wrapper Function */
CREATE OR REPLACE PROCEDURE clob_to_blob_java(table_name VARCHAR2, clob_column_name VARCHAR2, blob_column_name VARCHAR2, row_id VARCHAR2)
AS LANGUAGE JAVA
NAME 'HexConverter2.clobToBlob2(java.lang.String, java.lang.String, java.lang.String, java.lang.String)';
/


show errors function clob_to_blob_java


