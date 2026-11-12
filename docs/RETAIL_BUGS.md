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

## NOLF2's and Contract J.A.C.K.'s bugs Tron 2.0 does not have

(none checked yet)
