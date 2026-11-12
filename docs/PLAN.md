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
| Build 69's game code (`ext/rez/installed/devtools/jupiter-b69/Game`, `rez`'s `jupiter-b69`; also GitHub `jsj2008/lithtech`) | the TO2 and Tron 2.0 game code as merged into the 2006 Jupiter tree: `{ClientShellDLL,ObjectDLL,ClientRes,ServerRes,Launcher,Shared}/TRON` (108 files in ClientShellDLL/TRON alone: the disc weapons, the HUD, the ratings, the permissions, `TronMissionButeMgr`, `TronVersionMgr`), with VC6 `.dsp` projects | Tron's own game code. Not the shipped code (the shared code and the interfaces around it are 2006's), but the nearest thing to it: the starting point for Tron's part of CShell, Object, CRes, SRes and the launcher |
| nolf2src's reconstructions (`../nolf2src/src/Engine`) | NOLF2's engine (Jupiter, back-ported from the 2006 tree to 2002, D3D8), SndDrv.dll, ltmsg.dll, all VC6 | the engine's 2002 baseline, with VC6; SndDrv.dll and ltmsg.dll, which Tron's likely differ from mostly by compiler revision |
| The LithTech Jupiter tree (`../fear/Dev/Source/runtime`, and `jupiter-b69/Engine/runtime`) | Jupiter as Touchdown distributed it in 2006 ("Build 69"), D3D9 | the engine: Tron's (2003, and the first of these games to import `d3d9.dll`) lies between NOLF2's and this one; its D3D9 renderer is the nearer base here than nolf2src's D3D8 back-port |
| cjacksrc (`../cjacksrc`) | Contract J.A.C.K. (October 2003), reconstructed from the same sources, with VC .NET 2003 | the shared game code's 2003 state, and its engine, a month from Tron's update. Different compiler: its source carries over, its codegen lore does not |
| Jupiter SDK build 51 (`rez`'s `jupiter-b51`, March 2002) | the licensee SDK of NOLF2's generation: TO2's game tree (`Game/TO2.dsw`) and the engine's SDK headers | the 2002 interfaces, where nolf2src's tree does not settle them |
| The 1.010 release (`rez`'s `tron2-usa`, July 2003) | the discs' own engine and game DLLs (`GameDLL.rez`) | what the updates changed; a second witness for any function |
| `tron-devbuild` (archive.org), and a demo if one is found | a Tron 2.0 developer build of 2003-08-05 with its console (cjacksrc's `docs/PLAN.md`) | strings and asserts from the same months; an unprotected engine, if either has one, for `Lithtech.exe`'s SecuROM pages and redirected imports (as nolf2src's `tools/demoimports.py` did with NOLF2's MP demo). Neither is in `ext/rez/` yet |
| GameSpy SDK (cjacksrc's `src/Game/Libs/GameSpy`; nolfsrc's `lithshared/gamespy`) | GameSpy's C source and Monolith's wrappers of it | the GameSpy SDK linked into `CShell.dll` and `Object.lto` |
| Tron 2.0 Editing Tools (`rez`'s `tron2-tools`, `tron2-tools-1.2a`) | Monolith's tool release: DEdit, ModelEdit, FxED, the packers -- binaries only, no source | `../lithtools`; the game's object properties as the tools see them |

There is no Tron 2.0 PDB or map anywhere that could be found (2026-11).

## The retail game

`ext/rez/games/tron2-usa-1.030` (`build/retail.sh`): the USA discs with the
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
- **Project files.** VC6 `.dsw`/`.dsp` (the format `ext/tools/dsp2mk.py`
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
| 2 | `build`: `build/env.sh`, `build/build.mk` and `build/compile.sh` (nolf2src's, for `msvc-6.00-8168`), a DirectX 9 SDK in `ext/toolchains/` (the release `Lithtech.exe` links), the workspace `src/Game/TRON.dsw`, `build/all.sh` assembling a game directory, `build/variant.sh` (debug, modern) | 1 | compiling and linking done (`make compile`); the rest to come |
| 3 | `small`: `ltmsg.dll`, `CRes.dll`, `SRes.dll` (resource scripts from the retail resources; `ext/tools/rsrcdiff.py`), `SndDrv.dll` (nolf2src's reconstruction, compiled with 8168) | 2 | `ltmsg.dll` identical; `SndDrv.dll` built, 131 of 284 functions identical |
| 4 | `game`: `CShell.dll`, `Object.lto`, `ClientFx.fxd` -- NOLF2's shared code with Build 69's TRON directories, rolled back to 2003, placed by the scoreboard; the GameSpy SDK's blocks placed as `middleware` | 2 | `CShell.dll`, `Object.lto` and `ClientFx.fxd` build (below) |
| 5 | `engine`: `Lithtech.exe` and `server.dll` from nolf2src's engine and the Jupiter tree, with the D3D9 renderer; D3DX9 placed as `middleware`; an unprotected engine (the demo, the dev build) for the SecuROM pages and the imports | 2 | |
| 6 | `apps`: the launcher `Tron.exe` and the dedicated server `TRONSrv.exe` from NOLF2's Launcher and ServerApp and Build 69's `Launcher/TRON` | 2 | |
| 7 | `boot`: the built binaries in the game under Wine, to the menu, a level, saving and loading (`make test`, ext/tools/playtest.sh) | 3-6 | |
| 8 | `match`: byte-matching, module by module, ratcheted by `make progress` | 3-6 | |
| 9 | `modern`: the modern build (`TRON20_MODERN`, `MODERN.md`) | 7 | |
| 10 | `release`: `-private`/`-public` archives, install scripts from the player's own discs and Update 2 (`ext/tools/mkinstall.py`), and the GitHub Actions workflow building the public archives with `make progress` on the legacy release | 7, 9 | |

## Notes per package

### 1. scaffold

Done.

- `ext/rez/` installs `tron2-usa-1.030` (`ext/rez/tools/install.py`: the USA
  release, "Recommended", then Update 2's `Common Complete` and `Patch
  Files EN Complete` file groups; `UpdateNum` 2, `UpdateCommandLine0/1`
  `-rez gamep3.rez` / `-rez gamep4.rez`), recorded in `ext/rez/games.sha256`.
- `ext/toolchains/msvc-6.00-8168` is the compiler the Rich headers name.
- `ext/tools/retailextract.py` reads the retail layout from `build/retail.py`.
- `make reference` extracts the 11 retail binaries and enumerates them;
  `make progress` scores 39,790 functions, all unwritten.
- `targets.py`'s `app` boundaries are provisional (the first runtime
  member `libscan.py` finds, less the thunks and `operator delete` before
  it); each is to be pinned from our own `.map` once the binary links, as
  nolf2src did. `Lithtech.exe`'s D3DX9 is counted inside `app` until a
  DirectX 9 SDK is in `ext/toolchains/` to place it with.
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
  out (`ext/tools/releasesrc.py`; only `ltcodes.h` had any): `src/Engine/ltmsg`,
  `src/Engine/snddrv` (with `eaxguid`), `src/libs/ltmem`, the engine SDK's
  headers (`src/Engine/sdk`) and STLport's (`src/libs/STLPort-4.0/stlport`).
- `ltmsg.dll`: NOLF2's resource script gives all five of Tron's resources.
- `SndDrv.dll`: NOLF2's driver gives 131 of Tron's 284 functions. Tron's
  has what the 2006 Jupiter `s_dx8.cpp` added after NOLF2 -- the MP3 codec
  check, the `SoundParamMutex`, a streaming thread run by messages, and 7
  more `ILTSoundSys` slots (`progress/data-known.txt`,
  `progress/vtab-known.txt`) -- and is to be brought there from the 2006
  file (`ext/rez/installed/devtools/jupiter-b69/Engine/runtime/sound/src/sys/s_dx8/`).

### 4. game: where Tron's code comes from

What the first builds showed (2026-11-02):

- **Build 69's Tron code is a May 2002 snapshot**, not 2006's: its
  version strings are "TRON Build 20 (05/09/02) DO NOT DISTRIBUTE", and it
  still uses the engine API of Jupiter build 51 (March 2002) in places --
  `ReadByteFL`, `WriteByte`, `SendToClient(msg, MID, client, flags)`, a
  five-parameter `PreHook_PropChanged` -- beside NOLF2's API in others.
  Retail is "v1.030", "Build 74": 17 months on.
- **Tron's shared code is NOLF2's, of late 2002, with Tron's additions.**
  Of the string literals only NOLF2 1.3's `ObjectShared` has (and Build
  51's `ObjectDLL` lacks), 417 are in retail `Object.lto`; what it lacks is
  mostly NOLF2's last multiplayer additions (teams, Doomsday) and debug
  messages. So nolf2src's tree is the base, Build 69's Tron files are
  ported forward to its API (each change marked `PORT:`), and Tron's own
  additions to the shared code -- which neither Build 69 nor NOLF2 has --
  are written from the retail code.
- **Object.lto** compiles and links: Build 69's server files ported to
  NOLF2's message and object APIs (by hand, and by a
  mechanical renaming of the message accessors), `CTronVersionMgr` given
  NOLF2's `GetNetGameName`/`GetBuildGuid` and the shared code's version
  globals (`GAME_NAME` "TRON 2.0" and the GameSpy name "tron20" from
  retail; the build guid, handshake version and net version still to be
  read from the retail code), `CreatePlayer` NOLF2's signature. GameSpy is
  not linked yet (Build 69's project links a `GameSpyMgr.lib` that is not
  in it), and WON's ServerDir headers stand in for the shared shell's
  `iserverdir.h`.
- **CShell.dll** compiles and links. Most of Tron's screens in Build 69
  are build 51's TO2 screens with Tron's changes, so each is a three-way
  merge (NOLF2 1.3's TO2 file, with Tron's changes to build 51's replayed
  on it); where both sides changed the same lines, Tron's side is taken and
  NOLF2's kept beside it as a `// MERGE:` comment, for the matching to
  decide. Files that merged badly are Tron's own, ported by hand
  (HUDCrosshair, ScreenSingle). NOLF2's TO2 files stand in, said so at
  their top, where Tron's retail cannot have had either version: the
  multiplayer screens (TO2's spring 2002, on WON; retail Tron uses GameSpy)
  and ScreenPerformance (build 51's options). What Tron added to the shared
  client code is written in where Tron's files need it, from retail
  evidence where there is any: the defend commands (190, 191, from retail
  autoexec.cfg), build 51's weapon bindings in the profile, the activation
  distance. Still to read from the retail code: GAMEGUID and the build
  guid (zero), the handshake and net versions; GameSpy, for which a stub
  of WON's server-directory factory stands in.

### 4a. Tron's engine SDK

Tron's engine is not NOLF2's at its interfaces. It kept build 51's
(March 2002) ILTClient and ILTServer, with changes of its own, where the
Jupiter mainline -- NOLF2, and the 2006 tree -- reordered them: so every
call the game makes into the engine was at the wrong offset, and would call
the wrong function on Tron's engine. The game code's other interfaces
(messages: ILTMessage_Read/Write) are NOLF2's. So `src/Engine/sdk/inc` keeps
NOLF2's declarations, which the shared code compiles against, in Tron's
order, recovered from the retail binaries and recorded per interface in
`progress/engine/`:

- **Function-pointer members** (ILTClient, ILTServer): retail Lithtech.exe
  and server.dll fill them in runs of stores which line up one for one with
  nolf2src's named assignments (`CLTClient::InitFunctionPointers`,
  `winclientde_impl.cpp`, `serverde_impl.cpp`), whose order Tron's engines
  keep; the offsets the game's near functions call at agree everywhere.
  `*-offsets.csv`.
- **Virtuals** (ILTCSBase, ILTClient, ILTServer, ILTModel, ILTCommon,
  ILTDrawPrim): each slot of Tron's engine vtable is named by the NOLF2
  engine function whose body it has (nolf2src's built engine and map), with
  the game's calls as fixed points. VC6 lays a name's overloads out
  together, in declaration order, so a Tron slot inside an overload block is
  a Tron overload (ILTDrawPrim's DrawPrim at +0x60, SetUVWH at +0xcc).
  Each header is checked against what VC6 compiles from it
  (`out/work/probe.sh`: a member pointer to each virtual, whose vcall thunk
  names its slot). `*-vtable.csv`.
- **The text API** (ILTFontManager, CUIFont, CUIPolyString,
  CUIFormattedPolyString): not NOLF2's at all. Tron's text is wide
  (`std::wstring`, passed by reference to the engine and by value through
  the game's own controls), its fonts are CUIWideBitmapFonts read from a
  font file (CreateFont(ILTStream*); the two other CreateFont overloads are
  stubs), and its polystrings are engine classes whose 43 (45) virtuals the
  game calls directly, with integer positions and sizes. Named from the
  engine's bodies and the game's calls, `*-vtable.csv`. VC6 lays a name's
  overloads out in the reverse of their declaration order (the game's calls
  to CreateFont and SetText show it).
- **Not yet:** NOLF2's 1.x engine additions the shared code still calls
  -- ILTServer::GetSaveFileVersion (GameServerShell, GameBase,
  GameBaseLite, WorldProperties: NOLF2 1.x save formats), ILTModel's OBB
  calls -- which Tron's engine does not have.

### 4b. The game's classes: Tron's vtables

The game is built `/GR`, so every polymorphic class carries RTTI in both
images, and `out/work/rtti_vt.py` pairs our vtables with retail's by type
descriptor (`out/work/class_vt_align.py` then aligns one class's slots by
body). A vtable of the wrong size moves every virtual call past the
difference, so these are fixed first, from the retail evidence:

- **Game types.** Tron has eight (retail's GameTypeToString table):
  Single, SingleLightCycleCircuit, SingleLightCycleCustom, Cooperative,
  Deathmatch, DiscArena, DiscTournament, MultiplayerLightcycles.
- **The HUD.** Every CHUDItem says which game types it belongs to (a pure
  slot 3, one body per set of types), has an enabled flag with a setter
  and getter, and CHUDMgr builds its item list per game type.
- **Build 51's shape where NOLF2 later changed it**: no
  CSpecialFX::OnObjectRotate, no virtual EnablePitch,
  IsTargetGadgetActivatable, CHUDTransmission::Hide, CGadgetTargetFX
  getters, CVersionMgr's five virtuals only, CPlayerStats without energy.
- **Tron's additions**, named where the retail code shows what they are
  (CClientWeapon's fifteen model-key handlers, from its OnModelKey;
  CLTGUICtrl::SetAlpha; CHitBox::SetCanActivate) and otherwise stand-ins
  named `TronUnknownVirtual<slot>`, to be written from the retail code.

Left (CShell, from 133 classes of differing size to 9): CPlayerMgr (14
slots against 47: Tron's CTronPlayerMgr overrides nearly all of them, so it
is to be decompiled, not reshaped), CInterfaceMgr (19 against 42), the disc
weapons' own slots (94 against 105), CWeaponChooser, and the client shell:
Tron's IClientShell order differs from both NOLF2's and build 51's, and is
to be read from the engine's calls into it, as ILTClient's was.
Object.lto: 152 classes still differ.

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
