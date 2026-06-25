# UDP File Downloader

A C++ implementation of a reliable UDP downloader implementing a custom selective repeat ARQ protocol.

## Usage

### 1. Start the Server

Open a separate terminal, navigate to the `tests` directory, and run the Python server:

```bash
python3 tests/server.py
```

### 2. Build the Project

Create a build directory, then compile the project:

```bash
mkdir build && cd build
cmake ..
make
```

### 3. Execute

Run the client with the required parameters:

```bash
./transport <server_ip> <port> <filename> <file_size_in_bytes>
```

### 4. Verification

After the download completes, verify that the downloaded file has the same content as `tests/test_file.bin`:

```bash
diff -q <filename> tests/test_file.bin
```

---

**Note:** You can adjust parameters such as port, file size, and output filename in `tests/server.py`.
