# Microsoft Developer Studio Project File - Name="ClientShellDLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ClientShellDLL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ClientShellDLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ClientShellDLL.mak" CFG="ClientShellDLL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ClientShellDLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Final" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Source (TO2)/ClientShellDLL", RQRFAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Built\Release\ClientShellDll\TRON"
# PROP Intermediate_Dir "..\..\Built\Release\ClientShellDll\TRON"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Shared\TRON" /I ".\\" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\ClientShellShared" /I "..\..\clientres\shared" /I "..\..\clientres\TRON" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\libs\ltguimgr" /I "..\..\..\libs\regmgr" /I "..\..\libs\wonapi" /I "..\..\libs\serverdir" /D "NDEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_NOLFBUILD" /D "NO_PRAGMA_LIBS" /D "NOPS2" /D "MEMTRACK_CLIENT" /Yu"stdafx.h" /FD /Zm1000 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winmm.lib wsock32.lib ole32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"LIBCMTD" /out:"..\..\Built\Release\ClientShellDll\TRON\CShell.dll"
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
TargetPath=\proj\TRON\source\Game\Built\Release\ClientShellDll\TRON\CShell.dll
InputPath=\proj\TRON\source\Game\Built\Release\ClientShellDll\TRON\CShell.dll
SOURCE="$(InputPath)"

"$(GAME_REZ_DIR)\cshell.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Rem IncrediBuild_OutputFile $(TargetPath) 
	echo copy $(TargetPath) $(GAME_REZ_DIR) 
	copy $(TargetPath) $(GAME_REZ_DIR) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Built\Debug\ClientShellDll\TRON"
# PROP Intermediate_Dir "..\..\Built\Debug\ClientShellDll\TRON"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\..\Shared\TRON" /I ".\\" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\ClientShellShared" /I "..\..\clientres\shared" /I "..\..\clientres\TRON" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\libs\ltguimgr" /I "..\..\..\libs\regmgr" /I "..\..\libs\wonapi" /I "..\..\libs\serverdir" /D "_DEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "NOPS2" /D "MEMTRACK_CLIENT" /FR /Yu"stdafx.h" /FD /Zm1000 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib ole32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\Built\Debug\ClientShellDll\TRON\CShell.dll" /pdbtype:sept
# SUBTRACT LINK32 /verbose /pdb:none
# Begin Custom Build
TargetPath=\proj\TRON\source\Game\Built\Debug\ClientShellDll\TRON\CShell.dll
InputPath=\proj\TRON\source\Game\Built\Debug\ClientShellDll\TRON\CShell.dll
# End Source File
# Begin Source File

SOURCE="$(InputPath)"

"$(GAME_REZ_DIR)\cshell.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Rem IncrediBuild_OutputFile $(TargetPath) 
	echo copy $(TargetPath) $(GAME_REZ_DIR) 
	copy $(TargetPath) $(GAME_REZ_DIR) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Final"
# PROP BASE Intermediate_Dir "Final"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Built\Final\ClientShellDll\TRON"
# PROP Intermediate_Dir "..\..\Built\Final\ClientShellDll\TRON"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\Shared\TRON" /I ".\\" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\ClientShellShared" /I "..\..\clientres\shared" /I "..\..\clientres\TRON" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\libs\ltguimgr" /I "..\..\..\libs\regmgr" /I "..\..\libs\wonapi" /I "..\..\libs\serverdir" /D "NDEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_NOLFBUILD" /D "NO_PRAGMA_LIBS" /D "NOPS2" /D "_FINAL" /Yu"stdafx.h" /FD /Zm1000 /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\Shared\TRON" /I ".\\" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\ClientShellShared" /I "..\..\clientres\shared" /I "..\..\clientres\TRON" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\libs\ltguimgr" /I "..\..\..\libs\regmgr" /I "..\..\libs\wonapi" /I "..\..\libs\serverdir" /D "NDEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_NOLFBUILD" /D "NO_PRAGMA_LIBS" /D "NOPS2" /D "_FINAL" /D "MEMTRACK_CLIENT" /Yu"stdafx.h" /FD /Zm1000 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ole32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"LIBCMTD" /out:"..\..\Built\Final\ClientShellDll\TRON\CShell.dll"
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 winmm.lib wsock32.lib ole32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"LIBCMTD" /out:"..\..\Built\Final\ClientShellDll\TRON\CShell.dll"
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
TargetPath=\proj\TRON\source\Game\Built\Final\ClientShellDll\TRON\CShell.dll
InputPath=\proj\TRON\source\Game\Built\Final\ClientShellDll\TRON\CShell.dll
# End Source File
# Begin Source File

SOURCE="$(InputPath)"

"$(GAME_REZ_DIR)\cshell.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Rem IncrediBuild_OutputFile $(TargetPath) 
	echo copy $(TargetPath) $(GAME_REZ_DIR) 
	copy $(TargetPath) $(GAME_REZ_DIR) 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "ClientShellDLL - Win32 Release"
