#!/usr/bin/env python3
"""Check assets/rez/ and assets/game/ against the retail Update 2.

  build/assets.py check <retaildir>

assets/ holds Update 2's data, which the release ships (assets/README.md):
assets/rez/ is what the update's archives, Gamep3.rez and Gamep4.rez, hold
besides the game DLLs, and assets/game/ the update's loose files that the
game directory has. <retaildir> is the retail game as build/discs.py
installs it (out/retail/tron2-usa-1.030: the discs with Update 2 over
them). The check fails unless

  * each file of assets/rez/ is, byte for byte, the entry of the same path
    in the update's archives (in the later one, if both have it, as the
    rez stack gives it);
  * every entry of those archives that is not a PE file (the game DLLs,
    which we build) is in assets/rez/;
  * each file of assets/game/ is the retail file of the same name, and one
    that build/retail-files.txt says the update installs.

build/all.sh runs it before it packs tron20src.rez (step 1b), when it has
the retail game; `make art-check` runs it too.
"""
import os
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(REPO, "build"))
sys.path.insert(0, os.path.join(REPO, "ext", "tools"))
import discs  # noqa: E402
from rezextract import Rez  # noqa: E402

# The update's archives, in their order in the rez stack (the later wins)
UPDATE_REZ = ["Gamep3.rez", "Gamep4.rez"]
ASSETS_REZ = os.path.join(REPO, "assets", "rez")
ASSETS_GAME = os.path.join(REPO, "assets", "game")


def files_of(top):
    out = {}
    for root, _dirs, names in os.walk(top):
        for fn in names:
            full = os.path.join(root, fn)
            out[os.path.relpath(full, top).replace(os.sep, "/")] = full
    return out


def check(retaildir):
    bad = []
    entries = {}                       # lower-case path -> (archive, bytes)
    for name in UPDATE_REZ:
        path = os.path.join(retaildir, name)
        if not os.path.isfile(path):
            sys.exit(f"assets: {path} is missing: the retail game with Update 2 is not installed (make retail)")
        rez = Rez(path)
        for ename, pos, size in list(rez.walk()):
            rez.f.seek(pos)
            entries[ename.lower()] = (name, ename, rez.f.read(size))
        rez.f.close()
    have = files_of(ASSETS_REZ)
    for rel, full in sorted(have.items()):
        e = entries.get(rel.lower())
        if e is None:
            bad.append(f"assets/rez/{rel}: not in the update's archives")
        elif open(full, "rb").read() != e[2]:
            bad.append(f"assets/rez/{rel}: not the bytes of {e[0]}:{e[1]}")
    wanted = {k for k, (_a, _n, b) in entries.items() if b[:2] != b"MZ"}
    for k in sorted(wanted - {r.lower() for r in have}):
        a, n, _b = entries[k]
        bad.append(f"{a}:{n}: not in assets/rez/")

    _d, _u, _c, retail_files = discs.load()
    update_files = {f.path.lower(): f for f in retail_files if f.source == "update"}
    game = files_of(ASSETS_GAME)
    for rel, full in sorted(game.items()):
        f = update_files.get(rel.lower())
        if f is None:
            bad.append(f"assets/game/{rel}: not a file that the update installs (build/retail-files.txt)")
            continue
        theirs = os.path.join(retaildir, f.path)
        if not os.path.isfile(theirs) or open(full, "rb").read() != open(theirs, "rb").read():
            bad.append(f"assets/game/{rel}: not the bytes of the retail {f.path}")
    if bad:
        for b in bad:
            print(f"assets: {b}", file=sys.stderr)
        sys.exit("assets: assets/ is not Update 2's data (assets/README.md)")
    print(f"== assets/: {len(have)} files of assets/rez/, each the update's archive entry, and "
          f"{len(game)} of assets/game/, each the update's file, unchanged")


if __name__ == "__main__":
    a = sys.argv[1:]
    if len(a) == 2 and a[0] == "check":
        check(a[1])
    else:
        sys.exit(__doc__)
