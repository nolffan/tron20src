# Third-party code and its licences

This file lists the code in this repository and in the builds that is not
Monolith's and was not written for this project, and the terms of each.
It is not legal advice.

## In the source

| Path | Whose | Terms |
|---|---|---|
| `ext/ffmpeg` (submodule) | **FFmpeg**, upstream at github.com/FFmpeg/FFmpeg, n8.1.3 (the commit of nolf2src's and cjacksrc's submodule). Only the modern build uses it: `build/ext.sh` builds the Bink demuxer, the Bink video and audio decoders, the file protocol and the libraries that they need | LGPL-2.1-or-later, as `build/ext.sh` configures it: no GPL part and no non-free part is enabled. See its `LICENSE.md` |
| `src/modern/binkw32/` | written for this project: the modern build's Bink player (the design of nolf2src's and cjacksrc's player) | no licence is declared |
| `src/tron20src/mp3/minimp3/` | **minimp3** (github.com/lieff/minimp3, the copy of cjacksrc's, commit `ea99364f61c1`). The modern build's MP3 decoder, compiled into its `SndDrv.dll`. One change: the `<stdint.h>` types are defined for Visual C++ 6.0, which has no `<stdint.h>` (the change is marked in the file) | CC0 1.0 (a public domain dedication), `minimp3/LICENSE` |
| `src/tron20src/mp3/mp3acm.*`, `mp3acmtest.cpp` | written for this project: the ACM codec on minimp3 (MODERN.md X-2) and its test | no licence is declared |
| `src/modern/directmusic/` | written for this project: `Lithtech.exe.manifest`, the registration-free COM manifest that points the modern game at Microsoft's DirectMusic DLLs (the class list of nolfsrc's and avp2src's manifests), and its `README.md` | no licence is declared |
| `src/Game/Libs/GameSpy/GameSpySDK/` | **GameSpy Industries / IGN Entertainment**: the GameSpy SDK | the BSD licence in `GameSpySDK/LICENSE.txt` |
| `ext/lithtools` (submodule) | the LithTech development tools, rebuilt for these projects. Only its `lithrez` is built here (`build/lithrez.sh`), to pack `assets/rez/` | see `ext/lithtools/README.md` |
| `assets/rez/`, `assets/game/` | **Monolith Productions / Buena Vista Interactive**: the data of the free Update 2 (v1.030, `tron-update_v1x2_complete.exe`), unchanged (`assets/README.md`) | the publishers'; included because the update was distributed freely for anyone to download |

## In a game directory

`MANIFEST.txt` in each game directory gives the origin of each file, and
step 5 of `build/all.sh` (`ext/tools/retailextract.py audit`) gives the
origin of each executable file.

| File | Legacy build | Modern build |
|---|---|---|
| `binkw32.dll` | RAD Game Tools' Bink, from the retail game (`THIRD-PARTY`) | ours: `src/modern/binkw32/`, linked statically with FFmpeg (LGPL-2.1-or-later), built by `build/ext.sh` (`EXT`) |
| `SndDrv.dll` | built here from `src/Engine/snddrv/` | the same, with minimp3 (CC0) compiled in |
| `MFC42.DLL`, `MFC42U.DLL`, `MSVCP60.DLL`, `Msvcrt.dll` | Microsoft's runtime, from the retail game | the same |
| `tron20src.rez`, `Tron Update 2 and Dedicated Server Readme.txt` (`MANIFEST.txt`: `asset`) | Monolith Productions / Buena Vista Interactive: the data of the free Update 2, from `assets/` (`tron20src.rez`: `assets/rez/`, packed by lithrez) | the same |
| `ms_dmband.dll`, `ms_dmcompos.dll`, `ms_dmime.dll`, `ms_dmloader.dll`, `ms_dmscript.dll`, `ms_dmstyle.dll`, `ms_dmsynth.dll`, `ms_dmusic.dll`, `ms_dswave.dll`, `Lithtech.exe.manifest` | none | Microsoft's DirectMusic from DirectX 9.0c (`THIRD-PARTY`, from `ext/toolchains/directmusic-5.3.1.904/`), and our manifest for them (MODERN.md X-3) |

The modern build's `binkw32.dll` contains FFmpeg's code under the
LGPL-2.1-or-later. The LGPL lets a person who receives the DLL link it
again against a changed FFmpeg. The source of the player
(`src/modern/binkw32/`), the FFmpeg submodule and `build/ext.sh` let you
do this: change `ext/ffmpeg` and run `make modern`.

The modern build's DirectMusic DLLs (`ms_dmime.dll` and the others, from
`ext/toolchains/directmusic-5.3.1.904/`, which takes them unchanged from
Microsoft's DirectX End-User Runtime of February 2010) are under the
licence of that redistributable runtime. Check its terms before you pass
on a game directory or a modern public archive that holds them.
`Lithtech.exe.manifest`, which points the game at them, was written for
this project.

The modern build has no file of RAD Game Tools. It also does not need an
MP3 codec of the system (Fraunhofer's `L3CODECA.ACM`, or Wine's): its
`SndDrv.dll` has its own (MODERN.md X-2).

## In a public archive

A public archive (`out/<name>-public.tar.gz`, `build/package.sh`) holds
the files that this project builds, and of the retail game only the data
of the free Update 2 (`tron20src.rez` and the update's readme, the
`asset` rows of `MANIFEST.txt`: `assets/README.md`). `build/pubcheck.py`
fails an archive that holds any other file of the retail game, or an
`asset` file that is not exactly what `assets/` gives. It also holds this
file, and in `licenses/`:

| File | The licence of |
|---|---|
| `licenses/GameSpy-SDK.txt` | the GameSpy SDK, linked into `CShell.dll` and `Object.lto` (each build) |
| `licenses/FFmpeg-COPYING.LGPLv2.1` | FFmpeg, linked statically into the modern build's `binkw32.dll` (modern builds only) |
| `licenses/minimp3-LICENSE.txt` | minimp3, compiled into the modern build's `SndDrv.dll` (modern builds only) |

A modern public archive also holds Microsoft's nine DirectMusic DLLs
(`ms_<name>.dll`) and `Lithtech.exe.manifest`, as the modern game
directory does (above): its `MANIFEST.txt` gives their origin.

The install scripts of the archive copy the rest of the retail game's
data, RAD's `binkw32.dll` (legacy builds) and the Microsoft runtime from
the player's own two discs. The archive does not hold them.
