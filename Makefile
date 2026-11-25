# Tron 2.0, built from source.
#
#   make          build the game into out/tron20-legacy-release/ (and its -private / -public .tar.gz)
#   make modern   the game with TRON20SRC_MODERN, in out/tron20-modern-release/
#                 (build/variant.sh; also make debug, make modern-debug)
#   make test     run the game under Xvfb and Wine to its main menu
#   make progress how much of the build is byte-identical to the retail game;
#                 fails if a function recorded as identical stops matching
#   make ci       the CI build and check, without the retail game
#   make help     everything else
#
# Needs only docker on the host, the two submodules in ext/ (`git submodule
# update --init`): the compilers, ext/toolchains/ (ext/toolchains/README.md),
# and ext/tools/, the reconstruction tools -- and your own images of the two
# USA discs and the executable of Update 2 (v1.030), in isos/ (or
# TRON20SRC_ISOS=<dir>), from which `make retail`, which every build runs
# first, installs the retail game once (build/discs.py). Every step runs in
# a throwaway container of the image docker/Dockerfile describes, with this
# checkout mounted at /repo, as you.

IMAGE     ?= tron20src-build
CONTAINER ?= tron20src-build
# Parallel compile jobs inside the build (build/build.mk); empty: one per CPU
JOBS ?=
# The game directory `make test` plays, the world it starts (-: none, the
# main menu) and the input script (build/playtest-*.txt; empty: no input).
# make tests plays every script in TEST_SCRIPTS.
TEST_GAME ?= out/tron20-legacy-release
TEST_SCRIPT ?=
TEST_WORLD ?= -
TEST_NAME = $(if $(filter out/tron20-legacy-release,$(TEST_GAME:%/=%)),test,test-$(notdir $(TEST_GAME:%/=%)))
# make tests' run names: test-<script> for the legacy release, and
# test-<game directory>-<script> for any other game directory
TESTS_PREFIX = $(if $(filter out/tron20-legacy-release,$(TEST_GAME:%/=%)),test-,test-$(notdir $(TEST_GAME:%/=%))-)
TEST_SCRIPTS ?= menus level save race

# Your disc images and the update, if they are not in isos/:
# TRON20SRC_ISOS=<directory> (mounted read-only). NODISCS=1: compile without
# the retail game (`make ci`)
TRON20SRC_ISOS ?=
NODISCS ?=
ISO_MOUNT := $(if $(TRON20SRC_ISOS),-v $(abspath $(TRON20SRC_ISOS)):/isos:ro -e TRON20SRC_ISOS=/isos)

DOCKER_OPTS = --rm --init \
	-u $(shell id -u):$(shell id -g) -e HOME=/tmp/home $(if $(JOBS),-e TRON20SRC_JOBS=$(JOBS)) \
	$(if $(TRON20SRC_ARCHIVES),-e TRON20SRC_ARCHIVES="$(TRON20SRC_ARCHIVES)") \
	$(if $(filter 1,$(NODISCS)),-e TRON20SRC_NODISCS=1) \
	-v $(CURDIR):/repo -w /repo $(ISO_MOUNT)
DOCKER_RUN = docker run $(DOCKER_OPTS) --name $(CONTAINER) $(IMAGE)

# VC6 as Tron 2.0's Rich headers name it: cl and link 12.00.8168
# (progress/COMPILERS.md). The DirectX 9 SDK the engine needs is not in
# ext/toolchains/ yet (docs/PLAN.md, package 2).
TOOLCHAIN_FILES = ext/toolchains/msvc-6.00-8168/BIN/cl.exe ext/toolchains/msvc-6.00-8168/BIN/link.exe

.PHONY: all game compile debug modern modern-debug image retail check-tools check-toolchain \
	test tests mptest launchertest sweep control reference art art-check progress progress-update progress-ci ci shell clean distclean help

all: game

