#!/usr/bin/env bash
# The game directory's tron20cmds.txt, as a fresh build writes it:
#
#   build/tron20cmds.sh
#
# The rez stack the engine is started with. The retail Tron.exe launcher
# writes its own launchcmds.txt and starts "lithtech.exe -cmdfile
# launchcmds.txt" (src/Game/Launcher/LauncherDlg.cpp): Game.rez, Game2.rez,
# GameDLL.rez, Sound.rez, then custom/, then the updates' archives from the
# registry (Update 2's UpdateCommandLine: -rez gamep3.rez -rez gamep4.rez).
# Later archives override earlier ones; the engine opens Engine.REZ itself.
# Here custom/ -- our game DLLs -- is last, so that nothing overrides them:
# Gamep3.rez and Gamep4.rez still name the retail game DLLs (their entries
# are zeroed in our copies), and as the retail launcher puts them after
# custom/, those zeroed entries would hide ours. build/all.sh writes this
# file into the game directory; ext/tools/playtest.sh (build/playtest.sh)
# starts the engine with it.
printf -- '-rez Game.rez -rez Game2.rez -rez GameDLL.rez -rez Sound.rez -rez Gamep3.rez -rez Gamep4.rez -rez custom\r\n'
