
-- https://paulzipblog.wordpress.com/2021/11/28/blob-to-hex-and-hex-to-blob/

create or replace function BlobToHex(pBlob in blob) return clob is
-- Converts a blob to a clob containing the hex representation of the blob
  BUFF_SIZE constant integer := 16383; -- Each raw byte converts to 2 hex digits, so buffer is half of 32766
  vBuffer varchar2(32766 byte);  -- Twice buffer size
  vOffset pls_integer := 1;
  vLength pls_integer := DBMS_LOB.GetLength(pBlob);
  vResult clob;
begin
  if coalesce(vLength, 0) > 0 then
    DBMS_LOB.CreateTemporary(vResult, True, DBMS_LOB.Call);
    for i in 1..ceil(vLength / BUFF_SIZE)
    loop
      vBuffer := RawToHex(DBMS_LOB.Substr(pBlob, BUFF_SIZE, vOffset));
      DBMS_LOB.WriteAppend(vResult, length(vBuffer), vBuffer);
      vOffset := vOffset + BUFF_SIZE;
    end loop;
  end if;
  return vResult;
end;
/

create or replace function HexToBlob(pHexClob in clob) return blob is
-- Converts a clob containing hex representation of binary data into a blob
  BUFF_SIZE constant integer := 32764; -- Some Oracle versions have a bug which loses trailing bytes when using buffer sizes > 32764,
  vBuffer raw(16382);
  vOffset pls_integer := 1;
  vLength pls_integer := DBMS_LOB.GetLength(pHexClob);
  vResult blob;
begin
  if coalesce(vLength, 0) > 0 then
    DBMS_LOB.CreateTemporary(vResult, True, DBMS_LOB.Call);
    for i in 1..ceil(vLength / BUFF_SIZE)
    loop
      vBuffer := HexToRaw(DBMS_LOB.Substr(pHexClob, BUFF_SIZE, vOffset));
      DBMS_LOB.WriteAppend(vResult, UTL_RAW.Length(vBuffer), vBuffer);
      vOffset := vOffset + BUFF_SIZE;
    end loop;
  end if;
  return vResult;
end;
/

