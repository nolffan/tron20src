# Console commands

This file lists the console commands of Tron 2.0 that are useful to a
player: the engine's commands, the game's commands, the modern build's
commands, the console variables of the modern build, the cheat codes, and
the command line. The other changes of the modern build are in
[`MODERN.md`](MODERN.md).

Command names and variable names are not case-sensitive. Spaces separate
the arguments. Put quotes around an argument that has spaces. Cheat codes
are different: type them in lower case.

## Opening the console

| Build | How |
|---|---|
| Retail and legacy | You cannot open the console. The engine keeps its text, its command line and its commands, but no key opens it and the game does not draw it. Commands come only from the command line, `autoexec.cfg` and files that `Exec` runs (below). Type cheat codes in the chat line. |
| Modern | The key to the left of `1` (the backquote key on a US keyboard; the key in that position on other layouts), or **Shift+Esc**, opens the console over the top half of the screen. That key, or Esc, closes it. `ConsoleEnable 0` keeps it closed: the keys are then ordinary keys, as in retail ([`MODERN.md`](MODERN.md), EN-70). |

**In the modern console** you can type each line that the engine's
console accepts: an engine command (section 1), a variable (`Name value`;
`Name` alone shows its value), a game command (section 3: they exist when
the game has started, also in the menus), and the modern build's commands
(section 4). The console shows each line that the engine and the game
write.

| Key | What it does |
|---|---|
| Enter | Runs the line. When the line sets a variable, the console shows the new value. When a name is not a command or a variable, the console says so. |
| Up, Down | The lines that you typed before (only those that start with the text on the line). |
| Tab | Completes the command name or the variable name. When more than one name agrees, it completes as far as the names agree and lists them. After `Map `, Tab completes the name of a world. |
| Page Up, Page Down | Scrolls the text (Ctrl: to the start or to the end). Ctrl+Up and Ctrl+Down scroll one line. |
| Left, Right, Home, End, Backspace, Delete | Edit the line (Ctrl: by words). |

In single player, the game pauses while the console is open, and
continues when you close it. In multiplayer the game continues, and your
player stands still: no key goes to the game while the console is open.
The variables `ConsoleFont`, `ConsoleFontSize` and `ConsoleLeft`,
`ConsoleTop`, `ConsoleRight`, `ConsoleBottom` change the font and the
rectangle of the console (section 5).

**Other ways to give commands** (all builds):

- **The command line.** `+Name value` on the command line of
  `Lithtech.exe`, or in the file that `-cmdfile` gives. Each `+Name` takes
  one argument: the next word. `++Name value` also keeps the variable in
  `autoexec.cfg`. Section 7 gives examples.
- **`autoexec.cfg`** (and `display.cfg` in the modern build), in the game
  directory. One command or variable on each line.
- **A file that `Exec` runs**, for example `+Exec mycmds.cfg` on the
  command line.

These run one time, when the engine starts, before the game DLL loads.
Thus only the engine's commands (section 1) and the variables work there.
The game's commands and `Cheat` do not exist yet. When the game stops, the
engine writes `autoexec.cfg` again with only the variables, the key
bindings and `ModelAdd`. Thus it removes commands that you write into it.
Keep your commands in a file of your own and run it with `+Exec`.

**Output.** The commands write to the engine's console. Retail never shows
it; the modern console does. `+ErrorLog 1` also writes the console's text
to `error.log`.

**Cheat codes** (section 6): type them in the chat line. `T` (Send
message) opens the chat line, in the controls that the game has at the
start. In the modern build you can also type `Cheat <code>` in the
console.

## 1. Engine client commands

