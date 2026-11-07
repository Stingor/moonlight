#!/usr/bin/env python3
"""
Voix de Sting-Bot — synthèse XTTS-v2 locale.

Deux rôles dans le même processus :

- le LOCUTEUR : il lit les répliques que le PNJ dépose dans `chatbot_voice`
  (texte et position du PNJ), les synthétise, les encode en Opus 20 ms et les
  pousse au voice-server par son pont UDP (`bot_voice`). Le voice-server tient
  le bot pour une session sans connexion : on l'entend à proximité, et un
  joueur le coupe comme il coupe un joueur.
- un serveur HTTP local, pour essayer une voix :
    POST /tts   {"text": "..."}  → PCM s16le, 48 kHz, mono
    GET  /health                 → {"ok": true, "voice": "..."}

La voix vient d'un extrait WAV (STING_VOICE_SAMPLE, 10 à 20 s d'un seul
locuteur) ou, à défaut, d'un locuteur livré avec XTTS (STING_VOICE_SPEAKER).
Son jeu se règle par STING_VOICE_SPEED, _TEMPERATURE, _TOP_P, _TOP_K,
_REPETITION_PENALTY et _LENGTH_PENALTY ; le rattrapage des répliques qui
débordent, par STING_VOICE_OVERRUN_RATIO et _MAX_ATTEMPTS (voir SYNTHESIS).
Le modèle est sous licence Coqui Public Model License : usage non commercial.
Son premier téléchargement demande COQUI_TOS_AGREED=1.

La base vient de groq.env (DB_HOST, DB_USER, DB_PASSWORD, DB_NAME) ; l'adresse
et le secret du pont, de conf/voice_athena.conf. Pour un voice-server sur une
autre machine : STING_VOICE_BRIDGE_HOST, STING_VOICE_BRIDGE_PORT (son
voice_bot_port) et STING_VOICE_BRIDGE_SECRET, dans l'environnement ou dans
tools/sting_voice.env (même forme que groq.env, et prioritaire sur lui).

Install: environnement Python 3.12 à part, PyTorch ROCm (ou CUDA) puis
         pip install coqui-tts soundfile soxr pymysql
Run:     python tools/sting_voice_service.py
Essai:   python tools/sting_voice_service.py --say "Salut les tocards." --out essai.wav
"""

import argparse
import base64
import io
import json
import os
import socket
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import numpy as np
import soundfile
import soxr
import torch
import TTS.tts.models.xtts as xtts_module
from TTS.tts.configs.xtts_config import XttsConfig
from TTS.tts.models.xtts import Xtts
from TTS.utils.manage import ModelManager

# Redirigée, la sortie de Windows est en cp1252 : un caractère qu'il n'a pas
# (une flèche, un emoji d'une réplique) lèverait une exception à l'affichage.
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.stderr.reconfigure(encoding="utf-8", errors="replace")

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
VOICE_CONF_PATHS = [
    os.path.join(TOOLS_DIR, "..", "conf", "voice_athena.conf"),
    os.path.join(TOOLS_DIR, "..", "conf", "import", "voice_conf.txt"),
]

# L'environnement l'emporte sur sting_voice.env, qui l'emporte sur groq.env :
# la voix peut viser une autre base et un autre voice-server que le bot.
for _env_name in ("sting_voice.env", "groq.env"):
    _env_file = os.path.join(TOOLS_DIR, _env_name)
    if not os.path.exists(_env_file):
        continue
    with open(_env_file, encoding="utf-8") as _f:
        for _line in _f:
            _line = _line.strip()
            if _line and not _line.startswith("#") and "=" in _line:
                _k, _v = _line.split("=", 1)
                os.environ.setdefault(_k.strip(), _v.strip())

