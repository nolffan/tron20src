#!/usr/bin/env python3
"""The install scripts of a game directory's public archive.

  build/mkinstall.py <gamedir> <outdir>
  build/mkinstall.py ranges <retaildir> <gamedir>

A game directory holds the retail game's data as well as what we built: the
discs' .rez archives (GameDLL.rez with the retail game DLLs inside it
zeroed and their names changed: build/all.sh step 1, build/rezhide.py),
the movies, the manual, the shipped autoexec.cfg and default profile, and,
as third-party runtime, RAD's binkw32.dll (legacy builds only) and the
Microsoft C, C++ and MFC runtime (MANIFEST.txt's `data` and `THIRD-PARTY`
rows of the retail release). The public archive (build/package.sh) leaves
all of that out, and three scripts put it back from the player's own
images of the two discs of Tron 2.0 (USA). They need no update: Update 2's
data (tron20src.rez and the update's readme, MANIFEST.txt's `asset` rows:
assets/README.md) is in the archive, and its binaries are built here.

  linux-install.sh       bash, coreutils and python3 (its zlib inflates)
  windows-install.bat    cmd running the PowerShell it carries (5.1, as every
                         Windows 10 and 11 has; .NET's DeflateStream inflates)
  mac-install.command    the Perl every Mac has, with its own modules (its
                         Compress::Raw::Zlib inflates), for CrossOver or
                         another Wine

A disc image can be a plain .iso, or a raw .bin (2352-byte sectors, with
its .cue) or a .zip that holds one: the scripts convert it to an .iso
first. Nothing is installed or run from the discs, and
neither script reads ISO 9660 or an InstallShield header. An image is
recognised by its size and SHA-256, and then every byte that the game
needs is known. This script writes that as a list:

  D <id> <size> <sha256> <name>    a disc image (the .iso)
  R <id> <size>                    a disc's raw .bin (2352-byte sectors)
  S <name> <id>...                 the set of images the game installs from
  V <n> <size> <sha256> <name>     a retail file (as installed) ...
  W copy|is <id>:<offset>:<length>...
                                   ... and how to read it: those byte
                                   ranges, joined, are the file (copy), or
                                   the InstallShield 6 cabinet data it is
                                   stored as (is: a run of chunks, each a
                                   16-bit length and a raw deflate stream).
                                   A file split across cabinets has one
                                   range in each.
  F <size> <sha256> <flag> <n> <path>
                                   a file of the game directory, made from
                                   retail file <n> by, in order:
  C <length>                       its next bytes, as they are
  Z <length>                       zeros in place of its next bytes (the
                                   retail game DLLs blanked inside the .rez
                                   archives)
  L <base64>                       bytes of ours in place of its next ones
                                   (the changed names of those entries)
  E                                the end of the file: the script checks
                                   its size and SHA-256 before it moves it
                                   into place

A name (the last field of D, V and F) can contain spaces. A file's flag
is `-`, or `once` for autoexec.cfg and Profiles/Default.prf (the shipped
settings, which the game rewrites: written only if there are none). So the
result is the game directory as built, byte for byte. The list goes at the
end of each script, after a marker line. The scripts' code is in
build/game-scripts/ (the .bat's header is written here).

The bytes come from three committed lists, which hold no game data:
build/retail-files.txt (each retail file's size and SHA-256, and the
discs), build/retail-ways.txt (where each file's bytes are in the disc
images: build/retailways.py) and build/retail-ranges.txt (what the build
changes in the three archives, and each archive's SHA-256 after that). A
game directory built with the retail game is checked against them: each
file that the scripts write must be in it as they write it. One built
without it (NODISCS=1: it has the file NOT-PLAYABLE.txt) has no game data to
check, and gives the same scripts.

Writes <outdir>/linux-install.sh, <outdir>/windows-install.bat,
<outdir>/mac-install.command,
<outdir>/disc-files.txt (the game directory's paths that the scripts write:
the archive leaves them out), <outdir>/disc-dirs.txt (the directories that
only the scripts fill: the archive leaves them out too),
<outdir>/disc-sums.txt (the SHA-256 of each of those files but the `once`
files, for the archive's SHA256SUMS) and <outdir>/MANIFEST.txt (the archive's:
the game directory's, with an `install` row for each file that the scripts
write, so that it is the same whether or not the build had the retail
game).

`ranges` writes build/retail-ranges.txt again: it compares each archive of
a game directory built with the retail game with the retail file.
"""
import base64
import hashlib
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(REPO, "build"))
import discs  # noqa: E402

