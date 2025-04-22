CREATE TABLE clob_to_blob_columns (
    tablename           VARCHAR2(128),
    column_id           NUMBER,
    clob_column_name    VARCHAR2(128),
    blob_column_name    VARCHAR2(128),
    PRIMARY KEY (tablename, column_id)
);
