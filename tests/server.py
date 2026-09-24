import argparse
import socket
import os

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 8080
DEFAULT_FILENAME = "test_file.bin"
DEFAULT_FILE_SIZE = 50*1024*1024

def parse_args():
    parser = argparse.ArgumentParser(description="UDP downloader test server")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--file", default=DEFAULT_FILENAME)
    parser.add_argument("--size", type=int, default=DEFAULT_FILE_SIZE)
    parser.add_argument("--drop-every", type=int, default=0,
                        help="Ignore every nth GET request (0 disables packet loss)")
    return parser.parse_args()


def main():
    args = parse_args()

    if args.size <= 0:
        raise ValueError("File size must be positive")

    if args.drop_every < 0:
        raise ValueError("--drop-every must be non-negative")

    if not os.path.exists(args.file):
        pattern = bytes(range(256)) * 4096
        full_blocks, remainder = divmod(args.size, len(pattern))

        with open(args.file, "wb") as f:
            for _ in range(full_blocks):
                f.write(pattern)
            f.write(pattern[:remainder])

        print(f"Created test file: {args.file}", flush=True)
    elif os.path.getsize(args.file) != args.size:
        raise ValueError(f"Existing file size does not match --size: {args.file}")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.host, args.port))
    print(f"Listening on {args.host}:{args.port}", flush=True)

    with open(args.file, "rb") as f:
        request_count = 0

        while True:
            data, addr = sock.recvfrom(4096)
            msg = data.decode('utf-8').strip()
            # print(f"Received from {addr}: {msg}", flush=True)

            if msg.startswith("GET"):
                request_count += 1
                if args.drop_every > 0 and request_count % args.drop_every == 0:
                    continue

                parts = msg.split()
                start = int(parts[1])
                length = int(parts[2])

                f.seek(start)
                payload = f.read(length)

                response = f"DATA {start} {len(payload)}\n".encode() + payload
                sock.sendto(response, addr)
                # print(f"Sent {start} to {start + length}", flush=True)


if __name__ == "__main__":
    main()