SCRIPTS = os.path.join(REPO, "build", "game-scripts")
WAYS = os.path.join(REPO, "build", "retail-ways.txt")
RANGES = os.path.join(REPO, "build", "retail-ranges.txt")
RELEASE = "tron2-usa-1.030"            # build/retail.sh RETAIL_RELEASE
TAG = "TRON20"
MARKER = f"__{TAG}_INSTALL_DATA__"
CHUNK = 1 << 20
SECTOR = 2048
RAW_SECTOR = 2352
LITERAL_LINE = 3 * 1024          # raw bytes per L record (4 KB of base64)
LITERAL_MAX = 1 << 16            # more than this of ours means a wrong source
NODISCS_MARK = "NOT-PLAYABLE.txt"   # build/all.sh, TRON20SRC_NODISCS=1
# The settings the game rewrites: their shipped copies in the game
# directory, and the file each starts.
SETTINGS = {"autoexec.cfg.retail": "autoexec.cfg",
            "Default.prf.retail": "Profiles/Default.prf"}
# The ids of the images in the scripts' list
IDS = {"1": "disc1", "2": "disc2"}
RANGES_HEADER = """\
## What the build changes in the retail archives that it copies into a game
## directory (build/all.sh step 1): the retail game DLLs inside them
## zeroed (ext/tools/retailextract.py), and their names changed
## (build/rezhide.py). `build/mkinstall.py ranges` writes this list from a
## game directory built with the retail game; every package of one checks
## it. The install scripts of a public archive (build/mkinstall.py) use it
## in place of the retail game. It holds no game data: only names,
## offsets, sizes, SHA-256 values and the bytes that the build writes.
##
## ZERO <archive> <offset> <size>    zeros over the retail bytes there
## LIT <archive> <offset> <hex>      the build's bytes in place of them
## ZSUM <archive> <size> <SHA-256>   the archive after those changes
## The fields are separated by one TAB.
"""


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for b in iter(lambda: f.read(CHUNK), b""):
            h.update(b)
    return h.hexdigest()


def load_ways():
    ways = {}
    for line in open(WAYS, encoding="utf-8"):
        line = line.rstrip("\n")
        if not line or line.startswith("#"):
            continue
        f = line.split("\t")
        if f[0] == "W":
            ways[f[1]] = (f[2], [tuple(r.rsplit(":", 2)) for r in f[3].split(" ")])
        else:
            sys.exit(f"mkinstall: {WAYS}: a line that is not W: {line[:60]}")
    return ways


def load_ranges():
    """({archive: [(offset, kind, size, bytes)]}, {archive: (size, sha)})."""
    edits, zsum = {}, {}
    for line in open(RANGES, encoding="utf-8"):
        line = line.rstrip("\n")
        if not line or line.startswith("#"):
            continue
        f = line.split("\t")
        if f[0] == "ZERO":
            edits.setdefault(f[1], []).append((int(f[2]), "Z", int(f[3]), None))
        elif f[0] == "LIT":
            b = bytes.fromhex(f[3])
            edits.setdefault(f[1], []).append((int(f[2]), "L", len(b), b))
        elif f[0] == "ZSUM":
            zsum[f[1]] = (int(f[2]), f[3])
        else:
            sys.exit(f"mkinstall: {RANGES}: a line that is not ZERO, LIT or ZSUM: {line[:60]}")
    for a in edits:
        if a not in zsum:
            sys.exit(f"mkinstall: {RANGES}: no ZSUM of {a}")
    return edits, zsum


class Ops:
    def __init__(self):
        self.ops, self.literal = [], 0

    def add(self, kind, n, b=None):
        if n <= 0:
            return
        last = self.ops[-1] if self.ops else None
        if last and last[0] == kind:
            last[1] += n
            if b:
                last[2] += b
        else:
            self.ops.append([kind, n, bytearray(b or b"")])
        if kind == "L":
            self.literal += n

    def lines(self):
        for kind, n, b in self.ops:
            if kind == "L":
                b = bytes(b)
                for i in range(0, len(b), LITERAL_LINE):
                    yield "L " + base64.b64encode(b[i:i + LITERAL_LINE]).decode()
            else:
                yield f"{kind} {n}"


