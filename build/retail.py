# The retail game's layout, for ext/tools/retailextract.py (it reads this file
# from build/; see its header): Tron 2.0 (USA) with Update 2 (v1.030), as
# build/discs.py installs it from your discs and the update
# (out/retail/tron2-usa-1.030, build/retail.sh). The game directory takes
# the discs' files only; the update's are the reference of the checks.

# The archives of the retail install that the game directory takes, under
# their installed names: the discs' archives. GameDLL.rez holds the 1.010
# game DLLs (July 2003); our copy has every PE entry zeroed. Update 2's
# archives are not taken: its data ships with the release instead
# (assets/README.md). build/all.sh packs assets/rez/ -- what Gamep4.rez
# holds besides the four game DLLs; Gamep3.rez holds only the English
# CRES.dll -- into tron20src.rez, which loads where the update's setup put
# its archives (UpdateCommandLine0/1: -rez gamep3.rez -rez gamep4.rez).
# The retail install's Gamep3.rez and Gamep4.rez stay the source of the
# v1.030 game DLLs that the checks compare against (REFERENCE_GAME).
REZ = ["Engine.REZ", "Game.rez", "Game2.rez", "GameDLL.rez", "Sound.rez"]

DATA_DIRS = ["Movies"]
# The two files the game rewrites, kept as the retail copies, which
# build/all.sh starts a game directory's autoexec.cfg and
# Profiles/Default.prf from (ext/tools/playtest.sh reads both).
# Also the manual, which the launcher's Help opens (tron.chm). (The readme
# of Update 2 and the dedicated server is in assets/game/.)
RETAIL_COPIES = [("autoexec.cfg", "autoexec.cfg.retail"),
                 ("Profiles/Default.prf", "Default.prf.retail"),
                 ("TRON.chm", "TRON.chm")]

# Bink, and the Microsoft C, C++ and MFC 4.2 runtime setup installs.
# MSVCIRT.dll, which the game DLLs and the dedicated server import, is
# Windows' own: setup does not install it.
THIRD_PARTY_NAMES = {"binkw32.dll", "mfc42.dll", "mfc42u.dll", "msvcp60.dll", "msvcrt.dll"}

# The engine's binaries, ours to build. Unlike NOLF2 and Contract J.A.C.K.
# there is no ServerDir.dll or GameSpy.dll.
BUILT_ENGINE = ["Lithtech.exe", "server.dll", "SndDrv.dll", "ltmsg.dll"]

# The retail images the checks compare against: the engine, the launcher
# and the dedicated server, and the game DLLs v1.030 runs -- CRES from
# Gamep3.rez, the rest from Gamep4.rez (the last two in its rez stack).
REFERENCE = BUILT_ENGINE + ["Tron.exe", "TRONSrv.exe"]
REFERENCE_GAME = [("CSHELL.dll", "CShell.dll", "Gamep4.rez"),
                  ("OBJECT.lto", "Object.lto", "Gamep4.rez"),
                  ("CLIENTFX.fxd", "ClientFx.fxd", "Gamep4.rez"),
                  ("SRES.dll", "SRes.dll", "Gamep4.rez"),
                  ("CRES.dll", "CRes.dll", "Gamep3.rez")]

OPEN_RUNTIME = [
    ("binkw32.dll", "src/modern/binkw32 + ext/ffmpeg: our Bink player"),
]

# The modern build (openruntime) also gets Microsoft's DirectMusic: the nine
# DLLs of ext/toolchains/directmusic-5.3.1.904/ as ms_<name>.dll (THIRD-PARTY
# rows), and this registration-free COM manifest beside the engine, which
# points the engine's DirectMusic classes at them (docs/MODERN.md X-3,
# src/modern/directmusic/README.md). The legacy build gets neither.
DIRECTMUSIC_MANIFESTS = [("src/modern/directmusic/Lithtech.exe.manifest", "Lithtech.exe.manifest")]
