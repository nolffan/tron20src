#!/usr/bin/env bash
# The game directory's tron20cmds.txt, as a fresh build writes it:
#
#   build/tron20cmds.sh
#
# The rez stack the engine is started with. The retail Tron.exe launcher
# writes its own launchcmds.txt and starts "lithtech.exe -cmdfile
# launchcmds.txt" (src/Game/Launcher/LauncherDlg.cpp): Game.rez, Game2.rez,
# GameDLL.rez, Sound.rez, then custom/, then the updates' archives from the
# registry (Update 2's UpdateCommandLine0/1: -rez gamep3.rez -rez
# gamep4.rez). Later archives override earlier ones; the engine opens
# Engine.REZ itself. Update 2's data is tron20src.rez here, which
# build/all.sh packs from assets/ (assets/README.md), in the place of the
# update's two archives. custom/ -- our game DLLs -- is last, so that
# nothing overrides them (tron20src.rez holds no game DLL, and the zeroed
# ones of GameDLL.rez are renamed: build/rezhide.py). build/all.sh writes
# this file into the game directory; ext/tools/playtest.sh
# (build/playtest.sh) starts the engine with it.
printf -- '-rez Game.rez -rez Game2.rez -rez GameDLL.rez -rez Sound.rez -rez tron20src.rez -rez custom\r\n'
