


ALTER SESSION SET plsql_code_type = 'NATIVE';

set timing on

create or replace procedure baseline_bench_native
as
	r  float;
	n pls_integer := 1000000;
begin
	-- test code here
	for i in 2..n
	loop
		r := mod(n,i) + sqrt(i) * log(i,n);
	end loop;
end;
/

exec baseline_bench_native




