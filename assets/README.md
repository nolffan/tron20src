# assets/ -- the retail data this repository holds

This repository holds the source code that we wrote. It does not hold the
game. To build or play it, you must have your own copy of Tron 2.0 (USA)
and its Update 2: see "What you need" in the top-level `README.md`.

Only one set of retail files is in this repository: `art/`.

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
  art (`build/appres.py check`). `make art-check` does only this check.
- `make art` writes `art/` again from the retail binaries
  (`build/appres.py write`).

No other retail data is in this repository. The game data, the movies,
the archives and the retail binaries come from your discs and the update
at build time (`build/discs.py`). `build/retail-files.txt` and
`progress/hashes/` hold only paths, sizes, offsets and hashes.
