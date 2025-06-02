
-- to_blob_hextoraw function
-- this converts a CLOB containing hex digits to a BLOB
-- it is very fast, because it uses HEXTORAW

/*

There was an issue with a leading zero in the first byte of the BLOB.

This is a PL/SQL edge case.

As the slice is now bigger than the amount requested, Oracle now 
returns the full 32764 characters – an even string – and
hextoraw(Slice) produces the correct first byte (F2, no leading zero).

The f2 is the first byte of some test data that was getting prefixed with a zero

*/


create or replace function to_blob_hextoraw (p_hex IN clob)
return blob
is
	----------------------------------------------------------------
	-- 32764-char slices: even, < 32767, safe for HEXTORAW
	----------------------------------------------------------------
	c_chunk constant pls_integer := 32764;
	len     pls_integer := dbms_lob.getlength(p_hex);
	pos     pls_integer := 1;
	slen    pls_integer;
	slice   varchar2(32767);
	raw_val raw(32767);
	outb    blob;
begin
	if mod(len,2)=1 then
		raise_application_error(-20000,'Input length must be even.');
	end if;

	dbms_lob.createtemporary(outb, true);

	while pos <= len
	loop
		slen := least(c_chunk, len - pos + 1);
		dbms_lob.read(p_hex, slen, pos, slice);

		raw_val := hextoraw(slice);

		dbms_lob.writeappend(outb,
			utl_raw.length(raw_val),
			raw_val
		);

		pos := pos + slen;
	end loop;

return outb;
end to_blob_hextoraw;
/

@plsql-error to_blob_hextoraw

prompt
prompt compile native
prompt

alter function to_blob_hextoraw compile plsql_code_type=native reuse settings;


