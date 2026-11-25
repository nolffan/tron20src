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
  modern build removes the game's own disc check (APPS-3) and CD-key
  check (GS-4).
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
binary. `TRON20SRC_MIDDLEWARE=open` (the default of `make modern` and
`make modern-debug`) puts our `binkw32.dll` (X-1) into the game directory
in place of RAD's. `TRON20SRC_MIDDLEWARE=retail` (the default of `make
debug`, and what the legacy build does) puts RAD's there. You can set
either: `make modern TRON20SRC_MIDDLEWARE=retail`.

The modern build boots: `make tests TEST_GAME=out/tron20-modern-release`
(the menus, a level, saving and loading, a light-cycle race), and `make
launchertest` with `LAUNCHER_EXE=TronLauncher.exe` (the launcher) and with
`LAUNCHER_EXE=Tron.exe LAUNCHER_SCRIPT=build/playtest-launcher-direct.txt`
(the game starts directly) all end ALIVE.

## Console variables

The modern build's console variables (set in `autoexec.cfg` beside the
executable, or in the console), new or with a changed meaning.

| Variable | Type | Default | What | Item |
|---|---|---|---|---|
| `MaxFPS` | float | 240 (retail: 0, and not used in the final build) | the maximum number of frames each second; 0 is no limit; 1000 at most; the same in multiplayer; the Display screen's "Frame Rate Limit" (60, 120, 144, 240, None) | EN-40, EN-44 |
| `FrameTimeLog` | float | 0 | write each frame of this many milliseconds or more, with its parts, and a summary each ten seconds, to `frametime.log`; 0 is off | EN-42 |
| `PosLog` | float | 0 (no variable) | 1: write the player's position each frame to `poslog.txt`, and the light cycles' positions to `lcpos.txt` (the frame-rate tests, `build/fpscheck.py`) | EN-45 |

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
  launchertest`.) "Installed" is APPS-3's: `Lithtech.exe` beside the
  program.
* **APPS-2: the launcher stays until the game stops.** Retail's launcher
  starts `Lithtech.exe` and closes. Steam takes the end of the program
  that it started for the end of the game: the overlay and Remote Play
  stop. In the modern build, `CLauncherDlg::LaunchGame` starts the engine
  of the game directory by its full path, hides the launcher window, and
  waits until the engine stops (`CLauncherDlg::WaitForGame`:
  `MsgWaitForMultipleObjects`, so that the window's messages are
  dispatched). Then the launcher writes its settings and closes, as
  retail does after a start. This applies to Play, to the command line
  starts (`-GOGAME`, `-GOMULTIHOST`, `-GOMULTIJOIN`, `-GOMULTIFIND`,
  `-GOSERVER`) and to `Tron.exe`'s direct start (APPS-1), which has no
  window. `-GOSERVER` starts the dedicated server, and the launcher waits
  for it too. Retail started "TRON2Svr.EXE" (IDS_SERVEREXE), which does
  not exist: the modern launcher starts `TRONSrv.exe` (`RETAIL_BUGS.md`;
  `Launcher/Launcher.cpp` `InitInstance`, `Launcher/LauncherDlg.cpp`
  `LaunchGame`, `WaitForGame`). Tested with `make portabletest`
  (`build/portabletest.sh`): a monitor in the container records the
  processes each second. `TronLauncher.exe` and `Tron.exe` were present
  at each sample while `Lithtech.exe` ran, and stopped within 15 s after
  the game quit from its main menu. `make launchertest` with
  `LAUNCHER_EXE=TronLauncher.exe` and with `Tron.exe` ends ALIVE.
