"""Console interactive de la fenêtre « Serveur (SSH) » : ssh avec un terminal.

`ssh -tt` donne à la commande distante un vrai terminal : elle peut poser
une question ([o/N], `read`, mot de passe) et attendre la réponse, que la
fenêtre envoie sur l'entrée de ssh. Ctrl+C part comme le caractère ETX, que
le terminal distant change en interruption ; fermer ssh y envoie un
raccrochage, qui arrête la commande au lieu de la laisser tourner sans sortie.

Avec un terminal, la sortie n'arrive plus ligne à ligne : une invite ne finit
pas par un saut de ligne. Elle se lit donc par morceaux, et la console montre
la ligne en cours telle qu'elle est.

Pour qu'aucune commande ne s'arrête sur un pager (git log, systemctl) ou un
éditeur (rebase --continue, fusion), ni n'écrive de couleurs, la ligne
distante part après REMOTE_ENV. GIT_EDITOR=true garde le message que git
propose, sans ouvrir vi dans un terminal que personne ne voit.
"""
import os
import re
import subprocess
import sys
import threading

from tables_panel import NO_WINDOW

READ_CHUNK_BYTES = 4096
# COLUMNS : le diffstat de git ne raccourcit pas un chemin en « .../ ».
REMOTE_COLUMNS = 200
REMOTE_ENV = f"export PAGER=cat GIT_PAGER=cat SYSTEMD_PAGER=cat GIT_EDITOR=true TERM=dumb COLUMNS={REMOTE_COLUMNS}; "
INTERRUPT = "\x03"
# Séquences d'échappement d'un terminal : couleurs, effacements, titres.
ANSI_RE = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]|\x1b\][^\x07]*\x07|\x1b[()][A-Za-z0-9]|\x1b[=>]")
# Une suite UTF-8 coupée en fin de morceau : au plus trois octets en attente.
MAX_UTF8_TAIL = 3
UNSIGNED_RANGE = 2 ** 32
SIGNED_LIMIT = 2 ** 31


class ChunkDecoder:
    """Des octets en texte, morceau par morceau : UTF-8 (git, python, bash),
    à défaut Windows-1252, celui des scripts qu'un `cat` montre. Une suite
    UTF-8 coupée entre deux morceaux attend le suivant."""

    def __init__(self):
        self._tail = b""

    def feed(self, data):
        data = self._tail + data
        self._tail = b""
        try:
            return data.decode("utf-8")
        except UnicodeDecodeError as exc:
            if exc.start >= len(data) - MAX_UTF8_TAIL and exc.reason == "unexpected end of data":
                self._tail = data[exc.start:]
                return data[:exc.start].decode("utf-8", "replace")
            return data.decode("cp1252", "replace")

    def flush(self):
        rest, self._tail = self._tail, b""
        return rest.decode("cp1252", "replace")


def visible(line):
    """Ce qu'un terminal montrerait d'une ligne : sans échappements, et après
    le dernier retour chariot (une barre de progression s'y réécrit)."""
    line = ANSI_RE.sub("", line)
    segments = [s for s in line.split("\r") if s]
    return segments[-1] if segments else ""


class InteractiveRunner:
    """ssh avec terminal : ("chunk", texte) au fil de la sortie, puis
    ("exit", code) ; `send` écrit sur l'entrée de la commande distante."""

    def __init__(self, events):
        self.events = events
        self.proc = None

    @property
    def busy(self):
        return self.proc is not None and self.proc.poll() is None

    def start(self, argv):
        self.proc = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT, bufsize=0, creationflags=NO_WINDOW)
        proc = self.proc

        def pump():
            decoder = ChunkDecoder()
            fd = proc.stdout.fileno()
            while True:
                data = os.read(fd, READ_CHUNK_BYTES)
                if not data:
                    break
                self.events.put(("chunk", decoder.feed(data)))
            rest = decoder.flush()
            if rest:
                self.events.put(("chunk", rest))
            code = proc.wait()
            # Windows rend un code non signé : -1 (ssh interrompu) y vaut 2³² - 1.
            if code >= SIGNED_LIMIT:
                code -= UNSIGNED_RANGE
            self.events.put(("exit", code))

        threading.Thread(target=pump, daemon=True).start()

    def send(self, text):
        """Écrit `text` à la commande distante ; False si elle est finie."""
        if not self.busy:
            return False
        try:
            self.proc.stdin.write(text.encode("utf-8"))
            self.proc.stdin.flush()
        except OSError:
            return False
        return True

    def interrupt(self):
        return self.send(INTERRUPT)

    def stop(self):
        """Tue ssh.exe : le terminal distant raccroche et la commande s'arrête."""
        if not self.busy:
            return
        if sys.platform == "win32":
            subprocess.call(["taskkill", "/T", "/F", "/PID", str(self.proc.pid)],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, creationflags=NO_WINDOW)
        else:
            self.proc.terminate()