help:
	@echo "make            build everything into out/tron20-legacy-release/ and its .tar.gz archives"
	@echo "make debug      the game with symbols and TRON20SRC_DEBUG logging, in out/tron20-legacy-debug/"
	@echo "make modern     the game with TRON20SRC_MODERN, in out/tron20-modern-release/"
	@echo "make modern-debug  both, in out/tron20-modern-debug/"
	@echo "                (build/variant.sh: each is built in out/build-<variant>/ and is never"
	@echo "                scored; the release build and its outputs are not touched)"
	@echo "make test       run out/tron20-legacy-release/ under Xvfb and Wine to its main menu"
	@echo "                (build/playtest.sh; TEST_WORLD=<world>, TEST_SCRIPT=<script>,"
	@echo "                TEST_GAME=<dir>; results in out/play/test*/)"
	@echo "make control    out/tron20-control/: the retail game DLLs with our engine, to tell"
	@echo "                our game code's faults from the engine's or Wine's (diagnosis only)"
	@echo "make tests      every scripted test (TEST_SCRIPTS: build/playtest-<name>.txt);"
	@echo "                fails if one does not end ALIVE (out/play/test-<name>/, or"
	@echo "                test-<game dir>-<name>/ with TEST_GAME=<dir>)"
	@echo "make sweep      every single-player world, about a minute each (build/sweep.sh;"
	@echo "                SWEEP_WORLDS=<text>, SWEEP_JOBS=<n>; out/play/sweep-summary.txt)"
	@echo "make compile    compile and link what has source, in the source tree (build/compile.sh;"
	@echo "                TARGETS=\"<project or output>...\" for only those): what make progress scores"
	@echo "make progress   score the build against the retail binaries; fails if a"
	@echo "                function recorded in progress/identical.txt stops matching"
	@echo "make progress-update  record the score in progress/ and README.md, and the"
	@echo "                hashes make progress-ci checks against in progress/hashes/"
	@echo "make progress-ci  the check without the retail game: the build against"
	@echo "                progress/hashes/ (finds no new matches)"
	@echo "make reference  extract the retail binaries and their function tables into out/reference/"
	@echo "make ci         what CI runs: make NODISCS=1 compile (without the retail"
	@echo "                game: the same binaries, from assets/art/), then make progress-ci"
	@echo "make retail     install the retail game from your two disc images and Update 2"
	@echo "                in isos/ (once; every build runs it), and check it"
	@echo "make art        write the retail art our binaries are compiled with into"
	@echo "                assets/art/ again, from the retail binaries"
	@echo "make art-check  check that assets/art/ is the retail art (every compile with"
	@echo "                the retail game also does this)"
	@echo "make shell      a shell in the build container (build/*.sh, ext/tools/ work there)"
	@echo "make image      (re)build the docker image $(IMAGE)"
	@echo "make clean      remove out/ and every build output in the source tree"
	@echo "make distclean  clean, and remove the docker image too"
	@echo
	@echo "Variables: JOBS=<n> (parallel compiles; default one per CPU)"
	@echo "           TRON20SRC_ISOS=<dir> (your disc images and the update, if not in isos/)"
	@echo "           NODISCS=1 (make compile without the retail game; see make ci)"
	@echo "           TRON20SRC_ARCHIVES=\"private public\" (the archives build/package.sh writes)"
	@echo "           IMAGE_BUILD=0 (use the docker image as it is, as CI does)"
	@echo "           COMPLETE=1, COVERED=1, ACCEPT_LOSSES=1, ACCEPT_FAILED_CHECKS=1 (progress/README.md)"

game: check-toolchain retail
	$(DOCKER_RUN) bash build/all.sh

# The variants (build/variant.sh): the same source with TRON20SRC_DEBUG and
# symbols, or TRON20SRC_MODERN, or both, each in its own tree,
# out/build-<variant>/, and game directory
debug modern modern-debug: check-toolchain retail
	$(DOCKER_RUN) bash build/variant.sh $@

# The runtime test (build/playtest.sh, ext/tools/playtest.sh): the game
# directory under Xvfb and Wine, ALIVE or not, a screenshot in
# out/play/<name>/shot.png
test: image
	@[ -f $(TEST_GAME)/Lithtech.exe ] || { echo "error: no $(TEST_GAME)/ -- build it first (make)" >&2; exit 1; }
	$(DOCKER_RUN) env PLAYTEST_NAME=$(TEST_NAME) $(if $(TEST_SCRIPT),PLAYTEST_SCRIPT=$(TEST_SCRIPT)) bash build/playtest.sh $(TEST_GAME) '$(TEST_WORLD)' $(if $(TEST_SCRIPT),5,60)

# The control for the runtime tests (build/control.sh): retail's game DLLs
# with our engine, in out/tron20-control/ (make test TEST_GAME=out/tron20-control).
# For diagnosis only.
control:
	@bash build/control.sh

