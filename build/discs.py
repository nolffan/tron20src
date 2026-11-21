#!/usr/bin/env python3
"""Install the retail game from your own disc images and Update 2, and check it.

  build/discs.py install <outdir> [file ...]
  build/discs.py verify  <outdir>
  build/discs.py find    [file ...]

The retail game is Tron 2.0 (USA) on two discs, with its Update 2 (v1.030,
the "complete" update) installed over it. No part of it is in this
repository. build/retail-files.txt is the list of its files: for each file,
its size, its SHA-256, and where its bytes are. This script uses that list
and nothing else to install the game, as the discs' own setup and the
update's setup do.

You give the files: your images of the two discs, and the update's own
executable (tron-update_v1x2_complete.exe). If you give no files on the
command line, the script looks in $TRON20_ISOS, or else in isos/ at the
root of the repository. Each disc image can be:

  .iso   a plain image, 2048-byte sectors (your own rip, for example)
  .bin   a raw image, 2352-byte MODE1 sectors (for example a redump dump)
  .cue   a cue sheet: the .bin it names is used
  .zip   a zip that holds one such .bin or .iso (it is read in place;
         nothing is unpacked to the disk)

The update can be the .exe itself or a .zip that holds it.

A disc is recognised by the volume label and the volume size in its file
system. The image does not have to be identical to a known image: every
file that the install writes is checked against the list (its size and its
SHA-256). Thus any correct copy of the discs works. The update is
recognised by its size and its SHA-256.

Where a file comes from (the source field of its F line):

  <disc>  the file is on the disc, not compressed: its bytes are copied
          from the image.
  cab     the file is compressed in the discs' InstallShield cabinets
          (data1.hdr, data1.cab .. data3.cab: the C lines). The cabinets
          are copied out of the images into a work directory, checked, and
          the file group is unpacked with unshield.
  update  the file is in the update's InstallShield cabinets, which are in
          a Microsoft cabinet inside the update's executable. The script
          unpacks that cabinet, then the file group with unshield.

`install` writes every file of the list into <outdir> (the build uses
out/retail/tron2-usa-1.030/). A file that is already correct stays. Each
file is checked before it gets its name. The work directory
(<outdir>.work) is removed at the end. Then the script writes
<outdir>/.retail-files, a copy of the list that it installed. `verify`
checks every file of <outdir> against the list: all the files must be
there, with the correct size and SHA-256. Exit status 0 if they are, 1 if
not. `find` shows which file is which disc, and which is the update.

Exit status 3 if a disc image or the update is missing: the message tells
you what to supply.
"""
import hashlib
import os
import shutil
import struct
import subprocess
import sys
import zipfile
import zlib

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIST = os.path.join(REPO, "build", "retail-files.txt")
SECTOR = 2048
RAW_SECTOR = 2352
RAW_HEAD = 16                               # sync (12) + header (4), MODE1
SYNC = b"\x00" + b"\xff" * 10 + b"\x00"
CHUNK = 1 << 20
EXTS = (".iso", ".bin", ".cue", ".zip", ".exe")
UNSHIELD = os.environ.get("UNSHIELD", "unshield")


class Disc:
    """One disc of the list."""
    def __init__(self, n, label, blocks, size, sha, name):
        self.n, self.label, self.blocks = n, label, blocks
        self.size, self.sha, self.name = size, sha, name


class Update:
    """The update's executable."""
    def __init__(self, name, size, sha):
        self.name, self.size, self.sha = name, size, sha


class Cabinet:
    """One file of the discs' cabinet set."""
    def __init__(self, name, size, sha, disc, off, dpath):
        self.name, self.size, self.sha = name, size, sha
        self.disc, self.off, self.dpath = disc, off, dpath


class File:
    """One file of the install."""
    def __init__(self, path, size, sha, source, a, b):
        self.path, self.size, self.sha, self.source = path, size, sha, source
        if source in ("cab", "update"):
            self.group, self.gpath = a, b
            self.disc = None
        else:
            self.disc, self.off, self.dpath = int(source), int(a), b


