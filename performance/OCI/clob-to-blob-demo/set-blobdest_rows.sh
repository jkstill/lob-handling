#!/usr/bin/env bash

echo "resetting data in blobdest"

sqlplus -L -S  $dest_oracle_user/$dest_oracle_password@$dest_oracle_ezName <<EOF
  set serveroutput on
  set feedback off
  set heading off
  set echo off
  set verify off
  set termout off
  set pagesize 0
  set linesize 32767
  set trimspool on
  set trimout on
  set colsep '|'
  set timing off
  set time off
  set define off
  set sqlblanklines off
  set sqlprompt ''
  set newpage none
  set arraysize 5000
  set numwidth 38
  set long 2000000

  @@set-blobdest_rows.sql
  set termout on

  select trim(count(*)) || ' rows in blobdest_rows' blobdest_rows_count from blobdest_rows;

  exit

EOF

