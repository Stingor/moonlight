"""Panneau « Serveur (SSH) » de MoonlightAdmin.

Lance sur pc-serv, par `ssh.exe` d'OpenSSH, les commandes courantes du dépôt
serveur : git (status, pull, commit, push) et make (clean, server). Chaque
commande s'exécute dans le dossier du dépôt : `cd <dépôt> && <commande>`.

L'identification passe par une clé SSH, jamais par un mot de passe : ssh est
lancé en `BatchMode=yes`, qui échoue au lieu d'attendre une saisie que
personne ne peut taper. De même, `sudo` est appelé en `sudo -n` : sans règle
NOPASSWD sur le serveur, il refuse au lieu de rester bloqué.

Une chaîne de commandes s'arrête à la première qui échoue, comme `&&`.
« Arrêter » tue ssh.exe ; la commande distante perd alors sa sortie et
s'interrompt à sa prochaine écriture.

Le compte, le dossier du dépôt et l'emploi de sudo se retiennent dans la
section [ssh] de MoonlightAdmin.ini.
"""
import queue
import shlex
import subprocess
import time
import tkinter as tk
from dataclasses import dataclass
from datetime import datetime
from tkinter import messagebox, scrolledtext, ttk

from tables_panel import CONSOLE_COLORS, DRAIN_MS, ProcessRunner

SETTINGS_SECTION = "ssh"
DEFAULT_TARGET = "stingor@192.168.1.13"
DEFAULT_REPO = "~/moonlight"
HOME_PREFIX = "~/"
CONNECT_TIMEOUT_S = 8
SERVER_ALIVE_INTERVAL_S = 15
HISTORY_LIMIT = 50
HISTORY_SEPARATOR = "\x1f"
NOTHING_TO_COMMIT = "Aucune modification suivie : pas de commit, la chaîne continue."
# Une ligne qui contient l'un de ces motifs passe en rouge, en jaune pour
# les avertissements. git écrit son récit sur stderr : la couleur se décide
# au contenu, pas au flux.
ERROR_MARKERS = ("error:", "error ", "fatal:", "Error ", "CONFLICT", "[rejected]",
                 "Permission denied", "sudo:", "Could not resolve", "Connection refused",
                 "Connection timed out", "make: ***")
WARNING_MARKERS = ("warning:", "hint:")
PANEL_COLORS = dict(CONSOLE_COLORS, warn="#db5", step="#c8f")


@dataclass(frozen=True)
class RemoteCommand:
    label: str
    shell: str            # ligne de shell exécutée dans le dossier du dépôt
    sudo: bool = False    # préfixée de `sudo -n` si la case sudo est cochée
    confirm: str = ""     # question posée avant de lancer, vide : aucune


# --autostash : les scripts s'éditent sur pc-serv même, et un arbre modifié
# ferait refuser le pull ; les éditions en cours sont mises de côté puis
# reposées par-dessus ce qui arrive.
PULL_REBASE = RemoteCommand("git pull --rebase", "git pull --rebase --autostash")
GIT_PUSH = RemoteCommand("git push", "git push",
                         confirm="Pousser les commits de pc-serv vers le dépôt distant ?")
GIT_COMMANDS = (
    RemoteCommand("git status", "git status"),
    RemoteCommand("git log -10", "git log --oneline --decorate -10"),
    RemoteCommand("git diff --stat", "git diff --stat"),
    RemoteCommand("git fetch", "git fetch --prune"),
    RemoteCommand("git pull", "git pull --autostash"),
    PULL_REBASE,
    GIT_PUSH,
)
MAKE_CLEAN = RemoteCommand("make clean", "make clean", sudo=True,
                           confirm="Effacer les objets compilés ? Le prochain make server recompile tout.")
MAKE_SERVER = RemoteCommand("make server", "make server", sudo=True)
MAKE_COMMANDS = (MAKE_CLEAN, MAKE_SERVER)

