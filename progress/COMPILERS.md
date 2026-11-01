# Which compiler built what

Every retail PE carries a Rich header, the linker's inventory of the tool
builds that produced its objects (`python3 tools/richhdr.py <image>`). For
Tron 2.0 v1.030 it is the same story in every binary: **Visual C++ 6.0 with
its first compiler and linker, 12.00.8168** (`toolchains/msvc-6.00-8168`) --
not NOLF2's SP5 back end (8966), and not Contract J.A.C.K.'s Visual C++ .NET
2003, though both games came out of the same tree within months of Tron.
Where a revision or flag set has been *proven* by byte-matching, this file
says so; until then the attribution is the header's own testimony.

| Binary | Link date | cl 12.00.8168 objects (C / C++) | Also | Built here with |
|---|---|---|---|---|
| `Lithtech.exe` | 2003-10-03 | 11 / 292 | 110 C++ and 3 C objects of cl 13.00.9178, 6 of masm 6.14.8444, 9 C objects of tool 0x5f build 2067, 4 import descriptors of lib 7.00.9210: a DirectX 9 SDK's `d3dx9.lib` and import libraries (below); 4 objects without a `@comp.id` | -- (no build yet) |
| `server.dll` | 2003-08-29 | 5 / 118 | | -- |
| `SndDrv.dll` | 2003-08-29 | 4 / 8 | one C++ object of cl 13.00.9178 and one without a `@comp.id`: the DirectX SDK's `dxguid.lib` and EAX's GUIDs, as in NOLF2's driver (to be checked) | -- |
| `ltmsg.dll` | 2003-08-29 | 5 / 1 | resources (`cvtres` 5.00.1720) and msvcrt's DLL entry | -- |
| `Tron.exe` | 2003-09-29 | 11 / 26 | MFC 4.2 as a DLL (`MFC42.DLL`) | -- |
| `TRONSrv.exe` | 2003-10-03 | 14 / 28 | MFC 4.2 as a DLL; 3 objects without a `@comp.id` | -- |
| `CShell.dll` | 2003-10-03 | 24 / 346 | the C objects include the GameSpy SDK's | -- |
| `Object.lto` | 2003-10-03 | 9 / 379 | the C objects include the GameSpy SDK's query reporting | -- |
| `ClientFx.fxd` | 2003-08-29 | 101 / 35 | `libcmt.lib` statically: most C objects are the CRT's | -- |
| `CRes.dll`, `SRes.dll` | 2003-08-29 | 22 / 2 | resources and `libcmt.lib` | -- |

Every image's linker row is `link`/`lib` 6.00.8168 (tool 0x04), its
import libraries `lib` 5.12.8034 (0x13), its CRT assembly `ml` 6.13.7299
(0x0e), and, where it has resources, `cvtres` 5.00.1720 -- the one VC6's
first linker brings (NOLF's `ltmsg.dll`, proven with `msvc-6.00-8168`:
`toolchains/COMPILERS.md`; later linkers bring 5.00.1736). The C runtime's own objects are 8168 as
well. So the compiler, the linker and the libraries all look like one VC6
install at its original release; whether a later service pack that kept
that compiler is indistinguishable from it is still to be shown.

## The update's binaries

Update 2 (v1.030) replaced the engine, the launcher, the dedicated server
and the game DLLs. Their link dates fall in three groups: `Lithtech.exe`,
`TRONSrv.exe`, `CShell.dll` and `Object.lto` on 2003-10-03 (Update 2's
own); `server.dll`, `SndDrv.dll`, `ltmsg.dll`, `ClientFx.fxd`, `CRes.dll`
and `SRes.dll` on 2003-08-29 (Update 1's, which Update 2 carries); and the
launcher on 2003-09-29. The 1.010 binaries the discs install (July 2003:
`Lithtech.exe` of 2003-07-10, the game DLLs in `GameDLL.rez`) are not
scored.

## Notes

* **D3DX.** The engine imports `d3d9.dll`, as the 2006 Jupiter tree does,
  where NOLF2's and Contract J.A.C.K.'s import `d3d8.dll`. The cl
  13.00.9178 objects are the D3DX library of a DirectX 9 SDK, linked
  statically, as NOLF2's engine linked the DirectX 8.1 SDK's `d3dx8.lib`.
  Which SDK release is to be found (the 1.010 engine of July 2003 already
  imports `d3d9.dll`, so 9.0 or 9.0a, unless the update's engine moved to
  9.0b); none is in `toolchains/` yet. The 0x5f/2067 C objects are to be
  identified with it.
* NOLF2's codegen lore (nolf2src's `guides/`) is SP5's; nolfsrc's is
  closer, since NOLF's engine and `ltmsg.dll` are 8168/8447. Neither carries
  over unchecked.
