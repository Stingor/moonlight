"""Shared helpers for the admin channel tools (admin_gui.py, admin_client.py).

The admin secret is never hard-coded: it comes, by decreasing priority, from
  1. the --secret command-line option (handled by the callers),
  2. the MOONLIGHT_ADMIN_SECRET environment variable,
  3. ADMIN_SECRET=... in admin.env, next to the script (or next to
     MoonlightAdmin.exe once frozen by PyInstaller).
admin.env is git-ignored; admin.env.example documents its format.
"""
import os
import sys

ENV_FILENAME = "admin.env"
ENV_VAR = "MOONLIGHT_ADMIN_SECRET"


def app_dir():
    """Directory holding the tool: the .exe's folder once frozen, else this file's."""
    if getattr(sys, "frozen", False):
        return os.path.dirname(os.path.abspath(sys.executable))
    return os.path.dirname(os.path.abspath(__file__))


def env_path():
    return os.path.join(app_dir(), ENV_FILENAME)


def read_env_file(path=None):
    """Parse KEY=VALUE lines (# comments, optional quotes) into a dict."""
    values = {}
    path = path or env_path()
    if not os.path.exists(path):
        return values
    with open(path, encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, _, value = line.partition("=")
            value = value.strip()
            if len(value) >= 2 and value[0] == value[-1] and value[0] in "\"'":
                value = value[1:-1]
            values[key.strip()] = value
    return values


def default_secret():
    """Secret from the environment or admin.env; "" when none is configured."""
    return os.environ.get(ENV_VAR) or read_env_file().get("ADMIN_SECRET", "")
