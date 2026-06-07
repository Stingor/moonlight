#!/usr/bin/env python3
"""Build a standalone Windows executable of admin_gui.py with PyInstaller.

Prerequisite (build machine only -- the resulting .exe needs nothing):
    <python.org python> -m pip install pyinstaller

Use the official python.org CPython, NOT the MSYS2/MinGW python: the latter is
"externally managed" (PEP 668) and produces non-portable executables. This
script auto-detects a suitable Python and re-executes itself with it if the
interpreter it was launched with lacks PyInstaller.

Usage:
    python tools/build_admin_exe.py [--if-newer]

Output:
    tools/dist/MoonlightAdmin.exe   (single file, no console window)

Drop an "admin.ico" next to this script to give the exe a custom icon.
"""
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "admin_gui.py")
ICON = os.path.join(HERE, "admin.ico")  # optional


def _is_up_to_date():
    """True if the built exe exists and is newer than the source script."""
    exe = os.path.join(HERE, "dist", "MoonlightAdmin.exe")
    if not os.path.exists(exe):
        return False
    return os.path.getmtime(exe) >= os.path.getmtime(SCRIPT)


def _same(a, b):
    return os.path.normcase(os.path.abspath(a)) == os.path.normcase(os.path.abspath(b))


def _has_pyinstaller(python):
    return subprocess.call([python, "-c", "import PyInstaller"],
                           stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL) == 0


def _candidate_pythons():
    """Likely python.org interpreters, best-first, that may host PyInstaller."""
    cands = [sys.executable]
    for name in ("python", "python3"):
        found = shutil.which(name)
        if found:
            cands.append(found)
    # python.org per-user installs: %LOCALAPPDATA%\Programs\Python\PythonXY\python.exe
    base = os.path.join(os.environ.get("LOCALAPPDATA", ""), "Programs", "Python")
    if os.path.isdir(base):
        for d in sorted(os.listdir(base), reverse=True):
            exe = os.path.join(base, d, "python.exe")
            if os.path.exists(exe):
                cands.append(exe)
    # de-duplicate, preserving order
    seen, out = set(), []
    for c in cands:
        key = os.path.normcase(os.path.abspath(c))
        if key not in seen:
            seen.add(key)
            out.append(c)
    return out


def _run_pyinstaller():
    args = [
        sys.executable, "-m", "PyInstaller",
        "--onefile",      # bundle everything into a single .exe
        "--windowed",     # GUI app: no console window
        "--name", "MoonlightAdmin",
        "--distpath", os.path.join(HERE, "dist"),
        "--workpath", os.path.join(HERE, "build"),
        "--specpath", HERE,
        "--noconfirm",
    ]
    if os.path.exists(ICON):
        args += ["--icon", ICON]
    args.append(SCRIPT)

    print("Building:", " ".join(args))
    rc = subprocess.call(args)
    if rc == 0:
        print("\nDone -> %s" % os.path.join(HERE, "dist", "MoonlightAdmin.exe"))
    sys.exit(rc)


def main():
    if "--if-newer" in sys.argv and _is_up_to_date():
        print("MoonlightAdmin.exe is up to date, skipping.")
        return

    # Build with a Python that actually has PyInstaller. If the interpreter we
    # were launched with (e.g. MSYS2's python, first on PATH) doesn't, re-exec
    # into a detected python.org Python that does.
    if not _has_pyinstaller(sys.executable):
        for py in _candidate_pythons():
            if _same(py, sys.executable):
                continue
            if _has_pyinstaller(py):
                print("Building with", py)
                sys.exit(subprocess.call([py, os.path.abspath(__file__)] + sys.argv[1:]))

        if "--if-newer" in sys.argv:
            print("PyInstaller not installed in any detected Python, skipping exe build.")
            return  # don't break the server build because the tool is missing
        sys.exit("PyInstaller not found. Run:  <python.org python> -m pip install pyinstaller")

    _run_pyinstaller()


if __name__ == "__main__":
    main()
