#!/usr/bin/env bash

echo "Getting LOB Length Verification Data"

sqlplus -L -S  $dest_oracle_user/$dest_oracle_password@$dest_oracle_ezName <<EOF
  set serveroutput on
  set feedback off
  set heading on
  set echo off
  set verify off
  set termout on
  set pagesize 100
  set linesize 200 trimspool on
  set trimout on
  set arraysize 100
  set long 2000000

  @@get-lob-lengths.sql
  set termout on

  exit

EOF