* **APPS-3: no registry, no APPDATA: the settings are in `tron20.ini`
  beside the executable.** Retail keeps its values under
  `HKEY_LOCAL_MACHINE\SOFTWARE\Monolith Productions\TRON 2.0\1.0`
  (opened for writing, also to read: `RETAIL_BUGS.md`): the launcher's
  options, `DisplayWarning`, `OptionsWarning`, `Num Launcher Runs`,
  `Save Commands`, `Commands`, `SelectedMod`; the installer's
  `InstallDir`, `UpdateNum`, `UpdateCommandLine0/1`; the game's
  `Language` and `ProfileName` (`CVersionMgr`, `CProfileMgr`) and
  `CDKey` (`TronCDKey.cpp`); and GameSpy's ID for the version check
  (`HKEY_CURRENT_USER\Software\GameSpy\GameSpy 3D\Registration`,
  `Crypt`, `nonport.c` `GOAGetUniqueID`). The modern build compiles the
  same classes, `CRegMgr` (`libs/RegMgr/regmgr.cpp`: the game DLLs,
  `TRONSrv.exe`) and `CRegMgr32` (`libs/RegMgr32/regmgr32.cpp`: the
  launcher, `TRONSrv.exe`), on a file: `tron20.ini` in the directory of
  the running program (`libs/RegMgr/regfile.h`, the Windows profile
  functions). The launcher, `Lithtech.exe` with the game DLLs and the
  server are all in the game directory, so they use one file. Each key is
  a section, without the root key and without `SOFTWARE\`
  (`[Monolith Productions\TRON 2.0\1.0]`); strings are in quotes, DWORDs
  in decimal, binary data as `WritePrivateProfileStruct` writes it. Each
  call reads or writes the file at once, so that the launcher sees the
  `ProfileName` that the game wrote while it waited. The registry is not
  read: values of a retail install are not carried over (the detail
  dialog comes once more). The callers do not change, except:
  * "Installed": the game directory is the install. `CLauncherApp::
    InitInstance` changes to the directory of the program; the game is
    installed when `Lithtech.exe` is there, and `InstallDir` is that
    directory. `LaunchGame` does not read `InstallDir`. Without
    `Lithtech.exe`, Play (Install) shows "Could not launch LITHTECH.EXE"
    and does not start the disc's setup program. Uninstall is disabled:
    there is no installer.
  * Update 2's archives: with no `UpdateNum` in the file (an installer
    wrote it in retail), the launcher (`LaunchGame`) and the dedicated
    server (`CServerDlg::AddResources`) add `Gamep3.rez` and
    `Gamep4.rez` from the game directory, when they are there.
  * `CRegMgr32::OpenKey` always succeeds (a key with no values is an
    empty section): the server hosts with no values (`DoHost`,
    `AddResources` returned false without the key in retail).
  * The disc check is removed: `CD_GetDriveLetterWithGame` probes no
    drive (the `CdRom Drive` value, `\TRON.EXE` on each CD-ROM drive) and
    `CD_VerifyGame` only finds the rez files in the game directory. The
    message for a missing `Game.rez` does not mention a disc
    (`TRON/Launcher.rc`).
  * The CD key: `TronCDKey.cpp` reads `CDKey` from the file, where nothing
    writes it, so it is empty. GS-4 (Online) decides what multiplayer does
    without a key.
  Saved games (`Save/`), profiles (`Profiles/`), `autoexec.cfg` and
  `display.cfg` were already in the game directory. The engine
  (`Lithtech.exe`, `server.dll`) has no registry or `SHGetFolderPath`
  call of its own: `Lithtech.exe` imports `RegOpenKeyA` and
  `RegQueryValueExA` only for D3DX9 (static, it reads
  `Software\Microsoft\Direct3D`; not the game's). Temporary files are
  not settings, and the test does not count them: `CShell.dll`
  (`CClientFXDB::LoadFxDll`) copies `ClientFx.fxd` to `Temp\cfx*.tmp` and
  loads that copy. Under Wine the copy stays after the game stops (one
  for each start; not examined further). The imports of the
  modern binaries have no other registry function. A first run with no
  file: the launcher's defaults, written when it closes; the detail
  setting passes `+BitDepth 32` (32-bit colour; the 16-bit fallback of
  `DisplayMgr.cpp` is Display's, CL-35). Files: `libs/RegMgr/regfile.h`
  (new), `regmgr.h`, `regmgr.cpp`, `libs/RegMgr32/regmgr32.cpp`,
  `Libs/GameSpy/nonport.c`, `Launcher/Launcher.cpp`, `Launcher.h`,
  `LauncherDlg.cpp`, `LauncherDlg.h`, `TRON/Launcher.rc`,
  `ServerApp/ServerDlg.cpp`. Tested with `make portabletest`
  (`build/portabletest.sh`, `build/playtest-launcher-portable*.txt`): a
  fresh Wine prefix with no values of the game and no CD-ROM drive;
  `TronLauncher.exe` (Play, the detail setting, the main menu, Quit),
  `Tron.exe` (the same), then `TRONSrv.exe` hosting a LAN Disc Arena
  game to its loaded world. The key names and values of `system.reg`,
  `user.reg`, `userdef.reg` and the files under `drive_c/users/` before
  and after: no new line names the game, and no file was added under
  AppData. `tron20.ini` had the launcher's values and the game's
  `ProfileName`. `make launchertest` (whose script writes the retail
  installer's values into the registry): the modern launcher ignored
  them and its command line had `Gamep3.rez` and `Gamep4.rez` from the
  game directory.

### Middleware

* **X-1: our Bink player on FFmpeg in place of RAD's `binkw32.dll`.**
  `src/modern/binkw32/binkw32.c` plays the movies with FFmpeg's Bink
  demuxer and its Bink video and audio decoders, linked statically. It
  exports the twelve entry points that `Lithtech.exe` gets by name
  (`BinkVideoMgr::Init`, `binkvideomgrimpl.cpp`: `_BinkOpen@8` to
  `_BinkDX8SurfaceType@4`, `binkw32.def`). No other retail binary imports
  or loads Bink. Tron 2.0's engine always asks for `BINKSURFACE32` (its
  format table, Lithtech.exe 0x5ac214) and does not call
  `BinkDX8SurfaceType`; the player writes an opaque alpha byte, because the
  texture can be `A8R8G8B8`. The sound of a movie (its first audio track)
  is decoded when the movie opens and plays through `waveOut` from the
  first frame; the frames are timed from the same moment. The game has
  three movies, all Bink 1 (`BIKi`), 640x480, each with sound:
  `Movies/disnlogo.bik` (4 s), `lithlogo.bik` (18 s) and `nvidia.bik`
  (5 s), the logos at start (`CInterfaceMgr`'s movie state). No archive
  holds another `.bik`: the cinematics of the missions are scenes in the
  engine, not movies. `build/ext.sh` builds FFmpeg (only the Bink parts,
  no GPL or non-free part) and the DLL with Zig's C compiler into
  `out/ext/`. `build/all.sh` step 2 installs it when
  `TRON20SRC_MIDDLEWARE=open` (`ext/tools/retailextract.py thirdparty
  ... open` and `openruntime`; `MANIFEST.txt` origin `EXT`); build/
  variant.sh sets `open` for the modern builds. The legacy build keeps
  RAD's DLL. The FFmpeg submodule, `ext/ffmpeg`, is upstream FFmpeg at
  the commit of nolf2src's and cjacksrc's (n8.1.3); the licence notes are
  in `docs/LICENSE_NOTES.md`. Tested under Wine in the build container
  (`build/playtest-movies.txt` with `PLAYTEST_ARGS="+DisableMovies 0"` and
  `PLAYTEST_AUDIO`, `build/playtest.sh`): the three movies play in order,
  with their pictures moving and with correct colours (screenshots every 2
  to 4 s), then the main menu comes. The recording has three 44.1 kHz
  stereo streams from `waveOut`, one for each movie, with sound for about
  4, 17 and 4 s. Step 5 of the build lists `binkw32.dll` as `EXT
  src/modern/binkw32 + ext/ffmpeg: our Bink player (build/ext.sh)`.
  Our DLL imports the Universal C runtime (`api-ms-win-crt-*`), which
  Windows 10 and later have (an update gives it to Windows 7 and 8), and
  Wine has.
* **X-2: MP3 decoding in `SndDrv.dll`.** 3,404 of the 3,843 WAV files in
  `Sound.rez` are MP3 (format 0x55): `VOICE/` 2,084, `MUSIC/` 1,317,
  `SND/` 3 (`PLAYER/ENDLESS_FALL`, `EVENT/PISTONEXPLDE`,
  `EVENT/10_4MIXST`). The other 439 and the 427 WAV files of `Game.rez`
  are PCM (one of `Game.rez`'s is 32-bit float); no file is IMA ADPCM. Retail decodes the MP3 files through
  Windows' Audio Compression Manager (ACM) with the MP3 codec of the
  system: `L3CODECA.ACM` (Fraunhofer's on Windows, Wine's own under Wine).
  Without one, `CDx8SoundSys::Startup` fails and the game has no sound at
  all. The modern `SndDrv.dll` has its own ACM codec on minimp3 (CC0):
  `src/tron20src/mp3/mp3acm.cpp`, a driver procedure for MP3 to 16-bit
  PCM, which `SelectMP3Codec` adds to ACM as a local driver of the process
  (`Mp3Acm_Register`: `acmDriverAdd` with `ACM_DRIVERADDF_FUNCTION |
  ACM_DRIVERADDF_LOCAL`, as retail adds `L3CODECA.ACM`) and always selects.
  All of SndDrv's ACM calls stay as they are. A local driver comes before
  the drivers of the system, so a caller in the process that gives ACM no
  driver (DirectMusic, for the music waves) also gets it. cjacksrc's EN-50
  puts minimp3 in its driver in place of the ACM calls; here the codec is
  under ACM, so that DirectMusic can use it too. The codec takes all of
  the input of each convert, because `WaveFile::ReadCompressed` does not
  read `cbSrcLengthUsed`, and keeps what it cannot give yet. Three more
  changes in `s_dx8.cpp`, all modern only: `ReadCompressed` gives the codec
  only the bytes that it read (a retail bug, `RETAIL_BUGS.md`: the last
  block got the stale end of the block before), the first convert after
  `WaveFile::Cue` has `ACM_STREAMCONVERTF_START` (the codec drops what it
  holds; new member `WaveFile::m_bCodecRestart`), and the last block of the
  data has `ACM_STREAMCONVERTF_END` (the codec decodes the last frames).
  `mp3acm.cpp` is included at the end of `s_dx8.cpp`, so `snddrv.dsp`
  does not change. Tested: (1) `make mp3acmtest` (`build/mp3acmtest.sh`,
  `src/tron20src/mp3/mp3acmtest.cpp`, under Wine with Wine's MP3 codecs
  switched off): every one of the 3,404 MP3 files decodes whole (one
  convert, with no driver handle, so ACM finds the codec itself) and
  streamed (SndDrv's block sizes, through the driver, after a stop at half
  way and a restart), and the two give the same bytes; 19,499 s of PCM,
  each file within 0.03 s of the length that its data gives, except
  `MUSIC/CINEMATICS/CREDITS2.wav` (318.7 s for 318.0 s). (2) In the game
  (`build/playtest-sound.txt`, M01-01: a walk to the first bit and the
  action key) in a copy of the Wine prefix without an MP3 codec
  (`l3codeca.acm` deleted from `system32`, its `system.ini` line and its
  ACM cache key removed, and `l3codeca.acm` and `winemp3.acm` disabled in
  `DllOverrides`): the bit speaks its line, with its subtitle. The log
  (`+debugstr`) has "SndDrv: MP3 codec: minimp3, local ACM driver" at
  start and "SndDrv: MP3 codec: stream ..., 44100 Hz, 1 channel(s)" for the
  line; the recording (`PLAYTEST_AUDIO`) has sound. The legacy build in the
  same prefix plays nothing (its recording is silent: no MP3 codec, so no
  sound driver). The music could not be heard under Wine: see X-3.

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
* **EN-40:** the frame limiter (Frame rate, below). It replaces the
  retail `cm_HandleMaxFPS`, which times the frame from an uninitialised
  value.

### Frame rate

The game runs at frame rates above 60, in single player and in
multiplayer, and it plays the same at all frame rates. The limit is a
setting. The engine's limiter came from cjacksrc's (EN-40 to EN-43, EN-61,
EN-62); EN-44 and EN-45 are Tron 2.0's.

* **EN-40: the frame limiter.** `MaxFPS` is the maximum number of frames
  each second: 240 if the config does not set it, 0 is no limit, 1000 at
  most. Each frame waits until 1/`MaxFPS` has gone since the end of the
  last frame. It sleeps while more than 2 ms are left (the timer runs at
  1 ms, `timemgr.cpp`) and spins for the remainder. Retail calls its own
  limiter (`cm_HandleMaxFPS`) with an uninitialised frame time
  (`RETAIL_BUGS.md`); the modern build does not call it.
  (`client/src/clientmgr.cpp` `cm_ModernFrameLimit`, `CClientMgr::Update`;
  `shared/src/engine_vars.cpp`.) Checked in Tron 2.0 (`FrameTimeLog`):
  `MaxFPS 60` holds the menus and a race at 59 to 60 frames a second,
  with the wait in the log's `limit`.
* **EN-41: a game tick at most every 10 ms.** Kept as cjacksrc's. In single
  player and for a host, the client runs the server once each frame. A
  tick now runs only when 10 ms or more have gone since the last tick; the
  network runs at each call. Thus an object that the server updates every
  10 ms (each AI) does not step by a short frame's time above 100 frames a
  second. Tron 2.0's server objects step by the frame time
  (`GetFrameTime`): the AI light cycles, the discs, the energy and health
  zones, the core dumps. A search of `Object.lto`'s code found no amount
  given once each tick without the tick's time. (`server/src/
  servermgr.cpp` `CServerMgr::Update`.)
* **EN-42: the frame-time log.** Kept as cjacksrc's. `FrameTimeLog <ms>`
  writes each frame of that many milliseconds or more, and a summary each
  ten seconds (frames, frames each second, the parts), to `frametime.log`
  in the game directory. The tests below read their frame rates from it.
* **EN-43, EN-62: V-sync in a window under Wine.** Kept as cjacksrc's.
  Under Wine a window (borderless too) presents at once; with
  `VSyncOnFlip` on, the frame limiter holds the game to the refresh rate
  of the display. Full screen keeps the real wait for the refresh.
  Tron 2.0's renderer is Direct3D 9, which permits
  `D3DPRESENT_INTERVAL_IMMEDIATE` in a window also on Windows: there, with
  V-sync off, a window does not wait (the retail code), and with V-sync on
  it waits (not changed: not tested on Windows). (`render_a/.../
  d3d_device.cpp` `SetPresentationParams`.) Tested under Xvfb: in a window
  with `VSyncOnFlip 1` and `MaxFPS 0`, 58 to 60 frames a second, 13 ms of
  each frame in `limit`; full screen with V-sync, 68 to 74 (Xvfb has no
  refresh to wait for).
* **EN-61: projected shadows' culling buffer.** Kept as cjacksrc's, not
  measured here.
* **EN-44: frame rates above 60, with a limit of 60, 120, 144, 240 or
  none.** The Display screen has a "Frame Rate Limit" row under
  Synchronization: 60, 120, 144, 240 and None (`MaxFPS`). A config without
  `MaxFPS` shows 240 (the engine's default). A limit that is not on the
  list (set by hand) shows as the next higher one (None above 240). The
  screen writes `MaxFPS` only when the row changes, so a limit set by hand
  stays until then. The engine keeps `MaxFPS` in `autoexec.cfg` beside the
  executable. cjacksrc's limit of 60 in multiplayer (hosting or joining) is
  removed: the limit is the same in multiplayer (EN-45 has the
  multiplayer faults). Strings 31100 to 31102 (the block 31100 to 31109 is
  the frame rate's). (`client/src/clientmgr.cpp` `cm_ModernFrameLimit`;
  `TRON/ScreenDisplay.cpp` `Build`, `OnFocus`; `ClientRes/Shared/
  ClientResShared.h`, `ClientRes/TRON/ClientRes.rc`.) Tested
  (`build/playtest-framelimit.txt`): the row shows 240; two clicks show
  None, then 60; back and to the screen again, it shows 60. In multiplayer
  light cycle games, an unlimited client ran at 60 to 130 frames a second
  and an unlimited host at 75 to 660 (ten-second averages; below). The launcher has no frame rate setting (as nolf2src's
  and cjacksrc's).
* **EN-45: physics and animation at all frame rates, multiplayer
  included.** What was tied to the frame, and what changed:
  * **The light cycle on the grid** (Tron 2.0's own). The player's cycle
    is put on the grid (whole units) after each frame's move, so each
    frame moves a whole number of units: round(speed x frame time). At
    840 units a second a frame of 16 or 17 ms moves 13 or 14 units (825
    units a second), but frames of 5 to 6 ms move 4 to 5 (853). At 1000
    frames a second a cycle at 1400 moves 1 unit a frame (1000 units a
    second). A move under 0.1 units is not made; a turn made on a frame
    that rounds to no move is lost. Now the move is kept without rounding
    from frame to frame along the axis of travel, and only the position
    that the cycle is put at is rounded; the first move after a turn is
    at least one grid unit (`TRON/TronPlayerMgr.cpp`,
    `CTronPlayerMgr::TronUnknownVirtual64`, retail 0x100a7390).
  * **The other players' cycles** (Tron 2.0's own). A client puts another
    player's cycle `LCClientPredict` (10; a host `LCHostPredict`, 2)
    frames ahead of its last known position, and moves it `LCInterpTime`
    (0.2) of the way there, each frame: the higher the frame rate, the
    less it predicts (0.17 s at 60, 0.04 s at 240) and the faster it
    catches up. Now a frame counts as a 60th of a second at least for the
    prediction, and the share of the way is that of a 60th of a second:
    above 60 frames a second the cycle moves as at 60; at 60 and less,
    the prediction is retail's (`TRON/ClientLightCycleMgr.cpp`,
    `CClientLightCycleMgr::Update`, retail 0x10011410).
  * **Client prediction** (the engine; every object that the server
    moves, in single player and multiplayer: the other players, the AIs,
    the discs). Each frame moves an object half of the way to its
    predicted position, and turns it half of the way to its last known
    rotation: the faster the frame rate, the closer it follows and the
    less it smooths. Now the share is that of a 60th of a second:
    1 - 0.5^(60 x frame time), 0.5 at 60 frames a second
    (`client/src/predict.cpp` `pd_Update`).
  * **The player's movement**, as nolf2src's M-E4 and cjacksrc's CL-41,
    CL-42 (the code is NOLF2's): the speed cap is applied at the start of
    a frame and the frame's acceleration takes the speed past it, so the
    higher the frame rate, the slower the run; the cap now applies once
    each 60th of a second. A frame's move under 0.1 units is not made: the
    limit is now 0.1 units each 60th of a second (and so are the limits of
    the blocked fall). The "cannot move" damping (half each frame), the
    spectator damping (0.9 each frame) and a pusher's push (each frame)
    are now each 60th of a second. Out of the water, the rise stops two
    60ths of a second after the head comes out, and a jump out of the
    water gets a 60th of a second of the swim-up push. The jump: the
    engine's physics (`CalcMotion`) moves by the frame's speed before it
    takes off the frame's gravity, and by only a quarter of a*t*t, so a
    jump goes higher at a lower frame rate (82.8 units at 60 frames a
    second, 80.8 at 254). Above 60 frames a second, the jump starts faster
    by a quarter of the gravity of the difference between the frame and a
    60th of a second: it peaks as at 60. (This is the opposite of
    cjacksrc's correction: its engine's motion is not the same.)
    (`ClientShellShared/CMoveMgr.cpp` `UpdateMotion`, `UpdateNormalMotion`,
    `UpdateInLiquid`, `UpdatePushers`, `MoveLocalSolidObject`,
    `InitWorldData`.) Tron 2.0 has no water in its levels that the tests
    reach: the water changes are not measured.
  * **Particle and line systems**, as cjacksrc's CL-44: an update that
    owes less than one particle (line) adds none and starts its clock
    again, so a slow system never emits at a high frame rate. Now it keeps
    its time until one is owed (`ClientShellShared/ParticleSystemFX.cpp`,
    `LineSystemFX.cpp` `Update`).
  * **Fire sparks and debris**, as cjacksrc's CL-45: a fire makes a spark
    with one chance in ten each frame, now each 60th of a second
    (`FireFX.cpp` `Update`); bouncing debris comes to rest below a speed
    that grows with the frame rate, now the speed of 60 frames a second
    (`client_physics.cpp` `BounceMovingObject`).

  `RETAIL_BUGS.md` has the faults of the functions that are
  byte-identical (`UpdatePushers`, `MoveLocalSolidObject`,
  `UpdateInLiquid`, `BounceMovingObject`, `CFireFX::Update`,
  `CParticleSystemFX::Update`, `CLineSystemFX::Update`). `UpdateMotion`,
  `UpdateNormalMotion`, `CTronPlayerMgr::TronUnknownVirtual64` and
  `CClientLightCycleMgr::Update` are not yet byte-identical, and the
  engine's `pd_Update` is not checked: their faults have no entry yet.

  Examined and not changed: the light cycles' speed, acceleration, turns
  and trails (`Shared/TRON/LightCycleMgr.cpp`: the speed changes by its
  rate times the frame time; a turn and its loss of speed are once each
  turn; the trail and its collisions are segments from the last point to
  the position); the other `VehicleMgr.cpp` code (the camera turns by the
  frame time); `PlayerMgr.cpp`, `TRON/TronPlayerMgr.cpp`,
  `PlayerCamera.cpp`, `HeadBobMgr.cpp`, `LeanMgr.cpp` (each step is the
  frame time times a rate, or a time); the HUD (`HUDTronDecision.cpp`,
  `HUDHealthEnergy.cpp`, `HUDPings.cpp`: timers by the clock or the frame
  time); `ClientFXMgr.cpp` and the ClientFX effects (their emission keeps
  its time: `ClientFxDLL/ParticleSystemFX.cpp`); the discs
  (`ClientWeaponDisc.cpp`: the control line goes at most every
  `DiscControlRate` seconds in multiplayer; the disc itself is a server
  object); `ClientMultiplayerMgr.cpp` (no update by frame); what the client
  sends to the server: `MID_PLAYER_UPDATE` at most `CSendRate` times a
  second to a remote server, the light cycle's position at most every
  `m_fPredictInterval` unless it turns (to a host's own server, both each
  frame, as retail); the dedicated server (its own update rate). Tron
  2.0's weapons have no turn-dependent aim (cjacksrc's CL-45 part). The
  frame time is retail's whole milliseconds (`time_GetMSTime`): at 240
  frames a second frames take 4 or 5 ms. This is uneven motion, not lost
  time; not changed. `ShowFrameRate` (the engine's counter) prints only in
  a debug engine; the modern build has no frame rate counter on the
  screen (cjacksrc's CL-46 is not ported): use `FrameTimeLog`.

  How it was tested: `PosLog 1` (new, `CMoveMgr::ModernPosLog`) writes
  the player's position each frame to `poslog.txt`, and the light cycles'
  positions to `lcpos.txt` (`CClientLightCycleMgr::Update`).
  `build/playtest-fps-race.txt` and `build/playtest-fps-walk.txt` play the
  same keys at `MaxFPS` 60 and 0 (no limit), with the world not drawn
  (`build/fps-60.cfg`, `build/fps-0.cfg`: under Xvfb the software
  renderer holds a race at 11 to 24 frames a second when it draws), and
  `build/fpscheck.py` compares them. Without a limit, the runs reached
  150 to 254 frames a second (the frame rates are in the tables); a
  higher rate was not reached under Xvfb. "Before": the modern build
  without the EN-45 changes of that part.

  | Race (Newbie Authentication, 840 units a second) | before, 60 | before, 163 | after, 59 | after, 161 |
  |---|--:|--:|--:|--:|
  | distance in the first 5 s | 4141 | 4268 | 4209 | 4205 |
  | distance in the first 10 s (two turns) | 8280 | 8482 | 8375 | 8373 |
  | the leg between the turns, units a second | | | 822 | 827 |

  | Walk in M01-01 | before, 59 | before, 254 | after, 58 | after, 191 |
  |---|--:|--:|--:|--:|
  | run forward 0.7 s: distance, top speed | 214.5, 346 | 199.0, 313 | 204.6, 349 | 211.1, 352 |
  | run back 0.7 s: distance, top speed | 208.4, 331 | 201.1, 299 | 208.1, 331 | 209.0, 324 |
  | strafe 0.5 s each way: distances | 148.6, 154.5 | 143.9, 144.1 | 154.1, 154.2 | 149.7, 146.0 |
  | crouching walk 0.6 s: distance | 74.3 | 69.5 | 74.0 | 73.1 |
  | standing jump, two: units high | 82.79, 82.81 | 80.80, 80.80 | 82.84, 82.83 | 82.86, 82.82 |

  (The key holds come from xdotool, so a distance varies by about 3% from
  run to run at the same rate. The top speed shows the cap.)

  Multiplayer (`make mptest MPTEST_MODE=listen MPTEST_TYPE=lc`, with
  `MPTEST_CFG1`/`MPTEST_CFG2`, new, for each client's settings): two
  runs on the loopback, the world not drawn. In the first, the host ran
  at `MaxFPS` 60 (57 to 59 frames a second in its rounds) and the other
  client without a limit (95 to 116); in the second, the host without a
  limit (85 to 117) and the other client at 60. Both clients joined, saw
  each other and played to the end in both runs. A cycle that runs the
  full length of the arena (25596 units, from its start to the wall: the
  cycle accelerates with the forward key to the speed limit of 3200)
  took 9.06 to 9.09 s on each client in both runs, at 60 and above (one
  round of 9.31 s, the first of the unlimited host, which started its
  log before the race did). Where the clients saw each other's cycles
  stop (`lcpos.txt`) is within the prediction (`LCClientPredict` 10
  60ths of a second, `LCHostPredict` 2) of where those cycles stopped.
  Not tested: an Internet game, a dedicated server with clients above 60
  (its own update rate does not depend on the clients'), more than two
  players, and the disc games above 60 in multiplayer.

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
| EN-40 | the frame limiter, `MaxFPS` 240 by default (cjacksrc EN-40); checked, and changed by EN-44: no limit of 60 in multiplayer | `clientmgr.cpp` (`cm_ModernFrameLimit`, `CClientMgr::Update`), `shared/src/engine_vars.cpp` | Frame rate: done (below) |
| EN-41 | a game tick at most every 10 ms (cjacksrc EN-41); checked, kept | `server/src/servermgr.cpp` | Frame rate: done (below) |
| EN-42 | the frame-time log, `FrameTimeLog` (cjacksrc EN-42); checked, kept | `shared/src/sys/win/modernframelog.h`, `clientmgr.cpp`, `clientmgr.h`, `clientshell.cpp`, `d3d_device.cpp`, `rendershadowlist.cpp`, `engine_vars.cpp` | Frame rate: done (below) |
| EN-43 | V-sync off in a window under Wine (cjacksrc EN-43); checked, kept | `d3d_device.cpp` (`SetPresentationParams`) | Frame rate: done (below) |
| EN-60 | a music intensity out of range (cjacksrc EN-60) | `ltdirectmusic_impl.cpp` | (fix) |
| EN-61 | projected shadows' culling buffer (cjacksrc EN-61); kept | `modelshadowshader.cpp`, `rendershadowlist.cpp` | Frame rate: done (below) |
| EN-62 | V-sync in a window caps the rate, it does not wait (cjacksrc EN-62); checked, kept | `d3d_device.cpp`, `clientmgr.cpp`, `modernframelog.h` | Frame rate: done (below) |

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

APPS-2 and APPS-3 are done (above).

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

Done: EN-44 and EN-45 (under "Done", "Frame rate").

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

What Tron 2.0 loads that is not ours (`LoadLibrary`, the imports and the
DLL names in every retail binary):

| Retail | Loaded by | Modern |
|---|---|---|
| `binkw32.dll` (RAD Game Tools), the movies | `kernel/src/sys/win/binkvideomgrimpl.cpp` (`LoadLibrary("Binkw32.dll")`) | ours, X-1 (done) |
| `MSS32.DLL`, Miles (RAD Game Tools) | only RAD's `binkw32.dll` names it (with `DSOUND.DLL`, for its sound outputs); no binary of the game imports or loads it, and the game does not ship it | gone with X-1: our player plays through `waveOut` |
| `L3CODECA.ACM`, the MP3 codec of the system | `src/Engine/snddrv/s_dx8.cpp` (`SelectHardCodedMP3Codec`, then the ACM drivers of the system) | ours, in `SndDrv.dll`, X-2 (done) |
| `IMAADP32.ACM` (IMA ADPCM, a part of Windows and of Wine) | `s_dx8.cpp`; `Startup` fails without an ADPCM driver, though no file of the game is IMA ADPCM | kept: a part of the system |
| Microsoft's DirectMusic (`dmime`, `dmusic`, `dmsynth`, `dswave`, ...), the music | the engine (`ltdirectmusic_impl.cpp`, `CoCreateInstance`); a part of Windows | X-3 (open, below) |
| `cdaudio.dll` (Monolith's CD music driver) | the engine (`CClientMgr::AppInitMusic`), only if the game asks; Tron 2.0 does not ship it | nothing to do |
| `MFC42.DLL`, `MFC42U.DLL`, `MSVCP60.DLL`, `Msvcrt.dll`, `MSVCIRT.dll`; D3DX9 (linked statically) | | kept: Microsoft's runtime |
| SecuROM's DLL names in the retail `Lithtech.exe` (`CmdLineExt*.dll`, `SIntf*.dll`, `PID*.DLL`, ...) | the copy protection | not in our engine |
| the GameSpy SDK | compiled from our source (`src/Game/Libs/GameSpy`) | GS- items |

* **X-3: the music under Wine and Proton.** The music is DirectMusic:
  segments (`MUSIC/*/*.sgt`) with MP3 waves. Under Wine 9.0 the legacy
  build's main menu is silent (a recording with `PLAYTEST_AUDIO`), as
  nolfsrc and avp2src found for their games: Wine's DirectMusic loads the
  segments but plays nothing (and Wine's `dswave.dll` has no ACM code for
  compressed waves). nolfsrc's M-P4-12 and avp2src's M-X4 put Microsoft's
  DirectMusic in the game directory (`ms_<name>.dll` from
  `ext/toolchains/directmusic-5.3.1.904/` and a manifest for registration-
  free COM: `DIRECTMUSIC_MANIFESTS` in `build/retail.py`). That was tried
  here: the DLLs load, but `CLTDirectMusicMgr::Init` fails. Tron 2.0's
  engine (NOLF2's generation) gives DirectMusic its own DirectSound object
  in `IDirectMusicPerformance8::InitAudio` (`InitPerformance`), and
  Microsoft's DirectMusic then asks that object for an interface that
  Wine's DirectSound does not have (IID
  `{2a8af120-e9de-4132-aaa5-4bdda5f325b8}`, `IDirectSoundConnect`, the sink
  of DirectX 8 audio paths; Wine's trace: "unknown IID"). The NOLF and
  AvP2 engines use the older `Init` and a port, which work. The plan: (a)
  in the modern engine, start DirectMusic in the older way when the
  audio-path start fails (`ltdirectmusic_impl.cpp`, `InitPerformance`;
  the default audio path and the reverb would change), then (b) install
  Microsoft's DirectMusic as nolfsrc does. With it, the music waves come
  to X-2's codec through ACM (no driver handle), which `make mp3acmtest`
  checks for all 1,317 music files. Until then the music is silent under
  Wine, in the legacy build too; on Windows the system's DirectMusic plays
  it, with X-2's codec.

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
| `ClientShellShared/ProfileMgr.cpp` | Apps (APPS-3: `ProfileName` in the registry), Input (CL-20: bindings), Display (new settings) | each its own settings; APPS-3 did not change the file (`CRegMgr` keeps its calls) |
| `TRON/TronGameServerShell.cpp` | Online (GS-1, GS-4), Frame rate (EN-45: the update) | separate functions |
| `TRON/BaseScreenJoin.cpp`, `TRON/TronCDKey.cpp` | Online (GS-3, GS-4), Apps (APPS-3: the CD key in the registry) | Online owns them; Apps gives the storage |
| `Launcher/DisplayMgr.cpp`, `DisplayDlg.cpp`, `OptionsDlg.cpp` | Display (CL-30, CL-31, CL-35), Apps (APPS-3: where the options are stored), Input (CL-20: Disable Joysticks) | Apps changes only the storage calls |
| `binkvideomgrimpl.cpp`, `InterfaceMgr.cpp` (movies) | Display (CL-32), Middleware (X-1) | Middleware replaces the DLL behind the same calls; Display changes the rectangle |
| `build/all.sh`, `build/variant.sh` | Middleware (X-1, X-2) | Middleware only |
| `build/playtest.sh`, `Makefile`, `README.md` | Middleware (X-1, X-2: `PLAYTEST_AUDIO`, `TRON20SRC_MIDDLEWARE`, `make mp3acmtest`) and others | each adds its own lines |