# Cibles de ./start, dans l'ordre de sa table ; « tout » et « rathena » sont
# ses groupes. Jamais sous sudo : les services tournent sous le compte ssh.
START_TARGETS = ("tout", "rathena", "login", "char", "map", "web", "voice", "moonbot", "survivors")
DEFAULT_START_TARGET = "tout"
START_ACTIONS = (
    # (action, question avant de lancer : vide, aucune)
    ("status", ""),
    ("start", ""),
    ("stop", "Arrêter « {cible} » ? Les joueurs connectés seront déconnectés."),
    ("restart", "Redémarrer « {cible} » ? Les joueurs connectés seront déconnectés."),
)


def start_command(action, target, confirm=""):
    return RemoteCommand(f"./start {action} {target}", f"./start {action} {shlex.quote(target)}",
                         confirm=confirm.format(cible=target))


def remote_dir(path):
    """Le dossier du dépôt, cité pour le shell distant. `~/` y reste développé :
    entre apostrophes, le shell ne le remplacerait pas par le dossier personnel."""
    if path == "~":
        return '"$HOME"'
    if path.startswith(HOME_PREFIX):
        return '"$HOME"/' + shlex.quote(path[len(HOME_PREFIX):])
    return shlex.quote(path)


def classify(line):
    if any(m in line for m in ERROR_MARKERS):
        return "fail"
    if any(m in line for m in WARNING_MARKERS):
        return "warn"
    return "out"


