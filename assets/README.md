# assets/ -- the retail data this repository holds

This repository holds the source code that we wrote. It does not hold the
game. To build or play it, you must have your own copy of Tron 2.0 (USA):
see "What you need" in the top-level `README.md`.

Two sets of retail files are in this repository: `rez/` and `game/`, the
data of Update 2, and `art/`, the art that our binaries are compiled with.
All of it is Monolith's and the publisher's, as the rest of the game's
data is (`docs/LICENSE_NOTES.md`). The files are unchanged:
`.gitattributes` keeps git from changing their line endings. A build with
the retail game checks these directories against it (`make art-check`;
`build/all.sh` step 1b, `build/compile.sh`).

## rez/ and game/: the data of Update 2

These files are what the free Update 2 for *Tron 2.0* adds to the game,
other than code: `tron-update_v1x2_complete.exe`, v1.030, the "complete"
update (it includes Update 1), SHA-256
`66bc39211ded64f9999fd9fee8642d4b01287a6edf5f81c33c511a12ad784e9a`
(32,479,660 bytes), as it was published for download; its archives are
dated October 3, 2003. The release ships them, so that installing the
game needs only the player's two discs.

The update installs nine files. This table gives each one, and what this
repository does with it:

| File (as installed) | Size | SHA-256 | Here |
|---|---|---|---|
| `Gamep3.rez` | 614,602 | `976dae6aae7ee872e3eb24e889cc8900781fbecf06e4c97e6b00f38496f91d70` | nothing: its only entry is `CRES.DLL`, built from source |
| `Gamep4.rez` | 48,728,508 | `bdd03a214403c06d452204c647f7665322c8ffed3cd098a4dbcceb48c69cb496` | its six data entries in `rez/`; its four game DLLs are built from source |
| `Tron Update 2 and Dedicated Server Readme.txt` | 22,373 | `fa28708b0b408bbfba68a4bed904d18425a979573543fb6ff8d58f7ed0e22b99` | in `game/` |
| `Lithtech.exe`, `server.dll`, `SndDrv.dll`, `ltmsg.dll` | | | none: the engine, built from source |
| `Tron.exe`, `TRONSrv.exe` | | | none: the launcher and the dedicated server, built from source |

`rez/` holds the entries of `Gamep4.rez`, at their paths in that
archive, less the four game DLLs (`CSHELL.DLL`, `OBJECT.LTO`,
`CLIENTFX.FXD`, `SRES.DLL`): 6 files, 42,272,198 bytes. Each one replaces
the file of the same path in the discs' archives (`Game.rez`,
`Game2.rez`):

| File | Size | SHA-256 |
|---|---|---|
| `rez/ATTRIBUTES/CLIENTBUTES.TXT` | 27,494 | `f311f7c8a495c085dc62d14cdc67d27824a8f1ebcbe5ed085334a89e24e17ee6` |
| `rez/ATTRIBUTES/LAYOUT.TXT` | 126,065 | `d885fe96a03f5be3ba617ed9e5f97ea2a307757e44ab58bdbefeb21f8d78da43` |
| `rez/WORLDS/RETAILMULTIPLAYER/DISCARENA/DISCARENA02.DAT` | 6,329,058 | `1ec01c20ccdb7cc3c0abb28cc92ca3a54c81a679a4dc8e1d231a136b187caec2` |
| `rez/WORLDS/RETAILMULTIPLAYER/DISCARENA/DISCARENA03.DAT` | 8,521,458 | `2f3802098e0a4686b4d3e7ebca20591f95315524b87b5e1c3c8ec6a78a43d64c` |
| `rez/WORLDS/RETAILSINGLEPLAYER/M02-01.DAT` | 19,896,468 | `2713924df6595d6ca944e20a24bf46d5c252264f9e038d4b2755fe8d7173131a` |
| `rez/WORLDS/RETAILSINGLEPLAYER/M10-03.DAT` | 7,371,655 | `853faae74f004c5a43185b5d205d5b59a8c0ca0f6b6a180d51b86b555be3ff2d` |