def load(path=LIST):
    """({disc number: Disc}, Update, [Cabinet], [File]) from the list."""
    discs, cabs, files, update = {}, [], [], None
    for line in open(path, encoding="utf-8"):
        line = line.rstrip("\n")
        if not line or line.startswith("#"):
            continue
        f = line.split("\t")
        if f[0] == "D":
            discs[int(f[1])] = Disc(int(f[1]), f[2], int(f[3]), int(f[4]), f[5], f[6])
        elif f[0] == "U":
            update = Update(f[1], int(f[2]), f[3])
        elif f[0] == "C":
            cabs.append(Cabinet(f[1], int(f[2]), f[3], int(f[4]), int(f[5]), f[6]))
        elif f[0] == "F":
            files.append(File(f[1], int(f[2]), f[3], f[4], f[5], f[6]))
        else:
            sys.exit(f"discs.py: {path}: a line that is not D, U, C or F: {line[:60]}")
    return discs, update, cabs, files


# --- disc images ------------------------------------------------------------

class Image:
    """The 2048-byte user data of one disc image, read from any of the
    accepted formats. Reads that go forward are cheap in all of them; a
    .zip goes back to the start of its member for a read that goes back."""

    def __init__(self, path):
        self.path, self.label = path, os.path.basename(path)
        self.zip = self.f = None
        src = path
        if path.lower().endswith(".cue"):
            src = self._cue(path)
        if zipfile.is_zipfile(src):
            self.zip = zipfile.ZipFile(src)
            members = [i for i in self.zip.infolist()
                       if i.filename.lower().endswith((".bin", ".iso"))]
            if len(members) != 1:
                raise ValueError("not one .bin or .iso in the zip")
            self.member = members[0]
            self.f = self.zip.open(self.member)
            self.length = self.member.file_size
        else:
            self.f = open(src, "rb")
            self.length = os.path.getsize(src)
        head = self.f.read(len(SYNC))
        self.raw = head == SYNC and self.length % RAW_SECTOR == 0
        self._rewind()

    @staticmethod
    def _cue(path):
        import re
        text = open(path, encoding="latin-1").read()
        m = re.search(r'^\s*FILE\s+"([^"]+)"', text, re.M | re.I)
        if not m or not re.search(r"TRACK\s+01\s+MODE1/(2352|2048)", text, re.I):
            raise ValueError("not a cue sheet of one MODE1 data track")
        return os.path.join(os.path.dirname(path), m.group(1))

    def _rewind(self):
        if self.zip:
            self.f.close()
            self.f = self.zip.open(self.member)
        else:
            self.f.seek(0)
        self.pos = 0

    def _seek(self, raw_off):
        if raw_off < self.pos:
            self._rewind()
        if self.zip:
            while self.pos < raw_off:      # forward: read and drop
                n = len(self.f.read(min(CHUNK, raw_off - self.pos)))
                if not n:
                    raise EOFError
                self.pos += n
        else:
            self.f.seek(raw_off)
            self.pos = raw_off

    def _read(self, n):
        b = self.f.read(n)
        self.pos += len(b)
        return b

    def chunks(self, off, n):
        """The user data bytes off .. off + n, in chunks."""
        if not self.raw:
            self._seek(off)
            while n:
                b = self._read(min(CHUNK, n))
                if not b:
                    raise EOFError
                n -= len(b)
                yield b
            return
        sec, within = divmod(off, SECTOR)
        self._seek(sec * RAW_SECTOR)
        per = CHUNK // SECTOR
        while n:
            buf = self._read(RAW_SECTOR * per)
            if not buf or len(buf) % RAW_SECTOR and len(buf) < RAW_SECTOR:
                raise EOFError
            while len(buf) % RAW_SECTOR:
                more = self._read(RAW_SECTOR - len(buf) % RAW_SECTOR)
                if not more:
                    raise EOFError
                buf += more
            data = b"".join(buf[i + RAW_HEAD:i + RAW_HEAD + SECTOR]
                            for i in range(0, len(buf), RAW_SECTOR))
            data = data[within:within + n]
            within = 0
            n -= len(data)
            yield data

    def read(self, off, n):
        return b"".join(self.chunks(off, n))

    def volume(self):
        """(volume label, volume size in blocks) of its ISO 9660 file system."""
        pvd = self.read(16 * SECTOR, SECTOR)
        if pvd[1:6] != b"CD001" or pvd[0] != 1:
            return None
        return pvd[40:72].decode("ascii", "replace").rstrip(), struct.unpack_from("<I", pvd, 80)[0]


def candidates(args):
    """The files to look at: the arguments, or the files in $TRON20_ISOS or
    isos/ (and the directories given)."""
    if args:
        out = []
        for a in args:
            if os.path.isdir(a):
                out += candidates_in(a)
            else:
                out.append(a)
        return out
    d = os.environ.get("TRON20_ISOS") or os.path.join(REPO, "isos")
    return candidates_in(d)


