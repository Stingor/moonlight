"""Panneau « Serveur (SSH) » de MoonlightAdmin.

Lance sur pc-serv, par `ssh.exe` d'OpenSSH, les commandes courantes du dépôt
serveur : git (status, pull, commit, push) et make (clean, server). Chaque
commande s'exécute dans le dossier du dépôt : `cd <dépôt> && <commande>`.

L'identification passe par une clé SSH, jamais par un mot de passe : ssh est
lancé en `BatchMode=yes`, qui échoue au lieu d'attendre une saisie de la
connexion elle-même. `sudo` est appelé en `sudo -n` : sans règle NOPASSWD sur
le serveur, il refuse au lieu de rester bloqué.

Les commandes tournent avec un terminal distant (remote_console.py) : une
question qu'elles posent s'affiche, et le champ « Réponse » lui répond ;
« Ctrl+C » l'interrompt. Pour un éditeur ou un programme plein écran
(nano, htop, screen -r), « Terminal… » ouvre un vrai shell ssh dans une
console Windows, déjà dans le dossier du dépôt.

Une chaîne de commandes s'arrête à la première qui échoue, comme `&&`.
« Arrêter » tue ssh.exe : le terminal distant raccroche, ce qui arrête la
commande.

Le compte, le dossier du dépôt et l'emploi de sudo se retiennent dans la
section [ssh] de MoonlightAdmin.ini.
"""
import queue
import re
import shlex
import subprocess
import time
import tkinter as tk
from dataclasses import dataclass
from datetime import datetime
from tkinter import messagebox, scrolledtext, ttk

from remote_console import REMOTE_ENV, InteractiveRunner, visible
from debug_section import DebugSection
from repo_state import RepoStateBar
from tables_panel import CONSOLE_COLORS, DRAIN_MS
from tabs_host import PageSpec

SETTINGS_SECTION = "ssh"
PAGE = PageSpec("ssh", "Serveur (SSH)", "Moonlight - Serveur (SSH)", 700, 520)
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
PANEL_COLORS = dict(CONSOLE_COLORS, warn="#db5", step="#c8f", reply="#8cf")
# Un dossier que le shell distant lit tel quel, `~` développé : sinon cité.
PLAIN_PATH_RE = re.compile(r"[~\w./-]+")
MASK_CHAR = "•"

# Un script tiré par git pull, dans sa sortie : une ligne du diffstat
# (« moon/x.npc | 12 ++-- ») ou un résumé (« create mode 100644 moon/x.npc »).
# Un chemin raccourci par git (« .../x.npc ») ne se recharge pas : --stat
# et COLUMNS (remote_console.REMOTE_ENV) lui laissent toute sa largeur.
SCRIPT_STAT_RE = re.compile(r"^\s*(\S+\.(?:npc|txt))\s+\|\s+(?:\d+|Bin)")
SCRIPT_MODE_RE = re.compile(r"^\s*(create|delete) mode \d+ (\S+\.(?:npc|txt))\s*$")
SCRIPT_PATH_PREFIX = "moon/"
TRUNCATED_PATH_PREFIX = "..."
COMMAND_LINE_PREFIX = "── "
# Ce que l'atcommande écrit quand le chargement échoue ; la cause (ligne du
# script en défaut) part dans la console du map-server.
RELOAD_FAILURE_MARKERS = ("could not be loaded", "failed", "File not found")
# Réponse de MoonlightAdmin quand le canal admin n'est pas ouvert.
NOT_CONNECTED_REPLY = "ERR pas connecté"
SCRIPT_TAG = "script"
SCRIPT_TAG_COLOR = "#8cf"


@dataclass(frozen=True)
class PulledScript:
    path: str
    deleted: bool  # « delete mode » : il se décharge au lieu de se recharger

    def atcommand(self):
        return ("unloadnpcfile " if self.deleted else "reloadnpcfile ") + self.path


