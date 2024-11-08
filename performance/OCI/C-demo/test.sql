
prompt
prompt mynegative()
prompt

select mynegative(123) from dual;


prompt
prompt mygetenv()
prompt

select mygetenv('PATH') from dual;

prompt
prompt log_env_vars
prompt

exec log_env_vars('PATH')
--exec log_env_vars('LD_LIBRARY_PATH')

prompt
prompt cat /tmp/extproc_env.log
prompt

!cat /tmp/extproc_env.log


