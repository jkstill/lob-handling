
CREATE OR REPLACE PACKAGE hex64k IS
  PROCEDURE init;                        -- unchanged
  FUNCTION  to_blob_fast (p_hex CLOB)    -- new, chunked version
           RETURN BLOB;
END hex64k;
/

CREATE OR REPLACE PACKAGE BODY hex64k AS
  /* ---------- lookup table identical to the previous reply ---------- */
  -- g_lut, byte_raw(), init() stay exactly the same
  ------------------------------------------------------------------
  -- Lookup-table: 65 536 entries, each a single RAW(1) byte
  ------------------------------------------------------------------
  TYPE t_lut IS TABLE OF RAW(1) INDEX BY PLS_INTEGER;
  g_lut   t_lut;
  g_ready BOOLEAN := FALSE;                     -- built yet?

  ------------------------------------------------------------------
  -- helper: number (0-255) → RAW(1)
  ------------------------------------------------------------------
  FUNCTION byte_raw(p IN PLS_INTEGER) RETURN RAW DETERMINISTIC IS
  BEGIN
    -- LPAD + TO_CHAR gives two-digit hex, HEXTORAW turns that into 1-byte RAW
    RETURN HEXTORAW(LPAD(TO_CHAR(p, 'FM0X'), 2, '0'));
  END;

  ------------------------------------------------------------------
  -- one-time LUT build
  ------------------------------------------------------------------
  PROCEDURE init IS
    n1  PLS_INTEGER;
    n2  PLS_INTEGER;
    v   PLS_INTEGER;
  BEGIN
    IF g_ready THEN
      RETURN;
    END IF;

    FOR c1 IN 0 .. 255 LOOP
      n1 :=
        CASE
          WHEN c1 BETWEEN ASCII('0') AND ASCII('9') THEN c1 - ASCII('0')
          WHEN c1 BETWEEN ASCII('A') AND ASCII('F') THEN c1 - ASCII('A') + 10
          WHEN c1 BETWEEN ASCII('a') AND ASCII('f') THEN c1 - ASCII('a') + 10
          ELSE -1
        END;

      FOR c2 IN 0 .. 255 LOOP
        n2 :=
          CASE
            WHEN c2 BETWEEN ASCII('0') AND ASCII('9') THEN c2 - ASCII('0')
            WHEN c2 BETWEEN ASCII('A') AND ASCII('F') THEN c2 - ASCII('A') + 10
            WHEN c2 BETWEEN ASCII('a') AND ASCII('f') THEN c2 - ASCII('a') + 10
            ELSE -1
          END;

        IF n1 < 0 OR n2 < 0 THEN           -- any non-hex char → NULL (invalid)
          g_lut(c1 * 256 + c2) := NULL;
        ELSE                               -- valid pair → pre-built byte
          v := n1 * 16 + n2;
          g_lut(c1 * 256 + c2) := byte_raw(v);
        END IF;
      END LOOP;
    END LOOP;

    g_ready := TRUE;
  END init;


  /* ---------- NEW: chunked decoder ---------------------------------- */
  FUNCTION to_blob_fast (p_hex CLOB) RETURN BLOB IS
    c_chunk_chars CONSTANT PLS_INTEGER := 32766;      -- even & ≤ 32767
                                                      -- (~16 KiB binary)
    len      PLS_INTEGER := NVL(DBMS_LOB.getlength(p_hex), 0);
    pos      PLS_INTEGER := 1;
    slice    VARCHAR2(32767);
    outb     BLOB;
    raw_buf  RAW(32767);
    slen     PLS_INTEGER;
    idx      PLS_INTEGER;
  BEGIN
    IF len = 0 THEN
      RETURN EMPTY_BLOB();
    ELSIF MOD(len,2) = 1 THEN
      RAISE_APPLICATION_ERROR(-20000,'Input length must be even');
    END IF;

    init;                                            -- make LUT once
    DBMS_LOB.createtemporary(outb, TRUE);

    WHILE pos <= len LOOP
      slen  := LEAST(c_chunk_chars, len - pos + 1);  -- clip final piece
      DBMS_LOB.read(p_hex, slen, pos, slice);        -- 1 LOB read
      raw_buf := NULL;                               -- reset

      /* loop **inside** the VARCHAR2 slice, no more LOB calls */
      FOR i IN 1 .. slen LOOP
		  continue WHEN MOD(i,2) = 0;  -- only odd indices
        idx      := ASCII(substr(slice, i, 1))*256
                 + ASCII(substr(slice, i+1, 1));
        raw_buf  := raw_buf || g_lut(idx);           -- concat one byte
      END LOOP;

      DBMS_LOB.writeappend(outb, UTL_RAW.length(raw_buf), raw_buf); -- 1 LOB write
      pos := pos + slen;
    END LOOP;

    RETURN outb;
  END to_blob_fast;
END hex64k;
/

@plsql-error hex64k
