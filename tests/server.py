import socket
import os

PORT = 8080
FILENAME = "test_file.bin"
FILE_SIZE = 50*1024*1024

if not os.path.exists(FILENAME):
    with open(FILENAME, "wb") as f:
        f.write(bytes([i % 256 for i in range(FILE_SIZE)]))
    print("Created file with random characters", flush=True)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("127.0.0.1", PORT))
print(f"Listening on 127.0.0.1:{PORT}", flush=True)

while True:
    data, addr = sock.recvfrom(4096)
    msg = data.decode('utf-8').strip()
    # print(f"Received from {addr}: {msg}", flush=True)

    if msg.startswith("GET"):
        parts = msg.split()
        start = int(parts[1])
        length = int(parts[2])

        with open(FILENAME, "rb") as f:
            f.seek(start)
            payload = f.read(length)

        response = f"DATA {start} {len(payload)}\n".encode() + payload
        sock.sendto(response, addr)
        # print(f"Sent {start} to {start + length}", flush=True)