# The retail game's layout, for ext/tools/retailextract.py (it reads this file
# from build/; see its header): Tron 2.0 (USA) with Update 2 (v1.030), as
# ext/rez/ installs it (ext/rez/games/tron2-usa-1.030, build/retail.sh).

# The archives, under their installed names. GameDLL.rez holds the 1.010
# game DLLs (July 2003); Update 2 adds Gamep3.rez, the English CRES.dll,
# and Gamep4.rez, the other four (and data), which its setup puts last in
# the rez stack (UpdateCommandLine0/1: -rez gamep3.rez -rez gamep4.rez). Our
# copies of all three have every PE entry zeroed.
REZ = ["Engine.REZ", "Game.rez", "Game2.rez", "GameDLL.rez", "Sound.rez",
       "Gamep3.rez", "Gamep4.rez"]

DATA_DIRS = ["Movies"]
# Setup installs no default player profile (Profiles/ is empty).
RETAIL_COPIES = [("autoexec.cfg", "autoexec.cfg.retail")]

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