def pulled_script(line):
    """Le script qu'une ligne de sortie de git pull nomme, sinon None."""
    match = SCRIPT_MODE_RE.match(line)
    if match:
        return PulledScript(match.group(2), match.group(1) == "delete")
    match = SCRIPT_STAT_RE.match(line)
    if match:
        return PulledScript(match.group(1), False)
    return None


@dataclass(frozen=True)
class RemoteCommand:
    label: str
    shell: str            # ligne de shell exécutée dans le dossier du dépôt
    sudo: bool = False    # préfixée de `sudo -n` si la case sudo est cochée
    confirm: str = ""     # question posée avant de lancer, vide : aucune


# --autostash : les scripts s'éditent sur pc-serv même, et un arbre modifié
# ferait refuser le pull ; les éditions en cours sont mises de côté puis
# reposées par-dessus ce qui arrive.
# --stat : sans lui, un rebase ne liste pas les fichiers tirés, que le clic
# droit de la sortie propose de recharger.
PULL_REBASE = RemoteCommand("git pull --rebase", "git pull --rebase --autostash --stat")
GIT_PUSH = RemoteCommand("git push", "git push",
                         confirm="Pousser les commits de pc-serv vers le dépôt distant ?")
GIT_COMMANDS = (
    RemoteCommand("git status", "git status"),
    RemoteCommand("git log -10", "git log --oneline --decorate -10"),
    RemoteCommand("git diff --stat", "git diff --stat"),
    RemoteCommand("git fetch", "git fetch --prune"),
    RemoteCommand("git pull", "git pull --autostash --stat"),
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
    # (action, question avant de lancer : vide, aucune ; {effet} vient de STOP_EFFECTS)
    ("status", ""),
    ("start", ""),
    ("stop", "Arrêter « {cible} » ? {effet}"),
    ("restart", "Redémarrer « {cible} » ? {effet}"),
)
# Ce que l'arrêt d'une cible coûte aux joueurs : chaque service tourne dans son
# propre screen, et seul celui qu'on nomme s'arrête.
KICKS_PLAYERS = "Les joueurs connectés seront déconnectés."
STOP_EFFECTS = {
    "tout": KICKS_PLAYERS,
    "rathena": KICKS_PLAYERS,
    "map": KICKS_PLAYERS,
    "char": "Le map-server perd le char-server le temps du redémarrage : "
            "connexions et sauvegardes en attente.",
    "login": "Les joueurs en jeu restent connectés ; personne ne peut se connecter entre-temps.",
    "web": "Le site est indisponible le temps du redémarrage ; le jeu n'est pas touché.",
    "voice": "Le chat vocal coupe le temps du redémarrage ; le jeu n'est pas touché.",
    "moonbot": "Le bot Discord se reconnecte ; le jeu n'est pas touché.",
    "survivors": "La file de vérification s'interrompt ; le jeu n'est pas touché.",
}


NPC_CHECK = "tools/npc_check.py"
NPC_CHECK_MISSING = f"{NPC_CHECK} absent sur le serveur : contrôle d'encodage sauté (git pull pour l'avoir)."


def npc_check_command(paths=()):
    """Le contrôle d'encodage des scripts, première étape d'un commit : une
    erreur arrête la chaîne. Sans chemin, il porte sur ce que `commit -a`
    emporterait."""
    args = "".join(" " + shlex.quote(p) for p in paths)
    separator = " --" if paths else ""
    return RemoteCommand("contrôle d'encodage",
                         f"if [ -f {NPC_CHECK} ]; then python3 {NPC_CHECK}{separator}{args}; "
                         f"else echo {shlex.quote(NPC_CHECK_MISSING)}; fi")


# Une chaîne qui arrête des services : stop, restart, ou make + restart.
STOPPING_START_PREFIXES = ("./start stop", "./start restart")
# Après elle, un service trouvé arrêté passe pour demandé tant que dure ce délai
# (le map-server peut mettre plusieurs minutes à sauvegarder puis repartir).
PLANNED_STOP_WINDOW_S = 300