# Every scripted test, one after the other (each its own out/play/test-<name>/)
tests: image
	@[ -f $(TEST_GAME)/Lithtech.exe ] || { echo "error: no $(TEST_GAME)/ -- build it first (make)" >&2; exit 1; }
	@rc=0; for t in $(TEST_SCRIPTS); do \
	  w=$$(sed -n 's/^# world: *//p' build/playtest-$$t.txt | head -1); \
	  echo "== $$t (world $${w:--})"; \
	  $(DOCKER_RUN) env PLAYTEST_NAME=$(TESTS_PREFIX)$$t PLAYTEST_SCRIPT=build/playtest-$$t.txt \
	    bash build/playtest.sh $(TEST_GAME) "$${w:--}" 5 || rc=1; \
	done; exit $$rc

# The multiplayer test (build/mptest.sh): a server and two clients, each on
# a display and a Wine prefix of its own. MPTEST_MODE=dedicated (TRONSrv.exe)
# or listen (client 1 hosts); MPTEST_TYPE=arena, tourn or lc. Slow (some
# minutes): not part of make tests. Results in out/play/<game>-mp-<mode>-<type>/
MPTEST_MODE ?= dedicated
MPTEST_TYPE ?= arena
mptest: image
	@[ -f $(TEST_GAME)/Lithtech.exe ] || { echo "error: no $(TEST_GAME)/ -- build it first (make)" >&2; exit 1; }
	$(DOCKER_RUN) env MPTEST_MODE=$(MPTEST_MODE) MPTEST_TYPE=$(MPTEST_TYPE) $(if $(MPTEST_NAME),MPTEST_NAME=$(MPTEST_NAME)) $(if $(MPTEST_SRVEXE),MPTEST_SRVEXE=$(MPTEST_SRVEXE)) $(if $(MPTEST_CLIENTS),MPTEST_CLIENTS=$(MPTEST_CLIENTS)) $(if $(MPTEST_KEYS1),MPTEST_KEYS1="$(MPTEST_KEYS1)") $(if $(MPTEST_KEYS2),MPTEST_KEYS2="$(MPTEST_KEYS2)") $(if $(MPTEST_LANONLY),MPTEST_LANONLY=$(MPTEST_LANONLY)) $(if $(MPTEST_WINEDEBUG),MPTEST_WINEDEBUG=$(MPTEST_WINEDEBUG)) $(if $(MPTEST_MASTER),MPTEST_MASTER=$(MPTEST_MASTER)) $(if $(MPTEST_MASTERVIA),MPTEST_MASTERVIA=$(MPTEST_MASTERVIA)) $(if $(MPTEST_JOINKEYS2),MPTEST_JOINKEYS2="$(MPTEST_JOINKEYS2)") \
	  bash build/mptest.sh $(TEST_GAME)

# The launcher test (build/launchertest.sh): Tron.exe with the registry of
# the game's setup, its dialogs, and Launch to the main menu. Results in
# out/play/test-launcher/ (LAUNCHER_SCRIPT=<script>: another input script;
# LAUNCHERTEST_RETAIL=1: the retail Tron.exe, the control; LAUNCHER_EXE=<exe>:
# another executable of the game directory, as the modern build's
# TronLauncher.exe, the launcher, and Tron.exe, which starts the game:
# build/playtest-launcher-direct.txt)
LAUNCHER_SCRIPT ?= build/playtest-launcher.txt
LAUNCHER_EXE ?=
launchertest: image
	@[ -f $(TEST_GAME)/$(or $(LAUNCHER_EXE),Tron.exe) ] || { echo "error: no $(TEST_GAME)/$(or $(LAUNCHER_EXE),Tron.exe) -- build it first (make)" >&2; exit 1; }
	$(DOCKER_RUN) env $(if $(PLAYTEST_NAME),PLAYTEST_NAME=$(PLAYTEST_NAME)) $(if $(LAUNCHER_EXE),PLAYTEST_LAUNCH=$(LAUNCHER_EXE)) $(if $(LAUNCHERTEST_RETAIL),LAUNCHERTEST_RETAIL=1) bash build/launchertest.sh $(TEST_GAME) $(LAUNCHER_SCRIPT)

# The world sweep (build/sweep.sh): every single-player world of the retail
# game (build/sweep-worlds.txt), each played for about a minute in its own
# container, SWEEP_JOBS at a time; SWEEP_WORLDS=<text>: only the worlds
# whose path has it. Results in out/play/$(SWEEP_NAME)-summary.txt and
# -sheet.png. Slow (about 15 minutes): not part of make tests.
SWEEP_NAME ?= sweep
SWEEP_JOBS ?= 4
SWEEP_WORLDS ?=
sweep: image
	@SWEEP_DOCKER="docker run $(DOCKER_OPTS)" IMAGE=$(IMAGE) CONTAINER=$(CONTAINER) \
	  SWEEP_NAME=$(SWEEP_NAME) SWEEP_JOBS=$(SWEEP_JOBS) bash build/sweep.sh $(TEST_GAME) '$(SWEEP_WORLDS)'

