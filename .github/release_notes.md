<!--
The release text (.github/workflows/ci.yml, the job "release"):
{{TAG}}, {{ARCHIVE}} and {{REPO}} are filled in, and this comment is left out.
-->
## Download

| File | What it is |
|---|---|
| `tron20src-{{ARCHIVE}}.tar.gz` | **Use this file**: Tron 2.0 (USA, Update 2, v1.030) built from source, the modern build. |
| `tron20src-debug-{{ARCHIVE}}.tar.gz` | The same build with debug information (`.pdb` and `.map` files). Use it to report bugs. |

**Work in progress.** The reconstruction is not complete: see [the README](https://github.com/{{REPO}}/blob/{{TAG}}/README.md) before you expect a game that you can play from start to end.

**These files contain no game data.** They contain only the binaries that this project builds from source, the licences of the open-source code in them, and an installer. The installer copies the game's data from your own copies of:

- the two discs of Tron 2.0 (USA): redump's `Tron 2.0 (USA) (Disc 1)` and `Tron 2.0 (USA) (Disc 2)`, as a plain `.iso`, a raw `.bin`/`.cue`, or a `.zip` of one;
- Update 2 (v1.030), the "complete" update, as it was published: `tron-update_v1x2_complete.exe` (32479660 bytes), or a `.zip` that holds it.

The installer reads these files. It does not run them. It checks the SHA-256 of each file that it writes.

## Install

**This is the whole game, not a patch.** Unpack it into a new, empty directory and run the installer in that directory. To upgrade, unpack a new release over your installed game (`tar xzf <archive> --strip-components=1 -C <game directory>`); you do not have to run the installer again. `sha256sum -c SHA256SUMS` in the game directory checks the install.

### Windows

Unpack the archive. Drop the two disc images and the update onto `windows-install.bat`, or put them in the game folder and double-click `windows-install.bat`. Then start `Tron.exe` (or the launcher, `TronLauncher.exe`).

### Linux

There is no native Linux build. The game is a Windows program: use Wine or Proton.

```sh
tar xzf tron20src-{{ARCHIVE}}.tar.gz
cd tron20src-{{ARCHIVE}}
./linux-install.sh /path/to/disc1.iso /path/to/disc2.iso /path/to/tron-update_v1x2_complete.exe
wine Tron.exe
```

The installer needs bash, python3 and GNU coreutils.

## What the modern build changes

[`MODERN.md`](https://github.com/{{REPO}}/blob/{{TAG}}/docs/MODERN.md) lists each change from the retail game. [`RETAIL_BUGS.md`](https://github.com/{{REPO}}/blob/{{TAG}}/docs/RETAIL_BUGS.md) lists the bugs of the retail game that it fixes. The settings and the saved games stay in the game directory: the game does not use the registry or APPDATA. Its `binkw32.dll` is our Bink player, linked statically with [FFmpeg](https://ffmpeg.org) (LGPL-2.1-or-later). Its `SndDrv.dll` contains [minimp3](https://github.com/lieff/minimp3) (CC0). `LICENSE_NOTES.md` and `licenses/` in the archive tell which code is whose, and where its source is.

## Testing

The game is a 32-bit Windows program. We test it on Linux with Wine. Windows has had less testing. Report problems in the [issues](https://github.com/{{REPO}}/issues).
