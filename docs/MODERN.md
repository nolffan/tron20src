# The modern build

`TRON20SRC_MODERN` is the project's define for everything that departs from
the retail game: bug fixes (`RETAIL_BUGS.md`), new features, and
open-source middleware in place of the proprietary libraries. Only the
modern build sets it (`make modern`, `make modern-debug`;
`build/variant.sh`). The legacy build never sets it. The legacy build stays
the retail game as far as it can be rebuilt, and it is the only build that
`make progress` scores. Every change goes behind `#ifdef TRON20SRC_MODERN`,
with the retail code kept, unchanged, in the `#else`. (`TRON20SRC_DEBUG` is
the define of the debug builds, `make debug` and `make modern-debug`. It is
independent of `TRON20SRC_MODERN`. No code uses it yet.)

This file is the inventory: what the modern build changes, what it will
change next, and what was examined and not changed, and why. Its rules are
those of nolf2src's and cjacksrc's `docs/MODERN.md`. The items are Tron
2.0's own.

Each item has an id: a series and a number. An id is never used again for
a different item, also when an item is removed. The series:

| Series | What |
|---|---|
| `APPS-` | the launcher (`Tron.exe`, `TronLauncher.exe`) and the dedicated server (`TRONSrv.exe`) |
| `EN-` | the engine (`Lithtech.exe`, `server.dll`, `SndDrv.dll`) |
| `GAME-` | retail-bug fixes in the game DLLs (`CShell.dll`, `Object.lto`, `ClientFx.fxd`) |
| `CL-` | new features of the game client (`CShell.dll`) |
| `GS-` | Internet play: the GameSpy SDK and its replacement |
| `X-` | middleware: open-source libraries in place of proprietary ones |