XTTS_MODEL_NAME = "tts_models/multilingual/multi-dataset/xtts_v2"
XTTS_SAMPLE_RATE_HZ = 24_000
VOICE_SERVER_SAMPLE_RATE_HZ = 48_000
OPUS_FRAME_MS = 20
LANGUAGE = "fr"
# Au-delà, le texte vient d'ailleurs que de chatbot_queue.response (500).
MAX_TEXT_CHARS = 600
# XTTS tronque l'audio au-delà de 273 caractères en français : une réplique plus
# longue se dit en plusieurs passes.
SPEECH_CHUNK_MAX_CHARS = 250
# Les deux premiers paquets d'un flux Ogg Opus sont ses en-têtes.
OGG_OPUS_HEADER_PACKETS = 2

HOST = os.environ.get("STING_VOICE_HOST", "127.0.0.1")
PORT = int(os.environ.get("STING_VOICE_PORT", "5055"))
VOICE_SAMPLE = os.environ.get("STING_VOICE_SAMPLE", "")
VOICE_SPEAKER = os.environ.get("STING_VOICE_SPEAKER", "Damien Black")

# L'identité du bot au voice-server : hors des plages d'identifiants de
# rAthena (comptes dès 2000000, personnages dès 150000, jamais si haut). Le
# nom est celui du PNJ, pour que le client pose l'icône sur lui.
BOT_ID = int(os.environ.get("STING_VOICE_BOT_ID", "1999999999"))
BOT_NAME = os.environ.get("STING_VOICE_BOT_NAME", "Sting-Bot")
POLL_INTERVAL_S = 0.3
# Une réplique plus vieille ne se dit plus : la conversation est passée.
LINE_STALE_S = 30
LINES_PER_POLL = 5
# XTTS ne s'arrête pas toujours en fin de phrase : il enchaîne souffles et
# bribes, parfois des secondes durant. Une réplique saine dure de 0,6 à 1,4 fois
# 70 ms par caractère ; au-delà du plafond, elle se refait, et la plus courte
# des tentatives l'emporte (mesuré sur la voix clonée : ~1 génération sur 5
# déborde, quelle que soit la température ou la ponctuation finale).
MS_PER_CHAR = 70
OVERRUN_RATIO = float(os.environ.get("STING_VOICE_OVERRUN_RATIO", "1.6"))
MAX_ATTEMPTS = int(os.environ.get("STING_VOICE_MAX_ATTEMPTS", "3"))

# Le jeu de XTTS, aux valeurs du modèle par défaut. SPEED règle le débit (1.2
# plus vite, 0.85 plus posé) ; TEMPERATURE, TOP_P et TOP_K la variété de
# l'intonation, au prix de la stabilité ; REPETITION_PENALTY retient le
# bégaiement ; LENGTH_PENALTY pousse vers des générations courtes ou longues.
SYNTHESIS = {
    "speed": float(os.environ.get("STING_VOICE_SPEED", "1.0")),
    "temperature": float(os.environ.get("STING_VOICE_TEMPERATURE", "0.75")),
    "top_p": float(os.environ.get("STING_VOICE_TOP_P", "0.85")),
    "top_k": int(os.environ.get("STING_VOICE_TOP_K", "50")),
    "repetition_penalty": float(os.environ.get("STING_VOICE_REPETITION_PENALTY", "10.0")),
    "length_penalty": float(os.environ.get("STING_VOICE_LENGTH_PENALTY", "1.0")),
}

# Sous ROCm, les convolutions MIOpen du décodeur HiFi-GAN coûtent cent fois
# celles de PyTorch (10 s contre 0,1 s pour 7 s d'audio, RX 7900 XT).
torch.backends.cudnn.enabled = False


def load_sample(audio_path, sampling_rate):
    """L'extrait de clonage, en mono [1, N] dans [-1, 1], à la fréquence voulue.

    XTTS le lit par torchaudio, qui exige FFmpeg ; soundfile lit WAV, FLAC et MP3
    sans lui.
    """
    audio, rate = soundfile.read(audio_path, dtype="float32", always_2d=True)
    mono = audio.mean(axis=1)
    if rate != sampling_rate:
        mono = soxr.resample(mono, rate, sampling_rate)
    return torch.from_numpy(np.clip(mono, -1.0, 1.0)).unsqueeze(0)