def candidates_in(d):
    if not os.path.isdir(d):
        return []
    names = sorted(n for n in os.listdir(d) if n.lower().endswith(EXTS))
    # .iso first (no conversion), and a .bin before the .cue that names it
    order = {e: i for i, e in enumerate(EXTS)}
    names.sort(key=lambda n: order[os.path.splitext(n)[1].lower()])
    return [os.path.join(d, n) for n in names]


def update_bytes(path, update):
    """The update's executable out of `path` (the .exe, or a .zip that holds
    it), or None if `path` does not hold it."""
    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as z:
            for i in z.infolist():
                if i.file_size == update.size and i.filename.lower().endswith(".exe"):
                    b = z.read(i)
                    if hashlib.sha256(b).hexdigest() == update.sha:
                        return b
        return None
    if os.path.getsize(path) != update.size:
        return None
    b = open(path, "rb").read()
    return b if hashlib.sha256(b).hexdigest() == update.sha else None


def identify(discs, update, paths, quiet=False, want_update=True):
    """({disc number: Image}, the update's bytes or None) for the files
    among `paths`: the first image found for each disc, and the first file
    that holds the update."""
    found, seen, exe = {}, set(), None
    for p in paths:
        if len(found) == len(discs) and (exe is not None or not want_update):
            break
        try:
            real = os.path.realpath(Image._cue(p) if p.lower().endswith(".cue") else p)
        except (OSError, ValueError):
            real = p
        if real in seen:
            continue
        seen.add(real)
        if exe is None and want_update and p.lower().endswith((".exe", ".zip")):
            try:
                exe = update_bytes(p, update)
            except (OSError, zipfile.BadZipFile):
                exe = None
            if exe is not None:
                if not quiet:
                    print(f"   {os.path.basename(p)}: the update ({update.name})")
                continue
        if p.lower().endswith(".exe"):
            if not quiet:
                print(f"   {os.path.basename(p)}: not the update of this release")
            continue
        try:
            img = Image(p)
            vol = img.volume()
        except (OSError, ValueError, EOFError, zipfile.BadZipFile) as e:
            if not quiet:
                print(f"   {os.path.basename(p)}: not read ({e})")
            continue
        for n, d in sorted(discs.items()):
            if n not in found and vol == (d.label, d.blocks):
                found[n] = img
                if not quiet:
                    print(f"   {os.path.basename(p)}: disc {n} ({d.label})")
                break
        else:
            if not quiet:
                print(f"   {os.path.basename(p)}: not a disc of this release")
    return found, exe


def missing_message(discs, update, found, exe, want_update=True):
    lines = ["error: the retail game's disc images or its update are missing. Bring your own discs:"]
    for n, d in sorted(discs.items()):
        if n not in found:
            lines.append(f"  disc {n} (volume label \"{d.label}\", {d.blocks} blocks), for example {d.name}")
    if want_update and exe is None:
        lines.append(f"  the update {update.name} ({update.size} bytes, SHA-256 {update.sha})")
    lines += [
        "Put both images of Tron 2.0 (USA) and the executable of its Update 2",
        "(v1.030, the complete update) in isos/ at the root of the repository (a",
        "hard link is good: ln <file> isos/), or set TRON20_ISOS=<directory>. An",
        "image can be a .iso, a .bin with its .cue, or a .zip that holds one of",
        "them. The update can be the .exe or a .zip that holds it.",
        "See README.md, \"What you need\".",
    ]
    return "\n".join(lines)


# --- the update's Microsoft cabinet ---------------------------------------------
# The update is an InstallShield 6 setup in a self-extractor: a small PE
# whose overlay is one Microsoft cabinet (MSCF) that holds the setup's
# folder (setup.exe, data1.hdr, the InstallShield cabinets dataN.cab, ...).
# Only MSZIP and uncompressed folders occur.

def find_cabinet(data):
    """Offset of the cabinet: the first MSCF header whose fields hold up
    (the stub's code contains the four bytes too)."""
    pos = 0
    while True:
        pos = data.find(b"MSCF\0\0\0\0", pos)
        if pos < 0:
            raise ValueError("no Microsoft cabinet in the update")
        size, = struct.unpack_from("<I", data, pos + 8)
        _minor, major, nfolders, nfiles = struct.unpack_from("<BBHH", data, pos + 24)
        if major == 1 and 0 < nfolders < 64 and 0 < nfiles < 65536 and pos + size <= len(data):
            return pos
        pos += 4


