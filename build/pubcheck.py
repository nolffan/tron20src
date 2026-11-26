#!/usr/bin/env python3
"""Check a public archive: no retail data, nothing that names the builder.

  build/pubcheck.py <public .tar.gz> <gamedir>

build/package.sh runs it on each public archive before it gives the archive
its name. It fails if:

  * a file of the archive is a file of the retail game, by its SHA-256:
    any file of build/retail-files.txt (the discs, Update 2, the installed
    game), an archive as the build changes it (build/retail-ranges.txt)
    and, when the retail game is
    installed (out/retail/tron2-usa-1.030, build/retail.sh), every entry
    of every one of its .rez archives (the retail game DLLs among them) and
    the zeroed copy of each archive in <gamedir>. A file that MANIFEST.txt
    says we built (`built`) and that is byte-identical to a retail binary is
    the aim of the legacy build: it is listed, and it does not fail;
  * an `asset` row of MANIFEST.txt -- the retail data that the release
    ships on purpose, Update 2's (assets/README.md) -- is not exactly the
    committed assets/: tron20src.rez must hold the files of assets/rez/,
    each byte for byte, and nothing else; every other asset file must be
    the file of assets/ that its row names. A retail file that is such an
    asset is listed, and it does not fail;
  * a file of the archive is not one that we build or write: a `built` or
    `EXT` row of <gamedir>/MANIFEST.txt (or a non-retail `THIRD-PARTY` one,
    or an `ours` one: a file of our source, as the DirectMusic manifest
    and the run scripts linux-run.sh and linux-server.sh, or an `asset`
    one, as above),
    the .pdb and .map of a built binary, MANIFEST.txt, tron20cmds.txt,
    LICENSE_NOTES.md and licenses/, the install scripts and SHA256SUMS;
  * a file holds a home directory or a user's directory (/home/, /Users/,
    \\Users\\, "Documents and Settings"), or one of the words in
    $TRON20SRC_SCAN_WORDS (the Makefile gives the login and host names of
    the machine that builds: they are only looked for).

Prints what it checked; exit status 1 on any failure.
"""
import hashlib
import io
import os
import re
import sys
import tarfile

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(REPO, "build"))
sys.path.insert(0, os.path.join(REPO, "ext", "tools"))
import discs  # noqa: E402
import mkinstall  # noqa: E402

RELEASE = mkinstall.RELEASE
OURS = {"MANIFEST.txt", "tron20cmds.txt", "LICENSE_NOTES.md", "linux-install.sh",
        "windows-install.bat", "mac-install.command", "SHA256SUMS"}
PRIVATE = re.compile(rb"(?i)/home/|/Users/|\\Users\\|Documents and Settings")
CHUNK = 1 << 20


def retail_root():
    return os.environ.get("TRON20SRC_RETAIL") or os.path.join(REPO, "out", "retail", RELEASE)


def rez_entry_hashes(root, files):
    """{sha: name} of every entry of the retail .rez archives, cached in
    out/ by the list the retail game was installed from."""
    from rezextract import Rez
    key = hashlib.sha256(open(discs.LIST, "rb").read()).hexdigest()
    cache = os.path.join(REPO, "out", ".retail-rez-hashes")
    if os.path.isfile(cache):
        lines = open(cache).read().splitlines()
        if lines and lines[0] == key:
            return dict(l.split(" ", 1) for l in lines[1:])
    out = {}
    with open(cache + ".part", "w") as fh:
        fh.write(key + "\n")
        for f in files:
            if not f.path.lower().endswith(".rez"):
                continue
            rez = Rez(os.path.join(root, f.path))
            for name, pos, size in list(rez.walk()):
                if size < 64:              # (too small to mean anything)
                    continue
                rez.f.seek(pos)
                sha = hashlib.sha256(rez.f.read(size)).hexdigest()
                out.setdefault(sha, f"{f.path}:{name}")
            rez.f.close()
        for sha, name in out.items():
            fh.write(f"{sha} {name}\n")
    os.replace(cache + ".part", cache)
    return out


