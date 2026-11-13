# Bugs in the retail game

Bugs in *Tron 2.0* (v1.030) as it shipped, found while rebuilding it. Each
is a mistake in the code that a source change would fix. This is a living
list: add a bug when it is found, in the format below, whether or not it is
fixed yet.

The legacy build reproduces every one of them, on purpose: compiling to the
retail machine code means compiling its mistakes too. They are fixed in the
**modern build** (`make modern`, the `TRON20_MODERN` define; `MODERN.md`):
each fix goes behind `#ifdef TRON20_MODERN`, with the retail code kept in
the `#else`, so the legacy build is unchanged and still matches
(`make progress`).

An entry that is fixed says so under its heading, with the file the fix is
in and its `MODERN.md` id. One that is not says why, or which part of the
tree it waits for.

Every binary here is a reconstruction (`docs/PLAN.md`), so an entry is
listed only where our function is byte-identical to the retail one at the
address it names (`tools/exematch.py`; for a string, the string is in the
retail image too): it describes what the retail code does, not a guess.
Addresses are as in the retail images (`build/reference.sh` extracts them
into `out/reference/`).

Many of NOLF2's bugs (nolf2src's `docs/RETAIL_BUGS.md`) and Contract J.A.C.K.'s
(cjacksrc's) are likely to be here too, since Tron 2.0 was built from the
same code: check each against the retail binary before listing it. Those
Tron 2.0 does not have go at the end, so nobody looks again.

**Fixed so far** (in the modern build): none yet.

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

Not fixed yet: the modern build should make it a uint16, as NOLF2's.

### An I/O node's MoveToFloor property does nothing

Object.lto, IONode (TRON/IONode.cpp; ReadProp at 0x10022f60, its update at
0x10022c1a). The class lists a MoveToFloor property (default false), and
its first update moves it to the floor when its m_bMoveToFloor is set, but
ReadProp never reads the property: only the constructor (false) and Load
set the member, so no I/O node is ever moved to the floor.

Not fixed yet: the modern build should read MoveToFloor in ReadProp.

### An I/O node leaks its looping sound's name

Object.lto, IONode (TRON/IONode.cpp; destructor at 0x10022a60). The name of
the looping sound a model key starts (m_hstrLoopSound, created in
HandleModelString) is freed when the sound is killed by a key, but not in
the destructor: a node removed while its loop plays leaks the string.

Not fixed yet: the modern build should free it in the destructor.

### A lure's death FX is only played when it has no name (and crashes then)

CShell.dll 0x101288d0 (CPlayerMgr::HandleMsgPlayerStateChange,
ClientShellShared/PlayerMgr.cpp), as NOLF2's. When the player dies on a
lure (PPM_LURE), the test is `if( !pszDeathFX && pszDeathFX[0] != '\0' )`:
the FX is never created for a named death FX, and a missing one is read
through a NULL pointer. It should be `pszDeathFX && pszDeathFX[0]`. Tron's
levels may not use lures, so it may never show.

Not fixed yet.

### A posted string that takes more than one send is sent again from its start

CShell.dll 0x100202c0 (ghiPostStringStateDoPosting, the GameSpy SDK's
ghttp/ghttpPost.c as Tron links it, 2003). A string field of a multipart
HTTP post is sent with `ghiDoSend(connection, string, len - pos)`: the
length left, but from the string's start rather than `string + pos`, so a
partial send is followed by the beginning of the string again and the post
is corrupt. Later SDKs send through `ghiTrySendThenBuffer`. Only a post
that fills the socket's buffer shows it.

Not fixed yet (the modern build replaces GameSpy's services anyway).

### Thorne reports a valid summon command as invalid

Object.lto 0x10058497 (CThorne::ReadProp, TRON/Thorne.cpp). The
`SummonCmd` property is checked with `if (g_pCmdMgr->IsValidCmd(cmd))
ObjectCPrint(..., "Invalid summon command %s", ...)`: the test is
inverted, so a valid command prints the error (in the editor/console only)
and an invalid one passes silently. It should be `!IsValidCmd(...)`.

Not fixed yet.

### Thorne builds a special-fx message he never sends

Object.lto 0x10058dc1 (CThorne::InitialUpdate). After setting his SFX
message, Thorne writes a second MID_SFX_MESSAGE (his id, object and create
struct) and drops it without sending it. Harmless, wasted work.

Not fixed yet.

### A turret without a PropType reads an uninitialised prop type

Object.lto 0x1006a330 (TronTurret::ReadProp, TRON/TronTurret.cpp). The
PROPTYPE pointer is only set when the PropType property is found and not
empty, but it is tested (and used to set up the model) at the end either
way: a TronTurret without a PropType uses whatever was on the stack. It
should start as NULL.

Not fixed yet.

### A turret without a ViewFOV takes its default cosine for an angle

Object.lto 0x1006a330 (TronTurret::ReadProp). The view FOV member starts as
1.57 (a half angle in radians) and, with no ViewFOV property, is turned
into cos(1.57 * PI/360) as if it were degrees: an almost zero-wide view
cone. The editor always writes the property (default 180), so it may never
show.

Not fixed yet.

### A parented turret reads its parent's transform and drops it

Object.lto 0x1006bf20 (TronTurret's first update). With a parent, the
parent's position and rotation are read into locals and never used. The
per-frame update reads the parent again, so it is only wasted work.

Not fixed yet.

### The energy transfer meter draws part of its fill twice

CShell.dll 0x100294c0 (CHUDEnergyTransfer::Update, TRON/HUDEnergyTransfer.cpp).
Past half way the fill is three triangles fanned from the meter's center:
-90..-45, -45..0, and then -45..the transfer's angle instead of 0..the
angle. The third covers the second again, so with the meter's alpha blend
the -45..0 wedge draws brighter than the rest once the fill passes 0.

Not fixed yet: the modern build should start the third triangle at 0.

### A core dump never tells the player how much energy it gave

Object.lto 0x10019200 (CoreDump::HandleActivate, TRON/CoreDump.cpp). It
adds its energy to the player's (clamped to his maximum), then measures
what he gained as the new energy less the sum it clamped, which already
holds the core dump's energy: never more than 0, so the "energy gained"
message (164) it sends when that is over 1 is never sent. It should
subtract the player's energy before the core dump's was added.

Not fixed yet.

### Taking a custom item from a core dump sends an uninitialized message

Object.lto 0x10019da0 (CoreDump::HandleChoice, TRON/CoreDump.cpp). The
trigger message sent to the player is only built for subroutines,
procedurals, primitives, permission sets, emails and help files; for any
other item type (Custom) the uninitialized stack buffer is sent as it is.
Core dumps only get their items from bodies, which may never hold custom
items.

Not fixed yet.

### A patch routine gives the amount asked, not what it has left

Object.lto, PatchRoutine::ObjectMessageFn (TRON/PatchRoutine.cpp; retail
0x10032d29, 0x10032d95). When the player's client drains a health or
energy patch routine, the amount asked is clamped to what the routine has
left only to test the player's maximum; what is given (and taken from the
routine) is the unclamped amount. A request larger than what is left gives
the player more than the routine had and leaves its count negative (it is
then neither > 0 nor == 0: the routine is never removed).

Not fixed yet: the modern build should give the clamped amount.

### A spawned optimizer reads none of its properties

Object.lto 0x10030760 (Optimizer::EngineMessageFn, TRON/Optimizer.cpp; our
code matches retail's, but for the switch's jump table, which the scorer
reads as instructions). MID_PRECREATE calls ReadProp only for
PRECREATE_WORLDFILE, not for PRECREATE_STRINGPROP as IONode and most
classes do: an optimizer made by a spawner gets no model, permission sets,
keyframer or commands. Tron's levels may only place them in the world, so
it may never show.

Not fixed yet: the modern build should read the properties for both.

### A light cycle dialog's bad priority is never reported

Object.lto 0x10025b30 (CLightCycleButeMgr::Init, Shared/TRON/
LightCycleButeMgr.cpp; CShell.dll has its own copy). Reading a CycleDialog
<Type>Priority, the check is `if (fPriority < 0.0f && fPriority > 1.0f)`,
which is never true: a priority out of [0, 1] goes unreported ("ERROR:
Priority value %f < 0 or %f > 1 for %s") and is kept. It should be `||`.

Not fixed yet: the modern build should test with `||` (and clamp).

### A light cycle shade's Super color is not cleared

Object.lto 0x10026858 and 0x1002802a (LIGHTCYCLESHADE's constructor, inlined
in CLightCycleButeMgr::Init and GetShade, Shared/TRON/LightCycleButeMgr.h).
It clears the first two characters of the Regular color name instead of the
first of each color, so the Super color of a new shade is garbage until Init
reads it. Init always reads both, and GetShade's default shade is static
(zeroed), so it should never show.

Not fixed yet: harmless; the modern build may clear szCycleColors[1][0].

### A shield hit drops a trail's last point by erasing end()

CShell.dll 0x10100ce0 (CLightCycleMgr::HandleShieldCollision,
Shared/TRON/LightCycleMgr.cpp; the same code is in Object.lto's copy). When
the cut-off part of the hit trail cannot be shortened, the code calls
`collTrailPoints.erase(collTrailPoints.end())`: erasing end() is undefined.
With STLport 4.0 the copy loop runs no times and the end moves back one
point, so it works as a pop_back.

Not fixed yet: the modern build should call pop_back().

### Updating a listed server by IP adds it to the list again

CShell.dll 0x10052f50 (ServerBrowserAuxUpdateIPA, GameSpy SDK
serverbrowsing/sb_serverbrowsing.c, 2003). Updating a server directly
(not via the master), the browser looks the server up by IP and port and,
when it is already in the list, re-queries that entry, but then appends the
server to the list in both cases: an already-listed server appears twice and
the browser callback reports it added again. GameSpy fixed it in
serverbrowsing 2.02.11 (2004-05-24).

Not fixed yet: the modern build should append only a new server.

### The query engine marks timed-out servers in the wrong field

CShell.dll 0x10052220 (TimeoutOldQueries, GameSpy SDK
serverbrowsing/sb_queryengine.c). A server whose query timed out gets
STATE_QUERYFAILED set and its pending-query states cleared in `flags` (the
server's address flags) instead of `state`: the server keeps its pending
state and its flags gain bits that mean "has ICMP address"/"private port".
The 2007 SDK still has it.

Not fixed yet: the modern build should set them in `state`.

### A LAN browse never reports that its list is complete

CShell.dll 0x10053e30 (ProcessLanData, GameSpy SDK
serverbrowsing/sb_serverlist.c, 2003). When the LAN search time ends, the
list closes its socket and goes disconnected without the
slc_initiallistcomplete callback, so an asynchronous LAN browse is never
told it finished. GameSpy fixed it in serverbrowsing 2.00.36 (2003-11-10).

Not fixed yet: the modern build should make the callback.

### The light cycle dialog plays a line it has already popped

Object.lto 0x100509af (CServerLightCycleMgr::Update,
ObjectDLL/TRON/ServerLightCycleMgr.cpp). It keeps a pointer to the top of
the rider dialog priority_queue, pops it, then reads and plays through the
pointer. pop() moves the queue's last line into the top slot, so the line
checked and played is not the one that had the highest priority (unless the
queue held one line).

Not fixed yet: the modern build should copy the top entry before pop().

### The energy claw's line-of-sight test always passes

Object.lto 0x1003eb90 (CProjectileEnergyClaw::CanSee, ObjectDLL/TRON/
ProjectileEnergyClaw.cpp). It gets the position of the first object for both
ends of the segment it tests, so the segment is empty, nothing is hit, and
the claw "sees" any AI in range: it seeks and chases through walls.

Not fixed yet: the modern build should take the second end from hTo.

### A CD key server reply with no second backslash crashes the server

Object.lto 0x10020690 (cdkey_process_buf, Libs/GameSpy/GameSpySDK/gcdkey/
gcdkeys.c, GameSpy's 2003 CD key SDK). A packet from the key server (or
anyone spoofing it on the query socket) that starts with `\` is searched
for the next `\` with strchr, and `pos - buf` is compared with 32 without
checking pos for NULL: with no second backslash the difference is negative,
passes the test, and strncpy is handed a huge length. GameSpy's later SDKs
test `pos &&` first.

Not fixed yet: the modern build should test pos for NULL, as the 2007 SDK.

### The query SDK's host lookup can hand back an uninitialized hostent

Object.lto 0x10042450 (get_sockaddrin, Libs/GameSpy/GameSpySDK/qr2/qr2.c,
GameSpy's 2003 Query & Reporting 2 SDK). `hent` is only set when the host
needs a DNS lookup, but is stored through `savehent` whenever that is not
NULL: for a numeric address the caller would get whatever the stack slot
held (the compiler reuses the `saddr` argument's slot, so it is `saddr`).
The SDK's only caller (qr2_init_socket) passes NULL, so it never shows.

Not fixed yet: harmless; the modern build may initialize it to NULL, as the
2007 SDK does.

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

Not fixed yet (the engine is not rebuilt): the modern fix must also turn
the shipped scripts' Sin and Cos round (or keep the swap for them), or
every effect built on them moves a quarter turn.

### Three render styles use a colour operation that does not exist

RS/FULLBRIGHTON_ALPHA_SPECULAR.ltb (Game2.rez), PROPS/RS/ARCHIVECUBE.ltb
and CHARS/RS/INNER_GLOW.ltb (Game.rez): the second texture stage of their
only pass has colour op 19 (read from the files: the first stage has 3,
MODULATE; stages 3 and 4 are 0), but ltrenderstyle.h has 16 colour ops
(RENDERSTYLE_COLOROP_TYPE_COUNT). Build 69's renderer turns an op into
Direct3D's through CRenderStyleLookupTables::m_nColorOp[16], with
m_nAlphaOp right after it, so 19 reads m_nAlphaOp[3]: by NOLF2's tables
D3DTOP_MODULATE. That much is inferred: Tron 2.0's renderer is not
reconstructed, so its tables are not checked.

Not fixed yet (the engine is not rebuilt): the modern build should give
these stages the op they mean (MODULATE, as they now get) in the files, or
clamp an unknown op.

### A lip-sync file's long node name overruns a stack buffer

CShell.dll 0x10185b30 (CNodeController::HandleNodeControlLipSync,
ClientShellShared/NodeController.cpp; our function is near, not identical
yet, so read from the retail code). For each node the .lip file names, it
reads a 4-byte length (0x10185d19) and then that many bytes into the
256-byte `szBone` on the stack (0x10185d2f), and writes the terminator at
`szBone[length]` (0x10185d40), with no check: a name of 256 bytes or more
overwrites the stack. No shipped name is longer than 17 bytes, so only a
broken or hostile file shows it.

Not fixed yet: the modern build should refuse a length over 255.

### An empty sprite file divides by zero

Lithtech.exe 0x484e7e..0x484ef9 (spr_Create, the engine's sprite.cpp; not
reconstructed yet, so read from the retail code). It reads the frame
count, frame rate and three more dwords, then sets the animation's length
to `(1000 / rate) * frames` (`div` at 0x484ef9) before it looks at the
stream's error status. The rez stream zeroes what it cannot read (Build
69's RezFileStream::Read), so an empty file gives a rate of 0 and a divide
by zero. FX/SPR/FAKE.SPR, FX/SPR/JETREZIN.SPR and
INTERFACE/MENU/SPR/LOADING.SPR in Game.rez are 0 bytes; nothing loads
them, so it never shows.

Not fixed yet (the engine is not rebuilt): the modern build should check
the stream, or a rate of 0, before dividing.
### The pings HUD sets up only four of its eight pings

CShell.dll 0x100311e0 (CHUDPings::Init, ClientShellDLL/TRON/HUDPings.cpp).
The item holds eight pings, but Init sets the UVs, the color and the name
of the first four only, and Update (0x10031490) fills four at most; the
other four are never shown. UpdateLayout (0x10031960) also reads the font
size twice.

Not fixed yet: harmless (four pings show); the modern build may use all
eight.
### A light cycle with an unknown color dereferences NULL

Object.lto 0x10025503 (LightCycle::SetModel, ObjectDLL/TRON/LightCycle.cpp).
It reads the color's cycle type (and does nothing with it) before checking
that the CycleColor was found, so a cycle whose CycleColor names no color
reads through NULL before it can print its "Couldn't find Light Cycle
Color" error.

Not fixed yet: the modern build should drop the unused lookup, or make it
after the check.

### The light cycle missile's stand-in cyclist is left uninitialized

Object.lto 0x10029fd0 (LIGHT_CYCLIST's (float, float, bool) constructor,
Shared/TRON/LightCycleMgr.h; used for LightCycleMissile's static
cyclist, 0x10358ca0). It "calls" the default constructor with
`LIGHT_CYCLIST();`, which builds and destroys a temporary, so only the three
fields it sets afterwards are initialized. The rest of the static is zero
only because it is a static (its direction, flags and power-up are 0, not
the defaults).

Not fixed yet: the modern build should initialize the fields in the
constructor itself.

### A light cycle power-up's AllowInCustom cannot be turned off

Object.lto 0x1002b502 (LightCyclePowerup::ReadProp, ObjectDLL/TRON/
LightCyclePowerup.cpp). The property is only copied when it is true, and the
default is true, so a power-up set not to appear in custom light cycle games
appears anyway.

Not fixed yet: the modern build should copy the property's value always.

### An AI light cycle's Behavior property is ignored

Object.lto 0x1000259e (AILightCycle::ReadProp, ObjectDLL/TRON/
AILightCycle.cpp). The Behavior property ("aggressive", "wander", "avoid")
is read and compared, but the default behavior is then set to wander
whatever it said; "avoid" maps to wander as well. Every AI light cycle
starts wandering.

Not fixed yet: the modern build should keep the value read (aggressive 1,
wander 0, avoid 2).

### An AI light cycle only looks at the first trail of each cyclist

Object.lto 0x10002e92, 0x100031d0 (AILightCycle::Probe and ProbeAll). The
loops over a cyclist's trails check `collTrails[0]` every time instead of
the trail of the loop, so the AI does not see older trails (those left by
a trail spike or a derez) in its forward and side probes. CheckPath
(0x100036b9) indexes the trails properly.

Not fixed yet: the modern build should check `collTrails[j]`.

### An AI light cycle's trigger messages read past a light cycle message

Object.lto 0x10002079 (AILightCycle::ObjectMessageFn). The
MID_LIGHT_CYCLE_INFO case falls through into MID_TRIGGER after
LCI_START_UPDATING, which then reads the rest of the message as a trigger
string pointer.

Not fixed yet: the modern build should break after that case.

### Loading object.lto leaks or mis-frees its class module on an error

server.dll 0x10001839 (cb_LoadModule, the engine's shared/src/classbind.cpp).
The module record (12 bytes) is allocated with `new` (LTMemAlloc) before
object.lto is bound; when the DLL cannot be found or has no ObjectDLLSetup
it is never freed, and on a version mismatch it is freed with the C
runtime's `free()`, which is not the allocator that made it. The game then
fails to start the world either way, so it only shows as a leak (or a heap
complaint in a debug runtime).

Not fixed yet: the modern build should allocate it after the binding
succeeds and release it with `delete`.

### A refused connection leaves the client waiting on a closed socket

server.dll 0x1003bfaa (CUDPDriver::ReallyJoinSession, the engine's
kernel/net/src/sys/win/udpdriver.cpp; Lithtech.exe has the same driver).
When the server accepts but the engine's NewConnectionNotify refuses the
connection, the new connection is deleted and its socket closed, and the
loop then goes on receiving on that closed socket until the connection
time-out (10 s) passes and LT_TIMEOUT is returned. It should stop at once
with an error (or not close the socket it keeps using).

Not fixed yet.

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

Not fixed yet.

### The dedicated server hosts a config it could not read

TRONSrv.exe 0x40b185 (CServerDlg::DoHost, src/Game/ServerApp/ServerDlg.cpp).
Once the config file has parsed, DoHost calls
`ServerGameOptions::LoadFromBute` and goes on whatever it returns; NOLF2's
returned `IDS_NETERR_CORRUPTCONFIG` ("The config file could not be read
properly") when it failed. A config with a bad or missing setting is hosted
with whatever the options held, and the error that exists for it is never
shown. (Tron's LoadFromBute returns false only when it had to correct the
options -- a backslash in the server name, more than 8 light cycle players
-- so the config is hosted, corrected, either way.)

Not fixed yet (the modern build's server work, `MODERN.md`).

### A saved tournament mode is never loaded back

CShell.dll 0x100ea210, TRONSrv.exe 0x408160 (ServerGameOptions::LoadFromBute,
src/Game/Shared/ProfileUtils.cpp). SaveToBute (CShell.dll 0x100e9ab0) writes
the disc tournament's "TournamentMode", but LoadFromBute reads every other
tournament key and not that one, so the mode a player saves in a config
comes back as the default every time the config is loaded, in the game's
host screens and on the dedicated server.

Not fixed yet.

### A config without the light cycle power-up keys turns every power-up off

CShell.dll 0x100ea210, TRONSrv.exe 0x408b22 (ServerGameOptions::LoadFromBute,
src/Game/Shared/ProfileUtils.cpp). Clear allows every power-up
(m_nLCPowerups = 0xFFFFFFFF), but LoadFromBute sets the mask to 0 before it
reads the nine "PowerupAllow*" keys and passes each bit as its key's
default: the bit it has just cleared. A key missing from the config (an
older or hand-written one) leaves its power-up disallowed instead of at
the default, allowed.

Not fixed yet.

### The packet parser clears four bytes of its message buffer

server.dll 0x10021a76 (ParsePacket, src/Engine/runtime/kernel/net/src/
netmgr.cpp). On the first packet it clears the buffer with
`memset(aMsgBuff, 0, sizeof(aMsgBuff))`, the size of the pointer rather than
of the buffer, then appends to it with LTStrCat; it only works because the
first byte is cleared. The same line is in Jupiter's later netmgr.cpp. Only
the ParseNet console variable's debug output uses it.

Not fixed yet.

### A polystring's drop shadow takes the text colour at one corner

Lithtech.exe 0x563e20 (CUIPolyString_Impl::ApplyColors, src/Engine/runtime/
ui/src/cuipolystring_impl.cpp). Each character's shadow polygon is coloured
`SetRGBA4(shadow, m_pColors[0], 0xff000000, 0xff000000, 0xff000000)`: the
first vertex (the top left) gets the text's own colour where the other three
are opaque black, so every shadowed glyph has a tinted corner fading into
it. The formatted polystring's tag colouring (0x569550) makes the shadow
black at the text's alpha, as was meant; probably `m_pColors[0] &
0xff000000` was meant here.

Not fixed yet.

### A colour tag without three numbers reads past the end of the text

Lithtech.exe 0x569830 (CUIFormattedPolyString_Impl::ParseColor, src/Engine/
runtime/ui/src/cuiformattedpolystring_impl.cpp). After "<color " the parser
skips to the next digit, reads a number, and does so three times, without
stopping at the string's end or at the tag's '>': a malformed tag such as
"<color red>" walks on through the terminator into whatever memory follows
the string.

Not fixed yet.

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

Fixed in the modern build: d3d_device.cpp and d3d_init.cpp, MODERN.md M-G45.

### A new system memory configuration leaks the old bad blocks

CShell.dll 0x100954b0 (CSubroutineMgr::SetSystemMemoryConfiguration,
src/Game/ClientShellDLL/TRON/SubroutineMgr.cpp). Before it lays out the new
bad blocks and base code, it erases the player's unusable subroutines (the
old bad blocks and base code, made by AddBadBlock/AddBaseCodeBlock with
debug_new) from its array without deleting them. Each new configuration
(each level's) leaks those PlayerSubroutines (0x2c bytes each).

Not fixed yet (the modern build should delete them as it erases them).
### A team's alternative name is read past its table for teams 4 to 7

CShell.dll 0x100fafc0 (CTronTeam::GetName, Shared/SharedScoring.cpp). A
team of more than one player takes its name from one of two string id
tables by its id, for ids up to 7; the first table (0x101ea528) has eight
ids, the second (0x101ea548, chosen by the third argument) only four, so
teams 4 to 7 read whatever follows it (code addresses) as a string id and
get an empty or wrong name.

Not fixed yet: the modern build should give the second table eight ids or
fall back to the first for ids past it.
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

Not fixed yet.

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

Not fixed yet.

### DrawPrim never uses its camera's viewport

Lithtech.exe 0x41d8e0 (CD3DDrawPrim::SetCamera, src/Engine/runtime/
render_b/src/sys/d3d/d3ddrawprim.cpp; Build 69's code). When the draw
primitive's camera rectangle differs from the current viewport, it sets
the current viewport again (only its depth range changes) instead of the
camera's rectangle, so primitives drawn through a camera that does not
cover the screen are not confined to it.

Fixed in the modern build: d3ddrawprim.cpp sets the camera's rectangle.

## NOLF2's and Contract J.A.C.K.'s bugs Tron 2.0 does not have

- Contract J.A.C.K.'s DrawPrim(LT_POLYGT4*) colours a batch of screen-space
  quads with each quad's first vertex colour, and its DrawPrim functions
  check only the last draw's result: Tron's are Build 69's (each vertex's
  own colour, every result checked; Lithtech.exe 0x41f140).
- Contract J.A.C.K.'s CD3DDrawPrim::SetCamera writes the camera's size into
  the saved viewport and leaves the new viewport's size unset: Tron's is
  Build 69's (0x41d8e0; its own bug is above).
