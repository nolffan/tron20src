# The plan

Tron 2.0 from source, split into packages that can each be picked up again
after an interruption. The first aim is **a game that runs** with every
binary built here; the second is **byte-identity** with retail v1.030 (the
legacy build), ratcheted by `make progress`. The modern build
(`TRON20_MODERN`) comes after.

No source of Tron 2.0 as it shipped was ever released. What there is, and
what it is good for:

| Source | What it is | For |
|---|---|---|
| NOLF2's source release (`../nolf2src/src/Game`, 1.3, 2002) | Monolith's TO2 game tree: ClientShellDLL, ObjectDLL, ClientFxDLL, ClientRes, ServerRes, Launcher, ServerApp, the libraries, VC6 projects | the shared game code, the launcher and the dedicated server as of 2002. Tron 2.0 was built from its own copy of the tree: `Object.lto` names `C:\proj\Tron\Source\Game\ObjectDLL\ObjectShared\Attachments.cpp` |
| Build 69's game code (`rez/installed/devtools/jupiter-b69/Game`, `rez`'s `jupiter-b69`; also GitHub `jsj2008/lithtech`) | the TO2 and Tron 2.0 game code as merged into the 2006 Jupiter tree: `{ClientShellDLL,ObjectDLL,ClientRes,ServerRes,Launcher,Shared}/TRON` (108 files in ClientShellDLL/TRON alone: the disc weapons, the HUD, the ratings, the permissions, `TronMissionButeMgr`, `TronVersionMgr`), with VC6 `.dsp` projects | Tron's own game code. Not the shipped code (the shared code and the interfaces around it are 2006's), but the nearest thing to it: the starting point for Tron's part of CShell, Object, CRes, SRes and the launcher |
| nolf2src's reconstructions (`../nolf2src/src/Engine`) | NOLF2's engine (Jupiter, back-ported from the 2006 tree to 2002, D3D8), SndDrv.dll, ltmsg.dll, all VC6 | the engine's 2002 baseline, with VC6; SndDrv.dll and ltmsg.dll, which Tron's likely differ from mostly by compiler revision |
| The LithTech Jupiter tree (`../fear/Dev/Source/runtime`, and `jupiter-b69/Engine/runtime`) | Jupiter as Touchdown distributed it in 2006 ("Build 69"), D3D9 | the engine: Tron's (2003, and the first of these games to import `d3d9.dll`) lies between NOLF2's and this one; its D3D9 renderer is the nearer base here than nolf2src's D3D8 back-port |
| cjacksrc (`../cjacksrc`) | Contract J.A.C.K. (October 2003), reconstructed from the same sources, with VC .NET 2003 | the shared game code's 2003 state, and its engine, a month from Tron's update. Different compiler: its source carries over, its codegen lore does not |
| Jupiter SDK build 51 (`rez`'s `jupiter-b51`, March 2002) | the licensee SDK of NOLF2's generation: TO2's game tree (`Game/TO2.dsw`) and the engine's SDK headers | the 2002 interfaces, where nolf2src's tree does not settle them |
| The 1.010 release (`rez`'s `tron2-usa`, July 2003) | the discs' own engine and game DLLs (`GameDLL.rez`) | what the updates changed; a second witness for any function |
| `tron-devbuild` (archive.org), and a demo if one is found | a Tron 2.0 developer build of 2003-08-05 with its console (cjacksrc's `docs/PLAN.md`) | strings and asserts from the same months; an unprotected engine, if either has one, for `Lithtech.exe`'s SecuROM pages and redirected imports (as nolf2src's `tools/demoimports.py` did with NOLF2's MP demo). Neither is in `rez/` yet |
| GameSpy SDK (cjacksrc's `src/Game/Libs/GameSpy`; nolfsrc's `lithshared/gamespy`) | GameSpy's C source and Monolith's wrappers of it | the GameSpy SDK linked into `CShell.dll` and `Object.lto` |
| Tron 2.0 Editing Tools (`rez`'s `tron2-tools`, `tron2-tools-1.2a`) | Monolith's tool release: DEdit, ModelEdit, FxED, the packers -- binaries only, no source | `../lithtools`; the game's object properties as the tools see them |

There is no Tron 2.0 PDB or map anywhere that could be found (2026-11).

## The retail game

`rez/games/tron2-usa-1.030` (`build/retail.sh`): the USA discs with the
"complete" Update 2 (`rez`'s `tron2-usa-1.030`), the last update. Tron 2.0
had no GOTY edition, and no European release is held, so the USA one it
is. Update 2 put a new engine, launcher and dedicated server over the
install, and two archives last in the rez stack: `Gamep3.rez` (the English
`CRES.dll`) and `Gamep4.rez` (`CSHELL.dll`, `OBJECT.lto`, `CLIENTFX.fxd`,
`SRES.dll`, and data). The 1.010 game DLLs stay in `GameDLL.rez`, shadowed.

| Binary | What | Size | Functions (own code, enumerated) |
|---|---|---|---|
| `Lithtech.exe` | the engine; SecuROM (`.ifkui`, `.khpb`; five encrypted pages, nearly every import call redirected); D3D9, DInput8, D3DX9 linked in | 2.9 MB | 8,917 |
| `server.dll` | the engine's server | 404 KB | 3,550 |
| `SndDrv.dll` | the sound driver | 64 KB | 288 |
| `ltmsg.dll` | the engine's message strings | 24 KB | resources |
| `Tron.exe` | the launcher (MFC 4.2) | 4.8 MB, nearly all resources | 366 |
| `TRONSrv.exe` | the dedicated server (MFC 4.2, imports `server.dll`) | 856 KB | 612 |
| `CShell.dll` | the game: client, with the GameSpy SDK | 2.4 MB | 9,679 |
| `Object.lto` | the game: server objects, with GameSpy's query reporting | 3.5 MB | 15,814 |
| `ClientFx.fxd` | the game: client effects (static CRT) | 220 KB | 564 |
| `CRes.dll`, `SRes.dll` | the game's resources | 600 KB, 32 KB | resources |

39,790 functions in all. Every binary is VC6, cl and link 12.00.8168:
`progress/COMPILERS.md`. There is no `ServerDir.dll` or `GameSpy.dll`, as
NOLF2 and Contract J.A.C.K. have.

## Ground rules (every package)

- **Source.** Game code starts from NOLF2's source release with Build 69's
  `TRON` directories for Tron's own code; engine code from nolf2src's
  engine and the Jupiter tree (its D3D9 renderer: retail imports
  `d3d9.dll`). Keep Monolith's comments where the code is kept. Where
  nothing exists, write it from the retail disassembly. Mark reconstructed
  retail behaviour that differs from the base `RETAIL:` and cite the retail
  address (`0x...`) in a comment.
- **Compiler.** `msvc-6.00-8168` for everything, as the Rich headers say.
  Prove the flags per binary (`/O1` vs `/O2`, `/Ob2`, `/GX`, the
  precompiled headers), as nolf2src did.
- **Project files.** VC6 `.dsw`/`.dsp` (the format `tools/dsp2mk.py`
  reads), as Monolith's own were.
- **Legacy vs modern.** `TRON20_MODERN` gates every fix and feature, and
  `TRON20_DEBUG` the debug build's logging; the legacy build must keep
  compiling to the retail bytes.
- **Retail bugs.** Every bug found goes into `RETAIL_BUGS.md`, fixed (if at
  all) only behind `TRON20_MODERN`.
- **Commits.** Small commits, each leaving the tree building.

## Packages

| # | Package | Depends on | Status |
|---|---|---|---|
| 1 | `scaffold`: submodules (`rez`, `toolchains`, `tools`), the v1.030 install in rez, the build image, `make retail`/`reference`/`progress`, `targets.py` | -- | done (below) |
| 2 | `build`: `build/env.sh`, `build/build.mk` and `build/compile.sh` (nolf2src's, for `msvc-6.00-8168`), a DirectX 9 SDK in `toolchains/` (the release `Lithtech.exe` links), the workspace `src/Game/TRON.dsw`, `build/all.sh` assembling a game directory, `build/variant.sh` (debug, modern) | 1 | compiling and linking done (`make compile`); the rest to come |
| 3 | `small`: `ltmsg.dll`, `CRes.dll`, `SRes.dll` (resource scripts from the retail resources; `tools/rsrcdiff.py`), `SndDrv.dll` (nolf2src's reconstruction, compiled with 8168) | 2 | `ltmsg.dll` identical; `SndDrv.dll` built, 131 of 284 functions identical |
| 4 | `game`: `CShell.dll`, `Object.lto`, `ClientFx.fxd` -- NOLF2's shared code with Build 69's TRON directories, rolled back to 2003, placed by the scoreboard; the GameSpy SDK's blocks placed as `middleware` | 2 | |
| 5 | `engine`: `Lithtech.exe` and `server.dll` from nolf2src's engine and the Jupiter tree, with the D3D9 renderer; D3DX9 placed as `middleware`; an unprotected engine (the demo, the dev build) for the SecuROM pages and the imports | 2 | |
| 6 | `apps`: the launcher `Tron.exe` and the dedicated server `TRONSrv.exe` from NOLF2's Launcher and ServerApp and Build 69's `Launcher/TRON` | 2 | |
| 7 | `boot`: the built binaries in the game under Wine, to the menu, a level, saving and loading (`make test`, tools/playtest.sh) | 3-6 | |
| 8 | `match`: byte-matching, module by module, ratcheted by `make progress` | 3-6 | |
| 9 | `modern`: the modern build (`TRON20_MODERN`, `MODERN.md`) | 7 | |
| 10 | `release`: `-private`/`-public` archives, install scripts from the player's own discs and Update 2 (`tools/mkinstall.py`), and the GitHub Actions workflow building the public archives with `make progress` on the legacy release | 7, 9 | |

## Notes per package

### 1. scaffold

Done.

- `rez/` installs `tron2-usa-1.030` (`rez/tools/install.py`: the USA
  release, "Recommended", then Update 2's `Common Complete` and `Patch
  Files EN Complete` file groups; `UpdateNum` 2, `UpdateCommandLine0/1`
  `-rez gamep3.rez` / `-rez gamep4.rez`), recorded in `rez/games.sha256`.
- `toolchains/msvc-6.00-8168` is the compiler the Rich headers name.
- `tools/retailextract.py` reads the retail layout from `build/retail.py`.
- `make reference` extracts the 11 retail binaries and enumerates them;
  `make progress` scores 39,790 functions, all unwritten.
- `targets.py`'s `app` boundaries are provisional (the first runtime
  member `libscan.py` finds, less the thunks and `operator delete` before
  it); each is to be pinned from our own `.map` once the binary links, as
  nolf2src did. `Lithtech.exe`'s D3DX9 is counted inside `app` until a
  DirectX 9 SDK is in `toolchains/` to place it with.
- Unknowns: which DirectX 9 SDK; the 0x5f/2067 C objects in
  `Lithtech.exe`; the objects without a `@comp.id` in `Lithtech.exe`,
  `SndDrv.dll` and `TRONSrv.exe`.

### 2. build, and 3. the first binaries

- `build/build.mk`, `env.sh` and `compile.sh` are cjacksrc's (every
  workspace in `WORKSPACES` that is in the tree is built, so the game grows
  one binary at a time) with nolf2src's VC6 toolchain: `msvc-6.00-8168`'s
  compiler, linker, librarian, headers and libraries, SP6's `RC.EXE`, the
  DirectX 8.1 SDK first on the paths. `make compile` runs it; `build/all.sh`
  (a game directory) and `build/variant.sh` are still to come.
- Imported from nolf2src (`48c2506`), its `NOLF2SRC_MODERN` blocks resolved
  out (`tools/releasesrc.py`; only `ltcodes.h` had any): `src/Engine/ltmsg`,
  `src/Engine/snddrv` (with `eaxguid`), `src/libs/ltmem`, the engine SDK's
  headers (`src/Engine/sdk`) and STLport's (`src/libs/STLPort-4.0/stlport`).
- `ltmsg.dll`: NOLF2's resource script gives all five of Tron's resources.
- `SndDrv.dll`: NOLF2's driver gives 131 of Tron's 284 functions. Tron's
  has what the 2006 Jupiter `s_dx8.cpp` added after NOLF2 -- the MP3 codec
  check, the `SoundParamMutex`, a streaming thread run by messages, and 7
  more `ILTSoundSys` slots (`progress/data-known.txt`,
  `progress/vtab-known.txt`) -- and is to be brought there from the 2006
  file (`rez/installed/devtools/jupiter-b69/Engine/runtime/sound/src/sys/s_dx8/`).

### 9. modern

What the modern build is to have, besides the fixes in `RETAIL_BUGS.md`
(`MODERN.md` keeps the inventory as items land):

- widescreen support, with an option to keep 4:3 in the cinematics;
- field-of-view configuration;
- up to 16x anisotropic filtering and up to 8x antialiasing;
- raw mouse input;
- XInput for controllers, in place of DirectInput joysticks;
- open-source replacements for the proprietary libraries: our Bink player
  on FFmpeg in place of RAD's `binkw32.dll`, as nolf2src's;
- the launcher as `TronLauncher.exe`, with `Tron.exe` starting the game
  directly, as if Launch had been clicked.
