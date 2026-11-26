#!/usr/bin/env bash
# lithrez, the command line tool for LithTech .rez archives, built from the
# lithtools submodule (ext/lithtools/tools/lithrez; C++17, CMake) into
# out/lithrez/lithrez (or $TRON20SRC_LITHREZ_OUT/lithrez: build/variant.sh
# gives its trees the checkout's), without its Qt GUI and tests.
# build/all.sh packs assets/rez/ into the game directory's tron20src.rez
# with it (step 1b). It is built again only when its sources are newer than
# the binary. Prints the binary's path.
#
#   build/lithrez.sh
set -e
SRC=ext/lithtools/tools/lithrez
OUT="${TRON20SRC_LITHREZ_OUT:-out/lithrez}"
[ -f "$SRC/CMakeLists.txt" ] || {
    echo "error: ext/lithtools/ is empty -- it is a submodule: run git submodule update --init ext/lithtools" >&2
    exit 1
}
# its real path: a variant tree's ext/lithtools is a link to the
# checkout's, and CMake's cache keeps the source directory it was made for
SRC="$(cd "$SRC" && pwd -P)"
if [ ! -x "$OUT/lithrez" ] ||
   [ -n "$(find "$SRC/CMakeLists.txt" "$SRC/cli" "$SRC/core" -newer "$OUT/lithrez" -print -quit)" ]; then
    cmake -S "$SRC" -B "$OUT" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DLITHREZ_BUILD_GUI=OFF -DLITHREZ_BUILD_TESTS=OFF >/dev/null
    cmake --build "$OUT" >/dev/null
fi
echo "$OUT/lithrez"
