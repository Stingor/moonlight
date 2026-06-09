#!/usr/bin/env python3
"""Graphical client for the map-server admin channel (src/map/admin.cpp).

Tkinter GUI (ships with Python, no external dependencies). Connects to the
admin channel, authenticates, and lets you trigger reloads / run atcommands
with buttons or a free-form command box.

Run:  python admin_gui.py
"""
import argparse
import queue
import re
import socket
import threading
import tkinter as tk
import webbrowser
from datetime import datetime
from tkinter import ttk, scrolledtext

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = "7799"
DEFAULT_SECRET = "CHANGE-ME-please"  # must match admin_secret in conf/admin.conf

# Item database link base for clickable item names in the log.
ITEM_DB_URL = "https://moonlight-destiny.fr/index.php?page=itemdb&itemid="
# rAthena item link blob followed by "(<aegis name>, id: <id>)".
ITEML_RE = re.compile(r"<ITEML>.*?</ITEML>\s*\(([^,]+),\s*id:\s*(\d+)\)")
# Leftover item-link blobs without a parsable "(name, id)" suffix.
ITEML_STRIP_RE = re.compile(r"<ITEML>.*?</ITEML>")
# ANSI/CSI escape sequences embedded in server console messages (color "...m",
# clear-line "...K", etc.).
ANSI_RE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")
# enum msg_type (showmsg.hpp) -> server-log color tag.
MSG_TAGS = {1: "srv_status", 2: "srv_sql", 5: "srv_warn",
            6: "srv_debug", 7: "srv_err", 8: "srv_err"}
# Auto-reconnect retry interval (seconds) while "connect" is desired.
RETRY_SECONDS = 1

# One-click reload targets -> "RELOAD <target>" -> "@reload<target>"
RELOAD_TARGETS = [
    "itemdb", "mobdb", "skilldb", "script",
    "battleconf", "statusdb", "atcommand", "msgconf",
]


class AdminConnection:
    """A single persistent socket to the admin channel, with line framing."""

    def __init__(self):
        self.sock = None
        self.buf = b""
        self.lock = threading.Lock()

    def connect(self, host, port, timeout=10):
        self.sock = socket.create_connection((host, port), timeout=timeout)
        # Connect with a timeout, but switch to blocking I/O afterwards: the log
        # feed reader waits indefinitely for pushed lines, so recv() must not
        # time out during quiet periods.
        self.sock.settimeout(None)
        self.buf = b""

    def close(self):
        if self.sock:
            try:
                self.sock.close()
            finally:
                self.sock = None
        self.buf = b""

    @property
    def connected(self):
        return self.sock is not None

    def _read_line(self):
        while b"\n" not in self.buf:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError("server closed the connection")
            self.buf += chunk
        line, _, self.buf = self.buf.partition(b"\n")
        # rAthena message text is latin-1 (Windows-1252 / ANSI), not ASCII.
        return line.decode("latin-1").rstrip("\r")

    def send(self, line):
        """Send one command line; return (output_lines, status_line).

        Captured atcommand output arrives as lines prefixed with ": "; the first
        line without that prefix is the terminal "OK ..."/"ERR ..." status.
        """
        with self.lock:
            self.sock.sendall((line + "\n").encode("latin-1", "replace"))
            output = []
            while True:
                reply = self._read_line()
                if reply.startswith(": "):
                    output.append(reply[2:])
                else:
                    return output, reply