# Name "ClientShellDLL - Win32 Debug"
# Name "ClientShellDLL - Win32 Final"
# Begin Group "Source"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\AdditiveCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ArcCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientLightCycleMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseScreenJoin.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponBall.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponBallLauncher.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponBlaster.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponClusterDisc.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponContinuousFireBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponDisc.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponDrunkenDims.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponEnergyClaw.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponLOL.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponPrankster.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSequencerDisc.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSuffusion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\darray.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\gcdkey\gcdkeyc.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\ghttp\ghttpBuffer.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\ghttp\ghttpCallbacks.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\ghttp\ghttpCommon.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\ghttp\ghttpConnection.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\ghttp\ghttpMain.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\ghttp\ghttpPost.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\ghttp\ghttpProcess.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\GlobalsInit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\hashtable.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\HUDArmor.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDBuildNoteCount.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDChooser.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDCrosshair.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDDamageDir.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDDiscScores.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDEnergyTransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDHealthEnergy.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDIOCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDLightCycleLives.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDLightCyclePowerupIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\LightCycleMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDPermissions.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDPings.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDProcedurals.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDProfiler.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDProgress.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDSecurityCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDSingleString.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDSubChooser.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDVersion.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDWeapons.cpp
# End Source File
# Begin Source File

SOURCE=.\IONodeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\LCHUDHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\LightCycleGameDataMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\LightCycleButeMgr.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\LightLine.cpp
# End Source File
# Begin Source File

SOURCE=.\LTGUIListItemCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuPieceModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Engine\sdk\inc\iltbaseclass.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Engine\sdk\inc\ltmodule.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\engine\sdk\inc\ltobjref.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Engine\sdk\inc\ltquatbase.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\md5c.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\nonport.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ProceduralCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\pt\ptMain.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\qr2\qr2regkeys.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\RatingBarCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\RatingMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\serverbrowsing\sb_crypt.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\serverbrowsing\sb_queryengine.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\serverbrowsing\sb_server.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\serverbrowsing\sb_serverbrowsing.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Libs\GameSpy\GameSpySDK\serverbrowsing\sb_serverlist.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ScreenAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenConfigure.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenControls.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEndMission.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenFailure.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenGame.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenJoinServerList.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenJoystick.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenKeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenLightCycleVictory.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMain.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMouse.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMulti.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenPerformance.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSave.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenServerInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSingle.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSubroutines.cpp
# End Source File
# Begin Source File

SOURCE=.\Stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\SubroutineDataMgr.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\SubroutineMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TronClientWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\TRONClientWeaponAllocator.cpp
# End Source File
# Begin Source File

SOURCE=.\TronGameClientShell.cpp
# End Source File
# Begin Source File

SOURCE=.\TronHUDMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TronInterfaceMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TronLayoutMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\TronMissionButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TronPlayerMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TRONPlayerStats.cpp
# End Source File
# Begin Source File

SOURCE=.\TRONScreenMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TronTargetMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\Tron\TronVersionMgr.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\AdditiveCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ArcCtrl.h
# End Source File
# Begin Source File

SOURCE=.\BaseScreenJoin.h
# End Source File
# Begin Source File

SOURCE=.\CheatMgr.h
# End Source File
# Begin Source File

SOURCE=.\ClientLightCycleMgr.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponBall.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponBallLauncher.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponBlaster.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponClusterDisc.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponContinuousFireBase.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponDisc.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponDrunkenDims.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponEnergyClaw.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponLOL.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponPrankster.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSequencerDisc.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSuffusion.h
# End Source File
# Begin Source File

SOURCE=.\HUDArmor.h
# End Source File
# Begin Source File

SOURCE=.\HUDBuildNoteCount.h
# End Source File
# Begin Source File

SOURCE=.\HUDChooser.h
# End Source File
# Begin Source File

SOURCE=.\HUDDamageDir.h
# End Source File
# Begin Source File

SOURCE=.\HUDDiscScores.h
# End Source File
# Begin Source File

SOURCE=.\HUDEnergyTransfer.h
# End Source File
# Begin Source File

SOURCE=.\HUDHealthEnergy.h
# End Source File
# Begin Source File

SOURCE=.\HUDIOCamera.h
# End Source File
# Begin Source File

SOURCE=.\HUDLightCycleLives.h
# End Source File
# Begin Source File

SOURCE=.\HUDLightCyclePowerupIcon.h
# End Source File
# Begin Source File

SOURCE=.\HUDPings.h
# End Source File
# Begin Source File

SOURCE=.\HUDProcedurals.h
# End Source File
# Begin Source File

SOURCE=.\HUDProfiler.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\LightCycleButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\LightCycleMgr.h
# End Source File
# Begin Source File

