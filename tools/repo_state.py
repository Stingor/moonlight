"""État du dépôt de pc-serv, en tête de la section git du panneau SSH.

Une ligne : branche, commits à pousser et à recevoir (après un fetch),
fichiers modifiés. Relue à l'ouverture du panneau, après chaque chaîne et
sur « ↻ ».

Pendant un rebase ou une fusion interrompus par un conflit, une seconde ligne
apparaît : les fichiers en conflit, et de quoi finir ou abandonner sans rien
taper. « Continuer » refuse tant qu'un fichier en conflit porte encore ses
marqueurs (<<<<<<< ou >>>>>>> en début de ligne) : `git add -u` les
engagerait tels quels.
"""
import queue
import subprocess
import threading
from tkinter import ttk

from tables_panel import NO_WINDOW

STATE_TIMEOUT_S = 25
DRAIN_MS = 200
SHOWN_CONFLICTS = 6
FIELD_SEPARATOR = "|"
# Un relevé sur une seule ligne par champ, `clé=valeur`.
STATE_SHELL = (
    "git fetch -q 2>/dev/null; "
    "echo \"branch=$(git branch --show-current)\"; "
    "c=$(git rev-list --left-right --count @{u}...HEAD 2>/dev/null) && echo \"counts=$c\" || echo counts=; "
    "echo \"changed=$(git status --porcelain | wc -l)\"; "
    "g=$(git rev-parse --git-dir); "
    "if [ -d \"$g/rebase-merge\" ] || [ -d \"$g/rebase-apply\" ]; then echo op=rebase; "
    "elif [ -f \"$g/MERGE_HEAD\" ]; then echo op=merge; else echo op=; fi; "
    "echo \"conflicts=$(git diff --name-only --diff-filter=U | tr '\\n' '|')\""
)
# Refus de continuer tant qu'un fichier en conflit garde ses marqueurs.
MARKERS_GUARD = (
    "if git diff --name-only --diff-filter=U -z | xargs -0 -r grep -nE '^(<<<<<<<|>>>>>>>)( |$)'; then "
    "echo \"Marqueurs de conflit encore présents : résoudre ces lignes d'abord.\"; exit 1; fi; "
)


def parse_state(text):
    values = dict(line.split("=", 1) for line in text.splitlines() if "=" in line)
    counts = values.get("counts", "").split()
    behind, ahead = (int(counts[0]), int(counts[1])) if len(counts) == 2 else (None, None)
    return {
        "branch": values.get("branch", ""),
        "behind": behind,
        "ahead": ahead,
        "changed": int(values.get("changed", "0") or 0),
        "op": values.get("op", ""),
        "conflicts": [f for f in values.get("conflicts", "").split(FIELD_SEPARATOR) if f],
    }


def describe(state):
    parts = [state["branch"] or ("rebase en cours" if state["op"] == "rebase" else "HEAD détachée")]
    if state["ahead"] is None:
        parts.append("sans branche suivie")
    elif not state["ahead"] and not state["behind"]:
        parts.append("à jour avec le dépôt distant")
    else:
        if state["ahead"]:
            parts.append(f"{state['ahead']} à pousser")
        if state["behind"]:
            parts.append(f"{state['behind']} à recevoir")
    parts.append(f"{state['changed']} fichier(s) modifié(s)" if state["changed"] else "arbre propre")
    return "  ·  ".join(parts)


class RepoStateBar:
    """Les deux lignes, posées dans `parent` ; les commandes passent par la
    chaîne du panneau SSH (`ssh_window.run_chain`)."""

    def __init__(self, parent, ssh_window, make_command):
        self.ssh = ssh_window
        self._command = make_command  # RemoteCommand, passé pour éviter l'import circulaire
        self._results = queue.Queue()
        self._querying = False

        line = ttk.Frame(parent)
        line.pack(fill="x", padx=4, pady=(4, 0))
        self.state_lbl = ttk.Label(line, text="état du dépôt : …")
        self.state_lbl.pack(side="left")
        ttk.Button(line, text="↻", width=3, command=self.refresh).pack(side="right")

        self.conflict = ttk.Frame(parent)
        self.conflict_lbl = ttk.Label(self.conflict, text="", foreground="#e55")
        self.conflict_lbl.pack(side="left", padx=(0, 8))
        self.buttons = {}
        for key, text in (("continue", "Continuer"), ("skip", "Passer ce commit"), ("abort", "Abandonner")):
            button = ttk.Button(self.conflict, text=text, command=lambda k=key: self._resolve(k))
            button.pack(side="left", padx=2)
            self.buttons[key] = button
        self._op = ""
        parent.after(DRAIN_MS, lambda: self._drain(parent))

    # ---- relevé -------------------------------------------------------------
    def refresh(self):
        if self._querying:
            return
        self._querying = True
        self.state_lbl.config(text="état du dépôt : relevé…")
        argv = self.ssh.ssh_argv(STATE_SHELL)

        def worker():
            try:
                done = subprocess.run(argv, capture_output=True, stdin=subprocess.DEVNULL,
                                      timeout=STATE_TIMEOUT_S, creationflags=NO_WINDOW)
                result = (done.returncode, done.stdout.decode("utf-8", "replace"))
            except (OSError, subprocess.TimeoutExpired) as exc:
                result = (-1, str(exc))
            self._results.put(result)

        threading.Thread(target=worker, daemon=True).start()

    def _drain(self, widget):
        try:
            code, text = self._results.get_nowait()
        except queue.Empty:
            pass
        else:
            self._querying = False
            self._apply(code, text)
        widget.after(DRAIN_MS, lambda: self._drain(widget))

    def _apply(self, code, text):
        if code != 0 or "branch=" not in text:
            self.state_lbl.config(text="état du dépôt : relevé impossible", foreground="#d90")
            return
        state = parse_state(text)
        self.state_lbl.config(text=describe(state), foreground="")
        self._op = state["op"]
        if not self._op:
            self.conflict.pack_forget()
            return
        what = "Rebase" if self._op == "rebase" else "Fusion"
        files = state["conflicts"]
        shown = ", ".join(files[:SHOWN_CONFLICTS]) + (", …" if len(files) > SHOWN_CONFLICTS else "")
        self.conflict_lbl.config(text=f"{what} interrompu(e) : "
                                      + (f"en conflit : {shown}" if files else "conflits résolus, à continuer"))
        # « Passer ce commit » n'a de sens que dans un rebase.
        self.buttons["skip"].config(state="normal" if self._op == "rebase" else "disabled")
        self.conflict.pack(fill="x", padx=4, pady=(2, 2))

    # ---- résolution -----------------------------------------------------------
    def _resolve(self, key):
        command = self._command
        if self._op == "rebase":
            steps = {
                "continue": command("rebase --continue", MARKERS_GUARD + "git add -u && git rebase --continue"),
                "skip": command("rebase --skip", "git rebase --skip",
                                confirm="Abandonner le commit en conflit et poursuivre le rebase ?"),
                "abort": command("rebase --abort", "git rebase --abort",
                                 confirm="Abandonner le rebase et revenir à l'état d'avant le pull ?"),
            }
        else:
            steps = {
                "continue": command("terminer la fusion", MARKERS_GUARD + "git add -u && git commit --no-edit"),
                "abort": command("merge --abort", "git merge --abort",
                                 confirm="Abandonner la fusion et revenir à l'état d'avant le pull ?"),
            }
        if key in steps:
            self.ssh.run_chain([steps[key]])
