#!/usr/bin/env python3
"""Minimal client for the map-server admin channel (src/map/admin.cpp).

Line-based ASCII protocol over TCP:
    AUTH <secret>      authenticate (required first)
    RELOAD <target>    -> @reload<target>  (itemdb, mobdb, skilldb, script, ...)
    ATCMD <command>    run any atcommand (leading '@' optional)
    QUIT               close

Examples:
    python admin_client.py --host 127.0.0.1 RELOAD itemdb
    python admin_client.py ATCMD "broadcast Server maintenance in 5 min"
    python admin_client.py            # interactive mode
"""
import argparse
import socket
import sys

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 7799
DEFAULT_SECRET = "CHANGE-ME-please"  # must match ADMIN_SECRET in admin.cpp


def _read_line(state, sock):
    while b"\n" not in state["buf"]:
        chunk = sock.recv(4096)
        if not chunk:
            raise ConnectionError("server closed the connection")
        state["buf"] += chunk
    line, _, state["buf"] = state["buf"].partition(b"\n")
    # rAthena message text is latin-1 (Windows-1252 / ANSI), not ASCII.
    return line.decode("latin-1").rstrip("\r")


def send_line(sock, line, state):
    """Send a command; print captured output (": " lines) and return the status."""
    sock.sendall((line + "\n").encode("latin-1", "replace"))
    while True:
        reply = _read_line(state, sock)
        if reply.startswith(": "):
            print("   ", reply[2:])
        else:
            return reply


def main():
    ap = argparse.ArgumentParser(description="map-server admin channel client")
    ap.add_argument("--host", default=DEFAULT_HOST)
    ap.add_argument("--port", type=int, default=DEFAULT_PORT)
    ap.add_argument("--secret", default=DEFAULT_SECRET)
    ap.add_argument("--name", default="Admin Console",
                    help="display name used by commands like broadcast")
    ap.add_argument("command", nargs=argparse.REMAINDER,
                    help="optional one-shot command (e.g. RELOAD itemdb)")
    args = ap.parse_args()

    state = {"buf": b""}
    with socket.create_connection((args.host, args.port), timeout=10) as sock:
        print("auth:", send_line(sock, "AUTH " + args.secret, state))
        send_line(sock, "NAME " + args.name, state)

        if args.command:
            print(send_line(sock, " ".join(args.command), state))
            send_line(sock, "QUIT", state)
            return

        print("Interactive mode. Type commands (e.g. 'RELOAD itemdb'), 'quit' to exit.")
        try:
            while True:
                line = input("admin> ").strip()
                if not line:
                    continue
                print(send_line(sock, line, state))
                if line.upper() == "QUIT":
                    break
        except (EOFError, KeyboardInterrupt):
            print()


if __name__ == "__main__":
    sys.exit(main())
