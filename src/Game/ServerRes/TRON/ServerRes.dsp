# Microsoft Developer Studio Project File - Name="ServerRes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ServerRes - Win32 Final

# tron20src: SRes.dll, the server's resources: Build 69's ServerRes\TRON
# project with the Final configuration of ClientRes.dsp (NOLF2's Final
# flags). The DLL has only DllMain (MAIN.CPP). Retail SRes.dll has an empty
# resource directory (.rsrc is 0x10 bytes), and so has ServerRes.rc: its
# only resources are the APSTUDIO_INVOKED TEXTINCLUDEs. This is the VC6
# format ext/tools/dsp2mk.py reads, with its Final configuration only.

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ServerRes - Win32 Final"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\built\Final\ServerRes\TRON"
# PROP Intermediate_Dir "..\..\built\Final\ServerRes\TRON"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_FINAL" /FD /c
# ADD RSC /l 0x409 /i ".\\" /d "NDEBUG"
LINK32=link.exe
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\built\Final\ServerRes\TRON\SRes.dll"

!ENDIF

# Begin Target

# Name "ServerRes - Win32 Final"
# Begin Source File

SOURCE=.\MAIN.CPP
# End Source File
# Begin Source File

SOURCE=.\ServerRes.rc
# End Source File
# End Target
# End Project
