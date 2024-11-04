
-- this does work, but, Perl pack() is _much_ faster
-- fixing the 1 letter variable names

create or replace
function hex_to_blob (hex clob) return blob is
	targetBlob blob                := null;
   -- buf can be max of 32767 bytes
	buf varchar2(32767 byte) := null;
	-- readLen is set to 8192 as the max character size is 4 bytes
	-- it is not likely that the average character size is even close to 4 bytes
	-- set to int(32767/2) = 16384
	-- must be power of 2
	-- 16834 might usually work here, but 8192 should be safe
	readLen number              := 8192;
begin

	if hex is not null then
		dbms_lob.createtemporary(targetBlob, true);

		for i in 0 .. length(Hex) / readLen loop
			dbms_lob.read(hex, readLen, i * readLen + 1, buf);
			dbms_lob.append(targetBlob, to_blob(hextoraw(buf)));
		end loop;
	end if;

	return targetBlob;

end hex_to_blob;
/