def retail_ops(f, edits, zsum):
    """(ops, size, SHA-256) of the game directory's copy of a retail file."""
    ops = Ops()
    if f.path not in zsum:
        ops.add("C", f.size)
        return ops, f.size, f.sha
    size, sha = zsum[f.path]
    if size != f.size:
        sys.exit(f"mkinstall: {f.path}: build/retail-ranges.txt and retail-files.txt "
                 "do not agree on its size")
    pos = 0
    for off, kind, n, b in sorted(edits.get(f.path, []), key=lambda e: e[0]):
        if off < pos:
            sys.exit(f"mkinstall: {f.path}: overlapping ranges in build/retail-ranges.txt")
        ops.add("C", off - pos)
        ops.add(kind, n, b)
        pos = off + n
    ops.add("C", f.size - pos)
    return ops, size, sha


def manifest_retail(game, retail):
    """{game path: retail File} of the game directory's files that came
    from the retail release (MANIFEST.txt)."""
    wanted = {}
    for line in open(os.path.join(game, "MANIFEST.txt"), encoding="utf-8"):
        line = line.rstrip("\n")
        if line.startswith("#") or "\t" not in line:
            continue
        frm, rel = line.split("\t", 1)
        m = re.match(r"(data|THIRD-PARTY) ([^:]+):(.+?)( \(.*\))?$", frm)
        if not m or m.group(2) != RELEASE:
            continue
        f = retail.get(m.group(3).lower())
        if f is None:
            sys.exit(f"mkinstall: {rel}: {m.group(3)} is not in build/retail-files.txt")
        wanted[rel] = f
    return wanted


def layout():
    ns = {}
    path = os.path.join(REPO, "build", "retail.py")
    with open(path) as f:
        exec(compile(f.read(), path, "exec"), ns)
    return ns