| Command | Arguments | What it does |
|---|---|---|
| `quit` | | Stops the game at once. |
| `World` | `<name>` | Starts a world without the game's mission data (the engine's command; the path from the game's root, for example `Worlds\RetailSinglePlayer\m01-01`). Use `Map` (section 4) or `+runworld` (section 7) to start a level as the game does. |
| `serv` | `<command ...>` | Runs the rest of the line on the server's console (section 2). The server accepts it only from its local client (single player, or the host). |
| `Exec` | `<file>` | Runs a file of commands and variables. |
| `Set` | | Shows each console variable of the client and its value. |
| `ListCommands` | | Shows the engine's own commands. |
| `Bind` | `<device> <trigger> <action> [...]` | Binds a key or a button to one or more actions. It removes the old bindings of the trigger first. |
| `RangeBind` | `<device> <trigger> <low> <high> <action> [...]` | Binds ranges of an axis to actions. |
| `Scale` | `<device> <trigger> <scale>` | Sets the scale of an axis. |
| `RangeScale` | `<device> <trigger> <scale> <min> <max> <precenter>` | Sets the scale and the range of an axis. |
| `AddAction` | `<name> <number>` | Makes an input action. |
| `EnableDevice` | `<device>` | Starts an input device, for example `"##gamepad"` (the modern build's controller). |
| `ListInputDevices` | | Lists the input devices. |
| `SSFile` | `<name>` | The first part of the screenshots' file names. |
| `UpdateServer` | `0\|1` | `0` stops the local server: the game world stops. |
| `RenderCommand`, `RCom` | `<args ...>` | Gives a command to the renderer. |
| `RestartRender` | | Starts the renderer again. |
| `ResizeScreen` | `<width> <height>` | Changes the resolution. If the new resolution fails, the engine goes back to the old one. |
| `RestartConsole` | | Resets the console. |
| `MoveConsole` | `[Top\|Bottom\|Middle\|Full\|l t r b]` | Shows or sets the rectangle of the console (the `ConsoleLeft` ... variables). |
| `RebindTextures` | | Releases the textures. The engine loads them again. |
| `LogTextureInfo` | `<file.csv>` | Writes the loaded textures to a CSV file. |
| `ModelAdd` | `<r> <g> <b>` | Light that the engine adds to each model, each value 0 to 250. `autoexec.cfg` keeps it. |
| `ShowTicks` | `ALL\|NONE\|SUMMARY\|...` | Timing counters. They are compiled out of the final engine. |
| `ShowVersionInfo` | | Shows the engine's version and build. |
| `ConsoleHistory`, `ClearHistory` | | Shows or clears the list of typed lines. |
| `WriteHistory`, `ReadHistory` | `<file>` | Writes or reads the list of typed lines. |
| `HeapCompact` | | Compacts the memory heap. |
| `Mem` | | Says that the memory commands are not in this build. |
| `BlastServer` | | Does nothing. |

## 2. Engine server commands

These commands run on the server. In single player, or when you are the
host, type `serv <command>` in the console. The dedicated server
(`TRONSrv.exe`) runs each line of its command box on its console.

| Command | Arguments | What it does |
|---|---|---|
| `world` | `<name> [0]` | Starts a world on the server (without the client's mission data). |
| `SpawnObject` | `<class> ["props"]` | Makes an object 200 units from the first player. |
| `objectinfo` | | Counts the objects of each class. |
| `ShowGameVars` | | Shows each console variable of the server. |
| `ShowUsedFiles` | | Lists each file that the server opened. |
| `DisableWMPhysics` | | Gives box physics to each object. |
| `ExhaustMemory` | | Does nothing. |

## 3. Game commands

The game registers these commands when it starts. In retail they exist,
but no console can run them. Commands marked *server* send a message to
the game's server. Most are development commands.

| Command | Arguments | What it does |
|---|---|---|
| `Cheat` | `<code> [arguments]` | Runs a cheat code (section 6), for example `Cheat mpgod`. |
| `ChaseToggle` | | Changes between the first-person view and the third-person view (a living player, not zoomed). |
| `FragSelf` | | Kills your player. *server* |
| `ExitLevel` | | Finishes the level, as `mpmaphole` does. |
| `Teleport` | `<x> <y> <z>` | Does nothing: the final build has no player teleport here. Use `SetPos` (section 4). |
| `BuildInfo` | | Shows the game's build. |
| `LC` | `color <name>` | Gives your light cycle a color of `Attributes\LightCycles.txt` (for example `LC color LightCycleRed_1`). |
| `Cmd` | `<command> <argument> [...]` | Sends a command to the server's command manager, as a trigger of a level does. *server* |
| `Trigger` | `<object> <message>` | Sends a message to the named objects. *server* |
| `List` | `<class>` | Writes the objects of a class (case-sensitive) and their positions to the server's console. *server* |
| `Stimulus` | `<type> [multiplier]` | Makes an AI stimulus at your position. *server* |
| `RenderStimulus` | `[0\|1]` | Shows the AI stimuli as spheres. *server* |
| `Alpha` | `<object> <alpha>` | Sets the alpha of a named object. *server* |
| `AddGoal`, `RemoveGoal` | `<AI> <goal> [name=value ...]` | Gives a goal to a named AI, or removes it. *server* |
| `ClientFX` | `<fx> <object>` or `<fx> <x> <y> <z>` | Asks the server to play a ClientFX. *server* |
| `Music` | `I\|PS\|PM\|V\|SS\|SM\|S\|P ...` | Controls the music: the intensity, a secondary segment, a motif, the volume, stop, play. |
| `InitSound` | | Starts the sound again. |
| `ReloadWeapons`, `ReloadSurfaces`, `ReloadFX` | | Reads the attribute files again. |
| `Frame`, `FrameUV` | `<piece> <edge> <value>` | Moves an edge of a piece of the interface's textured frame (development). |
| `TestSubtitle` | `<string id>` | Shows a subtitle from the string table. |
| `SimulateChat` | | Adds test lines to the chat messages. |
| `EditFX` | `<scalefx>` | Moves and scales a ScaleFX of a menu (development). |

## 4. Modern-build commands

| Command | Arguments | What it does |
|---|---|---|
| `GetPos` | | Shows your position as the line `SetPos x y z`, in the coordinates of the level (as `mppos` shows them). EN-70. |
| `SetPos` | `<x> <y> <z>` | Moves your player to that point, on the floor below it, with the same direction. Only in single player, for a living player on foot. EN-70. |
| `Map` | `[name]` | Starts a world. Without a name, it lists the worlds. Only in single player. CL-50, below. |
| `UnlockCycles` | | Unlocks each light-cycle color, map and power-up in the current profile, and saves the profile. CL-51, below. |

**`Map`.** `Map` alone lists the worlds that the game has: each `.dat`
file under `Worlds\` in the game's archives, in their folders
(`Worlds\RetailSinglePlayer`, `Worlds\RetailLightCycle`,
`Worlds\RetailLightCycle\Zone`, `Worlds\RetailMultiplayer\DiscArena`).
`Map <name>` starts one. The name can be:

- the world's file name: `map m05-01`, `map LC_02s03`, `map intro`, `map
  fin`;
- the path, with or without `Worlds\`: `map RetailLightCycle\Zone\Map0`;
- with or without `.dat`, in upper or lower case.

Tab after `Map ` completes the file name. When a name agrees with more than
one world, `Map` lists them: give the folder too. In multiplayer `Map`
only lists the worlds and does not start one (as `SetPos`, it is a
single-player command).

`Map` starts the world as the game's menus and `+runworld` do: a new
single-player game, through the mission manager. The console closes, and
the loading screen comes. The type of world sets what you get:

- **A level of the campaign** (`Worlds\RetailSinglePlayer`): the level
  with the data of its mission in `Attributes\Missions.txt`. The loading
  screen shows the level's name. The first level of a mission shows the
  mission's opening cut-scene and the subroutine screen, as in the
  campaign. The objectives, the HUD and saving work as in the campaign.
- **A light-cycle arena of a circuit** (each world in
  `Worlds\RetailLightCycle`): the circuit's race, as Light cycles >
  Circuit Play starts it: its lives, its waves, its opponents and your
  profile's light cycle. When you win, you get the circuit's unlocks.
- **Another world** (the disc arenas of multiplayer): a custom level,
  without mission data. You walk in the arena alone.

Each `Map` is a new game. Your player does not keep what earlier levels
gave: Jet starts at version 0.0.0, with full health and energy, no build
points, no subroutines in the memory, and only the weapons that the
mission and the level give at their start (`DefaultWeapons` in
`Missions.txt`). To get what a player has at that level in the campaign,
type `Cheat mpideal` (modern build: the level's ideal weapons,
subroutines, procedurals and build points; section 6).

**`UnlockCycles`.** The current profile gets each light-cycle color (the
regular and the super cycle of each shade in
`Attributes\LightCycles.txt`, also the shades of multiplayer only), each
map of Custom Play that a circuit locks, and each power-up. Then the
profile is saved. The console shows the number of each that it added. The
light cycles screen shows them unlocked at once (the Light Cycles page:
no shade shows "LOCKED"; Custom Play: the maps). The circuits of Circuit
Play are never locked: retail's `IsCircuitLocked` (0x10067660) always
returns false, and no circuit has `InitiallyLock` TRUE. `UnlockCycles`
does not mark circuits as completed. The change stays in the profile; the
`mpunlockcycles` cheat (section 6) unlocks the colors only until the game
stops.

## 5. Console variables of the modern build

Set them in the console, on the command line (`+Name value`) or in
`autoexec.cfg`. [`MODERN.md`](MODERN.md), "Console variables", gives
more detail.

| Variable | Default | What it does |
|---|---|---|
| `MaxFPS` | 240 | The maximum frames each second; 0 is no limit, 1000 at most. Also in multiplayer. Options > Display > "Frame Rate Limit" (60, 120, 144, 240, None). |
| `FovX`, `FovY` | 90, 78 | The field of view in play, in degrees, for a 4:3 screen. The engine makes it wider on a wider screen. Options > Display > "Field of view" sets `FovX` (60 to 120). |
| `Windowed` | (engine) | 0 full screen, 1 a window. Options > Display > "Window mode". |
| `WindowMode` | 0 | With `Windowed 1`: 1 a window with a frame, 2 a borderless window over the desktop at the desktop's resolution. |
| `AntiAliasFSOverSample` | 0 | The antialiasing samples: 0, 2, 4, 8. Options > Display > "Antialiasing". |
| `Anisotropic` | 0 | The anisotropic filtering: 0 (off), 2, 4, 8, 16. Options > Performance. |
| `Cinematics43` | 0 | 1: on a screen wider than 4:3, black sides during cut-scenes. |
| `ScreenGlowAspect` | 1 | The shape of the screen glow on a wide screen: 1 as on a 4:3 screen; 0 as retail (stretched); 2 as 1 with a square texture. |
| `MusicPlayer` | 0 | 0 DirectMusic, and the modern music player when DirectMusic does not start (Wine, Proton); 1 always the modern player; 2 only DirectMusic. |
| `MasterDomain` | `openspy.net` | The domain of the master server and the Internet server list. |
| `RawMouse` | 1 | 0: the mouse from DirectInput, not from raw input. |
| `JoystickDisable` | 0 | 1 disables the controller. |
| `PadLookSpeed` | 1 | The right stick's turn rate (Options > Controls > Controller: "Look speed"). |
| `PadLookCurve` | 2 | The right stick's response curve (1 is linear). |
| `PadInvertY` | 0 | 1: the right stick's up looks down. |
| `PadLookDeadZone` | 0.15 | The right stick's dead zone (0 to 0.9). |
| `PadMoveDeadZone` | 0.2 | The left stick's dead zone (0 to 0.9). |
| `FrameTimeLog` | 0 | Writes each frame of this many milliseconds or more, with its parts, and a summary each ten seconds, to `frametime.log`. 0 is off. |
| `PosLog` | 0 | 1: writes your position each frame to `poslog.txt` (and the light cycles' positions to `lcpos.txt`). For tests. |
| `ConsoleEnable` | 1 | 0 keeps the console closed. |
| `ConsoleFont` | (empty) | A font file of the game for the console. Empty: `Interface\Fonts\TRON_CONSOLE11.ffb`. |
| `ConsoleFontSize` | 0 | The console's letter height in pixels (10 to 96). 0: a 45th of the screen's height, a whole multiple of 12. |
| `ConsoleLeft`, `ConsoleTop`, `ConsoleRight`, `ConsoleBottom` | -1, -1, -1, -2 | The console's rectangle. A negative value -n puts the edge at a nth of the screen. The default is the top half. |

Some variables of all builds that you can give on the command line:
`DisableSound 1`, `DisableMusic 1`, `DisableMovies 1` (no logo movies at
the start), `DisableJoystick 1`, `ScreenWidth`, `ScreenHeight`,
`ErrorLog 1`.

## 6. Cheat codes

Type a cheat code in the chat line (`T`, all builds), or `Cheat <code>` in
the modern console. Each code starts with `mp`. The game keeps the codes
encoded in `CheatMgr.cpp` (`s_CheatInfo`, CShell.dll 0x1022b870: each
character is `((c ^ 38) + i) ^ 7`, with `i` its position). The table has
57 codes. A known code plays the cheat sound, and the chat line does not
send it as a message, also when the code does nothing.

The cheats work only in single player. In multiplayer, only `mpasscam`,
`mpexorbitantamounts`, `mpmaphole`, `mpnextmission` and `mpboot` do
something; the server accepts `mpmaphole`, `mpnextmission` and `mpboot`
only from the host.

Columns: **Retail** is the retail game and the legacy build. **Modern** is
the modern build.

| Code | What it does | Retail | Modern |
|---|---|---|---|
| `mpgod` | God mode on and off: you take no damage. | yes | yes |
| `mpammo` | Full ammunition. | yes | yes |
| `mparmor` | Full armor. | yes | yes |
| `mphealth` | Full health. | yes | yes |
| `mpkfa` | All weapons (as `mpguns`), full health, full armor and full energy. | yes | yes |
| `mpguns` | All weapons: each weapon of the player (its primitive), all weapon modifications, and the level's ideal weapons. The chat says ON and OFF in turn, but each time the server gives the weapons. | yes | yes |
| `mptears` | Infinite ammunition on and off; when it goes on, also all weapons. | yes | yes |
| `mpmods` | All modifications of the weapons. | yes | yes |
| `mpgear` | Each gear item. | yes | yes |
| `mppos` | Shows your position on the screen, on and off. | yes | yes |
| `mpasscam` | First-person view and third-person view, in turn. Also in multiplayer. | yes | yes |
| `mprosebud` | Development: asks the server to make a snowmobile (`PlayerVehicle`). Tron has none. | yes | yes |
| `mpmaphole` | Finishes the level (multiplayer: the host). | yes | yes |
| `mpnextmission` | Goes to the next mission (multiplayer: the host). | yes | yes |
| `mpboot <name>` | Multiplayer only: the host disconnects the named player. | yes | yes |
| `mpbuild` | Shows the game's build in the chat. | yes | yes |
| `mpskillz` | The rewards of the current mission (`Missions.txt`, `Reward<n>`): their points go to the server's skill points. | yes | yes |
| `mpexorbitantamounts` | "Big blood" on and off (`BigBlood`: larger, longer hit effects). Also in multiplayer. | yes | yes |
| `mpgimmegun <weapon>` | One weapon, by name (`Attributes\Weapons.txt`, for example `Disc`, `Ball`, `PRod`, `Blaster`) or by number. | yes | yes |
| `mpgimmeprim <primitive>` | The first player weapon of a primitive (`Disc`, `Rod`, `Ball`, `Mesh`, `Hands`), as `mpgimmegun`. | yes | yes |
| `mpgimmeammo <ammo>` | One type of ammunition, by name or number. | yes | yes |
| `mpgimmemod <mod>` | One weapon modification, by name or number. | yes | yes |
| `mpgimmegear <gear>` | One gear item, by name or number. | yes | yes |
| `mpenergy` | Full energy. | only through `mpkfa` | yes |
| `mpprocedurals` | The procedurals Port, Defrag and Disinfect. | no | yes |
| `mpsubalpha` | The procedurals and each subroutine at alpha. | no | yes |
| `mpsubbeta` | The procedurals and each subroutine at beta. | no | yes |
| `mpsubgold` | The procedurals and each subroutine at gold. | no | yes |
| `mpclearmemory` | Removes each subroutine from the memory. | no | yes |
| `mpcorruption` | Infects 200 random sectors of the memory. | no | yes |
| `mpmaxperformance` | Sets the five performance ratings to 100. | no | yes |
| `mpclearperformance` | Sets the five performance ratings to 0. | no | yes |
| `mpidealweapons` | The weapons that the level's ideal player has (`Missions.txt`, `IdealWeapon`). | no | yes |
| `mpidealsubroutines` | The level's ideal subroutines. | no | yes |
| `mpidealprocedurals` | The level's ideal procedurals. | no | yes |
| `mpidealpoints` | The level's ideal build points. | no | yes |
| `mpideal` | The four `mpideal...` cheats together: use it after `Map`. | no | yes |
| `mpunlockgrids` | Each map of Custom Play is unlocked, on and off, until the game stops (cheat 0x36). | no | yes |
| `mpunlockcycles` | Each light-cycle color is unlocked, on and off, until the game stops (cheat 0x37). | no | yes |
| `mpunlockpowerups` | Each power-up of Custom Play is unlocked, on and off, until the game stops (cheat 0x38). | no | yes |
| `mppset`, `mpclearpset` | Development: nothing on the client (the server has a handler that the client does not call). | no | no |
| `mpclip`, `mppoltergeist`, `mpteleport`, `mpwpos`, `mpwmpos`, `mpcamera`, `mplightscale`, `mplightadd`, `mpfov`, `mppoly`, `mpboyisuck`, `mpcam`, `mpcampos`, `mpobtuse`, `mpbreachoffset` | Development cheats. The final build knows the codes but does nothing with them. | no | no |

**The cheats that do nothing in retail.** Retail's `CCheatMgr::Process`
(0x100f1f40) stops at `mpskillz` (cheat 0x24). The codes after it are
known, but do nothing, although their handlers are in the game. The
light-cycle screens test the flags of the three unlock cheats (0x36,
0x37, 0x38), but no code sets them. The modern build connects them
(`RETAIL_BUGS.md`, `MODERN.md` GAME-150). The legacy build keeps retail's
behavior. The name of cheat 0x37 is `mpunlockcycles`: the table's entry
`LPRLIILSJoHRHe` decodes to it.

`Attributes\ClientButes.txt`, `[Cheats]`, can list cheats that the game
runs each time a level starts: `Cheat0 = "mpgod"`, `Cheat1 = ...`.

## 7. The command line

`linux-run.sh` (and `Tron.exe`) give their arguments to the engine. In the
modern build `Tron.exe` starts the game at once. Examples:

```sh
./linux-run.sh '+runworld' 'Worlds\RetailSinglePlayer\m05-01'   # start a level of the campaign
./linux-run.sh +runworld 'Worlds\RetailLightCycle\LC_02s03'     # start a world (no circuit data: use Map)
./linux-run.sh +skiptitle 1                                     # no title screen: the main menu at once
./linux-run.sh +DisableMovies 1 +DisableMusic 1
./linux-run.sh ++windowed 1 ++WindowMode 2                      # (modern) a borderless window
./linux-run.sh +MaxFPS 144 +FovX 100                            # (modern)
./linux-run.sh +FrameTimeLog 20                                 # (modern) the slow frames, to frametime.log
./linux-run.sh +Exec mycmds.cfg                                 # run your own file of commands
./linux-run.sh +join 192.168.1.10:27888 +gametype DiscArena     # join a multiplayer game
```

`+runworld <world>` starts a single-player game on the world through the
mission manager, as `Map` does. Give the whole path, as
`Attributes\Missions.txt` writes it (`Worlds\RetailSinglePlayer\m05-01`):
only then does the game find the level's mission. A world that is not in
`Missions.txt` starts as a custom level, without mission data. In the legacy build,
`-gogame` starts the game without the launcher's window. `+host <world>`
hosts a multiplayer game on the world.
