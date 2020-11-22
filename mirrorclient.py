#!/usr/bin/env python3
import socket
from struct import pack
from ipaddress import ip_network, IPv4Interface


def get_sock_ip():
    """ Not a fan of this hack, but it finds the int with default routing """
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.connect(("169.254.1.1", 80))
        return IPv4Interface(sock.getsockname()[0] + '/24')


def main():
    header = pack('BI', 3, 0)
    ip = get_sock_ip()
    for host in list(ip_network(ip.network).hosts()):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(0.1)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                sock.connect((str(host), 3490))
                sock.send(header)
                tx_sock = sock.getsockname()
                rx_sock = sock.getpeername()
                sock.close()
                break
            except (ConnectionRefusedError, OSError):
                pass
    print("My Info   :", tx_sock)
    print("Their Info:", rx_sock)
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(tx_sock)
        sock.listen(1)
        while True:
            conn, addr = sock.accept()
            with conn:
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break
                    print('Received', repr(data))


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
