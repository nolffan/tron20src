#!/usr/bin/env python3
"""Hide the zeroed game DLL entries of a .rez archive from the engine.

    build/rezhide.py <file.rez>...

ext/tools/retailextract.py writes zeros over each retail binary in our
copies of the game's .rez archives (build/all.sh, step 1). The entries stay
in the archive's directory. The engine finds a file in the LAST archive of
the rez stack that has it. The retail launcher (Tron.exe,
src/Game/Launcher/LauncherDlg.cpp) puts the custom/ directory, where our
game DLLs are, BEFORE the updates' archives (Gamep3.rez, Gamep4.rez: the
registry's UpdateCommandLine0/1). Thus, started from the launcher, the
engine took the zeroed CSHELL.dll of Gamep4.rez and stopped: "Missing
shell DLL cshell.dll". (The game directory now has tron20src.rez in
place of the update's archives: it holds no game DLL. The build still
runs this script on GameDLL.rez, the discs' archive of the game DLLs, so
that no rez order, nor an archive that a player adds, can give the engine
a zeroed DLL.)

This script changes the last letter of the name of each such entry (a
game DLL whose data is all zeros) to "_" in the archive's directory, in
place: CSHELL.dll becomes CSHEL_.dll (and CRES, SRES: CRE_, SRE_, two
names still). No lookup finds the entry then, and
custom/ gives the DLL in every rez order. The archive keeps its size and
its layout. It is idempotent: a renamed entry is not changed again.
"""
import struct
import sys

# The game DLLs that the retail archives hold (build/retail.py,
# REFERENCE_GAME), by name and the type tag (the extension, reversed)
GAME_DLLS = {("CSHELL", b"LLD"), ("OBJECT", b"OTL"), ("CLIENTFX", b"DXF"),
             ("CRES", b"LLD"), ("SRES", b"LLD")}
HDR_TEXT = 127


def entries(f, pos, size):
    """(name, tag, data pos, data size, file offset of the name) of every
    file entry in the directory at pos, recursively."""
    f.seek(pos)
    data = f.read(size)
    off, dirs = 0, []
    while off + 4 <= len(data):
        (etype,) = struct.unpack_from("<I", data, off)
        off += 4
        if etype == 1:
            dpos, dsize, _t = struct.unpack_from("<3I", data, off)
            off += 12
            end = data.index(b"\0", off)
            dirs.append((dpos, dsize))
            off = end + 1
        else:
            fpos, fsize, _t, _id = struct.unpack_from("<4I", data, off)
            off += 16
            tag = data[off:off + 4].rstrip(b"\0")
            off += 4
            (numkeys,) = struct.unpack_from("<I", data, off)
            off += 4
            end = data.index(b"\0", off)
            yield data[off:end].decode("latin-1"), tag, fpos, fsize, pos + off
            off = end + 2 + 4 * numkeys
    for dpos, dsize in dirs:
        yield from entries(f, dpos, dsize)


def hide(path):
    n = 0
    with open(path, "r+b") as f:
        hdr = f.read(HDR_TEXT)
        if b"RezMgr" not in hdr:
            sys.exit(f"rezhide: {path} is not a .rez archive")
        version, rootpos, rootsize = struct.unpack("<3I", f.read(12))
        if version != 1:
            sys.exit(f"rezhide: {path}: REZ version {version}")
        for name, tag, fpos, fsize, noff in list(entries(f, rootpos, rootsize)):
            if (name.upper(), tag.upper()) not in GAME_DLLS:
                continue
            f.seek(fpos)
            if f.read(fsize).strip(b"\0"):
                sys.exit(f"rezhide: {path}: {name} is not zeroed")
            f.seek(noff + len(name) - 1)
            f.write(b"_")
            print(f"   {path}: {name} (zeroed) hidden as {name[:-1]}_")
            n += 1
    return n


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for p in sys.argv[1:]:
        hide(p)
