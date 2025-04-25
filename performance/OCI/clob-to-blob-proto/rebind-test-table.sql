CREATE TABLE test_table (
    id NUMBER PRIMARY KEY,
    name VARCHAR2(100)
);

INSERT INTO test_table (id, name) VALUES (1, 'Alpha');
INSERT INTO test_table (id, name) VALUES (2, 'Beta');
COMMIT;