def read_cabinet(data):
    """[(name, bytes)] of every file in the cabinet that `data` holds."""
    base = find_cabinet(data)
    files_off, = struct.unpack_from("<I", data, base + 16)
    nfolders, nfiles, flags = struct.unpack_from("<HHH", data, base + 26)
    pos = base + 36
    folder_res = data_res = 0
    if flags & 4:                       # cfhdrRESERVE_PRESENT
        hdr_res, folder_res, data_res = struct.unpack_from("<HBB", data, pos)
        pos += 4 + hdr_res
    folders = []
    for _ in range(nfolders):
        off, nblocks, kind = struct.unpack_from("<IHH", data, pos)
        pos += 8 + folder_res
        folders.append((off, nblocks, kind & 0xF))
    unpacked = []
    for off, nblocks, kind in folders:
        if kind not in (0, 1):
            raise ValueError(f"cabinet folder compression {kind}: only MSZIP or none is known")
        p, out, window = base + off, bytearray(), b""
        for _ in range(nblocks):
            _csum, cb, cu = struct.unpack_from("<IHH", data, p)
            p += 8 + data_res
            block = data[p:p + cb]
            p += cb
            if kind == 0:
                piece = block
            else:
                # MSZIP: "CK" and a deflate stream whose history is the
                # previous block's output.
                if block[:2] != b"CK":
                    raise ValueError("bad MSZIP block")
                z = zlib.decompressobj(-15, zdict=window) if window else zlib.decompressobj(-15)
                piece = z.decompress(block[2:]) + z.flush()
            if len(piece) != cu:
                raise ValueError("cabinet block size mismatch")
            out += piece
            window = bytes(piece[-32768:])
        unpacked.append(bytes(out))
    pos = base + files_off
    result = []
    for _ in range(nfiles):
        size, off, folder = struct.unpack_from("<IIH", data, pos)
        end = data.index(b"\0", pos + 16)
        name = data[pos + 16:end].decode("latin-1").replace("\\", "/").lstrip("/")
        pos = end + 1
        result.append((name, unpacked[folder][off:off + size]))
    return result


# --- install and verify -------------------------------------------------------

def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for b in iter(lambda: f.read(CHUNK), b""):
            h.update(b)
    return h.hexdigest()


def correct(path, f):
    return os.path.isfile(path) and os.path.getsize(path) == f.size and \
        sha256_file(path) == f.sha


class Failed(Exception):
    pass


def copy_range(img, off, size, sha, dest, what, disc):
    """Copy `size` bytes at `off` of `img` to `dest`, if their SHA-256 is `sha`."""
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    part = dest + ".part"
    h = hashlib.sha256()
    try:
        with open(part, "wb") as out:
            for b in img.chunks(off, size):
                h.update(b)
                out.write(b)
    except EOFError:
        os.unlink(part)
        raise Failed(f"{img.label} ends before {what}: the image is not a whole copy of disc {disc}")
    if h.hexdigest() != sha:
        os.unlink(part)
        raise Failed(f"{what} in {img.label} is not the retail file (SHA-256 {h.hexdigest()}, "
                     f"the list has {sha}): the image is not a correct copy of disc {disc} "
                     f"of this release")
    os.replace(part, dest)


