
set define off



CREATE OR REPLACE AND RESOLVE JAVA SOURCE NAMED "HexConverter2" AS
import java.sql.*;
import oracle.jdbc.driver.*;
import oracle.sql.BLOB;
import oracle.sql.CLOB;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;

public class HexConverter2 {

    public static void clobToBlob2(String tableName, String clobColumnName, String blobColumnName,  String rowId) throws SQLException {

        Connection conn = new OracleDriver().defaultConnection();
        BLOB blob = null;
        String query = "SELECT " + clobColumnName + " FROM " + tableName + " WHERE ROWID = ? for update";

        try (PreparedStatement pstmt = conn.prepareStatement(query)) {

            pstmt.setString(1, rowId);

            ResultSet rs = pstmt.executeQuery();
            if (rs.next()) {
                // Create a temporary BLOB
                blob = BLOB.createTemporary(conn, false, BLOB.DURATION_SESSION);

                // Open the BLOB to write data
                blob.open(BLOB.MODE_READWRITE);

                // Get the CLOB data as a reader
                try (Reader clobReader = rs.getCharacterStream(1);
                     OutputStream blobStream = blob.setBinaryStream(0L)) {

                    char[] charBuffer = new char[8192];
                    byte[] byteBuffer;
                    int charsRead;

							while ((charsRead = clobReader.read(charBuffer)) != -1) {
								// Convert hex characters to bytes
								for (int i = 0; i < charsRead; i += 2) {
									// Convert each pair of hex chars to a byte
									String hexPair = new String(charBuffer, i, 2);
									byte byteValue = (byte) Integer.parseInt(hexPair, 16);
									blobStream.write(byteValue);
							}

						}

                   blobStream.flush();
                } catch (IOException e) {
                    // Handle IOException
                    throw new SQLException("Error processing the CLOB data", e);
                }


                    // Update the BLOB column in the same row
                    String updateQuery = "UPDATE " + tableName + " SET " + blobColumnName + " = ? WHERE ROWID = ?";
                    try (PreparedStatement updatePstmt = conn.prepareStatement(updateQuery)) {
                        updatePstmt.setBlob(1, blob);
                        updatePstmt.setString(2, rowId);
                        updatePstmt.executeUpdate();
                    }

            }
        }

    }
}
/

--show errors Java source HexConverter
@@java-errors.sql


set define on

@@hex-to-blob-02


