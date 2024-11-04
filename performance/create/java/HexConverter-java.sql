
set define off


CREATE OR REPLACE AND RESOLVE JAVA SOURCE NAMED "HexConverter" AS
import java.io.BufferedReader;
import java.io.StringReader;
import java.sql.Connection;
import java.sql.SQLException;
import oracle.jdbc.driver.*;
import oracle.sql.BLOB;
import java.io.IOException;
import java.io.Reader;


public class HexConverter {
    private static final int DEFAULT_BUFFER_SIZE = 8192; // Default buffer size for reading

    public static oracle.sql.BLOB hexToBlob(java.sql.Clob clob) throws SQLException {

        Connection conn = new OracleDriver().defaultConnection();

        if (clob == null || clob.length() == 0) {
            return BLOB.createTemporary(conn, true, BLOB.DURATION_SESSION); // Return an empty temporary BLOB
        }

        int clobLength = (int) clob.length();
        char[] buffer = new char[Math.min(DEFAULT_BUFFER_SIZE, clobLength)];
        byte[] byteArray = new byte[clobLength / 2];
        int byteArrayIndex = 0;

        try (Reader reader = clob.getCharacterStream();
             BufferedReader bufferedReader = new BufferedReader(reader)) {
            int charsRead;
            while ((charsRead = bufferedReader.read(buffer)) != -1) {
                for (int i = 0; i < charsRead - 1 && byteArrayIndex < byteArray.length; i += 2) {
                    String hexByte = new String(buffer, i, 2);
                    byteArray[byteArrayIndex++] = (byte) Integer.parseInt(hexByte, 16);
                }
            }
        } catch (IOException e) {
            throw new SQLException("Error processing hex string", e);
        }

        // Create a temporary BLOB and write the byte array into it
        BLOB blob = BLOB.createTemporary(conn, true, BLOB.DURATION_SESSION);
        blob.setBytes(1, byteArray);

        return blob;
    }
}
/

--show errors Java source HexConverter
@@java-errors.sql


set define on

