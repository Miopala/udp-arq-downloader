# UDP File Downloader

A C++ implementation of a reliable UDP downloader implementing a custom selective repeat ARQ protocol.

## Usage

### 1. Start the Server

Open a separate terminal in the project root and run the Python server:

```bash
python3 tests/server.py --size 5242880 --drop-every 10
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
./transport 127.0.0.1 8080 downloaded.bin 5242880
```

### 4. Verification

After the download completes, verify that the downloaded file has the same content as the source file:

```bash
diff -q downloaded.bin ../test_file.bin
```

---

**Note:** Run `python3 tests/server.py --help` to see options for the host, port, file, file size, and simulated packet loss.
