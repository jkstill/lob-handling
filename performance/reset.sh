#!/usr/bin/env bash

source setenv.sh

echo "resetting data in blobdest"

sqlplus -L -S  jkstill/grok@$ezName <<EOF
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

  @@reset.sql
  exit

EOF

