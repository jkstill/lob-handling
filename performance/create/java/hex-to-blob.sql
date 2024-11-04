
--CREATE OR REPLACE FUNCTION hex_to_blob_java(hex_input CLOB) RETURN BLOB
--AS LANGUAGE JAVA
--NAME 'HexConverter.hexToBlob(java.sql.Connection, java.sql.Clob) returns oracle.sql.BLOB';
--/



CREATE OR REPLACE FUNCTION hex_to_blob_java(hex_input CLOB) RETURN BLOB
AS LANGUAGE JAVA
--NAME 'HexConverter.hexToBlob(java.sql.Connection, java.sql.Clob) returns oracle.sql.BLOB';
NAME 'HexConverter.hexToBlob(java.sql.Clob) returns oracle.sql.BLOB';
/



show error function hex_to_blob_java

