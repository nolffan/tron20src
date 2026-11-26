#!/usr/bin/env bash
# The control for the runtime tests: the retail game's own directory, with
# the retail game DLLs inside its rez files, and our engine (Lithtech.exe,
# server.dll, SndDrv.dll, ltmsg.dll) in place of retail's, whose SecuROM
# does not run here. Where a test does differently in
# out/tron20-legacy-release, the cause is our game code (CShell.dll,
# Object.lto, ClientFx.fxd, CRes.dll, SRes.dll). For diagnosis only: it is
# never a game directory that we ship.
#
#   build/control.sh [gamedir] [out]
#
# gamedir: the build whose engine is used (default out/tron20-legacy-release);
# out: the control directory (default out/tron20-control): symbolic links
# into out/retail/tron2-usa-1.030 (build/retail.sh; relative, so that they
# resolve in the build container too), except for the engine (copied), the
# command file (the retail rez stack, no custom/) and the shipped
# autoexec.cfg and Default.prf that build/playtest.sh starts from.
#   make test TEST_GAME=out/tron20-control
set -eu
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GAME="$REPO/${1:-out/tron20-legacy-release}"
OUT="$REPO/${2:-out/tron20-control}"
RETAIL="$(source "$REPO/build/retail.sh" && retail_root)"
for f in Lithtech.exe server.dll SndDrv.dll ltmsg.dll autoexec.cfg.retail Default.prf.retail; do
    [ -f "$GAME/$f" ] || { echo "control.sh: $GAME/$f missing -- build it first"; exit 2; }
done
[ -f "$RETAIL/Gamep4.rez" ] || { echo "control.sh: no retail game in $RETAIL (make retail)"; exit 2; }
rm -rf "$OUT"; mkdir -p "$OUT"
REL="$(python3 -c 'import os,sys; print(os.path.relpath(sys.argv[1], sys.argv[2]))' "$RETAIL" "$OUT")"
for f in "$RETAIL"/*; do
    b=$(basename "$f")
    case "$b" in Lithtech.exe|server.dll|SndDrv.dll|ltmsg.dll|autoexec.cfg|Profiles|Tron.exe|TRONSrv.exe|AutoRun*|EREG) continue ;; esac
    ln -s "$REL/$b" "$OUT/$b"
done
cp "$GAME/Lithtech.exe" "$GAME/server.dll" "$GAME/SndDrv.dll" "$GAME/ltmsg.dll" \
   "$GAME/autoexec.cfg.retail" "$GAME/Default.prf.retail" "$OUT/"
# the retail launcher's rez stack with Update 2's archives (as
# build/tron20cmds.sh's, which has tron20src.rez in their place)
printf -- '-rez Game.rez -rez Game2.rez -rez GameDLL.rez -rez Sound.rez -rez Gamep3.rez -rez Gamep4.rez\r\n' > "$OUT/tron20cmds.txt"
echo "control: ${2:-out/tron20-control} (retail's game DLLs, our engine from ${1:-out/tron20-legacy-release})"
