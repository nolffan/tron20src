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
| `src/Game/Libs/GameSpy/GameSpySDK/` | **GameSpy Industries / IGN Entertainment**: the GameSpy SDK | the BSD licence in `GameSpySDK/LICENSE.txt` |

## In a game directory

`MANIFEST.txt` in each game directory gives the origin of each file, and
step 5 of `build/all.sh` (`ext/tools/retailextract.py audit`) gives the
origin of each executable file.

| File | Legacy build | Modern build |
|---|---|---|
| `binkw32.dll` | RAD Game Tools' Bink, from the retail game (`THIRD-PARTY`) | ours: `src/modern/binkw32/`, linked statically with FFmpeg (LGPL-2.1-or-later), built by `build/ext.sh` (`EXT`) |
| `SndDrv.dll` | built here from `src/Engine/snddrv/` | the same, with minimp3 (CC0) compiled in |
| `MFC42.DLL`, `MFC42U.DLL`, `MSVCP60.DLL`, `Msvcrt.dll` | Microsoft's runtime, from the retail game | the same |

The modern build's `binkw32.dll` contains FFmpeg's code under the
LGPL-2.1-or-later. The LGPL lets a person who receives the DLL link it
again against a changed FFmpeg. The source of the player
(`src/modern/binkw32/`), the FFmpeg submodule and `build/ext.sh` let you
do this: change `ext/ffmpeg` and run `make modern`.

The modern build has no file of RAD Game Tools. It also does not need an
MP3 codec of the system (Fraunhofer's `L3CODECA.ACM`, or Wine's): its
`SndDrv.dll` has its own (MODERN.md X-2).
