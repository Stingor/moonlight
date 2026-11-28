"""Fenêtre « Journaux des services » de MoonlightAdmin.

Suit en direct, par ssh, la console des services que lance ./start sur
pc-serv : `tail -F log/<service>.console.log` pour ceux du dépôt (écrite
par leur session screen), `journalctl -f` pour l'unité systemd survivors.

Chaque source cochée est un processus ssh à elle ; la décocher la masque
sans arrêter son suivi, qui ne cesse qu'à la fermeture de la fenêtre. Un
suivi qui reprend relit les dernières lignes du journal et remplace celles
de la source déjà en mémoire. Les lignes reçues se gardent en mémoire, et
les filtres — sources affichées, recherche (texte ou expression régulière),
erreurs seules — s'appliquent aussi à ce qui est déjà affiché.

Alertes : une ligne qui répond au motif « Alerte sur » (expression régulière,
sans casse) se surligne, et s'écrit au journal principal, qui sonne et fait
clignoter la barre des tâches si sa case « Alertes » est cochée. L'historique
relu au départ d'un suivi n'alerte pas, et une même source n'alerte qu'une
fois par ALERT_COOLDOWN_S secondes. Il faut la fenêtre ouverte : fermée, elle
ne suit plus rien.

Le compte et le dossier du dépôt sont ceux de la fenêtre « Serveur (SSH) ».
"""
import queue
import re
import time
import tkinter as tk
from collections import deque
from dataclasses import dataclass
from datetime import datetime
from tkinter import scrolledtext, ttk

from ssh_panel import classify
from tables_panel import DRAIN_MS, ProcessRunner
from tabs_host import PageSpec

SETTINGS_SECTION = "journaux"
PAGE = PageSpec("logs", "Journaux", "Moonlight - Journaux des services", 760, 480)
BUFFER_LINES = 20000
# Lignes déjà présentes rapatriées à l'ouverture d'une source.
BACKLOG_LINES = 300
# « [start] … mort » : la ligne qu'écrit ./start quand un service meurt.
DEFAULT_ALERT_PATTERN = (r"Traceback|Segmentation fault|segfault|core dumped|\[Fatal Error\]|Exception"
                         r"|\[start\] .* mort")
# L'historique relu arrive d'un bloc au départ d'un suivi : il n'alerte pas.
BACKLOG_GRACE_S = 3
ALERT_COOLDOWN_S = 60
ALERT_SHOWN_CHARS = 160
SEARCH_DEBOUNCE_MS = 250
ANSI_ESCAPE_RE = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]|\x1b[()][A-Za-z0-9]")
SURVIVORS_UNIT = "survivors-verify-queue"


@dataclass(frozen=True)
class LogSource:
    key: str
    label: str
    color: str
    shell: str  # ligne de shell exécutée dans le dossier du dépôt


def console_tail(service):
    return f"tail -n {BACKLOG_LINES} -F log/{service}.console.log"


LOG_SOURCES = (
    LogSource("login", "login", "#8cf", console_tail("login")),
    LogSource("char", "char", "#8fc", console_tail("char")),
    LogSource("map", "map", "#fc8", console_tail("map")),
    LogSource("web", "web", "#c9f", console_tail("web")),
    LogSource("voice", "voice", "#f9c", console_tail("voice")),
    LogSource("moonbot", "moonbot", "#9df", console_tail("moonbot")),
    LogSource("survivors", "survivors", "#dd8",
              f"sudo -n journalctl -u {SURVIVORS_UNIT} -f -n {BACKLOG_LINES} --no-pager -o short-iso"),
)
# Ouvertes d'office : les consoles que la fenêtre principale ne montre pas
# déjà par le canal admin (login, char et map y ont la leur).
DEFAULT_FOLLOWED = ("web", "voice", "moonbot", "survivors")
LEVEL_COLORS = {"fail": "#f55", "warn": "#db5", "info": "#888"}


def clean_line(raw):
    """Une ligne de console screen sans ses séquences de couleur ni ses
    retours chariot (une ligne de progression s'y réécrit sur place)."""
    return ANSI_ESCAPE_RE.sub("", raw).split("\r")[-1]


