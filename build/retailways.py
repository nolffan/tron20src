#!/usr/bin/env python3
"""Where each retail file's bytes are in your disc images and the update.

  build/retailways.py write [file ...]   write build/retail-ways.txt
  build/retailways.py check [file ...]   check build/retail-ways.txt

build/discs.py installs the retail game with unshield: it copies the
InstallShield cabinets out of the disc images and the update, and unpacks
them. The install scripts of a public archive (build/mkinstall.py) cannot
use unshield (it is not on Windows, nor on every Linux). They read the
bytes of each file directly from the images. This script finds those
bytes, from the InstallShield 6 headers (data1.hdr) of the discs and of
the update, and writes them into build/retail-ways.txt. That list holds no
game data: only offsets, sizes and SHA-256 values.

The files are those of build/retail-files.txt, and the images are found as
build/discs.py finds them (the arguments, or $TRON20SRC_ISOS, or isos/).
`write` reads each file from its ranges, and writes the list only if every
file has the size and SHA-256 that build/retail-files.txt gives. `check`
does the same reads and compares the result with the committed list.

The list's lines (the fields are separated by one TAB):

  M <id> <of> <offset> <size> <SHA-256>
      The update's payload: the one folder of the Microsoft cabinet at
      <offset> of the update's executable (<of>: "update"), unpacked
      (MSZIP: deflate blocks, each with the previous one's output as its
      dictionary). The update's InstallShield cabinets are files in it.
  W <path> copy|is <id>:<offset>:<length> ...
      One retail file (its path in build/retail-files.txt), and how to read
      it: those byte ranges, joined, are the file (copy), or the
      InstallShield 6 cabinet data that holds it (is: a run of chunks, each
      a 16-bit length and a raw deflate stream). A file split across two
      cabinets has one range in each. <id> is a disc ("1", "2": the offset
      is in its 2048-byte-sector image, the .iso) or the update's payload.
"""
import hashlib
import os
import struct
import sys
import zlib

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(REPO, "build"))
import discs  # noqa: E402

WAYS = os.path.join(REPO, "build", "retail-ways.txt")
PAYLOAD = "payload"
HEADER = """\
## Where the bytes of each retail file are, in your own disc images and the
## update's executable (build/retailways.py writes and checks this list).
## The install scripts of a public archive read the game's data from these
## byte ranges (build/mkinstall.py). This list holds no game data: only
## offsets, sizes and SHA-256 values.
##
## M <id> <of> <offset> <size> <SHA-256>
##   The update's payload: the one folder of the Microsoft cabinet at
##   <offset> of the update's executable, unpacked (MSZIP).
## W <path> copy|is <id>:<offset>:<length> ...
##   A retail file (build/retail-files.txt), and the byte ranges that are
##   the file (copy), or that hold its InstallShield 6 cabinet data (is: a
##   run of chunks, each a 16-bit length and a raw deflate stream). <id> is
##   a disc (its .iso image) or the update's payload.
## The fields are separated by one TAB.
"""


class Memory:
    """The update's payload, read as an image."""
    def __init__(self, data):
        self.data = data

    def chunks(self, off, n):
        yield self.data[off:off + n]

    def read(self, off, n):
        return self.data[off:off + n]


def payload(exe):
    """(offset of the cabinet, the folder's bytes, {name: offset in it})."""
    base = discs.find_cabinet(exe)
    nfolders, nfiles = struct.unpack_from("<HH", exe, base + 26)
    if nfolders != 1:
        sys.exit(f"retailways: the update's cabinet has {nfolders} folders, not one")
    names = {}
    files_off, = struct.unpack_from("<I", exe, base + 16)
    pos = base + files_off
    for _ in range(nfiles):
        _size, off, _folder = struct.unpack_from("<IIH", exe, pos)
        end = exe.index(b"\0", pos + 16)
        names[exe[pos + 16:end].decode("latin-1").replace("\\", "/").lstrip("/").lower()] = off
        pos = end + 1
    data = b"".join(blob for _n, blob in discs.read_cabinet(exe))
    return base, data, names


