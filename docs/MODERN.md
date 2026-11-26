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
| `Windowed` | int | (engine) | existing: 0 full screen, 1 a window; now set by the Display screen's and the launcher's "Window mode" and kept in `display.cfg` | CL-31 |
| `WindowMode` | int | 0 | with `Windowed` 1: 1 a framed window, 2 a borderless window over the desktop at the desktop's resolution; 0 (a config from before it): borderless only when the mode is as large as the desktop | EN-32, CL-31 |
| `AntiAliasFSOverSample` | int | 0 | existing: the samples of antialiasing (0, 2, 4, 8); the Display screen's "Antialiasing"; kept in `display.cfg` | CL-34 |
| `Anisotropic` | int | 0 | existing: now the degree of anisotropic filtering (0 off, 2, 4, 8, 16), held to what the card can do; the Performance screen's "Anisotropic filtering" | EN-34, CL-34 |
| `FovX`, `FovY` | float | 90, 78 | existing: the field of view in play, degrees, for a 4:3 screen (the engine widens it for the screen's shape, EN-30); the Display screen's "Field of view" sets `FovX` (60 to 120) and `FovY` with retail's shape of view | CL-33 |
| `Cinematics43` | int | 0 | 1: on a screen wider than 4:3, the sides beyond the centred 4:3 area are black while the cinematic letterbox is up; the Display screen's "4:3 cinematics" | CL-32 |
| `RawMouse` | int | 1 (no variable) | 0: the system mouse from DirectInput (EN-23), not from raw input | EN-24 |
| `JoystickDisable` | int | 0 | existing (the launcher's "Disable Joysticks" sets it through `DisableJoystick 1`); now also disables the controller, `##gamepad`, and the client's own reading of the pad | EN-21, CL-20 |
| `PadLookSpeed` | float | 1 | the controller's right stick: its turn rate, times 180 degrees a second across and 120 up and down; the Controller screen's "Look speed" (25 to 300) | CL-20, CL-23 |
| `PadLookCurve` | float | 2 | the right stick's response: its travel past the dead zone, raised to this power (1 is linear; less than 1 is taken as 1); "Look curve" (100 to 300) | CL-20, CL-23 |
| `PadInvertY` | int | 0 | 1: the right stick's up looks down; "Invert look" | CL-20, CL-23 |
| `PadLookDeadZone` | float | 0.15 | the right stick's radial dead zone (0 to 0.9 of its travel); "Look dead zone" (0 to 50) | CL-20, CL-23 |
| `PadMoveDeadZone` | float | 0.2 | the left stick's radial dead zone (0 to 0.9); "Move dead zone" (0 to 50) | CL-20, CL-23 |
| `PadHoldButtons` | int | 0 | the controller's buttons (XInput's bits) that the bindings do not see while the client reads them itself (a decision window); the client sets it, `autoexec.cfg` does not keep it; not a setting | EN-20, CL-22 |
| `PosLog` | float | 0 (no variable) | 1: write the player's position each frame to `poslog.txt`, and the light cycles' positions to `lcpos.txt` (the frame-rate tests, `build/fpscheck.py`) | EN-45 |
| `MusicPlayer` | int | 0 | the music: 0 DirectMusic, and the modern music player when DirectMusic does not start (Wine, Proton); 1 always the modern music player; 2 only DirectMusic (as retail). Read when the music starts (each level) | X-3 |

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
  loads that copy. Retail leaves the copy after the game stops (one for
  each start); GAME-35 deletes it. The imports of the
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
  sound driver). The music is X-3.
