# The plan

Tron 2.0 from source, split into packages that can each be picked up again
after an interruption. The first aim is **a game that runs** with every
binary built here; the second is **byte-identity** with retail v1.030 (the
legacy build), ratcheted by `make progress`. The modern build
(`TRON20SRC_MODERN`) comes after.

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
| `tron-devbuild`, and a demo if one is found | a Tron 2.0 developer build of 2003-08-05 with its console (cjacksrc's `docs/PLAN.md`) | strings and asserts from the same months; an unprotected engine, if either has one, for `Lithtech.exe`'s SecuROM pages and redirected imports (as nolf2src's `tools/demoimports.py` did with NOLF2's MP demo). Neither is held yet |
| GameSpy SDK (cjacksrc's `src/Game/Libs/GameSpy`; nolfsrc's `lithshared/gamespy`) | GameSpy's C source and Monolith's wrappers of it | the GameSpy SDK linked into `CShell.dll` and `Object.lto` |
| Tron 2.0 Editing Tools (`rez`'s `tron2-tools`, `tron2-tools-1.2a`) | Monolith's tool release: DEdit, ModelEdit, FxED, the packers -- binaries only, no source | `../lithtools`; the game's object properties as the tools see them |

There is no Tron 2.0 PDB or map anywhere that could be found (2026-11).

## The retail game

`out/retail/tron2-usa-1.030` (`build/retail.sh`): the USA discs with the
"complete" Update 2, the last update, installed by `build/discs.py` from
your own disc images and the update's executable. Tron 2.0
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
- **Legacy vs modern.** `TRON20SRC_MODERN` gates every fix and feature, and
  `TRON20SRC_DEBUG` the debug build's logging; the legacy build must keep
  compiling to the retail bytes.
- **Retail bugs.** Every bug found goes into `RETAIL_BUGS.md`, fixed (if at
  all) only behind `TRON20SRC_MODERN`.
- **Commits.** Small commits, each leaving the tree building.

## Packages

| # | Package | Depends on | Status |
|---|---|---|---|
| 1 | `scaffold`: submodules (`rez`, `toolchains`, `tools`), the v1.030 install in rez, the build image, `make retail`/`reference`/`progress`, `targets.py` | -- | done (below) |
| 2 | `build`: `build/env.sh`, `build/build.mk` and `build/compile.sh` (nolf2src's, for `msvc-6.00-8168`), a DirectX 9 SDK in `ext/toolchains/` (the release `Lithtech.exe` links), the workspace `src/Game/TRON.dsw`, `build/all.sh` assembling a game directory, `build/variant.sh` (debug, modern) | 1 | compiling and linking done (`make compile`); the game directory done (`make`, `build/all.sh`); the variants done (`make debug`, `make modern`, `make modern-debug`: `build/variant.sh`) |
| 3 | `small`: `ltmsg.dll`, `CRes.dll`, `SRes.dll` (resource scripts from the retail resources; `ext/tools/rsrcdiff.py`), `SndDrv.dll` (nolf2src's reconstruction, compiled with 8168) | 2 | `ltmsg.dll` identical; `SndDrv.dll` Tron's: all 232 functions of its own code identical (34 initializer-sized ones `small`) |
| 4 | `game`: `CShell.dll`, `Object.lto`, `ClientFx.fxd` -- NOLF2's shared code with Build 69's TRON directories, rolled back to 2003, placed by the scoreboard; the GameSpy SDK's blocks placed as `middleware` | 2 | `CShell.dll`, `Object.lto` and `ClientFx.fxd` build (below) |
| 5 | `engine`: `Lithtech.exe` and `server.dll` from nolf2src's engine and the Jupiter tree, with the D3D9 renderer; D3DX9 placed as `middleware`; an unprotected engine (the demo, the dev build) for the SecuROM pages and the imports | 2 | |
| 6 | `apps`: the launcher `Tron.exe` and the dedicated server `TRONSrv.exe` from NOLF2's Launcher and ServerApp and Build 69's `Launcher/TRON` | 2 | |
| 7 | `boot`: the built binaries in the game under Wine, to the menu, a level, saving and loading (`make test`, ext/tools/playtest.sh) | 3-6 | legacy release: the menus, the first level, saving and loading, a light-cycle race (`make tests`, below) |
| 8 | `match`: byte-matching, module by module, ratcheted by `make progress` | 3-6 | |
| 9 | `modern`: the modern build (`TRON20SRC_MODERN`, `MODERN.md`) | 7 | done under Wine, except the music: every item of the list below is in `MODERN.md` with its tests; Microsoft's DirectMusic is in the game directory, but Tron 2.0's music stays silent under Wine (X-3, open). To check on real hardware: antialiasing and anisotropic filtering on a graphics card, a real controller, Windows itself, Internet play on OpenSpy's public servers |
| 10 | `release`: `-private`/`-public` archives, install scripts from the player's own discs and Update 2 (`build/mkinstall.py`), and the GitHub Actions workflow building the public archives with `make progress` on the legacy release | 7, 9 | done (below): the archives of the four variants, the install scripts (tested from the disc images for legacy and modern), the CI with the nightly and tagged releases |

## Notes per package

### 1. scaffold

Done.

- `build/discs.py` installs `tron2-usa-1.030` from your disc images and the
  update (the USA release, "Recommended", then Update 2's `Common Complete`
  and `Patch Files EN Complete` file groups), as `build/retail-files.txt`
  lists it. (Until 2026-11 the `rez` submodule installed it.)
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
  makes the game directory, and `build/variant.sh` the debug and modern
  builds, each in its own tree (`out/build-<variant>/`).
- Imported from nolf2src (`48c2506`), its `NOLF2SRC_MODERN` blocks resolved
  out (`ext/tools/releasesrc.py`; only `ltcodes.h` had any): `src/Engine/ltmsg`,
  `src/Engine/snddrv` (with `eaxguid`), `src/libs/ltmem`, the engine SDK's
  headers (`src/Engine/sdk`) and STLport's (`src/libs/STLPort-4.0/stlport`).
- `ltmsg.dll`: NOLF2's resource script gives all five of Tron's resources.
- `SndDrv.dll`: NOLF2's driver gave 121 of Tron's functions; brought to
  Tron's (`src/Engine/snddrv`), all of its own code is identical (232, the
  34 initializer-sized ones `small`). Tron's additions, none of them in
  Build 69's `s_dx8.cpp` but the codec message: EAX 3.0 (the EAX 2.0
  functions hand over to it), 16 parameter fades of a sample's frequency
  and the thread that steps them (`SoundParamMutex`, a `WM_QUIT`-ended
  message loop, `SuspendThread`/`ResumeThread`), ten more `ILTSoundSys`
  slots, the counted sound thread, the missing-codec message box in the
  Final build. Tron's `ltmem.lib` is `/O2` without `/Ob2` (its
  initializers are jump thunks, in `Lithtech.exe` and `server.dll` too).
  The Rich header's one cl 13.00.9178 object is the DirectX 8.1 SDK's
  `dxguid.lib` (`dxguid.obj`), linked, not compiled. Left: the image's data
  layout (ltmem.obj's `.bss` has 0x1c bytes more, unreferenced, after
  `g_LTMemInitialize`; MSACM32's and MSVCRT's imports are in another
  order), which no function's code shows.

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

### 4c. Inline helpers called out of line: the inline budget

Many nears call a small helper (TVector3's constructor or Init,
CAutoMessage's Init or Term, a string's clear(), _M_range_initialize, an
allocator proxy) where ours expands it, or the reverse. Measured with cl
12.00.8168 /O2 on small test files and on game functions (2026-11-24):

- VC6 gives each function an inline budget. The budget is approximately
  the larger of a fixed amount (about 100 call statements of growth) and
  1.2 times the size of the function before inlining. Dead code in the
  caller (code that the optimizer removes) makes the budget larger.
- Each expansion spends budget at the size of the inline body before
  optimization. Dead code in an inline body (`if (x) {}`, `do {} while
  (0)`, `((void)(x))`, unused locals) spends budget although it makes no
  code. `((void)0)` (our ASSERT) and an empty statement spend nothing.
- Direct calls expand in source order. When a body does not fit, the call
  stays a call, and a smaller body after it can still fit. Calls inside an
  expanded body (depth 2 and more) expand after that, from the end of the
  function to its start: the last sites get the full expansion and the
  first sites keep the inner call (MultiplayerLevelInfoMgr::ReadDefaults,
  0x1002f250: the first std::string calls _M_range_initialize, the second
  expands it).
- `__forceinline` always expands and spends no budget.
- These do not change the result: the order of functions in the file,
  the precompiled header (/Yu or none), unused inline functions, /Ob2,
  /Os, /Ot, /G5, /G6, /Gy, /GF, and cl 8168, 8447, 8966 and 9044.
  `#pragma inline_depth(n)` only limits the depth, and the last value in
  the file applies to all functions in the file, also the functions above
  it.

Proof on game code: a dead inline body put first in the function (a
probe, not kept) makes these identical: TronTurret::HandleAllObjectsCreated
(0x1006b560: the last clear() becomes a call), CoreDump::CreateFX
(0x10018d70) and CCharacterFX::CreateChatIcon (CShell 0x10179bb0: one
more TVector3::Init of ObjectCreateStruct::Clear becomes a call).

Thus a helper that retail calls and we expand tells you that retail spent
more budget before that site, or that the retail function was smaller;
the reverse tells you the opposite. Most often the cause is a different
statement in the same function. Do not change the helper (`inline`, out of
line, or its header): that changes all callers. Do not keep probes or
dead code to hit a budget. Two project-wide probes failed: dead code in
ObjectCreateStruct's constructor (Object -52, +0) and an ASSERT that spends
budget (Object -137, +2: only two CAutoMessage functions became
identical).

### 4d. Small forms that change the code

The source permuter (ext/tools/permuter.py, guides/22_permuter.md) finds
these forms. Each one is a test result. Use them as tests, not as rules.

- A double argument to a float parameter: VC6 narrows the value before it
  loads the vtable only when the argument has its own parentheses.
  `SAVE_FLOAT( (m_fValue) )` narrows first. `SAVE_FLOAT( m_fValue )` and
  `SAVE_FLOAT( (LTFLOAT)m_fValue )` narrow after (CAIGoalDash::Save,
  Object 0x1020bf10).
- `if( 0 == ( a & b ) )` and `if( !( a & b ) )` can give different code
  (CCharacter::ShouldGib, Object 0x10091370).
- A member copied into a local before a call can change the load order of
  the arguments (AINodeExitLevelByRezOut::ProcessOnUseCmd, Object
  0x1026ea10).

### 5. engine (Lithtech.exe; first steps)

- **Base:** cjacksrc's engine (`src/Engine/runtime`), compiled with VC6
  8168 and STLport (retail's engine RTTI is `_STL`'s; D3DX9's VC7 `std::`
  classes sit beside it) and NOLF2's VC6 flags: Lithtech `/O2 /Ob2`, no
  `/GX`; D3DRender `/GX /O2 /Ob2`, Lib_UI and Lib_ILTSound `/GX /O2`
  (retail's EH frames all lie above 0x4c0000; Lib_UI without `/GX` loses
  the wide font's destructor frame and 14 functions; with `/Ob2` its
  static initializers' `jmp` stubs fold away: 178 more functions identical
  without it).
  The engine's StdLith is `/O2` without `/Ob2` too (sb_Term calls
  sb_Term2, 0x561b90), the game DLLs' `/Ob2`: the engine's workspace builds
  it as `src/libs/stdlith/Lib_StdLithEngine.dsp` (ours; the library keeps
  its name) for Lithtech.exe and server.dll.
