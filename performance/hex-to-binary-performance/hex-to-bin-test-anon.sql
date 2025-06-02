
set serveroutput on size unlimited

declare
    hexdata  clob;
    blobdata blob;
    t_elapsed_start  number;
    t_elapsed_end    number;
    t_cpu_start  number;
    t_cpu_end    number;
    elapsed  number;
    avg_time number;
    cpu_elapsed  number;
    cpu_avg_time number;
	 iterations pls_integer := 100;
begin
    select hexdata into hexdata from hexdata where id = 1;

    t_elapsed_start := dbms_utility.get_time;
    --dbms_output.put_line('t_elapsed_start: ' || t_elapsed_start);

    t_cpu_start := dbms_utility.get_cpu_time;

    for i in 1 .. iterations
    loop
        blobdata := hextoblob(hexdata);
    end loop;

    t_elapsed_end := dbms_utility.get_time;
    t_cpu_end := dbms_utility.get_cpu_time;

    elapsed := (t_elapsed_end - t_elapsed_start); -- in hundredths of a second
    avg_time := elapsed / iterations;

    cpu_elapsed := (t_cpu_end - t_cpu_start); -- in hundredths of a second
    cpu_avg_time := cpu_elapsed / iterations;


    dbms_output.put_line('Elapsed time (cs): ' || elapsed);
    dbms_output.put_line('Average time per iteration (cs): ' || avg_time);
    dbms_output.put_line('CPU Elapsed time (cs): ' || cpu_elapsed);
    dbms_output.put_line('CPU Average time per iteration (cs): ' || cpu_avg_time);
end;
/