* **X-3: the music under Wine and Proton: our own music player in the
  engine.** The music of the game is DirectMusic segments
  (`MUSIC/<set>/*.sgt` in `Sound.rez`) with MP3 waves. Under Wine and
  Proton no DirectMusic plays them:
  * Wine's own DirectMusic starts the audio path, but plays nothing (the
    legacy build's main menu is silent).
  * Microsoft's DirectMusic (in the modern game directory, below):
    `IDirectMusicPerformance8::InitAudio` succeeds, but
    `CreateStandardAudioPath` fails with `E_NOINTERFACE`. The synthesizer
    asks the DirectSound object for `IDirectSoundConnect`
    (`{2a8af120-e9de-4132-aaa5-4bdda5f325b8}`), and Wine's DirectSound
    does not have it. Microsoft's `dsound.dll` has it, but the DirectX
    redistributable does not contain it.
  * DirectMusic started in the older way (`Init` and a synthesizer port,
    as the NOLF and AvP2 engines do; tried on the branch
    `m-finish-x3-attempt`, not kept): the port starts, but
    `IDirectMusicSegment8::Download` returns `E_NOTIMPL` for each segment
    with wave tracks, and the recording is silent. NOLF's and AvP2's music
    is DLS instruments, which a port plays. Tron 2.0's music is waves.
  * Microsoft's `dswave.dll` alone with Wine's other DirectMusic DLLs: the
    game crashes.

  **What the game uses.** The engine's `CLTDirectMusicMgr`
  (`ltdirectmusic_impl.cpp`) reads the level's control file (the client's
  `CMusic::InitLevel`: `MusicDirectory` and `MusicControlFile` of the
  world, `MenuMusicDir` and `MenuMusicCtrlFile` for the menus). The game
  calls `Init`, `InitLevel`, `TermLevel`, `Term`, `Play`, `Stop`,
  `ChangeIntensity` (from the server's `MUSIC I <n> measure` messages of
  the music moods, `ATTRIBUTES/MUSIC.txt`, from world commands and from
  the menu screens), `SetVolume` (the music volume, -3000 to 1000, plus
  the control file's `VOLUMEOFFSET`), `Pause` and `UnPause`;
  `PlaySecondary` and `PlayMotif` only from `MUSIC PS`/`PM` messages. A
  survey of the data (all of `Sound.rez`'s `MUSIC/`: 28 control files, 963
  segments, 1,317 waves; no other file type):
  * The control files use `NUMINTENSITIES`, `INITIALINTENSITY`,
    `INITIALVOLUME`, `VOLUMEOFFSET` (0, 200, -300, -600, 800), 594
    `INTENSITY` lines (each one segment; 564 go on to another intensity
    when the segment ends, 30 loop), 1,728 `TRANSITION` lines (all
    `MEASURE MANUAL`; 1,678 with a transition segment), and `REVERB OFF`.
    No file has `SECONDARYSEGMENT`, `MOTIF`, `STYLE`, `BAND` or
    `DLSBANK`.
  * The segments have a tempo track (961; 16 change the tempo), a time
    signature track (all; 110 change it; 32 beats a measure in the menu),
    and a wave track (941). The wave items are at music times (768 ticks
    a quarter note), all in all 32 variations, with no loop points, no
    pitch and no start offset; 28 have a volume, and parts have volumes
    (-180 to -1340, in 1/100 dB) on the PChannels 0 to 5. 909 waves are
    longer than their segment (they ring on into the next segment). No
    segment repeats or has loop points. 95 segments have a sequence track:
    only volume (CC 7) and expression (CC 11) curves (fades in the
    transitions), except `CORRUPT/WARNING1.sgt` (five notes, with no
    instrument). Two segments have an empty band track.
  * The waves are MP3 (format 0x55), stereo, 44100 Hz (the high quality
    set) or 22050 Hz (`LOW/`). The item durations are the wave lengths.
    27 references in `LOW/` name waves that do not exist.

  **The player.** `src/tron20src/music/musicplayer.cpp` and
  `musicplayer.h` (new; #included at the end and the start of
  `ltdirectmusic_impl.cpp` in `TRON20SRC_MODERN` blocks, so that
  `Lithtech.dsp` does not change), class `CModernMusicPlayer`:
  * `CLTDirectMusicMgr::Init` starts DirectMusic as retail does. When
    DirectMusic does not start, it starts the player
    (`ModernStartPlayer`), and DirectMusic is not tried again in the run
    (`s_bModernDirectMusicFailed`). Then each call of the interface
    (`InitLevel` to `GetVolumeOffset`) goes to the player
    (`m_pModernPlayer`). On Windows, DirectMusic starts, and nothing
    changes. The console variable `MusicPlayer` (below) selects the
    player on Windows too.
  * It reads the control file with the engine's `CControlFileMgrDStream`
    (as retail), the segments (RIFF `DMSG`: `segh`, `tetr`, `TIMS`,
    `wavt` with its parts and items and their `DMRF` file names, the CC 7
    and CC 11 curves of `seqt`), and the waves through the engine's file
    manager (from the rez files). It decodes MP3 with minimp3 (the copy of
    X-2, `src/tron20src/mp3/minimp3`, compiled again into `Lithtech.exe`)
    and reads PCM as it is.
  * The intensities, the transitions and the command queue are
    `CLTDirectMusicMgr`'s (the same results for the same calls): the
    transition segment, then the intensity's segments, the loop count, the
    next intensity. The next segment starts at the end of the segment
    (retail: at the notification that the segment almost ends, with the
    same result).
  * It mixes each wave at its time (from the segment's tempo map) for its
    duration, also past the end of its segment and after a change of
    segment (retail plays the segments with `DMUS_SEGF_NOINVALIDATE`), with
    the gain of its item, part and track, and of the PChannel's volume and
    expression ((CC 7 / 127)^2 (CC 11 / 127)^2; CC 7 starts at 100, CC 11 at
    127). It converts 22050 Hz to 44100 Hz (linear interpolation).
  * Enact times: "Immediately"; the next beat, grid or measure (from the
    time signature track of the playing segment, at or after the next
    sample to mix); the end of the segment; "Default" is the resolution in
    the header of the segment that starts (immediately when it has none).
    No segment has markers: "Marker" is immediately, as in DirectMusic.
  * `SetVolume` is DirectMusic's master volume (1/100 dB), `Stop` fades
    the waves out in 10 ms, `Pause` stops the mix where it is and
    `UnPause` continues from there.
  * The output is a DirectSound object of its own and a looping secondary
    buffer (44100 Hz, 16 bits, stereo, one second); a thread keeps about
    150 ms of mix ahead of the play cursor, and writes silence after the
    mix, so that a late mix does not play old data again.
  * What is approximate or not reproduced: the absolute level (CC 7 at
    100, as the GM synthesizer; no Windows recording to compare); the
    exponential and logarithmic curve shapes (Tron uses only instant,
    linear and sine); a change that comes while an earlier change waits
    for its time replaces it; retail's `Pause` stops DirectMusic and starts
    the segment again (the waves that played before the pause point do not
    play), the player continues all waves; a secondary segment with
    "Segment" plays at the end of the primary segment (retail: at the next
    measure after the notification); the notes of a sequence track (no
    instrument), clock-time tracks (none), DirectMusic's random variations
    (all items are in all variations), and the reverb (off in every
    control file). `PlayMotif` fails, as retail without styles.
  * Retail bugs that the player does not have (logged in
    `RETAIL_BUGS.md`; the DirectMusic path keeps them): `Stop` while the
    music is paused plays the segment again; a change of intensity in the
    queue that fails is tried again without end.
  * The player's trace (for a debugger, or Wine's `+debugstr`): the start
    of DirectSound, the control file's counts, each call, each change of
    intensity and the start of each segment ("LTDirectMusic: modern music
    player: ..."). The same lines go to the console at `LTDMConsoleOutput`
    4.

  How it was tested (under Wine 9.0 in the build container,
  `PLAYTEST_AUDIO` and `PLAYTEST_WINEDEBUG=+debugstr`; the recordings are
  48000 Hz float, which Wine makes from all of the game's sound; under
  Xvfb the null sound device takes data faster than in real time, so a
  recording holds more music than the run's time):
  * The main menu (`build/playtest-music.txt`): `menu.txt` (2
    intensities, 5 segments, 8 waves); intensity 1 (`Silence.sgt`), then
    2 (`MenuExp1.sgt` to `MenuExp4.sgt`, 24 s each, then again). Each of
    the eight waves (`MENUE1.wav` to `MENUE8.wav`, decoded with minimp3
    outside the game) is in the recording, one after the other, 12 s
    apart, as the segments put them: the normalized correlation of 2 s of
    each wave with the recording is 1.00. The legacy build's recording of
    the same script has only the menu's sound in the first 8 s, then
    silence (-97 dB): unchanged.
  * A level with music, `build/playtest-music-level.txt` (new: M04-01,
    `bradley.txt`, `MinMusicMood` "Investigate"): the server sends
    `MUSIC I 9 measure`; the player plays the transition segment from 1
    to 9 (`StoW.sgt`), then `Warning1.sgt`, and the queue changes the
    intensity to 10, 11, 12, 13 and 9 again, each at the end of its
    segment. In the recording, `WARNINGA.wav` (correlation 1.00, two
    times, one for each round of the intensities), `WARNINGB.wav` (0.98),
    `WARNINGH.wav` (0.96, intensity 10), and `STOW.wav` (0.59: it is 7.8 s
    long, and `WARNINGA.wav` starts 2.2 s into it).
  * M01-01 and the light-cycle race (Newbie Authentication,
    `ancient.txt`) start with intensity 1 (`Silence.sgt`), then the game
    calls `ChangeIntensity(0)` (`CMusic::Play` with no intensity yet),
    which stops the music. M01-01's intro sends `setmood 0` and
    `lockmood`, and a later command `setmood 3` (combat); the race has no
    music moods. The same calls come with retail's game DLLs
    (`build/control.sh` with this engine and `+MusicPlayer 1`), so this is
    the game's design, not the player.
  * Cost: minimp3 without SIMD decodes 1,636 s of the game's music in
    1.27 s of one core (0.08 % of a core for each wave that plays). The
    frame-rate tests (`build/playtest-fps-walk.txt`,
    `playtest-fps-race.txt` at `MaxFPS` 60 and 0, `build/fpscheck.py`)
    give the same movement at 60 frames a second and without a limit
    (854 and 667 frames a second, the world not drawn), as before.
  * `make tests` with the modern build: all ALIVE.

  nolf2src's and cjacksrc's music is also DirectMusic in the same engine
  code (`ltdirectmusic_impl.cpp`); if their segments are wave tracks, they
  can use this player as it is (two files and the calls in
  `CLTDirectMusicMgr`).

  **Microsoft's DirectMusic in the modern game directory** (kept), as
  nolfsrc's M-P4-12 and avp2src's M-X4: the nine DLLs of DirectX 9.0c from
  `ext/toolchains/directmusic-5.3.1.904/` as `ms_<name>.dll`
  (`THIRD-PARTY` rows of `MANIFEST.txt` with their source file), and
  `Lithtech.exe.manifest` (registration-free COM, `ours` row;
  `DIRECTMUSIC_MANIFESTS` in `build/retail.py`;
  `src/modern/directmusic/README.md`). They are in the modern public
  archive (`build/pubcheck.py` accepts the `ours` row; `docs/
  LICENSE_NOTES.md`). On Windows they are the DirectMusic that Windows
  has.

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
  `CServerDiscMatchMgr` update, retail 0x1004a67c; the retail code reads
  the next node only before the loop).
* **GAME-34:** the FOV check compares the camera's radians with
  `FovX`/`FovY` in radians, so the FOV is not set again each frame
  (`GameClientShell.cpp` `UpdateDebugInfo`, retail 0x100e57e4).
* **GAME-35:** the ClientFX DLL (the copy of `ClientFx.fxd` in the
  temporary directory, `cfx*.tmp`) is unloaded, and its copy deleted, in
  `CGameClientShell::OnEngineTerm`, with the game's and the interface's
  effects (`GameClientShell.cpp`, `InterfaceMgr.cpp` `Term`). Retail
  unloads it only from the static `CClientFXDB`'s destructor while
  `CShell.dll` unloads; the loader defers that `FreeLibrary`, and the copy
  stays (one for each start). Tested: a start and Quit from the main menu
  in a new Wine prefix leaves `Temp\cfx9ce3.tmp` with the legacy build and
  no file with the modern build.

### Retail-bug fixes, engine

* **EN-3:** the `VersionInfo` console command does not call a null pointer
  (`clientde_impl.cpp`, `consolecommands.cpp`).
* **EN-4:** the music thread's handle is closed (`ltdirectmusic_impl.cpp`
  `Term`).
* **EN-5:** an intensity change without a transition tests the segment
  state (`ltdirectmusic_impl.cpp` `ChangeIntensity`). The path with a
  transition has the test in retail too (cjacksrc's EN-5 was for that
  path); it is now in both builds.
* **EN-7:** no sound driver: no `Term` through a null pointer
  (`soundmgr.cpp` `SoundSys`).
* **EN-8:** DrawPrim uses its camera's viewport (`render_b/.../
  d3ddrawprim.cpp`).
* **EN-10:** `StopSecondary` and `StopMotif` walk their list once
  (`ltdirectmusic_impl.cpp`).
* **EN-11:** the light radius accessors and `GetSurfaceDims` test their
  pointers (`clientde_impl.cpp`, `winclientde_impl.cpp`).
* **EN-12:** `InitLevel`'s trace says `sDefine3` (`ltdirectmusic_impl.cpp`).
* **EN-13:** the console prints do not write past their buffers
  (`clientde_impl.cpp` `CPrint`, `dsys_interface.cpp`
  `dsi_PrintToConsole`).
* **EN-14:** the engine's error messages name their own function
  (`clientde_impl.cpp`, `winclientde_impl.cpp`, `client_iltphysics.cpp`,
  `ltinfo_impl.cpp`, `soundmgr.cpp`).
* **EN-15:** `streamsim_Open` closes its file on an error
  (`streamsim.cpp`).
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

  `RETAIL_BUGS.md` has each of these faults: from the byte-identical
  functions (`UpdatePushers`, `MoveLocalSolidObject`, `UpdateInLiquid`,
  `BounceMovingObject`, `CFireFX::Update`, `CParticleSystemFX::Update`,
  `CLineSystemFX::Update`, and the engine's `pd_Update`, 0x467630), and
  from the retail code of the functions that are not yet byte-identical
  (`UpdateMotion`, `UpdateNormalMotion`,
  `CTronPlayerMgr::TronUnknownVirtual64`, `CClientLightCycleMgr::Update`;
  the entries give the retail addresses).

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

### Display

The game runs at every resolution the display offers, at any shape, full
screen, in a window or in a borderless window, with a field-of-view
setting, antialiasing up to 8x, anisotropic filtering up to 16x and 32-bit
colour. The engine's widescreen and window code came from cjacksrc's
(EN-2, EN-30 to EN-33); the rest is Tron 2.0's. Strings 31000 to 31015
(the block 31000 to 31049 is the Display's; `ClientRes/Shared/
ClientResShared.h`, `ClientRes/TRON/ClientRes.rc`); the launcher's
31000 to 31003 (`Launcher/TRON/resource.h`, `Launcher.rc`).

* **EN-30: widescreen field of view (Hor+).** Checked, kept and extended.
  A view drawn over the whole screen wider than 4:3 keeps the vertical
  angle and gets a wider horizontal one; the camera's own angles are not
  changed. Now also a screen narrower than 4:3 (5:4, 1280x1024) keeps the
  horizontal angle and gets a taller vertical one (Vert+), so nothing is
  stretched up and down (`client/src/clientmgr.cpp` `cm_WidescreenFovX`,
  `cm_NarrowscreenFovY`, `CClientMgr::Render`). The interface camera's art
  follows the same view (`CInterfaceMgr::GetWorldFromScreenPos`,
  `GetScreenFromWorldPos`). The I/O node camera has its own rectangle and
  is not widened.
* **EN-31: a window at any size.** Checked, kept: a 1280x720 window on a
  1920x1080 desktop (Xvfb has one mode only) starts and plays.
* **EN-32: the window mode.** Checked, kept. The game no longer resizes the
  window at the start (retail's `SetWindowSize` gave a framed window the
  full size of the mode, so the picture lost the frame;
  `GameClientShell.cpp` `OnEngineInitialized`), and a window after full
  screen is not held to the full-screen rectangle (`ClipCursor`,
  `OnEvent` `LTEVENT_RENDERINIT`).
* **EN-33: antialiasing in a window too.** Checked, kept. With
  multisampling the back buffer is not lockable (Direct3D 9 refuses the
  two together; the retail flag `D3DPRESENTFLAG_LOCKABLE_BACKBUFFER` is
  left out). See CL-34 for what locked it.
* **EN-2: the window keeps its renderer.** Kept as cjacksrc's
  (`kernel/src/sys/win/client.cpp`).
* **CL-30: the interface at any shape of screen.** Retail scaled the
  640x480 page by width/640 and height/480, so at any shape but 4:3 every
  menu and HUD item was stretched. The page is now scaled the same both
  ways (the smaller of the two ratios) and centred: the margins are
  `CInterfaceResMgr::GetXOffset`, `GetYOffset` (static members: the class
  keeps retail's size). The controls add them (`CLTGUICtrl::SetScale`,
  `SetPageOffset`; the button's and the list's own `SetScale`), so do
  `ConvertScreenPos`, `ConvertScreenRect` and each place that puts text or
  polygons on the page itself: the help text, the loading and post-load
  screens, the credits, the subroutines screen (arcs, libraries,
  procedurals, the rating bars, the duplicate dialog), the host, join and
  light cycle screens' tabs, the audio and light cycle screens' pictures,
  the failure and victory screens, the HUD messages and subtitles, the
  mission text, the chat input, the disc scores' legend. The HUD keeps to
  the screen's edges: an item in the left third of the page stays at the
  left edge, one in the right third moves with the right edge, one in the
  middle stays centred (`HUDX`, `HUDY`, `GetHUDXOffset`, `GetHUDYOffset`;
  the same up and down on a 5:4 screen): the health and energy bars,
  armor, weapon, permissions, build notes, version, distance, damage,
  meter, damage direction, crosshair texts, pings, match statistics, the
  single string, the profiler, the security camera's overlay (the whole
  screen), the I/O node camera, and the light cycle HUD (speedometer,
  roster, lives, power-up icon, circuit intro). The HUD lays itself out
  again when the screen changes (`CHUDMgr::ScreenDimsChanged`), and the
  screens and menus scale again each time they open (two screens of one
  height have the same scale). The Display screen lists every 32-bit mode
  at least 640x480 at any shape (retail: exact 4:3 only), and the mode
  the game runs at (a window's size) (`ScreenDisplay.cpp`
  `GetRendererData`; files: `InterfaceResMgr.cpp/.h`, `InterfaceMgr.cpp`,
  `TronInterfaceMgr.cpp`, `BaseScreen.cpp`, `BaseMenu.cpp`,
  `MenuSystem.cpp`, `MenuIntel.cpp`, `MenuBuildNotes.cpp`, `MenuHelp.cpp`,
  `HUDMgr.cpp`, the `HUD*.cpp` named above, `LoadingScreen.cpp`,
  `ScreenPostload.cpp`, `Credits.cpp`, `Libs/LTGUIMgr/ltguictrl.cpp/.h`,
  `ltguibutton.cpp`, `ltguilistctrl.cpp` and the TRON screens and
  controls named above). The launcher's Display dialog lists every 32-bit
  mode too, and the config's size if the adapter has no such mode
  (`Launcher/DisplayMgr.cpp` `CRenderer::Init`, `ReadConfigFile`).
* **CL-31: window mode.** The Display screen's "Window mode", under the
  resolution: Full screen, Window, Borderless window (`Windowed`,
  `WindowMode`). Borderless greys the resolution out (the desktop's is
  used). Leaving the screen with another window mode or antialiasing
  restarts the renderer once; the interface, the camera and the profile
  then follow the mode the renderer got (`ProfileMgr.cpp`
  `CUserProfile::ApplyDisplay`, `g_bModernDisplayRestart`). A framed
  window larger than the desktop's work area takes the largest listed
  size that fits, else the largest of the desktop's shape (a window size
  the adapter has no mode for goes to the renderer as it is, EN-31).
  `display.cfg` keeps `Windowed`, `WindowMode` and `AntiAliasFSOverSample`
  (`SaveDisplaySettings`). The launcher's Display dialog has the same
  choice in a list under the renderer (the resolution list greys out for
  Borderless); it reads the mode from `display.cfg` and passes `+Windowed`
  and `+WindowMode` with the resolution (`Launcher/DisplayDlg.cpp/.h`,
  `DisplayMgr.cpp/.h` `GetDisplaySettingsString`).
* **CL-32: 4:3 cinematics.** With EN-30 a cinematic on a wide screen
  shows more at the sides than it was framed for. "4:3 cinematics"
  (`Cinematics43`, off by default) covers the sides beyond the centred 4:3
  area while the letterbox is up, fading with it
  (`InterfaceMgr.cpp` `UpdateLetterBox`). The Bink movies (the logos at
  the start) already keep their own shape, centred (`binkvideomgrimpl.cpp`
  fits them to the screen), so they need no change; X-1 keeps that
  rectangle.
* **CL-33: field of view.** The Display screen's "Field of view", 60 to
  120 degrees in steps of 5 (90 by default): `FovX`, the horizontal angle
  for a 4:3 screen, and `FovY` with retail's shape of view (90 by 78 has
  the tangents' ratio of `FovY`); the engine widens the view for the
  screen's shape (EN-30). The game sets the camera from `FovX`/`FovY`
  whenever it may change the view, so a new value applies at once in
  play; the zooms work from it. The light cycles keep their own
  `FovCycleX`/`FovCycleY`. (`ScreenDisplay.cpp` `ModernApplyOptions`.)
* **CL-34: antialiasing up to 8x and anisotropic filtering up to 16x.**
  The Display screen's "Antialiasing": Off, 2x, 4x, 8x
  (`AntiAliasFSOverSample`; the renderer restarts). What locked the back
  buffer, and fails with multisampling, goes another way: the game's
  surface blits to the screen (the pause and splash pictures, fades) are
  drawn as textured polygons, in a scene of their own if they come outside
  one, and the screenshot commands read a resolved copy
  (`render_a/.../d3d_surface.cpp` `d3d_BlitToScreen`,
  `d3d_MakeScreenShotBMP`, `d3d_MakeScreenShotTGA`). A `FillRect` of the
  screen (the pause screen's background) clears the rectangle on the
  device when the back buffer cannot be locked
  (`winclientde_impl.cpp` `cis_FillRect`). The engine console does not
  lock the screen: it draws with DrawPrim and the font manager (its
  background through `BlitToScreen`), and the shipped engine never draws
  it (`MaybeDrawConsole` is not called, and the tilde key is only for a
  build without `_FINAL`, retail too). No other function that locks the
  screen (`cis_LockSurface`: warps, masked draws, `SetPixel`) is called
  on the screen by Tron 2.0's game DLLs. Tested with
  `AntiAliasFSOverSample 4`: the pause screen of M01-01 is as without
  antialiasing (the paused picture covers the fill). A sample count the card cannot do falls back to the next
  lower (retail's `GetDefaultMultiSampleType`; under Wine with Mesa's
  software renderer 2 falls to `D3DMULTISAMPLE_NONMASKABLE`). The
  Performance screen's "Anisotropic filtering": Off, 2x, 4x, 8x, 16x; the
  setting is a level and `Anisotropic` the degree (EN-34), and a retail
  configuration's 1 ("on") is 2x (`PerformanceMgr.cpp`
  `ModernVarFromSetting`, `ModernSettingFromVar`;
  `TRON/ScreenPerformance.cpp`). A modern-debug build writes the
  presentation parameters, each device creation and reset, and the
  samplers' anisotropy to `modern-d3d.log` beside the executable
  (`d3d_device.cpp`).
* **CL-35: 32-bit colour by default.** The launcher's config reader starts
  at 32 bits (retail: 16), and its default resolution is 32-bit: the
  desktop's if listed, else 1024x768, 800x600, 640x480 (retail looked for
  16-bit modes, which the list never has) (`Launcher/DisplayMgr.cpp`
  `ReadConfigFile`, `SetDefaultParameters`).

Tested (the modern build, under Xvfb at each size; every shot looked at):
the menus, the first level and a light cycle race at 1280x720, 1920x1080,
2560x1080 and 1280x1024: the menus centred and not stretched, the HUD at
the screen's edges and the light cycle HUD too, the view widened (Hor+;
Vert+ at 5:4). `build/playtest-display.txt` (`PLAYTEST_RES=1280x720
PLAYTEST_SCREEN=1920x1080 PLAYTEST_WINDOWED=1`): a framed window, then
Borderless window (the window covers the 1920x1080 desktop, the game runs
at 1920x1080), then Window again (a framed window that fits).
`build/playtest-fov.txt` at 2560x1080: the slider from 90 to 110 in play,
the view wider. `Cinematics43 1` with `LetterBox 1`: black sides beyond
the 4:3 area. `build/playtest-aa.txt` on modern-debug, and `AntiAliasFSOverSample 4`,
`Anisotropic 16` in a config: `modern-d3d.log` shows the device created
with 4x multisampling and no lockable flag, a reset with the Display
screen's 2x, and `MAXANISOTROPY 16` with the anisotropic filter; the game
draws its menus, HUD and the level with it. `build/playtest-launcher-display.txt`
(`make launchertest LAUNCHER_EXE=TronLauncher.exe`): Borderless window in
the dialog, the game starts with `+Windowed 1 +WindowMode 2`. Not
testable here: real antialiasing and anisotropic filtering on a graphics
card (Wine's software rendering does not show them clearly: the log is
the check), more than one monitor.

### Input

The mouse turns the view by its own counts, each one time, at all frame
rates. A controller (an XInput pad: an Xbox-style pad, or what Steam Input
gives) plays the game, the light cycles and the menus. The engine half of
the controller (EN-20 to EN-22) came from cjacksrc's; the game half is
Tron 2.0's own, made with the design of cjacksrc's CL-20 to CL-24 and
nolf2src's M-G47, M-G60 to M-G63. The strings are 31300 to 31316 (the
block 31300 to 31399 is the controller's).

* **EN-24: raw mouse input.** The system mouse (`##mouse`) comes from
  `WM_INPUT` and not from DirectInput: the counts of the mouse, with no
  pointer acceleration. The window procedure gives each `WM_INPUT` to
  `input_ModernRawInput` (`client.cpp`, the `WM_INPUT` case only). It adds
  the motion and keeps the buttons and the wheel in their sequence. Each
  frame `input_ReadInput` gives them to the mouse's triggers as the events
  that DirectInput gives (`input_ModernRawEvents`). Thus the bindings, the
  scale of the look (`MouseSensitivity`) and EN-23 do not change. On
  Windows a DirectInput count is the count of the mouse too. DirectInput's
  mouse is not acquired while the game reads raw input
  (`input_ModernRawRelease`): DirectInput 8 reads the mouse through raw
  input itself, and a process has only one raw input target for the mouse.
  The first test showed this under Wine: after DirectInput acquired the
  mouse, the game's window got no more `WM_INPUT`. Configure controls'
  capture still uses DirectInput's mouse; after it, the raw input is
  registered again. There is no `RIDEV_NOLEGACY`, thus the menus keep
  their cursor. With `RawMouse 0`, when raw input cannot be registered,
  or after an absolute device (a tablet, a remote desktop) sends raw
  input, the mouse is DirectInput's (EN-23). (`kernel/src/sys/win/
  input.cpp`: `input_Init`, `input_Term`, `input_ReadInput`,
  `input_FlushInputBuffers`, the `Acquire` calls; `input.h`; `client.cpp`
  `MainWndProc`.) The Mouse screen has no "Input rate" now: it set the
  smoothing that the mouse look does not have (`TRON/ScreenMouse.cpp`).

  Tested (`build/playtest-mouselook.txt` and a script with five motions
  of 60 counts, cut into 1, 2, 5, 10 and 1 xdotool steps; the turn
  between the shots measured by `build/yawshift.py`, M01-01): a
  temporary log in `input_ModernRawEvents` showed every xdotool step as
  one raw event with its count (`x=6` ten times, `x=60`), and the mouse
  buttons (down, up) through the same path. The turn is linear in the
  counts: 9.8, 4.8, 9.6, 8.7 and 9.6 degrees, about 0.16 degrees a count
  at the default sensitivity, the same as DirectInput's (EN-23,
  `RawMouse 0`: 9.8 and 9.6 degrees for the single steps). Some of the
  smaller steps do not arrive at all (30 of the 60 counts of the two-step
  motion): under Wine the raw input comes from the pointer's position, and
  the engine puts the cursor back in the centre each frame; with
  DirectInput the loss was larger (5.6 and 6.8 degrees for the 5- and
  10-step motions). After each motion the view stays (no repeated last
  event). The legacy build, for comparison (RETAIL_BUGS.md, "The mouse
  look counts a mouse event more than one time"): 27.8 degrees for one
  step, 7.3 degrees to the left for two steps to the right. Not tested: a
  real mouse on Windows.
* **EN-20, EN-21, EN-22: checked and kept** (the engine half of the
  controller; see "Inherited engine items"). The test pad drives the
  `##gamepad` device in all the controller tests below. EN-21 now also
  applies the launcher's "Disable Joysticks" ("all joysticks and
  gamepads") to `##gamepad`, the only one that is left; the client then
  does not read the pad either (`input_ReadInput`; `clientpad.cpp`).
* **EN-23: checked and kept.** Tron 2.0's retail `input_ReadInput`
  (0x432d70) is byte-identical to ours, thus the bug is in Tron 2.0
  (RETAIL_BUGS.md).
* **CL-20: the controller's sticks.** The client reads the sticks itself
  (`src/tron20src/xinput/clientpad.cpp`, compiled as part of
  `GameClientShell.cpp`; `clientpad.h`), once a frame, in the menus too,
  while the game's window is in front (`CGameClientShell::Update`; the
  pad's scan thread stops in `OnEngineTerm`). The sticks have no bindings
  (`CUserProfile::ApplyJoystick` binds no axis). **Look**: the right stick
  past a radial dead zone (`PadLookDeadZone`), the rest of its travel
  rescaled to 0..1 and raised to `PadLookCurve`, is a turn rate: 180
  degrees a second across and 120 up and down, times `PadLookSpeed` and
  the frame time. It is added to the mouse's turn, slowed by the zoom as
  the mouse is, also with "Mouse look" off; up and down are the stick's
  own (`PadInvertY`), held as the mouse's are (level on a light cycle in
  first person, 45 degrees in third person) (`PlayerMgr.cpp`
  `CalculateCameraRotation`). **Movement**: the left stick past
  `PadMoveDeadZone` accelerates the way it points and sets the movement
  flags by octant (for the animations and the server); past three
  quarters of its travel it runs, below that it walks at that part of the
  walking speed (one third at least); the Walk binding held turns that
  over; not while ducking, carrying or leaning (`CMoveMgr.cpp`
  `UpdateNormalControlFlags`, `UpdateNormalMotion`, `GetMaxVelMag`).
