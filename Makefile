# Tron 2.0, built from source.
#
#   make          build the game into out/tron20-legacy-release/ (and its -private / -public .tar.gz)
#   make modern   the game with TRON20_MODERN, in out/tron20-modern-release/
#   make progress how much of the build is byte-identical to the retail game;
#                 fails if a function recorded as identical stops matching
#   make help     everything else
#
# Needs only docker on the host, the two submodules in ext/ (`git submodule
# update --init`): the compilers, ext/toolchains/ (ext/toolchains/README.md),
# and ext/tools/, the reconstruction tools -- and your own images of the two
# USA discs and the executable of Update 2 (v1.030), in isos/ (or
# TRON20_ISOS=<dir>), from which `make retail`, which every build runs
# first, installs the retail game once (build/discs.py). Every step runs in
# a throwaway container of the image docker/Dockerfile describes, with this
# checkout mounted at /repo, as you.

IMAGE     ?= tron20src-build
CONTAINER ?= tron20src-build
# Parallel compile jobs inside the build (build/build.mk); empty: one per CPU
JOBS ?=

# Your disc images and the update, if they are not in isos/:
# TRON20_ISOS=<directory> (mounted read-only)
TRON20_ISOS ?=
ISO_MOUNT := $(if $(TRON20_ISOS),-v $(abspath $(TRON20_ISOS)):/isos:ro -e TRON20_ISOS=/isos)

DOCKER_RUN = docker run --rm --init --name $(CONTAINER) \
	-u $(shell id -u):$(shell id -g) -e HOME=/tmp/home $(if $(JOBS),-e TRON20SRC_JOBS=$(JOBS)) \
	$(if $(TRON20_ARCHIVES),-e TRON20_ARCHIVES="$(TRON20_ARCHIVES)") \
	-v $(CURDIR):/repo -w /repo $(ISO_MOUNT) $(IMAGE)

# VC6 as Tron 2.0's Rich headers name it: cl and link 12.00.8168
# (progress/COMPILERS.md). The DirectX 9 SDK the engine needs is not in
# ext/toolchains/ yet (docs/PLAN.md, package 2).
TOOLCHAIN_FILES = ext/toolchains/msvc-6.00-8168/BIN/cl.exe ext/toolchains/msvc-6.00-8168/BIN/link.exe

.PHONY: all game compile debug modern modern-debug image retail check-tools check-toolchain \
	check-build reference progress progress-update progress-ci shell clean distclean help

all: game

help:
	@echo "make            build everything into out/tron20-legacy-release/ and its .tar.gz archives"
	@echo "make debug      the game with symbols and TRON20_DEBUG logging, in out/tron20-legacy-debug/"
	@echo "make modern     the game with TRON20_MODERN, in out/tron20-modern-release/"
	@echo "make modern-debug  both, in out/tron20-modern-debug/"
	@echo "                (none of these is ever scored)"
	@echo "make compile    compile and link what has source, in the source tree (build/compile.sh;"
	@echo "                TARGETS=\"<project or output>...\" for only those): what make progress scores"
	@echo "make progress   score the build against the retail binaries; fails if a"
	@echo "                function recorded in progress/identical.txt stops matching"
	@echo "make progress-update  record the score in progress/ and README.md, and the"
	@echo "                hashes make progress-ci checks against in progress/hashes/"
	@echo "make progress-ci  the check without the retail game: the build against"
	@echo "                progress/hashes/ (finds no new matches)"
	@echo "make reference  extract the retail binaries and their function tables into out/reference/"
	@echo "make retail     install the retail game from your two disc images and Update 2"
	@echo "                in isos/ (once; every build runs it), and check it"
	@echo "make shell      a shell in the build container (build/*.sh, ext/tools/ work there)"
	@echo "make image      (re)build the docker image $(IMAGE)"
	@echo "make clean      remove out/ and every build output in the source tree"
	@echo "make distclean  clean, and remove the docker image too"
	@echo
	@echo "Variables: JOBS=<n> (parallel compiles; default one per CPU)"
	@echo "           TRON20_ISOS=<dir> (your disc images and the update, if not in isos/)"
	@echo "           TRON20_ARCHIVES=\"private public\" (the archives build/package.sh writes)"
	@echo "           IMAGE_BUILD=0 (use the docker image as it is, as CI does)"
	@echo "           COMPLETE=1, COVERED=1, ACCEPT_LOSSES=1, ACCEPT_FAILED_CHECKS=1 (progress/README.md)"

game: check-toolchain check-build retail
	$(DOCKER_RUN) bash build/all.sh

debug modern modern-debug: check-toolchain check-build retail
	$(DOCKER_RUN) bash build/variant.sh $@

# The build itself is docs/PLAN.md's package 2: until it lands, say so
# rather than fail inside the container.
# Compile and link only (build/compile.sh): every binary that has source,
# in src/**/built/, which is what make progress scores. Until build/all.sh
# assembles a game directory, this is the build.
compile: check-toolchain retail
	$(DOCKER_RUN) bash build/compile.sh $(TARGETS)

check-build:
	@[ -f build/all.sh ] || { echo "error: no game directory is assembled yet (build/all.sh): docs/PLAN.md, package 2. make compile builds the binaries that have source; make progress scores them" >&2; exit 1; }

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
# from your disc images and the update (isos/, or TRON20_ISOS) by
# build/discs.py the first time, checked against build/retail-files.txt
# every time.
retail: check-tools image
	@$(DOCKER_RUN) bash -c 'source build/retail.sh && retail_install && retail_verify'

reference: retail
	$(DOCKER_RUN) bash build/reference.sh

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