def start_command(action, target, confirm=""):
    return RemoteCommand(f"./start {action} {target}", f"./start {action} {shlex.quote(target)}",
                         confirm=confirm.format(cible=target, effet=STOP_EFFECTS.get(target, KICKS_PLAYERS)))


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
    """Page « Serveur (SSH) » (tabs_host.py), construite à sa première apparition."""

    def __init__(self, root, host, settings, bind_copy_menu, open_logs=None, open_npc=None, send_admin=None):
        self.root = root
        self.host = host
        self._open_logs = open_logs
        self._open_npc = open_npc
        self._send_admin = send_admin  # (atcommande, rappel) : le canal admin de la fenêtre principale
        self._reload_queue = []
        target = settings.get(SETTINGS_SECTION, "start_target", DEFAULT_START_TARGET)
        self.start_target_var = tk.StringVar(
            value=target if target in START_TARGETS else DEFAULT_START_TARGET)
        self.events = queue.Queue()
        self.runner = InteractiveRunner(self.events)
        self._pending = ""  # ligne en cours, pas encore finie : une invite, souvent
        self.target_var = tk.StringVar(value=settings.get(SETTINGS_SECTION, "target", DEFAULT_TARGET))
        self.repo_var = tk.StringVar(value=settings.get(SETTINGS_SECTION, "repo", DEFAULT_REPO))
        self.sudo_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "sudo_make", True))
        self.jobs_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "parallel_make", True))
        stored = settings.get(SETTINGS_SECTION, "history")
        self._history = [h for h in stored.split(HISTORY_SEPARATOR) if h] if stored else []
        self._history_pos = len(self._history)
        self._bind_copy_menu = bind_copy_menu
        self._queue = []
        self._chain_started = None
        self._step_started = None
        self._commit_pending = False  # la chaîne en cours commite : vider le message si elle réussit
        self._on_done = None          # rappel d'une chaîne lancée par une autre fenêtre
        self._service_action_at = float("-inf")  # fin du dernier ./start stop ou restart
        self._chain_stops = False     # la chaîne en cours arrête des services
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
        }

    def busy(self):
        # La chaîne compte, pas seulement ssh : entre la fin d'un processus et
        # la lecture de son code, une chaîne lancée recevrait ce code périmé.
        return self.runner.busy or bool(self._queue)

    # ---- fenêtre ------------------------------------------------------------
    def show(self):
        self.host.reveal(PAGE.key)

    def on_page_shown(self):
        if self.win is None:
            self._build()
            self.repo_state.refresh()
            self.debug.refresh()

    def on_page_hidden(self):
        pass

    def _build(self):
        self.win = self.host.body(PAGE.key)
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
        ttk.Button(conn, text="Terminal…", command=self.open_terminal).grid(row=0, column=5, padx=4)
        ttk.Label(conn, text="Identification par clé SSH : aucun mot de passe n'est demandé ni retenu. "
                             "Terminal… : un vrai shell, pour nano, htop ou screen -r.",
                  foreground="#888").grid(row=1, column=0, columnspan=6, padx=4, pady=(0, 4), sticky="w")

        git = ttk.LabelFrame(body, text="git")
        git.pack(fill="x", padx=8, pady=4)
        self.repo_state = RepoStateBar(git, self, RemoteCommand)
        row = ttk.Frame(git)
        row.pack(fill="x", padx=4, pady=(4, 2))
        for cmd in GIT_COMMANDS:
            self._action(ttk.Button(row, text=cmd.label, command=lambda c=cmd: self._run([c]))).pack(
                side="left", padx=2)
        if self._open_npc is not None:
            ttk.Button(row, text="Atelier NPC…", command=self._open_npc).pack(side="right", padx=2)
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

        self.debug = DebugSection(body, self, RemoteCommand)

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
        self.console.pack(fill="both", expand=True, padx=4, pady=(4, 2))
        for tag, color in PANEL_COLORS.items():
            self.console.tag_config(tag, foreground=color)
        # La ligne en cours s'écrit après cette marque, et s'efface quand elle
        # se complète : une invite s'y montre avant sa fin de ligne.
        self.console.mark_set("partial", "end-1c")
        self.console.mark_gravity("partial", "left")
        self.console.tag_config(SCRIPT_TAG, foreground=SCRIPT_TAG_COLOR, underline=True)
        self._bind_copy_menu(self.console, extra=self._script_menu)

        reply = ttk.Frame(out)
        reply.pack(fill="x", padx=4, pady=(0, 4))
        ttk.Label(reply, text="Réponse").pack(side="left", padx=(0, 4))
        self.reply_var = tk.StringVar()
        self.reply_entry = ttk.Entry(reply, textvariable=self.reply_var)
        self.reply_entry.pack(side="left", fill="x", expand=True, padx=2)
        self.reply_entry.bind("<Return>", lambda _e: self._send_reply())
        self.mask_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(reply, text="masquer", variable=self.mask_var,
                        command=lambda: self.reply_entry.config(
                            show=MASK_CHAR if self.mask_var.get() else "")).pack(side="left", padx=4)
        self.reply_btns = [ttk.Button(reply, text="Envoyer", command=self._send_reply),
                           ttk.Button(reply, text="Ctrl+C", command=self._interrupt)]
        for button in self.reply_btns:
            button.pack(side="left", padx=2)

        self.win.after(DRAIN_MS, self._drain)

    def _action(self, widget):
        """Un bouton qui lance une commande : grisé tant qu'une autre tourne."""
        self.action_widgets.append(widget)
        return widget

    # ---- composition des commandes -----------------------------------------
    def shell_of(self, cmd):
        line = cmd.shell
        if cmd is MAKE_SERVER and self.jobs_var.get():
            line += ' -j"$(nproc)"'
        if cmd.sudo and self.sudo_var.get():
            line = "sudo -n " + line
        return line

    def tty_argv(self, shell_line):
        """La commande de la chaîne, avec un terminal distant (-tt) qui lui
        permet de poser une question et d'en attendre la réponse."""
        remote = f"{REMOTE_ENV}cd {remote_dir(self.repo_var.get().strip() or DEFAULT_REPO)} && {shell_line}"
        # -q : sans la ligne « Connection to … closed. » qu'ajoute -tt.
        return ["ssh", "-tt", "-q", "-o", "BatchMode=yes",
                "-o", f"ConnectTimeout={CONNECT_TIMEOUT_S}",
                "-o", f"ServerAliveInterval={SERVER_ALIVE_INTERVAL_S}",
                self.target_var.get().strip() or DEFAULT_TARGET, remote]

    def open_terminal(self):
        """Un vrai shell ssh dans une console Windows, dans le dossier du dépôt :
        pour un éditeur ou un programme plein écran."""
        repo = self.repo_var.get().strip() or DEFAULT_REPO
        path = repo if PLAIN_PATH_RE.fullmatch(repo) else shlex.quote(repo)
        argv = ["ssh", "-t", self.target_var.get().strip() or DEFAULT_TARGET, f"cd {path} && exec bash -l"]
        try:
            subprocess.Popen(argv, creationflags=getattr(subprocess, "CREATE_NEW_CONSOLE", 0))
        except OSError as exc:
            self._write(f"Terminal impossible : {exc}", "fail")

    def ssh_argv(self, shell_line):
        """Sans terminal : pour les fenêtres qui lisent une sortie d'un bloc."""
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
        steps = [npc_check_command(), RemoteCommand(f"commit « {message} »", commit_line)]
        if rebase:
            steps.append(PULL_REBASE)
        if push:
            steps.append(GIT_PUSH)
        self._run(steps, commits=True)

    def service_action_recent(self):
        """Vrai pendant une chaîne qui arrête des services, et un temps après :
        le bandeau d'état y voit un arrêt demandé, pas une panne."""
        return self.busy() or time.monotonic() - self._service_action_at < PLANNED_STOP_WINDOW_S

    def run_chain(self, steps, on_done=None):
        """Lance `steps` pour une autre fenêtre, sortie montrée ici ;
        `on_done(réussi)` est rappelé à la fin. False si rien n'a démarré."""
        self.show()
        if self.busy():
            self._write("Une commande tourne déjà.", "fail")
            return False
        # Posé avant le départ : un ssh qui ne se lance pas finit la chaîne
        # aussitôt, et le rappel doit déjà être là.
        self._on_done = on_done
        if not self._run(steps):
            self._on_done = None
            return False
        return True

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
        self._chain_stops = any(s.shell.startswith(STOPPING_START_PREFIXES) for s in steps)
        if self._chain_stops:
            self._service_action_at = time.monotonic()
        self._commit_pending = commits
        self._chain_started = time.monotonic()
        if len(steps) > 1:
            self._write(f"══ {datetime.now():%H:%M:%S}  chaîne : "
                        + " → ".join(s.label for s in steps), "step")
        self._next_step()
        return True

    def _next_step(self):
        step = self._queue[0]
        line = self.shell_of(step)
        self._write(f"── {datetime.now():%H:%M:%S}  {self.target_var.get().strip()} $ {line}", "cmd")
        try:
            self.runner.start(self.tty_argv(line))
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
        if self._chain_stops:
            # Le délai court depuis la fin de la chaîne : un arrêt long ne l'use pas.
            self._service_action_at = time.monotonic()
            self._chain_stops = False
        # Une chaîne a pu commiter, tirer, laisser un conflit, ou un service
        # mourir : l'état du dépôt et la liste des cores se relisent.
        self.repo_state.refresh()
        self.debug.refresh()
        on_done, self._on_done = self._on_done, None
        if on_done is not None:
            on_done(failed_label is None)

    def _stop(self):
        if self.runner.busy:
            self._write("Arrêt demandé.", "fail")
            self.runner.stop()

    def _update_buttons(self):
        state = "disabled" if self.busy() else "normal"
        for w in self.action_widgets:
            w.config(state=state)
        self.stop_btn.config(state="normal" if self.busy() else "disabled")
        for button in self.reply_btns:
            button.config(state="normal" if self.busy() else "disabled")

    # ---- réponses -----------------------------------------------------------
    def _send_reply(self):
        text = self.reply_var.get()
        if not self.runner.send(text + "\n"):
            self._write("Aucune commande n'attend de réponse.", "fail")
            return
        self.reply_var.set("")
        # Le terminal distant renvoie lui-même ce qu'on tape, sauf pour un mot
        # de passe : l'écho manquant se signale ici, sans le texte.
        if self.mask_var.get():
            self._write("(réponse masquée envoyée)", "reply")

    def _interrupt(self):
        if self.runner.interrupt():
            self._write("^C", "fail")

    # ---- scripts tirés : rechargement en jeu ----------------------------------
    def _script_menu(self, menu, index):
        """Le clic droit sur la sortie d'un pull : recharger le script de la
        ligne cliquée, ou tous ceux que ce pull a tirés."""
        if self._send_admin is None:
            return
        line_no = int(index.split(".")[0])
        lines = self.console.get("1.0", "end-1c").split("\n")
        clicked = pulled_script(lines[line_no - 1]) if line_no <= len(lines) else None
        block = self._pulled_block(lines, line_no - 1)
        if clicked is None and not block:
            return
        menu.add_separator()
        if clicked is not None:
            verb = "Décharger" if clicked.deleted else "Recharger"
            menu.add_command(label=f"{verb} {clicked.path}", state=self._reload_state(clicked),
                             command=lambda: self._reload_scripts([clicked]))
        reloadable = [s for s in block if self._reload_state(s) == "normal"]
        if len(block) > 1 or (block and clicked is None):
            count = len(reloadable)
            noun = "script" if count == 1 else "scripts"
            menu.add_command(label=f"Recharger les {count} {noun} de moon/ de ce pull",
                             state="normal" if reloadable else "disabled",
                             command=lambda: self._reload_scripts(reloadable))

    @staticmethod
    def _pulled_block(lines, line_idx):
        """Les scripts nommés par la sortie de la commande qui contient la
        ligne `line_idx`, d'une ligne « ── » à la suivante ; chacun une fois,
        le résumé « delete mode » l'emportant sur la ligne du diffstat."""
        start = line_idx
        while start > 0 and not lines[start].startswith(COMMAND_LINE_PREFIX):
            start -= 1
        end = line_idx + 1
        while end < len(lines) and not lines[end].startswith(COMMAND_LINE_PREFIX):
            end += 1
        found = {}
        for line in lines[start:end]:
            script = pulled_script(line)
            if script is not None and (script.path not in found or script.deleted):
                found[script.path] = script
        return list(found.values())

    @staticmethod
    def _reload_state(script):
        # Le serveur ne recharge un par un que les scripts de moon/ ; un chemin
        # raccourci par git ne désigne aucun fichier.
        ok = script.path.startswith(SCRIPT_PATH_PREFIX) and not script.path.startswith(TRUNCATED_PATH_PREFIX)
        return "normal" if ok else "disabled"

    def _reload_scripts(self, scripts):
        if self._reload_queue:
            self._write("Des rechargements sont déjà en cours.", "fail")
            return
        self._reload_queue = list(scripts)
        self._next_reload()

    def _next_reload(self):
        if not self._reload_queue:
            return
        script = self._reload_queue[0]
        self._write(f"> {script.atcommand()}", "cmd")
        self._send_admin(script.atcommand(), lambda output, reply: self._on_reloaded(script, output, reply))

    def _on_reloaded(self, script, output, reply):
        self._reload_queue.pop(0)
        if reply.startswith(NOT_CONNECTED_REPLY):
            self._reload_queue = []
            self._write("  Pas connecté : connecter d'abord la fenêtre principale au canal admin.", "fail")
            return
        failed = not reply.startswith("OK") or any(
            m in line for line in output for m in RELOAD_FAILURE_MARKERS)
        for line in output:
            self._write("  " + line, "fail" if failed else "out")
        if failed:
            self._write(f"  {script.path} : échec, la ligne en défaut est dans la console du map-server.", "fail")
        else:
            self._write(f"  {script.path} : {'déchargé' if script.deleted else 'rechargé'}", "ok")
        self._next_reload()

    # ---- sortie -------------------------------------------------------------
    def _drain(self):
        try:
            while True:
                kind, payload = self.events.get_nowait()
                if kind == "chunk":
                    self._feed(payload)
                elif kind == "exit":
                    if self._pending:
                        line, self._pending = visible(self._pending), ""
                        self._write(line, classify(line))
                    self._on_exit(payload)
        except queue.Empty:
            pass
        self.win.after(DRAIN_MS, self._drain)

    def _feed(self, text):
        """Un morceau de sortie : chaque ligne finie s'écrit, la ligne en cours
        (une invite) se montre telle quelle après la marque « partial »."""
        self._pending += text
        *lines, self._pending = self._pending.split("\n")
        for line in lines:
            shown = visible(line)
            self._write(shown, classify(shown))
        if not lines:
            self._write_partial()

    def _clear(self):
        self.console.configure(state="normal")
        self.console.delete("1.0", "end")
        self.console.mark_set("partial", "end-1c")
        self.console.configure(state="disabled")
        self._write_partial()

    def _write(self, text, tag="out"):
        at_bottom = self.console.yview()[1] >= 0.999
        self.console.configure(state="normal")
        self.console.delete("partial", "end-1c")
        tags = (tag, SCRIPT_TAG) if tag == "out" and pulled_script(text) else tag
        self.console.insert("end-1c", text + "\n", tags)
        self.console.mark_set("partial", "end-1c")
        self.console.configure(state="disabled")
        self._write_partial()
        if at_bottom:
            self.console.see("end")

    def _write_partial(self):
        self.console.configure(state="normal")
        self.console.delete("partial", "end-1c")
        partial = visible(self._pending)
        if partial:
            self.console.insert("end-1c", partial, "out")
        self.console.configure(state="disabled")