* **CL-21: the controller's default layout.** One time for each profile
  (`[Controls] PadLayout`), the controller's column of the bindings gets:
  right trigger Fire, left trigger Disc recall / block, A Jump, B Crouch,
  X Action, Y Disc attack modifier, right and left bumper Next and
  Previous weapon, left stick click Walk, right stick click Center view,
  d-pad up and down Zoom in and Zoom out, d-pad left Holster weapon, d-pad
  right Toggle status display. A command that the player bound on the
  controller, and a button that is bound, do not change. A new profile and
  Restore defaults get the layout too (`Default.prf` has no `PadLayout`).
  Back opens the subroutine screen, as F1 does: "Subroutines" is not a
  command of Configure controls, thus `ApplyBindings` binds Back to it
  while no command of the profile has Back. Start is the menu (CL-22).
  (`clientpad.cpp` `Pad_DefaultBindings`; `ProfileMgr.cpp` `LoadControls`,
  `ApplyBindings`.)
* **CL-22: menus, cut-scenes and decisions with the controller.** Out of
  play (the menus and screens, a message box, the subroutine screen, a
  paused game, the loading screens, a movie) the d-pad or the left stick
  is the arrow keys (they repeat after 0.4 s, every 0.12 s), A is Enter,
  B and Start are Escape. In play Start is Escape (the menu), and A in a
  cut-scene or when the player is dead is Space. While a decision window
  is open (a conversation, or an archive bin's downloads) the d-pad up and
  down move a highlight (amber) over its choices, A takes the choice and
  B is Escape. The bindings do not see A, B and the d-pad then
  (`PadHoldButtons`). The keys go through the client shell's own
  `OnKeyDown` and `OnKeyUp`, as the engine sends a key. Not while
  Configure controls waits for a control to bind (`ScreenConfigure.cpp`).
  (`clientpad.cpp` `Pad_Update`; `HUDDecision.cpp`, `.h`,
  `TRON/HUDTronDecision.cpp`, `.h`: `PadIsChoice`, `PadHighlight`.)
* **CL-23: the Controller screen.** Options, Controls has no "Use
  joystick/gamepad" and "Configure joystick/gamepad" now (an axis bound
  there would move a second time) but "Controller": the joystick screen,
  made again with Look speed (25 to 300), Look curve (100 to 300), Invert
  look, the look and move dead zones (0 to 50) and Restore controller
  defaults (only the controller's column, after a yes/no box). The values
  are the console variables above, kept in `autoexec.cfg` beside the
  executable (set with "+"), not in the registry. Configure controls names
  the third column "Controller" ("Controller A Button").
  (`TRON/ScreenControls.cpp`, `TRON/ScreenJoystick.cpp`, `.h`,
  `TRON/ScreenConfigure.cpp`; `ClientResShared.h`, `TRON/ClientRes.rc`.)
* **CL-24: a gamepad's buttons are bound by name.** Configure controls
  bound a gamepad's button by its instance number, which the engine takes
  for an axis (RETAIL_BUGS.md). The modern build binds a joystick's and a
  gamepad's controls by their names (`TRON/ScreenConfigure.cpp` `Bind`).
* **CL-25: the light cycle with the controller.** The left stick pushed to
  the left or the right (more than half its travel; then back near the
  centre before the next turn), or the d-pad's left or right, turns the
  cycle: the Strafe left and Strafe right commands, which are the turns of
  the keyboard. The right trigger or the left stick up makes the cycle
  faster, the left trigger or the left stick down makes it slower (the
  flags of Forward and Backward). The right stick turns the camera, as
  the mouse does. (`clientpad.cpp`; `VehicleMgr.cpp`
  `UpdateLightCycleControlFlags`.)

Tested with the test pad (`TRON20SRC_PADTEST`, the `pad` and `tap`
commands of the playtest scripts), looked at in the shots:

* `build/playtest-pad-menus.txt`: the d-pad and the left stick move
  through the main menu, Options and Controls; A opens Options, Controls
  and Controller; on the Controller screen the d-pad right changes Look
  speed from 100 to 150, and the value is there again after B and A;
  Configure controls shows the default layout ("space, Controller A
  Button"); B goes back to the main menu.
* `build/playtest-pad-level.txt` (M01-01): the right stick turns the view
  to the right and back to the left, the left stick walks forward, Start
  opens the in-game menu and B closes it. A goes past the "Press any key
  to continue" screen. The right trigger's throw of the disc is not seen
  in the shots.
* `build/playtest-pad-race.txt` and a comparison of three runs of the
  first race (the stick to the left and the d-pad right, the keys A and D,
  and no input; shots 0.7 s after each turn): the pad's views are like the
  keys' views and not like the views with no input (mean difference 0.145
  and 0.195 between pad and keys, 0.20 to 0.22 to no input). The speed
  (the triggers) is not measured.

Not tested: a real controller (none under Wine and Xvfb), the decision
windows with the pad (no decision in the first minutes of the first
level), Configure controls' capture of a pad button, and the Back binding.

### Inherited engine items

The engine came from cjacksrc's with its modern code. Each item is now
checked against Tron 2.0's own engine: the retail `Lithtech.exe` (its
functions that our legacy build compiles byte-identical, and its
disassembly where a function was not), with the developer build of
2003-08-05 as a second witness. Each retail bug among them has its entry
in `RETAIL_BUGS.md`.

