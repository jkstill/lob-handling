

set -v

#gcc -shared -fPIC -o test.so test.c
gcc -shared -fPIC -o test.so test.c -I$ORACLE_HOME/rdbms/public -L$ORACLE_HOME/lib -lclntsh -std=c99 -Wimplicit-function-declaration -g

cp test.so $ORACLE_HOME/lib