def asset_problem(data, source):
    """Why a file of an `asset` row is not the committed asset it names,
    or None. A .rez archive (source assets/rez) must hold exactly the files
    of assets/rez/, byte for byte; any other file must be its source."""
    from rezextract import Rez
    src = os.path.join(REPO, source)
    if not os.path.realpath(src).startswith(os.path.join(os.path.realpath(REPO), "assets") + os.sep):
        return f"its MANIFEST.txt row names {source}, which is not in assets/"
    if os.path.isfile(src):
        return None if open(src, "rb").read() == data else f"not the bytes of {source}"
    if not os.path.isdir(src):
        return f"its source {source} is not in this checkout"
    want = {}
    for root, _dirs, names in os.walk(src):
        for fn in names:
            full = os.path.join(root, fn)
            want[os.path.relpath(full, src).replace(os.sep, "/").lower()] = full
    try:
        rez = Rez(io.BytesIO(data))
        have = list(rez.walk())
    except (RuntimeError, ValueError, OSError) as e:
        return f"not a .rez archive ({e})"
    seen = set()
    for name, pos, size in have:
        key = name.lower()
        if key not in want:
            return f"holds {name}, which is not in {source}/"
        if data[pos:pos + size] != open(want[key], "rb").read():
            return f"its {name} is not the bytes of {source}/"
        seen.add(key)
    if seen != set(want):
        return f"does not hold all of {source}/: " + " ".join(sorted(set(want) - seen))
    # and no bytes besides them but the header and the directories
    rest = len(data) - sum(size for _n, _p, size in have)
    if rest > 4096 + 64 * len(have):
        return f"{rest} bytes besides its files: more than its header and directories"
    return None


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    archive, game = sys.argv[1], sys.argv[2].rstrip("/")
    dlist, update, cabs, files = discs.load()
    retail = {}
    for f in files:
        retail[f.sha] = f"{RELEASE}:{f.path}"
    for d in dlist.values():
        retail[d.sha] = d.name
    retail[update.sha] = update.name
    for c in cabs:
        retail[c.sha] = f"the discs' {c.name}"
    _edits, zsum = mkinstall.load_ranges()
    for a, (_size, sha) in zsum.items():
        retail[sha] = f"{a} (as the build changes it)"
    root = retail_root()
    entries = 0
    if os.path.isfile(os.path.join(root, ".retail-files")):
        rez = rez_entry_hashes(root, files)
        entries = len(rez)
        for sha, name in rez.items():
            retail.setdefault(sha, name)

    # what we build or write (MANIFEST.txt)
    allowed, built, assets = set(OURS), set(), {}
    for line in open(os.path.join(game, "MANIFEST.txt"), encoding="utf-8"):
        line = line.rstrip("\n")
        if line.startswith("#") or "\t" not in line:
            continue
        frm, rel = line.split("\t", 1)
        if frm.startswith(("built ", "EXT ")):
            allowed.add(rel)
            built.add(rel)
            stem = os.path.splitext(rel)[0]
            allowed.update({stem + ".pdb", stem + ".map"})
        elif frm.startswith("THIRD-PARTY ") and not frm.startswith(f"THIRD-PARTY {RELEASE}:"):
            allowed.add(rel)
        elif frm.startswith("ours "):
            # a file of our source, as it is (the modern build's
            # Lithtech.exe.manifest, src/modern/directmusic/)
            allowed.add(rel)
        elif frm.startswith("asset "):
            # Update 2's data, which the release ships (assets/README.md):
            # "asset assets/rez (lithrez)" or "asset assets/<path>"
            allowed.add(rel)
            assets[rel] = re.match(r"asset (.+?)( \(.*\))?$", frm).group(1)
    words = [w.encode() for w in os.environ.get("TRON20SRC_SCAN_WORDS", "").split() if len(w) >= 3]

    bad, same, shipped, n = [], [], [], 0
    with tarfile.open(archive) as t:
        for m in t:
            if not m.isfile():
                continue
            rel = m.name.split("/", 1)[1] if "/" in m.name else m.name
            data = t.extractfile(m).read()
            n += 1
            sha = hashlib.sha256(data).hexdigest()
            if rel in assets:
                why = asset_problem(data, assets[rel])
                if why:
                    bad.append(f"{rel}: {why}")
                else:
                    shipped.append(f"{rel} (= {retail[sha]})" if sha in retail else
                                   f"{rel} ({assets[rel]})")
            elif sha in retail:
                if rel in built:
                    same.append(f"{rel} (= {retail[sha]})")
                else:
                    bad.append(f"{rel}: is the retail file {retail[sha]}")
            if rel not in allowed and not rel.startswith("licenses/"):
                bad.append(f"{rel}: not a file that we build or write (MANIFEST.txt)")
            hit = PRIVATE.search(data)
            if hit:
                bad.append(f"{rel}: holds {hit.group(0).decode('latin-1')!r} at {hit.start():#x}")
            for w in words:
                at = data.find(w)
                if at >= 0:
                    bad.append(f"{rel}: holds a name of the machine that built it at {at:#x}")
    print(f"   pubcheck: {n} files against {len(retail)} retail SHA-256 values"
          + (f" ({entries} of them .rez entries)" if entries else
             " (the retail game is not installed: its .rez entries not checked)"))
    for s in same:
        print(f"   pubcheck: built here, byte-identical to retail: {s}")
    for s in shipped:
        print(f"   pubcheck: retail data shipped on purpose (assets/README.md): {s}")
    for b in bad:
        print(f"   pubcheck: FAILED: {b}", file=sys.stderr)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
