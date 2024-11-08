
def libFile='/u01/app/oracle/product/19.0.0/dbhome_1/lib/test.so'

set echo on verify off

!ls -l &libFile

CREATE OR REPLACE LIBRARY test1 AS '&libFile';
/

CREATE OR REPLACE FUNCTION mygetenv(x VARCHAR2)
RETURN VARCHAR2
AS LANGUAGE C
LIBRARY test1
NAME "mygetenv";
/

show error function mygetenv

CREATE OR REPLACE FUNCTION mynegative( y BINARY_INTEGER)
RETURN BINARY_INTEGER
AS LANGUAGE C
LIBRARY test1
NAME "negative";
/

show error function mynegative


CREATE OR REPLACE PROCEDURE log_env_vars(x VARCHAR2)
AS LANGUAGE C
LIBRARY test1
NAME "log_env_vars";
/

show error function log_env_vars


CREATE OR REPLACE FUNCTION mybench(n pls_integer, i pls_integer) return FLOAT
AS LANGUAGE C
LIBRARY test1
NAME "mybench";
/

show error function mybench