def unshield(hdr, group, outdir):
    """Unpack file group `group` of the cabinet set of `hdr` into outdir;
    returns the directory that holds the group's files."""
    shutil.rmtree(outdir, ignore_errors=True)
    os.makedirs(outdir)
    try:
        r = subprocess.run([UNSHIELD, "-d", outdir, "-g", group, "x", hdr],
                           stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
    except FileNotFoundError:
        raise Failed(f"{UNSHIELD} is not installed: the build image has it (docker/Dockerfile); "
                     f"on the host, install the unshield package")
    if r.returncode:
        raise Failed(f"unshield could not unpack the file group {group!r} of {hdr}: "
                     f"{r.stderr.strip()[-300:]}")
    tops = os.listdir(outdir)             # unshield writes <dir>/<group>/...
    if len(tops) != 1:
        raise Failed(f"unshield wrote {len(tops)} directories for the file group {group!r}")
    return os.path.join(outdir, tops[0])


def lookup(top, rel):
    """The path of `rel` under `top`, found case-insensitively."""
    p = top
    for part in rel.split("/"):
        names = {n.lower(): n for n in os.listdir(p)} if os.path.isdir(p) else {}
        if part.lower() not in names:
            return None
        p = os.path.join(p, names[part.lower()])
    return p


def take(files, hdr, work, outdir, origin):
    """Unpack the groups of `files` from the cabinet set of `hdr` and move
    each file into place, checked."""
    groups = {}
    for f in files:
        groups.setdefault(f.group, []).append(f)
    for group, members in groups.items():
        top = unshield(hdr, group, os.path.join(work, "x"))
        for f in members:
            src = lookup(top, f.gpath)
            if src is None:
                raise Failed(f"{f.gpath} is not in the file group {group!r} of {origin}")
            if os.path.getsize(src) != f.size or sha256_file(src) != f.sha:
                raise Failed(f"{f.gpath} of {origin} is not the retail file {f.path}")
            dest = os.path.join(outdir, f.path)
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            shutil.move(src, dest + ".part")
            os.replace(dest + ".part", dest)
        shutil.rmtree(os.path.join(work, "x"), ignore_errors=True)


def install(outdir, args):
    discs, update, cabs, files = load()
    outdir = os.path.abspath(outdir)
    todo = [f for f in files if not correct(os.path.join(outdir, f.path), f)]
    need_update = any(f.source == "update" for f in todo)
    found, exe = identify(discs, update, candidates(args), want_update=need_update)
    if len(found) < len(discs) or (need_update and exe is None):
        print(missing_message(discs, update, found, exe, need_update), file=sys.stderr)
        return 3
    os.makedirs(outdir, exist_ok=True)
    stamp = os.path.join(outdir, ".retail-files")
    if os.path.exists(stamp):
        os.unlink(stamp)                   # not whole until the end
    work = outdir.rstrip("/") + ".work"
    shutil.rmtree(work, ignore_errors=True)
    try:
        # off the discs, in disc order, forward through each image
        for f in sorted((f for f in todo if f.disc), key=lambda f: (f.disc, f.off)):
            copy_range(found[f.disc], f.off, f.size, f.sha, os.path.join(outdir, f.path),
                       f.dpath, f.disc)
        # out of the discs' cabinets
        cab_files = [f for f in todo if f.source == "cab"]
        if cab_files:
            print(f"   copying the cabinets out of the disc images ({len(cabs)} files)")
            for c in sorted(cabs, key=lambda c: (c.disc, c.off)):
                copy_range(found[c.disc], c.off, c.size, c.sha, os.path.join(work, "cab", c.name),
                           c.dpath, c.disc)
            print(f"   unpacking {len(cab_files)} files from them (unshield)")
            take(cab_files, os.path.join(work, "cab", "data1.hdr"), work, outdir, "the discs' cabinets")
            shutil.rmtree(os.path.join(work, "cab"))
        # out of the update
        upd_files = [f for f in todo if f.source == "update"]
        if upd_files:
            print(f"   unpacking {len(upd_files)} files from {update.name} (unshield)")
            setup = os.path.join(work, "update")
            hdr = None
            for name, blob in read_cabinet(exe):
                dest = os.path.join(setup, name)
                os.makedirs(os.path.dirname(dest), exist_ok=True)
                with open(dest, "wb") as out:
                    out.write(blob)
                if os.path.basename(name).lower() == "data1.hdr":
                    hdr = dest
            if hdr is None:
                raise Failed(f"{update.name} has no data1.hdr")
            take(upd_files, hdr, work, outdir, update.name)
    except Failed as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    finally:
        shutil.rmtree(work, ignore_errors=True)
    with open(LIST, "rb") as src, open(stamp, "wb") as out:
        out.write(src.read())
    print(f"   {len(todo)} files written, {len(files) - len(todo)} already correct -> {outdir}")
    return 0


def verify(outdir):
    _discs, _update, _cabs, files = load()
    bad = []
    for f in files:
        p = os.path.join(outdir, f.path)
        if not os.path.isfile(p):
            bad.append(f"missing  {f.path}")
        elif os.path.getsize(p) != f.size or sha256_file(p) != f.sha:
            bad.append(f"changed  {f.path}")
    for line in bad:
        print(f"   {line}")
    if bad:
        return 1
    print(f"   all {len(files)} files match build/retail-files.txt")
    return 0


def main():
    a = sys.argv[1:]
    if a[:1] == ["install"] and len(a) >= 2:
        return install(a[1], a[2:])
    if a[:1] == ["verify"] and len(a) == 2:
        return verify(a[1])
    if a[:1] == ["find"]:
        discs, update, _cabs, _files = load()
        found, exe = identify(discs, update, candidates(a[1:]))
        if len(found) < len(discs) or exe is None:
            print(missing_message(discs, update, found, exe), file=sys.stderr)
            return 3
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
