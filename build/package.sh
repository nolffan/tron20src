#!/usr/bin/env bash
# The game directory as one file, to copy to the machine that plays it:
#
#   build/package.sh out/tron20-<legacy|modern>-<release|debug>[-nodiscs] [<name>]
#
# writes two archives, each unpacking to <name>/ (by default the game
# directory's own name):
#
#   out/<name>-private.tar.gz   the whole directory, the retail game's data
#                               included: for your own machines, and anyone
#                               who develops on the build. Never give it to
#                               other people.
#   out/<name>-public.tar.gz    the same without anything that came from the
#                               retail game (MANIFEST.txt's `data` and
#                               `THIRD-PARTY` rows of tron2-usa-1.030): only
#                               what we build, the licence notes of the
#                               open-source code in it, and the install
#                               scripts linux-install.sh and
#                               windows-install.bat, which copy the rest out
#                               of the player's own disc images and Update 2
#                               (build/mkinstall.py, from build/retail-files.txt,
#                               build/retail-ways.txt and build/retail-ranges.txt:
#                               no disc image is needed to make it). This is
#                               the one to give to other people. Its SHA256SUMS
#                               lists every file of the archive and every file
#                               the scripts write (`sha256sum -c SHA256SUMS`
#                               checks an install).
#
# TRON20SRC_ARCHIVES (default "private public") says which to write; with
# "public" alone (the CI build, .github/workflows/ci.yml) the private one is
# not made.
#
# A game directory built without the retail game (NODISCS=1: it has
# NOT-PLAYABLE.txt, and it is out/<name>-nodiscs/) has no private archive:
# it is skipped, or an error if TRON20SRC_ARCHIVES asks for it. Its public
# archive is the same, byte for byte, as one built with the retail game
# (NOT-PLAYABLE.txt is left out of it).
#
# Then build/pubcheck.py checks the public archive: no file in it is a file
# of the retail game, nor holds one (by SHA-256: build/retail-files.txt,
# build/retail-ranges.txt and, if it is installed, every file of the
# retail game and every entry of its .rez archives), every file is one
# that we build or write, and no file names a home directory or a user.
#
# The archives are reproducible: the files are in name order, every time
# is TRON20SRC_EPOCH (default 2000-01-01 00:00 UTC), the owner is 0:0 with no
# user or group names, the modes are 0644 or 0755, and gzip stores no name
# or time. Links are stored as the files they point to. Each archive is a
# fresh install, whatever has been played in the directory: no saves (an
# empty Save/), no logs, no player profiles but the shipped default one,
# autoexec.cfg as the shipped one (autoexec.cfg.retail), tron20cmds.txt as
# the build writes it (build/tron20cmds.sh), and none of the per-machine
# state that the game and the tests create (tron20.ini, launchcmds.txt);
# the public one has neither autoexec.cfg nor the default profile, which
# its install scripts write.
#
# build/all.sh runs it for the release build (step 6) and build/variant.sh
# for each variant, once its data and symbols are in place. Each archive is
# written beside and renamed into place, so a failed run never leaves a
# truncated one.
set -eo pipefail
OUT="${1:?usage: build/package.sh <game directory> [<name>]}"
OUT="${OUT%/}"
DIR="$(basename "$OUT")"
NAME="${2:-$DIR}"
PARENT="$(dirname "$OUT")"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(dirname "$HERE")"
ARCHIVES=" ${TRON20SRC_ARCHIVES:-private public} "
EPOCH="${TRON20SRC_EPOCH:-946684800}"
NODISCS=
[ -f "$OUT/NOT-PLAYABLE.txt" ] && NODISCS=1

# The fresh files, staged beside the directory (.pkg-<NAME>/, which is
# <NAME>/ in the archives, as is <DIR>/; tar matches its excludes before it
# renames, so the stage's names never meet them).
SB=".pkg-$NAME"
S="$PARENT/$SB"
rm -rf "$S"
mkdir -p "$S"
trap 'rm -rf "$S"' EXIT
bash "$HERE/tron20cmds.sh" > "$S/tron20cmds.txt"
# The licence notes, and the licences of the open-source code that the
# binaries contain (docs/LICENSE_NOTES.md)
mkdir -p "$S/licenses"
cp "$REPO/docs/LICENSE_NOTES.md" "$S/LICENSE_NOTES.md"
cp "$REPO/src/Game/Libs/GameSpy/GameSpySDK/LICENSE.txt" "$S/licenses/GameSpy-SDK.txt"
if grep -q $'^EXT .*\tbinkw32\.dll$' "$OUT/MANIFEST.txt"; then
    cp "$REPO/ext/ffmpeg/COPYING.LGPLv2.1" "$S/licenses/FFmpeg-COPYING.LGPLv2.1"
    cp "$REPO/src/tron20src/mp3/minimp3/LICENSE" "$S/licenses/minimp3-LICENSE.txt"
fi
touch -d "@$EPOCH" "$S/licenses"

