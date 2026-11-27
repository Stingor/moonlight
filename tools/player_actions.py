"""Actions sur un joueur, au clic droit des consoles de MoonlightAdmin.

Sélectionner un nom dans une console (ou cliquer droit sur un mot) ouvre, en
plus du menu de copie, un sous-menu d'atcommandes sur ce personnage :
informations, inventaire, puis sanctions, qui demandent confirmation. Les
formes suivent atcommand.cpp : `mute` prend des minutes puis le nom,
`jailfor` et `ban` une durée (30mn, 2h, 1d, +7d) puis le nom. Une commande
de personnage (`#itemlist`) met le nom entre guillemets, ce qui laisse
passer un nom à espaces.
"""
import tkinter as tk
from dataclasses import dataclass
from tkinter import messagebox, simpledialog

MAX_DURATION_LENGTH = 20


@dataclass(frozen=True)
class PlayerAction:
    label: str
    template: str        # {name}, et {duration} si `ask` est posé
    ask: tuple = ()      # (question, valeur proposée) pour {duration}
    confirm: bool = False


SEPARATOR = None
PLAYER_ACTIONS = (
    PlayerAction("Infos du compte (accinfo)", "accinfo {name}"),
    PlayerAction("Où est-il (where)", "where {name}"),
    PlayerAction("Inventaire (#itemlist)", '#itemlist "{name}"'),
    PlayerAction("Stockage (#storagelist)", '#storagelist "{name}"'),
    SEPARATOR,
    PlayerAction("Muter…", "mute {duration} {name}", ("Durée du silence, en minutes :", "30"), True),
    PlayerAction("Rendre la parole (unmute)", "unmute {name}"),
    PlayerAction("Expulser (kick)", "kick {name}", confirm=True),
    PlayerAction("Prison…", "jailfor {duration} {name}", ("Durée de prison (30mn, 2h, 1d) :", "1h"), True),
    PlayerAction("Sortir de prison (unjail)", "unjail {name}"),
    PlayerAction("Bannir le compte…", "ban {duration} {name}",
                 ("Durée du bannissement (+1d, +7d, +1m) :", "+1d"), True),
)


def add_player_menu(menu, name, send, parent):
    """Ajoute à `menu` le sous-menu des actions sur `name` ; `send(atcmd)`
    envoie l'atcommande par le canal admin."""
    sub = tk.Menu(menu, tearoff=0)
    for action in PLAYER_ACTIONS:
        if action is SEPARATOR:
            sub.add_separator()
            continue
        sub.add_command(label=action.label,
                        command=lambda a=action: run_action(a, name, send, parent))
    menu.add_cascade(label=f"Joueur « {name} »", menu=sub)


def build_command(action, name, duration=""):
    return action.template.format(name=name, duration=duration)


def run_action(action, name, send, parent):
    duration = ""
    if action.ask:
        question, proposed = action.ask
        answer = simpledialog.askstring(action.label.rstrip("…"), f"{question}\n\nJoueur : {name}",
                                        initialvalue=proposed, parent=parent)
        duration = (answer or "").strip()
        if not duration or " " in duration or len(duration) > MAX_DURATION_LENGTH:
            return
    command = build_command(action, name, duration)
    if action.confirm and not messagebox.askyesno(
            action.label.rstrip("…"), f"{action.label.rstrip('…')} « {name} » ?\n\n@{command}",
            parent=parent, icon="warning", default="no"):
        return
    send(command)