xtts_module.load_audio = load_sample


class StingVoice:
    def __init__(self):
        _, config_path, _ = ModelManager().download_model(XTTS_MODEL_NAME)
        model_dir = os.path.dirname(config_path)
        config = XttsConfig()
        config.load_json(config_path)
        self.model = Xtts.init_from_config(config)
        self.model.load_checkpoint(config, checkpoint_dir=model_dir, use_deepspeed=False)
        self.device = "cuda" if torch.cuda.is_available() else "cpu"
        self.model.to(self.device)
        # Un seul GPU : une synthèse à la fois.
        self.lock = threading.Lock()
        if VOICE_SAMPLE:
            self.voice_name = os.path.basename(VOICE_SAMPLE)
            self.gpt_cond_latent, self.speaker_embedding = self.model.get_conditioning_latents(
                audio_path=[VOICE_SAMPLE]
            )
        else:
            self.voice_name = VOICE_SPEAKER
            speaker = self.model.speaker_manager.speakers[VOICE_SPEAKER]
            self.gpt_cond_latent = speaker["gpt_cond_latent"]
            self.speaker_embedding = speaker["speaker_embedding"]

    def synthesize_float(self, text: str) -> np.ndarray:
        """Rend le texte en flottants 48 kHz mono."""
        expected_samples = (
            len(text) * MS_PER_CHAR / 1000 / SYNTHESIS["speed"] * XTTS_SAMPLE_RATE_HZ
        )
        best = None
        with self.lock, torch.inference_mode():
            for _ in range(MAX_ATTEMPTS):
                out = self.model.inference(
                    text, LANGUAGE, self.gpt_cond_latent, self.speaker_embedding, **SYNTHESIS
                )
                if best is None or len(out["wav"]) < len(best):
                    best = out["wav"]
                if len(best) <= expected_samples * OVERRUN_RATIO:
                    break
        wav_24k = np.asarray(best, dtype=np.float32)
        wav_48k = soxr.resample(wav_24k, XTTS_SAMPLE_RATE_HZ, VOICE_SERVER_SAMPLE_RATE_HZ)
        return np.clip(wav_48k, -1.0, 1.0)

    def synthesize(self, text: str) -> bytes:
        """Rend le texte en PCM s16le 48 kHz mono."""
        return (self.synthesize_float(text) * 32767.0).astype("<i2").tobytes()


def opus_frames(wav_48k: np.ndarray) -> list[bytes]:
    """Encode en Opus et rend les trames de 20 ms, sans leur conteneur Ogg."""
    buf = io.BytesIO()
    soundfile.write(buf, wav_48k, VOICE_SERVER_SAMPLE_RATE_HZ, format="OGG", subtype="OPUS")
    data = buf.getvalue()
    packets, partial, pos = [], b"", 0
    while pos < len(data):
        if data[pos:pos + 4] != b"OggS":
            raise ValueError("page Ogg attendue")
        segment_count = data[pos + 26]
        lacing = data[pos + 27:pos + 27 + segment_count]
        body = pos + 27 + segment_count
        for lace in lacing:
            partial += data[body:body + lace]
            body += lace
            # Un segment de moins de 255 octets clôt son paquet.
            if lace < 255:
                packets.append(partial)
                partial = b""
        pos = body
    return packets[OGG_OPUS_HEADER_PACKETS:]