| Id | What (cjacksrc's or nolf2src's item) | Files | Owner |
|---|---|---|---|
| EN-2 | in a window the game keeps its renderer when it loses the focus (cjacksrc EN-2) | `kernel/src/sys/win/client.cpp` | Display: done (above) |
| EN-4 | the music thread's handle is closed (cjacksrc EN-4); checked: `Term` (0x43e340) is byte-identical, retail does not close it; kept, RETAIL_BUGS.md | `ltdirectmusic_impl.cpp` | done (above) |
| EN-5 | a failed transition segment is tested (cjacksrc EN-5); checked: Tron's retail `ChangeIntensity` (0x43f490) tests the transition's segment state (0x43f6c5), so that test is now in both builds (the function became byte-identical); the path without a transition has no test in retail (0x43f7e5): the modern build tests it; RETAIL_BUGS.md | `ltdirectmusic_impl.cpp` | done (above) |
| EN-7 | no sound driver: no `Term` through a null pointer (cjacksrc EN-7); checked: `SoundSys` (0x4b1890) byte-identical, the call at 0x4b1926; kept, RETAIL_BUGS.md | `sound/src/soundmgr.cpp` | done (above) |
| EN-10 | `StopSecondary`/`StopMotif` walk their list once (nolf2src M-E7); checked: both (0x43fb50, 0x43fec0) byte-identical; kept, RETAIL_BUGS.md | `ltdirectmusic_impl.cpp` | done (above) |
| EN-11 | null checks in the client interface (nolf2src M-E8); checked: `ci_GetLightRadius` (0x40b2e0), `ci_SetLightRadius` (0x40b300), `cis_GetSurfaceDims` (0x495e30) byte-identical; kept, RETAIL_BUGS.md | `clientde_impl.cpp`, `winclientde_impl.cpp` | done (above) |
| EN-12 | `InitLevel`'s trace text (nolf2src M-E9); checked: `InitLevel` (0x43e570) byte-identical, "sDeine3" at 0x5b36e4; kept, RETAIL_BUGS.md | `ltdirectmusic_impl.cpp` | done (above) |
| EN-13 | the console print does not write past its buffer (nolf2src M-E10); checked: `CPrint` (0x408740), `dsi_PrintToConsole` (0x42a630) byte-identical; kept, RETAIL_BUGS.md | `clientde_impl.cpp`, `dsys_interface.cpp` | done (above) |
| EN-14 | error messages name their own function (nolf2src M-E11); checked: the ten functions byte-identical, the wrong names referenced from them and the right ones not in the image; kept, RETAIL_BUGS.md | `clientde_impl.cpp`, `winclientde_impl.cpp`, `client_iltphysics.cpp`, `ltinfo_impl.cpp`, `soundmgr.cpp` | done (above) |
| EN-15 | `streamsim_Open` closes its file on an error (nolf2src M-E12); checked: `streamsim_Open` (0x485480) byte-identical; kept, RETAIL_BUGS.md | `streamsim.cpp` | done (above) |
| EN-20 | `##gamepad`: the XInput pad as a DirectInput gamepad (cjacksrc EN-20; `src/tron20src/xinput/`); checked, kept; the game half is CL-20 to CL-25 | `kernel/src/sys/win/input.cpp` | Input: done (below) |
| EN-21 | DirectInput joysticks are left out (cjacksrc EN-21); checked, kept; `JoystickDisable` also disables `##gamepad` | `input.cpp` | Input: done (below) |
| EN-22 | no DirectInput device enumeration (cjacksrc EN-22); checked, kept | `input.cpp` | Input: done (below) |
| EN-23 | mouse look: the whole buffer once a frame, no smoothing (nolf2src M-E3); checked, kept; the retail bug is in RETAIL_BUGS.md | `input.cpp` | Input: done (below) |
| EN-30 | widescreen field of view, Hor+ (cjacksrc EN-30) | `client/src/clientmgr.cpp` (`cm_WidescreenFovX`, `CClientMgr::Render`) | Display: done (above) |
| EN-31 | a window at any size (cjacksrc EN-31) | `render_a/.../d3d_shell.cpp` | Display: done (above) |
| EN-32 | the window mode: framed or borderless (cjacksrc EN-32) | `kernel/src/sys/win/render.cpp`, `render_a/.../common_init.cpp` | Display: done (above) |
| EN-33 | antialiasing in a window too; 2D at whole pixels (nolf2src M-E13) | `d3d_device.cpp` (`SetPresentationParams`, `TryFallingBack_OnFailedDevCreate`), `render_b/.../d3ddrawprim.cpp` | Display: done (above) |
| EN-40 | the frame limiter, `MaxFPS` 240 by default (cjacksrc EN-40); checked, and changed by EN-44: no limit of 60 in multiplayer | `clientmgr.cpp` (`cm_ModernFrameLimit`, `CClientMgr::Update`), `shared/src/engine_vars.cpp` | Frame rate: done (below) |
| EN-41 | a game tick at most every 10 ms (cjacksrc EN-41); checked, kept | `server/src/servermgr.cpp` | Frame rate: done (below) |
| EN-42 | the frame-time log, `FrameTimeLog` (cjacksrc EN-42); checked, kept | `shared/src/sys/win/modernframelog.h`, `clientmgr.cpp`, `clientmgr.h`, `clientshell.cpp`, `d3d_device.cpp`, `rendershadowlist.cpp`, `engine_vars.cpp` | Frame rate: done (below) |
| EN-43 | V-sync off in a window under Wine (cjacksrc EN-43); checked, kept | `d3d_device.cpp` (`SetPresentationParams`) | Frame rate: done (below) |
| EN-60 | a music intensity out of range (cjacksrc EN-60); checked: not a retail bug in Tron 2.0: retail inlines `GetTransition` with the index check (0x43f57c); the check is now in both builds, and EN-60 is removed (its id is not used again) | `ltdirectmusic_impl.cpp` | removed |
| EN-61 | projected shadows' culling buffer (cjacksrc EN-61); kept | `modelshadowshader.cpp`, `rendershadowlist.cpp` | Frame rate: done (below) |
| EN-62 | V-sync in a window caps the rate, it does not wait (cjacksrc EN-62); checked, kept | `d3d_device.cpp`, `clientmgr.cpp`, `modernframelog.h` | Frame rate: done (below) |

