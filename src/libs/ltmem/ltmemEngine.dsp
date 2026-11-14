# Microsoft Developer Studio Project File - Name="ltmemEngine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ltmemEngine - Win32 Final
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ltmemEngine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ltmemEngine.mak" CFG="ltmemEngine - Win32 Final"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ltmemEngine - Win32 Final" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Ours: ltmem.dsp as Lithtech.exe and server.dll link it (the library keeps
# its name), /O2 without /Ob2 as the engine's StdLith: retail's static
# initializers keep their `jmp` stubs and CLTMemInitialize's constructor is
# out of line, calling LTMemInit (Lithtech.exe 0x4c8f10..0x4c8f70, server.dll
# 0x1004b150..). ltmem.dsp (/Ob2) is the sound driver's and the game DLLs'.
# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ltmemEngine - Win32 Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Final"
# PROP BASE Intermediate_Dir "Final"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Engine\built\Final\ltmemEngine"
# PROP Intermediate_Dir "..\..\Engine\built\Final\ltmemEngine"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\Engine\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Engine\built\Final\ltmemEngine\ltmem.lib"
!ENDIF 

# Begin Target

# Name "ltmemEngine - Win32 Final"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ltmem.cpp
# End Source File
# Begin Source File

SOURCE=.\ltmemstats.cpp
# End Source File
# Begin Source File

SOURCE=.\ltmemtrack.cpp
# End Source File
# Begin Source File

SOURCE=.\ltmemheap.cpp
# End Source File
# End Group
# End Target
# End Project