def layout_retail(game, retail):
    """{game path: retail File} of the files that build/all.sh takes from
    the retail release, by its layout (build/retail.py; the same as
    ext/tools/retailextract.py's `resources` and `thirdparty`). A game
    directory built without the retail game has none of them, nor their
    MANIFEST.txt rows: so the list is worked out, not read."""
    ns = layout()
    files = list(retail.values())
    wanted = {}
    for name in ns["REZ"]:
        f = retail[name.lower()]
        wanted[f.path] = f
    for f in files:
        if "/" in f.path and f.path.split("/", 1)[0] in ns["DATA_DIRS"]:
            wanted[f.path] = f
    for name, copy in ns["RETAIL_COPIES"]:
        wanted[copy] = retail[name.lower()]
    # Bink and the Microsoft runtime; with our Bink player (MANIFEST.txt's
    # EXT rows: TRON20SRC_MIDDLEWARE=open) not RAD's
    ext = set()
    for line in open(os.path.join(game, "MANIFEST.txt"), encoding="utf-8"):
        if line.startswith("EXT ") and "\t" in line:
            ext.add(line.rstrip("\n").split("\t", 1)[1].lower())
    for f in files:
        base = os.path.basename(f.path).lower()
        if base in {n.lower() for n in ns["THIRD_PARTY_NAMES"]} and base not in ext:
            wanted[f.path] = f
    return wanted


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    game, outdir = sys.argv[1].rstrip("/"), sys.argv[2]
    nodiscs = os.path.isfile(os.path.join(game, NODISCS_MARK))
    dlist, _update, _cabs, files = discs.load()
    ways = load_ways()
    edits, zsum = load_ranges()
    retail = {f.path.lower(): f for f in files}
    wanted = layout_retail(game, retail)
    if not nodiscs:
        listed = manifest_retail(game, retail)
        if {k: v.path for k, v in listed.items()} != {k: v.path for k, v in wanted.items()}:
            sys.exit(f"mkinstall: {game}/MANIFEST.txt does not list the retail files that "
                     "build/retail.py gives: "
                     + " ".join(sorted(set(listed) ^ set(wanted))))
    for rel in list(wanted) + [f.path for f in wanted.values()]:
        if re.search(r"[\t\"'`$\\%!*?]", rel) or rel != rel.strip():
            sys.exit(f"mkinstall: {rel}: a name the scripts do not quote")

    # Each retail file, and the way to read it from the images.
    sources, vlines, used = {}, [], set()
    for f in sorted({id(f): f for f in wanted.values()}.values(), key=lambda f: f.path.lower()):
        if f.path not in ways or f.source == "update":
            sys.exit(f"mkinstall: {f.path}: not on the discs (build/retail-ways.txt)")
        kind, ranges = ways[f.path]
        n = len(sources) + 1
        sources[f.path] = n
        vlines.append(f"V {n} {f.size} {f.sha} {f.path}")
        vlines.append(f"W {kind} " + " ".join(f"{IDS[i]}:{o}:{c}" for i, o, c in ranges))
        used.update(i for i, _o, _c in ranges)

    records, sums, literal, wanted_once = [], [], 0, []
    for rel in sorted(wanted, key=str.lower):
        f = wanted[rel]
        ops, size, sha = retail_ops(f, edits, zsum)
        path = os.path.join(game, rel)
        if not nodiscs and (not os.path.isfile(path) or os.path.getsize(path) != size
                            or sha256(path) != sha):
            sys.exit(f"mkinstall: {rel} in {game} is not what the scripts would write "
                     "(build/retail-ways.txt, build/retail-ranges.txt: make retail-ranges)")
        literal += ops.literal
        lines = [f"F {size} {sha} - {sources[f.path]} {rel}"] + list(ops.lines()) + ["E"]
        records += lines
        sums.append(f"{sha}  {rel}")
        if rel in SETTINGS:
            records += [f"F {size} {sha} once {sources[f.path]} {SETTINGS[rel]}"] + lines[1:]
            wanted_once.append(SETTINGS[rel])
    if literal > LITERAL_MAX:
        sys.exit(f"mkinstall: {literal} bytes would ship as literal data")

    data = [f"{TAG}INSTALL 1"]
    for n, d in sorted(dlist.items()):
        data.append(f"D {IDS[str(n)]} {d.size} {d.sha} {d.name}")
        data.append(f"R {IDS[str(n)]} {d.size // SECTOR * RAW_SECTOR}")
    if used - {"1", "2"}:
        sys.exit("mkinstall: build/retail-ways.txt reads from an image that is not a disc")
    data.append("S USA disc1 disc2")
    data += vlines + records

    os.makedirs(outdir, exist_ok=True)
    body = open(os.path.join(SCRIPTS, "linux-install.sh")).read()
    with open(os.path.join(outdir, "linux-install.sh"), "w", newline="\n") as fh:
        fh.write(body + MARKER + "\n" + "\n".join(data) + "\n")
    os.chmod(os.path.join(outdir, "linux-install.sh"), 0o755)

    perl = open(os.path.join(SCRIPTS, "mac-install.command")).read()
    if not perl.endswith("\n__END__\n"):
        sys.exit("mkinstall: mac-install.command must end with __END__ (the list follows it)")
    with open(os.path.join(outdir, "mac-install.command"), "w", newline="\n") as fh:
        fh.write(perl + MARKER + "\n" + "\n".join(data) + "\n")
    os.chmod(os.path.join(outdir, "mac-install.command"), 0o755)

    ps = open(os.path.join(SCRIPTS, "windows-install.ps1")).read()
    psm = f"#__{TAG}_PS__"
    if not ps.isascii() or psm in ps or MARKER in ps:
        sys.exit("mkinstall: windows-install.ps1 must be ASCII, without the markers")
    bat = BAT_HEADER + [psm] + ps.splitlines() + [psm, MARKER] + data
    with open(os.path.join(outdir, "windows-install.bat"), "w", newline="\r\n") as fh:
        fh.write("\n".join(bat) + "\n")

    # The public archive's MANIFEST.txt: the game directory's, but each
    # file of the retail release as the scripts install it (the same rows
    # whether or not the build had the retail game), and the directories
    # that only the scripts fill (left out of the archive)
    rows = {}
    for line in open(os.path.join(game, "MANIFEST.txt"), encoding="utf-8"):
        line = line.rstrip("\n")
        if line.startswith("#") or "\t" not in line:
            continue
        frm, rel = line.split("\t", 1)
        if not re.match(rf"(data|THIRD-PARTY) {re.escape(RELEASE)}:", frm):
            rows[rel] = frm
    for rel, f in wanted.items():
        rows[rel] = f"install {RELEASE}:{f.path}"
    with open(os.path.join(outdir, "MANIFEST.txt"), "w", newline="\n") as fh:
        fh.write("# origin<TAB>path, one row per file build/all.sh put here "
                 "(install: the install scripts write it)\n")
        for rel in sorted(rows, key=str.lower):
            fh.write(f"{rows[rel]}\t{rel}\n")
    with open(os.path.join(outdir, "disc-dirs.txt"), "w", newline="\n") as fh:
        for d in layout()["DATA_DIRS"]:
            fh.write(d + "\n")
    with open(os.path.join(outdir, "disc-files.txt"), "w", newline="\n") as fh:
        for rel in sorted(list(wanted) + wanted_once, key=str.lower):
            fh.write(rel + "\n")
    with open(os.path.join(outdir, "disc-sums.txt"), "w", newline="\n") as fh:
        fh.write("".join(line + "\n" for line in sums))
    print(f"   install scripts: {len(wanted) + len(wanted_once)} files from {len(sources)} "
          f"retail files on the discs, {literal} bytes of ours"
          + (" (built without the retail game: no game data to check)" if nodiscs else ""))