`game/` holds the update's readme, which the game directory has beside
the manual. The update changes no setting and no profile: the
`autoexec.cfg` and `Profiles/Default.prf` that the game starts from come
from the discs.

How the build uses them:

- `build/all.sh` step 1b packs `rez/` into the game directory's
  `tron20src.rez` with lithtools' `lithrez` (`build/lithrez.sh`, from the
  `ext/lithtools` submodule). It gives every entry the fixed time
  1065224897 (the time of `Gamep4.rez`) and writes IDs of 0, as the
  retail archives have (`--compat 1.02`). Thus every build makes the same
  bytes. It copies `game/` into the game directory. `MANIFEST.txt` gives
  these files the origin `asset`.
- The game loads `tron20src.rez` where the update's setup put its two
  archives: after the discs' archives, before `custom/`
  (`build/tron20cmds.sh`; the registry values that the legacy launcher
  reads, `UpdateNum 1` and `UpdateCommandLine0 -rez tron20src.rez`, which
  `linux-run.sh`, `linux-server.sh` and `windows-install.bat` write; the
  modern launcher and dedicated server add it themselves, MODERN.md
  APPS-3).
- A build without the retail game (`make NODISCS=1`, the CI build) makes
  the same `tron20src.rez`, and its public archive has it.
- `build/pubcheck.py` lets a public archive hold these files, and only
  if they are exactly the committed ones: `tron20src.rez` must hold the
  files of `rez/`, each byte for byte, and nothing else.

The files were taken out of the update's `Gamep4.rez` and its install
directory, as `build/discs.py` installs it in
`out/retail/tron2-usa-1.030/`. `build/assets.py check` compares each file
of `rez/` with the entry of the same path in the update's archives, makes
sure that no data entry of them is missing, and compares each file of
`game/` with the update's file.

The retail install (`make retail`) still takes the update's executable:
its game DLLs, engine, launcher and dedicated server are the reference
that `make progress` and `make art-check` compare our binaries with. The
game directory takes nothing else from the update.

## art/: the art that our binaries are compiled with

The launcher (`Tron.exe`), the dedicated server (`TRONSrv.exe`) and the
client resources (`CRes.dll`) have art in their resources. Monolith did not
release this art as source. Every build compiles it into these binaries,
so we keep it here, unchanged, as the files that the resource compiler
reads:

| Directory | What | Read by |
|---|---|---|
| `art/Launcher/` | the launcher's backgrounds and buttons (bitmaps), its eight sounds (waves), its icon and its cursor | `src/Game/Launcher/TRON/` |
| `art/ServerApp/` | the dedicated server's splash bitmap and its two icons | `src/Game/ServerApp/` |
| `art/ClientRes/` | the client resources' cursor | `src/Game/ClientRes/TRON/` |

These are 98 files, about 5.7 MB. Each file is a resource of the retail
v1.030 binary, written out as a file: a bitmap with its file header, a
wave as it is, an icon or cursor group as an `.ico` or `.cur` file.
Compiled again, each resource is the retail resource byte for byte.

Thus the binaries are the same with or without the retail game: the CI
build (`make NODISCS=1 compile`) compiles them from these files too.

- `build/compile.sh` copies `art/` into `src/Game/built/retail/`, where
  the resource scripts find it (`build/appres.py install`).
- A build with the retail game first makes sure that `art/` is the retail
  art (`build/appres.py check`). `make art-check` does this check, and
  `build/assets.py check` too.
- `make art` writes `art/` again from the retail binaries
  (`build/appres.py write`).

No other retail data is in this repository. The rest of the game's data,
the movies and the archives come from your discs at build time
(`build/discs.py`). `build/retail-files.txt` and `progress/hashes/` hold
only paths, sizes, offsets and hashes.
