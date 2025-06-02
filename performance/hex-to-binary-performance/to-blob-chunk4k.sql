------------------------------------------------------------------------
--  Hex-CLOB  →  BLOB
--  * Reads   4 000 hex characters   ( = 2 000 bytes ) per loop
--  * HEXTORAW() converts whole slice in one go
--  * Appends that RAW(2 000) to the output BLOB
------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION to_blob_chunk4k (p_hex  IN CLOB)
RETURN BLOB
IS
    ----------------------------------------------------------------
    -- 4 000 is comfortably below VARCHAR2/RAW 4 000 limit
    ----------------------------------------------------------------
    c_chunk  CONSTANT PLS_INTEGER := 4000;           -- must be even
    len      PLS_INTEGER := DBMS_LOB.getlength(p_hex);
    pos      PLS_INTEGER := 1;                       -- 1-based
    slen     PLS_INTEGER;                            -- actual slice length
    slice    VARCHAR2(4000);
    raw_val  RAW(2000);                              -- 4k hex -> 2k bytes
    outb     BLOB;
BEGIN
    ----------------------------------------------------------------
    -- Reject odd-length input up-front
    ----------------------------------------------------------------
    IF MOD(len, 2) = 1 THEN
        RAISE_APPLICATION_ERROR(
            -20000,
            'Input length must be even; found '||len||' characters');
    END IF;

    DBMS_LOB.createtemporary(outb, TRUE);

    WHILE pos <= len LOOP
        ------------------------------------------------------------
        -- Calculate remaining size and ensure it’s even
        ------------------------------------------------------------
        slen := LEAST(c_chunk, len - pos + 1);
        IF MOD(slen, 2) = 1 THEN
            slen := slen - 1;  -- make slice length even
        END IF;

        ------------------------------------------------------------
        -- 1) read ≤4 000 characters from the CLOB
        ------------------------------------------------------------
        slice := DBMS_LOB.SUBSTR(p_hex, slen, pos);  -- substr(len,pos,OUT)

        ------------------------------------------------------------
        -- 2) convert whole slice to RAW
        ------------------------------------------------------------
        raw_val := HEXTORAW(slice);            -- RAW(2 000) or smaller

        ------------------------------------------------------------
        -- 3) append to output BLOB in one call
        ------------------------------------------------------------
        DBMS_LOB.WRITEAPPEND(outb,
                             UTL_RAW.LENGTH(raw_val),   -- byte count
                             raw_val);

        pos := pos + slen;                     -- advance pointer
    END LOOP;

    RETURN outb;
END to_blob_chunk4k;
/

@plsql-error to_blob_chunk4k

