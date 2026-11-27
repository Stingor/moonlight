"""Catalogue des atcommandes du serveur, pour l'aide et la complétion de la
commande libre de MoonlightAdmin, et les actions sur un joueur des consoles.

Le catalogue est la table que moonclient tire de ce dépôt même
(scripts/server_atcommands.json, régénérée par
scripts/gen_server_atcommands.py) : nom, alias, forme, aide et niveau de GM
requis. Il se lit dans le dépôt moonclient du panneau Tables ; absent, la
commande libre marche sans aide.

Les actions sur un joueur ne lisent pas leur forme dans le catalogue : elle y
vient de l'aide de conf/atcommands.yml, quand l'action suit ce qu'attend
atcommand.cpp.
"""
import json
import os
from dataclasses import dataclass

CATALOG_RELATIVE_PATH = os.path.join("scripts", "server_atcommands.json")
COMMAND_SYMBOLS = "@#"
RAW_PREFIXES = ("RELOAD ", "LOG ", "NAME ", "QUIT")
SCRIPT_SUFFIXES = (".npc", ".txt")
SHOWN_MATCHES = 8
# NAME_LENGTH du serveur, terminateur compris.
MAX_CHAR_NAME_LENGTH = 23
NAME_DELIMITERS = " \t'\"«»[](){}<>:;,.!?"


@dataclass(frozen=True)
class Atcommand:
    name: str
    aliases: tuple
    usage: str
    help: str
    level: int


class AtcommandCatalog:
    def __init__(self, commands):
        self._by_word = {}
        for command in commands:
            for word in (command.name, *command.aliases):
                self._by_word.setdefault(word, command)
        self.words = sorted(self._by_word)

    @classmethod
    def load(cls, moonclient_repo):
        """Le catalogue du dépôt moonclient, ou None s'il manque ou se lit mal."""
        path = os.path.join(moonclient_repo, CATALOG_RELATIVE_PATH)
        try:
            with open(path, encoding="utf-8") as f:
                raw = json.load(f)["commandes"]
        except (OSError, ValueError, KeyError):
            return None
        commands = []
        for entry in raw:
            # Une forme relevée peut traîner la fin d'une chaîne C (« \n" "… »).
            usage = entry.get("forme", "").split("\\n")[0].strip()
            help_text = entry.get("aide", "").split("\n")[0].strip()
            commands.append(Atcommand(entry["nom"], tuple(entry.get("alias", [])), usage, help_text,
                                      int(entry.get("niveau_min", 0))))
        return cls(commands)

    def lookup(self, word):
        return self._by_word.get(word.lower())

    def matches(self, prefix):
        prefix = prefix.lower()
        return [w for w in self.words if w.startswith(prefix)]

    def hint(self, text):
        """La ligne d'aide de la commande en cours de frappe, ou "" sans rien à dire."""
        body = text.lstrip()
        # Une ligne du protocole brut, ou un chemin de script collé (la commande
        # libre en fait un reloadnpcfile) : rien à dire.
        if not body or body.upper().startswith(RAW_PREFIXES) or body.lower().endswith(SCRIPT_SUFFIXES):
            return ""
        word = body.split(" ", 1)[0].lstrip(COMMAND_SYMBOLS)
        if not word:
            return ""
        command = self.lookup(word)
        if command is not None:
            aliases = f"  (alias : {', '.join(command.aliases)})" if command.aliases else ""
            usage = f" {command.usage}" if command.usage else ""
            return f"{command.name}{usage} — {command.help or 'sans aide'}  [GM {command.level}]{aliases}"
        if " " in body:
            return f"« {word} » : commande inconnue du catalogue"
        found = self.matches(word)
        if not found:
            return f"« {word} » : aucune commande ne commence ainsi"
        more = f", … (+{len(found) - SHOWN_MATCHES})" if len(found) > SHOWN_MATCHES else ""
        return "Tab : " + ", ".join(found[:SHOWN_MATCHES]) + more


def clean_char_name(text):
    """Un nom de personnage tiré d'une sélection, ou None s'il n'en a pas la
    forme : ponctuation autour retirée, une seule ligne, 23 caractères au plus."""
    name = (text or "").strip().strip(NAME_DELIMITERS).strip()
    if not name or "\n" in name or "\t" in name or len(name) > MAX_CHAR_NAME_LENGTH:
        return None
    return name