class AdminGUI:
    def __init__(self, root, args=None):
        self.root = root
        self.conn = AdminConnection()     # command channel (request/response)
        self.logconn = AdminConnection()  # dedicated server-console feed
        self.events = queue.Queue()  # (kind, text) from worker threads
        self._last_name = None       # last display name sent to the server
        self._link_counter = 0       # unique tag id per clickable link
        self._history = []           # sent free-form commands
        self._history_pos = 0        # cursor in history (== len means "new line")
        self._want_connected = False # user intent: keep (re)connecting
        self._connect_inflight = False  # a connect attempt is running
        self._retry_scheduled = False   # a reconnect is queued

        root.title("Moonlight - Admin map-server")
        root.minsize(620, 460)

        self._build_connection_bar()
        self._build_actions()
        self._build_log()
        self._set_connected(False)

        self.root.after(80, self._drain_events)
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

        self._apply_args(args)

    def _apply_args(self, args):
        """Pre-fill fields from command-line arguments and optionally connect."""
        if not args:
            return
        if args.host:
            self.host_var.set(args.host)
        if args.port:
            self.port_var.set(str(args.port))
        if args.secret is not None:
            self.secret_var.set(args.secret)
        if args.name:
            self.name_var.set(args.name)
        if args.topmost:
            self.topmost_var.set(True)
            self._toggle_topmost()
        if args.autoconnect:
            self.root.after(200, self._connect)

    # ---- UI construction ----------------------------------------------------
    def _build_connection_bar(self):
        bar = ttk.LabelFrame(self.root, text="Connexion")
        bar.pack(fill="x", padx=8, pady=(8, 4))

        ttk.Label(bar, text="Hote").grid(row=0, column=0, padx=4, pady=4, sticky="w")
        self.host_var = tk.StringVar(value=DEFAULT_HOST)
        ttk.Entry(bar, textvariable=self.host_var, width=16).grid(row=0, column=1, padx=4)

        ttk.Label(bar, text="Port").grid(row=0, column=2, padx=4, sticky="w")
        self.port_var = tk.StringVar(value=DEFAULT_PORT)
        ttk.Entry(bar, textvariable=self.port_var, width=7).grid(row=0, column=3, padx=4)

        ttk.Label(bar, text="Secret").grid(row=0, column=4, padx=4, sticky="w")
        self.secret_var = tk.StringVar(value=DEFAULT_SECRET)
        ttk.Entry(bar, textvariable=self.secret_var, width=18, show="*").grid(row=0, column=5, padx=4)

        self.connect_btn = ttk.Button(bar, text="Connecter", command=self._toggle_connect)
        self.connect_btn.grid(row=0, column=6, padx=8)

        self.status_lbl = ttk.Label(bar, text="deconnecte", foreground="#b00")
        self.status_lbl.grid(row=0, column=7, padx=4)

        self.topmost_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(bar, text="Toujours au premier plan",
                        variable=self.topmost_var,
                        command=self._toggle_topmost).grid(
            row=1, column=0, columnspan=4, padx=4, pady=(0, 4), sticky="w")

        ttk.Label(bar, text="Nom d'affichage").grid(row=1, column=4, padx=4, sticky="e")
        self.name_var = tk.StringVar(value="Admin Console")
        ttk.Entry(bar, textvariable=self.name_var, width=20).grid(
            row=1, column=5, columnspan=3, padx=4, pady=(0, 4), sticky="w")

    def _build_actions(self):
        # Quick reload buttons
        reloads = ttk.LabelFrame(self.root, text="Reloads rapides")
        reloads.pack(fill="x", padx=8, pady=4)
        self.reload_btns = []
        cols = 4
        for i, target in enumerate(RELOAD_TARGETS):
            b = ttk.Button(reloads, text=target,
                           command=lambda t=target: self._send_async("RELOAD " + t))
            b.grid(row=i // cols, column=i % cols, padx=4, pady=4, sticky="ew")
            self.reload_btns.append(b)
        for c in range(cols):
            reloads.columnconfigure(c, weight=1)

        # Free-form atcommand
        free = ttk.LabelFrame(self.root, text="Commande libre")
        free.pack(fill="x", padx=8, pady=4)
        free.columnconfigure(0, weight=1)

        self.cmd_var = tk.StringVar()
        self.cmd_entry = ttk.Entry(free, textvariable=self.cmd_var)
        self.cmd_entry.grid(row=0, column=0, padx=4, pady=6, sticky="ew")
        self.cmd_entry.bind("<Return>", lambda e: self._send_atcmd())
        self.cmd_entry.bind("<Up>", self._history_up)
        self.cmd_entry.bind("<Down>", self._history_down)

        self.atcmd_btn = ttk.Button(free, text="ATCMD", command=self._send_atcmd)
        self.atcmd_btn.grid(row=0, column=1, padx=4)
        ttk.Label(free, text="ex: broadcast Maintenance dans 5 min  |  kick PlayerName").grid(
            row=1, column=0, columnspan=2, padx=6, sticky="w")

    def _build_log(self):
        paned = ttk.PanedWindow(self.root, orient="vertical")
        paned.pack(fill="both", expand=True, padx=8, pady=(4, 8))

        cmd_frame = ttk.LabelFrame(paned, text="Journal (commandes)")
        self.log = scrolledtext.ScrolledText(cmd_frame, height=10, state="disabled", wrap="word")
        self.log.pack(fill="both", expand=True, padx=4, pady=4)
        self.log.tag_config("sent", foreground="#0050c0")
        self.log.tag_config("ok", foreground="#0a0")
        self.log.tag_config("err", foreground="#c00")
        self.log.tag_config("info", foreground="#666")
        self.log.tag_config("out", foreground="#222")
        paned.add(cmd_frame, weight=1)

        srv_frame = ttk.LabelFrame(paned, text="Console serveur")
        self.srvlog = scrolledtext.ScrolledText(srv_frame, height=10, state="disabled",
                                                wrap="word", background="#111", foreground="#ccc")
        self.srvlog.pack(fill="both", expand=True, padx=4, pady=4)
        self.srvlog.tag_config("srv_status", foreground="#5c5")
        self.srvlog.tag_config("srv_sql", foreground="#c5c")
        self.srvlog.tag_config("srv_warn", foreground="#db3")
        self.srvlog.tag_config("srv_debug", foreground="#5cc")
        self.srvlog.tag_config("srv_err", foreground="#e55")
        self.srvlog.tag_config("srv_def", foreground="#ccc")
        paned.add(srv_frame, weight=1)

    # ---- logging ------------------------------------------------------------
    def _log(self, text, tag="info"):
        ts = datetime.now().strftime("%H:%M:%S")
        self.log.configure(state="normal")
        self.log.insert("end", f"[{ts}] {text}\n", tag)
        self.log.see("end")
        self.log.configure(state="disabled")

    def _log_output(self, text):
        """Log a captured atcommand line, turning <ITEML> item links into
        clickable links to the Moonlight-Destiny item database."""
        ts = datetime.now().strftime("%H:%M:%S")
        self.log.configure(state="normal")
        self.log.insert("end", f"[{ts}]     ", "out")

        pos = 0
        for m in ITEML_RE.finditer(text):
            if m.start() > pos:
                self._insert_plain(text[pos:m.start()])
            name, item_id = m.group(1), m.group(2)
            self._insert_link(name, ITEM_DB_URL + item_id)
            self.log.insert("end", f" (id: {item_id})", "out")
            pos = m.end()
        if pos < len(text):
            self._insert_plain(text[pos:])

        self.log.insert("end", "\n", "out")
        self.log.see("end")
        self.log.configure(state="disabled")

    def _insert_plain(self, text):
        # Drop any stray item-link blob we could not turn into a link.
        self.log.insert("end", ITEML_STRIP_RE.sub("", text), "out")

    def _log_server(self, payload):
        """Display one server console line ("<flag> <message>") in the server panel."""
        flag_str, _, msg = payload.partition(" ")
        try:
            flag = int(flag_str)
        except ValueError:
            flag, msg = 0, payload
        msg = ANSI_RE.sub("", msg)
        tag = MSG_TAGS.get(flag, "srv_def")
        ts = datetime.now().strftime("%H:%M:%S")
        self.srvlog.configure(state="normal")
        self.srvlog.insert("end", f"[{ts}] {msg}\n", tag)
        self.srvlog.see("end")
        self.srvlog.configure(state="disabled")

    def _log_reader(self):
        """Background loop: read the pushed server-console feed on logconn."""
        try:
            while self.logconn.connected:
                line = self.logconn._read_line()
                if line.startswith("* "):
                    self.events.put(("serverlog", line[2:]))
        except Exception as exc:  # noqa: BLE001
            # The feed dropping usually means the server went down/restarted;
            # trigger the supervised reconnect.
            self.events.put(("lost", f"[feed] flux console interrompu: {exc!r}"))

    def _insert_link(self, label, url):
        tag = f"link-{self._link_counter}"
        self._link_counter += 1
        self.log.tag_config(tag, foreground="#06c", underline=True)
        self.log.tag_bind(tag, "<Button-1>", lambda e, u=url: webbrowser.open(u))
        self.log.tag_bind(tag, "<Enter>", lambda e: self.log.config(cursor="hand2"))
        self.log.tag_bind(tag, "<Leave>", lambda e: self.log.config(cursor=""))
        self.log.insert("end", label, (tag,))

    def _drain_events(self):
        # The reschedule is in a finally so a handler exception can never kill
        # the event pump (which would freeze all log/command updates).
        try:
            while True:
                try:
                    kind, text = self.events.get_nowait()
                except queue.Empty:
                    break
                try:
                    self._handle_event(kind, text)
                except Exception as exc:  # noqa: BLE001
                    try:
                        self._log(f"[GUI] erreur sur '{kind}': {exc!r}", "err")
                    except Exception:  # noqa: BLE001
                        pass
        finally:
            self.root.after(80, self._drain_events)

    def _handle_event(self, kind, text):
        if kind == "output":
            self._log_output(text)
        elif kind == "serverlog":
            self._log_server(text)
        elif kind == "reply":
            self._log(text, "ok" if text.startswith("OK") else "err")
        elif kind == "info":
            self._log(text, "info")
        elif kind == "connected":
            self._connect_inflight = False
            if not self._want_connected:
                # user cancelled while connecting -> drop it
                self.conn.close()
                self.logconn.close()
                self._set_connected(False)
            else:
                self._set_connected(True)
                self._log(text, "ok")
        elif kind == "lost":
            self._handle_lost(text)

    # ---- connection handling ------------------------------------------------
    def _set_connected(self, on):
        if not on:
            self._last_name = None  # force a NAME resync on next connection
        if on:
            self.status_lbl.config(text="connecte", foreground="#0a0")
        elif self._want_connected:
            self.status_lbl.config(text="connexion...", foreground="#c80")
        else:
            self.status_lbl.config(text="deconnecte", foreground="#b00")
        self.connect_btn.config(text="Deconnecter" if (on or self._want_connected) else "Connecter")
        state = "normal" if on else "disabled"
        for b in self.reload_btns:
            b.config(state=state)
        self.atcmd_btn.config(state=state)

    def _toggle_topmost(self):
        self.root.attributes("-topmost", self.topmost_var.get())

    def _toggle_connect(self):
        if self._want_connected:
            self._disconnect()
        else:
            self._connect()

    def _connect(self):
        # User intent: stay connected, retrying until it works.
        self._want_connected = True
        self._set_connected(False)
        self._start_connect()

    def _start_connect(self):
        if not self._want_connected or self.conn.connected or self._connect_inflight:
            return
        host = self.host_var.get().strip()
        secret = self.secret_var.get()
        try:
            port = int(self.port_var.get().strip())
        except ValueError:
            self.events.put(("lost", "Port invalide."))
            return

        self._connect_inflight = True
        self._log(f"Connexion a {host}:{port} ...", "info")

        def worker():
            try:
                self.conn.connect(host, port)
                _out, reply = self.conn.send("AUTH " + secret)
                if not reply.startswith("OK"):
                    raise RuntimeError("Echec AUTH: " + reply)
                # Second connection: live server console feed (best-effort).
                try:
                    self.logconn.connect(host, port)
                    _o, rep = self.logconn.send("AUTH " + secret)
                    if not rep.startswith("OK"):
                        raise RuntimeError("AUTH refuse: " + rep)
                    _o, rep = self.logconn.send("LOG ON")
                    if not rep.startswith("OK"):
                        raise RuntimeError("LOG ON refuse: " + rep)
                    threading.Thread(target=self._log_reader, daemon=True).start()
                    self.events.put(("info", "[feed] console serveur connectee."))
                except Exception as exc:  # noqa: BLE001
                    self.logconn.close()
                    self.events.put(("info", f"[feed] console serveur indisponible: {exc!r}"))
                self.events.put(("connected", "Authentifie."))
            except Exception as exc:  # noqa: BLE001
                self.conn.close()
                self.logconn.close()
                self.events.put(("lost", f"Connexion impossible: {exc}"))

        threading.Thread(target=worker, daemon=True).start()

    def _handle_lost(self, reason):
        self._connect_inflight = False
        self.conn.close()
        self.logconn.close()
        self._set_connected(False)
        if self._want_connected:
            self._log(reason, "err")
            self._schedule_retry()

    def _schedule_retry(self):
        if self._retry_scheduled or not self._want_connected:
            return
        self._retry_scheduled = True
        self._log(f"Nouvelle tentative dans {RETRY_SECONDS}s...", "info")
        self.root.after(RETRY_SECONDS * 1000, self._retry_now)

    def _retry_now(self):
        self._retry_scheduled = False
        self._start_connect()

    def _disconnect(self):
        self._want_connected = False
        self._set_connected(False)
        self._log("Deconnecte.", "info")

        def worker():
            try:
                self.conn.send("QUIT")
            except Exception:  # noqa: BLE001
                pass
            self.conn.close()
            self.logconn.close()
        threading.Thread(target=worker, daemon=True).start()

    # ---- sending commands ---------------------------------------------------
    def _send_atcmd(self):
        cmd = self.cmd_var.get().strip()
        if cmd:
            if not self._history or self._history[-1] != cmd:
                self._history.append(cmd)
            self._history_pos = len(self._history)
            self._send_async("ATCMD " + cmd)
            self.cmd_var.set("")

    def _history_up(self, event=None):
        if self._history and self._history_pos > 0:
            self._history_pos -= 1
            self._set_cmd(self._history[self._history_pos])
        return "break"

    def _history_down(self, event=None):
        if not self._history:
            return "break"
        if self._history_pos < len(self._history) - 1:
            self._history_pos += 1
            self._set_cmd(self._history[self._history_pos])
        else:
            self._history_pos = len(self._history)
            self._set_cmd("")
        return "break"

    def _set_cmd(self, text):
        self.cmd_var.set(text)
        self.cmd_entry.icursor("end")

    def _send_async(self, line):
        if not self.conn.connected:
            self._log("Pas connecte.", "err")
            return
        name = self.name_var.get().strip() or "Admin Console"
        self._log("> " + line, "sent")

        def worker():
            try:
                # Keep the server-side display name in sync before each command.
                if name != self._last_name:
                    self.conn.send("NAME " + name)
                    self._last_name = name
                output, reply = self.conn.send(line)
                for out_line in output:
                    self.events.put(("output", out_line))
                self.events.put(("reply", reply))
            except Exception as exc:  # noqa: BLE001
                self.conn.close()
                self.events.put(("lost", f"Erreur reseau: {exc}"))

        threading.Thread(target=worker, daemon=True).start()

    def _on_close(self):
        self._want_connected = False  # stop any pending reconnect
        if self.conn.connected:
            try:
                self.conn.send("QUIT")
            except Exception:  # noqa: BLE001
                pass
            self.conn.close()
        self.logconn.close()
        self.root.destroy()


def parse_args():
    ap = argparse.ArgumentParser(description="Moonlight - Admin map-server (GUI)")
    ap.add_argument("--host", help="server host (default in-app: %s)" % DEFAULT_HOST)
    ap.add_argument("--port", type=int, help="admin port (default in-app: %s)" % DEFAULT_PORT)
    ap.add_argument("--secret", help="admin password (or its plaintext if MD5 mode)")
    ap.add_argument("--name", help="display name (e.g. for broadcast)")
    ap.add_argument("--autoconnect", action="store_true",
                    help="connect automatically on launch")
    ap.add_argument("--topmost", action="store_true",
                    help="keep the window always on top")
    return ap.parse_args()


def main():
    args = parse_args()
    root = tk.Tk()
    AdminGUI(root, args)
    root.mainloop()


if __name__ == "__main__":
    main()
