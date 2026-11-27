#!/usr/bin/env python3
"""Graphical client for the map-server admin channel (src/map/admin.cpp).

Tkinter GUI (ships with Python, no external dependencies). Connects to the
admin channel, authenticates, and lets you trigger reloads / run atcommands
with buttons or a free-form command box.

Files next to the script (or next to MoonlightAdmin.exe once frozen):
    admin.env           ADMIN_SECRET=...  (git-ignored, see admin.env.example)
    MoonlightAdmin.ini  window, filters and favorites, saved on exit
    logs/               daily server-console logs, when "Enregistrer" is on

Run:  python admin_gui.py
"""
import argparse
import configparser
import os
import queue
import re
import socket
import sys
import threading
import tkinter as tk
import webbrowser
from collections import deque
from datetime import datetime
from tkinter import ttk, scrolledtext, simpledialog, messagebox, font as tkfont

from admin_env import ENV_VAR, app_dir, default_secret, env_path
from atcmd_catalog import COMMAND_SYMBOLS, AtcommandCatalog, clean_char_name
from build_panel import SETTINGS_SECTION as BUILD_SECTION, BuildWindow
from log_panel import SETTINGS_SECTION as LOGS_SECTION, LogWindow
from maintenance_panel import SETTINGS_SECTION as MAINTENANCE_SECTION, MaintenanceWindow
from npc_panel import NOT_CONNECTED_REPLY, SETTINGS_SECTION as NPC_SECTION, NpcWindow
from player_actions import add_player_menu
from players_panel import SETTINGS_SECTION as PLAYERS_SECTION, PlayersWindow
from ssh_panel import SETTINGS_SECTION as SSH_SECTION, SshWindow
from status_bar import SETTINGS_SECTION as STATUS_SECTION, StatusBar
from tables_panel import SETTINGS_SECTION as TABLES_SECTION, TablesWindow

DEFAULT_HOST = "192.168.1.13"
DEFAULT_PORT = "7799"
DEFAULT_NAME = "Admin Console"

SETTINGS_PATH = os.path.join(app_dir(), "MoonlightAdmin.ini")
LOG_DIR = os.path.join(app_dir(), "logs")

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
# Level checkboxes: label -> msg_type flags it covers (0 = no/unknown flag,
# 3 = info, 4 = notice).
LOG_LEVELS = [
    ("Info", (0, 3, 4)),
    ("Status", (1,)),
    ("SQL", (2,)),
    ("Warning", (5,)),
    ("Debug", (6,)),
    ("Erreur", (7, 8)),
]
# Auto-reconnect retry interval (seconds) while "connect" is desired.
RETRY_SECONDS = 1

# Console-only feeds of the other two servers (src/common/admin_channel.cpp):
# (source name, MoonlightAdmin.ini key under [connexion], default port). Same
# host and secret as the map-server. They live independently of the map
# connection: a map-server restart does not cut them, and they keep retrying
# while the map-server is down, which is exactly when their console matters.
SIDE_FEEDS = [
    ("char", "port_char", "7798"),
    ("login", "port_login", "7797"),
]
SIDE_RETRY_SECONDS = 5
# Server checkboxes of the console panel: source name -> label.
SERVER_SOURCES = [("map", "Map"), ("char", "Char"), ("login", "Login")]

# One-click reload targets -> "RELOAD <target>" -> "@reload<target>".
# Rendered alphabetically as compact buttons (see _build_actions).
RELOAD_TARGETS = [
    "achievementdb", "atcommand", "attendancedb", "barterdb",
    "battleconf", "instancedb", "itemdb", "mobdb",
    "msgconf", "pcdb", "questdb", "script",
    "skilldb", "statusdb",
]

# Favorites used until the user edits them (then MoonlightAdmin.ini wins).
DEFAULT_FAVORITES = [
    ("Joueurs par carte", "users"),
    ("Qui (niveau / job)", "who2"),
    ("Qui (AID / carte)", "who3"),
]
FAV_COLS = 5
CMD_HINT_DEFAULT = ("ex: broadcast Maintenance dans 5 min  |  kick PlayerName  "
                    "—  Tab complète la commande")

# Server-console line filters: (substring, checkbox label). A line containing
# the substring is shown only while its box is checked; lines matching no filter
# are always shown. Add a tuple here to expose another toggle.
LOG_FILTERS = [
    ("[CheatDetector]", "CheatDetector"),
    ("[Heuristique]", "Heuristique"),
    ("[moonclient]", "moonclient"),
]
# Lines that ring the bell / flash the taskbar while "Alertes" is checked,
# and that are always drawn with the alert background.
ALERT_PATTERNS = [
    "[CheatDetector]",
    "[Heuristique]",
    "[moonclient] refused",
]
# Keep at most this many server-console lines buffered so that toggling a filter
# can re-display the past lines that match (older lines are dropped).
SRVLOG_BUFFER = 5000
# Delay before re-filtering while typing in the search box (ms).
SEARCH_DEBOUNCE_MS = 250

# Colors of the command journal per theme (the server console stays dark).
LOG_COLORS = {
    False: {"bg": "#ffffff", "fg": "#222", "sent": "#0050c0", "ok": "#0a0",
            "err": "#c00", "info": "#666", "out": "#222", "link": "#06c"},
    True: {"bg": "#1e1e1e", "fg": "#ddd", "sent": "#6af", "ok": "#5c5",
           "err": "#f66", "info": "#999", "out": "#ddd", "link": "#6bf"},
}
# Liste déroulante des combos en thème clair : les couleurs du système.
LIGHT_LISTBOX = ({"background": "SystemWindow", "foreground": "SystemWindowText",
                  "selectBackground": "SystemHighlight", "selectForeground": "SystemHighlightText"}
                 if sys.platform == "win32" else
                 {"background": "white", "foreground": "black",
                  "selectBackground": "#4a6984", "selectForeground": "white"})
DARK = {"bg": "#2b2b2b", "fg": "#ddd", "field": "#1e1e1e",
        "button": "#3a3a3a", "active": "#4a4a4a", "border": "#555"}


