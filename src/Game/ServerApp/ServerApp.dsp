# Microsoft Developer Studio Project File - Name="ServerApp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ServerApp - Win32 Final

# tron20src: TRONSrv.exe, the dedicated server: NOLF2's ServerApp project
# (its source release's; MFC 4.2 in a DLL, server.dll's import library,
# ButeMgr and CryptMgr built against MFC) with Tron 2.0's version manager
# (Shared\TRON\TronVersionMgr.cpp in place of NOLF2's TO2VersionMgr.cpp)
# and its Final configuration's flags, VC6 8168. This is the VC6 format
# ext/tools/dsp2mk.py reads, with that configuration only.

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ServerApp - Win32 Final"

# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Built\Final\ServerApp"
# PROP Intermediate_Dir "..\Built\Final\ServerApp"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".\\" /I "..\..\libs\STLPORT-4.0\STLPORT" /I "..\..\Engine\sdk\inc" /I "..\shared" /I "..\shared\lang\en" /I "..\..\libs\regmgr" /I "..\..\libs\regmgr32" /I "..\..\libs\lith" /I "..\..\libs\butemgr" /I "..\..\libs\cryptmgr" /I "..\..\libs\stdlith" /I "..\libs" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D "_FINAL" /Yu"stdafx.h" /FD /c
# ADD RSC /l 0x409 /i ".\\" /i "res\lang\en" /i "..\shared" /i "..\shared\lang\en" /i "..\built\retail\ServerApp" /d "NDEBUG" /d "_REG" /d "_AFXDLL"
LINK32=link.exe
# ADD LINK32 winmm.lib /nologo /subsystem:windows /machine:I386 /out:"..\Built\Final\ServerApp\TRONSrv.exe"

!ENDIF 

# Begin Target

# Name "ServerApp - Win32 Final"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;rc;lib"
# Begin Source File

SOURCE=..\Shared\CommonUtilities.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DebugNew.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\GameAlloc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Engine\sdk\inc\ltmodule.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\Libs\GameSpy\nonport.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\Shared\NetDefs.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ParsedMsg.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ProfileUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ScmdConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\ScmdConsoleDriver_ServerApp.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectConfigDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerApp.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\Shared\TRON\TronVersionMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\VersionMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Libs\GameSpy\GameSpySDK\qr2\qr2.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\Libs\GameSpy\GameSpySDK\qr2\qr2regkeys.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\res\Lang\EN\ServerApp.rc
# End Source File
# Begin Source File

SOURCE=..\..\Engine\built\Final\server\server.lib
# End Source File
# Begin Source File

SOURCE=..\..\libs\built\Final\ButeMgrMfcDll\ButeMgrMfcDll.lib
# End Source File
# Begin Source File

SOURCE=..\..\libs\built\Final\CryptMgrMfcDll\CryptMgrMfcDll.lib
# End Source File
# Begin Source File

SOURCE=..\..\libs\built\Final\Lib_Lith\Lib_Lith.lib
# End Source File
# Begin Source File

SOURCE=..\..\libs\built\Final\Lib_StdLith\Lib_StdLith.lib
# End Source File
# Begin Source File

SOURCE=..\..\libs\built\Final\RegMgr\regmgr.lib
# End Source File
# Begin Source File

SOURCE=..\..\libs\built\Final\RegMgr32\regmgr32.lib
# End Source File
# End Group
# End Target
# End Project