class Cabinets:
    """An InstallShield 6 cabinet set: data1.hdr, and where each dataN.cab
    is (its image id and offset there)."""
    def __init__(self, hdr, volumes, read):
        self.volumes, self.read = volumes, read
        sig, ver, _vi, cdo, _cds = struct.unpack_from("<5I", hdr, 0)
        if sig != 0x28635349 or ver >> 24 != 1 or (ver >> 12) & 0xF != 6:
            sys.exit(f"retailways: not an InstallShield 6 header (version {ver:#x})")
        fto, = struct.unpack_from("<I", hdr, cdo + 0x0C)
        ndirs, = struct.unpack_from("<I", hdr, cdo + 0x1C)
        nfiles, fto2 = struct.unpack_from("<II", hdr, cdo + 0x28)
        ft = cdo + fto
        cstr = lambda o: hdr[o:hdr.index(b"\0", o)].decode("latin-1")
        dirs = [cstr(ft + struct.unpack_from("<I", hdr, ft + 4 * i)[0]) for i in range(ndirs)]
        self.files = []
        for i in range(nfiles):
            p = ft + fto2 + i * 0x57
            flags, = struct.unpack_from("<H", hdr, p)
            usize, csize, off = struct.unpack_from("<3Q", hdr, p + 2)
            name_off, d = struct.unpack_from("<IH", hdr, p + 58)
            volume, = struct.unpack_from("<H", hdr, p + 85)
            path = (dirs[d] + "/" if dirs[d] else "") + cstr(ft + name_off)
            self.files.append((i, path.replace("\\", "/"), flags, usize, csize, off, volume))
        self.vh = {}
        for v, (ident, at) in volumes.items():
            h = read(ident, at, 84)
            self.vh[v] = struct.unpack_from("<16I", h, 20)

    def ranges(self, f):
        """[(image id, offset, length)] of file f's stored data."""
        i, _path, flags, usize, csize, off, v = f
        if flags & 2:
            sys.exit(f"retailways: {f[1]} is obfuscated: not known")
        if not flags & 1:                  # in one volume
            ident, at = self.volumes[v]
            return [(ident, at + off, csize)]
        out, left = [], csize
        while left:
            h = self.vh[v]
            ident, at = self.volumes[v]
            if h[3] == i and h[14]:        # the last file of this volume
                o, n = h[10], h[14]
            elif h[2] == i:                # the first file of this volume
                o, n = h[4], h[8]
            else:
                sys.exit(f"retailways: {f[1]}: split, but not at volume {v}'s ends")
            out.append((ident, at + o, n))
            left -= n
            v += 1
        return out


def inflate(img, ranges):
    buf = b""
    gen = (b for ident, off, n in ranges for b in img[ident].chunks(off, n))
    for b in gen:
        buf += b
        while len(buf) >= 2:
            n, = struct.unpack_from("<H", buf)
            if len(buf) < 2 + n:
                break
            yield zlib.decompress(buf[2:2 + n], -15)
            buf = buf[2 + n:]
    if buf:
        sys.exit("retailways: compressed data ends inside a chunk")


def digest(img, kind, ranges):
    h, size = hashlib.sha256(), 0
    gen = inflate(img, ranges) if kind == "is" else \
        (b for ident, off, n in ranges for b in img[ident].chunks(off, n))
    for b in gen:
        h.update(b)
        size += len(b)
    return size, h.hexdigest()


def build(args):
    dlist, update, cabs, files = discs.load()
    found, exe = discs.identify(dlist, update, discs.candidates(args))
    if len(found) < len(dlist) or exe is None:
        print(discs.missing_message(dlist, update, found, exe), file=sys.stderr)
        sys.exit(3)
    img = {str(n): i for n, i in found.items()}
    base, data, names = payload(exe)
    img[PAYLOAD] = Memory(data)
    lines = [f"M\t{PAYLOAD}\tupdate\t{base}\t{len(data)}\t{hashlib.sha256(data).hexdigest()}"]

    def read(ident, off, n):
        return img[ident].read(off, n)

    bycab = {c.name.lower(): c for c in cabs}
    hdr = bycab["data1.hdr"]
    disc_set = Cabinets(read(str(hdr.disc), hdr.off, hdr.size),
                        {int(c.name[4]): (str(c.disc), c.off) for c in cabs
                         if c.name.lower().endswith(".cab")}, read)
    upd_hdr = names["disk1/data1.hdr"]
    upd_set = Cabinets(data[upd_hdr:], {int(n[10]): (PAYLOAD, o) for n, o in names.items()
                                         if n.startswith("disk1/data") and n.endswith(".cab")}, read)

    for f in sorted(files, key=lambda f: f.path.lower()):
        if f.disc:
            ways = [("copy", [(str(f.disc), f.off, f.size)])]
        else:
            cset = disc_set if f.source == "cab" else upd_set
            name = f.gpath.lower()
            ways = []
            for c in cset.files:
                if c[1].lower() == name and c[3] == f.size and not c[2] & 8:
                    ways.append(("is" if c[2] & 4 else "copy", cset.ranges(c)))
        for kind, ranges in ways:
            if digest(img, kind, ranges) == (f.size, f.sha):
                lines.append(f"W\t{f.path}\t{kind}\t" +
                             " ".join(f"{i}:{o}:{n}" for i, o, n in ranges))
                print(f"   {f.path}: {kind}, {len(ranges)} range(s)")
                break
        else:
            sys.exit(f"retailways: {f.path}: no bytes of the images make it")
    return HEADER + "".join(line + "\n" for line in lines)


def main():
    a = sys.argv[1:]
    if a[:1] == ["write"]:
        text = build(a[1:])
        with open(WAYS, "w", newline="\n") as out:
            out.write(text)
        print(f"   wrote {os.path.relpath(WAYS, REPO)}")
        return 0
    if a[:1] == ["check"]:
        text = build(a[1:])
        if open(WAYS).read() != text:
            print("error: build/retail-ways.txt is not what the images give "
                  "(build/retailways.py write)", file=sys.stderr)
            return 1
        print("   build/retail-ways.txt agrees with the images")
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