def _flash_taskbar(root):
    """Flash the window's taskbar button until it gets focus (Windows only)."""
    if sys.platform != "win32":
        return
    try:
        import ctypes
        from ctypes import wintypes

        class FLASHWINFO(ctypes.Structure):
            _fields_ = [("cbSize", wintypes.UINT), ("hwnd", wintypes.HWND),
                        ("dwFlags", wintypes.DWORD), ("uCount", wintypes.UINT),
                        ("dwTimeout", wintypes.DWORD)]

        hwnd = int(root.wm_frame(), 16)
        info = FLASHWINFO(ctypes.sizeof(FLASHWINFO), hwnd,
                          0x3 | 0xC,  # FLASHW_ALL | FLASHW_TIMERNOFG
                          0, 0)
        ctypes.windll.user32.FlashWindowEx(ctypes.byref(info))
    except Exception:  # noqa: BLE001 - cosmetic, never break the log feed
        pass


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


class Settings:
    """MoonlightAdmin.ini wrapper. Never stores the admin secret."""

    def __init__(self, path):
        self.path = path
        self.cp = configparser.ConfigParser(interpolation=None)
        self.cp.optionxform = str  # keep favorite labels' case
        try:
            self.cp.read(path, encoding="utf-8")
        except (configparser.Error, UnicodeDecodeError):
            self.cp = configparser.ConfigParser(interpolation=None)
            self.cp.optionxform = str

    def get(self, section, key, default=""):
        return self.cp.get(section, key, fallback=default)

    def get_bool(self, section, key, default=False):
        try:
            return self.cp.getboolean(section, key, fallback=default)
        except ValueError:
            return default

    def favorites(self):
        if not self.cp.has_section("favoris"):
            return list(DEFAULT_FAVORITES)
        return list(self.cp.items("favoris"))

    def save(self, data, favorites):
        cp = configparser.ConfigParser(interpolation=None)
        cp.optionxform = str
        for section, values in data.items():
            cp[section] = {k: str(v) for k, v in values.items()}
        cp["favoris"] = {label: cmd for label, cmd in favorites}
        tmp = self.path + ".tmp"
        with open(tmp, "w", encoding="utf-8") as f:
            cp.write(f)
        os.replace(tmp, self.path)


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
        self.catalog = None          # catalogue des atcommandes, lu après le panneau Tables
        self._want_connected = False # user intent: keep (re)connecting
        self._connect_inflight = False  # a connect attempt is running
        self._retry_scheduled = False   # a reconnect is queued
        # char/login console feeds: one connection each, plus a generation
        # counter so a dead reader thread cannot close its successor, and the
        # last reported state (None / True / False) to log changes only once.
        self.side_conns = {src: AdminConnection() for src, _k, _p in SIDE_FEEDS}
        self.side_ports = {src: tk.StringVar(value=port) for src, _k, port in SIDE_FEEDS}
        self._side_gen = {src: 0 for src, _k, _p in SIDE_FEEDS}
        self._side_state = {src: None for src, _k, _p in SIDE_FEEDS}
        self._side_retry = set()     # sources with a reconnect queued
        # Server-console history: (timestamp, flag, color tag, message, alert,
        # source) kept so the filters can re-render the panel retroactively.
        self._srv_lines = deque(maxlen=SRVLOG_BUFFER)
        self._src_vars = {}          # source name -> BooleanVar (checked = shown)
        self._log_filter_vars = {}   # filter substring -> BooleanVar (checked = shown)
        self._level_vars = {}        # level label -> BooleanVar (checked = shown)
        self._flag_level = {f: label for label, flags in LOG_LEVELS for f in flags}
        self._search_re = None       # compiled search, None when the box is empty
        self._search_job = None      # pending debounced re-filter
        self._alert_count = 0
        self._logfile = None         # (date string, open file) of the daily log
        self.settings = Settings(SETTINGS_PATH)
        self.favorites = self.settings.favorites()
        self._light_theme = ttk.Style(root).theme_use()

        root.title("Moonlight - Admin map-server")
        root.minsize(680, 520)

        self._build_connection_bar()
        self._build_actions()
        self._build_log()
        self.tables = TablesWindow(root, self.settings, self._bind_copy_menu)
        # Le catalogue des atcommandes vit dans le dépôt moonclient du panneau Tables.
        self.catalog = AtcommandCatalog.load(self.tables.vars["repo"].get().strip())
        self.builder = BuildWindow(root, self.settings, self._bind_copy_menu, self.tables.vars)
        self.ssh = SshWindow(root, self.settings, self._bind_copy_menu,
                             open_logs=lambda: self.logs.show(),
                             open_npc=lambda: self.npc.show())
        self.logs = LogWindow(root, self.settings, self._bind_copy_menu, self.ssh,
                              on_alert=lambda text, urgent: self._alert("journaux", text, urgent))
        self.npc = NpcWindow(root, self.settings, self._bind_copy_menu, self.ssh, self.send_admin)
        self.status_bar = StatusBar(root, self.connection_bar, self.settings, self.ssh, self._status_alert)
        self.players = PlayersWindow(root, self.settings, self.send_quiet,
                                     lambda cmd: self._send_async("ATCMD " + cmd))
        self.maintenance = MaintenanceWindow(root, self.settings, self.ssh, self.send_admin,
                                             lambda text, urgent: self._alert("maintenance", text, urgent))
        self._load_settings()
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

    # ---- settings -----------------------------------------------------------
    def _load_settings(self):
        s = self.settings
        self.host_var.set(s.get("connexion", "host", DEFAULT_HOST))
        self.port_var.set(s.get("connexion", "port", DEFAULT_PORT))
        for src, key, port in SIDE_FEEDS:
            self.side_ports[src].set(s.get("connexion", key, port))
        self.name_var.set(s.get("connexion", "name", DEFAULT_NAME))
        self.topmost_var.set(s.get_bool("fenetre", "topmost"))
        self._toggle_topmost()
        self.dark_var.set(s.get_bool("fenetre", "dark"))
        self._apply_theme()
        geometry = s.get("fenetre", "geometry")
        if geometry:
            try:
                self.root.geometry(geometry)
            except tk.TclError:
                pass
        for pattern, label in LOG_FILTERS:
            self._log_filter_vars[pattern].set(s.get_bool("filtres", label, True))
        for label, _flags in LOG_LEVELS:
            self._level_vars[label].set(s.get_bool("niveaux", label, True))
        for src, label in SERVER_SOURCES:
            self._src_vars[src].set(s.get_bool("serveurs", label, True))
        self.regex_var.set(s.get_bool("console", "regex"))
        self.only_matches_var.set(s.get_bool("console", "only_matches"))
        self.pause_var.set(False)
        self.savelog_var.set(s.get_bool("console", "save_log"))
        self.alerts_var.set(s.get_bool("console", "alerts", True))
        self.reloads_open_var.set(s.get_bool("fenetre", "reloads_open"))
        self._show_reloads()

    def _save_settings(self):
        data = {
            "connexion": {"host": self.host_var.get().strip(),
                          "port": self.port_var.get().strip(),
                          **{key: self.side_ports[src].get().strip()
                             for src, key, _p in SIDE_FEEDS},
                          "name": self.name_var.get().strip()},
            "fenetre": {"geometry": self.root.geometry(),
                        "topmost": self.topmost_var.get(),
                        "dark": self.dark_var.get(),
                        "reloads_open": self.reloads_open_var.get()},
            "filtres": {label: self._log_filter_vars[p].get() for p, label in LOG_FILTERS},
            "niveaux": {label: self._level_vars[label].get() for label, _ in LOG_LEVELS},
            "serveurs": {label: self._src_vars[src].get() for src, label in SERVER_SOURCES},
            "console": {"regex": self.regex_var.get(),
                        "only_matches": self.only_matches_var.get(),
                        "save_log": self.savelog_var.get(),
                        "alerts": self.alerts_var.get()},
            TABLES_SECTION: self.tables.settings_data(),
            BUILD_SECTION: self.builder.settings_data(),
            SSH_SECTION: self.ssh.settings_data(),
            LOGS_SECTION: self.logs.settings_data(),
            NPC_SECTION: self.npc.settings_data(),
            STATUS_SECTION: self.status_bar.settings_data(),
            MAINTENANCE_SECTION: self.maintenance.settings_data(),
            PLAYERS_SECTION: self.players.settings_data(),
        }
        try:
            self.settings.save(data, self.favorites)
        except OSError as exc:
            self._log(f"Reglages non sauvegardes ({SETTINGS_PATH}): {exc}", "err")

    # ---- UI construction ----------------------------------------------------
    def _build_connection_bar(self):
        bar = ttk.LabelFrame(self.root, text="Connexion")
        bar.pack(fill="x", padx=8, pady=(8, 4))
        self.connection_bar = bar  # le bandeau d'état se place dessous

        ttk.Label(bar, text="Hote").grid(row=0, column=0, padx=4, pady=4, sticky="w")
        self.host_var = tk.StringVar(value=DEFAULT_HOST)
        ttk.Entry(bar, textvariable=self.host_var, width=16).grid(row=0, column=1, padx=4)

        ttk.Label(bar, text="Port").grid(row=0, column=2, padx=4, sticky="w")
        self.port_var = tk.StringVar(value=DEFAULT_PORT)
        ttk.Entry(bar, textvariable=self.port_var, width=7).grid(row=0, column=3, padx=4)

        ttk.Label(bar, text="Secret").grid(row=0, column=4, padx=4, sticky="w")
        self.secret_var = tk.StringVar(value=default_secret())
        ttk.Entry(bar, textvariable=self.secret_var, width=18, show="*").grid(row=0, column=5, padx=4)

        self.connect_btn = ttk.Button(bar, text="Connecter", command=self._toggle_connect)
        self.connect_btn.grid(row=0, column=6, padx=8)

        self.status_lbl = ttk.Label(bar, text="deconnecte", foreground="#b00")
        self.status_lbl.grid(row=0, column=7, padx=4)

        opts = ttk.Frame(bar)
        opts.grid(row=1, column=0, columnspan=4, padx=4, pady=(0, 4), sticky="w")
        self.topmost_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(opts, text="Toujours au premier plan",
                        variable=self.topmost_var,
                        command=self._toggle_topmost).pack(side="left")
        self.dark_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(opts, text="Theme sombre", variable=self.dark_var,
                        command=self._apply_theme).pack(side="left", padx=(10, 0))
        ttk.Button(opts, text="Tables moonclient…",
                   command=lambda: self.tables.show()).pack(side="left", padx=(10, 0))
        ttk.Button(opts, text="Build client…",
                   command=lambda: self.builder.show()).pack(side="left", padx=(6, 0))
        ttk.Button(opts, text="Serveur (SSH)…",
                   command=lambda: self.ssh.show()).pack(side="left", padx=(6, 0))
        ttk.Button(opts, text="Journaux…",
                   command=lambda: self.logs.show()).pack(side="left", padx=(6, 0))
        ttk.Button(opts, text="Atelier NPC…",
                   command=lambda: self.npc.show()).pack(side="left", padx=(6, 0))
        ttk.Button(opts, text="Joueurs…",
                   command=lambda: self.players.show()).pack(side="left", padx=(6, 0))
        ttk.Button(opts, text="Maintenance…",
                   command=lambda: self.maintenance.show()).pack(side="left", padx=(6, 0))

        ttk.Label(bar, text="Nom d'affichage").grid(row=1, column=4, padx=4, sticky="e")
        self.name_var = tk.StringVar(value=DEFAULT_NAME)
        ttk.Entry(bar, textvariable=self.name_var, width=20).grid(
            row=1, column=5, columnspan=3, padx=4, pady=(0, 4), sticky="w")

    def _build_actions(self):
        # Quick reload buttons, alphabetical and compact.
        # Collapsible: the header label toggles the button grid (state kept in
        # MoonlightAdmin.ini). Frequent reloads belong in the favorites
        # ("RELOAD itemdb" is sent as-is), which stay visible.
        self.reloads_open_var = tk.BooleanVar(value=False)
        # The header is a plain label above the grid, not a LabelFrame title: an
        # emptied LabelFrame shrinks to its border line and clips its title.
        self.reloads_box = ttk.Frame(self.root)
        self.reloads_box.pack(fill="x", padx=8, pady=4)
        self.reloads_header = ttk.Label(self.reloads_box, cursor="hand2")
        self.reloads_header.pack(anchor="w")
        self.reloads_header.bind("<Button-1>", lambda e: self._toggle_reloads())
        reloads = ttk.Frame(self.reloads_box, relief="groove", borderwidth=2, padding=2)
        self.reloads_inner = reloads

        # Dedicated small style so the larger target set stays on a few short
        # rows: smaller font + tight padding shrink each button.
        base_family = tkfont.nametofont("TkDefaultFont").actual("family")
        self._small_font = (base_family, 8)
        ttk.Style(self.root).configure("Reload.TButton",
                                       font=self._small_font, padding=(2, 1))

        self.reload_btns = []
        cols = 5
        for i, target in enumerate(sorted(RELOAD_TARGETS)):
            b = ttk.Button(reloads, text=target, style="Reload.TButton",
                           command=lambda t=target: self._send_async("RELOAD " + t))
            b.grid(row=i // cols, column=i % cols, padx=2, pady=2, sticky="ew")
            self.reload_btns.append(b)
        for c in range(cols):
            reloads.columnconfigure(c, weight=1)
        self._show_reloads()

        # Favorite commands (MoonlightAdmin.ini [favoris]).
        self.fav_frame = ttk.LabelFrame(self.root, text="Favoris  (clic droit : modifier / supprimer)")
        self.fav_frame.pack(fill="x", padx=8, pady=4)
        for c in range(FAV_COLS):
            self.fav_frame.columnconfigure(c, weight=1)
        self.fav_btns = []
        self._render_favorites()

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

        self.cmd_entry.bind("<Tab>", self._complete_cmd)

        self.atcmd_btn = ttk.Button(free, text="ATCMD", command=self._send_atcmd)
        self.atcmd_btn.grid(row=0, column=1, padx=4)
        ttk.Button(free, text="+ Favori", command=self._add_favorite).grid(row=0, column=2, padx=(0, 4))
        # L'aide de la commande en cours de frappe, tirée du catalogue.
        self.cmd_hint_lbl = ttk.Label(free, text=CMD_HINT_DEFAULT, foreground="#888")
        self.cmd_hint_lbl.grid(row=1, column=0, columnspan=3, padx=6, sticky="w")
        self.cmd_var.trace_add("write", lambda *_: self._update_cmd_hint())
        self._tab_state = None  # (texte posé par Tab, candidats, rang) pour faire défiler

    def _update_cmd_hint(self):
        text = self.cmd_var.get()
        hint = self.catalog.hint(text) if self.catalog is not None and text.strip() else ""
        self.cmd_hint_lbl.config(text=hint or CMD_HINT_DEFAULT)

    def _complete_cmd(self, _event=None):
        """Tab complète le nom de la commande ; répété, il fait défiler les
        candidats que le préfixe laisse."""
        if self.catalog is None:
            return "break"
        text = self.cmd_var.get()
        body = text.lstrip()
        if " " in body:
            return "break"  # le nom est écrit : rien à compléter
        symbol = body[:1] if body[:1] in COMMAND_SYMBOLS else ""
        if self._tab_state is not None and self._tab_state[0] == text:
            _set, candidates, rank = self._tab_state
            rank = (rank + 1) % len(candidates)
            if symbol + candidates[rank] == text:
                # Le préfixe commun est lui-même une commande (« kick ») : on
                # passe à la suivante plutôt que de laisser le texte tel quel.
                rank = (rank + 1) % len(candidates)
            new = symbol + candidates[rank]
        else:
            candidates = self.catalog.matches(body[len(symbol):])
            if not candidates:
                self.root.bell()
                return "break"
            common = os.path.commonprefix(candidates)
            rank = 0
            if len(candidates) == 1:
                new = symbol + candidates[0] + " "
            elif len(common) > len(body) - len(symbol):
                new = symbol + common
                rank = -1  # le Tab suivant donne le premier candidat
            else:
                new = symbol + candidates[0]
        self.cmd_var.set(new)
        self.cmd_entry.icursor("end")
        self._tab_state = (new, candidates, rank) if len(candidates) > 1 else None
        return "break"

    def _toggle_reloads(self):
        self.reloads_open_var.set(not self.reloads_open_var.get())
        self._show_reloads()

    def _show_reloads(self):
        """Show or hide the reload grid and update the header's arrow."""
        is_open = self.reloads_open_var.get()
        arrow = "▾" if is_open else "▸"
        hint = "" if is_open else "  (clic pour afficher)"
        self.reloads_header.config(text=f"{arrow} Reloads rapides ({len(RELOAD_TARGETS)}){hint}")
        if is_open:
            self.reloads_inner.pack(fill="x")
        else:
            self.reloads_inner.pack_forget()

    def _build_log(self):
        paned = ttk.PanedWindow(self.root, orient="vertical")
        paned.pack(fill="both", expand=True, padx=8, pady=(4, 8))

        cmd_frame = ttk.LabelFrame(paned, text="Journal (commandes)")
        self.log = scrolledtext.ScrolledText(cmd_frame, height=8, state="disabled", wrap="word")
        self.log.pack(fill="both", expand=True, padx=4, pady=4)
        self._bind_copy_menu(self.log, players=True)
        paned.add(cmd_frame, weight=1)

        srv_frame = ttk.LabelFrame(paned, text="Console serveur")

        # Row 1: source filters (checked = visible) + level filters.
        filt = ttk.Frame(srv_frame)
        filt.pack(fill="x", padx=4, pady=(4, 0))
        ttk.Label(filt, text="Serveurs :").pack(side="left", padx=(0, 4))
        for src, label in SERVER_SOURCES:
            var = tk.BooleanVar(value=True)
            self._src_vars[src] = var
            ttk.Checkbutton(filt, text=label, variable=var,
                            command=self._refilter_serverlog).pack(side="left", padx=2)
        ttk.Separator(filt, orient="vertical").pack(side="left", fill="y", padx=8)
        ttk.Label(filt, text="Sources :").pack(side="left", padx=(0, 4))
        for pattern, label in LOG_FILTERS:
            var = tk.BooleanVar(value=True)
            self._log_filter_vars[pattern] = var
            ttk.Checkbutton(filt, text=label, variable=var,
                            command=self._refilter_serverlog).pack(side="left", padx=2)
        ttk.Separator(filt, orient="vertical").pack(side="left", fill="y", padx=8)
        ttk.Label(filt, text="Niveaux :").pack(side="left", padx=(0, 4))
        for label, _flags in LOG_LEVELS:
            var = tk.BooleanVar(value=True)
            self._level_vars[label] = var
            ttk.Checkbutton(filt, text=label, variable=var,
                            command=self._refilter_serverlog).pack(side="left", padx=2)

        # Row 2: search, pause, save to file, alerts.
        tools = ttk.Frame(srv_frame)
        tools.pack(fill="x", padx=4, pady=(2, 0))
        ttk.Label(tools, text="Recherche :").pack(side="left", padx=(0, 4))
        self.search_var = tk.StringVar()
        self.search_entry = tk.Entry(tools, textvariable=self.search_var, width=24)
        self.search_entry.pack(side="left")
        self.search_var.trace_add("write", lambda *_: self._schedule_search())
        self.search_entry.bind("<Escape>", lambda e: self.search_var.set(""))
        self.regex_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(tools, text="Regex", variable=self.regex_var,
                        command=self._update_search).pack(side="left", padx=(4, 0))
        self.only_matches_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(tools, text="Seulement les correspondances",
                        variable=self.only_matches_var,
                        command=self._refilter_serverlog).pack(side="left", padx=(4, 0))
        ttk.Separator(tools, orient="vertical").pack(side="left", fill="y", padx=8)
        self.pause_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(tools, text="Pause defilement", variable=self.pause_var,
                        command=self._on_pause_toggle).pack(side="left")
        self.savelog_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(tools, text="Enregistrer", variable=self.savelog_var,
                        command=self._on_savelog_toggle).pack(side="left", padx=(6, 0))
        self.alerts_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(tools, text="Alertes", variable=self.alerts_var).pack(side="left", padx=(6, 0))
        self.alert_lbl = ttk.Label(tools, text="", foreground="#e55", cursor="hand2")
        self.alert_lbl.pack(side="left", padx=(6, 0))
        self.alert_lbl.bind("<Button-1>", lambda e: self._reset_alerts())

        self.srvlog = scrolledtext.ScrolledText(srv_frame, height=12, state="disabled",
                                                wrap="word", background="#111", foreground="#ccc",
                                                insertbackground="#ccc")
        self.srvlog.pack(fill="both", expand=True, padx=4, pady=(2, 4))
        self.srvlog.tag_config("srv_status", foreground="#5c5")
        self.srvlog.tag_config("srv_sql", foreground="#c5c")
        self.srvlog.tag_config("srv_warn", foreground="#db3")
        self.srvlog.tag_config("srv_debug", foreground="#5cc")
        self.srvlog.tag_config("srv_err", foreground="#e55")
        self.srvlog.tag_config("srv_def", foreground="#ccc")
        self.srvlog.tag_config("alert", background="#4a1414")
        self.srvlog.tag_config("match", background="#7a6a00", foreground="#fff")
        self.srvlog.tag_raise("match")
        self._bind_copy_menu(self.srvlog, players=True)
        paned.add(srv_frame, weight=2)

    def _apply_theme(self):
        """Switch ttk widgets and the command journal between light and dark."""
        dark = self.dark_var.get()
        style = ttk.Style(self.root)
        if dark:
            style.theme_use("clam")
            d = DARK
            style.configure(".", background=d["bg"], foreground=d["fg"],
                            fieldbackground=d["field"], bordercolor=d["border"],
                            lightcolor=d["bg"], darkcolor=d["bg"],
                            troughcolor=d["field"], insertcolor=d["fg"])
            style.configure("TButton", background=d["button"])
            style.map("TButton", background=[("active", d["active"]),
                                             ("disabled", d["bg"])],
                      foreground=[("disabled", "#777")])
            style.map("TCheckbutton", background=[("active", d["bg"])],
                      indicatorcolor=[("selected", "#6af"), ("!selected", d["field"])])
            style.configure("TEntry", fieldbackground=d["field"], foreground=d["fg"])
            # « clam » donne aux combos en lecture seule un fond clair : le texte
            # clair n'y est lisible que surligné, sur celui qui a le focus.
            style.map("TCombobox",
                      fieldbackground=[("readonly", d["field"]), ("disabled", d["bg"])],
                      foreground=[("disabled", "#777"), ("readonly", d["fg"])],
                      background=[("active", d["active"]), ("!active", d["button"])],
                      selectbackground=[("focus", "#2f5d8a"), ("!focus", d["field"])],
                      selectforeground=[("focus", "#fff"), ("!focus", d["fg"])],
                      arrowcolor=[("disabled", "#777"), ("!disabled", d["fg"])])
            self.root.option_add("*TCombobox*Listbox.background", d["field"])
            self.root.option_add("*TCombobox*Listbox.foreground", d["fg"])
            self.root.option_add("*TCombobox*Listbox.selectBackground", "#2f5d8a")
            self.root.option_add("*TCombobox*Listbox.selectForeground", "#fff")
            style.configure("TLabelframe", background=d["bg"])
            style.configure("TLabelframe.Label", background=d["bg"], foreground=d["fg"])
            # Listes (Atelier NPC) : « clam » leur laisse un fond blanc.
            style.configure("Treeview", background=d["field"], fieldbackground=d["field"],
                            foreground=d["fg"], bordercolor=d["border"])
            style.map("Treeview", background=[("selected", "#2f5d8a")],
                      foreground=[("selected", "#fff")])
            style.configure("Treeview.Heading", background=d["button"], foreground=d["fg"],
                            bordercolor=d["border"])
            style.map("Treeview.Heading", background=[("active", d["active"])])
            self.root.configure(background=d["bg"])
            entry_colors = dict(background=d["field"], foreground=d["fg"],
                                insertbackground=d["fg"])
        else:
            # The dark colors were configured on "clam" only: going back to the
            # native theme restores its own look untouched.
            style.theme_use(self._light_theme)
            for key, color in LIGHT_LISTBOX.items():
                self.root.option_add(f"*TCombobox*Listbox.{key}", color)
            self.root.configure(background="SystemButtonFace" if sys.platform == "win32" else "")
            entry_colors = dict(background="white", foreground="black",
                                insertbackground="black")
        style.configure("Reload.TButton", font=self._small_font, padding=(2, 1))
        self.search_entry.configure(**entry_colors)
        self._update_search()  # re-apply the red "bad regex" background if needed

        c = LOG_COLORS[dark]
        self.log.configure(background=c["bg"], foreground=c["fg"], insertbackground=c["fg"])
        for tag in ("sent", "ok", "err", "info", "out"):
            self.log.tag_config(tag, foreground=c[tag])
        for i in range(self._link_counter):
            self.log.tag_config(f"link-{i}", foreground=c["link"])

    def _bind_copy_menu(self, widget, players=False):
        """Attach a right-click context menu with copy actions to a Text widget.
        Avec `players`, le menu ajoute les actions sur le joueur dont le nom est
        sélectionné, ou à défaut sur le mot cliqué."""

        def show_menu(event):
            menu = tk.Menu(widget, tearoff=0)
            try:
                selected = widget.get("sel.first", "sel.last")
            except tk.TclError:
                selected = ""
            menu.add_command(label="Copier la sélection", state="normal" if selected else "disabled",
                             command=lambda: self._copy_selection(widget))
            menu.add_command(label="Tout copier", command=lambda: self._copy_all(widget))
            if players:
                clicked = widget.index(f"@{event.x},{event.y}")
                name = clean_char_name(selected) or clean_char_name(
                    widget.get(f"{clicked} wordstart", f"{clicked} wordend"))
                menu.add_separator()
                if name:
                    add_player_menu(menu, name, lambda cmd: self._send_async("ATCMD " + cmd), self.root)
                else:
                    menu.add_command(label="Joueur : sélectionner son nom", state="disabled")
            menu.add_separator()
            menu.add_command(label="Effacer", command=lambda: self._clear_log(widget))
            menu.tk_popup(event.x_root, event.y_root)

        widget.bind("<Button-3>", show_menu)

    def _copy_selection(self, widget):
        try:
            text = widget.selection_get()
            self.root.clipboard_clear()
            self.root.clipboard_append(text)
        except tk.TclError:
            pass

    def _copy_all(self, widget):
        text = widget.get("1.0", "end-1c")
        self.root.clipboard_clear()
        self.root.clipboard_append(text)

    def _clear_log(self, widget):
        widget.configure(state="normal")
        widget.delete("1.0", "end")
        widget.configure(state="disabled")
        if widget is self.srvlog:
            self._srv_lines.clear()

    # ---- favorites ----------------------------------------------------------
    def _render_favorites(self):
        for b in self.fav_btns:
            b.destroy()
        self.fav_btns = []
        if not self.favorites:
            lbl = ttk.Label(self.fav_frame, text="Aucun favori : tape une commande puis « + Favori ».")
            lbl.grid(row=0, column=0, columnspan=FAV_COLS, padx=4, pady=2, sticky="w")
            self.fav_btns.append(lbl)
            return
        state = "normal" if self.conn.connected else "disabled"
        for i, (label, cmd) in enumerate(self.favorites):
            b = ttk.Button(self.fav_frame, text=label, style="Reload.TButton", state=state,
                           command=lambda c=cmd: self._send_command(c))
            b.grid(row=i // FAV_COLS, column=i % FAV_COLS, padx=2, pady=2, sticky="ew")
            b.bind("<Button-3>", lambda e, idx=i: self._favorite_menu(e, idx))
            self.fav_btns.append(b)

    def _favorite_menu(self, event, idx):
        menu = tk.Menu(self.root, tearoff=0)
        menu.add_command(label="Copier dans la commande",
                         command=lambda: self._set_cmd(self.favorites[idx][1]))
        menu.add_command(label="Renommer...", command=lambda: self._rename_favorite(idx))
        menu.add_command(label="Supprimer", command=lambda: self._delete_favorite(idx))
        menu.tk_popup(event.x_root, event.y_root)

    def _ask_label(self, title, initial=""):
        label = simpledialog.askstring(title, "Libelle du bouton :", initialvalue=initial,
                                       parent=self.root)
        label = (label or "").strip()
        # configparser keys cannot hold these; keep the ini readable back.
        return label.replace("=", "-").replace(":", "-").replace("[", "(").replace("]", ")")

    def _add_favorite(self):
        cmd = self.cmd_var.get().strip()
        if not cmd:
            self._log("Tape d'abord la commande a mettre en favori.", "err")
            return
        label = self._ask_label("Nouveau favori", cmd[:24])
        if not label:
            return
        self.favorites = [f for f in self.favorites if f[0] != label] + [(label, cmd)]
        self._render_favorites()
        self._save_settings()

    def _rename_favorite(self, idx):
        old, cmd = self.favorites[idx]
        label = self._ask_label("Renommer le favori", old)
        if label and label != old:
            self.favorites = [f for f in self.favorites if f[0] != label]
            idx = next(i for i, f in enumerate(self.favorites) if f[0] == old)
            self.favorites[idx] = (label, cmd)
            self._render_favorites()
            self._save_settings()

    def _delete_favorite(self, idx):
        del self.favorites[idx]
        self._render_favorites()
        self._save_settings()

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

    def _log_server(self, payload, src="map"):
        """Display one server console line ("<flag> <message>") in the server panel.

        Lines of the char and login servers carry a "[char] " / "[login] "
        prefix; map-server lines stay as they always were."""
        flag_str, _, msg = payload.partition(" ")
        try:
            flag = int(flag_str)
        except ValueError:
            flag, msg = 0, payload
        msg = ANSI_RE.sub("", msg).rstrip("\n")
        if src != "map":
            msg = f"[{src}] {msg}"
        tag = MSG_TAGS.get(flag, "srv_def")
        now = datetime.now()
        ts = now.strftime("%H:%M:%S")
        alert = any(p in msg for p in ALERT_PATTERNS)
        self._srv_lines.append((ts, flag, tag, msg, alert, src))
        self._write_logfile(now, flag, msg)
        if alert and self.alerts_var.get():
            self._raise_alert()
        if self._line_visible(flag, msg, src):
            self._append_serverline(ts, tag, msg, alert)

    def _line_visible(self, flag, msg, src="map"):
        """Apply server, source and level filters, and "only matches"."""
        var = self._src_vars.get(src)
        if var is not None and not var.get():
            return False
        for pattern, var in self._log_filter_vars.items():
            if not var.get() and pattern in msg:
                return False
        level = self._flag_level.get(flag, "Info")
        if not self._level_vars[level].get():
            return False
        if self.only_matches_var.get() and self._search_re is not None:
            return self._search_re.search(msg) is not None
        return True

    def _insert_serverline(self, ts, tag, msg, alert):
        """Insert one line at the end of the (writable) server panel."""
        start = self.srvlog.index("end-1c")
        tags = (tag, "alert") if alert else (tag,)
        prefix = f"[{ts}] "
        self.srvlog.insert("end", f"{prefix}{msg}\n", tags)
        if self._search_re is not None:
            for m in self._search_re.finditer(msg):
                if m.end() > m.start():
                    a = f"{start} + {len(prefix) + m.start()} chars"
                    b = f"{start} + {len(prefix) + m.end()} chars"
                    self.srvlog.tag_add("match", a, b)

    def _append_serverline(self, ts, tag, msg, alert):
        at_bottom = self.srvlog.yview()[1] >= 0.999
        self.srvlog.configure(state="normal")
        self._insert_serverline(ts, tag, msg, alert)
        self.srvlog.configure(state="disabled")
        if at_bottom and not self.pause_var.get():
            self.srvlog.see("end")

    def _refilter_serverlog(self):
        """Re-render the server panel from the buffer after a filter change."""
        keep_view = self.pause_var.get()
        view = self.srvlog.yview()[0]
        self.srvlog.configure(state="normal")
        self.srvlog.delete("1.0", "end")
        for ts, flag, tag, msg, alert, src in self._srv_lines:
            if self._line_visible(flag, msg, src):
                self._insert_serverline(ts, tag, msg, alert)
        self.srvlog.configure(state="disabled")
        if keep_view:
            self.srvlog.yview_moveto(view)
        else:
            self.srvlog.see("end")

    def _on_pause_toggle(self):
        if not self.pause_var.get():
            self.srvlog.see("end")

    # ---- search -------------------------------------------------------------
    def _schedule_search(self):
        if self._search_job is not None:
            self.root.after_cancel(self._search_job)
        self._search_job = self.root.after(SEARCH_DEBOUNCE_MS, self._update_search)

    def _update_search(self):
        self._search_job = None
        text = self.search_var.get()
        ok = True
        if not text:
            new = None
        else:
            try:
                new = re.compile(text if self.regex_var.get() else re.escape(text), re.IGNORECASE)
            except re.error:
                new, ok = None, False
        dark = self.dark_var.get()
        normal_bg = DARK["field"] if dark else "white"
        self.search_entry.configure(background=normal_bg if ok else ("#6a1f1f" if dark else "#fcc"))
        old = self._search_re.pattern if self._search_re else None
        self._search_re = new
        if (new.pattern if new else None) != old:
            self._refilter_serverlog()

    # ---- alerts & log file --------------------------------------------------
    def _raise_alert(self):
        self._alert_count += 1
        self.alert_lbl.config(text=f"{self._alert_count} alerte(s) - clic pour effacer")
        if self.root.focus_displayof() is None:
            self.root.bell()
            _flash_taskbar(self.root)

    def _reset_alerts(self):
        self._alert_count = 0
        self.alert_lbl.config(text="")

    def _on_savelog_toggle(self):
        if self.savelog_var.get():
            self._log(f"Console serveur enregistree dans {LOG_DIR}", "info")
        else:
            self._close_logfile()

    def _write_logfile(self, now, flag, msg):
        """Append every received line (unfiltered) to logs/console-YYYY-MM-DD.log."""
        if not self.savelog_var.get():
            return
        day = now.strftime("%Y-%m-%d")
        try:
            if self._logfile is None or self._logfile[0] != day:
                self._close_logfile()
                os.makedirs(LOG_DIR, exist_ok=True)
                path = os.path.join(LOG_DIR, f"console-{day}.log")
                self._logfile = (day, open(path, "a", encoding="utf-8"))
            f = self._logfile[1]
            f.write(f"{now.strftime('%H:%M:%S')} [{self._flag_level.get(flag, 'Info')}] {msg}\n")
            f.flush()
        except OSError as exc:
            self.savelog_var.set(False)
            self._close_logfile()
            self._log(f"Enregistrement de la console desactive: {exc}", "err")

    def _close_logfile(self):
        if self._logfile is not None:
            try:
                self._logfile[1].close()
            except OSError:
                pass
            self._logfile = None

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
        self.log.tag_config(tag, foreground=LOG_COLORS[self.dark_var.get()]["link"], underline=True)
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
        elif kind == "sidelog":
            src, gen, payload = text
            if gen == self._side_gen[src]:  # drop lines of an orphaned reader
                self._log_server(payload, src)
        elif kind == "sidestate":
            self._handle_side_state(*text)
        elif kind == "reply":
            self._log(text, "ok" if text.startswith("OK") else "err")
        elif kind == "callback":
            on_result, output, reply = text
            on_result(output, reply)
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
        for b in self.fav_btns:
            if isinstance(b, ttk.Button):
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
        if not self.secret_var.get():
            self._log(f"Secret vide : renseigne ADMIN_SECRET dans {env_path()} "
                      f"(ou la variable {ENV_VAR}), ou saisis-le dans le champ Secret.", "err")
            return
        # User intent: stay connected, retrying until it works.
        self._want_connected = True
        self._set_connected(False)
        self._start_connect()
        for src, _k, _p in SIDE_FEEDS:
            self._start_side_feed(src)

    # ---- char / login console feeds -----------------------------------------
    def _start_side_feed(self, src):
        """Connect the console feed of one secondary server (best-effort)."""
        self._side_retry.discard(src)
        if not self._want_connected or self.side_conns[src].connected:
            return
        host = self.host_var.get().strip()
        secret = self.secret_var.get()
        try:
            port = int(self.side_ports[src].get().strip())
        except ValueError:
            self._log(f"[{src}] port invalide dans MoonlightAdmin.ini.", "err")
            return
        if port == 0:  # 0 = this feed disabled, like on the server side
            return
        self._side_gen[src] += 1
        gen = self._side_gen[src]
        # A fresh object per attempt: an orphaned reader keeps its own socket
        # and can never read from (or close) the one of its successor.
        conn = self.side_conns[src] = AdminConnection()

        def worker():
            try:
                conn.connect(host, port)
                if gen != self._side_gen[src]:  # stopped while connecting
                    conn.close()
                    return
                _o, rep = conn.send("AUTH " + secret)
                if not rep.startswith("OK"):
                    raise RuntimeError("AUTH refuse: " + rep)
                _o, rep = conn.send("LOG ON")
                if not rep.startswith("OK"):
                    raise RuntimeError("LOG ON refuse: " + rep)
                self.events.put(("sidestate", (src, gen, True, f"{host}:{port}")))
                while conn.connected:
                    line = conn._read_line()
                    if line.startswith("* "):
                        self.events.put(("sidelog", (src, gen, line[2:])))
            except Exception as exc:  # noqa: BLE001
                self.events.put(("sidestate", (src, gen, False, repr(exc))))

        threading.Thread(target=worker, daemon=True).start()

    def _handle_side_state(self, src, gen, ok, detail):
        if gen != self._side_gen[src]:
            return  # a reader of an older connection: its successor is in charge
        if ok:
            self._side_state[src] = True
            self._log(f"[feed] console {src} connectee ({detail}).", "ok")
            return
        self.side_conns[src].close()
        if not self._want_connected:
            return
        # Log the first failure and the loss of a live feed, not every retry.
        if self._side_state[src] is not False:
            self._log(f"[feed] console {src} indisponible: {detail} "
                      f"(nouvel essai toutes les {SIDE_RETRY_SECONDS}s)", "info")
        self._side_state[src] = False
        if src not in self._side_retry:
            self._side_retry.add(src)
            self.root.after(SIDE_RETRY_SECONDS * 1000, lambda s=src: self._start_side_feed(s))

    def _stop_side_feeds(self):
        for src, _k, _p in SIDE_FEEDS:
            self._side_gen[src] += 1  # orphan the running reader
            self._side_state[src] = None
            self.side_conns[src].close()

    def _start_connect(self):
        if not self._want_connected or self.conn.connected or self._connect_inflight:
            return
        host = self.host_var.get().strip()
        secret = self.secret_var.get()
        try:
            port = int(self.port_var.get().strip())
        except ValueError:
            self._want_connected = False
            self._set_connected(False)
            self._log("Port invalide.", "err")
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
        self._stop_side_feeds()
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
            self._send_command(cmd)
            self.cmd_var.set("")

    @staticmethod
    def _npc_path_to_reload(cmd):
        """If cmd is a pasted NPC script path (.npc / .txt), return the
        "reloadnpcfile <path>" atcommand for it, else None.

        Accepts what Explorer's "Copy as path" gives (quotes, absolute path):
        an absolute path is cut back to its moon/ part, since the map-server
        resolves @reloadnpcfile relative to its own root (and prepends moon/
        itself when missing). Backslashes become forward slashes.
        """
        path = cmd.strip().strip('"').strip("'").strip()
        if not re.search(r"\.(npc|txt)$", path, re.IGNORECASE):
            return None
        # A command with arguments ("loadnpc x.npc", "@unloadnpcfile x.npc")
        # is not a bare path: leave it alone.
        if path.startswith("@") or (" " in path and not re.search(r"[\\/]", path.split(" ", 1)[0])):
            return None
        norm = path.replace("\\", "/")
        m = re.search(r"(?:^|/)(moon/.+)$", norm, re.IGNORECASE)
        if m:
            path = m.group(1)
        elif re.match(r"^[A-Za-z]:/|^/", norm):
            return None  # absolute path outside moon/: the server can't load it
        return "reloadnpcfile " + path

    def _send_command(self, cmd):
        """Send a raw protocol line as-is, anything else as an atcommand."""
        reload_cmd = self._npc_path_to_reload(cmd)
        if reload_cmd:
            self._log("Chemin NPC détecté -> " + reload_cmd, "info")
            cmd = reload_cmd
        _RAW = ("RELOAD ", "LOG ", "NAME ", "QUIT")
        line = cmd if any(cmd.upper().startswith(p) for p in _RAW) else "ATCMD " + cmd
        self._send_async(line)

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
        self.cmd_entry.focus_set()

    def _status_alert(self, text, urgent):
        self._alert("pc-serv", text, urgent)

    def _alert(self, source, text, urgent):
        """Une alerte d'une autre fenêtre : au journal, et si elle est urgente
        et la case « Alertes » cochée, la sonnerie et la barre des tâches."""
        self._log(f"[{source}] {text}", "err" if urgent else "info")
        if urgent and self.alerts_var.get():
            self.root.bell()
            _flash_taskbar(self.root)

    def send_admin(self, atcmd, on_result=None):
        """Envoie une atcommande par le canal admin, pour une autre fenêtre.
        `on_result(lignes, statut)` est rappelé dans le fil de l'interface :
        lignes capturées, puis « OK … » ou « ERR … »."""
        self._send_async("ATCMD " + atcmd, on_result)

    def send_quiet(self, line, on_result):
        """Une ligne du protocole envoyée pour une autre fenêtre, sans rien
        écrire au journal : un relevé répété (PLAYERS) l'inonderait."""
        self._send_async(line, on_result, quiet=True)

    def _send_async(self, line, on_result=None, quiet=False):
        if not self.conn.connected:
            if not quiet:
                self._log("Pas connecte.", "err")
            if on_result is not None:
                on_result([], NOT_CONNECTED_REPLY + " au canal admin")
            return
        name = self.name_var.get().strip() or DEFAULT_NAME
        if not quiet:
            self._log("> " + line, "sent")

        def worker():
            try:
                # Keep the server-side display name in sync before each command.
                if name != self._last_name:
                    self.conn.send("NAME " + name)
                    self._last_name = name
                output, reply = self.conn.send(line)
                if not quiet:
                    for out_line in output:
                        self.events.put(("output", out_line))
                    self.events.put(("reply", reply))
                if on_result is not None:
                    self.events.put(("callback", (on_result, output, reply)))
            except Exception as exc:  # noqa: BLE001
                self.conn.close()
                self.events.put(("lost", f"Erreur reseau: {exc}"))
                if on_result is not None:
                    self.events.put(("callback", (on_result, [], f"ERR réseau : {exc}")))

        threading.Thread(target=worker, daemon=True).start()

    def _on_close(self):
        # Closing the window closes the script's pipes: a script cut off in the
        # middle of --apply would leave its tables half written.
        if self.tables.busy() and not messagebox.askyesno(
                "Script en cours",
                "Un script des tables moonclient tourne encore. Quitter quand même ?",
                parent=self.root, icon="warning", default="no"):
            return
        if self.builder.busy():
            if not messagebox.askyesno(
                    "Build en cours",
                    "Une construction du client tourne encore. L'arrêter et quitter ?",
                    parent=self.root, icon="warning", default="no"):
                return
            self.builder.runner.stop()
        if self.maintenance.busy() and not messagebox.askyesno(
                "Maintenance en cours",
                "Une maintenance programmée tourne : fermer MoonlightAdmin l'arrête sans prévenir "
                "les joueurs. Quitter quand même ?",
                parent=self.root, icon="warning", default="no"):
            return
        if self.ssh.busy():
            if not messagebox.askyesno(
                    "Commande SSH en cours",
                    "Une commande tourne encore sur le serveur. L'arrêter et quitter ?",
                    parent=self.root, icon="warning", default="no"):
                return
            self.ssh.runner.stop()
        self.logs.stop_all()
        self._want_connected = False  # stop any pending reconnect
        self._save_settings()
        if self.conn.connected:
            try:
                self.conn.send("QUIT")
            except Exception:  # noqa: BLE001
                pass
            self.conn.close()
        self.logconn.close()
        self._stop_side_feeds()
        self._close_logfile()
        self.root.destroy()


def parse_args():
    ap = argparse.ArgumentParser(description="Moonlight - Admin map-server (GUI)")
    ap.add_argument("--host", help="server host (default in-app: %s)" % DEFAULT_HOST)
    ap.add_argument("--port", type=int, help="admin port (default in-app: %s)" % DEFAULT_PORT)
    ap.add_argument("--secret", help="admin password (default: $%s, then admin.env)" % ENV_VAR)
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