def cmd_ranges(retaildir, game):
    """build/retail-ranges.txt from a game directory built with the retail
    game: the zeroed PE entries of each changed archive, then every other
    byte that differs."""
    sys.path.insert(0, os.path.join(REPO, "ext", "tools"))
    from retailextract import rez_pe_entries  # noqa: E402
    _d, _u, _c, files = discs.load()
    retail = {f.path.lower(): f for f in files}
    lines = []
    for rel, f in sorted(manifest_retail(game, retail).items(), key=lambda kv: kv[0].lower()):
        if not rel.lower().endswith(".rez"):
            continue
        ours, theirs = os.path.join(game, rel), os.path.join(retaildir, f.path)
        if os.path.samefile(ours, theirs) or sha256(ours) == f.sha:
            continue
        a = bytearray(open(theirs, "rb").read())
        b = open(ours, "rb").read()
        if len(a) != len(b):
            sys.exit(f"mkinstall: {rel}: not the size of the retail file")
        out = []
        for _name, pos, size in sorted(rez_pe_entries(theirs), key=lambda e: e[1]):
            a[pos:pos + size] = bytes(size)
            out.append(f"ZERO\t{f.path}\t{pos}\t{size}")
        step = 1 << 16
        for c in range(0, len(a), step):
            if a[c:c + step] == b[c:c + step]:
                continue
            i, end = c, min(c + step, len(a))
            while i < end:
                if a[i] == b[i]:
                    i += 1
                    continue
                j = i
                while j < len(a) and a[j] != b[j]:
                    j += 1
                out.append(f"LIT\t{f.path}\t{i}\t{b[i:j].hex()}")
                a[i:j] = b[i:j]
                i = j
        out.append(f"ZSUM\t{f.path}\t{len(b)}\t{hashlib.sha256(b).hexdigest()}")
        lines += out
        print(f"   {rel}: {len(out) - 1} changes")
    with open(RANGES, "w", newline="\n") as fh:
        fh.write(RANGES_HEADER + "".join(line + "\n" for line in lines))
    print(f"   wrote {os.path.relpath(RANGES, REPO)}")


# cmd runs this; everything from the first #__TRON20_PS__ line on is never
# parsed by it (`exit /b` comes first). The PowerShell between the markers is
# written to a temporary .ps1 and run with -File, which passes the dropped or
# typed paths through as they are.
BAT_HEADER = r"""@echo off
rem windows-install.bat -- install the game data from your Tron 2.0 discs.
rem
rem   Drop the images of both discs of Tron 2.0 (USA) onto this file -- or
rem   put them in this folder or the one above it and double-click it. You
rem   need no update: Update 2's data is in this release. An image is a
rem   plain .iso, a raw .bin (or its .cue), or a .zip of one; a .bin or .zip
rem   is first converted to a temporary .iso in this folder (about 0.7 GB a
rem   disc, removed at the end).
rem
rem First it checks that this folder is the release unpacked and nothing
rem else (SHA256SUMS). It checks each image's SHA-256, copies the game's
rem data out of them into this folder and checks every file it writes.
rem Nothing is installed or run from them. In the legacy build it then
rem registers the game as the original setup and Update 2 did, with
rem tron20src.rez in the place of the update's archives (the registry
rem values the retail launcher reads). The modern build needs no registry.
setlocal
set "TRON20_SELF=%~f0"
set "TRON20_PS1=%TEMP%\tron20-install-%RANDOM%%RANDOM%.ps1"
powershell -NoProfile -ExecutionPolicy Bypass -Command "$t=[IO.File]::ReadAllText($env:TRON20_SELF); $m='#'+'__TRON20_PS__'; $a=$t.IndexOf($m); $b=$t.IndexOf($m,$a+1); [IO.File]::WriteAllText($env:TRON20_PS1,$t.Substring($a,$b-$a))"
if errorlevel 1 goto failed
powershell -NoProfile -ExecutionPolicy Bypass -File "%TRON20_PS1%" "%~f0" %*
set "TRON20_RC=%errorlevel%"
del "%TRON20_PS1%" 2>nul
pause
exit /b %TRON20_RC%
:failed
echo Could not start PowerShell.
pause
exit /b 1""".splitlines()

if __name__ == "__main__":
    if sys.argv[1:2] == ["ranges"] and len(sys.argv) == 4:
        cmd_ranges(sys.argv[2], sys.argv[3])
    else:
        main()