How they were checked: `out/work/srvscore.py Lithtech` on the legacy
build gives the retail address and status of each function; a function
that is byte-identical with the retail code in its `#else` has the fault.
For the messages of EN-14, the strings that the functions reference in
the retail image were checked too. `ChangeIntensity` was near: its retail
disassembly showed the two checks that cjacksrc's EN-5 and EN-60 add
(Tron 2.0's engine is later than the code cjacksrc's fixes were made
for).

### Online

Internet play uses OpenSpy (openspy.net), the community replacement of
GameSpy's servers. The protocols stay those of the retail GameSpy SDK:
only the host names change. OpenSpy knows Tron 2.0 under the retail
game name `tron20` and secret key `t9D3vB` (`TronGameServerShell.cpp`,
`BaseScreenJoin.cpp`). We checked this one time with a list request
(read only) to `tron20.ms9.openspy.net`: the list decrypts with that key
and it is correct (no servers at that time). A wrong key does not
decrypt, and an unknown game name gets no list. We did not send a
heartbeat to OpenSpy: no public server was hosted.

* **GS-1: the master server.** A server that is not LAN only reports to
  `tron20.master.<domain>` (UDP 27900), and `<domain>` is `openspy.net`
  by default (`MasterDomain`, below). `qr2_init_socket` makes the host
  name with `gsiMakeHostName` (`Libs/GameSpy/GameSpySDK/qr2/qr2.c`).
  `gsiGetDomainName` and `gsiMakeHostName` are in `Libs/GameSpy/
  nonport.c` and `nonport.h` (the two copies of `nonport.h` are the
  same). Each binary finds the domain in this sequence: the environment
  variable `TRON20SRC_MASTERDOMAIN`, `+MasterDomain <x>` on its command
  line, `"MasterDomain"` in `autoexec.cfg`, and then the default. A
  domain that is an IPv4 address is the host of all services (a local
  or private master server). The client copies its console variable
  `MasterDomain` into `TRON20SRC_MASTERDOMAIN` before it shows the Internet
  list and before it hosts a game (`TronSyncMasterDomain`,
  `TRON/BaseScreenJoin.cpp`; called from `ScreenJoinServerList.cpp`
  `FindServers` and `BaseScreenHost.cpp` `StartServer`). Thus the
  server in the same process and a `TRONSrv.exe` that the client starts
  use the same domain. A `TRONSrv.exe` that you start yourself reads its
  command line (`TRONSrv.exe -config <name> +MasterDomain <x>`), the
  environment or `autoexec.cfg`. The server writes `GameSpy: reporting
  to the master server of <domain>` to its console
  (`CTronGameServerShell::TronUnknownVirtual4C`).
