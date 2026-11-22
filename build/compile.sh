#!/usr/bin/env bash
# Compile and link: build/build.mk, in parallel, in the tree that is built.
#
#   build/compile.sh [target...]     (inside the build container)
#
# Targets are build/build.mk's: all (the default), a project by its workspace
# name (ClientShellDLL, Object, ClientFxDLL, ClientRes, ServerRes, Launcher,
# ServerApp, or a library), or any output file. Started in a checkout it
# builds in out/release-tree/ (src/ without its variant blocks: build/env.sh,
# tron20src_release_tree) and copies the outputs back into this checkout's
# src/; in a build/variant.sh tree, there. TRON20SRC_JOBS sets the number of
# parallel jobs (default: the CPU count). TRON20_NODISCS=1: without the
# retail game (make NODISCS=1 compile): the same binaries, from the
# committed art (assets/art/), with no check against the retail game.
set -e
source "$(dirname "$0")/env.sh"
tron20src_release_tree "$@"
cd "$REPO"
# the launcher's, the dedicated server's and CRes.dll's art: the retail art
# committed in assets/art/ (build/appres.py), copied to where their resource
# scripts read it. With the retail game it is first checked against the
# retail binaries' resources; without it (TRON20_NODISCS=1) it is not.
if [ "${TRON20_NODISCS:-0}" != 1 ]; then
    source "$REPO/build/retail.sh"
    python3 build/appres.py check "$(retail_dir)" assets/art >/dev/null || {
        python3 build/appres.py check "$(retail_dir)" assets/art; exit 1; }
fi
python3 build/appres.py install assets/art src/Game/built/retail
tron20src_make "${@:-all}"