SOURCE=.\HUDProgress.h
# End Source File
# Begin Source File

SOURCE=.\HUDSecurityCamera.h
# End Source File
# Begin Source File

SOURCE=.\HUDSingleString.h
# End Source File
# Begin Source File

SOURCE=.\HUDSubChooser.h
# End Source File
# Begin Source File

SOURCE=.\HUDWeapon.h
# End Source File
# Begin Source File

SOURCE=.\IONodeMgr.h
# End Source File
# Begin Source File

SOURCE=.\LCHUDHelper.h
# End Source File
# Begin Source File

SOURCE=.\LightCycleGameDataMgr.h
# End Source File
# Begin Source File

SOURCE=.\LightLine.h
# End Source File
# Begin Source File

SOURCE=.\LTGUIListItemCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MenuPieceModel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Engine\sdk\inc\iltbaseclass.h
# End Source File
# Begin Source File

SOURCE=..\..\Libs\ServerDir\IServerDir.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Engine\sdk\inc\ltmodule.h
# End Source File
# Begin Source File

SOURCE=..\..\..\engine\sdk\inc\ltobjref.h
# End Source File
# Begin Source File

SOURCE=.\ProceduralCtrl.h
# End Source File
# Begin Source File

SOURCE=.\RatingBarCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ScreenAudio.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCommands.h
# End Source File
# Begin Source File

SOURCE=.\ScreenConfigure.h
# End Source File
# Begin Source File

SOURCE=.\ScreenControls.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDisplay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEndMission.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEnum.h
# End Source File
# Begin Source File

SOURCE=.\ScreenFailure.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGame.h
# End Source File
# Begin Source File

SOURCE=.\ScreenJoinServerList.h
# End Source File
# Begin Source File

SOURCE=.\ScreenJoystick.h
# End Source File
# Begin Source File

SOURCE=.\ScreenKeyboard.h
# End Source File
# Begin Source File

SOURCE=.\ScreenLightCycleVictory.h
# End Source File
# Begin Source File

SOURCE=.\ScreenLoad.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMain.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMouse.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMulti.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPerformance.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPlayer.h
# End Source File
# Begin Source File

SOURCE=.\ScreenProfile.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSave.h
# End Source File
# Begin Source File

SOURCE=.\ScreenServerInfo.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSingle.h
# End Source File
# Begin Source File

SOURCE=.\ScreenWeaponControls.h
# End Source File
# Begin Source File

SOURCE=.\Stdafx.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\SubroutineDataMgr.h
# End Source File
# Begin Source File

SOURCE=.\SubroutineMgr.h
# End Source File
# Begin Source File

SOURCE=.\TO2ScreenMgr.h
# End Source File
# Begin Source File

SOURCE=.\TronClientWeapon.h
# End Source File
# Begin Source File

SOURCE=.\TRONClientWeaponAllocator.h
# End Source File
# Begin Source File

SOURCE=.\TronGameClientShell.h
# End Source File
# Begin Source File

SOURCE=.\TronLayoutMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\TronMissionButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\TronPlayerMgr.h
# End Source File
# Begin Source File

SOURCE=.\TRONPlayerStats.h
# End Source File
# Begin Source File

SOURCE=.\TRONScreenMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\Tron\TronVersionMgr.h
# End Source File
# End Group
# Begin Group "libs_debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\built\Debug\ClientShellDLL\ClientShellShared\ClientShellShared.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\ButeMgr\ButeMgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\MFCStub\MFCStub.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\CryptMgr\cryptmgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\Lib_StdLith\Lib_StdLith.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\Lib_Lith\Lib_Lith.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Built\Debug\libs\LTGuiMgr\ltguimgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\RegMgr\regmgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "libs_release"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\built\Release\ClientShellDll\ClientShellShared\ClientShellShared.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\ButeMgr\ButeMgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\MFCStub\MFCStub.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\CryptMgr\cryptmgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\Lib_StdLith\Lib_StdLith.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\Lib_Lith\Lib_Lith.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Built\Release\libs\LTGuiMgr\ltguimgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\RegMgr\regmgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "libs_final"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\built\Final\ClientShellDll\ClientShellShared\ClientShellShared.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Final\MFCStub\MFCStub.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Final\CryptMgr\cryptmgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Final\Lib_StdLith\Lib_StdLith.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Final\Lib_Lith\Lib_Lith.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Built\Final\libs\LTGuiMgr\ltguimgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Final\RegMgr\regmgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Final\ButeMgr\ButeMgr.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# End Group
# Begin Group "libs_finaldemo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\built\FinalDemo\ClientShellDLL\ClientShellShared\ClientShellShared.lib
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "libs_debugfinaldemo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\built\DebugFinalDemo\ClientShellDLL\ClientShellShared\ClientShellShared.lib
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