* **GS-2: the server list.** The Internet list asks
  `tron20.ms<n>.<domain>` (TCP 28910; for `tron20`, n is 9):
  `ServerListConnect` in `serverbrowsing/sb_serverlist.c` makes the host
  name with `gsiMakeHostName`.
* **GS-3: no version check.** GameSpy's patch service
  (`motd.gamespy.com`) is gone, and Update 2 (1.042) is the last patch.
  The request also sent a machine ID that the SDK keeps in the registry
  (`GOAGetUniqueID`). `CBaseScreenJoin::CheckForUpdate` returns at once,
  so the Internet screens open without the "checking for updates" box
  (`TRON/BaseScreenJoin.cpp`). `pt/ptMain.c` does not change.
* **GS-4: no CD-key check.** The server does not start GameSpy's CD-key
  module (`gcd_init_qr2`, `gcd_think`, `gcd_shutdown`) and lets every
  client in, as in a LAN game (`gcd_authenticate_user`,
  `gcd_disconnect_user` are not called; `TRON/TronGameServerShell.cpp`:
  `TronUnknownVirtual4C`, `50`, `54`, `78`, `ProcessHandshake`). The
  client does not read its CD key: it answers the server's challenge
  with the response of an empty key, so that the message keeps its
  form and does not identify the player
  (`TRON/TronClientMultiplayerMgr.cpp` `HandleMsgHandshake`). Thus the
  modern client does not use `CTronCDKey` (APPS-3 can remove the
  registry read).

