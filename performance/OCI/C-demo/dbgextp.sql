Rem
Rem dbgextp.sql
Rem
Rem  Copyright (c) Oracle Corporation 1997. All Rights Reserved.
Rem
Rem    NAME
Rem      dbgextp.sql - Debugging package for external procedures
Rem
Rem
Rem
Rem    PACKAGE INSTALL NOTES
Rem
Rem      o Install/load this package in the Oracle USER where you wish
Rem        to debug the 'extproc' process.
Rem
Rem      o The user should have 'create library' privileges. Ask the
Rem        system administrator of this database if this Oracle user
Rem        account does not have them.
Rem
Rem      o Ensure that you have execute privileges on package DEBUG_EXTPROC
Rem
Rem          SELECT SUBSTR(OBJECT_NAME, 1, 20) FROM USER_OBJECTS
Rem                WHERE OBJECT_NAME = 'DEBUG_EXTPROC';
Rem
Rem      o You can install this package as any other user, as long as
Rem        as you have execute privileges on the package.
Rem
Rem    USAGE ASSUMPTIONS
Rem
Rem      o The Listner has been appropriately configured to startup an
Rem        external procedures 'extproc' agent.
Rem
Rem      o You have built your shared library with debug symbols to
Rem        aid in the debugging process. Please check the C compiler
Rem        manual pages for the appropriate C compiler switches to build
Rem        the shared library with debug symbols.
Rem
Rem
Rem    USAGE NOTES
Rem
Rem      o Start a brand new oracle session thru sqlplus or oci program
Rem        by connecting to ORACLE.
Rem
Rem      o Execute procedure DEBUG_EXTPROC.STARTUP_EXTPROC_AGENT to startup the
Rem        extproc agent in this session
Rem
Rem          E.g. execute DEBUG_EXTPROC.STARTUP_EXTPROC_AGENT;
Rem
Rem        Do not exit this session since this will terminate the
Rem        extproc agent.
Rem
Rem      o Determine the PID of the extproc agent that was started up for this
Rem        session.
Rem
Rem      o Using a debugger (e.g. gdb, dbx or the native system debugger)
Rem        load the extproc executable and attach to the running process.
Rem
Rem      o Set a breakpoint on function 'pextproc' and let the debugger
Rem        continue with its execution.
Rem
Rem      o Now execute your external procedure in the same session where you
Rem        first executed DEBUG_EXTPROC.STARTUP_EXTPROC_AGENT
Rem
Rem      o Your debugger should now break in function 'pextproc'. At this point
Rem        in time, the shared library referenced by your PL/SQL external
Rem        function would have been loaded and the function resolved.
Rem        Now set a breakpoint in your C function and let the debugger
Rem        continue its execution.
Rem
Rem        Since PL/SQL loads the shared library at runtime, the debugger
Rem        you use may or may not automatically be able to track the new
Rem        symbols from the shared library. You may have to issue some
Rem        debugger command to load the symbols (e.g. 'share' in gdb)
Rem
Rem      o The debugger should now break in your C function. Its assumed that
Rem        you had built the shared library with debugging symbols.
Rem
Rem      o Now proceed with your debugging.
Rem
Rem
Rem
Rem
Rem ----------------------------------------------------------------------
Rem
Rem THIS PACKAGE SHOULD NOT BE MODIFIED IN ANY WAY BY ANY USER
Rem
Rem ----------------------------------------------------------------------
Rem
Rem

CREATE OR REPLACE PACKAGE debug_extproc IS

  --
  -- Startup the extproc agent process in the session
  --
  --   Executing this procedure, starts up the extproc agent process
  --   in the session allowing one to be able get the PID of the
  --   executing process. This PID is needed to be able to attach
  --   to the running process using a debugger.
  --
  PROCEDURE startup_extproc_agent;

END debug_extproc;
/

CREATE OR REPLACE LIBRARY debug_extproc_library IS STATIC;
/

CREATE OR REPLACE PACKAGE BODY debug_extproc IS

  extproc_lib_error EXCEPTION;
  PRAGMA EXCEPTION_INIT (extproc_lib_error, -6520);

  extproc_func_error EXCEPTION;
  PRAGMA EXCEPTION_INIT (extproc_func_error, -6521);

  PROCEDURE local_startup_extproc_agent IS EXTERNAL
    LIBRARY debug_extproc_library;

  PROCEDURE startup_extproc_agent is
  BEGIN

    -- call a dummy procedure and trap all errors.
    local_startup_extproc_agent;

  EXCEPTION
    -- Ignore any errors if the function or library is not found.
    WHEN extproc_func_error then NULL;
    WHEN extproc_lib_error then NULL;
  END startup_extproc_agent;

END debug_extproc;
/
