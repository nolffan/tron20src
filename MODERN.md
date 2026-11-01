# The modern build

`TRON20_MODERN` is the project's define for everything that departs from
the retail game: bug fixes (`RETAIL_BUGS.md`), new features, and
open-source middleware in place of the proprietary libraries. It is set
only in the modern build (`make modern`, `make modern-debug`;
`build/variant.sh`), never in the legacy build, which stays the retail game
as far as it can be rebuilt and is the only build `make progress` scores.
Every change goes behind `#ifdef TRON20_MODERN` with the retail code kept,
unchanged, in the `#else`. (`TRON20_DEBUG` is the debug builds' logging and
checks, independent of it.)

This file is the inventory: what the modern build changes, what it could
change next, and what was looked at and left alone, and why. It follows
nolf2src's and cjacksrc's `MODERN.md` in its rules; the items are Tron
2.0's own.

## Rules for this work

* **Reimplement, never copy.** Fan fixes and patches are a fine source for
  *what* is wrong and *why*; their code is not copied here. The same goes
  for nolf2src's and cjacksrc's fixes: the idea often carries over, since
  the code is the same generation, but each change is made and checked
  here.
* **The 2003 compiler.** The modern build uses the legacy build's toolchain
  (Visual C++ 6.0, `msvc-6.00-8168`, and the DirectX 9 SDK the engine
  links).
* **No SecuROM, no disc check.** The engine built here has no SecuROM; the
  modern build also drops the game's own disc and CD-key checks.

## Planned

To bring in once the game runs (each checked against Tron 2.0's code
first; `docs/PLAN.md`, package 9):

* widescreen: menus, HUD and field of view at any aspect ratio, with an
  option to keep the cinematics at 4:3;
* a field-of-view setting;
* up to 16x anisotropic filtering and up to 8x antialiasing;
* raw mouse input;
* controllers through XInput, in place of DirectInput joysticks;
* our Bink player on FFmpeg in place of RAD's `binkw32.dll`, as nolf2src's;
* the launcher as `TronLauncher.exe`, and `Tron.exe` starting the game
  directly, as if Launch had been clicked in the launcher;
* from nolf2src's modern build, where Tron has the same code: windowed
  modes at any size, a frame limiter, Internet play without the dead
  GameSpy master servers.

## Done

(nothing yet)