- **Rich header:** 292 C++ objects of 8168 (ours); D3DX9's are the 110
  C++ and 3 C objects of cl 13.00.9178, 6 of masm 6.14.8444 and 9 C
  objects of cl 13.10.2067 (the developer build of 2003-08-05 has the
  same D3DX rows, with its own code at 8966): a DirectX 9 SDK of early
  2003 (9.0 or 9.0a, before 9.0b's August release: to be told by matching
  its `d3dx9.lib` against retail). `ext/toolchains/` has none:
  `build/build.mk` takes it as `DXSDK9` (default `dxsdk-9.0`) for the
  engine's projects only. Its `d3d9.h` must give D3D_SDK_VERSION as
  retail passes it.
- **SecuROM:** `ext/tools/demoimports.py` profile `tron2`, keyed by the
  unprotected developer build (`rez/installed/devtools/tron2-devbuild`):
  1,463 redirected calls (1,372 through 24 `.khpb` slots, 91 through
  three `.ifkui` dispatchers), 1,336 paired exactly, 89 by context, 38
  weakly (`progress/engine/Lithtech-iat-calls.csv`, the target's
  `iat_calls`). The original import directory is at 0x5a9428 (WINMM's and
  DINPUT8's descriptors first). `plain` fills 0x572000..0x577000 from the
  developer build at shifts -0x5fb0..-0x600a.
- **The SDK's unknowns, named from retail:** ILTCSBase slot 48
  (GetObjectUnguaranteedData), ILTServer's after SetNetFlags
  (SetObjectUnguaranteedData), ILTModel slots 6 and 8 (GetSocketParentNode,
  GetNumSockets), ILTDrawPrim's Build 69 SaveViewport/RestoreViewport,
  SetFogEnable, DrawPrim(LT_POLYGT4 **), SetUVWH(.., HTEXTURE, ..).
  ILTCSBase 44/45 are the game clock (client: the shell's game time).
- **The text API** (Tron's wide fonts and polystrings; package 4a) is the
  engine's own: CUIFont_Impl and CUIWideBitmapFont are written; the
  polystring (0x562ba0.., 0x84 bytes: a std::wstring, two STLport maps at
  +0x60/+0x6c), the formatted polystring (0x568550.., derived from it),
  the font manager (0x566200..0x5664a5), the console's text and the
  widgets that draw text are what keep Lithtech from compiling.
- **The renderer** is Direct3D 9 (DirectX 9.0 SDK, `ext/toolchains/
  dxsdk-9.0`): Build 69's CDirect3DDevice9Wrapper (its render state
  tables are retail's, 0x5c204c/0x5c2120), its shell (modes by known
  format, 0x5c236c), device (end-of-frame query at +0x188,
  PreventFrameBuffering; not its frame textures or render targets),
  sampler states, SetFVF, depth bias and stream offsets on cjacksrc's
  renderer; DrawPrim, the render struct and Bink likewise. No effects,
  vertex shader manager or render targets in Tron (no strings); the pixel
  shader manager is NOLF2's on IDirect3DPixelShader9 (0x512fc0..). D3DX9
  is middleware (libscan.py: 0x4b59b0..0x4c8060, 0x574678..0x581b9b).
  What is left is not Direct3D's: Contract J.A.C.K.'s world-model render
  block batching (CRenderShader_Base's m_nFirstQueuedSection/
  m_nLastQueuedSection, CInternalSection's m_pWorld/m_mWorld/
  m_bBreakBatch, AddSection's world, GetShaderDrawRange) is not in Tron:
  its shader members sit 8 bytes lower (nearpos.py: every Pre/PostFlush
  and FlushChangeSection off by 8) and CD3D_RenderWorld is 0x14c bytes,
  not 0x248 (LoadWorldData, 0x4cae80). Build 69's shader base without
  its effect code is the model. client/src/sys/win/winconsole_impl.cpp
  still names D3DVIEWPORT8 (four lines).

### 7. boot

The legacy release build runs under Wine (2026-11-25): every binary in
`out/tron20-legacy-release/` is ours, except the third-party runtime.

- **The game directory** (`make`, `build/all.sh`, the steps of
  cjacksrc's): the `.rez` archives and movies of the retail game as hard
  links (about 70 MB of copies: `GameDLL.rez`, `Gamep3.rez` and
  `Gamep4.rez`, with the retail game DLLs in them set to zeros), Bink and
  the Microsoft runtime from the retail game, and the eleven binaries we
  build. The game DLLs go in `custom/`, which is last in the rez stack
  (`tron20cmds.txt`, `build/tron20cmds.sh`). The retail launcher puts
  `custom` before the updates' `gamep3.rez` and `gamep4.rez`, so there
  the zeroed entries would hide ours. An audit (`ext/tools/retailextract.py
  audit`) lists every PE file with its origin and stops the build on any
  other. `SRes.dll` is built now (`ServerRes.dsp`, Final): retail's has no
  resources, only `DllMain`.
- **The tests** (`make test`: the main menu; `make tests`: each
  `build/playtest-<name>.txt`, each to end ALIVE): `menus` (the main menu
  and the screens it opens), `level` (the first level, M01-01: into
  control, moves, the disc), `save` (F5 quicksave, F9 quickload, a save
  and a load through the in-game menu) and `race` (Light cycles, Circuit
  play, "Newbie Authentication": turns, a derez, Continue). The runner is
  `ext/tools/playtest.sh` with Tron's names (`build/playtest.sh`); its
  `skipcine` waits for Tron's health bar (green, lower left). In M01-01 a
  long walk falls off the start platform and kills the player, in retail
  too: the scripts move in short steps.
- **The control** (`make control`, `out/tron20-control/`): the retail game
  DLLs with our engine. Retail's `Lithtech.exe` does not run here
  (SecuROM). A test that passes on the control and fails on our build
  points at our game code.
- **Fixed:** the quickload crashed in `CHUDSingleString::Update` (CShell
  0x1003ae50): the HUD item had no polystring after a load. Retail's
  `CHUDSingleString` has its own `Init` (vtable slot 1, 0x101613a0, the
  body that `CHUDProcedurals::Init` has: `UpdateLayout(); return TRUE;`),
  which `CTronHUDMgr::AddItems` calls each time a world is entered after
  `OnExitWorld` terminated the items. Ours had none. Found with the
  control: winedbg, attached to it, with breakpoints on retail's `Term`
  and `UpdateLayout`, showed which function calls `UpdateLayout`. CShell
  7,599 identical functions before, 7,601 after.
- **Fixed:** after a quickload or a load from the menu, ours went into
  the game at once; retail shows the postload screen ("Press any key to
  continue"). `CScreenPostload::OnFocus` (0x1016b4e0) was NOLF2's; it is
  now retail's (identical), and `build/playtest-save.txt` checks it
  (`nohud`). The engine prints "LithTech build 4294967295.4294967295",
  as retail does: retail's `Lithtech.exe` has no build-number resource
  (`RETAIL_BUGS.md`).
- **The launcher** (`make launchertest`, `build/launchertest.sh`,
  `build/playtest-launcher*.txt`): our `Tron.exe` on the registry values
  of the game's setup and Update 2, with disc 1 in a CD-ROM drive (Play
  looks for `\TRON.EXE` on it, as retail's does). Display, Options (with
  their first-time warnings), the detail setting, Play to the main menu,
  Uninstall (no uninstaller: a message), Help (`TRON.chm`) and Quit work.
  The command line in `launchcmds.txt` and the values that the launcher
  stores (`HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0`: the
  options, `DisplayWarning`, `OptionsWarning`, `Num Launcher Runs`) are
  the same as the retail launcher's (`LAUNCHERTEST_RETAIL=1`). It writes
  nothing to APPDATA. The launcher exits when the game starts, as
  retail's does. The retail launcher puts `custom/` before `Gamep3.rez`
  and `Gamep4.rez`: thus their zeroed game DLLs came first ("Missing
  shell DLL cshell.dll"), until `build/rezhide.py` changed their names.
- **Multiplayer** (`make mptest`, `build/mptest.sh`): the dedicated server
  (`TRONSrv.exe`) or a listen server (client 1 hosts from the multiplayer
  menu), and two clients on the loopback, each with its own Xvfb display
  and Wine prefix. Disc Arena, Disc Tournament and Light Cycles: the
  clients join, see each other, choose a team (F1), and play: in Disc
  Arena a match starts, a player is derezzed and the scores change; in
  Light Cycles the races run and the points change. The command line's
  `+host <world>` stops at the loading screen, with the retail game DLLs
  too (the control). LAN only: a game that is not reports to
  `tron20.master.gamespy.com` (UDP 27900; with `gcdkey`, the CD key
  check), and the client's Internet list asks `tron20.ms<n>.gamespy.com`
  (TCP 28910) and `motd.gamespy.com` (HTTP, the version check): the
  modern build's OpenSpy work.
- **Fixed:** in Disc Arena a player in no match saw "Waiting for next
  point." for "Press F1 to join a team." (`CTronInterfaceMgr::
  TronUnknownVirtual7C`, 0x1009d7a0: the case labels of states 2 and 3
  were swapped). Its code was byte-identical modulo relocations, which
  does not compare a jump table: a check of the jump tables of all the
  identical functions with one (106 in CShell.dll, 185 in Object.lto)
  found two more in CShell.dll, `CRatingMgr::GetTransferRate`
  (0x10051710, a retail bug: RETAIL_BUGS.md) and
  `CScreenSubroutines::PickupSubroutine` (0x100803b0), now fixed, and
  one in Object.lto, `CCharacter::GetPriorityBodyState` (0x100914e0),
  which waits on Tron's AI state enum.
- **Open (multiplayer):** our `TRONSrv.exe` with the retail `Object.lto`
  stops after "Server initialized." (the splash screen stays); with ours,
  and the retail `TRONSrv.exe` with our `Object.lto`, it runs.
- **Fixed:** in Light Cycles, a player who watches the game (the
  spectator) fell to the floor of the arena: thus the camera was on the
  floor and the grid showed as white bands. The control's camera stays
  where the player starts. `CMoveMgr::UpdateMotion` (0x100f6ac0) was
  NOLF2's; Tron's has a light cycle part: the spectator has no gravity
  and flies with the camera (the move keys), and a player who is not on a
  light cycle stands still. It is now retail's (identical). Found with the
  control and a control with only our `CShell.dll` in `custom/`: the
  second looked as ours did. In single player the race does not show it.
- **Fixed:** the HUD's subroutine icons (`CHUDSubChooser::Arrange`,
  0x1003bad0) were in a row; retail's are in a column, from the
  procedurals down to the armor. `Arrange` starts a new column when the
  next icon goes below the armor item's top, which is `CHUDArmor`'s +0x14.
  Retail's `CHUDArmor::UpdateLayout` (0x100221b0) puts the item's y there;
  ours put the X ratio (1.0), so each icon started a new column. This
  was in single player too.
- **The world sweep** (`make sweep`, `build/sweep.sh`; not part of
  `make tests`, about 40 minutes with `SWEEP_JOBS=5`): all 55
  single-player worlds of the retail game (`build/sweep-worlds.txt`): the
  38 worlds of the campaign (`ATTRIBUTES\MISSIONS.txt`) and the 17
  light-cycle arenas (`ATTRIBUTES\CIRCUITS.txt`). Each world plays for
  about 60 s in its own container, with a script for its kind: `sp` (a
  level), `sub` (the first level of a mission: a cinematic, then the
  subroutine screen, which Escape closes), `cine`, `lc` and `splc` (a
  light-cycle level of the campaign). The summary is
  `out/play/sweep-summary.txt`; the last shots are in
  `out/play/sweep-sheet.png`. `SWEEP_WORLDS=<text>` plays only the worlds
  whose path has the text; `TEST_GAME=out/tron20-control` plays the
  control. On 2026-11-25 all 55 worlds end ALIVE and draw correctly. One
  run of M09-01 ended NO HUD: the HUD came up only after the 120 s of
  `skipcine` (the opening is long); the shots are correct, and the next
  run ended ALIVE. In
  M05-01, M08-01, M09-02 and M10-01 the AI kills the idle player; the
  control does the same (M04-03 and M05-01 there). An idle light cycle
  hits a wall, and the game asks "Retry light cycle?": that is correct.
- **Gameplay, against the control** (scripts in `out/work/tc/`, not
  kept): combat (M10-01: the AI locks on, moves, fires and kills the
  player; `mpgod` stops it; the disc and, after `mpguns`, the rod and the
  ball primitive fire), cheats through the chat (`mpgod`, `mpguns`,
  `mppos`, `mpmaphole`), the weapon chooser, the level transition
  (`mpmaphole` in M01-02 loads M02-01), death (M01-01: a walk off the
  platform; "END OF LINE.", the load screen) and F9 reload, the
  subroutine screen (F1, with the performance ratings), and the options
  (display, sound, controls, game, performance): a changed setting stays
  when the screen opens again, and the game writes it to
  `Profiles/Player.txt` next to the executable. Not checked by a script:
  a pickup by walking to it, a subroutine put into a slot, and an enemy
  that dies (the scripts cannot aim; the control kills enemies in the
  same script, ours shows hits only).
- **Fixed** (found by the sweep and the gameplay scripts): the weapon
  chooser drew the selected weapon as a white square and the others too
  bright: `CHUDWpnChooser::Init` (0x10022c50) loaded
  `WEAPON::GetDisabledIcon`, which has no texture, where retail loads
  `GetNormalIcon`, and `GetNormalIcon` where retail loads
  `GetUnselectedIcon`. The death screen did not show "END OF LINE.":
  `CScreenFailure::Build` (0x1005ac20) made its polystring centred, and
  `OnFocus` centres it again; retail makes it `CUI_HALIGN_LEFT`. `Build`
  is now identical: CShell 7,601 identical functions before, 7,602
  after. After each single-player load, the postload screen now asks for
  a key, as retail does (`CScreenPostload::OnFocus`, 0x1016b4e0).
- **Open:** in M01-02, the control shows a small blue machine that moves
  on the floor near the start, in the first seconds; ours does not show
  it. Not found yet: the world has 34 `CAIHuman` and 36 `Spawner`
  objects.

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
  directly, as if Launch had been clicked;
- a launcher that does not exit after it starts the game (this keeps the
  Steam overlay and Steam Remote Play working);
- modern resolutions, and a borderless-window fullscreen mode;
- unlocked frame rates, with limits of 60, 120, 144 and 240 and no limit.
  The frame rate must not change physics or animation, and this applies to
  multiplayer too;
- multiplayer restored through a GameSpy replacement such as OpenSpy;
- 32-bit colour as the default, in place of 16-bit;
- settings and saved games beside the executable. The launcher and the
  game must not write to APPDATA or to the registry (that data is lost
  when a Proton version changes).

Phase 0 (2026-11-25): `build/variant.sh` makes `make debug`, `make
modern` and `make modern-debug`. The modern build compiles with all the
`TRON20SRC_MODERN` blocks: the retail-bug fixes of the game DLLs (GAME-1
to GAME-33), the launcher split (APPS-1), and the engine's modern code
from cjacksrc's engine (EN- items; its XInput headers are now in
`src/tron20src/xinput/`). The three variants pass `make tests`, and the
modern build passes `make launchertest` both as `TronLauncher.exe` and as
`Tron.exe`. `MODERN.md` lists every item with its id, and the Phase 1
items with the files that each agent owns.

Phase 1 (2026-11-25): every item of the list above is done and tested
under Wine and Xvfb (`MODERN.md`: Apps, Middleware, Display, Frame rate,
Input, Online), and every engine item that came from cjacksrc's engine is
checked against Tron 2.0's retail `Lithtech.exe` ("Inherited engine
items"). The modern build passes `make tests` (and, in each section's tests,
`make launchertest`, `make portabletest` and `make mptest`), and `make
package` gives its public archive (with Microsoft's DirectMusic), which
`build/pubcheck.py` accepts. What is open, and what Wine and
Xvfb cannot show:

- the music under Wine and Proton (X-3): Microsoft's DirectMusic needs a
  DirectSound with `IDirectSoundConnect` for Tron 2.0's audio paths, and
  Wine's has none; on Windows the music plays as in retail;
- antialiasing and anisotropic filtering on a real graphics card (under
  Wine the device log is the check, CL-34);
- a real controller (the tests use the test pad, CL-20 to CL-25);
- Windows itself (V-sync in a window, the DirectMusic manifest, the raw
  mouse);
- Internet play through OpenSpy's public servers (the tests use a local
  master server; the list request to OpenSpy was read only, GS-1 to GS-4).

Before this package: package 7. The modern build changes only code under
`TRON20SRC_MODERN`, and the release tree removes those blocks, so the
legacy build and its score do not change (`make ci` checks this).

### 10. release

Done 2026-11-25. Each build packages its game directory last
(`build/package.sh`): `out/<name>-private.tar.gz`, the whole directory,
and `out/<name>-public.tar.gz`, without a file of the retail game.
`README.md` ("The archives: private and public", "Releases") tells how to
use them.

- **The install scripts** (`build/mkinstall.py`, the code in
  `build/game-scripts/`, from nolf2src's): `linux-install.sh` (bash and
  python3) and `windows-install.bat` (PowerShell 5.1). They read the two
  disc images (`.iso`, `.bin`/`.cue`, `.zip`) and Update 2's executable
  (or a `.zip` of it). The files of the discs are in InstallShield 6
  cabinets, and those of Update 2 are in InstallShield 6 cabinets in a
  Microsoft cabinet (MSZIP) in the executable. Thus the scripts get byte
  ranges and inflate them: they use no unshield. `build/retailways.py`
  finds the ranges from the cabinets' headers and checks each file by its
  SHA-256 (`build/retail-ways.txt`; `make retail-ways`). The build's
  changes to `GameDLL.rez`, `Gamep3.rez` and `Gamep4.rez` (the game DLLs
  zeroed, their names changed by `build/rezhide.py`) are in
  `build/retail-ranges.txt` (`build/mkinstall.py ranges`). A name with
  spaces ("Tron Update 2 and Dedicated Server Readme.txt") is the last
  field of its line. The legacy build's Windows script writes the registry
  values of the retail setup and Update 2 (`build/launchertest.sh` has the
  same values).
- **Tests.** The public archives of the legacy and the modern release
  builds, installed with `linux-install.sh` from the disc images in a new
  directory, are the game directory byte for byte, pass `sha256sum -c
  SHA256SUMS`, and reach the main menu (`make test TEST_GAME=<that
  directory>`: ALIVE). The PowerShell of `windows-install.bat`, run with
  PowerShell 7 on Linux, gives the same files. It is not yet tested with
  Windows PowerShell 5.1 on Windows.
- **Reproducible.** `build/pestamp.py` sets the link times of the game
  directory's binaries (and the `.pdb` signatures, the `.map` times) to
  2000-01-01. The archives have fixed times, sorted names and no owner
  names. A build without the retail game (`NODISCS=1`, the CI) thus makes
  the same public archive, byte for byte.
- **Checks.** `build/pubcheck.py` refuses a public archive that holds a
  retail file or a `.rez` entry of the retail game (by SHA-256), a file
  that the build does not make, or a home directory or the builder's user
  or host name.
- **CI** (`.github/workflows/ci.yml`, from cjacksrc's): one job for each
  variant, `make NODISCS=1 <target>`, each with its public archive as an
  artifact, and `make progress-ci` on the legacy release build. A push to
  master updates the pre-release `nightly`. A `v*` tag makes a release
  with the modern archives. The text is `.github/release_notes.md`.

What stays in the archives that a person could see: the debug builds'
`.pdb` and `.map` files name the build's own drives (`S:\` for `src/`,
`T:\` for `ext/toolchains/`, as `build/build.mk` maps them) and the paths
of Microsoft's libraries (`d:\builds\nt32_fre`, `E:\8168\VC98`): the
same on each machine. The modern `binkw32.dll` names FFmpeg's source
files relative to its tree (`src/libavutil/...`). The binaries contain
the retail art of the launcher, the dedicated server and `CRes.dll`
(`assets/art/`), as the repository does.