# Compile and link only (build/compile.sh): every binary that has source,
# in src/**/built/, which is what make progress scores (make game does this
# too, then assembles the game directory: build/all.sh).
compile: check-toolchain retail
	$(DOCKER_RUN) bash build/compile.sh $(TARGETS)

# IMAGE_BUILD=0: use $(IMAGE) as it is (CI builds it with a layer cache first).
IMAGE_BUILD ?= 1
image:
ifeq ($(IMAGE_BUILD),0)
	@docker image inspect $(IMAGE) >/dev/null 2>&1 || \
	  { echo "error: IMAGE_BUILD=0, but there is no docker image $(IMAGE)" >&2; exit 1; }
else
	@docker build -q -t $(IMAGE) docker/ >/dev/null || \
	  { echo "error: could not build the docker image (docker/Dockerfile)" >&2; exit 1; }
endif
	@echo "== docker image $(IMAGE) ready"

# The retail game the build takes its data from (build/retail.sh): installed
# from your disc images and the update (isos/, or TRON20SRC_ISOS) by
# build/discs.py the first time, checked against build/retail-files.txt
# every time. Nothing with NODISCS=1.
retail: check-tools image
ifeq ($(NODISCS),1)
	@echo "== NODISCS=1: no retail game (the compile takes its art from assets/art/)"
else
	@$(DOCKER_RUN) bash -c 'source build/retail.sh && retail_install && retail_verify'
endif

reference: retail
	$(DOCKER_RUN) bash build/reference.sh

# The retail art and text our binaries are compiled with, committed in
# assets/art/: written again from the retail binaries' resources, or checked
# against them (build/appres.py; every compile with the retail game checks)
art: check-tools retail
	$(DOCKER_RUN) bash -c 'source build/retail.sh && python3 build/appres.py write "$$(retail_dir)" assets/art'

art-check: check-tools retail
	$(DOCKER_RUN) bash -c 'source build/retail.sh && python3 build/appres.py check "$$(retail_dir)" assets/art'

check-tools:
	@[ -f ext/tools/progress.py ] || { echo "error: ext/tools/ is empty -- the reconstruction tools are the ext/tools/ submodule: run git submodule update --init" >&2; exit 1; }

check-toolchain:
	@for f in $(TOOLCHAIN_FILES); do \
	  [ -f "$$f" ] || { echo "error: $$f is missing -- the compilers are the ext/toolchains/ submodule: run git submodule update --init (see ext/toolchains/README.md)" >&2; exit 1; }; \
	done

# what ext/tools/evidence.py cannot ask git for in the container (the build
# image has no git)
EVIDENCE_ENV = EVIDENCE_REVISION=$(shell git rev-parse HEAD 2>/dev/null) \
	EVIDENCE_MODIFIED=$(shell git status --porcelain --untracked-files=no 2>/dev/null | wc -l) \
	EVIDENCE_SUBMODULES="$(shell git submodule status 2>/dev/null | sed 's/^ *//' | tr '\n' ';')"

progress: retail
	$(DOCKER_RUN) env COMPLETE=$(COMPLETE) COVERED=$(COVERED) $(EVIDENCE_ENV) bash build/progress.sh check

progress-update: retail
	$(DOCKER_RUN) env ACCEPT_LOSSES=$(ACCEPT_LOSSES) ACCEPT_FAILED_CHECKS=$(ACCEPT_FAILED_CHECKS) $(EVIDENCE_ENV) bash build/progress.sh update

# Without the retail game: the build against progress/hashes/ (build/progress.sh ci)
progress-ci: check-tools image
	$(DOCKER_RUN) bash build/progress.sh ci

# What CI runs (.github/workflows/ci.yml): the compile without the retail
# game, then the check without it
ci:
	$(MAKE) --no-print-directory NODISCS=1 compile
	$(MAKE) --no-print-directory progress-ci

shell: image
	docker run --rm -it --init --name $(CONTAINER)-shell \
	  -u $(shell id -u):$(shell id -g) -e HOME=/tmp/home \
	  -v $(CURDIR):/repo -w /repo $(ISO_MOUNT) $(IMAGE) bash

# (out/retail/, the installed retail game, goes too: the next build installs
# it again from your disc images)
clean:
	rm -rf out .wine .build
	[ ! -d src ] || find src -depth -type d \( -name built -o -name Built \) -exec rm -rf {} +

distclean: clean
	-docker rmi $(IMAGE)