def speech_chunks(line: str) -> list[str]:
    """Coupe une réplique en passes que XTTS dit sans les tronquer, en fin de phrase si possible."""
    chunks = []
    remaining = line.strip()
    while len(remaining) > SPEECH_CHUNK_MAX_CHARS:
        head = remaining[:SPEECH_CHUNK_MAX_CHARS]
        cut = len(head)
        for sep in (". ", "! ", "? ", ", ", " "):
            pos = head.rfind(sep, len(head) // 2)
            if pos > 0:
                cut = pos + len(sep)
                break
        chunks.append(remaining[:cut].rstrip())
        remaining = remaining[cut:].lstrip()
    if remaining:
        chunks.append(remaining)
    return chunks


def read_voice_conf() -> dict:
    """Les clés du voice-server, l'import après le fichier de base."""
    conf = {}
    for path in VOICE_CONF_PATHS:
        if not os.path.exists(path):
            continue
        with open(path, encoding="utf-8", errors="replace") as f:
            for line in f:
                line = line.split("//", 1)[0].strip()
                if ":" in line:
                    key, value = line.split(":", 1)
                    conf[key.strip()] = value.strip()
    return conf


class BotSpeaker:
    """Dit au voice-server, trame par trame, les répliques du PNJ."""

    def __init__(self, voice: StingVoice):
        self.voice = voice
        # Sur la même machine, le pont du map-server ; ailleurs, le port des
        # bots (voice_bot_port), que l'environnement désigne.
        conf = read_voice_conf()
        self.bridge = (
            os.environ.get("STING_VOICE_BRIDGE_HOST", conf.get("voice_api_ip", "127.0.0.1")),
            int(os.environ.get("STING_VOICE_BRIDGE_PORT", conf.get("voice_api_port", "7001"))),
        )
        self.secret = os.environ.get("STING_VOICE_BRIDGE_SECRET", conf.get("voice_bridge_secret", ""))
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.seq = 0
        # Ouverte à la première lecture : une base injoignable au démarrage se
        # réessaie comme une base perdue en route, sans arrêter le service.
        self.db = None

    def connect_db(self):
        import pymysql

        return pymysql.connect(
            host=os.environ.get("DB_HOST", "localhost"),
            user=os.environ.get("DB_USER", "ragnarok"),
            password=os.environ.get("DB_PASSWORD", ""),
            database=os.environ.get("DB_NAME", "ragnarok"),
            charset="utf8mb4",
            autocommit=True,
        )

    def take_lines(self) -> list[tuple]:
        if self.db is None:
            self.db = self.connect_db()
        self.db.ping(reconnect=True)
        with self.db.cursor() as cur:
            cur.execute(
                "SELECT id, line, map, x, y, TIMESTAMPDIFF(SECOND, created_at, NOW()) "
                "FROM chatbot_voice ORDER BY id LIMIT %s",
                (LINES_PER_POLL,),
            )
            rows = cur.fetchall()
            if rows:
                cur.execute("DELETE FROM chatbot_voice WHERE id <= %s", (rows[-1][0],))
        return rows

    def say(self, line: str, map_name: str, x: int, y: int):
        started = time.perf_counter()
        frames = opus_frames(self.voice.synthesize_float(line))
        synth_ms = (time.perf_counter() - started) * 1000
        print(
            f"[voix] {BOT_NAME} sur {map_name} ({x},{y}) : {len(frames)} trames, "
            f"synthèse {synth_ms:.0f} ms — {line[:60]}",
            flush=True,
        )
        # Au rythme de la parole : le voice-server limite le débit d'un locuteur.
        start = time.perf_counter()
        for index, frame in enumerate(frames):
            delay = start + index * OPUS_FRAME_MS / 1000 - time.perf_counter()
            if delay > 0:
                time.sleep(delay)
            packet = {
                "type": "bot_voice",
                "bot_id": BOT_ID,
                "name": BOT_NAME,
                "map": map_name,
                "x": x,
                "y": y,
                "seq": self.seq,
                "opus": base64.b64encode(frame).decode("ascii"),
            }
            if self.secret:
                packet["bridge_secret"] = self.secret
            self.seq = (self.seq + 1) & 0xFFFF
            self.sock.sendto(json.dumps(packet).encode(), self.bridge)

    def run(self):
        print(f"[voix] locuteur {BOT_NAME} → pont {self.bridge[0]}:{self.bridge[1]}", flush=True)
        while True:
            try:
                rows = self.take_lines()
            except Exception as exc:
                print(f"[voix] base injoignable : {exc}", flush=True)
                time.sleep(5)
                continue
            for _, line, map_name, x, y, age_s in rows:
                line = (line or "").strip()
                if not line or not map_name or age_s > LINE_STALE_S:
                    continue
                try:
                    for chunk in speech_chunks(line):
                        self.say(chunk, map_name, int(x), int(y))
                except Exception as exc:
                    print(f"[voix] réplique perdue : {exc}", flush=True)
            if not rows:
                time.sleep(POLL_INTERVAL_S)


def serve(voice: StingVoice):
    class Handler(BaseHTTPRequestHandler):
        def _reply(self, status: int, content_type: str, body: bytes):
            self.send_response(status)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def do_GET(self):
            if self.path != "/health":
                return self._reply(404, "text/plain", b"not found")
            body = json.dumps({"ok": True, "voice": voice.voice_name}).encode()
            self._reply(200, "application/json", body)

        def do_POST(self):
            if self.path != "/tts":
                return self._reply(404, "text/plain", b"not found")
            try:
                length = int(self.headers.get("Content-Length", "0"))
                text = str(json.loads(self.rfile.read(length))["text"]).strip()
            except (ValueError, KeyError, TypeError):
                return self._reply(400, "text/plain", b'attendu : {"text": "..."}')
            if not text or len(text) > MAX_TEXT_CHARS:
                return self._reply(400, "text/plain", b"texte vide ou trop long")
            started = time.perf_counter()
            pcm = voice.synthesize(text)
            elapsed_ms = (time.perf_counter() - started) * 1000
            audio_ms = len(pcm) / 2 / VOICE_SERVER_SAMPLE_RATE_HZ * 1000
            print(
                f"[voix] {len(text)} car. → {audio_ms:.0f} ms d'audio en {elapsed_ms:.0f} ms",
                flush=True,
            )
            self._reply(200, "audio/L16; rate=48000; channels=1", pcm)

        def log_message(self, format, *args):
            pass

    server = ThreadingHTTPServer((HOST, PORT), Handler)
    print(f"[voix] {voice.voice_name} sur {voice.device}, écoute http://{HOST}:{PORT}", flush=True)
    server.serve_forever()


def main():
    parser = argparse.ArgumentParser(description="Voix de Sting-Bot (XTTS-v2)")
    parser.add_argument("--say", help="synthétise ce texte une fois, sans serveur")
    parser.add_argument("--out", default="sting_voice.wav", help="fichier WAV de --say")
    parser.add_argument("--no-speaker", action="store_true", help="HTTP seul, sans chatbot_voice")
    args = parser.parse_args()

    started = time.perf_counter()
    voice = StingVoice()
    print(f"[voix] modèle chargé en {time.perf_counter() - started:.1f} s sur {voice.device}")
    settings = " ".join(f"{name}={value}" for name, value in SYNTHESIS.items())
    print(f"[voix] {settings} débordement×{OVERRUN_RATIO} tentatives={MAX_ATTEMPTS}", flush=True)
    if not args.say:
        if not args.no_speaker:
            threading.Thread(target=BotSpeaker(voice).run, daemon=True).start()
        return serve(voice)

    # Le premier passage compile les noyaux GPU : on mesure le second.
    voice.synthesize("Échauffement.")
    started = time.perf_counter()
    pcm = voice.synthesize(args.say)
    elapsed_ms = (time.perf_counter() - started) * 1000
    audio_ms = len(pcm) / 2 / VOICE_SERVER_SAMPLE_RATE_HZ * 1000
    soundfile.write(
        args.out, np.frombuffer(pcm, dtype="<i2"), VOICE_SERVER_SAMPLE_RATE_HZ, subtype="PCM_16"
    )
    print(f"[voix] {audio_ms:.0f} ms d'audio en {elapsed_ms:.0f} ms → {args.out}")


if __name__ == "__main__":
    sys.exit(main())