# tar's options for a reproducible archive
TAR=(--sort=name --format=gnu --mtime="@$EPOCH" --owner=0 --group=0 --numeric-owner
     --mode='a=rX,u+w' --hard-dereference -h)
NAMES=(--transform="s|^$SB/private/|$NAME/|" --transform="s|^$SB/|$NAME/|")
[ "$DIR" = "$NAME" ] || NAMES+=(--transform="s|^$DIR/|$NAME/|" --transform="s|^$DIR\$|$NAME|")
# what neither archive holds: the per-machine state that the game, the
# launcher and the tests write
EXCLUDE=(--exclude="$DIR/Save/*" --exclude="$DIR/Profiles/*" --exclude="$DIR/autoexec.cfg"
         --exclude="$DIR/tron20cmds.txt" --exclude="$DIR/NOT-PLAYABLE.txt" --exclude='*.log'
         --exclude="$DIR/tron20.ini" --exclude="$DIR/display.cfg" --exclude="$DIR/launchcmds.txt"
         --exclude="$DIR/run-cmds.txt" --exclude='*.part' --exclude="$DIR/SHA256SUMS"
         --exclude="$DIR/LICENSE_NOTES.md" --exclude="$DIR/licenses")
STAGED=("$SB/tron20cmds.txt" "$SB/LICENSE_NOTES.md" "$SB/licenses")

# tar <archive> <tar arguments>: a .tar.gz, written beside and renamed in
targz() {
    local dest="$1"; shift
    tar "${TAR[@]}" -cf - "$@" | gzip -n -6 > "$dest.part"
    mv -f "$dest.part" "$dest"
}

PKG="$PARENT/$NAME-private.tar.gz"
if [[ $ARCHIVES == *" private "* ]] && [ -n "$NODISCS" ]; then
    if [ -n "${TRON20SRC_ARCHIVES:-}" ]; then
        echo "   $PKG: $OUT was built without the retail game, but TRON20SRC_ARCHIVES asks for it" >&2
        exit 1
    fi
    echo "   $PKG: skipped (built without the retail game)"
elif [[ $ARCHIVES == *" private "* ]]; then
    mkdir -p "$S/private/Profiles"
    cp "$OUT/autoexec.cfg.retail" "$S/private/autoexec.cfg"
    cp "$OUT/Default.prf.retail" "$S/private/Profiles/Default.prf"
    targz "$PKG" -C "$PARENT" "${EXCLUDE[@]}" "${NAMES[@]}" \
        "$DIR" "${STAGED[@]}" "$SB/private/autoexec.cfg" "$SB/private/Profiles/Default.prf"
    echo "   $PKG: $(du -h "$PKG" | cut -f1) (unpacks to $NAME/)"
fi
[[ $ARCHIVES == *" public "* ]] || exit 0

# The public archive: the install scripts beside the rest, the retail files
# left out.
PUB="$PARENT/$NAME-public.tar.gz"
python3 "$HERE/mkinstall.py" "$OUT" "$S"
cat "$S/disc-files.txt" "$S/disc-dirs.txt" | sed "s|^|$DIR/|" > "$S/exclude.txt"
echo "$DIR/MANIFEST.txt" >> "$S/exclude.txt"
tar "${TAR[@]}" -C "$PARENT" -cf "$S/public.tar" "${EXCLUDE[@]}" \
    --anchored --no-wildcards --exclude-from="$S/exclude.txt" \
    "${NAMES[@]}" \
    "$DIR" "${STAGED[@]}" "$SB/MANIFEST.txt" "$SB/linux-install.sh" "$SB/windows-install.bat"
# Last, SHA256SUMS: every file of the archive and every file the install
# scripts write, but autoexec.cfg and Profiles/Default.prf, which the game
# rewrites. `sha256sum -c SHA256SUMS` checks an install.
python3 - "$S/public.tar" "$S/disc-sums.txt" > "$S/SHA256SUMS" <<'PY'
import hashlib, sys, tarfile
lines = []
with tarfile.open(sys.argv[1]) as t:
    for m in t:
        rel = m.name.split("/", 1)[1] if "/" in m.name else ""
        if m.isfile() and rel:
            lines.append(f"{hashlib.sha256(t.extractfile(m).read()).hexdigest()}  {rel}")
lines += open(sys.argv[2]).read().splitlines()
names = [l[66:] for l in lines]
if len(set(n.lower() for n in names)) != len(names):
    sys.exit("package.sh: a file twice in SHA256SUMS")
for l in sorted(lines, key=lambda l: l[66:].lower()):
    sys.stdout.write(l + "\n")
PY
tar "${TAR[@]}" -C "$PARENT" -rf "$S/public.tar" "${NAMES[@]}" "$SB/SHA256SUMS"
gzip -n -6 < "$S/public.tar" > "$PUB.part"
# no retail file in it, and nothing that names the machine it was built on
python3 "$HERE/pubcheck.py" "$PUB.part" "$OUT"
mv -f "$PUB.part" "$PUB"
echo "   $PUB: $(du -h "$PUB" | cut -f1) (unpacks to $NAME/; its data from the discs and Update 2)"
