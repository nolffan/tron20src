# src/modern/directmusic/: Microsoft's DirectMusic in the game directory

The music of Tron 2.0 is DirectMusic: segments (`MUSIC/*/*.sgt`) whose
wave tracks stream MP3 waves. The modern build carries Microsoft's
DirectMusic in its game directory, as nolfsrc (M-P4-12) and avp2src
(M-X4) do, so that the game does not depend on the DirectMusic of the
system or of a Wine prefix (`docs/MODERN.md`, X-3).

Under Wine 9.0 DirectMusic does not play the music: see "Under Wine"
below. There, the engine plays the music with its own player.

## How

* **The DLLs.** `build/all.sh` step 2 (`ext/tools/retailextract.py
  openruntime`, with `DIRECTMUSIC_MANIFESTS` in `build/retail.py`) copies
  the nine DirectMusic DLLs of DirectX 9.0c from
  `ext/toolchains/directmusic-5.3.1.904/` (checked against its
  `SHA256SUMS`) into the game directory as `ms_<name>.dll`: `ms_dmband.dll`,
  `ms_dmcompos.dll`, `ms_dmime.dll`, `ms_dmloader.dll`, `ms_dmscript.dll`,
  `ms_dmstyle.dll`, `ms_dmsynth.dll`, `ms_dmusic.dll`, `ms_dswave.dll`. The
  audit of step 5 lists them as `THIRD-PARTY`, with the file that each one
  comes from. Microsoft's `dsound.dll` is not one of them: the game uses
  the DirectSound of the system.
* **The manifest.** `Lithtech.exe.manifest`, beside `Lithtech.exe`, is
  registration-free COM. It maps each DirectMusic class to its DLL in the
  game directory. Thus each `CoCreateInstance` of one (the engine's, and
  the ones that the DirectMusic DLLs make of each other) finds Microsoft's
  code, and nothing goes into the registry. `Lithtech.exe` has no
  manifest resource of its own, so Windows and Wine both read this file.
* **The names.** Wine loads its builtin `dmime.dll` also when a file of
  that name is in the game directory. Wine has no builtin `ms_dmime.dll`,
  so it loads the file. No one of the nine DLLs imports another by name.
* **The MP3 waves.** Microsoft's `dswave.dll` decodes compressed waves
  through ACM with no driver handle. The modern `SndDrv.dll` adds its MP3
  codec to ACM as a local driver of the process (X-2), so these waves can
  use it.

## Under Wine

Tron 2.0's engine starts DirectMusic with a DirectX 8 audio path
(`IDirectMusicPerformance8::InitAudio`, then `CreateStandardAudioPath`,
`ltdirectmusic_impl.cpp` `InitPerformance`). With Microsoft's DirectMusic
under Wine 9.0, `InitAudio` succeeds, but `CreateStandardAudioPath` fails
with `E_NOINTERFACE`: the synthesizer asks DirectSound for
`IDirectSoundConnect` (`{2a8af120-e9de-4132-aaa5-4bdda5f325b8}`), which
Wine's DirectSound does not have. Retail then has no music. The modern
engine then plays the music with its own player
(`src/tron20src/music/musicplayer.cpp`, `docs/MODERN.md` X-3). With Wine's own
DirectMusic the audio path starts, but nothing plays.

The NOLF and AvP2 engines start DirectMusic in the older way
(`IDirectMusicPerformance::Init` and a synthesizer port). That was tried
and not kept: the port starts, but each segment
with wave tracks fails `IDirectMusicSegment8::Download` with `E_NOTIMPL`
(waves need a performance with audio paths), so nothing plays. The music
of NOLF and AvP2 is DLS instruments, which a port plays; Tron 2.0's is
waves. Microsoft's `dsound.dll` has `IDirectSoundConnect`, but it is not
in the DirectX redistributable runtime. With only `ms_dswave.dll` mapped
(Wine's other DirectMusic DLLs), the game crashes.

On Windows the DLLs are the same DirectX 9.0c DirectMusic that Windows
has, and the audio path starts as in retail.

When `ext/toolchains/directmusic-5.3.1.904/` is not there, the build
gives a warning and installs neither the DLLs nor the manifest (a manifest
that names missing files stops DirectMusic on Windows).

## The class list

The 46 classes are those that the nine DLLs register (the list of
nolfsrc's manifest, less two classes that no one of the DLLs implements;
avp2src checked it). To make the list again from a new set of DLLs,
register them in a scratch Wine prefix and read the registry:

```sh
WINEPREFIX=/tmp/pfx wineboot -i
cp ext/toolchains/directmusic-5.3.1.904/*.dll /tmp/pfx/drive_c/windows/system32/
for f in dmband dmcompos dmime dmloader dmscript dmstyle dmsynth dmusic dswave; do
    WINEPREFIX=/tmp/pfx WINEDLLOVERRIDES="$f=n" wine regsvr32 /s $f.dll
done
grep -A3 'CLSID\\\\{.*}\\\\InprocServer32\]' /tmp/pfx/system.reg
```

Each `InprocServer32` that names one of the nine DLLs is a `<comClass>`
of that DLL's `<file>` (the threading model is `Both` for all of them).
