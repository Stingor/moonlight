"""Fenêtre « Maintenance programmée » de MoonlightAdmin.

Un compte à rebours qui prévient les joueurs, puis redémarre les services :

  1. au lancement, la préparation part tout de suite, le jeu tournant
     encore : git pull --rebase et make server, si cochés, dans la fenêtre
     « Serveur (SSH) ». Si elle échoue, la maintenance est reportée sans
     avoir expulsé personne ni rien arrêté ;
  2. pendant le compte à rebours, des annonces `kami` à chaque palier
     (ANNOUNCE_MINUTES, puis 30 s avant) ;
  3. à l'heure dite, la préparation finie : annonce finale, kickall, puis
     ./start restart de la cible.

« Annuler » vaut jusqu'au kickall, et l'annule auprès des joueurs. Après, la
chaîne se suit et s'arrête dans la fenêtre Serveur (SSH). « À blanc » écrit
chaque annonce et chaque commande sans rien envoyer ni lancer.

Les annonces passent par le canal admin, qui encode en Windows-1252 : leur
texte s'en tient aux caractères de cette page.
"""
import time
import tkinter as tk
from datetime import datetime, timedelta
from tkinter import messagebox, scrolledtext, ttk

from ssh_panel import MAKE_SERVER, PULL_REBASE, START_TARGETS, RemoteCommand, start_command

SETTINGS_SECTION = "maintenance"
DELAYS_MIN = (1, 5, 10, 15, 30)
DEFAULT_DELAY_MIN = 10
DEFAULT_REASON = "mise à jour du serveur"
DEFAULT_TARGET = "tout"
# Paliers d'annonce en minutes avant l'heure ; celui du lancement s'y ajoute.
ANNOUNCE_MINUTES = (30, 15, 10, 5, 3, 2, 1)
LAST_CALL_S = 30
TICK_MS = 1000
SECONDS_PER_MINUTE = 60
MAX_ANNOUNCE_LENGTH = 255
THRESHOLDS_S = frozenset({m * SECONDS_PER_MINUTE for m in ANNOUNCE_MINUTES} | {LAST_CALL_S})
LOG_COLORS = {"info": "#999", "say": "#6af", "ok": "#5c5", "fail": "#f55", "cmd": "#c8f"}


def announce_text(seconds_left, reason):
    if seconds_left < SECONDS_PER_MINUTE:
        when = f"{seconds_left} seconde{'s' if seconds_left > 1 else ''}"
    else:
        minutes = round(seconds_left / SECONDS_PER_MINUTE)
        when = f"{minutes} minute{'s' if minutes > 1 else ''}"
    return f"Maintenance dans {when} : {reason}. Pensez à vous mettre à l'abri."


