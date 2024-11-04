
ALTER SESSION SET plsql_code_type = 'INTERPRETED';
--ALTER SESSION SET plsql_code_type = 'NATIVE';
-- tread compiling DBMS_LOB native
-- that succeeded, but attempting to use it cause ORA-07445
-- setting this to NATIVE has little affect, as expected

create or replace
function clob2blob (sourceClob clob) return blob deterministic
as

	targetOffset           int := 1;
	sourceOffset           int := 1;
	targetBlob             blob;
	languageContext        int := dbms_lob.default_lang_ctx;
	blobConversionWarning  int := dbms_lob.warn_inconvertible_char;

begin

	dbms_lob.createtemporary(
		lob_loc => targetBlob,
		cache   => true
	);

/* from the docs: documentation on 'warning'

Currently, the only possible warning is — inconvertible character.
This occurs when the character in the source cannot be properly converted to a character in destination.
The default replacement character (for example, '?') is used in place of the inconvertible character.
The return value of this error message is defined as the constant warn_inconvertible_char in the DBMS_LOB package.

The two possible values for the warning parameter are:
- dbms_lob.no_warning: No warning is issued.
- dbms_lob.warn_inconvertible_char: A warning is issued if an inconvertible character is found.

*/

	dbms_lob.converttoblob(
		dest_lob       => targetBlob,
		src_clob       => sourceClob,
		amount         => dbms_lob.lobmaxsize,
		dest_offset    => targetOffset,
		src_offset     => sourceOffset,
		blob_csid      => dbms_lob.default_csid,
		lang_context   => languageContext,
		warning        => blobConversionWarning
	);

	if (blobConversionWarning = dbms_lob.warn_inconvertible_char) then
		dbms_output.put_line('Warning: inconvertible character found');
		raise_application_error(-20000, 'Warning: inconvertible character found');
	end if;

	return targetBlob;

end;
/

--ALTER SESSION SET plsql_code_type = 'INTERPRETED';