class SshWindow:
    """Fenêtre « Serveur (SSH) », créée une fois, masquée à la fermeture."""

    def __init__(self, root, settings, bind_copy_menu, open_logs=None):
        self.root = root
        self._open_logs = open_logs
        target = settings.get(SETTINGS_SECTION, "start_target", DEFAULT_START_TARGET)
        self.start_target_var = tk.StringVar(
            value=target if target in START_TARGETS else DEFAULT_START_TARGET)
        self.events = queue.Queue()
        self.runner = ProcessRunner(self.events)
        self.target_var = tk.StringVar(value=settings.get(SETTINGS_SECTION, "target", DEFAULT_TARGET))
        self.repo_var = tk.StringVar(value=settings.get(SETTINGS_SECTION, "repo", DEFAULT_REPO))
        self.sudo_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "sudo_make", True))
        self.jobs_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "parallel_make", True))
        stored = settings.get(SETTINGS_SECTION, "history")
        self._history = [h for h in stored.split(HISTORY_SEPARATOR) if h] if stored else []
        self._history_pos = len(self._history)
        self._geometry = settings.get(SETTINGS_SECTION, "geometry")
        self._bind_copy_menu = bind_copy_menu
        self._queue = []
        self._chain_started = None
        self._step_started = None
        self._commit_pending = False  # la chaîne en cours commite : vider le message si elle réussit
        self.win = None

    # ---- réglages -----------------------------------------------------------
    def settings_data(self):
        return {
            "target": self.target_var.get().strip(),
            "repo": self.repo_var.get().strip(),
            "sudo_make": self.sudo_var.get(),
            "parallel_make": self.jobs_var.get(),
            "start_target": self.start_target_var.get(),
            "history": HISTORY_SEPARATOR.join(self._history[-HISTORY_LIMIT:]),
            "geometry": self._current_geometry(),
        }

    def _current_geometry(self):
        try:
            if self.win is not None and self.win.winfo_exists():
                return self.win.geometry()
        except tk.TclError:  # fenêtre principale déjà détruite
            pass
        return self._geometry

    def busy(self):
        return self.runner.busy

    # ---- fenêtre ------------------------------------------------------------
    def show(self):
        if self.win is None:
            self._build()
        self.win.deiconify()
        self.win.lift()

    def _hide(self):
        self._geometry = self.win.geometry()
        self.win.withdraw()

    def _build(self):
        self.win = tk.Toplevel(self.root)
        self.win.title("Moonlight - Serveur (SSH)")
        self.win.minsize(700, 520)
        if self._geometry:
            try:
                self.win.geometry(self._geometry)
            except tk.TclError:
                pass
        self.win.protocol("WM_DELETE_WINDOW", self._hide)
        body = ttk.Frame(self.win)
        body.pack(fill="both", expand=True)
        self.action_widgets = []

        conn = ttk.LabelFrame(body, text="Connexion")
        conn.pack(fill="x", padx=8, pady=(8, 4))
        conn.columnconfigure(1, weight=1)
        conn.columnconfigure(3, weight=1)
        ttk.Label(conn, text="Compte").grid(row=0, column=0, padx=4, pady=4, sticky="w")
        ttk.Entry(conn, textvariable=self.target_var).grid(row=0, column=1, padx=4, sticky="ew")
        ttk.Label(conn, text="Dépôt").grid(row=0, column=2, padx=4, sticky="w")
        ttk.Entry(conn, textvariable=self.repo_var).grid(row=0, column=3, padx=4, sticky="ew")
        self._action(ttk.Button(conn, text="Tester", command=self._test_connection)).grid(
            row=0, column=4, padx=4)
        ttk.Label(conn, text="Identification par clé SSH : aucun mot de passe n'est demandé ni retenu.",
                  foreground="#888").grid(row=1, column=0, columnspan=5, padx=4, pady=(0, 4), sticky="w")

        git = ttk.LabelFrame(body, text="git")
        git.pack(fill="x", padx=8, pady=4)
        row = ttk.Frame(git)
        row.pack(fill="x", padx=4, pady=(4, 2))
        for cmd in GIT_COMMANDS:
            self._action(ttk.Button(row, text=cmd.label, command=lambda c=cmd: self._run([c]))).pack(
                side="left", padx=2)
        commit = ttk.Frame(git)
        commit.pack(fill="x", padx=4, pady=(2, 4))
        ttk.Label(commit, text="Message").pack(side="left", padx=(2, 4))
        self.commit_var = tk.StringVar()
        entry = ttk.Entry(commit, textvariable=self.commit_var)
        entry.pack(side="left", fill="x", expand=True, padx=2)
        entry.bind("<Return>", lambda _e: self._commit(push=False))
        self._action(ttk.Button(commit, text="commit -a",
                                command=lambda: self._commit(push=False))).pack(side="left", padx=2)
        self._action(ttk.Button(commit, text="commit -a + push",
                                command=lambda: self._commit(push=True))).pack(side="left", padx=2)
        self._action(ttk.Button(commit, text="commit -a + pull --rebase + push",
                                command=lambda: self._commit(push=True, rebase=True))).pack(
            side="left", padx=2)

        make = ttk.LabelFrame(body, text="make")
        make.pack(fill="x", padx=8, pady=4)
        for cmd in MAKE_COMMANDS:
            self._action(ttk.Button(make, text=cmd.label, command=lambda c=cmd: self._run([c]))).pack(
                side="left", padx=(4, 2), pady=4)
        self._action(ttk.Button(make, text="pull --rebase + make server",
                                command=lambda: self._run([PULL_REBASE, MAKE_SERVER]))).pack(
            side="left", padx=2)
        self._action(ttk.Button(make, text="make server + ./start restart",
                                command=self._make_and_restart)).pack(side="left", padx=2)
        ttk.Checkbutton(make, text="sudo", variable=self.sudo_var).pack(side="left", padx=(12, 2))
        ttk.Checkbutton(make, text="-j$(nproc)", variable=self.jobs_var).pack(side="left", padx=2)

        services = ttk.LabelFrame(body, text="Services  (./start ; console : screen -r moonlight-<service>)")
        services.pack(fill="x", padx=8, pady=4)
        ttk.Label(services, text="Cible").pack(side="left", padx=(4, 2), pady=4)
        ttk.Combobox(services, textvariable=self.start_target_var, values=START_TARGETS,
                     state="readonly", width=10).pack(side="left", padx=2)
        for action, confirm in START_ACTIONS:
            self._action(ttk.Button(services, text=f"./start {action}",
                                    command=lambda a=action, c=confirm: self._run_start(a, c))).pack(
                side="left", padx=2)
        if self._open_logs is not None:
            ttk.Button(services, text="Journaux…", command=self._open_logs).pack(side="right", padx=4)

        free = ttk.LabelFrame(body, text="Commande libre  (dans le dossier du dépôt ; ↑ ↓ : historique)")
        free.pack(fill="x", padx=8, pady=4)
        self.free_var = tk.StringVar()
        self.free_entry = ttk.Entry(free, textvariable=self.free_var)
        self.free_entry.pack(side="left", fill="x", expand=True, padx=4, pady=4)
        self.free_entry.bind("<Return>", lambda _e: self._run_free())
        self.free_entry.bind("<Up>", lambda _e: self._history_step(-1))
        self.free_entry.bind("<Down>", lambda _e: self._history_step(1))
        self._action(ttk.Button(free, text="Lancer", command=self._run_free)).pack(side="left", padx=4)

        out = ttk.LabelFrame(body, text="Sortie")
        out.pack(fill="both", expand=True, padx=8, pady=(4, 8))
        bar = ttk.Frame(out)
        bar.pack(fill="x", padx=4, pady=(4, 0))
        self.run_lbl = ttk.Label(bar, text="aucune commande")
        self.run_lbl.pack(side="left")
        ttk.Button(bar, text="Effacer", command=self._clear).pack(side="right", padx=(4, 0))
        self.stop_btn = ttk.Button(bar, text="Arrêter", state="disabled", command=self._stop)
        self.stop_btn.pack(side="right")
        self.console = scrolledtext.ScrolledText(out, height=16, state="disabled", wrap="word",
                                                 background="#111", foreground="#ccc",
                                                 insertbackground="#ccc")
        self.console.pack(fill="both", expand=True, padx=4, pady=4)
        for tag, color in PANEL_COLORS.items():
            self.console.tag_config(tag, foreground=color)
        self._bind_copy_menu(self.console)

        self.win.after(DRAIN_MS, self._drain)

    def _action(self, widget):
        """Un bouton qui lance une commande : grisé tant qu'une autre tourne."""
        self.action_widgets.append(widget)
        return widget

    # ---- composition des commandes -----------------------------------------
    def _shell_of(self, cmd):
        line = cmd.shell
        if cmd is MAKE_SERVER and self.jobs_var.get():
            line += ' -j"$(nproc)"'
        if cmd.sudo and self.sudo_var.get():
            line = "sudo -n " + line
        return line

    def ssh_argv(self, shell_line):
        remote = f"cd {remote_dir(self.repo_var.get().strip() or DEFAULT_REPO)} && {shell_line}"
        return ["ssh", "-o", "BatchMode=yes",
                "-o", f"ConnectTimeout={CONNECT_TIMEOUT_S}",
                "-o", f"ServerAliveInterval={SERVER_ALIVE_INTERVAL_S}",
                self.target_var.get().strip() or DEFAULT_TARGET, remote]

    # ---- actions ------------------------------------------------------------
    def _run_start(self, action, confirm):
        self._run([start_command(action, self.start_target_var.get(), confirm)])

    def _make_and_restart(self):
        restart_confirm = dict(START_ACTIONS)["restart"]
        self._run([MAKE_SERVER, start_command("restart", self.start_target_var.get(), restart_confirm)])

    def _test_connection(self):
        self._run([RemoteCommand("Test", 'echo "$(hostname) : $(pwd)" && git branch --show-current '
                                         '&& (sudo -n true 2>/dev/null && echo "sudo sans mot de passe : oui" '
                                         '|| echo "sudo sans mot de passe : non")')])

    def _commit(self, push, rebase=False):
        """commit -a, puis au besoin pull --rebase et push. Le rebase passe
        après le commit : il replace les commits de pc-serv au-dessus de ce
        qui est arrivé, et le push qui suit n'est pas refusé comme en retard."""
        message = self.commit_var.get().strip()
        if not message:
            self._write("Message de commit vide : rien n'est lancé.", "fail")
            return
        commit_line = "git commit -a -m " + shlex.quote(message)
        if rebase:
            # Rien à commiter n'arrête pas la chaîne : le pull et le push
            # gardent leur sens sans commit neuf.
            commit_line = (f"if git diff --quiet HEAD; then echo {shlex.quote(NOTHING_TO_COMMIT)}; "
                           f"else {commit_line}; fi")
        steps = [RemoteCommand(f"commit « {message} »", commit_line)]
        if rebase:
            steps.append(PULL_REBASE)
        if push:
            steps.append(GIT_PUSH)
        self._run(steps, commits=True)

    def _run_free(self):
        line = self.free_var.get().strip()
        if not line:
            return
        if self._run([RemoteCommand(line, line)], commits=False):
            if not self._history or self._history[-1] != line:
                self._history.append(line)
                del self._history[:-HISTORY_LIMIT]
            self._history_pos = len(self._history)
            self.free_var.set("")

    def _history_step(self, delta):
        if not self._history:
            return "break"
        self._history_pos = max(0, min(len(self._history), self._history_pos + delta))
        text = self._history[self._history_pos] if self._history_pos < len(self._history) else ""
        self.free_var.set(text)
        self.free_entry.icursor("end")
        return "break"

    def _run(self, steps, commits=False):
        """Lance la chaîne `steps` ; False si elle n'a pas démarré."""
        if self.busy():
            self._write("Une commande tourne déjà.", "fail")
            return False
        for step in steps:
            if step.confirm and not messagebox.askyesno(step.label, step.confirm,
                                                        parent=self.win, icon="question"):
                return False
        self._queue = list(steps)
        self._commit_pending = commits
        self._chain_started = time.monotonic()
        if len(steps) > 1:
            self._write(f"══ {datetime.now():%H:%M:%S}  chaîne : "
                        + " → ".join(s.label for s in steps), "step")
        self._next_step()
        return True

    def _next_step(self):
        step = self._queue[0]
        line = self._shell_of(step)
        self._write(f"── {datetime.now():%H:%M:%S}  {self.target_var.get().strip()} $ {line}", "cmd")
        try:
            self.runner.start(self.ssh_argv(line), None)
        except OSError as exc:
            self._write(f"Lancement de ssh impossible : {exc}", "fail")
            self._finish(failed_label=step.label)
            return
        self._step_started = time.monotonic()
        self.run_lbl.config(text=f"en cours : {step.label}")
        self._update_buttons()

    def _on_exit(self, code):
        step = self._queue.pop(0)
        seconds = time.monotonic() - self._step_started
        self._write(f"   {step.label} : code {code}  ({seconds:.1f} s)", "ok" if code == 0 else "fail")
        if code != 0:
            self._finish(failed_label=step.label)
        elif self._queue:
            self._next_step()
        else:
            self._finish()

    def _finish(self, failed_label=None):
        skipped = [s.label for s in self._queue]
        self._queue = []
        if failed_label:
            text = f"   ÉCHEC à « {failed_label} »"
            if skipped:
                text += "  — non lancées : " + ", ".join(skipped)
            self._write(text, "fail")
            self.run_lbl.config(text=f"échec : {failed_label}")
            self.win.bell()
        else:
            if self._commit_pending:
                self.commit_var.set("")
            total = time.monotonic() - (self._chain_started or time.monotonic())
            self.run_lbl.config(text=f"réussi  ({total:.1f} s)")
        self._commit_pending = False
        self._update_buttons()

    def _stop(self):
        if self.runner.busy:
            self._write("Arrêt demandé.", "fail")
            self.runner.stop()

    def _update_buttons(self):
        state = "disabled" if self.busy() else "normal"
        for w in self.action_widgets:
            w.config(state=state)
        self.stop_btn.config(state="normal" if self.busy() else "disabled")

    # ---- sortie -------------------------------------------------------------
    def _drain(self):
        try:
            while True:
                kind, payload = self.events.get_nowait()
                if kind == "exit":
                    self._on_exit(payload)
                else:
                    self._write(payload, classify(payload))
        except queue.Empty:
            pass
        self.win.after(DRAIN_MS, self._drain)

    def _clear(self):
        self.console.configure(state="normal")
        self.console.delete("1.0", "end")
        self.console.configure(state="disabled")

    def _write(self, text, tag="out"):
        at_bottom = self.console.yview()[1] >= 0.999
        self.console.configure(state="normal")
        self.console.insert("end", text + "\n", tag)
        self.console.configure(state="disabled")
        if at_bottom:
            self.console.see("end")
