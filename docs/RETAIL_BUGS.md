# Bugs in the retail game

Bugs in *Tron 2.0* (v1.030) as it shipped, found while rebuilding it. Each
is a mistake in the code that a source change would fix. This is a living
list: add a bug when it is found, in the format below, whether or not it is
fixed yet.

The legacy build reproduces every one of them, on purpose: compiling to the
retail machine code means compiling its mistakes too. They are fixed in the
**modern build** (`make modern`, the `TRON20SRC_MODERN` define; `MODERN.md`):
each fix goes behind `#ifdef TRON20SRC_MODERN`, with the retail code kept in
the `#else`, so the legacy build is unchanged and still matches
(`make progress`).

An entry that is fixed says so under its heading, with the file the fix is
in and its `MODERN.md` id. One that is not says why, or which part of the
tree it waits for.

Every binary here is a reconstruction (`docs/PLAN.md`), so an entry is
listed only where our function is byte-identical to the retail one at the
address it names (`ext/tools/exematch.py`; for a string, the string is in the
retail image too): it describes what the retail code does, not a guess.
Addresses are as in the retail images (`build/reference.sh` extracts them
into `out/reference/`).

Many of NOLF2's bugs (nolf2src's `docs/RETAIL_BUGS.md`) and Contract J.A.C.K.'s
(cjacksrc's) are likely to be here too, since Tron 2.0 was built from the
same code: check each against the retail binary before listing it. Those
Tron 2.0 does not have go at the end, so nobody looks again.

**Fixed so far** (in the modern build): the entries that say so below.
`MODERN.md` lists each fix with its id.

## Format

```
### <What goes wrong, as a player or a programmer would put it>

<binary> <address> (<function>, <source file>). What the code does, and
what it should do; how it shows in the game, if it does.

Fixed in the modern build: <file>, MODERN.md <id>. (Or: not fixed, because ...)
```

## The bugs

### The command manager counts its variables in a byte

Object.lto, CCommandMgr (ObjectShared/CommandMgr.h, .cpp; its constructor at
0x100b45e0). The manager keeps room for 512 variables (CMDMGR_MAX_VARS), but
its count, m_nNumVars, is a uint8: every read and write of it is a byte
(+0x167f0). The `>= CMDMGR_MAX_VARS` checks never fire; a level declaring
more than 255 variables wraps the count to 0 and overwrites the first ones.
NOLF2's count is a uint16.

Fixed in the modern build: ObjectShared/CommandMgr.h (m_nNumVars is a uint16), MODERN.md GAME-50.

### An I/O node's MoveToFloor property does nothing

Object.lto, IONode (TRON/IONode.cpp; ReadProp at 0x10022f60, its update at
0x10022c1a). The class lists a MoveToFloor property (default false), and
its first update moves it to the floor when its m_bMoveToFloor is set, but
ReadProp never reads the property: only the constructor (false) and Load
set the member, so no I/O node is ever moved to the floor.

Fixed in the modern build: TRON/IONode.cpp (ReadProp reads MoveToFloor), MODERN.md GAME-51.

### An I/O node leaks its looping sound's name

Object.lto, IONode (TRON/IONode.cpp; destructor at 0x10022a60). The name of
the looping sound a model key starts (m_hstrLoopSound, created in
HandleModelString) is freed when the sound is killed by a key, but not in
the destructor: a node removed while its loop plays leaks the string.

Fixed in the modern build: TRON/IONode.cpp (the destructor frees the name), MODERN.md GAME-52.

### A lure's death FX is only played when it has no name (and crashes then)

CShell.dll 0x101288d0 (CPlayerMgr::HandleMsgPlayerStateChange,
ClientShellShared/PlayerMgr.cpp), as NOLF2's. When the player dies on a
lure (PPM_LURE), the test is `if( !pszDeathFX && pszDeathFX[0] != '\0' )`:
the FX is never created for a named death FX, and a missing one is read
through a NULL pointer. It should be `pszDeathFX && pszDeathFX[0]`. Tron's
levels may not use lures, so it may never show.

Fixed in the modern build: ClientShellShared/PlayerMgr.cpp, MODERN.md GAME-100.

### A posted string that takes more than one send is sent again from its start

CShell.dll 0x100202c0 (ghiPostStringStateDoPosting, the GameSpy SDK's
ghttp/ghttpPost.c as Tron links it, 2003). A string field of a multipart
HTTP post is sent with `ghiDoSend(connection, string, len - pos)`: the
length left, but from the string's start rather than `string + pos`, so a
partial send is followed by the beginning of the string again and the post
is corrupt. Later SDKs send through `ghiTrySendThenBuffer`. Only a post
that fills the socket's buffer shows it.

Fixed in the modern build: Libs/GameSpy/GameSpySDK/ghttp/ghttpPost.c, MODERN.md GAME-101. (No code in Tron 2.0 makes a post: the only ghttp request is the patch check's ghttpGetFile, which GS-3 removes.)

### Thorne reports a valid summon command as invalid

Object.lto 0x10058497 (CThorne::ReadProp, TRON/Thorne.cpp). The
`SummonCmd` property is checked with `if (g_pCmdMgr->IsValidCmd(cmd))
ObjectCPrint(..., "Invalid summon command %s", ...)`: the test is
inverted, so a valid command prints the error (in the editor/console only)
and an invalid one passes silently. It should be `!IsValidCmd(...)`.

Fixed in the modern build: TRON/Thorne.cpp (the message is for a command that is there and not valid), MODERN.md GAME-54.

### Thorne builds a special-fx message he never sends

Object.lto 0x10058dc1 (CThorne::InitialUpdate). After setting his SFX
message, Thorne writes a second MID_SFX_MESSAGE (his id, object and create
struct) and drops it without sending it. Harmless, wasted work.

Fixed in the modern build: TRON/Thorne.cpp (the message is not made), MODERN.md GAME-55.

### A turret without a PropType reads an uninitialised prop type

Object.lto 0x1006a330 (TronTurret::ReadProp, TRON/TronTurret.cpp). The
PROPTYPE pointer is only set when the PropType property is found and not
empty, but it is tested (and used to set up the model) at the end either
way: a TronTurret without a PropType uses whatever was on the stack. It
should start as NULL.

Fixed in the modern build: TRON/TronTurret.cpp (the pointer starts as NULL), MODERN.md GAME-56.

### A turret without a ViewFOV takes its default cosine for an angle

Object.lto 0x1006a330 (TronTurret::ReadProp). The view FOV member starts as
1.57 (a half angle in radians) and, with no ViewFOV property, is turned
into cos(1.57 * PI/360) as if it were degrees: an almost zero-wide view
cone. The editor always writes the property (default 180), so it may never
show.

Fixed in the modern build: TRON/TronTurret.cpp (without the property, the cosine of the property's default, 180 degrees), MODERN.md GAME-57.

### A parented turret reads its parent's transform and drops it

Object.lto 0x1006bf20 (TronTurret's first update). With a parent, the
parent's position and rotation are read into locals and never used. The
per-frame update reads the parent again, so it is only wasted work.

Fixed in the modern build: TRON/TronTurret.cpp (the parent is not read there), MODERN.md GAME-58.

### The energy transfer meter draws part of its fill twice

CShell.dll 0x100294c0 (CHUDEnergyTransfer::Update, TRON/HUDEnergyTransfer.cpp).
Past half way the fill is three triangles fanned from the meter's center:
-90..-45, -45..0, and then -45..the transfer's angle instead of 0..the
angle. The third covers the second again, so with the meter's alpha blend
the -45..0 wedge draws brighter than the rest once the fill passes 0.

Fixed in the modern build: TRON/HUDEnergyTransfer.cpp, MODERN.md GAME-102.

### A core dump never tells the player how much energy it gave

Object.lto 0x10019200 (CoreDump::HandleActivate, TRON/CoreDump.cpp). It
adds its energy to the player's (clamped to his maximum), then measures
what he gained as the new energy less the sum it clamped, which already
holds the core dump's energy: never more than 0, so the "energy gained"
message (164) it sends when that is over 1 is never sent. It should
subtract the player's energy before the core dump's was added.

Fixed in the modern build: TRON/CoreDump.cpp (the gain is the new energy less the energy before), MODERN.md GAME-59. The server now sends the message, thus the client must not fall through into the HUD text case ("The energy message of a core dump falls through into the HUD text message", below; MODERN.md GAME-136).

### Taking a custom item from a core dump sends an uninitialized message

Object.lto 0x10019da0 (CoreDump::HandleChoice, TRON/CoreDump.cpp). The
trigger message sent to the player is only built for subroutines,
procedurals, primitives, permission sets, emails and help files; for any
other item type (Custom) the uninitialized stack buffer is sent as it is.
Core dumps only get their items from bodies, which may never hold custom
items.

Fixed in the modern build: TRON/CoreDump.cpp (no message for another item type), MODERN.md GAME-60. TRON/ArchiveBin.cpp has the same code (its function is near, thus it has no entry of its own). There a Custom item is real: it runs its command, and then the bin sends the uninitialized buffer to the player. The same fix is there.

### A patch routine gives the amount asked, not what it has left

Object.lto, PatchRoutine::ObjectMessageFn (TRON/PatchRoutine.cpp; retail
0x10032d29, 0x10032d95). When the player's client drains a health or
energy patch routine, the amount asked is clamped to what the routine has
left only to test the player's maximum; what is given (and taken from the
routine) is the unclamped amount. A request larger than what is left gives
the player more than the routine had and leaves its count negative (it is
then neither > 0 nor == 0: the routine is never removed).

Fixed in the modern build: TRON/PatchRoutine.cpp (it gives the clamped amount), MODERN.md GAME-89.

### A spawned optimizer reads none of its properties

Object.lto 0x10030760 (Optimizer::EngineMessageFn, TRON/Optimizer.cpp; our
code matches retail's, but for the switch's jump table, which the scorer
reads as instructions). MID_PRECREATE calls ReadProp only for
PRECREATE_WORLDFILE, not for PRECREATE_STRINGPROP as IONode and most
classes do: an optimizer made by a spawner gets no model, permission sets,
keyframer or commands. Tron's levels may only place them in the world, so
it may never show.

Fixed in the modern build: TRON/Optimizer.cpp (ReadProp for PRECREATE_STRINGPROP too), MODERN.md GAME-61.

### A light cycle dialog's bad priority is never reported

Object.lto 0x10025b30 (CLightCycleButeMgr::Init, Shared/TRON/
LightCycleButeMgr.cpp; CShell.dll has its own copy). Reading a CycleDialog
<Type>Priority, the check is `if (fPriority < 0.0f && fPriority > 1.0f)`,
which is never true: a priority out of [0, 1] goes unreported ("ERROR:
Priority value %f < 0 or %f > 1 for %s") and is kept. It should be `||`.

Fixed in the modern build: Shared/TRON/LightCycleButeMgr.cpp (both DLLs), MODERN.md GAME-103.

### A light cycle shade's Super color is not cleared

Object.lto 0x10026858 and 0x1002802a (LIGHTCYCLESHADE's constructor, inlined
in CLightCycleButeMgr::Init and GetShade, Shared/TRON/LightCycleButeMgr.h).
It clears the first two characters of the Regular color name instead of the
first of each color, so the Super color of a new shade is garbage until Init
reads it. Init always reads both, and GetShade's default shade is static
(zeroed), so it should never show.

Fixed in the modern build: Shared/TRON/LightCycleButeMgr.h (the first character of each color), MODERN.md GAME-62. It has no effect on the game.

### A shield hit drops a trail's last point by erasing end()

CShell.dll 0x10100ce0 (CLightCycleMgr::HandleShieldCollision,
Shared/TRON/LightCycleMgr.cpp; the same code is in Object.lto's copy). When
the cut-off part of the hit trail cannot be shortened, the code calls
`collTrailPoints.erase(collTrailPoints.end())`: erasing end() is undefined.
With STLport 4.0 the copy loop runs no times and the end moves back one
point, so it works as a pop_back.

Fixed in the modern build: Shared/TRON/LightCycleMgr.cpp (both DLLs), MODERN.md GAME-104.

### Updating a listed server by IP adds it to the list again

CShell.dll 0x10052f50 (ServerBrowserAuxUpdateIPA, GameSpy SDK
serverbrowsing/sb_serverbrowsing.c, 2003). Updating a server directly
(not via the master), the browser looks the server up by IP and port and,
when it is already in the list, re-queries that entry, but then appends the
server to the list in both cases: an already-listed server appears twice and
the browser callback reports it added again. GameSpy fixed it in
serverbrowsing 2.02.11 (2004-05-24).

Fixed in the modern build: Libs/GameSpy/GameSpySDK/serverbrowsing/sb_serverbrowsing.c, MODERN.md GAME-105.

### The query engine marks timed-out servers in the wrong field

CShell.dll 0x10052220 (TimeoutOldQueries, GameSpy SDK
serverbrowsing/sb_queryengine.c). A server whose query timed out gets
STATE_QUERYFAILED set and its pending-query states cleared in `flags` (the
server's address flags) instead of `state`: the server keeps its pending
state and its flags gain bits that mean "has ICMP address"/"private port".
The 2007 SDK still has it.

Fixed in the modern build: Libs/GameSpy/GameSpySDK/serverbrowsing/sb_queryengine.c, MODERN.md GAME-106.

### A LAN browse never reports that its list is complete

CShell.dll 0x10053e30 (ProcessLanData, GameSpy SDK
serverbrowsing/sb_serverlist.c, 2003). When the LAN search time ends, the
list closes its socket and goes disconnected without the
slc_initiallistcomplete callback, so an asynchronous LAN browse is never
told it finished. GameSpy fixed it in serverbrowsing 2.00.36 (2003-11-10).

Fixed in the modern build: Libs/GameSpy/GameSpySDK/serverbrowsing/sb_serverlist.c, MODERN.md GAME-107.

### The light cycle dialog plays a line it has already popped

Object.lto 0x100509af (CServerLightCycleMgr::Update,
ObjectDLL/TRON/ServerLightCycleMgr.cpp). It keeps a pointer to the top of
the rider dialog priority_queue, pops it, then reads and plays through the
pointer. pop() moves the queue's last line into the top slot, so the line
checked and played is not the one that had the highest priority (unless the
queue held one line).

Fixed in the modern build: TRON/ServerLightCycleMgr.cpp (the line is copied before pop()), MODERN.md GAME-63. The retail loop also does not set its skip flag back to false for each line, and it plays the last line it skipped when the queue becomes empty. The modern build tests each line by itself and plays no skipped line.

### The energy claw's line-of-sight test always passes

Object.lto 0x1003eb90 (CProjectileEnergyClaw::CanSee, ObjectDLL/TRON/
ProjectileEnergyClaw.cpp). It gets the position of the first object for both
ends of the segment it tests, so the segment is empty, nothing is hit, and
the claw "sees" any AI in range: it seeks and chases through walls.

Fixed in the modern build: TRON/ProjectileEnergyClaw.cpp (the second end is the position of hTo), MODERN.md GAME-64. The claw does not seek or chase an AI through a wall.

### A CD key server reply with no second backslash crashes the server

Object.lto 0x10020690 (cdkey_process_buf, Libs/GameSpy/GameSpySDK/gcdkey/
gcdkeys.c, GameSpy's 2003 CD key SDK). A packet from the key server (or
anyone spoofing it on the query socket) that starts with `\` is searched
for the next `\` with strchr, and `pos - buf` is compared with 32 without
checking pos for NULL: with no second backslash the difference is negative,
passes the test, and strncpy is handed a huge length. GameSpy's later SDKs
test `pos &&` first.

Fixed in the modern build: Libs/GameSpy/GameSpySDK/gcdkey/gcdkeys.c (pos is tested for NULL), MODERN.md GAME-65. The modern server does not start the CD key module (GS-4), thus this code does not run there.

### The query SDK's host lookup can hand back an uninitialized hostent

Object.lto 0x10042450 (get_sockaddrin, Libs/GameSpy/GameSpySDK/qr2/qr2.c,
GameSpy's 2003 Query & Reporting 2 SDK). `hent` is only set when the host
needs a DNS lookup, but is stored through `savehent` whenever that is not
NULL: for a numeric address the caller would get whatever the stack slot
held (the compiler reuses the `saddr` argument's slot, so it is `saddr`).
The SDK's only caller (qr2_init_socket) passes NULL, so it never shows.

Fixed in the modern build: Libs/GameSpy/GameSpySDK/qr2/qr2.c (hent starts as NULL), MODERN.md GAME-66. It has no effect on the game.

### Client prediction applies gravity to visible objects, not falling ones

Lithtech.exe 0x467b40 (predict_EvaluateCurve, the engine's
client/src/predict.cpp). When it extrapolates a moving object from the
server's last update, it tells CalcMotion whether to apply gravity with
`pObj->m_Flags & FLAG_GRAVITY != 0`; `!=` binds first, so the test is
`m_Flags & 1`, FLAG_VISIBLE (0x467cd4: `mov cl,[esi+0x84]; and cl,1`).
A hidden object with FLAG_GRAVITY is extrapolated without gravity. A
visible one without FLAG_GRAVITY gets gravity applied too, but pd_Update
hands it a zero gravity vector, so only the first case shows. The
developer build of 2003-08-05 has the same code; Build 69's source has
the intended `(pObj->m_Flags & FLAG_GRAVITY) != 0`.

Fixed in the modern build: src/Engine/runtime/client/src/predict.cpp
(TRON20SRC_MODERN) (MODERN.md EN-16).

### A texture script's Sin is the cosine, and its Cos the sine

Lithtech.exe 0x554fc0 and 0x554fd0 (TSFuncSin and TSFuncCos, the engine's
texturescriptinterpreter.cpp; not reconstructed yet, so read from the
retail machine code). The interpreter calls a script's functions through
the table g_FuncHandlers at 0x5c4000, in the order of TSFUNC_SIN and
TSFUNC_COS: its first entry, 0x554fc0, is `fld [esp+4]; fcos`, and its
second, 0x554fd0, is `fsin` (then Tan at 0x554fe0, and SqrSin at
0x554ff0, which does use fsin). So a script's `Sin(x)` runs as `cos(x)`
and its `Cos(x)` as `sin(x)`. Build 69's source has the same lines
(TSFuncSin returning `cos(fVal)`), as do NOLF2's and Contract J.A.C.K.'s
engines. The shipped .tfs scripts were written and tuned on it: a rotation
built from the two is still a rotation, a quarter turn off.

Not fixed: the shipped .tfs scripts are compiled for this engine and were
tuned on the swap. A correct Sin and Cos in the engine turns every
scripted effect a quarter turn, so the modern build keeps the swap.
(TSFuncSin and TSFuncCos are compiled from our source now, with the
retail bytes, `fld [esp+4]; fcos` and `fsin`.)

### Three render styles use a colour operation that does not exist

RS/FULLBRIGHTON_ALPHA_SPECULAR.ltb (Game2.rez), PROPS/RS/ARCHIVECUBE.ltb
and CHARS/RS/INNER_GLOW.ltb (Game.rez): the second texture stage of their
only pass has colour op 19 (read from the files: the first stage has 3,
MODULATE; stages 3 and 4 are 0), but Tron 2.0 has 17 colour ops (0 to 16,
MODULATETEXALPHA last: ltrenderstyle.h, RENDERSTYLE_COLOROP_TYPE_COUNT).
The device caps check (Lithtech.exe 0x51cc18) lets an op
above 16 through. The renderer then turns the op into Direct3D's through
CRenderStyleLookupTables::m_nColorOp[17], with m_nAlphaOp right after it
(the constructor, 0x55d190, and CD3DRenderStateMgr::SetRenderStyleStates,
0x553a30, are byte-identical to ours), so op 19 reads m_nAlphaOp[2],
D3DTOP_SELECTARG2: the stage gives its second colour argument. That is
the texture in INNER_GLOW and ARCHIVECUBE (ColorArg2 2, TEXTURE), and the
result of the first stage in FULLBRIGHTON_ALPHA_SPECULAR (ColorArg2 0,
CURRENT), as read from the developer build's copies of the files. The table
is a constant static object, so the read stays in it and gives the same
value each time. (An earlier version of this entry inferred 16 ops and
D3DTOP_MODULATE from NOLF2's tables; the rebuilt renderer corrects it.)

Not fixed: harmless, because the read stays in the constant table and
the three styles always render the same (SELECTARG2). Which op the
artists meant is not known, so a different op only changes how they
look.

### A lip-sync file's long node name overruns a stack buffer

CShell.dll 0x10185b30 (CNodeController::HandleNodeControlLipSync,
ClientShellShared/NodeController.cpp; our function is near, not identical
yet, so read from the retail code). For each node the .lip file names, it
reads a 4-byte length (0x10185d19) and then that many bytes into the
256-byte `szBone` on the stack (0x10185d2f), and writes the terminator at
`szBone[length]` (0x10185d40), with no check: a name of 256 bytes or more
overwrites the stack. No shipped name is longer than 17 bytes, so only a
broken or hostile file shows it.

Fixed in the modern build: ClientShellShared/NodeController.cpp, MODERN.md GAME-108. The modern build also reads no more nodes than kMaxNodeControls: retail writes one array entry for each node of the file on the stack (aiNodeControls), with no limit.

### An empty sprite file divides by zero

Lithtech.exe 0x484e7e..0x484ef9 (spr_Create, the engine's sprite.cpp; not
reconstructed yet, so read from the retail code). It reads the frame
count, frame rate and three more dwords, then sets the animation's length
to `(1000 / rate) * frames` (`div` at 0x484ef9) before it looks at the
stream's error status. The rez stream zeroes what it cannot read (Build
69's RezFileStream::Read), so an empty file gives a rate of 0 and a divide
by zero. FX/SPR/FAKE.SPR, FX/SPR/JETREZIN.SPR and
INTERFACE/MENU/SPR/LOADING.SPR in Game.rez are 0 bytes; nothing loads
them, so it never shows. Also, spr_Create leaves the sprite's link zeroed
(0x484e1d), and each of its error returns calls spr_Destroy, whose
dl_Remove writes through that NULL link (0x484fd7): a short sprite file,
or a frame name longer than 1000 characters, is a crash too, not the
LT_INVALIDSPRITEFILE that LoadSprite (0x47d510) expects. LoadSprite then
leaves the file open.

Fixed in the modern build: sprite.cpp, setupobject.cpp, MODERN.md EN-71.

### The pings HUD sets up only four of its eight pings

CShell.dll 0x100311e0 (CHUDPings::Init, ClientShellDLL/TRON/HUDPings.cpp).
The item holds eight pings, but Init sets the UVs, the color and the name
of the first four only, and Update (0x10031490) fills four at most; the
other four are never shown. UpdateLayout (0x10031960) also reads the font
size twice.

Not fixed: harmless, because four pings is what the layout holds. The layout (Game.rez) puts the pings down from (6, 80) with icons 52 pixels high and a spacing of 0: eight icons go past the bottom of the 480-line page. The I/O node manager keeps all the pings, and a ping that is not shown is shown when one of the first four goes.

### A light cycle with an unknown color dereferences NULL

Object.lto 0x10025503 (LightCycle::SetModel, ObjectDLL/TRON/LightCycle.cpp).
It reads the color's cycle type (and does nothing with it) before checking
that the CycleColor was found, so a cycle whose CycleColor names no color
reads through NULL before it can print its "Couldn't find Light Cycle
Color" error.

Fixed in the modern build: TRON/LightCycle.cpp (no unused lookup), MODERN.md GAME-67.

### The light cycle missile's stand-in cyclist is left uninitialized

Object.lto 0x10029fd0 (LIGHT_CYCLIST's (float, float, bool) constructor,
Shared/TRON/LightCycleMgr.h; used for LightCycleMissile's static
cyclist, 0x10358ca0). It "calls" the default constructor with
`LIGHT_CYCLIST();`, which builds and destroys a temporary, so only the three
fields it sets afterwards are initialized. The rest of the static is zero
only because it is a static (its direction, flags and power-up are 0, not
the defaults).

Fixed in the modern build: Shared/TRON/LightCycleMgr.h (both constructors use the defaults), MODERN.md GAME-68. The collision test reads only the fields that the constructor sets and the zeroed flags, thus the missile does not change.

### A light cycle power-up's AllowInCustom cannot be turned off

Object.lto 0x1002b502 (LightCyclePowerup::ReadProp, ObjectDLL/TRON/
LightCyclePowerup.cpp). The property is only copied when it is true, and the
default is true, so a power-up set not to appear in custom light cycle games
appears anyway.

Fixed in the modern build: TRON/LightCyclePowerup.cpp (the value is copied, true or false), MODERN.md GAME-69.

### An AI light cycle's Behavior property is ignored

Object.lto 0x1000259e (AILightCycle::ReadProp, ObjectDLL/TRON/
AILightCycle.cpp). The Behavior property ("aggressive", "wander", "avoid")
is read and compared, but the default behavior is then set to wander
whatever it said; "avoid" maps to wander as well. Every AI light cycle
starts wandering.

Not fixed: the change would alter the light cycle races. The property's default is "aggressive", thus with the fix most AI cycles would start aggressive, not wandering. The retail statement after the test sets wander for every value, which can be an intentional change to the balance of the races. The legacy and the modern build keep the retail behavior.

### An AI light cycle only looks at the first trail of each cyclist

Object.lto 0x10002e92, 0x100031d0 (AILightCycle::Probe and ProbeAll). The
loops over a cyclist's trails check `collTrails[0]` every time instead of
the trail of the loop, so the AI does not see older trails (those left by
a trail spike or a derez) in its forward and side probes. CheckPath
(0x100036b9) indexes the trails properly.

Fixed in the modern build: TRON/AILightCycle.cpp (Probe and ProbeAll test collTrails[j]), MODERN.md GAME-70. The AI sees the older trails of a cyclist too.

### An AI light cycle's trigger messages read past a light cycle message

Object.lto 0x10002079 (AILightCycle::ObjectMessageFn). The
MID_LIGHT_CYCLE_INFO case falls through into MID_TRIGGER after
LCI_START_UPDATING, which then reads the rest of the message as a trigger
string pointer.

Fixed in the modern build: TRON/AILightCycle.cpp (the case returns LT_OK, as the retail path after the trigger code), MODERN.md GAME-71.

### Loading object.lto leaks or mis-frees its class module on an error

server.dll 0x10001839 (cb_LoadModule, the engine's shared/src/classbind.cpp).
The module record (12 bytes) is allocated with LTMemAlloc before
object.lto is bound; when the DLL cannot be found or has no ObjectDLLSetup
it is never freed, and on a version mismatch it is freed with the C
runtime's `free()` and not with LTMemFree. LTMemAlloc uses `malloc()` in
this engine, thus only the leak is a real fault. The game then fails to
start the world either way, so it only shows as a leak.

Fixed in the modern build: the engine's shared/src/classbind.cpp (the module is allocated after the binding succeeds, and freed with LTMemFree), MODERN.md EN-80. Lithtech.exe compiles the same file.

### A refused connection leaves the client waiting on a closed socket

server.dll 0x1003bfaa (CUDPDriver::ReallyJoinSession, the engine's
kernel/net/src/sys/win/udpdriver.cpp; Lithtech.exe has the same driver).
When the server accepts but the engine's NewConnectionNotify refuses the
connection, the new connection is deleted and its socket closed, and the
loop then goes on receiving on that closed socket until the connection
time-out (10 s) passes and LT_TIMEOUT is returned. It should stop at once
with an error (or not close the socket it keeps using).

Fixed in the modern build: the engine's kernel/net/src/sys/win/udpdriver.cpp
(ReallyJoinSession returns LT_ERROR at once after it closes the socket),
MODERN.md EN-79. Lithtech.exe compiles the same file.

### A model whose child model is not loaded is checked through a NULL pointer

server.dll 0x1001a1ac (Model::Load, the engine's model/src/model_load.cpp;
the check itself at 0x1001a4ee). Each child model of a model file is
loaded through the request's callback, and its skeleton is then compared
with the parent's node for node. The callback may load nothing (pModel
NULL, LT_NOCHANGE: the default callback, or a request without child
models), and the comparison then reads the NULL model's node count: a
crash. It should skip the check when there is no child model, as the
loader does for the rest of the child's setup. Lithtech.exe has no such
check (no "Skeleton Mismatch" string; the developer build's exe has it),
and the game's own local server is linked into it: only server.dll (the
dedicated server's) can crash this way.

Fixed in the modern build: model_load.cpp, MODERN.md EN-72.

### A saved tournament mode is never loaded back

CShell.dll 0x100ea210, TRONSrv.exe 0x408160 (ServerGameOptions::LoadFromBute,
src/Game/Shared/ProfileUtils.cpp). SaveToBute (CShell.dll 0x100e9ab0) writes
the disc tournament's "TournamentMode", but LoadFromBute reads every other
tournament key and not that one, so the mode a player saves in a config
comes back as the default every time the config is loaded, in the game's
host screens and on the dedicated server.

Fixed in the modern build: Shared/ProfileUtils.cpp (CShell.dll and TRONSrv.exe), MODERN.md GAME-109.

### A config without the light cycle power-up keys turns every power-up off

CShell.dll 0x100ea210, TRONSrv.exe 0x408b22 (ServerGameOptions::LoadFromBute,
src/Game/Shared/ProfileUtils.cpp). Clear allows every power-up
(m_nLCPowerups = 0xFFFFFFFF), but LoadFromBute sets the mask to 0 before it
reads the nine "PowerupAllow*" keys and passes each bit as its key's
default: the bit it has just cleared. A key missing from the config (an
older or hand-written one) leaves its power-up disallowed instead of at
the default, allowed.

Fixed in the modern build: Shared/ProfileUtils.cpp (CShell.dll and TRONSrv.exe), MODERN.md GAME-110.

### The packet parser clears four bytes of its message buffer

server.dll 0x10021a76 (ParsePacket, src/Engine/runtime/kernel/net/src/
netmgr.cpp). On the first packet it clears the buffer with
`memset(aMsgBuff, 0, sizeof(aMsgBuff))`, the size of the pointer rather than
of the buffer, then appends to it with LTStrCat; it only works because the
first byte is cleared. The same line is in Jupiter's later netmgr.cpp. Only
the ParseNet console variable's debug output uses it.

Fixed in the modern build: netmgr.cpp, MODERN.md EN-73.

### A polystring's drop shadow takes the text colour at one corner

Lithtech.exe 0x563e20 (CUIPolyString_Impl::ApplyColors, src/Engine/runtime/
ui/src/cuipolystring_impl.cpp). Each character's shadow polygon is coloured
`SetRGBA4(shadow, m_pColors[0], 0xff000000, 0xff000000, 0xff000000)`: the
first vertex (the top left) gets the text's own colour where the other three
are opaque black, so every shadowed glyph has a tinted corner fading into
it. The formatted polystring's tag colouring (0x569550) makes the shadow
black at the text's alpha, as was meant; probably `m_pColors[0] &
0xff000000` was meant here.

Fixed in the modern build: cuipolystring_impl.cpp, MODERN.md EN-74.

### A colour tag without three numbers reads past the end of the text

Lithtech.exe 0x569830 (CUIFormattedPolyString_Impl::ParseColor, src/Engine/
runtime/ui/src/cuiformattedpolystring_impl.cpp). After "<color " the parser
skips to the next digit, reads a number, and does so three times, without
stopping at the string's end or at the tag's '>': a malformed tag such as
"<color red>" walks on through the terminator into whatever memory follows
the string.

Fixed in the modern build: cuiformattedpolystring_impl.cpp, MODERN.md
EN-75.

### Anisotropic filtering is never switched on

Lithtech.exe 0x4ca140 (CD3D_Device::SetDefaultRenderStates, src/Engine/
runtime/render_a/src/sys/d3d/d3d_device.cpp) and d3d_ReadExtraConsoleVariables
(d3d_init.cpp). With the Anisotropic console variable set, the renderer
asks for D3DTEXF_ANISOTROPIC minification and magnification only if the
device's VertexTextureFilterCaps (D3DCAPS9 +0x124) has the anisotropic
bits: the filter caps of vertex shader texture sampling, which no card of
the time had. It should test TextureFilterCaps. So anisotropic filtering
only ever sets D3DSAMP_MAXANISOTROPY, and the textures stay bilinear or
trilinear. Build 69's Direct3D 9 renderer has the same lines.

Fixed in the modern build: d3d_device.cpp and d3d_init.cpp, MODERN.md
EN-34.

### A new system memory configuration leaks the old bad blocks

CShell.dll 0x100954b0 (CSubroutineMgr::SetSystemMemoryConfiguration,
src/Game/ClientShellDLL/TRON/SubroutineMgr.cpp). Before it lays out the new
bad blocks and base code, it erases the player's unusable subroutines (the
old bad blocks and base code, made by AddBadBlock/AddBaseCodeBlock with
debug_new) from its array without deleting them. Each new configuration
(each level's) leaks those PlayerSubroutines (0x2c bytes each).

Fixed in the modern build: TRON/SubroutineMgr.cpp, MODERN.md GAME-111.

### A team's alternative name is read past its table for teams 4 to 7

CShell.dll 0x100fafc0 (CTronTeam::GetName, Shared/SharedScoring.cpp). A
team of more than one player takes its name from one of two string id
tables by its id, for ids up to 7; the first table (0x101ea528) has eight
ids, the second (0x101ea548, chosen by the third argument) only four, so
teams 4 to 7 read whatever follows it (code addresses) as a string id and
get an empty or wrong name.

Fixed in the modern build: Shared/SharedScoring.cpp, MODERN.md GAME-112 (teams 4 to 7 take the name from the first table).

### A UI static text or check box reads address 0 when it first gets text

Lithtech.exe 0x571760 (CUIStaticText_Impl::SetText) and 0x56f4c0
(CUICheck_Impl::SetText, src/Engine/runtime/ui/src/cuistatictext_impl.cpp,
cuicheck_impl.cpp). The first SetText creates the widget's polystring with
`CreateFormattedPolyString(m_pFont, NULL, ...)`, whose text parameter is a
`const std::wstring&`: the NULL becomes a wide string read from address 0
(0x571815: `cmp WORD PTR ds:0x0`), an access violation. NOLF2's
CreateFormattedPolyString took a `const char*`, for which NULL was safe;
Tron's wide polystrings made it a crash. The game itself makes none of
these widgets, so it is never reached in play.

Fixed in the modern build: cuistatictext_impl.cpp, cuicheck_impl.cpp,
MODERN.md EN-76.

### The MaxFPS frame limiter times the frame from garbage

Lithtech.exe 0x40df21 (CClientMgr::Update, cm_HandleMaxFPS inlined; src/
Engine/runtime/client/src/clientmgr.cpp). Update declares frameTicks for
the frame-rate limiter but sets it only in the debug build's tick
counting; the final build still calls the limiter, which divides that
uninitialised value (the stack slot last holding the frame's time in
milliseconds, 0x40df80) by the counter's ticks per second. With MaxFPS
(0, off, by default) set, the wait per frame bears no relation to how
long the frame took.

Fixed in the modern build: clientmgr.cpp does not call it; its own frame
limiter replaces it (MODERN.md EN-40).

### The VersionInfo console command calls a null pointer

Lithtech.exe 0x41ad9f (con_ShowVersionInfo, src/Engine/runtime/client/src/
consolecommands.cpp) and 0x40bef6 (CLTClient::InitFunctionPointers,
clientde_impl.cpp). The command calls ILTClient's GetVersionInfoExt
(+0x2b8) without testing it, and the engine never sets that pointer (nor
GetPerformanceInfo's beside it), so typing it in the console crashes the
game.

Fixed in the modern build: clientde_impl.cpp sets both, and the command
tests the pointer (MODERN.md EN-3).

### HighPriority announces itself every frame and cannot be undone

Lithtech.exe 0x405193 (RunClientApp's main loop, src/Engine/runtime/
kernel/src/sys/win/client.cpp). The loop compares the HighPriority console
variable with bPrevHighPriority, which it never updates: while the
variable is set, every frame prints "Setting process to high priority" to
the console and sets the priority class again, and clearing it never puts
the process back to normal priority (the compiler drops that branch, and
its string, as unreachable).

Fixed in the modern build: the engine's kernel/src/sys/win/client.cpp
(the loop keeps the value in bPrevHighPriority), MODERN.md EN-81.

### DrawPrim never uses its camera's viewport

Lithtech.exe 0x41d8e0 (CD3DDrawPrim::SetCamera, src/Engine/runtime/
render_b/src/sys/d3d/d3ddrawprim.cpp; Build 69's code). When the draw
primitive's camera rectangle differs from the current viewport, it sets
the current viewport again (only its depth range changes) instead of the
camera's rectangle, so primitives drawn through a camera that does not
cover the screen are not confined to it.

Fixed in the modern build: d3ddrawprim.cpp sets the camera's rectangle
(MODERN.md EN-8).

### The music thread's handle is never closed

Lithtech.exe 0x43e340 (CLTDirectMusicMgr::Term, src/Engine/runtime/kernel/
src/sys/win/ltdirectmusic_impl.cpp). Term tells the notification thread
to stop and waits for it (WaitForSingleObject, 0x43e393), but does not
close the thread's handle. Each start and stop of the music (a level, the
menus) leaks one handle.

Fixed in the modern build: ltdirectmusic_impl.cpp closes the handle
(MODERN.md EN-4).

### A music change without a transition uses a segment state it did not test

Lithtech.exe 0x43f7e5 (CLTDirectMusicMgr::ChangeIntensity, ltdirectmusic_
impl.cpp). When an intensity change has no transition segment, the code
plays the first segment of the new intensity and calls QueryInterface on
the segment state that PlaySegment returned, without a test: if
PlaySegment fails, it calls through a null pointer. The path with a
transition segment tests the pointer (0x43f6c5).

Fixed in the modern build: ltdirectmusic_impl.cpp tests the pointer on
this path too (MODERN.md EN-5).

### Without a sound driver, the engine calls Term through a null pointer

Lithtech.exe 0x4b1926 (SoundSys, src/Engine/runtime/sound/src/
soundmgr.cpp, 0x4b1890). When the named sound driver does not load, the
engine takes the default driver. If that does not load either, the code
calls Term on the null pointer that it got.

Fixed in the modern build: soundmgr.cpp leaves the pointer null
(MODERN.md EN-7).

### Stopping secondary segments or motifs can hang the game

Lithtech.exe 0x43fb50 (CLTDirectMusicMgr::StopSecondary) and 0x43fec0
(CLTDirectMusicMgr::StopMotif, ltdirectmusic_impl.cpp). To stop a segment
by name, the code finds it again and again until the search finds
nothing. A segment that it does not clean up (one to play to its end, or
one without a DirectMusic segment) is found again each time: the loop does
not stop, and it holds the lock of the command queue. To stop all, the
loop goes to the next segment only after a segment that has a segment
state, so a segment without one stops the loop from advancing.

Fixed in the modern build: one walk over the list, for a name or for all
(MODERN.md EN-10).

### Light and surface accessors use a null handle

Lithtech.exe 0x40b2e0 (ci_GetLightRadius; one body with the server's
si_GetLightRadius), 0x40b300 (ci_SetLightRadius, src/Engine/runtime/
client/src/clientde_impl.cpp) and 0x495e30 (cis_GetSurfaceDims,
client/src/sys/win/winclientde_impl.cpp). The two light functions read
the object's type through the handle without a test; the other client
accessors test it. GetSurfaceDims, for a null surface, writes 0 to both
outputs, also when the caller gave a null pointer for one of them (the
other branch tests each pointer).

Fixed in the modern build: the tests are added (MODERN.md EN-11).

### The music's InitLevel trace misspells sDefine3

Lithtech.exe 0x43e570 (CLTDirectMusicMgr::InitLevel,
ltdirectmusic_impl.cpp). The trace at LTDMConsoleOutput 3 says
"sDeine3=" (the string is at 0x5b36e4).

Fixed in the modern build: the text says "sDefine3" (MODERN.md EN-12).

### A console print that fills its buffer writes past it

Lithtech.exe 0x408740 (CLTClient::CPrint, src/Engine/runtime/client/src/
clientde_impl.cpp) and 0x42a630 (dsi_PrintToConsole, kernel/src/sys/win/
dsys_interface.cpp). The message is formatted into a buffer of 512 (500)
bytes; if its last character is not a newline, the code writes a newline
and a terminator after it. A message that fills the buffer gets them one
and two bytes past its end. An empty message reads the byte before the
buffer.

Fixed in the modern build: the format keeps a byte for the newline, and
an empty message is tested (MODERN.md EN-13).

### Engine error messages give the name of another function

Lithtech.exe, the debug messages of RETURN_ERROR and CHECK_PARAMS (src/
Engine/runtime/client/src/): ci_GetSoundTimer (0x409790) reports its bad
parameters as "GetSoundDuration" (0x40987f); ci_SortParticles (0x40ad20)
as "CLTClient::OptimizeParticles"; ci_FreeServiceList (0x40b6a0) as
"CLTClient::GetServiceList"; CLTClient::SetCanvasRadius (0x408260) as
"CLTClient::GetCanvasRadius"; CLTClient::GetMaxRadiusInPoly (0x4082d0,
0x40831b) as "CLTServer::GetMaxRadiusInPoly"; in_GetPerformanceInfo
(0x444110, 0x444181) its size check as "ClientDE::GetVersionInfoExt";
cis_UnoptimizeSurface (0x495a20) as "OptimizeSurface"; cis_RenderObjects
(0x497da0) its first three checks as "RenderCamera";
CLTPhysicsClient::SetObjectDims (0x4af3c0) its parameter check as
"SPhysicsLT::SetObjectDims"; CSoundMgr::SetSoundFilter (0x4b4cb0) as
"CSoundMgr::SetSoundFilterParam". The correct names are not in the retail
image.

Fixed in the modern build: each message names its own function
(MODERN.md EN-14).

### streamsim_Open leaves its file open when it cannot make the stream

Lithtech.exe 0x485480 (streamsim_Open, src/Engine/runtime/kernel/src/sys/
win/streamsim.cpp). If the stream object cannot be allocated, the
function returns null but does not close the file that it opened.

Fixed in the modern build: streamsim.cpp closes the file (MODERN.md
EN-15).

### Client prediction follows closer and smooths less at a high frame rate

Lithtech.exe 0x467630 (pd_Update, src/Engine/runtime/client/src/
predict.cpp). Each frame the client moves each object that the server
moves half of the way from where it is to its predicted position, and
turns it half of the way to its last known rotation (quat_Slerp with
0.5). The share is for a frame, not for a time: at 240 frames a second an
object follows the prediction four times as closely each 60th of a second
as at 60, and its motion is smoothed less.

Fixed in the modern build: the share is that of a 60th of a second,
1 - 0.5^(60 x frame time) (predict.cpp, MODERN.md EN-45).

### Cancelling a decision window resets the memory after its choices

CShell.dll 0x10140810 (CHUDDecision::Choose, ClientShellShared/
HUDDecision.cpp). A choice of MAX_DECISION_CHOICES or more (cancelling the
window) is clamped to MAX_DECISION_CHOICES and, when the layout's
RemoveItems is set, m_Choices[nChoice].Reset() runs on it: one past the
array. That "choice" starts at the window's CTexturedFrame (+0x1b54) and
runs past the end of CHUDDecision (0x1f6c bytes) into CHUDTronDecision's
members: Reset clears the frame's first byte, frees the two pointers it
finds at +0x1fcc/+0x1fd0 as poly strings if they are set, and writes 200
and two nulls at +0x1fc8..+0x1fd0.

Fixed in the modern build: ClientShellShared/HUDDecision.cpp, MODERN.md GAME-113.

### The textured frame tuners accept a tenth piece

CShell.dll 0x1013e740 and 0x1013e8c0 (CTexturedFrame::AdjustPiece and
AdjustPieceUV, ClientShellShared/TexturedFrame.cpp, the "Frame" and
"FrameUV" console programs). The piece index is checked against 9, not 8,
so piece 9 writes past the nine quads and texture rects. A debugging aid.

Fixed in the modern build: ClientShellShared/TexturedFrame.cpp, MODERN.md GAME-114.

### The host screens pass a std::string to a "%s" format

CShell.dll 0x10003f10 (CBaseScreenHost::LoadOptions, ClientShellDLL/TRON/
BaseScreenHost.cpp). The option set's name is passed to FormatString's
"..." as the std::string object itself (its twelve bytes copied), not its
c_str(). It works only because STLport's string starts with the pointer to
its text, which "%s" reads; the other two words are ignored.

Fixed in the modern build: TRON/BaseScreenHost.cpp, MODERN.md GAME-115.

### The host screen never sets the default remote-command password

CShell.dll 0x1005bb20 (CScreenHostConnection's constructor, ClientShellDLL/
TRON/ScreenHostConnection.cpp). It loads the default password (string
0x1c0) into m_sPassword twice; NOLF2's CScreenHost loads it into the
password and then the remote-command (SCMD) password. m_sScmdPassword stays
empty until the option set is read (LoadOptions sets both), so nothing
shows in the game.

Fixed in the modern build: TRON/ScreenHostConnection.cpp, MODERN.md GAME-116.

### The upgrade points ring is placed with the horizontal scale only

CShell.dll 0x1004fc70 (CBuildPointsCtrl::ScreenDimsChanged, src/Game/
ClientShellDLL/TRON/RatingBarCtrl.cpp). The subroutine screen's ring of
upgrade points scales its centre, its top and bottom and its radii from
640x480 by the interface's X ratio alone (it reads the X ratio twice where
the vertical values want the Y ratio), so at any resolution that is not
4:3 the ring's hit area (0x1004fe60) and the help text's position
(0x1004fef0) are off vertically from the ring that is drawn.

Fixed in the modern build: TRON/RatingBarCtrl.cpp, MODERN.md GAME-117. (In the modern build CL-30 gives the interface one scale for the two axes, so the two ratios are equal and the ring was correct there already.)

### The light cycle scores never free their controls

CShell.dll 0x10035500 (CHUDScores::Term, src/Game/ClientShellDLL/TRON/
HUDScores.cpp). Term destroys the score titles' column control and three
of the frames only when the controls were never created (it tests the
"created" flag the wrong way round), so the controls it did create are
never destroyed, and the header, footer and row column controls are not
in its list at all.

Fixed in the modern build: TRON/HUDScores.cpp, MODERN.md GAME-118.

### The sky is clipped to the whole screen, not to the camera's viewport

Lithtech.exe 0x52cd00 (CD3D_RenderBlock::ExtendSkyBounds, the engine's
d3d_renderblock.cpp). The bounds of the sky portals seen through a camera
are tested against and clamped to 0..m_fScreenWidth/m_fScreenHeight, the
screen's extents, where they should be the view's rectangle (m_Rect). With
a camera that draws into part of the screen (a split or inset view) the sky
is drawn for portals outside that view, or past its edges. Contract
J.A.C.K.'s engine has the fix ("SKYBOX FIX for Multiple Cameras": m_Rect).
The extents are in screen coordinates (d3d_InitFrustum2 puts a view at
its rectangle), so the error is in each view that does not start at the
top left of the screen. Tron 2.0 has one: the I/O node camera (the inset
view at the top middle of the screen, LAYOUT.TXT IOCameraRect). For it,
a sky portal that lies in the view is skipped, because its left edge is
past the view's width; a large portal is clamped to 0..width, which is
to the left of the view. The sky's frustum (d3d_InitViewBox2) is then
made from bounds outside the view, and the inset shows the sky that is
to the side of its view, or no sky. The engine sets no viewport of its
own for the sky (d3d_drawsky.cpp: commented out, for "second camera
viewports"), so the sky is drawn over the whole inset with that frustum. In M01-01 and
M01-02 the sky in the inset's direction is a haze of one colour, so the
error does not show there (build/playtest-skyview.txt).

Fixed in the modern build: the engine's render_a/src/sys/d3d/
d3d_renderblock.cpp (the bounds are tested against and clamped to the
view's rectangle), MODERN.md EN-82.

### The sound driver's parameter fades lock a mutex nobody creates

SndDrv.dll, the parameter fades (src/Engine/snddrv/s_dx8.cpp). Tron's
driver keeps 16 fades of a sample's playback frequency (CDx8SoundSys's
m_aSoundParams, +0x4c4), which its own thread steps every 20 ms
(SoundParamThread, 0x10003430) while the engine's sound manager sets them
up, switches them and ends them (CDx8SoundSys's slots 23..28,
0x10005c60..0x10005f10; Stop3DSample 0x10006990, End3DSample 0x10006c10,
StopSample/EndSample 0x10007d00). Both sides mean to lock them, but neither
lock exists: the thread opens a mutex named "SoundParamMutex"
(OpenMutexA, 0x10003448) that nothing in the game creates -- no binary of
Tron 2.0's has a CreateMutex of that name -- so it gets NULL, and the
driver's own handle (m_hSoundParamMutex, +0xb0) is only ever cleared
(Reset, 0x100039d6). Every WaitForSingleObject and ReleaseMutex on them
fails at once, and the thread and the game's calls touch the fades
unsynchronised: a fade can be freed or reassigned while the thread is
stepping it. Term also closes the never-opened handle (0x100042d9). The
driver should create the mutex (CreateMutexA in Init) and keep its handle
in m_hSoundParamMutex for both sides. Also, the game pauses the fades
with SuspendThread (slot 29, 0x10005fa0). With a real lock, a thread
suspended while it holds the lock stops every later fade call; and in
retail, Term while the thread is suspended waits for it without end.

Fixed in the modern build: s_dx8.cpp, MODERN.md EN-77.

### The sound driver blames the MP3 codec when the PCM converter is missing

SndDrv.dll 0x10005083 (CDx8SoundSys::Startup, src/Engine/snddrv/s_dx8.cpp).
Startup opens the ADPCM decompressor, then the MP3 decompressor, then the
PCM converter; only the last failing shows the message box "Sound System
Startup - Missing .mp3 codec", though the MP3 driver opened. Build 69's
source has the same code under `#ifndef _FINAL`; Tron's Final build shows
it. The message should name the converter (and a missing MP3 codec, which
fails silently, should be the one reported).

Fixed in the modern build: s_dx8.cpp, MODERN.md EN-78.

### A streamed MP3's last block gives the codec the stale end of the block before

SndDrv.dll 0x100019a0 (WaveFile::ReadCompressed, src/Engine/snddrv/
s_dx8.cpp). A streamed MP3 sound (WAVE_FORMAT_MPEGLAYER3) is read in blocks
of the size that acmStreamSize gave for 8 KB of PCM, and each block goes to
the codec with acmStreamConvert. CDx8SoundSys::OpenStream sets the
header's cbSrcLength to that size once. ReadCompressed does not change it
when the last read of the data chunk is shorter, so the codec gets the
bytes of this read and, after them, the end of the block before. These
stale bytes are MP3 frames of the sound itself, and the codec can decode
them: the end of a streamed line can play a fragment of audio that came
earlier. The convert should get only the bytes that were read
(cbSrcLength = cb).

Fixed in the modern build: src/Engine/snddrv/s_dx8.cpp (TRON20SRC_MODERN)
(MODERN.md X-2).

### A player model without a stand animation gets standing dims as its crouch dims

CShell.dll 0x100f8830 (CMoveMgr::CanStandUp, ClientShellShared/CMoveMgr.cpp).
Tron reads the crouch and stand dims the first time the player asks to stand
up. When the model has no "LSt" animation the default stand dims (24, 53, 24)
are written over m_vCrouchDims instead of m_vStandDims (0x100f88a2), so the
stand dims stay zero and the crouch dims become the standing size: the next
"am I crouching" test is wrong and standing up tests against empty dims.
Player models normally have the animation, so it does not show in the game.

Fixed in the modern build: CMoveMgr.cpp (TRON20SRC_MODERN sets
m_vStandDims) (MODERN.md GAME-1).

### A client weapon's alt-fire key-up follows the fire key

CShell.dll 0x10130680 (CClientWeapon::Update, ClientShellShared/
ClientWeapon.cpp). Tron's update takes a second, alt-fire key and keeps its
state from last update (+0xc2) to call the weapon's alt-fire key-down and
key-up transitions (slots 0xf8, 0xfc). The key-down test uses the alt-fire
key, but the key-up test (`m_bAltFireKeyDownLastUpdate && !bFire`) and the
remembered state (`m_bAltFireKeyDownLastUpdate = bFire`) use the fire key:
the alt-fire state copies the fire key, so its key-up transition runs when
the fire key is up rather than when alt-fire is released, and an alt-fire
key held down calls the key-down transition every update while fire is up.
Both should use bAltFire.

Not fixed: harmless, and a fix changes how the disc is recalled. No weapon has a key-up transition (slot 0xfc is empty in CClientWeapon and in each Tron weapon). Only the disc has a key-down transition (slot 0xf8, CClientWeaponDisc::TronUnknownVirtualF8: SendDiscReturn). While alt-fire is held and fire is not, retail sends MPROJ_RETURN each update, and the server returns the disc at the first update that it is not deflecting (ProjectileDisc.cpp). Thus "hold alt-fire" recalls the disc as soon as it can return. With bAltFire in the test and the state, a recall pressed during a deflection is lost.

### A prop's looping sound name is never freed

Object.lto 0x100fa080 (Prop::~Prop, ObjectShared/Prop.cpp). Tron's props
keep the name of a looping sound (the BUTE_SOUND_KEY_LOOPING model string)
as an engine HSTRING (+0x838). The destructor kills the sound but never
frees the string, so every prop that played a looping sound leaks its name
when it is removed. KILL_LOOPING_SOUND and a new looping sound free it.

Fixed in the modern build: Prop.cpp (TRON20SRC_MODERN frees
m_hstrLoopSound) (MODERN.md GAME-2).

### ARMOR_UNLOCK never unlocks a power armor

Object.lto 0x1013b430 (PowerArmor::ObjectMessageFn, ObjectShared/
PowerArmor.cpp). Tron's power armor can be locked on or off (ARMOR_LOCK_ON,
ARMOR_LOCK_OFF) and has an ARMOR_UNLOCK command (its string is in the image,
0x102987b0), but the trigger handler compares the message with an empty
string instead (0x1013b5b4): ARMOR_UNLOCK falls through and does nothing,
and only an empty trigger message unlocks the armor. A locked armor stays
locked.

Fixed in the modern build: PowerArmor.cpp (TRON20SRC_MODERN compares with
KEY_UNLOCK) (MODERN.md GAME-3).

### A "REMOVE" message does not remove a lite object

Object.lto 0x10153720 (GameBaseLite::OnTrigger, ObjectShared/
GameBaseLite.cpp). A lite object (CommandObject, Group, Controller, the AI
volumes) that is sent REMOVE while registered answers that it handled the
message, but Tron's code never asks the CLiteObjectMgr to remove it (NOLF2's
calls RemoveObject), so the object stays and keeps updating. A level script
that removes a command object to stop it does nothing.

Fixed in the modern build: GameBaseLite.cpp (TRON20SRC_MODERN calls
CLiteObjectMgr::RemoveObject) (MODERN.md GAME-4).

### A searchable object's MaxSearchTime is read into its minimum

Object.lto 0x10194700 (CSearchable::ReadProp, ObjectShared/Searchable.cpp).
The search aggregate reads MinSearchTime and then MaxSearchTime, but stores
the second one over the minimum too (0x1019478d writes +0x18, not +0x1c), as
build 51's code already did. A level designer's MinSearchTime is lost and the
maximum stays unset, so InitialUpdate gives it the 3 second default: a search
lasts a random time between MaxSearchTime and 3 seconds.

Fixed in the modern build: Searchable.cpp (TRON20SRC_MODERN stores
m_fMaxSearchTime) (MODERN.md GAME-5).

### An active world model's ADDPSET reads the sender, not its argument

Object.lto 0x10176140 (ActiveWorldModel::OnTrigger, ObjectShared/
ActiveWorldModel.cpp). Tron's doors and switches take ADDPSET <digits> and
REMOVEPSET <digits> to change the permission sets a player needs. ADDPSET
measures and scans the bytes of the sender's object handle (0x10176709: the
argument's address is formed and dropped), so the digits given are ignored
and whatever bytes the handle points at decide which sets are added.
REMOVEPSET uses its argument but always tests eight characters (0x101767ad),
reading past the end of a shorter argument.

Fixed in the modern build: ActiveWorldModel.cpp (TRON20SRC_MODERN parses
the argument, up to its length) (MODERN.md GAME-6).

### An EventCounter's REPORT message is registered as a second RESET

Object.lto 0x10260d30 (EventCounter's command manager table,
ObjectShared/EventCounter.cpp). Tron added RESET and REPORT messages to the
event counter (OnTrigger 0x102611e0 handles both), but the table entry for
REPORT is named "RESET" with the syntax "REPORT" (a copy of the line above):
the command manager knows RESET twice and REPORT not at all, so the
editor-time command check (CCommandMgrPlugin::PreCheckMsg) rejects a
command sending REPORT to an event counter as an unknown message, although
the game itself handles it.

Fixed in the modern build: EventCounter.cpp (TRON20SRC_MODERN registers
REPORT) (MODERN.md GAME-7).

### An AI path's initial direction ignores its second control point

Object.lto 0x10231970 (CAIPath::GetInitialDir, ObjectShared/AIPath.cpp).
Tron's version finds the first two control-point waypoints and stores the
unit direction from the first to the second in the result, then breaks out
of the loop into the code meant for a path with a single control point,
which overwrites the result with the unit direction from the first control
point to the AI's position (0x10231aff jumps back to 0x102319da). The
direction to the second point is never returned, and for an AI standing on
the first control point the result divides a zero vector by zero.

Fixed in the modern build: ObjectShared/AIPath.cpp (it returns the direction to the second control point; an AI on the only control point gets its forward vector), MODERN.md GAME-72. The direction for one control point stays retail's (from the control point to the AI).

### The sound screen ignores the EAX toggle's layout

CShell.dll 0x10055442..0x1005546a (CScreenAudio::Build,
ClientShellDLL/TRON/ScreenAudio.cpp). Build reads "EAXTogglePos" and
"EAXToggleHeaderWidth" from the layout but does not use them: the EAX
toggle gets the default position and the screen's column width, as the
other controls do.

Not fixed: harmless, because the layout has no such values. [ScreenAudio] in layout.txt (Game.rez, Gamep4.rez) has no EAXTogglePos and no EAXToggleHeaderWidth, so the two reads give (0, 0) and 0. To use them would put the toggle at the page's top left corner.

### The DRAWALLMODELSHADOWS command sets the sky fog near Z

Object.lto 0x101c0c60 (WorldProperties::OnTrigger,
ObjectShared/WorldProperties.cpp). The DrawAllModelShadows message writes
its argument to the SkyFogNearZ console variable, not to
DrawAllModelShadows. NOLF2 has the same bug. A level that sends it changes
the sky fog and never the model shadows.

Fixed in the modern build: ObjectShared/WorldProperties.cpp (it writes DrawAllModelShadows), MODERN.md GAME-73.

### A failed level load restores the level name from freed memory

Object.lto 0x101ca350 (CServerSaveLoadMgr::LoadLevel,
ObjectShared/ServerSaveLoadMgr.cpp). Tron keeps the pointer that
CGameServerShell::GetCurLevel returns (the buffer of the shell's level
CString), sets the new level name, and gives the kept pointer back to
SetCurLevel when the load fails. The assignment can free or reuse that
buffer, so the restored name can be garbage.

Fixed in the modern build: ObjectShared/ServerSaveLoadMgr.cpp (a copy of the old name), MODERN.md GAME-74.

### A relation's timer takes an end time as its duration

Object.lto 0x10168260 (CRelationUser::ResetRelationTime,
ObjectShared/RelationButeMgr.cpp). Tron keeps a CTimer in each relation
momento where NOLF2 keeps the expiration time. ResetRelationTime still
computes the expiration as NOLF2 does, the relation's lifetime plus the
current game time, and starts the timer with that value as its duration.
The timer therefore stops at twice the current time plus the lifetime, not
at the current time plus the lifetime: a temporary relation lasts longer
the later in the level it is reset.

Fixed in the modern build: ObjectShared/RelationButeMgr.cpp (the timer starts with the lifetime), MODERN.md GAME-75. CRelationUser::AddRelation (retail 0x101680a9..0x101680e3) gives the same sum to the momento's constructor, which starts the timer with it. The modern build gives the lifetime there too.

### Following bouncy chunks and particle systems do not follow

ClientFx.fxd 0x1000ff20 (CLTBBouncyChunkFX::GetSpawnTransform) and
0x10017d30 (CParticleSystemFX::GetSpawnTransform; the names are ours).
For the follow type UP_FOLLOW, the function reads the parent's position
into the effect's m_vPos, not into the position it returns. It then adds
the offset to the old position. The chunk or system therefore starts at
its create position plus the offset, not at the parent. The particle
system's function also adds m_vTronUnknown to the offset two times.

Fixed in the modern build: ClientFxDLL/LTBbouncychunkfx.cpp, ClientFxDLL/ParticleSystemFX.cpp, MODERN.md GAME-119 (the parent's position, and m_vTronUnknown added one time).

### A cylinder particle system emits on a line, not in a cylinder

ClientFx.fxd 0x10018660 (CParticleSystemFX::AddParticles,
ClientFxDLL/ParticleSystemFX.cpp). For the emission type PS_eCylinder, the
function turns the two plane vectors m_vPerp1 and m_vPerp2 by the spawn
rotation. It writes both results into the same vector (0x10019253 and
0x100192c9), and then uses that vector for the two terms of the point.
The point is thus on the line of the turned m_vPerp2, not on a circle.
The circle and cone types use two vectors and are correct.

Fixed in the modern build: ClientFxDLL/ParticleSystemFX.cpp, MODERN.md GAME-120.

### The prankster bit does not release its object lists

Object.lto 0x10040950 (CProjectilePrankster::AcquireTargets,
TRON/ProjectilePrankster.cpp). Each update in the acquire state calls
FindObjectsTouchingSphere. The function does not give the list back with
RelinquishList. The engine therefore loses one object list for each frame
while a prankster bit catches targets.

Fixed in the modern build: TRON/ProjectilePrankster.cpp (RelinquishList), MODERN.md GAME-76.

### A disc game team change can use a null player

Object.lto 0x1004b981 (CDiscMatchMgr::ChangeTeam, TRON/ServerDiscMatchMgr.cpp).
After the move, the manager finds the player on the new team. To set the
player's team colour (CPlayerObj 0x31c9), it checks the player only on one
path: when the player is not found, it reads 0x31ca of a null pointer and
writes 0x31c9 through it. The server crashes when the client left the team
list before the move completes.

Fixed in the modern build: TRON/ServerDiscMatchMgr.cpp (no player, no colour), MODERN.md GAME-90.

### The energy claw's range data uses a wrong subroutine name

CShell.dll 0x1001aa70 (CClientWeaponEnergyClaw::GetClawData,
TRON/ClientWeaponEnergyClaw.cpp). The claw looks for the utility
subroutine "EnergyClaw" to select the version and state of its range data.
The subroutine has the name "Energy_claw" (its energy use, 0x1001a680,
uses that name). The look-up always fails, so the claw always uses the
alpha, non-corrupt range and in-range FX.

Fixed in the modern build: TRON/ClientWeaponEnergyClaw.cpp, MODERN.md GAME-121. The beta and gold claws now have the ranges of the subroutine data (448 and 512; alpha 384).

### A permission set bin gives a wrong buffer size

CShell.dll 0x1003c000 (CHUDDecisionBin::Init, TRON/HUDTronDecision.cpp).
For a permission set, Init formats the bin's text into a 128-byte buffer,
but it gives FormatString a size of 1024. A long translated string can
write past the buffer on the stack.

Fixed in the modern build: TRON/HUDTronDecision.cpp, MODERN.md GAME-122.

### The circuit intro destroys two strings twice and two never

CShell.dll 0x1002ce50 (CHUDLightCycleCircuitIntro::TermStrings,
TRON/HUDLightCycleCircuitIntro.cpp). The function destroys the trail
label and the trail value twice. It does not destroy the lives label and
the lives value. Each Init and Term leaks the two lives strings.

Fixed in the modern build: TRON/HUDLightCycleCircuitIntro.cpp, MODERN.md GAME-123.

### The circuit intro can format an unset string ID

CShell.dll 0x1002cf00 (CHUDLightCycleCircuitIntro::UpdateText). The
speed and trail length switches set the string ID only for the indices
0 to 3. For a different index, FormatString gets an unset value from the
stack.

Fixed in the modern build: TRON/HUDLightCycleCircuitIntro.cpp, MODERN.md GAME-124 (an unknown index gives the string of index 0).

### An explosion's bad ammo name is reported as a single character

Object.lto 0x100efb07 (Explosion::ReadProp, ObjectShared/Explosion.cpp).
When the Ammo property names an ammo type that the weapon manager does not
know, ReadProp prints "Explosion::ReadProp - No such ammo type %s". It gives
the first character of the name as the argument, not the name. The %s then
reads a pointer from that small number, and the print can crash the server.

Fixed in the modern build: ObjectShared/Explosion.cpp (the name), MODERN.md GAME-77.

### An arena's plugin checks the wrong properties as commands

Object.lto 0x10110b70 (CArenaPlugin::PreHook_PropChanged, TRON/Arena.cpp).
The plugin must give only the six event properties (OnPlayerSpawn to
OnPointEnd) to the command manager plugin. The tests use the result of
_stricmp without "!". Thus every property that is not one of the six names
goes to the command manager plugin, and so do the six names (a name can not
be equal to all six). In the editor, the Arena property can be reported as a
bad command.

Fixed in the modern build: TRON/Arena.cpp (the tests use "!"), MODERN.md GAME-78. Only the editor runs this code.

### The help menu tests the wrong item before it shows the first file

CShell.dll 0x101c7bb6 (CMenuHelp::OnFocus,
ClientShellShared/MenuHelp.cpp). The menu keeps the control index and the
item index of the first help file. Before it shows that file at once, it
gets the item at the control index from the intel list and tests if the
item is read. The control index is not an item index: build notes come
first in the list, so the menu tests a different item. The menu can show a
file that the player read, or not show a new file.

Fixed in the modern build: TRON/MenuHelp.cpp, MODERN.md GAME-125.

### A light cycle's intro lookup reads the colour before it tests it

CShell.dll 0x100a7c60 (CTronPlayerMgr::TronUnknownVirtual70,
TRON/TronPlayerMgr.cpp). The function gets the cyclist's colour from the
light cycle attributes and reads the colour's cycle type before it tests
the colour for NULL. An unknown colour name makes the client read through
a NULL pointer.

Fixed in the modern build: TRON/TronPlayerMgr.cpp, MODERN.md GAME-126.

### A light cycle's death fx lookup clears four bytes, not the buffer

CShell.dll 0x100a7d50 (CTronPlayerMgr::TronUnknownVirtual74,
TRON/TronPlayerMgr.cpp). When the colour has no death fx, the function
clears the caller's buffer with SAFE_STRCPY. The buffer is a pointer
argument, so the macro's sizeof gives 4: it copies three bytes and writes
the terminator at offset 3, whatever the buffer's size. The branch also
runs only when the count is negative, so an empty list does not reach it
and the random index is taken from 0..-1.

Fixed in the modern build: TRON/TronPlayerMgr.cpp, MODERN.md GAME-127.

### A mission's weapon list looks up the wrong weapon

CShell.dll 0x1013d3e0, Object.lto 0x1015ec10 (MISSION::BuildWeaponsList,
Shared/MissionButeMgr.cpp). The function copies each weapon name from the
list and writes its terminator. Then it gives that terminator character
(always 0) to CWeaponMgr::GetWeapon, not the name. Thus each weapon in
"DefaultWeapons" and "IdealWeapons" gets the ID of weapon 0. If weapon 0
does not exist, the error message gets the character 0 as its "%s"
argument.

Not fixed: a fix changes play. The shipped missions (missions.txt, Game.rez) give "NoWeapon" or "NoWeapon,Disc" as DefaultWeapons, and the ideal weapons begin with "NoWeapon"; weapon 0 is NoWeapon, so the error never shows. The game was made and tested with every one of these weapons as NoWeapon: CPlayerObj::AcquireLevelIdealWeapons gives each ideal weapon by its name through CTronPlayerObj::TronUnknownVirtual127, and with the correct ids it would also give each weapon with 10000 rounds of its default ammo (ObtainWeapon), and AcquireLevelDefaultWeapons would give the disc at the start of each mission.

### The RESETPATH message of a keyframer does not use the rotation of the first key

Object.lto 0x10119c40 (KeyFramer::ResetToKey, our name,
ObjectShared/KeyFramer.cpp). The function copies the rotation of the first
key into a local value. It does not use this value. For each object, it
starts from the identity rotation, and then adds the offset of the object.
Thus, when the keyframer has no target and does not align to the path, the
objects go to the position of the first key with a wrong rotation.

Fixed in the modern build: ObjectShared/KeyFramer.cpp (each object starts from the rotation of the key), MODERN.md GAME-79.

### CScreenSubroutines::OnFocus resets the wrong fade time (CShell.dll)

When the subroutine screen gets focus, it sets the build note count text
and hides it until the intro is done (CShell.dll 0x1007e898..0x1007e8ad,
TRON/ScreenSubroutines.cpp).
It sets the fade flag of the build note count (+0x2084) to false, but it
sets the fade time of the version text (+0x20fc) to zero, not the fade
time of the build note count (+0x2088). The intro sets both fade times
again before they are used (0x1007f0d8, 0x1007f13f), so there is no
visible effect.

Fixed in the modern build: TRON/ScreenSubroutines.cpp, MODERN.md GAME-137.

### The weapon FX message sends a flag as a float, and the client reads one bit

Object.lto 0x101d6750 (CreateClientWeaponFX, ObjectShared/
ClientWeaponSFX.cpp) and CShell.dll 0x1018e060 (CWeaponFX::Init,
ClientShellShared/WeaponFX.cpp). After the impact type, the server writes
two values of 32 bits: a float, then a flag converted to a float. The
client reads the float, then reads the flag as a bool of one bit. The bool
is the lowest bit of the float's representation, which is 0 for 0.0 and
1.0. Thus the client always gets false for the flag.

Not fixed: harmless, because nothing uses the flag. The client gives it only to PlayWeaponSound (CShell.dll 0x100d8730) as its bool argument, and Tron's PlayWeaponSound does not use that argument (ClientShellShared/ClientUtilities.cpp).

### A defense class override converts its zones to radians again

Object.lto 0x10108660 (DEFENSECLASSDATA::InitMembers, Shared/WeaponMgr.cpp).
The function reads "OrientationDeadZone" and "OrientationMaxZone" in
degrees and keeps them in radians. The default value of each read is the
current member, which is in radians. When an override file
(LoadOverrideButes) does not give a zone, the function converts the radian
value again, and the zone becomes approximately 57 times too small.

Fixed in the modern build: Shared/WeaponMgr.cpp (the default is in degrees), MODERN.md GAME-80.

### Adding gear does not check that the gear exists

Object.lto 0x100e9550 (CDestructible::AddGear,
ObjectShared/Destructible.cpp). The function tests the GEAR pointer from
CWeaponMgr::GetGear only before it reads bExclusive. Then it reads fArmor
and fHealth from the pointer without a test. A gear ID that is in range but
has no GEAR record causes a null pointer read.

Fixed in the modern build: ObjectShared/Destructible.cpp (no GEAR record, no gear added), MODERN.md GAME-81.

### The disc class data does not keep its "Controlable" value

Object.lto 0x10145ab0 and CShell.dll 0x10154090 (DISCCLASSDATA::Init,
Shared/FXButeMgr.cpp). The function reads the "Controlable" attribute with
CButeMgr::GetByte, but it does not use the result. It sets bControlable
(0x1d4) to LTTRUE for each disc. Thus a disc that the attributes make not
controllable is controllable.

Fixed in the modern build: src/Game/Shared/FXButeMgr.cpp keeps the value
that GetByte returns (MODERN.md GAME-8).

### The team selection screen reads one row team past its list

CShell.dll 0x10083050 (CScreenTeamSelect::OnCommand, TRON/
ScreenTeamSelect.cpp). The screen has 16 team rows, with the commands
0x433 to 0x442. The command test accepts 0x433 to 0x443. For the command
0x443, the screen reads the team of row 16, which is not in the list of
row teams (+0x7f0): it reads the next member (the spectator header
pointer, +0x830) and moves the local player to that "team". No control
sends 0x443, so there is no visible effect.

Fixed in the modern build: TRON/ScreenTeamSelect.cpp, MODERN.md GAME-128.

### Two core dump help files of a character are in the wrong group

Object.lto 0x10087500 (CCharacter's property list, ObjectShared/
Character.cpp). A character has three groups of core dump item properties
(CoreDumpItem1 to CoreDumpItem3). HelpFile2 and HelpFile3 have the flags of
the first group (PF_GROUP(8)), not of their own groups (PF_GROUP(9) and
PF_GROUP(10)). DEdit shows these two properties in the first item's group.
The game reads them by name, thus play is not affected.

Fixed in the modern build: ObjectShared/Character.cpp (PF_GROUP(9) and PF_GROUP(10)), MODERN.md GAME-82. Only the editor shows the groups.

### A console variable oscillation has a wrong center

CShell.dll 0x1009ec40 (CTronInterfaceMgr::HandleConsoleVarOscillate,
TRON/TronInterfaceMgr.cpp). The message gives a minimum and a maximum. The
function sets the center of the sine to minimum + maximum * 0.5 and the
amplitude to (maximum - minimum) * 0.5. The center must be
(minimum + maximum) * 0.5. Thus the value goes from 1.5 * minimum to the
maximum, not from the minimum to the maximum.

Fixed in the modern build: TRON/TronInterfaceMgr.cpp, MODERN.md GAME-129.

### A console variable restore formats a float as an integer

CShell.dll 0x1009edb0 (CTronInterfaceMgr::HandleConsoleVarStop,
TRON/TronInterfaceMgr.cpp). To set the value that a console variable had
before an oscillation, the function makes the console string with
"%s %4.2d" and gives a double. The "d" conversion reads an integer, so the
string gets the low half of the double, not the value.

Fixed in the modern build: TRON/TronInterfaceMgr.cpp, MODERN.md GAME-130 ("%s %f").

### An interface object render can write past its object array

CShell.dll 0x100cea40 (CInterfaceMgr::RenderInterfaceObject,
ClientShellShared/InterfaceMgr.cpp). The function puts the object and the
interface lights in a local array of six handles (one object and
MAX_INTERFACE_LIGHTS lights). The loop that adds the lights stops at
kMaxFX (107), the limit of the interface fx list, not at the size of the
array. With more than five interface lights, the function writes past the
array on the stack.

Fixed in the modern build: ClientShellShared/InterfaceMgr.cpp, MODERN.md GAME-131.

### A loaded projectile does not get its object flags back

Object.lto 0x101246c0 (CProjectile::Load, ObjectShared/Projectile.cpp).
CProjectile::Save (0x10124320) writes the object flags of the projectile
last. Load gets the current flags of the object, then reads the saved flags
into the same variable and does not use them. NOLF2's Load sets the saved
flags on the object. Thus, after a saved game loads, a projectile has the
flags that the engine gave it, not the flags that it had when the game was
saved (for example, the visible flag that DoProjectile sets).

Fixed in the modern build: ObjectShared/Projectile.cpp (SetObjectFlags with the saved flags), MODERN.md GAME-83.

### A lens flare message is read one field past its end

CShell.dll 0x101a58f0 (LENSFLARECREATESTRUCT::InitFromMessage,
ClientShellShared/LensFlareFX.cpp). After the lens flare fields, the
function reads a flag and, if the flag is set, the name of a socket. The
server does not write this flag: Object.lto 0x10274300
(AddLensFlareInfoToMessage, ObjectShared/SFXFuncs.cpp) ends with the last
blind scale. Thus the client reads past the end of the message. The read
gives zero, so the flare has no socket and works, but only by chance.
The search light message has the same lens flare fields at its end, so the
same read occurs for it.

Fixed in the modern build: ClientShellShared/LensFlareFX.cpp, MODERN.md GAME-132 (the client reads the flag only when the message has more data; a server that writes it is still read).

### The OBB hit test does not give the distance of its hit

Object.lto 0x10139460 (CCharacterHitBox::FindHitNodeOBB,
ObjectShared/CharacterHitBox.cpp). When the model has OBBs, the hit box
tests the ray against them. The function sets the node of the nearest OBB
in the hit, but not the distance along the ray, and not the object that was
hit. The caller (HandleVectorImpact, 0x101397d0) moves the impact point
along the ray by that distance, which stays at its initial value FLT_MAX.
Also, when an OBB that has no skeleton node is nearer than the nearest hit
so far, the function sets eModelNodeInvalid in the hit. Thus a hit on an OBB
that it tested before is lost.

Fixed in the modern build: ObjectShared/CharacterHitBox.cpp (the nearest OBB with a node gives the node, the distance along vDir and the hit box user), MODERN.md GAME-84.

### The handshake does not mask a short player name

Object.lto 0x100de650 (CGameServerShell::ProcessHandshake,
ObjectShared/GameServerShell.cpp). For MID_HANDSHAKE_LETMEIN, the server
makes the expected password from the first four bytes of the player's net
name. The code that keeps only the bytes before the terminator of a name
shorter than four characters uses the logical operator && where it must
use the bitwise operator &. Thus the tests are always false (or do
nothing), and the bytes after the terminator go into the password. The
compiler removes the tests. Build 51 has the same code.

Not fixed: the password is part of the network protocol. The client makes it from the same bytes in the same way, thus the handshake works. If only one side used "&", a modern server and a retail client (or the opposite) would not agree for a name of less than four characters. The legacy and the modern build keep the retail code on the server and on the client.

### A random node search without an AI can return a bad pointer

Object.lto 0x101a8c30 (CAINodeMgr::FindRandomNodeFromThreat,
ObjectShared/AINodeMgr.cpp). When the fifth argument is false and the AI
is NULL, the function returns its node variable without a path check.
That variable holds the last node that the search loop examined, not a
selected node. If no node of the type exists, the loop does not set the
variable, and the function returns an uninitialized pointer. The only
caller (CAIGoalAttackFromRandomVantage::FindNearestAttractorNode) always
gives an AI, so the game does not hit this.

Fixed in the modern build: ObjectShared/AINodeMgr.cpp (without an AI, a random node of the list, or NULL), MODERN.md GAME-85.

### An edit control without text makes a wide string from a null pointer

CShell.dll 0x100bb0d0 (CLTGUIEditCtrl::GetText, Libs/LTGUIMgr/
ltguieditctrl.cpp). In Tron, GetText returns a std::wstring. When the
control has no text string, the function returns LTNULL, as NOLF2's
narrow version did. The compiler makes the wide string from a null
pointer, and the string constructor reads the characters at address 0.
This causes an access violation. The function must return an empty string.
The game always sets the text before it reads it (the message box, the
chat input), so the game does not hit this.

Fixed in the modern build: src/Game/Libs/LTGUIMgr/ltguieditctrl.cpp returns
an empty string (MODERN.md GAME-9).

### The mission text uses its string before it tests it

CShell.dll 0x10166810 (CHUDMissionText::UpdateLayout,
ClientShellShared/HUDMissionText.cpp). The function makes the text string
and calls SetDropShadow on it. Then it tests the string for NULL. If the
font manager cannot make the string, the call uses a null pointer and the
client stops. The test must come before the call.

Fixed in the modern build: src/Game/ClientShellDLL/ClientShellShared/
HUDMissionText.cpp tests the string first (MODERN.md GAME-10).

### The chat input can write one byte past its buffer

CShell.dll 0x100bab10 (CLTGUIEditCtrl::UpdateData, Libs/LTGUIMgr/
ltguieditctrl.cpp). The function clears and fills the caller's buffer for
the length of the text plus one byte. It does not use the size of the
buffer. The chat input (CHUDChatInput, ClientShellShared/HUDChatInput.cpp)
gives a buffer of 64 bytes and a maximum length of 64 characters. Thus a
chat line of 64 characters writes its terminator one byte past the buffer,
into the first character of the first chat history line.

Fixed in the modern build: src/Game/Libs/LTGUIMgr/ltguieditctrl.cpp writes
at most the maximum length, with the terminator (MODERN.md GAME-11).

### The system menu does not show the load control again

CShell.dll 0x101c80f0 (CMenuSystem::OnFocus, ClientShellShared/
MenuSystem.cpp). In a multiplayer game the function hides the save and
load controls. In single player it shows the save control again, but not
the load control. Thus after a multiplayer game, the system menu has no
load control until the game starts again.

Fixed in the modern build: src/Game/ClientShellDLL/ClientShellShared/
MenuSystem.cpp shows the load control too (MODERN.md GAME-12).

### The trigger's key list reports the wrong result

Object.lto 0x101c2840 (CTriggerPlugin::PreHook_EditStringList,
ObjectShared/Trigger.cpp). For the "PlayerKey" property, the function gives
the list to the key manager plugin's PreHook_EditStringList. It keeps the
LTRESULT in a boolean and returns LT_OK when the result is not zero. Thus a
list that the key plugin fills correctly (LT_OK, zero) gives LT_ERROR to
DEdit, and a failure gives LT_OK.

Fixed in the modern build: src/Game/ObjectDLL/ObjectShared/Trigger.cpp
returns the key plugin's result (MODERN.md GAME-13).

### An AI sees a far target instantly

Object.lto 0x101d8810 (CAISenseRecorderTools::ComplexVisibilityCheck,
ObjectShared/AISenseRecorderGame.cpp). When the AI uses the instant see
distance, a target outside of that distance must get a rate from its
distance and the darkness. The code calculates this rate. Then, because an
"else" is missing, it also runs the code for a target inside of the
distance: it sets the rate to 99999999 (seen instantly). Thus only a hidden
target outside of the personal bubble is not seen instantly.

Fixed in the modern build: src/Game/ObjectDLL/ObjectShared/
AISenseRecorderGame.cpp has the "else" (MODERN.md GAME-14).

### A deadly deflected disc with no ammo data reads its FX name from address 0x164

Object.lto 0x1003d1b0 (CDisc::SmiteObject, the name is ours; ObjectDLL/
TRON/ProjectileDisc.cpp). The function gets the disc's class data only when
the disc has ammo data. When it has none, it uses a null class data pointer
and reads the smite FX name at offset 0x164 from it. A disc always has ammo
data after Setup, so the game does not show the bug.

Fixed in the modern build: TRON/ProjectileDisc.cpp (no class data, no FX), MODERN.md GAME-86.

### A grenade bounce reads the projectile FX of a grenade with no ammo data

Object.lto 0x1011c760 and 0x1011d3b0 (CGrenade::HandleImpact and its kept
touch slot, ObjectShared/ProjectileTypes.cpp). Tron multiplies the bounce
dampening by the projectile FX's Elasticity. The functions do not examine
m_pAmmoData or its projectile FX before they read it. A grenade always has
ammo data after Setup, so the game does not show the bug.

Fixed in the modern build: ObjectShared/ProjectileTypes.cpp (no elasticity scale without the data), MODERN.md GAME-87.

### A failed use object path trace can read a null node

Object.lto 0x101e8f50 (CAIHumanStateUseObject::SetNode,
ObjectShared/AIHumanState.cpp). When the AI cannot find a path to the new
node, the function traces the name of its old node. The trace is under
`if( m_pUseNode )` without braces, but AITRACE expands to two statements.
Thus the test holds only the first statement (the console variable's
Init), and the print always runs. If the AIShowStates console variable is
on and the AI has no old node, the print reads the name through a null
pointer and the server stops. The trace is off in a normal game, so the
game does not hit this.

Fixed in the modern build: src/Game/ObjectDLL/ObjectShared/
AIHumanState.cpp puts braces around the trace (MODERN.md GAME-16).

### The core dump item properties can write past their name buffer

Object.lto 0x1008ac40 (CCharacter::ReadProp, ObjectShared/Character.cpp).
The function makes the name of each core dump item property (for example
"SubroutineCondition1") in a buffer of 32 bytes. But it gives _snprintf a
maximum length of 100 bytes. The longest property name is 21 characters,
thus the retail properties do not overflow the buffer. A longer name would
write past the buffer, into the GenericProp after it.

Fixed in the modern build: src/Game/ObjectDLL/ObjectShared/Character.cpp
gives the buffer 100 bytes (MODERN.md GAME-17).

### A hold block straight at the disc uses a defense value that is not set

Object.lto 0x10063f00 (CTronPlayerObj::GetDefensePercentage, ObjectDLL/
TRON/TronPlayerObj.cpp). The function adds an orientation percentage to the
timing percentage. It sets the orientation percentage only when the angle
between the player's view and the disc is MATH_EPSILON or more. When the
angle is less than MATH_EPSILON (the disc comes straight at the view), the
value is not set. The retail code then adds the stack slot of the weapon's
orientation max zone (+0x58). Thus a block straight at the disc takes the
max zone angle (radians) as its orientation defense, not the max defense.

Fixed in the modern build: the orientation percentage starts at 0.0f
(MODERN.md GAME-18).

### The AI cycle spawner error message has one value too few

Object.lto 0x1005f3b0 (CTronGameServerShell::SpawnAICycles, our name,
ObjectDLL/TRON/TronGameServerShell.cpp). When the search for a spawner
"LCSpawner<n>" does not find exactly one object, the function writes the
message "ERROR - Looked for LCSpawner%2d and found %d objects!" to the
console. The format has two values, but the call (0x1005f7e2..0x1005f7f1)
gives only the number of objects. Thus the message shows the number of
objects as the spawner number, and a value from the stack as the number of
objects. The function is not yet byte-identical (the other differences are
in its stack layout and inline expansions), but the instructions of this
call are the same as in the retail code.

Fixed in the modern build: src/Game/ObjectDLL/TRON/TronGameServerShell.cpp
also gives the spawner number (MODERN.md GAME-19).

### The cheat manager reads one entry past its cheat table

CShell.dll 0x100f1e30 (CCheatMgr::IsCheatActive, ClientShellDLL/
ClientShellShared/CheatMgr.cpp). The table has 0x39 entries (CHEAT_MAX),
but the function refuses only codes greater than 0x39. Thus a code of
CHEAT_MAX reads the "active" flag of the word after the table.

Fixed in the modern build: src/Game/ClientShellDLL/ClientShellShared/
CheatMgr.cpp refuses CHEAT_MAX too (MODERN.md GAME-20).

### The light cycle power-up lock test reads past the name table

CShell.dll 0x10069580 (CScreenLightCycleGame::IsPowerupLocked, ClientShellDLL/
TRON/ScreenLightCycleGame.cpp). The function accepts a power-up index from 0
to 11. The name table it reads (0x102162d4) has 11 entries, 0 to 10. Thus
the index 11 reads the pointer after the table. The game calls the function
only with the indexes 0 to 8, so this does not show in the game.

Fixed in the modern build: the index must be less than the number of
power-up types (MODERN.md GAME-21).

### Loading a game without a current weapon reads past the weapon list

CShell.dll 0x10168290 (CClientWeaponMgr::Load, ClientShellDLL/
ClientShellShared/ClientWeaponMgr.cpp). The function reads the current
weapon index (one byte) and then reads the weapon pointer at that index. It
examines the index for "no weapon" (255) only after it reads the pointer.
Thus a save without a current weapon reads entry 255 of the weapon list,
which is past its end. The value is then discarded, so the game does not
use it, but the read can fault.

Fixed in the modern build: the pointer is read only for a valid index
(MODERN.md GAME-22).

### The cluster disc uses its class data when it has no ammo data

Object.lto 0x10035740 (CClusterDisc::Setup,
ObjectDLL/TRON/ProjectileClusterDisc.cpp). The function finds the cluster
disc class data only when the projectile has ammo data. When it has no ammo
data, the pointer to the class data is NULL, but the function reads the
initial explosion suppress time from it (0x10035781). This causes an access
violation.

Fixed in the modern build: the function stops after the base class setup
when it has no class data (MODERN.md GAME-23).

### One energy claw debug message uses a misspelt console variable

Object.lto 0x10192390 (CAIGoalSpecialDamage::IsEnergyClawAttackerAiming, our
name, ObjectDLL/ObjectShared/AIGoalSpecialDamage.cpp). The function writes
its debug messages when the console variable "AIShowEnergyClawDamage" is
set. The message "Early out - Not a character!" uses the variable
"AIShowEnergyCLawDamage" (capital L) instead (0x101924b2). Console variable
names are case-sensitive, thus this one message does not show with the
other messages. It has no effect on the game.

Fixed in the modern build: ObjectShared/AIGoalSpecialDamage.cpp (AIShowEnergyClawDamage), MODERN.md GAME-88.

### The tracked node manager reports the wrong flag as discomfort

CShell.dll 0x101c8f80 and Object.lto 0x10270b10
(CTrackedNodeMgr::IsAtDiscomfort, Shared/TrackedNodeMgr.cpp). The function
must return the flag that tells if the node reached its discomfort extent
(m_bInDiscomfort, +0xb5). It reads the byte at +0xb9, which is the flag that
tells the node to ignore the animation of its parent
(m_bIgnoreParentAnimation; SetIgnoreParentAnimation at 0x101c8f50 writes the
same byte). Thus a caller gets a value that does not change with the
tracking.

Fixed in the modern build: src/Game/Shared/TrackedNodeMgr.cpp returns
m_bInDiscomfort (MODERN.md GAME-24).

### The binary bit ignores the permission set messages

Object.lto 0x10014080 (BinaryBit::ObjectMessageFn, ObjectDLL/TRON/
BinaryBit.cpp). For each trigger message, the function compares the first
word with "ADDPSET" and "REMOVEPSET" to add or remove permission sets. The
retail code compares the pointers (0x10014260, 0x100142b6), not the text:
the parsed word is in the parse buffer, not at the address of the string
constant. Thus the comparisons are always false, and "ADDPSET" and
"REMOVEPSET" do nothing to a binary bit. The class registers the two
messages with the command manager, so the level designer can send them.

Fixed in the modern build: the function compares the text (_stricmp)
(MODERN.md GAME-25).

### The "SetEnergy" trigger limits the energy to the maximum hit points

Object.lto 0x1007994b (CPlayerObj::OnTrigger, ObjectDLL/ObjectShared/
PlayerObj.cpp). The "SetEnergy" command reads a value and limits it to the
range 0.0 to a maximum before it sets the player's energy. The retail code
gets the maximum from CDestructible::GetMaxHitPoints (0x100e82c0), not from
CDestructible::GetMaxEnergy (0x100e82f0). Thus the energy can be more than
the maximum energy when the maximum hit points are more than the maximum
energy, and less than the value that the command gives when they are less.

Fixed in the modern build: the command limits the energy to the maximum
energy (MODERN.md GAME-26).

### The "DestroyProjectile" command can read past its object array

Object.lto 0x10086370 (CPlayerObj::RemoveProjectiles, ObjectDLL/
ObjectShared/PlayerObj.cpp). The function finds the objects with the name
"TronProjectile" into an array of 50 objects. The loop then uses the total
number of objects found, not the number of objects in the array. When the
level has more than 50 such projectiles, the loop reads past the end of the
array.

Fixed in the modern build: the loop uses the number of objects in the array
(MODERN.md GAME-27).

### An attached energy claw does not move with its target

Object.lto 0x1003e320 (CProjectileEnergyClaw::UpdateAttached, ObjectDLL/
TRON/ProjectileEnergyClaw.cpp). The function gets the position of the
target and then sets the position of the target, not of the claw, to that
position. Thus the claw stays where it hit and the call has no effect.

Fixed in the modern build: the claw moves to the position of its target
(MODERN.md GAME-28).

### The AI debug information has operator precedence errors

Object.lto 0x10130580 (CAI::UpdateInfo, ObjectDLL/ObjectShared/AI.cpp).
The "AIInfo" console variable shows debug text for each AI. Tron writes the
text to a stream. Some lines use the "?:" operator after the "<<" operator
without parentheses. Thus the stream, not the value, is the condition, and
the stream gets the value of the condition, not the text that the
condition selects. The script list (AIInfo 2) shows 0 or 1 and an address
in place of "*" and the command text. The goal list (AIInfo 3) shows 0 or
1 in place of "*" and "(scripted)". The function also compares the address
of the new text with the address of the old text, not the two texts. Thus
it sends the text to the clients on each update, also when the text does
not change. The holster text is cut at a "," character, not at a ";"
character, because the function gives the multi-character constant ';,'
to strchr.

Fixed in the modern build: the lines put the selected text, the function
compares the texts and cuts the holster text at ";" (MODERN.md GAME-29).

### An unknown download type gives unset values to the progress callback

CShell.dll 0x10021110 (ghiDeliverIncomingFileData, Libs/GameSpy/
GameSpySDK/ghttp/ghttpProcess.c). The function does not set its buffer
pointer and its buffer length before it examines the type of the
connection. For a type that is not GET, SAVE or STREAM, it gives these two
unset values to ghiCallProgressCallback. The 2003 GameSpy SDK has this
mistake; later SDKs set the values to NULL and 0.

Fixed in the modern build: Libs/GameSpy/GameSpySDK/ghttp/ghttpProcess.c, MODERN.md GAME-133.

### A full HUD bar sets its right cap texture on the wrong polygon

CShell.dll 0x101cc346 (CHUDBar::Update, ClientShellDLL/ClientShellShared/
HUDBar.cpp). When the fill reaches the right cap, the code sets the
position of polygon 2 (the cap) and then sets the full cap's texture
coordinates on polygon 5, which it hides at once. Polygon 2 keeps the
texture coordinates of the last partial fill. NOLF2 sets them on polygon 2.
A bar that goes from part full to full can show a right cap cut short.

Fixed in the modern build (HUDBar.cpp, `TRON20SRC_MODERN`) (MODERN.md
GAME-30).

### A human AI reports its SETONLYDAMAGEBYPLAYER command as not handled

Object.lto 0x1016dead (CAIHuman::HandleCommand, ObjectDLL/ObjectShared/
AIHuman.cpp). The command sets the flag that lets only the player damage
the AI, but the branch does not return true: it goes to the end of the
function, which returns false. Thus OnTrigger tells the sender that the AI
did not handle the command, although the AI changed its state. All the
other commands that the function handles return true.

Fixed in the modern build (AIHuman.cpp, `TRON20SRC_MODERN`: the branch
returns true) (MODERN.md GAME-31).

### A help file and a custom file download at the rate of each other

CShell.dll 0x10051710 (CRatingMgr::GetTransferRate, ClientShellDLL/TRON/
RatingMgr.cpp). CRatingMgr::Init (0x100512d0) reads the attribute
"HelpFileTransfer" into +0x119c and "CustomTransfer" into +0x11a8
(0x1005158f, 0x100515ca). The jump table of GetTransferRate (0x100518a8)
sends a help file (type 5) to the code that reads +0x11a8 (0x10051822) and
a custom file (type 6) to the code that reads +0x119c (0x10051806). Thus
each of the two downloads uses the rate of the other.

Fixed in the modern build (RatingMgr.cpp, `TRON20SRC_MODERN`: each type
uses its own rate) (MODERN.md GAME-32).

### The engine writes an incorrect build number when it starts

Lithtech.exe 0x493d50 (GetLTExeVersion, runtime/kernel/src/sys/win/
version_resource.cpp), from the client start-up at 0x405157 (client.cpp).
The engine reads its build number from an RCDATA resource, number 12346,
in Lithtech.exe. The retail Lithtech.exe has no such resource (and NOLF2's,
Contract J.A.C.K.'s and the 2003-08-05 developer build's do not have it
too). Thus the function returns LT_ERROR, the version stays at its initial
value 0xFFFFFFFF.0xFFFFFFFF, and the debug output says "LithTech build
4294967295.4294967295 initialized in ... seconds." The console variable "version"
(CClientMgr::Init) gets the same incorrect value.

Fixed in the modern build: the engine's shared/src/version_info.cpp
(LTVersionInfo::GetString writes "unknown" for a version that is not
set), MODERN.md EN-83. The modern build has no true build number to give
the engine, so it does not add the resource: the debug output says
"LithTech build unknown initialized in ... seconds.", and the variable
"version" is "unknown". Nothing reads the variable.

### The player runs slower at a high frame rate

CShell.dll 0x100f6f20 (CMoveMgr::UpdateNormalMotion, ClientShellShared/
CMoveMgr.cpp; our function is not yet byte-identical, so this is from
the retail code). The code caps the player's speed at the start of each
frame (0x100f715f, 0x100f7220), and then the frame's acceleration takes
the speed past the cap. The shorter the frame, the less the speed goes
past it: in M01-01 the run's top speed was 346 units a second at 59
frames a second and 313 at 254. In spectator mode the speed is multiplied
by 0.9 each frame (0x100f7989), so it falls faster at a higher frame
rate. A jump also goes a little higher at a lower frame rate (82.8 units
at 60 frames a second, 80.8 at 254): the engine's physics moves by the
frame's speed before it takes off the frame's gravity.

Fixed in the modern build: the cap and the damping are applied each 60th
of a second, and a jump above 60 frames a second starts as fast as at 60
(CMoveMgr.cpp, MODERN.md EN-45).

### A blocked player stops sooner at a high frame rate

CShell.dll 0x100f6ac0 (CMoveMgr::UpdateMotion, ClientShellShared/
CMoveMgr.cpp; not yet byte-identical, from the retail code). When the
player cannot move, the code halves the horizontal speed each frame
(0x100f6eaf, 0x100f6ebd), so the speed falls faster at a higher frame
rate.

Fixed in the modern build: halved each 60th of a second (CMoveMgr.cpp,
MODERN.md EN-45).

### The light cycle's speed depends on the frame rate

CShell.dll 0x100a7390 (CTronPlayerMgr slot 0x64, TRON/TronPlayerMgr.cpp;
not yet byte-identical, from the retail code). After the frame's move the
code puts the cycle on the grid (CLightCycleMgr::SnapToGrid, 0x100a7462),
which rounds the position to whole units. So each frame moves a whole
number of units: at 840 units a second, frames of 16 or 17 ms move 13 or
14 units (825 a second), frames of 5 or 6 ms 4 or 5 (853 a second), and at
1000 frames a second a cycle at 1400 units a second moves 1 unit a frame.
A frame's move of 0.1 units or less is not made (0x100a7401). A turn on a
frame that rounds to no move is lost.

Fixed in the modern build: the move is kept without rounding from frame
to frame (TronPlayerMgr.cpp, MODERN.md EN-45).

### Other players' light cycles are predicted less at a high frame rate

CShell.dll 0x10011410 (CClientLightCycleMgr::Update, TRON/
ClientLightCycleMgr.cpp; not yet byte-identical, from the retail code).
The client puts another player's cycle LCClientPredict (10; a host
LCHostPredict, 2) frames ahead of its last known position: the frame
time times that number (0x10011817). Then it moves the cycle LCInterpTime
(0.2) of the way there, each frame. At 60 frames a second the prediction
is 0.17 s, at 240 it is 0.04 s, and the cycle catches up faster.

Fixed in the modern build: a frame counts as a 60th of a second at least
for the prediction, and the share of the way is that of a 60th of a
second (ClientLightCycleMgr.cpp, MODERN.md EN-45).

### The FOV is set again each frame

CShell.dll 0x100e57af..0x100e58b3 (CGameClientShell::UpdateDebugInfo,
ClientShellShared/GameClientShell.cpp; not yet byte-identical, from the
retail code). The code gets the camera's field of view (radians) and
compares it with the console variables FovX and FovY (degrees, 0x100e57e8,
0x100e582e). They are never equal, so each frame it sets the camera's
FOV again from the variables. Build 51 and NOLF2 have the same line.

Fixed in the modern build: the comparison is with the variables in
radians (GameClientShell.cpp, MODERN.md GAME-34).

### The ClientFX DLL's copy in the temporary directory is left at each start

CShell.dll 0x101816c0 (CClientFXDB::UnloadFxDll), from 0x101811e0
(~CClientFXDB, ClientShellShared/ClientFXDB.cpp). The client copies
ClientFx.fxd from the archives to the temporary directory (cfx*.tmp) and
loads that copy. It unloads it only from the destructor of the static
CClientFXDB, while CShell.dll itself is unloaded. There the loader
defers the FreeLibrary, so UnloadFxDll still finds the module and does
not delete the file. Under Wine one copy (220 KB) is left for each start.

Fixed in the modern build: CGameClientShell::OnEngineTerm unloads the DLL
and its effects before CShell.dll is unloaded (GameClientShell.cpp,
InterfaceMgr.cpp, MODERN.md GAME-35).

### A disc tournament's time-out uses a deleted match

Object.lto 0x1004a67c (CServerDiscMatchMgr's update, TRON/
ServerDiscMatchMgr.cpp; not yet byte-identical, from the retail code).
When the tournament times out, the loop ends and deletes each match. It
reads the next node once, before the loop (0x1004a67c), and does not
read it again (0x1004a6ad). With two matches or more, the second is ended
and deleted, and then the loop uses its freed node again.

Fixed in the modern build: the next node is read in each pass
(ServerDiscMatchMgr.cpp, MODERN.md GAME-33).

### A push lasts the same time but pushes more at a high frame rate

CShell.dll 0x100f8040 (CMoveMgr::UpdatePushers, ClientShellShared/
CMoveMgr.cpp). Each frame, while a pusher lasts (an explosion, an impact),
the code adds its full push to the player's velocity. The pusher lasts a
time, so at 120 frames a second the player gets two times the push of 60.

Fixed in the modern build: the push is the amount of a 60th of a second
(CMoveMgr.cpp, MODERN.md EN-45).

### A slow move of the player is not made at a high frame rate

CShell.dll 0x100f8980 (CMoveMgr::MoveLocalSolidObject, ClientShellShared/
CMoveMgr.cpp). The code does not move the player when the frame's move is
0.1 units or less. That is 6 units a second at 60 frames a second, but 100
units a second at 1000: a slow walk stops. The test of a blocked fall
(0.01 units) has the same fault.

Fixed in the modern build: the limits are those of a 60th of a second
(CMoveMgr.cpp, MODERN.md EN-45).

### The player floats lower in water at a high frame rate

CShell.dll 0x100f5be0 (CMoveMgr::UpdateInLiquid, ClientShellShared/
CMoveMgr.cpp). When the player swims up and the head comes out of the
water, the code stops the rise on the next frame. At a higher frame rate
the rise stops sooner, so the player floats lower and a jump out of the
water starts lower.

Fixed in the modern build: the rise continues for two 60ths of a second
(CMoveMgr.cpp, MODERN.md EN-45). Tron 2.0's levels that the tests reach
have no water: not measured.

### Bouncing debris stops at a higher speed at a high frame rate

CShell.dll 0x10183c60 (BounceMovingObject, ClientShellShared/
client_physics.cpp). An object that bounces on the ground stops when its
speed squared is less than 15*15 divided by the frame time. The shorter
the frame, the faster the speed at which it stops: 116 units a second at
60 frames a second, 260 at 300.

Fixed in the modern build: the frame time of 60 frames a second
(client_physics.cpp, MODERN.md EN-45).

### A fire makes more sparks at a high frame rate

CShell.dll 0x101a43c0 (CFireFX::Update, ClientShellShared/FireFX.cpp). The
fire makes a spark with one chance in ten each frame.

Fixed in the modern build: one chance in ten each 60th of a second
(FireFX.cpp, MODERN.md EN-45).

### A slow particle or line system does not emit at a high frame rate

CShell.dll 0x101b7400 (CParticleSystemFX::Update, ClientShellShared/
ParticleSystemFX.cpp) and 0x101b60e0 (CLineSystemFX::Update,
LineSystemFX.cpp). Each update adds floor(rate * time since the last
update) particles (lines), and starts its clock again. When a frame is
shorter than the time of one particle, no particle is ever added.

Fixed in the modern build: an update that adds none keeps its time
(ParticleSystemFX.cpp, LineSystemFX.cpp, MODERN.md EN-45).
### The launcher's -GOSERVER starts a server that does not exist

Tron.exe 0x407460 (CLauncherApp::InitInstance, Launcher/Launcher.cpp) loads
the name of the dedicated server from IDS_SERVEREXE, "TRON2Svr.EXE" (the
string is in the retail image). CLauncherDlg::LaunchGame (0x4096a0) starts
it with WinExec for ACTION_STANDALONESERVER, which only the -GOSERVER
command line argument asks for (the launcher has no server button). The
dedicated server of the game is TRONSrv.exe, and there is no
TRON2Svr.EXE. Thus "tron.exe -goserver" shows the IDS_CANTLAUNCHSERVER
error and starts nothing.

Fixed in the modern build: Launcher/Launcher.cpp (InitInstance), MODERN.md
APPS-2.

### The game keeps its settings under HKEY_LOCAL_MACHINE, opened for writing

CShell.dll and Object.lto 0x100c4600 / 0x10075480 (CRegMgr::CreateKey,
libs/RegMgr/regmgr.cpp, from CRegMgr::Init 0x100c4360 / 0x10075240, which
CVersionMgr calls with HKEY_LOCAL_MACHINE) open each key with
RegCreateKeyEx and KEY_READ | KEY_WRITE, also to read a value. Tron.exe
and TRONSrv.exe 0x40c8d0 / 0x40fb40 (CRegMgr32::OpenKey,
libs/RegMgr32/regmgr32.cpp) open HKEY_LOCAL_MACHINE\SOFTWARE\Monolith
Productions\TRON 2.0\1.0 with KEY_READ | KEY_WRITE, also only to read. A
user who is not an administrator cannot open the machine's keys for
writing: then the game does not read or keep the profile name (the
launcher asks for the detail setting at each start), and the launcher
does not read or keep its options. It works only through the registry
virtualization of Windows Vista and later, which writes the values to a
place of the user's own.

Fixed in the modern build: libs/RegMgr/regmgr.cpp, libs/RegMgr32/
regmgr32.cpp, MODERN.md APPS-3 (the values are in a file beside the
executable; the registry is not used).

### Stop plays the music again when the music is paused

Lithtech.exe 0x43efa0 (CLTDirectMusicMgr::Stop, kernel/src/sys/win/
ltdirectmusic_impl.cpp) stops the performance, then calls UnPause
(0x43f350) until the pause count is zero. UnPause plays the segment that
Pause kept, from the time where it paused. Thus Stop while the music is
paused starts the music again, and it plays until the end of that segment
(Stop clears the queue, so nothing comes after it). Tron 2.0's game DLLs
stop the music with Stop, and do not pause it, so the game does not show
it.

Not changed in the DirectMusic path. The modern build's own music player
(MODERN.md X-3) does not have it: its Stop clears the pause.

### A change of intensity in the music queue that fails is tried again without end

Lithtech.exe 0x43daa0 (CLTDirectMusicMgr::HandleSegmentNotification_
SegAlmostEnd, kernel/src/sys/win/ltdirectmusic_impl.cpp): for a
LTDMCommandChangeIntensity command, when ChangeIntensity does not return
LT_OK (an intensity that is not in the control file), the loop does not go
to the next command and tries the same command again, without end, in
the notification thread. Each control file of the game gives valid next
intensities, so the game does not show it.

Not changed in the DirectMusic path. The modern build's own music player
(MODERN.md X-3) goes to the next command.

### A cloned tracking node that is not made is not reported

CShell.dll 0x101c9763 and Object.lto 0x101e2203 (CTrackedNodeContext::_Init,
Shared/TrackedNodeContext.cpp). After the code makes a cloned tracking node,
it tests the node that it clones, not the clone. When the clone is not made,
the code does not write the "Failed to create tracking node" message. When
the clone is made but the first node is not, the code writes the message
for the clone's name. The code must test the clone.

Fixed in the modern build: Shared/TrackedNodeContext.cpp (both DLLs), MODERN.md GAME-134.

### The HUD does not delete its version item

CShell.dll 0x1009b240 (CTronHUDMgr::~CTronHUDMgr, TRON/TronHUDMgr.cpp). The
constructor makes fourteen HUD items with new. The destructor removes and
deletes thirteen of them. It does not remove or delete m_pVersion (the
CHUDVersion item). The memory of the item is not released when the game
stops. The player does not see an effect.

Fixed in the modern build: TRON/TronHUDMgr.cpp, MODERN.md GAME-135.

### The energy message of a core dump falls through into the HUD text message

CShell.dll 0x1009dec8 (CTronInterfaceMgr::OnMessage, TRON/
TronInterfaceMgr.cpp). The MID_TRON_ENERGY_GAINED case (164) reads one
byte and shows string 3271 in the chat. It has no break, thus the code of
the MID_TRON_HUD_TEXT case (127, at 0x1009df00) runs after it. That code
reads two values of 32 bits that the message does not have, and it plays
the "msgfadeout" sound. The server (Object.lto CoreDump, TRON/
CoreDump.cpp) writes only the byte. In retail, the server does not send
the message, because its energy test is always false (see CoreDump.cpp).
Thus there is no visible effect.

Fixed in the modern build: TRON/TronInterfaceMgr.cpp, MODERN.md GAME-136.

### The mouse look counts a mouse event more than one time, and at random

Lithtech.exe 0x432d70 (input_ReadInput, src/Engine/runtime/kernel/src/
sys/win/input.cpp). The engine reads one buffered DirectInput event of the
system mouse at a time ("This fixes the USB mouse lockups") and runs all
the triggers after each event. Each axis gives the mean of its last three
samples (InputRate), and it keeps its last value until InputRate / 10
milliseconds pass with no new event. Thus one mouse event turns the view in
more than one pass and in more than one frame, and an axis with no new
event (the x axis when only y moves) adds its old motion again. The turn
for one count of mouse motion changes with the frame rate, with the report
rate of the mouse and with how the motion is divided into events. Also,
the device buffer holds 64 events, which a fast mouse fills ("Input buffer
overflow") at a low frame rate. Measured (build/playtest-mouselook.txt,
build/yawshift.py, the legacy build under Wine): 60 counts to the right in
one event turned the view 28 degrees; 60 counts in two events turned it 7
degrees to the left; 60 counts in five events, 8 degrees to the right.
nolf2src's `RETAIL_BUGS.md` has the same bug in NOLF2's engine.

Fixed in the modern build: input.cpp, MODERN.md EN-23 (each look axis
adds the motion of the frame and gives it one time; a buffer of 1024
events) and EN-24 (the motion from raw input).

### Configure controls binds a gamepad's button by its instance number

CShell.dll 0x10057300 (CScreenConfigure::Bind, src/Game/ClientShellDLL/
TRON/ScreenConfigure.cpp). The test `nControlCode && (deviceType !=
DEVICETYPE_JOYSTICK || deviceType == DEVICETYPE_GAMEPAD)` is true for a
gamepad. Thus a gamepad's button is bound as "##<instance>". The engine
takes that name for the first object with that instance number, which is
an axis. A joystick's control is bound by its name, as the code intends
for both. The test should be false for the two device types.

Fixed in the modern build: ScreenConfigure.cpp, MODERN.md CL-24.

### The console's Shift and Ctrl keys are all Ctrl

Lithtech.exe 0x49a800 (CConsole::OnKeyPress) and 0x498fc0
(CConCommandBox::OnKeyPress), src/Engine/runtime/client/src/sys/win/
winconsole_impl.cpp. Both call IsKeyDown to test Ctrl or Shift, but
IsKeyDown tests VK_CONTROL whatever key it gets. Thus Shift+Tab does what
Tab does (the next name, not the one before). The final build never shows
the console, so a player does not see it.

Fixed in the modern build: winconsole_impl.cpp (IsKeyDown), MODERN.md
EN-70.

## NOLF2's and Contract J.A.C.K.'s bugs Tron 2.0 does not have

- NOLF2's CRelationUser::RemoveRelation deletes the relation momento only
  when it has expired, but always removes it from the list: an unexpired
  momento leaks. Tron's deletes the momento in all conditions (Object.lto
  0x10168180).
- Contract J.A.C.K.'s DrawPrim(LT_POLYGT4*) colours a batch of screen-space
  quads with each quad's first vertex colour, and its DrawPrim functions
  check only the last draw's result: Tron's are Build 69's (each vertex's
  own colour, every result checked; Lithtech.exe 0x41f140).
- Contract J.A.C.K.'s CD3DDrawPrim::SetCamera writes the camera's size into
  the saved viewport and leaves the new viewport's size unset: Tron's is
  Build 69's (0x41d8e0; its own bug is above).
- NOLF2's dedicated server stops with `IDS_NETERR_CORRUPTCONFIG` when
  `ServerGameOptions::LoadFromBute` fails. Tron's `CServerDlg::DoHost`
  (TRONSrv.exe 0x40b185) does not examine the result, but Tron's
  LoadFromBute returns false only when it has corrected the options (a
  backslash in the server name, more than 8 light cycle players). A
  missing key keeps its default. Thus the server hosts the corrected
  options, and that is correct.
- The loop sounds of the archive bin, the I/O node and the optimizer
  (`PlayLoopSound`, Object.lto 0x10011110, 0x10024bc0, 0x10032420) are
  played without PLAYSOUND_LOOP, but this is not a bug. The sound comes
  from a model key (`BUTE_SOUND_KEY_LOOPING <sound set>`), and
  `PlaySoundFromObject` adds the flags of the sound set. The retail sound
  sets that loop (`finderidle`, `finderscan`, `finderseek`, `finderalarm`)
  have `LoopSound = 1`. The optimizer's `opti_graze` does not, and its
  model keys start it again in each animation. Thus the sounds play as
  the data tells them.