class LogWindow:
    """Page « Journaux des services » (tabs_host.py), construite à sa première
    apparition ; les journaux suivis le restent, onglet caché."""

    def __init__(self, root, host, settings, bind_copy_menu, ssh_window, on_alert=None):
        self.root = root
        self.host = host
        self.ssh = ssh_window
        self.on_alert = on_alert  # (texte, urgent) vers le journal principal
        self.alert_var = tk.StringVar(value=settings.get(SETTINGS_SECTION, "alert_pattern", DEFAULT_ALERT_PATTERN))
        self.alerts_on_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "alerts", True))
        self._alert_re = None
        self._follow_started = {}  # source -> départ de son suivi (time.monotonic)
        self._last_alert = {}      # source -> dernière alerte (time.monotonic)
        self._bind_copy_menu = bind_copy_menu
        self.follow_vars = {s.key: tk.BooleanVar(value=settings.get_bool(
            SETTINGS_SECTION, s.key, s.key in DEFAULT_FOLLOWED)) for s in LOG_SOURCES}
        self.search_var = tk.StringVar(value=settings.get(SETTINGS_SECTION, "search"))
        self.regex_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "regex"))
        self.errors_only_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "errors_only"))
        self.pause_var = tk.BooleanVar(value=False)
        self._lines = deque(maxlen=BUFFER_LINES)  # (horodatage, clé de source, niveau, texte)
        self._runners = {}  # clé de source -> (ProcessRunner, file d'événements)
        self._search_re = None
        self._search_job = None
        self._held = 0  # lignes reçues pendant la pause
        self._shown = False
        self.win = None

    # ---- réglages -----------------------------------------------------------
    def settings_data(self):
        data = {s.key: self.follow_vars[s.key].get() for s in LOG_SOURCES}
        data.update(search=self.search_var.get(), regex=self.regex_var.get(),
                    errors_only=self.errors_only_var.get(), alert_pattern=self.alert_var.get(),
                    alerts=self.alerts_on_var.get())
        return data

    def stop_all(self):
        for runner, _events in self._runners.values():
            runner.stop()
        self._runners.clear()

    # ---- fenêtre ------------------------------------------------------------
    def show(self):
        self.host.reveal(PAGE.key)

    def on_page_shown(self):
        if self.win is None:
            self._build()
        self._shown = True
        self._sync_followers()

    def on_page_hidden(self):
        # Les journaux suivis le restent onglet caché : leurs alertes
        # comptent surtout quand on regarde ailleurs.
        pass

    def _build(self):
        self.win = self.host.body(PAGE.key)
        body = ttk.Frame(self.win)
        body.pack(fill="both", expand=True)

        sources = ttk.LabelFrame(body, text="Sources suivies  (cocher : suivre et afficher ; décocher : masquer)")
        sources.pack(fill="x", padx=8, pady=(8, 4))
        self.state_lbls = {}
        for column, source in enumerate(LOG_SOURCES):
            cell = ttk.Frame(sources)
            cell.grid(row=0, column=column, padx=6, pady=4, sticky="w")
            tk.Checkbutton(cell, text=source.label, variable=self.follow_vars[source.key],
                           foreground=source.color, selectcolor="#222", background="#333",
                           activebackground="#333", activeforeground=source.color,
                           command=self._sync_followers).pack(side="left")
            lbl = ttk.Label(cell, text="", foreground="#888")
            lbl.pack(side="left", padx=(2, 0))
            self.state_lbls[source.key] = lbl

        filters = ttk.LabelFrame(body, text="Filtres")
        filters.pack(fill="x", padx=8, pady=4)
        ttk.Label(filters, text="Recherche").pack(side="left", padx=(4, 2), pady=4)
        search = ttk.Entry(filters, textvariable=self.search_var, width=32)
        search.pack(side="left", padx=2)
        self.search_var.trace_add("write", lambda *_: self._schedule_render())
        ttk.Checkbutton(filters, text="regex", variable=self.regex_var,
                        command=self._render).pack(side="left", padx=(6, 2))
        ttk.Checkbutton(filters, text="erreurs et avertissements seuls", variable=self.errors_only_var,
                        command=self._render).pack(side="left", padx=6)
        ttk.Checkbutton(filters, text="pause", variable=self.pause_var,
                        command=self._toggle_pause).pack(side="left", padx=6)
        self.search_err_lbl = ttk.Label(filters, text="", foreground="#f55")
        self.search_err_lbl.pack(side="left", padx=4)
        ttk.Button(filters, text="Effacer", command=self._clear).pack(side="right", padx=4)

        alerts = ttk.LabelFrame(body, text="Alertes  (journal principal ; sonnerie si « Alertes » y est coché)")
        alerts.pack(fill="x", padx=8, pady=4)
        ttk.Checkbutton(alerts, text="alerter sur", variable=self.alerts_on_var).pack(side="left", padx=(4, 2), pady=4)
        ttk.Entry(alerts, textvariable=self.alert_var, width=60).pack(side="left", padx=2, fill="x", expand=True)
        self.alert_var.trace_add("write", lambda *_: self._compile_alert())
        self.alert_err_lbl = ttk.Label(alerts, text="", foreground="#f55")
        self.alert_err_lbl.pack(side="left", padx=4)
        self._compile_alert()

        out = ttk.Frame(body)
        out.pack(fill="both", expand=True, padx=8, pady=(4, 8))
        self.count_lbl = ttk.Label(out, text="", foreground="#888")
        self.count_lbl.pack(anchor="w")
        self.console = scrolledtext.ScrolledText(out, height=20, state="disabled", wrap="none",
                                                 background="#111", foreground="#ccc",
                                                 insertbackground="#ccc")
        self.console.pack(fill="both", expand=True)
        for source in LOG_SOURCES:
            self.console.tag_config("src_" + source.key, foreground=source.color)
        for level, color in LEVEL_COLORS.items():
            self.console.tag_config(level, foreground=color)
        self.console.tag_config("match", background="#554400")
        self.console.tag_config("alert", background="#4a1414")
        self.console.tag_lower("alert")
        self._bind_copy_menu(self.console, players=True)

        self._compile_search()
        self.win.after(DRAIN_MS, self._drain)

    # ---- suivi --------------------------------------------------------------
    def _sync_followers(self):
        """Un ssh par source cochée. Décocher ne fait que masquer : le suivi
        continue, sans trou à combler ni historique à relire au retour."""
        if not self._shown:
            return
        for source in LOG_SOURCES:
            if self.follow_vars[source.key].get() and source.key not in self._runners:
                self._start_follow(source)
        self._update_state()
        self._render()

    def _start_follow(self, source):
        # Le suivi rapatrie les BACKLOG_LINES dernières lignes : celles déjà
        # en mémoire pour cette source, d'un suivi précédent, en feraient des
        # doublons.
        kept = [entry for entry in self._lines if entry[1] != source.key]
        self._lines.clear()
        self._lines.extend(kept)
        self._follow_started[source.key] = time.monotonic()
        events = queue.Queue()
        runner = ProcessRunner(events)
        try:
            runner.start(self.ssh.ssh_argv(source.shell), None)
        except OSError as exc:
            self._append(source.key, f"ssh impossible : {exc}", "fail")
            return
        self._runners[source.key] = (runner, events)

    def _update_state(self):
        for source in LOG_SOURCES:
            entry = self._runners.get(source.key)
            text = "● suivi" if entry and entry[0].busy else ""
            self.state_lbls[source.key].config(text=text)

    def _drain(self):
        if self.win is None:
            return
        changed = False
        for key, (runner, events) in list(self._runners.items()):
            try:
                while True:
                    kind, payload = events.get_nowait()
                    if kind == "exit":
                        self._append(key, f"── suivi interrompu (code {payload})", "fail")
                        self._runners.pop(key, None)
                        changed = True
                        break
                    self._append(key, clean_line(payload))
            except queue.Empty:
                pass
        if changed:
            self._update_state()
        self.win.after(DRAIN_MS, self._drain)

    # ---- tampon et filtres ----------------------------------------------------
    def _append(self, key, text, level=None):
        entry = (datetime.now().strftime("%H:%M:%S"), key, level or classify(text), text)
        self._lines.append(entry)
        self._check_alert(key, text)
        if self.pause_var.get():
            self._held += 1
            self._update_count()
            return
        if self._visible(entry):
            self._insert(entry)
            self._update_count()

    def _visible(self, entry):
        _stamp, key, level, text = entry
        if not self.follow_vars[key].get():
            return False
        if self.errors_only_var.get() and level not in ("fail", "warn"):
            return False
        if self._search_re is not None and not self._search_re.search(text):
            return False
        return True

    def _compile_search(self):
        pattern = self.search_var.get()
        self.search_err_lbl.config(text="")
        if not pattern:
            self._search_re = None
            return
        if self.regex_var.get():
            try:
                self._search_re = re.compile(pattern, re.IGNORECASE)
            except re.error as exc:
                self._search_re = None
                self.search_err_lbl.config(text=f"regex invalide : {exc}")
        else:
            self._search_re = re.compile(re.escape(pattern), re.IGNORECASE)

    def _schedule_render(self):
        if self.win is None:
            return
        if self._search_job is not None:
            self.win.after_cancel(self._search_job)
        self._search_job = self.win.after(SEARCH_DEBOUNCE_MS, self._render)

    def _render(self):
        """Réaffiche tout le tampon sous les filtres en vigueur."""
        if self.win is None:
            return
        self._search_job = None
        self._compile_search()
        self.console.configure(state="normal")
        self.console.delete("1.0", "end")
        self.console.configure(state="disabled")
        for entry in self._lines:
            if self._visible(entry):
                self._insert(entry, scroll=False)
        self.console.see("end")
        self._update_count()

    def _insert(self, entry, scroll=True):
        stamp, key, level, text = entry
        at_bottom = self.console.yview()[1] >= 0.999
        self.console.configure(state="normal")
        self.console.insert("end", f"{stamp} ", "info")
        self.console.insert("end", f"[{key}] ", "src_" + key)
        start = self.console.index("end-1c")
        tags = (level,) if level in LEVEL_COLORS else ()
        if self._alert_re is not None and self._alert_re.search(text):
            tags += ("alert",)
        self.console.insert("end", text + "\n", tags)
        if self._search_re is not None:
            for m in self._search_re.finditer(text):
                if m.end() > m.start():
                    self.console.tag_add("match", f"{start}+{m.start()}c", f"{start}+{m.end()}c")
        self.console.configure(state="disabled")
        if scroll and at_bottom:
            self.console.see("end")

    # ---- alertes ------------------------------------------------------------
    def _compile_alert(self):
        pattern = self.alert_var.get().strip()
        self._alert_re = None
        self.alert_err_lbl.config(text="")
        if not pattern:
            return
        try:
            self._alert_re = re.compile(pattern, re.IGNORECASE)
        except re.error as exc:
            self.alert_err_lbl.config(text=f"motif invalide : {exc}")

    def _check_alert(self, key, text):
        if self.on_alert is None or self._alert_re is None or not self.alerts_on_var.get():
            return
        if not self._alert_re.search(text):
            return
        now = time.monotonic()
        if now - self._follow_started.get(key, now) < BACKLOG_GRACE_S:
            return
        if now - self._last_alert.get(key, float("-inf")) < ALERT_COOLDOWN_S:
            return
        self._last_alert[key] = now
        self.on_alert(f"{key} : {text[:ALERT_SHOWN_CHARS]}", True)

    def _toggle_pause(self):
        if not self.pause_var.get():
            self._held = 0
            self._render()
        self._update_count()

    def _update_count(self):
        text = f"{len(self._lines)} lignes en mémoire"
        if self.pause_var.get():
            text += f"  —  en pause, {self._held} reçues depuis"
        self.count_lbl.config(text=text)

    def _clear(self):
        self._lines.clear()
        self._held = 0
        self._render()