Many engine items keep the numbers of cjacksrc's items with the same
change (the engine came from cjacksrc's, with its modern code). Those
numbers are Tron 2.0's ids now. nolf2src's ids (`M-E3`, `M-G45`, ...)
have Tron 2.0 `EN-` ids here, as the table "Inherited engine items" shows.

## Rules for this work

* **Reimplement, never copy.** Fan fixes and patches are a good source for
  *what* is wrong and *why*. Do not copy their code. The same rule applies
  to nolf2src's and cjacksrc's fixes: the idea often applies here, because
  the code is of the same generation, but make and check each change here.
* **The 2003 compiler.** The modern build uses the toolchain of the legacy
  build (Visual C++ 6.0, `msvc-6.00-8168`, and the DirectX 9 SDK that the
  engine links).
* **No SecuROM, no disc check.** The engine built here has no SecuROM. The
  modern build will also remove the game's own disc and CD-key checks
  (APPS-3, GS-4).
* **One owner for each file.** The Phase 1 work (below) is divided by
  files, so that each agent changes its own files. Where two items must
  change the same file, the table of overlaps says who changes what.

## The builds

`build/variant.sh` makes the three variant builds. Each copies `src/` (no
build output) into its own tree, `out/build-<variant>/`, and builds it
there with the same `build/all.sh` as the legacy build. It does not touch
`src/*/built`, `out/release-tree/` or `out/tron20-legacy-release/`. A
second run builds only what changed.

| Target | Defines | Game directory | Disk (tree + game directory) |
|---|---|---|---|
| `make` (legacy) | none | `out/tron20-legacy-release/` | (the release tree) |
| `make debug` | `TRON20SRC_DEBUG`, `/Z7 /Oy-`, `/DEBUG` | `out/tron20-legacy-debug/` | about 0.79 GB + 0.15 GB |
| `make modern` | `TRON20SRC_MODERN` | `out/tron20-modern-release/` | about 0.32 GB + 0.08 GB |
| `make modern-debug` | both | `out/tron20-modern-debug/` | about 0.79 GB + 0.16 GB |

The game directories hold hard links to the retail `.rez` archives and
movies. Only the three archives with the zeroed retail game DLLs (56 MB),
the third-party runtime and our binaries are copies. The trees share one
Wine prefix with `out/release-tree/` through hard links (about 0.6 GB,
counted once). The debug builds put a `.pdb` and a `.map` next to each
binary. `TRON20SRC_MIDDLEWARE=retail` (the default) puts the retail
third-party DLLs into the game directory; `open` is for X-1 and X-2 and is
refused until they exist.

The modern build boots: `make tests TEST_GAME=out/tron20-modern-release`
(the menus, a level, saving and loading, a light-cycle race), and `make
launchertest` with `LAUNCHER_EXE=TronLauncher.exe` (the launcher) and with
`LAUNCHER_EXE=Tron.exe LAUNCHER_SCRIPT=build/playtest-launcher-direct.txt`
(the game starts directly) all end ALIVE.

## Done

### Applications

* **APPS-1: `TronLauncher.exe` is the launcher; `Tron.exe` starts the
  game.** build/build.mk links the objects of the launcher two times in
  the modern build, and build/all.sh puts both into the game directory.
  `CLauncherApp::IsDirectLaunch` uses the name of the executable to tell
  the two apart. When the game is installed, `Tron.exe` does not show the
  launcher window. It does what the Play button does
  (`CLauncherDlg::LaunchDirect`: the same checks, then
  `LaunchGame(ACTION_PLAY)`, with the saved options, the rez files and the
  detail prompt of the first run). When the game is not installed,
  `Tron.exe` is the launcher, for its Install button.
  (`src/Game/Launcher/Launcher.cpp`, `LauncherDlg.cpp`; tested with `make
  launchertest`.) "Installed" still comes from the registry (APPS-3).

### Retail-bug fixes, game DLLs

One line each; the entry in `RETAIL_BUGS.md` has the addresses and the
details.

* **GAME-1:** a player model without a stand animation gets stand
  dimensions, not crouch dimensions (`CMoveMgr.cpp`).
* **GAME-2:** a prop frees the name of its looping sound (`Prop.cpp`).
* **GAME-3:** `ARMOR_UNLOCK` unlocks a power armor (`PowerArmor.cpp`).
* **GAME-4:** a `REMOVE` message removes a lite object (`GameBaseLite.cpp`).
* **GAME-5:** a searchable object's `MaxSearchTime` goes into its maximum
  (`Searchable.cpp`).
* **GAME-6:** an active world model's `ADDPSET` reads its argument, and
  `REMOVEPSET` reads only the length of its argument
  (`ActiveWorldModel.cpp`).
* **GAME-7:** an EventCounter registers `REPORT` (`EventCounter.cpp`).
* **GAME-8:** the disc class data keeps its `Controlable` value
  (`Shared/FXButeMgr.cpp`).
* **GAME-9:** an edit control without text returns an empty string
  (`Libs/LTGUIMgr/ltguieditctrl.cpp`).
* **GAME-10:** the mission text tests its string before it uses it
  (`HUDMissionText.cpp`).
* **GAME-11:** the chat input writes at most its buffer
  (`Libs/LTGUIMgr/ltguieditctrl.cpp`).
* **GAME-12:** the system menu shows the load control again
  (`MenuSystem.cpp`).
* **GAME-13:** the trigger's key list returns the key plugin's result
  (`Trigger.cpp`).
* **GAME-14:** an AI does not see a far target instantly
  (`AISenseRecorderGame.cpp`).
* **GAME-15:** an AI returns its "done processing stimuli" flag, not the
  ray count (`AI.cpp`).
* **GAME-16:** a failed use-object path trace does not read a null node
  (`AIHumanState.cpp`).
* **GAME-17:** the core dump item properties have a buffer of 100 bytes
  (`Character.cpp`). The archive bin has the same code and the same fix
  (`TRON/ArchiveBin.cpp`), without an entry of its own in
  `RETAIL_BUGS.md`.
* **GAME-18:** a hold block straight at the disc starts its orientation
  value at 0 (`TRON/TronPlayerObj.cpp`).
* **GAME-19:** the AI cycle spawner's error message gives the spawner
  number (`TRON/TronGameServerShell.cpp`).
* **GAME-20:** the cheat manager does not read past its table
  (`CheatMgr.cpp`).
* **GAME-21:** the light cycle power-up lock test does not read past the
  name table (`TRON/ScreenLightCycleGame.cpp`).
* **GAME-22:** loading a game without a current weapon does not read past
  the weapon list (`ClientWeaponMgr.cpp`).
* **GAME-23:** the cluster disc without class data stops after the setup
  of its base class (`TRON/ProjectileClusterDisc.cpp`).
* **GAME-24:** the tracked node manager returns the discomfort flag
  (`Shared/TrackedNodeMgr.cpp`).
* **GAME-25:** the binary bit compares the text of the permission set
  messages (`TRON/BinaryBit.cpp`).
* **GAME-26:** `SetEnergy` limits the energy to the maximum energy
  (`PlayerObj.cpp`).
* **GAME-27:** `DestroyProjectile` does not read past its object array
  (`PlayerObj.cpp`).
* **GAME-28:** an attached energy claw moves with its target
  (`TRON/ProjectileEnergyClaw.cpp`).
* **GAME-29:** the AI debug information shows the selected text, compares
  the texts, and cuts the holster text at ";" (`AI.cpp`).
* **GAME-30:** a full HUD bar sets its right cap texture on the correct
  polygon (`HUDBar.cpp`).
* **GAME-31:** a human AI reports `SETONLYDAMAGEBYPLAYER` as handled
  (`AIHuman.cpp`).
* **GAME-32:** a help file and a custom file download at their own rates
  (`TRON/RatingMgr.cpp`).
* **GAME-33:** the disc tournament's time-out moves to the next match
  before it deletes the current one (`TRON/ServerDiscMatchMgr.cpp`,
  `CServerDiscMatchMgr` update, retail 0x1004a67c). No entry in
  `RETAIL_BUGS.md` yet: the function is not byte-identical, so the bug is
  not confirmed in retail.

### Retail-bug fixes, engine

* **EN-3:** the `VersionInfo` console command does not call a null pointer
  (`clientde_impl.cpp`, `consolecommands.cpp`).
* **EN-8:** DrawPrim uses its camera's viewport (`render_b/.../
  d3ddrawprim.cpp`).
* **EN-16:** client prediction applies gravity to objects with
  `FLAG_GRAVITY`, not to visible objects (`client/src/predict.cpp`).
* **EN-34:** anisotropic filtering is switched on when the card can do it
  (`TextureFilterCaps`), and `Anisotropic` is the degree, held to what the
  card can do (`render_a/.../d3d_device.cpp`, `d3d_init.cpp`; nolf2src's
  M-G45).
* **EN-40:** the frame limiter: see below. It replaces the retail
  `cm_HandleMaxFPS`, which times the frame from an uninitialised value.

### Inherited engine items

The engine came from cjacksrc's with its modern code. These items compile
and boot in Tron 2.0's modern build (the tests above), but nobody has
checked them against Tron 2.0's own engine yet: the retail-bug fixes among
them have no entry in Tron 2.0's `RETAIL_BUGS.md`. The Phase 1 agent named
in the last column owns each one: it checks it, keeps it, changes it or
removes it.

| Id | What (cjacksrc's or nolf2src's item) | Files | Owner |
|---|---|---|---|
| EN-2 | in a window the game keeps its renderer when it loses the focus (cjacksrc EN-2) | `kernel/src/sys/win/client.cpp` | Display |
| EN-4 | the music thread's handle is closed (cjacksrc EN-4) | `ltdirectmusic_impl.cpp` | (fix) |
| EN-5 | a failed transition segment is tested (cjacksrc EN-5) | `ltdirectmusic_impl.cpp` | (fix) |
| EN-7 | no sound driver: no `Term` through a null pointer (cjacksrc EN-7) | `sound/src/soundmgr.cpp` | (fix) |
| EN-10 | `StopSecondary`/`StopMotif` walk their list once (nolf2src M-E7) | `ltdirectmusic_impl.cpp` | (fix) |
| EN-11 | null checks in the client interface (nolf2src M-E8) | `clientde_impl.cpp`, `winclientde_impl.cpp` | (fix) |
| EN-12 | `InitLevel`'s trace text (nolf2src M-E9) | `ltdirectmusic_impl.cpp` | (fix) |
| EN-13 | the console print does not write past its buffer (nolf2src M-E10) | `clientde_impl.cpp`, `dsys_interface.cpp` | (fix) |
| EN-14 | error messages name their own function (nolf2src M-E11) | `clientde_impl.cpp`, `winclientde_impl.cpp`, `client_iltphysics.cpp`, `ltinfo_impl.cpp`, `soundmgr.cpp` | (fix) |
| EN-15 | `streamsim_Open` closes its file on an error (nolf2src M-E12) | `streamsim.cpp` | (fix) |
| EN-20 | `##gamepad`: the XInput pad as a DirectInput gamepad; the engine half only (cjacksrc EN-20; `src/tron20src/xinput/`) | `kernel/src/sys/win/input.cpp` | Input |
| EN-21 | DirectInput joysticks are left out (cjacksrc EN-21) | `input.cpp` | Input |
| EN-22 | no DirectInput device enumeration (cjacksrc EN-22) | `input.cpp` | Input |
| EN-23 | mouse look: the whole buffer once a frame, no smoothing (nolf2src M-E3) | `input.cpp` | Input |
| EN-30 | widescreen field of view, Hor+ (cjacksrc EN-30) | `client/src/clientmgr.cpp` (`cm_WidescreenFovX`, `CClientMgr::Render`) | Display |
| EN-31 | a window at any size (cjacksrc EN-31) | `render_a/.../d3d_shell.cpp` | Display |
| EN-32 | the window mode: framed or borderless (cjacksrc EN-32) | `kernel/src/sys/win/render.cpp`, `render_a/.../common_init.cpp` | Display |
| EN-33 | antialiasing in a window too; 2D at whole pixels (nolf2src M-E13) | `d3d_device.cpp` (`SetPresentationParams`, `TryFallingBack_OnFailedDevCreate`), `render_b/.../d3ddrawprim.cpp` | Display |
| EN-40 | the frame limiter, `MaxFPS` 240 by default, 60 in multiplayer (cjacksrc EN-40) | `clientmgr.cpp` (`cm_ModernFrameLimit`, `CClientMgr::Update`), `shared/src/engine_vars.cpp` | Frame rate |
| EN-41 | a game tick at most every 10 ms (cjacksrc EN-41) | `server/src/servermgr.cpp` | Frame rate |
| EN-42 | the frame-time log, `FrameTimeLog` (cjacksrc EN-42) | `shared/src/sys/win/modernframelog.h`, `clientmgr.cpp`, `clientmgr.h`, `clientshell.cpp`, `d3d_device.cpp`, `rendershadowlist.cpp`, `engine_vars.cpp` | Frame rate |
| EN-43 | V-sync off in a window under Wine (cjacksrc EN-43) | `d3d_device.cpp` (`SetPresentationParams`) | Frame rate |
| EN-60 | a music intensity out of range (cjacksrc EN-60) | `ltdirectmusic_impl.cpp` | (fix) |
| EN-61 | projected shadows' culling buffer (cjacksrc EN-61) | `modelshadowshader.cpp`, `rendershadowlist.cpp` | Frame rate |
| EN-62 | V-sync in a window caps the rate, it does not wait (cjacksrc EN-62) | `d3d_device.cpp`, `clientmgr.cpp`, `modernframelog.h` | Frame rate |

"(fix)": a retail-bug fix that belongs to no Phase 1 agent. Whoever checks
the bug against Tron 2.0's retail `Lithtech.exe` writes its
`RETAIL_BUGS.md` entry, or removes the fix if Tron 2.0 does not have the
bug.

Known open points: EN-33's antialiasing cannot work yet with Direct3D 9.
The renderer always asks for `D3DPRESENTFLAG_LOCKABLE_BACKBUFFER`, which
Direct3D 9 does not allow with multisampling, so the device falls back to
no antialiasing. EN-20 is only the engine half: the game has no controller
code (cjacksrc's CL-20 to CL-24) and no default bindings for the pad.

## Planned (Phase 1)

Six agents do Phase 1, each with its own files. Each item lists the files
and functions that it will most likely change (from a search of the code).
Engine paths are under `src/Engine/runtime/`, game paths under
`src/Game/`.

### Apps

* **APPS-2: the launcher stays open after it starts the game.** Retail
  closes the launcher when the game starts (`CLauncherDlg::OnPlay` and the
  other actions: `LaunchGame`, then `EndDialog`; `CLauncherApp::
  InitInstance` then ends). The modern launcher stays (minimised, or
  waits for `Lithtech.exe` to exit), so that the Steam overlay and Steam
  Remote Play keep the process tree. `Tron.exe` (APPS-1) also waits for
  the game. Files: `Launcher/LauncherDlg.cpp` (`LaunchGame`, `OnPlay`,
  `LaunchDirect`), `Launcher/Launcher.cpp` (`InitInstance`).
* **APPS-3: settings and saved games beside the executable, no registry,
  no APPDATA.** The registry users: the launcher (`CLauncherApp` reads
  `InstallDir`, `UpdateNum`, `UpdateCommandLine0/1` to decide
  "installed" and the rez list; `LauncherDlg.cpp`, `OptionsDlg.cpp`,
  `DisplayDlg.cpp` store the options, `DisplayWarning`, `OptionsWarning`,
  `Num Launcher Runs` through `CRegMgr`, `src/libs/RegMgr/regmgr.cpp`),
  the game (`Shared/VersionMgr.cpp` `Language`;
  `ClientShellShared/ProfileMgr.cpp` `ProfileName`;
  `TRON/TronCDKey.cpp` the CD key), and the dedicated server
  (`ServerApp/ServerDlg.cpp` through `src/libs/RegMgr32/regmgr32.cpp`).
  Saved games and profiles (`Save/`, `Profiles/`) and `autoexec.cfg` are
  already in the game directory. A file beside the executable (as
  cjacksrc's APPS-6) replaces the registry; the modern build drops the
  disc check (`CLauncherApp::CD_VerifyGame`, `CD_GetDriveLetterWithGame`).

### Display

* **CL-30: modern and widescreen resolutions.** The Display screen lists
  only modes the engine enumerates and the menus and HUD are laid out for
  640x480. Files: `ClientShellDLL/TRON/ScreenDisplay.cpp`,
  `ClientShellShared/InterfaceMgr.cpp`, `TRON/TronInterfaceMgr.cpp`,
  `ClientShellShared/InterfaceResMgr.cpp` (the screen scale),
  `ClientShellShared/LayoutMgr.cpp`, the HUD items (`HUD*.cpp`);
  the launcher's `Launcher/DisplayMgr.cpp`, `DisplayDlg.cpp`.
* **CL-31: borderless fullscreen.** On EN-32's `WindowMode`: a Display
  screen and launcher option. Files: `kernel/src/sys/win/render.cpp`,
  `client.cpp` (the window, EN-2), `render_a/.../common_init.cpp`,
  `ScreenDisplay.cpp`, `Launcher/DisplayDlg.cpp`.
* **CL-32: an option to keep 4:3 in the cinematics.** The movies play
  through the engine's Bink video manager
  (`kernel/src/sys/win/binkvideomgrimpl.cpp`, `videomgr.cpp`,
  `client/src/client_iltvideomgr.cpp`), started from
  `ClientShellShared/InterfaceMgr.cpp` (the intro movies) and the
  in-game cinematics (`TRON/TronInterfaceMgr.cpp`). With X-1 the movie
  player changes: coordinate with Middleware.
* **CL-33: field of view.** On EN-30: a FOV setting. Files:
  `ClientShellShared/GameClientShell.cpp`, `PlayerMgr.cpp`,
  `PlayerCamera.cpp`, `TRON/TronPlayerMgr.cpp` (the camera's FOV),
  `ScreenDisplay.cpp`, engine `clientmgr.cpp` (`cm_WidescreenFovX`).
* **CL-34: up to 16x anisotropic filtering and up to 8x antialiasing.** On
  EN-34 and EN-33 (fix the lockable back buffer with multisampling).
  Files: `render_a/.../d3d_device.cpp` (`SetPresentationParams`,
  `SetDefaultRenderStates`), `d3d_init.cpp`, `TRON/ScreenPerformance.cpp`,
  `ClientShellShared/PerformanceMgr.cpp`.
* **CL-35: 32-bit colour by default.** The engine (`BitDepth` 32,
  `shared/src/engine_vars.cpp`), the retail `autoexec.cfg` and the
  launcher's detail settings are already 32-bit. The launcher's
  `CDisplayMgr` still falls back to 16 bits (`Launcher/DisplayMgr.cpp`:
  the default depth "16" when it reads the config, and the 16-bit
  `FindResolution` fallbacks).

### Frame rate

* **EN-44: frame rates above 60, with limits of 60, 120, 144, 240 and no
  limit.** On EN-40: the limits, a Display screen option, and the
  multiplayer limit (EN-40 holds multiplayer at 60; the item lifts it).
  Files: `client/src/clientmgr.cpp` (`cm_ModernFrameLimit`,
  `CClientMgr::Update`), `shared/src/engine_vars.cpp`,
  `kernel/src/sys/win/timemgr.cpp`, `TRON/ScreenDisplay.cpp` (one
  control).
* **EN-45: physics and animation do not depend on the frame rate,
  multiplayer included.** On EN-41. The game client steps with the frame
  time in `ClientShellShared/CMoveMgr.cpp`, `VehicleMgr.cpp` (the light
  cycles), `PlayerMgr.cpp`, `PlayerCamera.cpp`, `HeadBobMgr.cpp`,
  `LeanMgr.cpp`, `ClientFXMgr.cpp`, `TRON/TronPlayerMgr.cpp`,
  `TRON/HUDTronDecision.cpp`; the server in `server/src/servermgr.cpp`
  and `TRON/TronGameServerShell.cpp`; the light cycles in
  `Shared/TRON/LightCycleMgr.cpp`; the client's update rate to the server
  in `ClientShellShared/ClientMultiplayerMgr.cpp`.

### Input

* **EN-24: raw mouse input.** `WM_INPUT` (or DirectInput's buffered mouse,
  EN-23) for the look axes. Files: `kernel/src/sys/win/input.cpp`, the
  window procedure in `kernel/src/sys/win/client.cpp`.
* **CL-20: controllers through XInput.** On EN-20 to EN-22: the game half
  (cjacksrc's CL-20 to CL-24: look speed, dead zones, bindings, menus with
  the pad). Files: `src/tron20src/xinput/` (cjacksrc's `clientpad.cpp`),
  `ClientShellShared/GameClientShell.cpp`, `TRON/TronGameClientShell.cpp`,
  `TRON/ScreenControls.cpp`, `TRON/ScreenMouse.cpp`, a controller screen,
  `ClientShellShared/ProfileMgr.cpp` (the bindings), and the launcher's
  "Disable Joysticks" (`Launcher/OptionsDlg.cpp`).

### Middleware

What Tron 2.0 loads that is proprietary (`LoadLibrary` and the imports of
every retail binary):

| Retail | Loaded by | Modern |
|---|---|---|
| `binkw32.dll` (RAD Game Tools), the movies | `kernel/src/sys/win/binkvideomgrimpl.cpp` (`LoadLibrary("Binkw32.dll")`, by name) | X-1 |
| `L3CODECA.ACM`, the Fraunhofer MP3 codec of Windows' Audio Compression Manager (under Wine, `winemp3.acm`): 2,133 of the 2,572 WAV files in `Sound.rez` are MP3 (format 0x55) | `src/Engine/snddrv/s_dx8.cpp` | X-2 |
| `MSS32.DLL`, Miles | only `binkw32.dll` names it (its optional sound); nothing of the game loads it | goes with X-1 |
| `IMAADP32.ACM` (IMA ADPCM, part of Windows), `MFC42.DLL`, `MSVCP60.DLL`, `Msvcrt.dll`, D3DX9 (linked statically) | | kept: Microsoft runtime |
| the GameSpy SDK | compiled from our source (`src/Game/Libs/GameSpy`) | GS- items |

* **X-1: our Bink player on FFmpeg in place of `binkw32.dll`**, as
  nolf2src's and cjacksrc's X-1. Files: a new `src/modern/binkw32/` (or
  cjacksrc's), an FFmpeg submodule in `ext/`, a `build/ext.sh`,
  `build/all.sh` step 2 and `build/variant.sh` (`TRON20SRC_MIDDLEWARE=
  open`), `ext/tools/retailextract.py` (the third-party list).
* **X-2: MP3 decoding in `SndDrv.dll`** (minimp3, as cjacksrc's EN-50),
  so that the speech plays without a codec installed. Files:
  `src/Engine/snddrv/s_dx8.cpp`, `snddrv.dsp`.

### Online

* **GS-1: the master server: OpenSpy.** The game and the dedicated server
  report to `tron20.master.gamespy.com` (UDP 27900): `qr2.c` (the host
  name is made from the game name), started in
  `ObjectDLL/TRON/TronGameServerShell.cpp` (`qr2_init_socket`). The
  domain is `GSI_DOMAIN_NAME` in `Libs/GameSpy/GameSpySDK/nonport.h` (and
  `Libs/GameSpy/nonport.h`).
* **GS-2: the server list.** The client asks `tron20.ms<n>.gamespy.com`
  (TCP 28910): `serverbrowsing/sb_serverlist.c`, from
  `ClientShellDLL/TRON/ScreenJoinServerList.cpp` (`ServerBrowserNew`,
  `ServerBrowserUpdate`), `BaseScreenJoin.cpp`, `ScreenServerInfo.cpp`.
* **GS-3: the version check.** `motd.gamespy.com` (HTTP): `pt/ptMain.c`
  (`ptCheckForPatchAndTrackUsage`), from `TRON/BaseScreenJoin.cpp`.
* **GS-4: the CD-key check.** It goes through the master server: the
  server's `gcdkey/gcdkeys.c` (`gcd_init_qr2`, `gcd_authenticate_user`,
  `TronGameServerShell.cpp`), the client's `gcdkey/gcdkeyc.c`
  (`gcd_compute_response`, `TRON/TronClientMultiplayerMgr.cpp`), the key
  from the registry (`TRON/TronCDKey.cpp`, APPS-3). Without a GameSpy
  key server it must accept every client (or use OpenSpy's).

### Overlaps

| File | Agents | Who changes what |
|---|---|---|
| `render_a/.../d3d_device.cpp` `SetPresentationParams` | Display (EN-33, CL-34: multisampling, the flags), Frame rate (EN-43, EN-62: the presentation interval) | Display first; Frame rate changes only the interval lines |
| `client/src/clientmgr.cpp` | Display (EN-30, CL-33: `CClientMgr::Render`, `cm_WidescreenFovX`), Frame rate (EN-40, EN-44: `CClientMgr::Update`, `cm_ModernFrameLimit`) | separate functions |
| `kernel/src/sys/win/client.cpp` | Display (EN-2, CL-31: focus, window), Input (EN-24: `WM_INPUT` in the window procedure) | Input adds only the `WM_INPUT` case |
| `shared/src/engine_vars.cpp` | Display (new display variables), Frame rate (`MaxFPS`) | append only |
| `TRON/ScreenDisplay.cpp` | Display (CL-30, CL-31, CL-32, CL-33), Frame rate (EN-44: one frame-limit control) | Display owns it; Frame rate adds its control after Display |
| `ClientShellShared/PlayerCamera.cpp`, `PlayerMgr.cpp`, `TRON/TronPlayerMgr.cpp` | Display (CL-33: FOV), Frame rate (EN-45: frame-time steps) | separate functions; coordinate |
| `ClientShellShared/GameClientShell.cpp`, `TRON/TronGameClientShell.cpp` | Display (CL-33), Input (CL-20: the pad's update) | separate functions |
| `ClientShellShared/ProfileMgr.cpp` | Apps (APPS-3: `ProfileName` in the registry), Input (CL-20: bindings), Display (new settings) | each its own settings |
| `TRON/TronGameServerShell.cpp` | Online (GS-1, GS-4), Frame rate (EN-45: the update) | separate functions |
| `TRON/BaseScreenJoin.cpp`, `TRON/TronCDKey.cpp` | Online (GS-3, GS-4), Apps (APPS-3: the CD key in the registry) | Online owns them; Apps gives the storage |
| `Launcher/DisplayMgr.cpp`, `DisplayDlg.cpp`, `OptionsDlg.cpp` | Display (CL-30, CL-31, CL-35), Apps (APPS-3: where the options are stored), Input (CL-20: Disable Joysticks) | Apps changes only the storage calls |
| `binkvideomgrimpl.cpp`, `InterfaceMgr.cpp` (movies) | Display (CL-32), Middleware (X-1) | Middleware replaces the DLL behind the same calls; Display changes the rectangle |
| `build/all.sh`, `build/variant.sh` | Middleware (X-1, X-2) | Middleware only |
