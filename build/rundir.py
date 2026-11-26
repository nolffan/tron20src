#!/usr/bin/env python3
"""Fill a test's run directory from a game directory, so that the game
never writes into the game directory.

  build/rundir.py <game directory> <run directory> [<name>...]

The tests (build/playtest.sh, build/mptest.sh and the tests that use them)
run each program in a run directory of its own. This puts in it:

  * a copy of each binary (a file that starts with "MZ": the engine, the
    game DLLs, the launcher, the server), so that the run loads its own;
  * a symbolic link (relative: it resolves in the build container too) to
    each other file;
  * real directories: a directory of the game directory is never linked.
    If it were, the files that the game writes into it (for example
    LightCycleGameData/Default.txt) would go into the game directory.

The files are the ones that the game directory's MANIFEST.txt names (with
MANIFEST.txt itself and the .pdb and .map beside each built binary): no
file that the game, the server, the launcher or a test wrote there at run
time. A file that the game writes again (as the ban list bl.dat of the
dedicated server) would go through its link into the game directory. A
game directory without MANIFEST.txt (the control, out/tron20-control:
build/control.sh) gives all its files, but the run-time files of RUNTIME.

<name>...: the top-level names to leave out (the caller writes them: the
settings, the command file).
"""
import os
import shutil
import sys

# What the game, the dedicated server, the launcher and the tests write in
# the game directory at run time (the first part of the path): never taken
# into a run directory.
RUNTIME_DIRS = {"save", "profiles", "serverdata", "lightcyclegamedata"}
RUNTIME_FILES = {"autoexec.cfg", "tron20cmds.txt", "tron20.ini", "display.cfg",
                 "launchcmds.txt", "run-cmds.txt", "bl.dat"}


def manifest_files(game):
    man = os.path.join(game, "MANIFEST.txt")
    files = {"MANIFEST.txt"}
    for line in open(man, encoding="utf-8"):
        line = line.rstrip("\n")
        if line.startswith("#") or "\t" not in line:
            continue
        frm, rel = line.split("\t", 1)
        files.add(rel)
        if frm.startswith(("built ", "EXT ")):
            stem = os.path.splitext(rel)[0]
            files.update({stem + ".pdb", stem + ".map"})
    return {f for f in files if os.path.isfile(os.path.join(game, f))}


def all_files(game):
    files = set()
    for top, dirs, names in os.walk(game, followlinks=True):
        rel = os.path.relpath(top, game)
        if rel == ".":
            dirs[:] = [d for d in dirs if d.lower() not in RUNTIME_DIRS]
            names = [n for n in names if n.lower() not in RUNTIME_FILES]
            rel = ""
        for n in names:
            if n.lower().endswith(".log"):
                continue
            files.add(os.path.join(rel, n) if rel else n)
    return files


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    game, run = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
    skip = {n.lower() for n in sys.argv[3:]}
    if os.path.isfile(os.path.join(game, "MANIFEST.txt")):
        files = manifest_files(game)
    else:
        files = all_files(game)
    os.makedirs(run, exist_ok=True)
    for rel in sorted(files):
        if rel.replace("\\", "/").split("/", 1)[0].lower() in skip:
            continue
        src = os.path.join(game, rel)
        dst = os.path.join(run, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        if os.path.lexists(dst):
            os.remove(dst)
        with open(src, "rb") as fh:
            mz = fh.read(2) == b"MZ"
        if mz:
            shutil.copy(src, dst)
        else:
            os.symlink(os.path.relpath(src, os.path.dirname(dst)), dst)


if __name__ == "__main__":
    main()