Tests (`make mptest TEST_GAME=out/tron20-modern-release`, the new
`MPTEST_MASTER=local`): `build/gamespy.py master` is a local stand-in
for the master server and the server list (qr2 and the encrypted list
of the SDK, written from the SDK source; its ciphers give the same bytes
as the SDK's C code). All programs get `TRON20SRC_MASTERDOMAIN=127.0.0.1`.
The dedicated server (not LAN only) sent its heartbeat. It answered the
challenge correctly and was listed. Client 1 joined with `+join`.
Client 2 opened Multiplayer, Join, saw `mptest` in the Internet list
(ping, players 1/8, Disc Arena, DiscArena01: its query went to the
server), and joined from the list. Neither client has a CD key. Both
played and saw the other. The same with `MPTEST_MODE=listen` (client 1
hosts from the menus). With `MPTEST_MASTERVIA=cvar` there is no
environment variable: the clients get `+MasterDomain 127.0.0.1` (the
console variable, which `TronSyncMasterDomain` copies) and `TRONSrv.exe`
gets it on its command line. Dedicated and listen both passed. A LAN
game (`make mptest` without `MPTEST_MASTER`) is not changed and still
works. Not tested: that the engine keeps `MasterDomain` in
`autoexec.cfg` when the game exits (the tests stop the clients), and a
game over OpenSpy itself (that needs a public server). `build/gamespy.py list
<host>` is a read-only browser (for example, against OpenSpy).

### Console variables

| Variable | Type | Default | What | Items |
|---|---|---|---|---|
| `MasterDomain` | string | (empty = `openspy.net`) | the domain of the master server and the server list: `tron20.master.<domain>`, `tron20.ms<n>.<domain>`; an IPv4 address is the host of both. Read from the client's console, `+MasterDomain` on a command line (`TRONSrv.exe` too), `autoexec.cfg`, or the environment variable `TRON20SRC_MASTERDOMAIN` | GS-1, GS-2 |

## Planned (Phase 1)

Six agents do Phase 1, each with its own files. Each item lists the files
and functions that it will most likely change (from a search of the code).
Engine paths are under `src/Engine/runtime/`, game paths under
`src/Game/`.

### Apps

APPS-2 and APPS-3 are done (above).

### Display

CL-30 to CL-35 are done (above).

### Frame rate

Done: EN-44 and EN-45 (under "Done", "Frame rate").

### Input

Done: EN-24, CL-20 to CL-25 (under "Done", "Input").

### Middleware

What Tron 2.0 loads that is not ours (`LoadLibrary`, the imports and the
DLL names in every retail binary):

| Retail | Loaded by | Modern |
|---|---|---|
| `binkw32.dll` (RAD Game Tools), the movies | `kernel/src/sys/win/binkvideomgrimpl.cpp` (`LoadLibrary("Binkw32.dll")`) | ours, X-1 (done) |
| `MSS32.DLL`, Miles (RAD Game Tools) | only RAD's `binkw32.dll` names it (with `DSOUND.DLL`, for its sound outputs); no binary of the game imports or loads it, and the game does not ship it | gone with X-1: our player plays through `waveOut` |
| `L3CODECA.ACM`, the MP3 codec of the system | `src/Engine/snddrv/s_dx8.cpp` (`SelectHardCodedMP3Codec`, then the ACM drivers of the system) | ours, in `SndDrv.dll`, X-2 (done) |
| `IMAADP32.ACM` (IMA ADPCM, a part of Windows and of Wine) | `s_dx8.cpp`; `Startup` fails without an ADPCM driver, though no file of the game is IMA ADPCM | kept: a part of the system |
| Microsoft's DirectMusic (`dmime`, `dmusic`, `dmsynth`, `dswave`, ...), the music | the engine (`ltdirectmusic_impl.cpp`, `CoCreateInstance`); a part of Windows | Microsoft's, in the game directory; under Wine and Proton our own music player in the engine (X-3, done) |
| `cdaudio.dll` (Monolith's CD music driver) | the engine (`CClientMgr::AppInitMusic`), only if the game asks; Tron 2.0 does not ship it | nothing to do |
| `MFC42.DLL`, `MFC42U.DLL`, `MSVCP60.DLL`, `Msvcrt.dll`, `MSVCIRT.dll`; D3DX9 (linked statically) | | kept: Microsoft's runtime |
| SecuROM's DLL names in the retail `Lithtech.exe` (`CmdLineExt*.dll`, `SIntf*.dll`, `PID*.DLL`, ...) | the copy protection | not in our engine |
| the GameSpy SDK | compiled from our source (`src/Game/Libs/GameSpy`) | GS- items |

* X-3 is done (above, under "Done", "Middleware").

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
| `ClientShellShared/CMoveMgr.cpp`, `VehicleMgr.cpp`, `PlayerMgr.cpp` | Frame rate (EN-45), Input (CL-20, CL-25: the pad's movement, look and light cycle) | Input adds blocks in `UpdateNormalControlFlags`, `UpdateNormalMotion`, `GetMaxVelMag`, `UpdateLightCycleControlFlags`, `CalculateCameraRotation` only |
| `ClientRes/Shared/ClientResShared.h`, `ClientRes/TRON/ClientRes.rc` | all (new strings) | each its own block of IDs: Frame rate 31100 to 31109, Input 31300 to 31399 |
| `build/playtest.sh`, `Makefile`, `README.md` | Middleware (X-1, X-2: `PLAYTEST_AUDIO`, `TRON20SRC_MIDDLEWARE`, `make mp3acmtest`) and others | each adds its own lines |
