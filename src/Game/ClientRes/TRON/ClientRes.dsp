# Microsoft Developer Studio Project File - Name="ClientRes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ClientRes - Win32 Final

# tron20src: CRes.dll, the client's resources: Build 69's ClientRes\TRON
# project with the Final configuration of NOLF2's source release
# (TO2\ClientRes.dsp). The DLL has only DllMain (main.cpp) and resources.
# ClientRes.rc is retail CRes.dll's resources written back out
# (ext/tools/rsrc2rc.py); its cursor is written out of the retail image at
# build time by build/appres.py into ..\..\built\retail\ClientRes. This is
# the VC6 format ext/tools/dsp2mk.py reads, with its Final configuration
# only.

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ClientRes - Win32 Final"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\built\Final\ClientRes\TRON"
# PROP Intermediate_Dir "..\..\built\Final\ClientRes\TRON"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_FINAL" /FD /c
# ADD RSC /l 0x409 /i ".\\" /i "..\Shared" /i "..\..\built\retail\ClientRes" /d "NDEBUG"
LINK32=link.exe
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\built\Final\ClientRes\TRON\CRes.dll"

!ENDIF

# Begin Target

# Name "ClientRes - Win32 Final"
# Begin Source File

SOURCE=.\ClientRes.rc
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# End Target
# End Project
