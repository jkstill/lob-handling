# clob-to-blob Utility

This utility converts CLOB columns containing hex-encoded data into BLOBs, using the Oracle OCILib wrapper and SIMD hex decoding. It is optimized for high-throughput migration and includes logging, batching, and metadata-driven dynamic SQL.

---

## 📦 Components

- `clob-to-blob.c` — Complete C source code using OCILib
- `Makefile` — For building the binary
- `create-queue.sql` — PL/SQL to create Oracle Advanced Queue
- `enqueue-example.sql` — PL/SQL to enqueue test messages
- `clob-to-blob-columns.sql` — Table schema for CLOB/BLOB column metadata

---

## 🛠 Build Instructions

Ensure OCILib and Oracle client libraries are installed.

```bash
sudo apt install libocilib-dev libclntsh
cd generated_code
make
```

---

## 🔁 Usage

### 1. Prepare Credentials File

Create a file `ora-creds.txt`:

```text
username:blobtest
password:blobtest
database:orcl
```

### 2. Run the Program

```bash
./clob-to-blob
```

This will:
- Read `MAX_BATCH_SIZE` messages from the AQ
- For each message:
  - Load table metadata and SQL
  - Fetch the CLOBs
  - Convert hex → binary using SIMD
  - Update the BLOB columns
  - Truncate and reset the CLOBs

### 3. Logs

- Logs are saved to: `c2b-0log/pid_<PID>.log`
- Generated SQL per table is saved to: `c2b-0log/sqlgen_<tablename>.log`

---

## 🗃 Database Setup

### Create the AQ:

```sql
@create-queue.sql
```

### Create Metadata Table:

```sql
@clob-to-blob-columns.sql
```

### Enqueue Sample Message:

```sql
@enqueue-example.sql
```

---

## ✅ Notes

- Update `MAX_COLUMNS` and `MAX_BATCH_SIZE` in `clob-to-blob.c` as needed.
- Ensure your Oracle session has permissions for AQ and LOB updates.
- The program commits each batch after processing.

---

Enjoy the speed of SIMD and the flexibility of dynamic SQL!
