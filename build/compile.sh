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
# parallel jobs (default: the CPU count).
set -e
source "$(dirname "$0")/env.sh"
tron20src_release_tree "$@"
cd "$REPO"
# the launcher's and the dedicated server's art, from the retail game
# (build/appres.py, once the launcher and the server have source:
# docs/PLAN.md, package 6): what their resource scripts compile
if [ -f build/appres.py ]; then
    source "$REPO/build/retail.sh"
    python3 build/appres.py "$(retail_dir)" src/Game/built/retail
fi
tron20src_make "${@:-all}"