class MaintenanceWindow:
    """Fenêtre « Maintenance programmée », créée une fois, masquée à la
    fermeture ; le compte à rebours continue, fenêtre masquée."""

    def __init__(self, root, settings, ssh_window, send_admin, log):
        self.root = root
        self.ssh = ssh_window
        self.send_admin = send_admin
        self.log = log  # (texte, alerte) vers le journal de la fenêtre principale
        self._geometry = settings.get(SETTINGS_SECTION, "geometry")
        self.delay_var = tk.StringVar(value=settings.get(SETTINGS_SECTION, "delay_min", str(DEFAULT_DELAY_MIN)))
        self.reason_var = tk.StringVar(value=settings.get(SETTINGS_SECTION, "reason", DEFAULT_REASON))
        target = settings.get(SETTINGS_SECTION, "target", DEFAULT_TARGET)
        self.target_var = tk.StringVar(value=target if target in START_TARGETS else DEFAULT_TARGET)
        self.pull_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "pull", True))
        self.make_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "make", True))
        self.kick_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "kickall", True))
        self.dry_var = tk.BooleanVar(value=False)
        self._deadline = None     # time.monotonic() de l'heure dite
        self._announced = set()   # paliers déjà annoncés, en secondes
        self._prep = "aucune"     # aucune | en cours | réussie | échouée
        self._phase = "idle"      # idle | countdown | restarting
        self._dry = False
        self.win = None

    # ---- réglages -----------------------------------------------------------
    def settings_data(self):
        return {"delay_min": self.delay_var.get(), "reason": self.reason_var.get().strip(),
                "target": self.target_var.get(), "pull": self.pull_var.get(),
                "make": self.make_var.get(), "kickall": self.kick_var.get(),
                "geometry": self._current_geometry()}

    def _current_geometry(self):
        try:
            if self.win is not None and self.win.winfo_exists():
                return self.win.geometry()
        except tk.TclError:  # fenêtre principale déjà détruite
            pass
        return self._geometry

    def busy(self):
        return self._phase != "idle"

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
        self.win.title("Moonlight - Maintenance programmée")
        self.win.minsize(620, 460)
        if self._geometry:
            try:
                self.win.geometry(self._geometry)
            except tk.TclError:
                pass
        self.win.protocol("WM_DELETE_WINDOW", self._hide)
        body = ttk.Frame(self.win)
        body.pack(fill="both", expand=True)

        form = ttk.LabelFrame(body, text="Maintenance")
        form.pack(fill="x", padx=8, pady=(8, 4))
        form.columnconfigure(1, weight=1)
        self.inputs = []
        ttk.Label(form, text="Dans (minutes)").grid(row=0, column=0, padx=4, pady=3, sticky="w")
        delay = ttk.Combobox(form, textvariable=self.delay_var, values=DELAYS_MIN, width=6)
        delay.grid(row=0, column=1, padx=4, sticky="w")
        ttk.Label(form, text="Raison annoncée").grid(row=1, column=0, padx=4, pady=3, sticky="w")
        reason = ttk.Entry(form, textvariable=self.reason_var)
        reason.grid(row=1, column=1, padx=4, sticky="ew")
        ttk.Label(form, text="Redémarrer").grid(row=2, column=0, padx=4, pady=3, sticky="w")
        target = ttk.Combobox(form, textvariable=self.target_var, values=START_TARGETS,
                              state="readonly", width=10)
        target.grid(row=2, column=1, padx=4, sticky="w")
        self.target_box = target
        options = ttk.Frame(form)
        options.grid(row=3, column=0, columnspan=2, padx=2, pady=(2, 4), sticky="w")
        checks = [ttk.Checkbutton(options, text=text, variable=var) for text, var in (
            ("git pull --rebase", self.pull_var), ("make server", self.make_var),
            ("kickall avant l'arrêt", self.kick_var), ("à blanc (rien n'est envoyé)", self.dry_var))]
        for check in checks:
            check.pack(side="left", padx=(2, 10))
        self.inputs = [delay, reason, target, *checks]
        ttk.Label(form, text="pull et make tournent dès le lancement, le jeu encore ouvert : un échec "
                             "reporte la maintenance avant toute expulsion.",
                  foreground="#888", wraplength=580).grid(row=4, column=0, columnspan=2, padx=4,
                                                          pady=(0, 4), sticky="w")

        bar = ttk.Frame(body)
        bar.pack(fill="x", padx=8, pady=4)
        self.start_btn = ttk.Button(bar, text="Lancer le compte à rebours", command=self._start)
        self.start_btn.pack(side="left")
        self.cancel_btn = ttk.Button(bar, text="Annuler", command=self._cancel, state="disabled")
        self.cancel_btn.pack(side="left", padx=6)
        self.countdown_lbl = ttk.Label(bar, text="", font=("TkDefaultFont", 12, "bold"))
        self.countdown_lbl.pack(side="left", padx=12)

        out = ttk.LabelFrame(body, text="Déroulé")
        out.pack(fill="both", expand=True, padx=8, pady=(4, 8))
        self.output = scrolledtext.ScrolledText(out, height=12, state="disabled", wrap="word",
                                                background="#111", foreground="#ccc")
        self.output.pack(fill="both", expand=True, padx=4, pady=4)
        for tag, color in LOG_COLORS.items():
            self.output.tag_config(tag, foreground=color)

    # ---- déroulé ------------------------------------------------------------
    def _start(self):
        try:
            delay_min = int(self.delay_var.get())
        except ValueError:
            messagebox.showerror("Maintenance", "Délai en minutes entier, 1 au moins.", parent=self.win)
            return
        reason = self.reason_var.get().strip() or DEFAULT_REASON
        if delay_min < 1:
            messagebox.showerror("Maintenance", "Délai en minutes entier, 1 au moins.", parent=self.win)
            return
        if len(announce_text(SECONDS_PER_MINUTE * delay_min, reason)) > MAX_ANNOUNCE_LENGTH:
            messagebox.showerror("Maintenance", "Raison trop longue pour une annonce.", parent=self.win)
            return
        try:
            reason.encode("cp1252")
        except UnicodeEncodeError:
            messagebox.showerror("Maintenance", "La raison contient un caractère que le jeu "
                                 "n'affiche pas (hors Windows-1252).", parent=self.win)
            return
        if self.ssh.busy():
            messagebox.showerror("Maintenance", "Une commande tourne déjà dans la fenêtre Serveur (SSH).",
                                 parent=self.win)
            return
        self._dry = self.dry_var.get()
        target = self.target_var.get()
        at = datetime.now() + timedelta(minutes=delay_min)
        prep = [name for name, var in (("git pull --rebase", self.pull_var), ("make server", self.make_var))
                if var.get()]
        summary = (f"Maintenance à {at:%H:%M} (dans {delay_min} min) : « {reason} ».\n\n"
                   f"Tout de suite : {', '.join(prep) or 'aucune préparation'}.\n"
                   f"À l'heure dite : {'kickall, ' if self.kick_var.get() else ''}./start restart {target}.")
        if self._dry:
            summary += "\n\nÀ BLANC : rien ne sera envoyé ni lancé."
        if not messagebox.askyesno("Lancer la maintenance ?", summary, parent=self.win,
                                   icon="question" if self._dry else "warning"):
            return
        self._clear()
        self._reason = reason
        self._target = target
        total_s = delay_min * SECONDS_PER_MINUTE
        self._deadline = time.monotonic() + total_s
        # Le lancement annonce le délai réel ; les paliers qu'il couvre déjà
        # ne se répètent pas.
        self._announced = {s for s in THRESHOLDS_S if s >= total_s}
        self._phase = "countdown"
        self._set_inputs("disabled")
        self.cancel_btn.config(state="normal")
        self.start_btn.config(state="disabled")
        self._write(f"Maintenance lancée pour {at:%H:%M:%S}" + (" — à blanc" if self._dry else ""), "info")
        self.log(f"Maintenance programmée à {at:%H:%M} : {reason}" + (" (à blanc)" if self._dry else ""), False)
        self._say(announce_text(total_s, reason))
        self._start_prep()
        self._tick()

    def _start_prep(self):
        steps = []
        if self.pull_var.get():
            steps.append(PULL_REBASE)
        if self.make_var.get():
            steps.append(MAKE_SERVER)
        if not steps:
            self._prep = "réussie"
            return
        self._prep = "en cours"
        if self._dry:
            steps = [self._dry_step(s) for s in steps]
        self._write("Préparation : " + " → ".join(s.label for s in steps), "cmd")
        if not self.ssh.run_chain(steps, on_done=self._on_prep_done):
            self._on_prep_done(False)

    def _dry_step(self, step):
        """L'étape réduite à un écho : la chaîne se déroule sans rien lancer."""
        line = self.ssh.shell_of(step).replace("'", "")
        return RemoteCommand(f"à blanc : {step.label}", f"echo 'à blanc : {line}'")

    def _on_prep_done(self, succeeded):
        if self._phase != "countdown":
            return
        if succeeded:
            self._prep = "réussie"
            self._write("Préparation réussie : il ne restera que le redémarrage.", "ok")
            return
        self._prep = "échouée"
        self._write("Préparation échouée : maintenance reportée, rien n'a été arrêté.", "fail")
        self.log("Maintenance reportée : la préparation a échoué (voir Serveur (SSH)).", True)
        self._say(f"Maintenance reportée : {self._reason}. Le jeu continue normalement.")
        self._end()

    def _tick(self):
        if self._phase != "countdown":
            return
        left = max(0, int(round(self._deadline - time.monotonic())))
        # Le premier palier non annoncé que le temps restant a franchi ; ceux
        # qu'un retard du minuteur aurait sautés ne s'annoncent pas en rafale.
        due = [s for s in THRESHOLDS_S if s not in self._announced and left <= s]
        if due and left > 0:
            self._announced.update(s for s in THRESHOLDS_S if s >= min(due))
            self._say(announce_text(min(due), self._reason))
        minutes, seconds = divmod(left, SECONDS_PER_MINUTE)
        state = {"en cours": " · préparation en cours", "réussie": " · préparation faite"}.get(self._prep, "")
        self.countdown_lbl.config(text=f"{minutes:02d}:{seconds:02d}{state}")
        if left == 0:
            # Une préparation qui finit, ou toute autre commande de la fenêtre
            # SSH : le redémarrage l'attend, sans quoi kickall partirait seul.
            if self._prep == "en cours" or self.ssh.busy():
                self.countdown_lbl.config(text="00:00 · attente de la fenêtre Serveur (SSH)")
            else:
                self._restart()
                return
        self.win.after(TICK_MS, self._tick)

    def _restart(self):
        self._phase = "restarting"
        self.cancel_btn.config(state="disabled")
        self._say(f"Maintenance : redémarrage du serveur maintenant ({self._reason}). "
                  f"Retour dans quelques minutes.")
        if self.kick_var.get():
            self._admin("kickall")
        step = start_command("restart", self._target)
        if self._dry:
            step = self._dry_step(step)
        self._write(f"Redémarrage : {step.label}", "cmd")
        if not self.ssh.run_chain([step], on_done=self._on_restarted):
            self._on_restarted(False)

    def _on_restarted(self, succeeded):
        if succeeded:
            self._write("Maintenance terminée : services redémarrés.", "ok")
            self.log("Maintenance terminée.", False)
        else:
            self._write("Le redémarrage n'a pas abouti : voir Serveur (SSH).", "fail")
            self.log("Maintenance : le redémarrage n'a pas abouti (voir Serveur (SSH)).", True)
        self._end()

    def _cancel(self):
        if self._phase != "countdown":
            return
        if not messagebox.askyesno("Annuler la maintenance ?", "Annoncer aux joueurs que la maintenance "
                                   "est annulée ?", parent=self.win):
            return
        self._say(f"Maintenance annulée : {self._reason}. Le jeu continue normalement.")
        self._write("Maintenance annulée.", "fail")
        self.log("Maintenance annulée.", False)
        if self._prep == "en cours":
            self._write("La préparation continue dans Serveur (SSH) : rien n'est arrêté pour autant.", "info")
        self._end()

    def _end(self):
        self._phase = "idle"
        self._deadline = None
        self._set_inputs("normal")
        self.cancel_btn.config(state="disabled")
        self.start_btn.config(state="normal")

    # ---- envois -------------------------------------------------------------
    def _say(self, text):
        self._admin(f"kami {text}", shown=f"« {text} »")

    def _admin(self, atcmd, shown=None):
        self._write(("[à blanc] " if self._dry else "") + (shown or f"@{atcmd}"), "say")
        if self._dry:
            return
        self.send_admin(atcmd, lambda _out, reply, c=atcmd: self._on_admin(c, reply))

    def _on_admin(self, atcmd, reply):
        if not reply.startswith("OK"):
            self._write(f"  échec de @{atcmd.split(' ', 1)[0]} : {reply}", "fail")

    # ---- sortie -------------------------------------------------------------
    def _set_inputs(self, state):
        for widget in self.inputs:
            if widget is self.target_box:
                widget.config(state="readonly" if state == "normal" else "disabled")
            else:
                widget.config(state=state)

    def _clear(self):
        self.output.configure(state="normal")
        self.output.delete("1.0", "end")
        self.output.configure(state="disabled")

    def _write(self, text, tag="info"):
        self.output.configure(state="normal")
        self.output.insert("end", f"{datetime.now():%H:%M:%S}  {text}\n", tag)
        self.output.configure(state="disabled")
        self.output.see("end")
