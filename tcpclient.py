#!/usr/bin/env python3
import sys
import socket
from struct import pack

server_add = ("172.21.10.21", 3490)


def main():
    args = sys.argv[1:]
    if len(args) != 2:
        print('usage: --type [string]')
        sys.exit(1)
    string = bytes(args[1], 'utf-8')
    header = pack('BI', int(args[0]), len(string))
    print('TX:', string.decode("utf-8"))
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect(server_add)
        sock.send(header)
        sock.sendall(string)
        data = sock.recv(1024)
    print('RX:', data.decode("utf-8"))


if __name__ == '__main__':
    try:
        main()
    except (SystemExit, KeyboardInterrupt, GeneratorExit, Exception) as err:
        # Disabled for production, maintained for debugging
        print("Error: ", err)
        print("Error.__cause__", err.__cause__)
        print("Error.__class__", err.__class__.__name__)
        print("Error.with_traceback", err.with_traceback)
        # pass exists only for when debug off, so last line ends with a \n
        pass
