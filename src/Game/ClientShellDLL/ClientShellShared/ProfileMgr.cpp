// ----------------------------------------------------------------------- //
//
// MODULE  : ProfileMgr.cpp
//
// PURPOSE : Manages player profiles
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "profileMgr.h"
#include "clientutilities.h"
#include "commandids.h"
#include "interfacemgr.h"
#include "winutil.h"
#include "ClientResShared.h"
#include "GameClientShell.h"
#include "GameSettings.h"
#include "MenuMgr.h"
#include "ClientSaveLoadMgr.h"
#include "ClientMultiplayerMgr.h"
#include "VarTrack.h"
#include "CMoveMgr.h"
#include "MsgIds.h"
#include "VersionMgr.h"
#include "PlayerCamera.h"
#include "ClientSoundMgr.h"
#include "VehicleMgr.h"
#include "PlayerMgr.h"

extern CGameClientShell* g_pGameClientShell;

#include <Direct.h>			// For _rmdir
#include "dinput.h"
#include <set>
#include <IO.H>

VarTrack	g_vtMouseMinSensitivity;
VarTrack	g_vtMouseMaxSensitivity;
VarTrack	g_vtMouseMinInputRate;
VarTrack	g_vtMouseMaxInputRate;

// RETAIL: Tron builds the profile file names from these (0x10270818 and
// 0x10270808; Init 0x100ecc40, Save 0x100ee500)
static std::string s_sProfileDir = PROFILE_DIR;
static std::string s_sProfileExt = PROFILE_EXT;

// RETAIL: the light cycle progress section and its keys (0x101ea2e4)
static const char* const s_aLightCycleInfoKeys[] =
{
	"LightCycleInfo",
	"CompletedLightCycleCircuits",
	"UnlockedLightCycleBikes",
	"UnlockedLightCyclePowerUps",
	"UnlockedLightCycleCustomMaps",
};
const int kMaxLightCycleInfoItems = 40;


CProfileMgr* g_pProfileMgr = LTNULL;

// RETAIL: string table entries 752 and 753 ("TRUE" or "FALSE" by the
// language build; the developer build's CRes.dll has "FALSE") say whether
// the build has no gore and no bodies (LoadGameOptions 0x100ed8d0); names ours.
const int kNoGoreStringId = 752;
const int kNoBodiesStringId = 753;
const int kDefaultProfileNameStringId = 1008;

namespace
{
    uint32 devices[3] =
	{
		DEVICETYPE_KEYBOARD,
		DEVICETYPE_MOUSE,
		DEVICETYPE_JOYSTICK
	};

	char strDeviceName[3][256] =
	{
		"","",""
	};

	char szWheelUp[32] = "";
	char szWheelDown[32] = "";

	char szJoyLookUp[32] = "";
	char szJoyLookDown[32] = "";
	char szJoyForward[32] = "";
	char szJoyBackward[32] = "";
	char szJoyLeft[32] = "";
	char szJoyRight[32] = "";
	char szJoyStrafeLeft[32] = "";
	char szJoyStrafeRight[32] = "";


	char szDefProfileName[16] = "";
}

	// RETAIL: s_aAttName is 100 bytes (LoadControls 0x100ed180) and comes
	// first (0x10270740; s_aTagName at 0x102707a4)
	char s_aAttName[100];
	char s_aTagName[30];

// RETAIL: Tron's command table (CShell.dll 0x1022af80, 35 commands and the
// unassigned entry). The string ids are Tron's string table's (its control
// names from 1200, its action strings from 1300, its own from 2001/2101);
// ClientResShared.h still has NOLF2's values for the names, so the ids are
// given as numbers here.
CommandData g_CommandArray[] =
{
	{ 1200,	COMMAND_ID_FORWARD,		1300,	COM_MOVE },
	{ 1201,	COMMAND_ID_REVERSE,		1301,	COM_MOVE },
	{ 1205,	COMMAND_ID_STRAFE_LEFT,		1305,	COM_MOVE },
	{ 1206,	COMMAND_ID_STRAFE_RIGHT,	1306,	COM_MOVE },
	{ 1247,	COMMAND_ID_LEAN_LEFT,		1337,	COM_MOVE },
	{ 1248,	COMMAND_ID_LEAN_RIGHT,		1338,	COM_MOVE },
	{ 1202,	COMMAND_ID_LEFT,		1302,	COM_MOVE },
	{ 1203,	COMMAND_ID_RIGHT,		1303,	COM_MOVE },
	{ 1204,	COMMAND_ID_STRAFE,		1304,	COM_MOVE },
	{ 1211,	COMMAND_ID_JUMP,		1311,	COM_MOVE },
	{ 1213,	COMMAND_ID_DUCK,		1313,	COM_MOVE },
	{ 1249,	COMMAND_ID_DUCKLOCK,		1328,	COM_MOVE },
	{ 1208,	COMMAND_ID_RUN,		1308,	COM_MOVE },
	{ 1228,	COMMAND_ID_RUNLOCK,		1327,	COM_MOVE },
	{ 1207,	COMMAND_ID_TURNAROUND,		1307,	COM_MOVE },
	{ 1209,	COMMAND_ID_FIRING,		1309,	COM_INV },
	{ 2001,	COMMAND_ID_DEFEND,		2101,	COM_INV },
	{ 2002,	COMMAND_ID_DEFEND_MODIFIER,	2102,	COM_INV },
	{ 1210,	COMMAND_ID_ACTIVATE,		1310,	COM_INV },
	{ 1214,	COMMAND_ID_NEXT_WEAPON,		1314,	COM_INV },
	{ 1215,	COMMAND_ID_PREV_WEAPON,		1315,	COM_INV },
	{ 1221,	COMMAND_ID_HOLSTER,		1321,	COM_INV },
	{ 1216,	COMMAND_ID_LOOKUP,		1316,	COM_VIEW },
	{ 1217,	COMMAND_ID_LOOKDOWN,		1317,	COM_VIEW },
	{ 1218,	COMMAND_ID_CENTERVIEW,		1318,	COM_VIEW },
	{ 1239,	COMMAND_ID_ZOOM_IN,		1331,	COM_VIEW },
	{ 1240,	COMMAND_ID_ZOOM_OUT,		1332,	COM_VIEW },
	{ 1242,	COMMAND_ID_MANUAL_LC_CAM,	1333,	COM_VIEW },
	{ 1253,	COMMAND_ID_TOGGLE_LC_CAMERA,	1335,	COM_VIEW },
	{ 2003,	COMMAND_ID_TOGGLE_PROGRESS,	2103,	COM_MISC },
	{ 2004,	COMMAND_ID_TOGGLE_MATCHES,	2104,	COM_MISC },
	{ 1243,	COMMAND_ID_NEXT_UTILITY,	1342,	COM_MISC },
	{ 1245,	COMMAND_ID_UTILITY_CHOOSER,	1343,	COM_MISC },
	{ 1223,	COMMAND_ID_MESSAGE,		1323,	COM_MISC },
	{ 1224,	COMMAND_ID_SEND_TEAM_MESSAGE,	1344,	COM_MISC },
	{ 1241,	COMMAND_ID_UNASSIGNED,		1329,	COM_MISC }
};

const int g_kNumCommands = sizeof(g_CommandArray) / sizeof(g_CommandArray[0]) - 1;

//Called to write out the display.cfg file which holds data for restoring the video
//mode the next time the game is run
void SaveDisplaySettings()
{
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-31, CL-34: the window mode and the antialiasing too, for
	// the launcher (which reads display.cfg) and the next run
	const char* pszValsToSave[] = {	"ScreenWidth",
									"ScreenHeight",
									"BitDepth",
									"HardwareCursor",
									"VSyncOnFlip",
									"GammaR",
									"GammaG",
									"GammaB",
									"Windowed",
									"WindowMode",
									"AntiAliasFSOverSample"
								};
#else
	//save out the display configuration
	const char* pszValsToSave[] = {	"ScreenWidth",
									"ScreenHeight",
									"BitDepth",
									"HardwareCursor",
									"VSyncOnFlip",
									"GammaR",
									"GammaG",
									"GammaB"
								};
#endif

	uint32 nNumVals = sizeof(pszValsToSave) / sizeof(pszValsToSave[0]);
	g_pLTClient->WriteConfigFileFields("display.cfg", nNumVals, pszValsToSave);
}

#ifdef TRON20SRC_MODERN
// MODERN.md CL-31, CL-34: set by the Display screen when a new window mode
// or antialiasing needs the renderer restarted (ApplyDisplay)
bool g_bModernDisplayRestart = false;
#endif

//-------------------------------------------------------------------------------------------
// GetCommandName
//
// Retrieves the command name from a command number
// Arguments:
//		nCommand - command number
// Return:
//		String containing the name of the action
//-------------------------------------------------------------------------------------------

char* GetCommandName(int nCommand)
{
	static char buffer[128];

    uint32 nStringID = 0;

	for (int i=0; i < g_kNumCommands; i++)
	{
		if (g_CommandArray[i].nCommandID == nCommand)
		{
			nStringID = g_CommandArray[i].nActionStringID;
			break;
		}
	}

	if (nStringID)
	{
		LoadString(nStringID,buffer,sizeof(buffer));
	}
	else
	{
		SAFE_STRCPY(buffer, "Error in CommandName()!");
	}

	return buffer;
}

const CommandData* GetCommandData(int nIndex)
{
	if (nIndex < 0 || nIndex >= g_kNumCommands)
		return LTNULL;
	return &g_CommandArray[nIndex];
}


CUserProfile::CUserProfile()
{
	// RETAIL: 0x100ec8a0
	m_sName			= "";		
	m_bInitted		= LTFALSE;
	m_pCryptKey		= LTNULL;

	//controls
    m_bInvertY = LTFALSE;
	m_bInvertLCMouse = LTFALSE;
    m_bMouseLook = LTFALSE;
	m_nInputRate = 0;
	m_nSensitivity = 0;

	m_nLCCamLag = 5;
	m_nLCCamSpeed = 2;
	m_bLCDialog = LTTRUE;
	m_fLCCamZoom = 0.0f;
	m_fLCCamPitch = 0.0f;
	m_fLCCamYaw = 0.0f;
	m_fLCPlayerPitch = 0.0f;

	m_bAutoCenter = LTFALSE;
	m_nNormalTurn = 15;
	m_nFastTurn = 23;
	m_nLookUp = 25;

	m_nVehicleTurn	= 100;

	m_bUseJoystick = LTFALSE;
	memset(m_nAxis,0,sizeof(m_nAxis));

	//game options
	m_nDifficulty = 1;
    m_nSubtitles  = 0;
	m_bGoreAllowed = true;
	m_bBodiesAllowed = true;
    m_bGore = false;
	m_bLCThirdPersonCam = true;
	m_bSkipSubIntro = false;
	m_bLCManualCam = false;
	m_bAlwaysRun = LTTRUE;
	m_nLayout = 0;
	m_nHeadBob = 0;
	m_nWeaponSway = 0;
	m_nMsgDur = 0;
	m_bLoadScreenTips = LTFALSE;
	m_bAlwaysShowToolTip = true;
	m_bMouseClosesPopup = LTFALSE;
	m_bDiscAutoAim = LTFALSE;

	m_bCrosshair = LTTRUE;
	m_CrosshairR = 0x1F;
	m_CrosshairG = 0xFF;
	m_CrosshairB = 0xFF;
	m_CrosshairA = 0xBF;
	m_nStyle = 0;
    m_bDynamic = 1;


	//sound
	m_nSoundVolume=SOUND_DEFAULT_VOL;
	m_nMusicVolume=MUSIC_DEFAULT_VOL;
    m_bSoundQuality=LTFALSE;
    m_bMusicQuality=LTFALSE;
	m_bEAXEnabled=LTFALSE;


	//display
	m_bHardwareCursor = LTTRUE;
	m_bVSync = LTTRUE;
	m_nScreenWidth = 0;
	m_nScreenHeight = 0;
	m_nScreenDepth = 0;

	//performance
	memset(&m_sPerformance,0,sizeof(m_sPerformance));

	//multi player
	m_nCPPlayerModel = 0;
	m_nDMPlayerModel = 0;
	m_nPlayerColor = 0;
	m_nBandwidthClient = 3;
	m_nBandwidthClientCustom = g_BandwidthClient[m_nBandwidthClient];
	m_nDifficulty = 1;

	//misc
	m_szVehicleColor[0] = 0;
	m_nLightCycleStyle = 0;
	m_nLightCycleShade = 0;
}

static bool CreatePlayerGuid( char* pszPlayerGuid, int nSize )
{
	// Check inputs.
	if( !pszPlayerGuid )
	{
		ASSERT( false );
		return false;
	}

	// Clear out the result.
	pszPlayerGuid[0] = 0;

	// Create the guid.
	GUID guid;
	if( !SUCCEEDED( CoCreateGuid( &guid )))
		return false;

	// Write the guid into the string.
	_snprintf( pszPlayerGuid, nSize, "%X-%X-%X-%X%X-%X%X%X%X%X%X", guid.Data1, guid.Data2, 
		guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], 
		guid.Data4[5], guid.Data4[6], guid.Data4[7] );
	pszPlayerGuid[nSize-1] = 0;

	if( !pszPlayerGuid[0] )
		return false;

	return true;
}

LTBOOL CUserProfile::Init(const std::string& profileName, LTBOOL bCreate)
{
	// RETAIL: 0x100ecc40
	std::string fn = s_sProfileDir + profileName + s_sProfileExt;
	std::string dfn = s_sProfileDir + "default.prf";

	g_vtMouseMinSensitivity.Init(g_pLTClient, "MouseSensitivityMin", NULL, 0.0);
	g_vtMouseMaxSensitivity.Init(g_pLTClient, "MouseSensitivityMax", NULL, 10.0);
	g_vtMouseMinInputRate.Init(g_pLTClient, "MouseInputRateMin", NULL, 0.0);
	g_vtMouseMaxInputRate.Init(g_pLTClient, "MouseInputRateMax", NULL, 40.0);

	LTBOOL bRet = LTFALSE;

	if (m_bInitted)
		m_buteMgr.Term();

	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(fn.c_str(), m_pCryptKey);
	}
	else
	{
		bRet = m_buteMgr.Parse(fn.c_str());
	}

	// Handle case where we had a previous profile file.
	if (bRet)
	{
		m_sName = profileName;
	}
	// Handle brand new profile.
	else
	{
		// No previous profile and not told to create one.
		if (!bCreate)
			return LTFALSE;

		m_sName = profileName;

		if (m_pCryptKey)
		{
			bRet = m_buteMgr.Parse(dfn.c_str(), m_pCryptKey);
		}
		else
		{
			bRet = m_buteMgr.Parse(dfn.c_str());
		}

		if (!bRet) 
			return LTFALSE;
	}
	
	// Since we changed our profile, we'll need to re-init the saveloadmgr, which
	// uses the profile name.
	if( !g_pClientSaveLoadMgr->Init( m_sName.c_str( ), false ))
		return LTFALSE;

	LoadControls();
	LoadMultiplayer();
	LoadGameOptions();
	LoadSound(true);
	LoadLightCycleInfo();
	LoadMisc();

	// A performance level from the command line replaces the profile's.
	char* pszPerformance = GetConsoleTempString("SetPerformanceLevel","");
	if (pszPerformance[0])
	{
		g_pPerformanceMgr->SetPerformanceCfg(pszPerformance);
		g_pPerformanceMgr->GetPerformanceOptions(&m_sPerformance);
		SavePerformance();
		WriteConsoleString("SetPerformanceLevel","");
	}
	else if (m_buteMgr.Exist("Settings"))
	{
		LoadPerformance(false);//defer applying now, because ApplyPerformance() will do it below
	}
	else
	{
		g_pPerformanceMgr->GetPerformanceOptions(&m_sPerformance);
	}

	ApplyPerformance(false);


	m_bInitted = LTTRUE;


	return LTTRUE;

}

void CUserProfile::LoadControls()
{

	char temp[256];
	memset(m_bindings,0,sizeof(m_bindings));
	m_nNumBindings = 0;


	strcpy(s_aTagName,"Bindings");
	for (int c = 0; c < g_kNumCommands; c++)
	{

		int n = m_nNumBindings;
		m_nNumBindings++;

		m_bindings[n].nStringID = g_CommandArray[c].nStringID;
		m_bindings[n].nAction = g_CommandArray[c].nActionStringID;

		for( int nDev = 0; nDev < NUM_DEVICES; nDev++ )
		{
			m_bindings[n].nDeviceObjectId[nDev] = 0;
		}

		strcpy(m_bindings[n].strTriggerName[0],"");

		if (g_CommandArray[c].nCommandID < 0)
		{
			strcpy(m_bindings[n].strRealName[0]," ");
		}
		else
		{
			LoadString(m_bindings[n].nAction,s_aAttName,sizeof(s_aAttName));
			m_buteMgr.GetString(s_aTagName, s_aAttName,temp,sizeof(temp));

			char *pTok = strtok(temp,"@");
			if (pTok)
				strcpy(m_bindings[n].strRealName[0],pTok);

			pTok = strtok(LTNULL,"@");
			if (pTok)
				strcpy(m_bindings[n].strRealName[1],pTok);

			pTok = strtok(LTNULL,"@");
			if (pTok)
				strcpy(m_bindings[n].strRealName[2],pTok);
		}

	}

	strcpy(s_aTagName,"Controls");
    m_bInvertY = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"InvertMouse",0);
	m_bInvertLCMouse = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"InvertLCMouse",0);
    m_bMouseLook = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"MouseLook",1);
	m_nInputRate = m_buteMgr.GetInt(s_aTagName,"InputRate",12);
	m_nSensitivity = m_buteMgr.GetInt(s_aTagName,"Sensitivity",5);

	m_nNormalTurn	= m_buteMgr.GetInt(s_aTagName,"NormalTurn",15);
	m_nFastTurn		= m_buteMgr.GetInt(s_aTagName,"FastTurn",23);
	m_nLookUp		= m_buteMgr.GetInt(s_aTagName,"LookUp",25);
	m_bAutoCenter	= (LTBOOL)m_buteMgr.GetInt(s_aTagName,"AutoCenter",0);

	m_nVehicleTurn = m_buteMgr.GetInt(s_aTagName,"VehicleTurnRate",100);

	m_bUseJoystick = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"UseJoystick",0);

	for (int a = 0; a < g_pProfileMgr->GetNumAxis(); a++)
	{
		CDeviceAxisData *pAxisData = g_pProfileMgr->GetAxisData(a);
		uint8 nDef = 0;

		if (pAxisData->m_nType == CONTROLTYPE_YAXIS || pAxisData->m_nType == CONTROLTYPE_RYAXIS)
		{
			nDef = 2;
		}

		char szAxis[8] = "";
		sprintf(szAxis,"Axis%d",a);
		m_nAxis[a] = (uint8)m_buteMgr.GetInt(s_aTagName,szAxis,nDef);
		
	}

	for (a = 0; a < g_pProfileMgr->GetNumPOV(); a++)
	{
		CDevicePOVData *pPOVData = g_pProfileMgr->GetPOVData(a);
		char szPOV[8] = "";
		sprintf(szPOV,"POV%d",a);
		m_nPOV[a] = (uint8)m_buteMgr.GetInt(s_aTagName,szPOV,0);
		
	}

	ApplyControls();
	ApplyBindings();
	ApplyMouse();
	ApplyKeyboard();
	ApplyJoystick();

}


void CUserProfile::LoadMultiplayer()
{
	strcpy(s_aTagName,"Multiplayer");

	char szStr[64];
	m_buteMgr.GetString(s_aTagName, "PlayerName",m_sName.c_str(),szStr,sizeof(szStr));
	m_sPlayerName = szStr;

	m_buteMgr.GetString( s_aTagName, "Guid", "", szStr, sizeof( szStr ));
	// If they didn't have a guid, create one.
	if( !m_buteMgr.Success( ))
	{
		CreatePlayerGuid( szStr, ARRAY_LEN( szStr ));
	}
	m_sPlayerGuid = szStr;

	// RETAIL: the host option set's name, the profile's by default
	// (0x100ed6fd); the sets are CServerOptionMgr's
	m_buteMgr.GetString(s_aTagName, "ServerConfig", m_sName.c_str(), szStr, sizeof(szStr));
	m_sServerOptions = szStr;

	m_nCPPlayerModel = (uint8)m_buteMgr.GetInt(s_aTagName,"PlayerModel",0);
	// RETAIL: 0x100ed5a0 -- Tron keeps a player colour, no skills
	m_nPlayerColor = m_nDMPlayerModel = (uint8)m_buteMgr.GetInt(s_aTagName,"PlayerColor",0);

	m_nBandwidthClient = (uint8)m_buteMgr.GetInt(s_aTagName,"BandwidthClient",3);
	m_nBandwidthClientCustom = (uint16)m_buteMgr.GetInt(s_aTagName,"BandwidthClientCustom",g_BandwidthClient[3]);

	ApplyMultiplayer();
}


void CUserProfile::LoadGameOptions()
{
	strcpy(s_aTagName,"Game");

	// RETAIL: 0x100ed8d0 -- the string table says whether this build has
	// no gore or no bodies ("TRUE").
	char szAllowed[8];
	FormatString(kNoGoreStringId,szAllowed,sizeof(szAllowed));
	m_bGoreAllowed = (stricmp(szAllowed,"TRUE") != 0);
	FormatString(kNoBodiesStringId,szAllowed,sizeof(szAllowed));
	m_bBodiesAllowed = ((stricmp(szAllowed,"TRUE") != 0) && !GetConsoleInt("NoBodies",0));

	m_nDifficulty = m_buteMgr.GetInt(s_aTagName,"Difficulty",2);
    m_nSubtitles  = (uint8)m_buteMgr.GetInt(s_aTagName,"Subtitles",0);
    m_bGore = (m_bGoreAllowed && (m_buteMgr.GetInt(s_aTagName,"Gore",0) > 0));
	m_bLCThirdPersonCam = !!m_buteMgr.GetInt(s_aTagName,"LCThirdPersonCam",1);
	m_bSkipSubIntro = !!m_buteMgr.GetInt(s_aTagName,"SkipSubIntro",0);
	m_bLCManualCam = !!m_buteMgr.GetInt(s_aTagName,"LCManualCam",1);
	m_bAlwaysRun = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"AlwaysRun",1);
	m_nLayout = m_buteMgr.GetInt(s_aTagName,"HUDLayout",0);
	m_nHeadBob = m_buteMgr.GetInt(s_aTagName,"HeadBob",0);
	m_nWeaponSway = m_buteMgr.GetInt(s_aTagName,"WeaponSway",0);
	m_nMsgDur = m_buteMgr.GetInt(s_aTagName,"PickupMessage",0);
	m_bLoadScreenTips = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"LoadScreenTips",1);
	m_bAlwaysShowToolTip = !!m_buteMgr.GetInt(s_aTagName,"AlwaysShowToolTip",1);
	m_bMouseClosesPopup = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"MouseClosesPopup",0);
	m_bDiscAutoAim = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"DiscAutoAim",0);

	m_bCrosshair = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"UseCrosshair",1);
	m_CrosshairR = (uint32)m_buteMgr.GetInt(s_aTagName,"CrosshairRed",0x1F);
	m_CrosshairG = (uint32)m_buteMgr.GetInt(s_aTagName,"CrosshairGreen",0xFF);
	m_CrosshairB = (uint32)m_buteMgr.GetInt(s_aTagName,"CrosshairBlue",0xFF);
	m_CrosshairA = (uint8)m_buteMgr.GetInt(s_aTagName,"CrosshairAlpha",0xBF);
	m_nStyle = m_buteMgr.GetInt(s_aTagName,"CrosshairStyle",0);
    m_bDynamic = m_buteMgr.GetInt(s_aTagName,"CrosshairDynamic",1);

	m_nLCCamSpeed = m_buteMgr.GetInt(s_aTagName,"LCCamSpeed",2);
	m_nLCCamLag = m_buteMgr.GetInt(s_aTagName,"LCCamLag",5);
	m_bLCDialog = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"LCDialog",1);
	m_fLCCamZoom = m_buteMgr.GetFloat(s_aTagName,"LCCamZoom",1000.0f);
	m_fLCCamPitch = m_buteMgr.GetFloat(s_aTagName,"LCCamPitch",55.0f);
	m_fLCCamYaw = m_buteMgr.GetFloat(s_aTagName,"LCCamYaw",94.0f);
	m_fLCPlayerPitch = m_buteMgr.GetFloat(s_aTagName,"LCPlayerPitch",0.0f);

	ApplyGameOptions();
	ApplyCrosshair();

}

void CUserProfile::LoadSound(bool bApply)
{
	strcpy(s_aTagName,"Sound");


	m_nSoundVolume = m_buteMgr.GetInt(s_aTagName,"soundvolume",SOUND_DEFAULT_VOL);
	m_nMusicVolume = m_buteMgr.GetInt(s_aTagName,"musicvolume",MUSIC_DEFAULT_VOL);
	m_fWeaponsSoundMultiplier = m_buteMgr.GetFloat( s_aTagName, "weaponssoundmultiplier", WEAPONS_DEFAULT_MULTIPLIER );
	m_fSpeechSoundMultiplier = m_buteMgr.GetFloat( s_aTagName, "speechsoundmultiplier", SPEECH_DEFAULT_MULTIPLIER );
	m_fDefaultSoundMultiplier = m_buteMgr.GetFloat( s_aTagName, "defaultsoundmultiplier", DEFAULT_DEFAULT_MULTIPLIER );

	m_bSoundQuality = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"sound16bit",0);
	m_bMusicQuality = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"MusicQuality",0);
	m_bEAXEnabled = (LTBOOL)m_buteMgr.GetInt(s_aTagName,"eaxenabled",0);

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
	if (!(dwAdvancedOptions & AO_MUSIC))
		m_nMusicVolume = MUSIC_MIN_VOL;

	if (bApply)
		ApplySound();

}

void CUserProfile::LoadPerformance(bool bApply)
{

	g_pPerformanceMgr->LoadPerformanceOptions(m_buteMgr,&m_sPerformance);

	if (bApply)
		ApplyPerformance(false);

}

// RETAIL: 0x100eddc0 (name ours)
void CUserProfile::LoadMisc()
{
	strcpy(s_aTagName,"Misc");

	m_buteMgr.GetString(s_aTagName,"VehicleColor","LightCycleBlue",m_szVehicleColor,sizeof(m_szVehicleColor));

	char szStr[64];
	m_buteMgr.GetString(s_aTagName,"LightCycleConfig",m_sName.c_str(),szStr,sizeof(szStr));
	m_sLightCycleConfig = szStr;

	m_buteMgr.GetString(s_aTagName,"LightCyclePlayerName",m_sLightCycleName.c_str(),szStr,sizeof(szStr));
	m_sLightCycleName = szStr;

	m_nLightCycleStyle = m_buteMgr.GetInt(s_aTagName,"CycleType");
	m_nLightCycleShade = m_buteMgr.GetInt(s_aTagName,"CycleShade");

	ApplyMisc();
}

// RETAIL: 0x100edfc0 (name ours)
void CUserProfile::LoadLightCycleInfo()
{
	const char* pszTag = s_aLightCycleInfoKeys[0];
	int i;

	m_CompletedCircuits.clear();
	m_UnlockedCycleColors.clear();
	m_UnlockedPowerups.clear();
	m_UnlockedMaps.clear();

	for (i = 0; i < kMaxLightCycleInfoItems; i++)
	{
		char szKey[32];
		sprintf(szKey,"%s%d",s_aLightCycleInfoKeys[1],i);

		char szValue[128];
		szValue[0] = 0;
		if (m_buteMgr.Exist(pszTag,szKey))
		{
			m_buteMgr.GetString(pszTag,szKey,szValue,sizeof(szValue));
		}

		if (szValue[0])
		{
			m_CompletedCircuits.push_back(std::string(szValue));
		}
	}

	for (i = 0; i < kMaxLightCycleInfoItems; i++)
	{
		char szKey[32];
		sprintf(szKey,"%s%d",s_aLightCycleInfoKeys[2],i);

		char szValue[128];
		szValue[0] = 0;
		if (m_buteMgr.Exist(pszTag,szKey))
		{
			m_buteMgr.GetString(pszTag,szKey,szValue,sizeof(szValue));
		}

		if (szValue[0])
		{
			m_UnlockedCycleColors.push_back(std::string(szValue));
		}
	}

	for (i = 0; i < kMaxLightCycleInfoItems; i++)
	{
		char szKey[32];
		sprintf(szKey,"%s%d",s_aLightCycleInfoKeys[3],i);

		char szValue[128];
		szValue[0] = 0;
		if (m_buteMgr.Exist(pszTag,szKey))
		{
			m_buteMgr.GetString(pszTag,szKey,szValue,sizeof(szValue));
		}

		if (szValue[0])
		{
			m_UnlockedPowerups.push_back(std::string(szValue));
		}
	}

	for (i = 0; i < kMaxLightCycleInfoItems; i++)
	{
		char szKey[32];
		sprintf(szKey,"%s%d",s_aLightCycleInfoKeys[4],i);

		char szValue[128];
		szValue[0] = 0;
		if (m_buteMgr.Exist(pszTag,szKey))
		{
			m_buteMgr.GetString(pszTag,szKey,szValue,sizeof(szValue));
		}

		if (szValue[0])
		{
			m_UnlockedMaps.push_back(std::string(szValue));
		}
	}
}


void CUserProfile::Save()
{
	// RETAIL: 0x100ee500
	std::string fn = s_sProfileDir + m_sName + s_sProfileExt;

	SaveControls();
	SaveMultiplayer();
	SaveGameOptions();
	SaveSound();
	SavePerformance();
	SaveLightCycleInfo();
	SaveMisc();

	m_buteMgr.Save(fn.c_str());

	// Reread what we wrote.
	m_buteMgr.Term();
	m_buteMgr.Init();
	if (m_pCryptKey)
	{
		m_buteMgr.Parse(fn.c_str(), m_pCryptKey);
	}
	else
	{
		m_buteMgr.Parse(fn.c_str());
	}
}

void CUserProfile::SaveControls()
{
	//************************ save bindings
	strcpy(s_aTagName,"Bindings");
	for (int c = 0; c < g_kNumCommands; c++)
	{
		if ( g_CommandArray[c].nCommandID < 0 )
			continue;

		CBindingData *pData = FindBinding(c);
		if (pData)
		{
			LoadString(pData->nAction,s_aAttName,sizeof(s_aAttName));
			
			char szStr[128] = "";
			for (int d = 0; d < 3; d++)
			{
				if (pData->strRealName[d] && pData->strRealName[d][0])
				{
					strcat(szStr,pData->strRealName[d]);
				}
				else
				{
					strcat(szStr," ");
				}
				if (d < 2)
					strcat(szStr,"@");
			}
	
			m_buteMgr.SetString(s_aTagName, s_aAttName, szStr);

		}
	}

	//************************ save controls
	strcpy(s_aTagName,"Controls");

	//save mouse
	m_buteMgr.SetInt(s_aTagName,"InvertMouse",(int)m_bInvertY);
	m_buteMgr.SetInt(s_aTagName,"InvertLCMouse",(int)m_bInvertLCMouse);
	m_buteMgr.SetInt(s_aTagName,"MouseLook",(int)m_bMouseLook);
    m_buteMgr.SetInt(s_aTagName,"InputRate",m_nInputRate);
    m_buteMgr.SetInt(s_aTagName,"Sensitivity",m_nSensitivity);
	m_buteMgr.SetInt(s_aTagName,"VehicleTurnRate",m_nVehicleTurn);


	m_buteMgr.SetInt(s_aTagName,"NormalTurn",m_nNormalTurn);
	m_buteMgr.SetInt(s_aTagName,"FastTurn",m_nFastTurn);
	m_buteMgr.SetInt(s_aTagName,"LookUp",m_nLookUp);
	m_buteMgr.SetInt(s_aTagName,"AutoCenter",(int)m_bAutoCenter);

	m_buteMgr.SetInt(s_aTagName,"UseJoystick",(int)m_bUseJoystick);

	for (int a = 0; a < g_pProfileMgr->GetNumAxis(); a++)
	{
		char szAxis[8] = "";
		sprintf(szAxis,"Axis%d",a);
		m_buteMgr.SetInt(s_aTagName,szAxis,(int)m_nAxis[a]);
		
	}

}


void CUserProfile::SaveMultiplayer()
{
	strcpy(s_aTagName,"Multiplayer");
	m_buteMgr.SetString(s_aTagName, "PlayerName",m_sPlayerName.c_str());
	m_buteMgr.SetString( s_aTagName, "Guid", m_sPlayerGuid.c_str( ));
	m_buteMgr.SetString( s_aTagName, "ServerConfig", m_sServerOptions.c_str( ));
	m_buteMgr.SetInt(s_aTagName,"PlayerModel",(int)m_nCPPlayerModel);
	m_buteMgr.SetInt(s_aTagName,"PlayerColor",(int)m_nDMPlayerModel);
	m_buteMgr.SetInt(s_aTagName,"BandwidthClient",(int)m_nBandwidthClient);
	m_buteMgr.SetInt(s_aTagName,"BandwidthClientCustom",(int)m_nBandwidthClientCustom);
}

void CUserProfile::SaveGameOptions()
{
	strcpy(s_aTagName,"Game");
	m_buteMgr.SetInt(s_aTagName,"Difficulty",m_nDifficulty);
    m_buteMgr.SetInt(s_aTagName,"Subtitles",m_nSubtitles);
    m_buteMgr.SetInt(s_aTagName,"Gore",(int)(m_bGoreAllowed && m_bGore));
	m_buteMgr.SetInt(s_aTagName,"LCThirdPersonCam",(int)m_bLCThirdPersonCam);
	m_buteMgr.SetInt(s_aTagName,"SkipSubIntro",(int)m_bSkipSubIntro);
	m_buteMgr.SetInt(s_aTagName,"LCManualCam",(int)m_bLCManualCam);
	m_buteMgr.SetInt(s_aTagName,"AlwaysRun",(int)m_bAlwaysRun);
	m_buteMgr.SetInt(s_aTagName,"HUDLayout",m_nLayout);
	m_buteMgr.SetInt(s_aTagName,"HeadBob",m_nHeadBob);
	m_buteMgr.SetInt(s_aTagName,"WeaponSway",m_nWeaponSway);
	m_buteMgr.SetInt(s_aTagName,"PickupMessage",m_nMsgDur);
	m_buteMgr.SetInt(s_aTagName,"LoadScreenTips",(int)m_bLoadScreenTips);
	m_buteMgr.SetInt(s_aTagName,"AlwaysShowToolTip",(int)m_bAlwaysShowToolTip);
	m_buteMgr.SetInt(s_aTagName,"MouseClosesPopup",(int)m_bMouseClosesPopup);
	m_buteMgr.SetInt(s_aTagName,"DiscAutoAim",(int)m_bDiscAutoAim);

	m_buteMgr.SetInt(s_aTagName,"UseCrosshair",m_bCrosshair);
	m_buteMgr.SetInt(s_aTagName,"CrosshairRed",(int)m_CrosshairR);
	m_buteMgr.SetInt(s_aTagName,"CrosshairGreen",(int)m_CrosshairG);
	m_buteMgr.SetInt(s_aTagName,"CrosshairBlue",(int)m_CrosshairB);
	m_buteMgr.SetInt(s_aTagName,"CrosshairAlpha",(int)m_CrosshairA);
	m_buteMgr.SetInt(s_aTagName,"CrosshairStyle",m_nStyle);
    m_buteMgr.SetInt(s_aTagName,"CrosshairDynamic",m_bDynamic);

	m_buteMgr.SetInt(s_aTagName,"LCCamSpeed",m_nLCCamSpeed);
	m_buteMgr.SetInt(s_aTagName,"LCCamLag",m_nLCCamLag);
	m_buteMgr.SetInt(s_aTagName,"LCDialog",(int)m_bLCDialog);
	m_buteMgr.SetFloat(s_aTagName,"LCCamZoom",m_fLCCamZoom);
	m_buteMgr.SetFloat(s_aTagName,"LCCamPitch",m_fLCCamPitch);
	m_buteMgr.SetFloat(s_aTagName,"LCCamYaw",m_fLCCamYaw);
	m_buteMgr.SetFloat(s_aTagName,"LCPlayerPitch",m_fLCPlayerPitch);
}


void CUserProfile::SaveSound()
{
	strcpy(s_aTagName,"Sound");

	m_buteMgr.SetInt(s_aTagName,"soundvolume",m_nSoundVolume);
	m_buteMgr.SetInt(s_aTagName,"musicvolume",m_nMusicVolume);
	m_buteMgr.SetFloat(s_aTagName,"defaultsoundmultiplier",m_fDefaultSoundMultiplier);
	m_buteMgr.SetFloat(s_aTagName,"weaponssoundmultiplier", m_fWeaponsSoundMultiplier);
	m_buteMgr.SetFloat(s_aTagName,"speechsoundmultiplier", m_fSpeechSoundMultiplier);
	
	m_buteMgr.SetInt(s_aTagName,"sound16bit",(int)m_bSoundQuality);
	m_buteMgr.SetInt(s_aTagName,"MusicQuality",(int)m_bMusicQuality);
	m_buteMgr.SetInt(s_aTagName,"eaxenabled",(int)m_bEAXEnabled);
}

// RETAIL: 0x100eee40 (name ours)
void CUserProfile::SaveMisc()
{
	strcpy(s_aTagName,"Misc");
	m_buteMgr.SetString(s_aTagName,"VehicleColor",m_szVehicleColor);
	m_buteMgr.SetString(s_aTagName,"LightCycleConfig",m_sLightCycleConfig.c_str());
	m_buteMgr.SetString(s_aTagName,"LightCyclePlayerName",m_sLightCycleName.c_str());
	m_buteMgr.SetInt(s_aTagName,"CycleType",m_nLightCycleStyle);
	m_buteMgr.SetInt(s_aTagName,"CycleShade",m_nLightCycleShade);
}

void CUserProfile::SavePerformance()
{
	g_pPerformanceMgr->SavePerformanceOptions(m_buteMgr,&m_sPerformance);
}

// RETAIL: 0x100eef10 (name ours)
void CUserProfile::SaveLightCycleInfo()
{
	const char* pszTag = s_aLightCycleInfoKeys[0];
	uint32 i;

	for (i = 0; i < kMaxLightCycleInfoItems; i++)
	{
		char szKey[32];
		sprintf(szKey,"%s%d",s_aLightCycleInfoKeys[1],i);
		if (i < m_CompletedCircuits.size())
		{
			m_buteMgr.SetString(pszTag,szKey,m_CompletedCircuits[i].c_str());
		}
		else
		{
			m_buteMgr.SetString(pszTag,szKey,"");
		}
	}

	for (i = 0; i < kMaxLightCycleInfoItems; i++)
	{
		char szKey[32];
		sprintf(szKey,"%s%d",s_aLightCycleInfoKeys[2],i);
		if (i < m_UnlockedCycleColors.size())
		{
			m_buteMgr.SetString(pszTag,szKey,m_UnlockedCycleColors[i].c_str());
		}
		else
		{
			m_buteMgr.SetString(pszTag,szKey,"");
		}
	}

	for (i = 0; i < kMaxLightCycleInfoItems; i++)
	{
		char szKey[32];
		sprintf(szKey,"%s%d",s_aLightCycleInfoKeys[3],i);
		if (i < m_UnlockedPowerups.size())
		{
			m_buteMgr.SetString(pszTag,szKey,m_UnlockedPowerups[i].c_str());
		}
		else
		{
			m_buteMgr.SetString(pszTag,szKey,"");
		}
	}

	for (i = 0; i < kMaxLightCycleInfoItems; i++)
	{
		char szKey[32];
		sprintf(szKey,"%s%d",s_aLightCycleInfoKeys[4],i);
		if (i < m_UnlockedMaps.size())
		{
			m_buteMgr.SetString(pszTag,szKey,m_UnlockedMaps[i].c_str());
		}
		else
		{
			m_buteMgr.SetString(pszTag,szKey,"");
		}
	}
}

void CUserProfile::Term()
{
	m_buteMgr.Term();
	m_bInitted = LTFALSE;
}

CBindingData* CUserProfile::FindBinding(int commandIndex)
{
	int n = 0;
	while (n < m_nNumBindings && m_bindings[n].nAction != g_CommandArray[commandIndex].nActionStringID)
		n++;

	if (n >= m_nNumBindings)
		return LTNULL;
	return &m_bindings[n];
}

char const* CUserProfile::GetDeviceName( uint32 nDeviceIndex )
{
	if( nDeviceIndex > 2 )
		return NULL;

	return strDeviceName[nDeviceIndex];
}


//**************************************************************************
//functions to read settings from the profile and write them to the console
//**************************************************************************

	//take bindings from profile and apply them to the game
void CUserProfile::ApplyBindings()
{
	g_pProfileMgr->ClearBindings();

	for (int d = 0; d < 3; d++)
	{
		CBindingData *pWheelUp = LTNULL;
		CBindingData *pWheelDown = LTNULL;
		for (int c = 0; c < g_kNumCommands; c++)
		{
			CBindingData *pData = FindBinding(c);
			if (pData)
			{

				if (devices[d] == DEVICETYPE_MOUSE && stricmp(pData->strRealName[d],"#U") == 0)
				{
					pWheelUp = pData;
				}
				else if (devices[d] == DEVICETYPE_MOUSE && stricmp(pData->strRealName[d],"#D") == 0)
				{
					pWheelDown = pData;
				}
				else if (pData->strRealName[d][0] && pData->strRealName[d][0] != ' ')
				{
					char tempStr[64];
					LoadString(pData->nAction,tempStr,sizeof(tempStr));

					// Set the binding
					g_pLTClient->AddBinding(strDeviceName[d], pData->strRealName[d],tempStr,0.0f,0.0f);

				}
			}


		}
		if (devices[d] == DEVICETYPE_MOUSE)
		{
			char tempStr[512] = "";
			if (pWheelUp || pWheelDown)
			{
				// Set the mouse wheel binding
				char upStr[64] = "";
				if (pWheelUp)
				{
					LoadString(pWheelUp->nAction,upStr,sizeof(upStr));
				}
				char downStr[64] = "";
				if (pWheelDown)
				{
					LoadString(pWheelDown->nAction,downStr,sizeof(downStr));
				}

				g_pLTClient->AddBinding(strDeviceName[d],"##z-axis",upStr,0.10f,255.0f);
				g_pLTClient->AddBinding(strDeviceName[d],"##z-axis",downStr,-0.10f,-255.0f);
			}

		}
				
	}
}


void CUserProfile::ApplyMouse()
{
	WriteConsoleInt("MouseInvertY",m_bInvertY);
	WriteConsoleInt("MouseInvertLCY",m_bInvertLCMouse);
	WriteConsoleInt("MouseLook",m_bMouseLook);
	WriteConsoleFloat("MouseSensitivity",(float)m_nSensitivity);
	WriteConsoleFloat("InputRate",(float)m_nInputRate);

	float fTemp = (float)m_nVehicleTurn / 100.0f;
	WriteConsoleFloat("VehicleTurnRateScale",fTemp);

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	pSettings->ImplementMouseSensitivity();
}

void CUserProfile::ApplyKeyboard()
{
	if (!m_bMouseLook)
		WriteConsoleInt("AutoCenter",(int)m_bAutoCenter);

	float fTemp = (float)m_nNormalTurn / 10.0f;
	WriteConsoleFloat("NormalTurnRate",fTemp);

	fTemp = (float)m_nFastTurn / 10.0f;
	WriteConsoleFloat("FastTurnRate",fTemp);

	fTemp = (float)m_nLookUp / 10.0f;
	WriteConsoleFloat("LookUpRate",fTemp);

	fTemp = (float)m_nVehicleTurn / 100.0f;
	WriteConsoleFloat("VehicleTurnRateScale",fTemp);
}

void CUserProfile::ApplyMultiplayer()
{
	// RETAIL: 0x100ef4c0 -- no LAN case and no clamp in Tron
	//set mp console vars here
	if (m_nBandwidthClient < eBandwidth_Custom)
		WriteConsoleFloat("BandwidthTargetClient",((float)g_BandwidthClient[m_nBandwidthClient] * 1024.0f));
	else
		WriteConsoleFloat("BandwidthTargetClient",((float)m_nBandwidthClientCustom * 1024.0f));

}

void CUserProfile::ApplyGameOptions()
{
	// RETAIL: 0x100ef510
	int nGore = (int)(m_bGoreAllowed && m_bGore);
	WriteConsoleInt("Gore", nGore);
	WriteConsoleInt("Subtitles",m_nSubtitles);
	WriteConsoleInt("HUDLayout",m_nLayout);
	WriteConsoleInt("Difficulty",m_nDifficulty);

	g_pGameClientShell->SetDifficulty((GameDifficulty)m_nDifficulty);

	WriteConsoleFloat("HeadBob",((LTFLOAT)m_nHeadBob / 10.0f));
	WriteConsoleFloat("WeaponSway",((LTFLOAT)m_nWeaponSway / 10.0f));
	WriteConsoleFloat("MessageDuration",((LTFLOAT)m_nMsgDur / 10.0f));

	g_pMoveMgr->SetRunLock(m_bAlwaysRun);

	// The light cycle camera...
	if (g_pPlayerMgr)
	{
		g_pPlayerMgr->TronUnknownVirtual24(m_bLCThirdPersonCam, false);
		g_pPlayerMgr->TronUnknownVirtual28(m_bLCManualCam);

		g_pPlayerMgr->GetPlayerCamera()->SetTronUnknown100(m_fLCCamZoom);

		float fPitch = RAD2DEG(m_fLCPlayerPitch) + m_fLCCamPitch;
		if (fPitch < 0.0f || fPitch > 90.0f)
		{
			m_fLCPlayerPitch = 0.0f;
			m_fLCCamPitch = 45.0f;
		}

		if (m_fLCCamYaw < 0.0f || m_fLCCamYaw > 360.0f)
		{
			m_fLCCamYaw = 94.0f;
			m_fLCPlayerPitch = 0.0f;
		}

		g_pPlayerMgr->GetPlayerCamera()->SetTronUnknownF8(DEG2RAD(m_fLCCamPitch));
		g_pPlayerMgr->GetPlayerCamera()->SetTronUnknownFC(DEG2RAD(m_fLCCamYaw));
		g_pPlayerMgr->SetPitch(m_fLCPlayerPitch);
	}

	if (g_pMoveMgr && g_pMoveMgr->GetVehicleMgr())
	{
		float fLag;
		if (m_nLCCamLag == 11)
		{
			fLag = -1.0f;
		}
		else if (m_nLCCamLag == 0)
		{
			fLag = 0.0f;
		}
		else
		{
			fLag = (float)(m_nLCCamLag - 1) * (1.0f / 9.0f) + 1.0f;
		}
		g_pMoveMgr->GetVehicleMgr()->SetLightCycleCamLag(fLag);
		g_pMoveMgr->GetVehicleMgr()->SetLightCycleCamSpeed((float)m_nLCCamSpeed * 0.2f);
	}

	g_pGameClientShell->GetClientFXMgr()->SetGoreEnabled(nGore != 0);
}

void CUserProfile::ApplyCrosshair()
{
    WriteConsoleInt("CrosshairRed",m_CrosshairR );
    WriteConsoleInt("CrosshairGreen",m_CrosshairG );
    WriteConsoleInt("CrosshairBlue",m_CrosshairB );
    WriteConsoleInt("CrosshairAlpha",m_CrosshairA );

    WriteConsoleInt("CrosshairStyle", m_nStyle);
	WriteConsoleInt("CrosshairDynamic", m_bDynamic );
	g_pInterfaceMgr->EnableCrosshair(m_bCrosshair);
}

void CUserProfile::ApplySound()
{

	WriteConsoleInt("soundvolume",m_nSoundVolume);
	WriteConsoleFloat( "weaponssoundmultiplier", m_fWeaponsSoundMultiplier );
	WriteConsoleFloat( "defaultsoundmultiplier", m_fDefaultSoundMultiplier );
	WriteConsoleFloat( "speechsoundmultiplier", m_fSpeechSoundMultiplier );
	ImplementSoundVolume();

	// RETAIL: 0x100ef800 -- Tron's EAX option turns the sound filters on
	// and off.
	if (!m_bEAXEnabled)
	{
		ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();
		if (pSoundMgr)
		{
			pSoundMgr->EnableSoundFilter(m_bEAXEnabled != LTFALSE);
		}
	}
	else
	{
		g_pClientSoundMgr->SetSoundFilter(g_pGameClientShell->GetPlayerMgr()->GetSoundFilter());
	}

	LTBOOL bOldEAX = GetConsoleInt("SoundFilters",0);
	WriteConsoleInt("SoundFilters",(int)m_bEAXEnabled);

	LTBOOL bOldSoundQuality = GetConsoleInt("sound16bit",0);
	WriteConsoleInt("sound16bit",(int)m_bSoundQuality);
	if (m_bSoundQuality != bOldSoundQuality || bOldEAX != m_bEAXEnabled)
	{
		g_pGameClientShell->InitSound();
	}

	// Set the music quality...

	WriteConsoleInt("MusicQuality",(int)m_bMusicQuality);
	g_pGameClientShell->GetMusic()->SetQuality(!!m_bMusicQuality);	

	// RETAIL: Tron rereads the performance settings here.
	g_pPerformanceMgr->GetPerformanceOptions(&m_sPerformance);

	//if we haven't gotten to the menus yet, don't start the music yet...
	if (g_pInterfaceMgr->HasEnteredScreenState())
	{
		uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
		LTBOOL bMusicOn = (m_nMusicVolume > MUSIC_MIN_VOL) && (dwAdvancedOptions & AO_MUSIC) && GetConsoleInt("MusicActive",0);
		WriteConsoleInt("MusicActive",(int)bMusicOn);
		if (bMusicOn != g_pGameClientShell->GetMusic()->IsPlaying())
		{
			if (bMusicOn)
			{
				// [KLS 8/11/02] SetupMusic will handle turning on the
				// music and making sure it plays the appropriate music.
				g_pInterfaceMgr->SetupMusic();
			}
			else 
			{
				g_pGameClientShell->GetMusic()->Term();

			}
		}
	}
	
	ImplementMusicVolume();
}


void CUserProfile::ImplementSoundVolume()
{
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetVolume(m_nSoundVolume);

	// set up the sound volume classes for the game
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetSoundClassMultiplier( WEAPONS_SOUND_CLASS, m_fWeaponsSoundMultiplier );
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetSoundClassMultiplier( SPEECH_SOUND_CLASS, m_fSpeechSoundMultiplier, false );
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetSoundClassMultiplier( DEFAULT_SOUND_CLASS, m_fDefaultSoundMultiplier );

	// now have the sound manager update any playing sounds
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->UpdateVolumeSettings();
}


void CUserProfile::ImplementMusicVolume()
{
	WriteConsoleInt("musicvolume",m_nMusicVolume);
	g_pGameClientShell->GetMusic()->SetMenuVolume(m_nMusicVolume);
}

void CUserProfile::ApplyDisplay()
{

	LTBOOL initVS = (LTBOOL)GetConsoleInt("VSyncOnFlip",1);
	WriteConsoleInt("HardwareCursor",m_bHardwareCursor);
	WriteConsoleInt("VSyncOnFlip",m_bVSync);

	WriteConsoleFloat("GammaR",m_fGamma);
	WriteConsoleFloat("GammaG",m_fGamma);
	WriteConsoleFloat("GammaB",m_fGamma);

	bool bRestart = (initVS != m_bVSync);
#ifdef TRON20SRC_MODERN
	bool bModernDisplayRestart = g_bModernDisplayRestart;
	g_bModernDisplayRestart = false;
	if (bModernDisplayRestart)
		bRestart = true;
#endif
//	if (bRestart)
//	{
//		g_pLTClient->CPrint("CUserProfile::ApplyDisplay() : restart because of VSync change");
//	}

	// RETAIL: Tron's is Build 51's: the render modes are listed first and
	// matched without the hardware T&L flag (0x100efb23)

	// Build the list of render modes
	RMode *pRenderModes=g_pLTClient->GetRenderModes();
	RMode *pMode=pRenderModes;
	LTBOOL bFound = LTFALSE;
	while (pMode != LTNULL && !bFound)
	{
		bFound = (	pMode->m_Width == m_nScreenWidth &&
					pMode->m_Height == m_nScreenHeight &&
					pMode->m_BitDepth == m_nScreenDepth );
		if (!bFound)
			pMode = pMode->m_pNext;
	}

	RMode currentMode;
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-31: a window can be any size (EN-31): a size the adapter
	// has no mode for is the current renderer at that size
	RMode modernWindowMode;
	if (!pMode && GetConsoleInt("Windowed",0) && g_pGameClientShell->IsRendererInitted() &&
		g_pLTClient->GetRenderMode(&modernWindowMode) == LT_OK && m_nScreenWidth >= 640 && m_nScreenHeight >= 480)
	{
		modernWindowMode.m_Width = m_nScreenWidth;
		modernWindowMode.m_Height = m_nScreenHeight;
		modernWindowMode.m_pNext = LTNULL;
		pMode = &modernWindowMode;
	}
#endif
	if (g_pGameClientShell->IsRendererInitted() && g_pLTClient->GetRenderMode(&currentMode) == LT_OK)
	{
		if (pMode && !IsRendererEqual(pMode,&currentMode))

		{
			//switch renderers
			// RETAIL: Tron's string 911 (0x100efbbc; NOLF2's IDS_REINITIALIZING_RENDERER
			// is 1226)
			g_pInterfaceResMgr->DrawMessage(911);
			g_pLTClient->SetRenderMode(pMode);
			g_pInterfaceMgr->ScreenDimsChanged();

			//update the game camera to have the new dimensions
			g_pLTClient->SetCameraRect(g_pGameClientShell->GetPlayerMgr()->GetCamera(), LTTRUE, 0, 0, pMode->m_Width, pMode->m_Height);
//			if (bRestart)
//				g_pLTClient->CPrint("CUserProfile::ApplyDisplay() : cancel restart because of mode change");
			bRestart = false;
		}
	}

	// Free the linked list of render modes
	g_pLTClient->RelinquishRenderModes(pRenderModes);

	
	//make sure to save out these changed settings so that they will be applied next
	//time we run
	SaveDisplaySettings();

	if (bRestart)
	{
		// Set the renderer mode
		g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
//		g_pLTClient->CPrint("CUserProfile::ApplyDisplay() : restart");
		g_pLTClient->RunConsoleString("RestartRender");
		g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);

		//force this because restarting the renderer may affect the cursor
		g_pCursorMgr->UseHardwareCursor(m_bHardwareCursor,true);
	}

#ifdef TRON20SRC_MODERN
	// MODERN.md CL-31: a borderless window takes the desktop's size, so the
	// mode the renderer got can differ from the one asked for: the
	// interface, the camera and the profile follow the mode it got
	if (bModernDisplayRestart && g_pGameClientShell->IsRendererInitted())
	{
		RMode modernMode;
		if (g_pLTClient->GetRenderMode(&modernMode) == LT_OK)
		{
			g_pInterfaceMgr->ScreenDimsChanged();
			g_pLTClient->SetCameraRect(g_pGameClientShell->GetPlayerMgr()->GetCamera(), LTTRUE, 0, 0, modernMode.m_Width, modernMode.m_Height);
			m_nScreenWidth = modernMode.m_Width;
			m_nScreenHeight = modernMode.m_Height;
			m_nScreenDepth = modernMode.m_BitDepth;
			SaveDisplaySettings();
		}
	}
#endif


}

// RETAIL: 0x100efc80 (name ours)
void CUserProfile::ApplyMisc()
{
	if (g_pPlayerMgr)
		g_pPlayerMgr->SetTronUnknown1B5(m_szVehicleColor);
}

void CUserProfile::SetPerformanceCfg(char *pszCfg)
{
	if (!pszCfg || !pszCfg[0]) return;
	
	g_pPerformanceMgr->SetPerformanceCfg(pszCfg);
	g_pPerformanceMgr->GetPerformanceOptions(&m_sPerformance);
	SavePerformance();
	ApplyPerformance(false);
}

void CUserProfile::ApplyPerformance(bool bForceDisplay)
{
	g_pPerformanceMgr->SetPerformanceOptions(&m_sPerformance);

	//other settings affected by performance
	//if we are going to force the display resolution to match out overall performance level,
	// we should ignore the current resolution when determining what our level is.
	uint8 nOverall = g_pPerformanceMgr->GetPerformanceCfg(bForceDisplay);
	SetSound();
	SetDisplay();

	g_pLTClient->CPrint("CUserProfile::ApplyPerformance(%d) : nOverall = %d",bForceDisplay,nOverall);
	switch (nOverall)
	{
	case 0:
		if (m_nScreenWidth != 640)
		{
			m_nScreenWidth  = 640;
			m_nScreenHeight = 480;
			ApplyDisplay();
		}
		break;
	case 1:
		if (m_nScreenWidth != 800)
		{
			m_nScreenWidth  = 800;
			m_nScreenHeight = 600;
			ApplyDisplay();
		}

		break;

	case 2:
		if (m_nScreenWidth < 1024)
		{
			m_nScreenWidth  = 1024;
			m_nScreenHeight = 768;
			ApplyDisplay();
		}
		break;

	}
	ApplySound();

	if (g_pLTClient->IsConnected())
		SendPerformanceMsg();
}


void CUserProfile::SendPerformanceMsg()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PERFORMANCE );
	cMsg.Writeuint8( m_sPerformance.nSettings[kPerform_EnvironmentalDetail] );
	cMsg.Writebool( (m_sPerformance.nSettings[kPerform_PreCacheAssets] > 0) );
    g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

}


void CUserProfile::ApplyControls()
{

	WriteConsoleInt("UseJoystick",(int)m_bUseJoystick);
}

void CUserProfile::ApplyJoystick()
{
	uint32 devType = DEVICETYPE_UNKNOWN;
	if (g_pGameClientShell->HasGamepad())
		devType = DEVICETYPE_GAMEPAD;
	else if (g_pGameClientShell->HasJoystick())
		devType = DEVICETYPE_JOYSTICK;

	//clear all joystick actions bound to an axis
    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (devType);
	//step through each binding
	DeviceBinding* ptr = pBindings;
	while (ptr)
	{
		uint32 contType = g_pProfileMgr->GetControlType(devType, ptr->m_nObjectId );
		if (contType != CONTROLTYPE_BUTTON && contType != CONTROLTYPE_KEY)							 
		{
			g_pLTClient->ClearBinding(ptr->strDeviceName, ptr->strRealName);
		}
		ptr = ptr->pNext;
	}

    g_pLTClient->FreeDeviceBindings (pBindings);

	if (!m_bUseJoystick)
	{
		WriteConsoleInt("UseJoystick",0);
		return;
	}
	
	WriteConsoleInt("UseJoystick",1);

	char strJoystick[128];
	memset(strJoystick, 0, 128);
	LTRESULT result = g_pLTClient->GetDeviceName(devType, strJoystick, 127);
	if (result != LT_OK) return;

	for (int a = 0; a < g_pProfileMgr->GetNumAxis(); a++)
	{
		CDeviceAxisData *pAxisData = g_pProfileMgr->GetAxisData(a);
		if (!pAxisData || m_nAxis[a] == eJoystick_None) continue;

		// figure out the intermediate range information
		float fRange = pAxisData->m_fRangeHigh - pAxisData->m_fRangeLow;
		float fActiveRange = fRange * 0.45f;

		char *pszHigh = NULL;
		char *pszLow = NULL;

		switch (pAxisData->m_nType)
		{
			case CONTROLTYPE_XAXIS:
			case CONTROLTYPE_RXAXIS:
			case CONTROLTYPE_ZAXIS:
				if (m_nAxis[a] == eJoystick_Look)
				{
					pszHigh = szJoyRight;
					pszLow = szJoyLeft;
				}
				else if (m_nAxis[a] == eJoystick_Move)
				{
					pszHigh = szJoyStrafeRight;
					pszLow = szJoyStrafeLeft;
				}
				break;
			case CONTROLTYPE_YAXIS:
			case CONTROLTYPE_RYAXIS:
			case CONTROLTYPE_RZAXIS:
				if (m_nAxis[a] == eJoystick_Look)
				{
					pszHigh = szJoyLookDown;
					pszLow = szJoyLookUp;
				}
				else if (m_nAxis[a] == eJoystick_Move)
				{
					pszHigh = szJoyBackward;
					pszLow = szJoyForward;
				}
				else if (m_nAxis[a] == eJoystick_LookInvert)
				{
					pszHigh = szJoyLookUp;
					pszLow = szJoyLookDown;
				}
				break;
		}
		if (pszHigh && pszLow)
		{
			g_pLTClient->AddBinding(strJoystick, pAxisData->m_sName,pszLow,pAxisData->m_fRangeLow,pAxisData->m_fRangeLow+fActiveRange);
			g_pLTClient->AddBinding(strJoystick, pAxisData->m_sName,pszHigh,pAxisData->m_fRangeHigh-fActiveRange,pAxisData->m_fRangeHigh);
		}
	}

	for (a = 0; a < g_pProfileMgr->GetNumPOV(); a++)
	{
		CDevicePOVData *pPOVData = g_pProfileMgr->GetPOVData(a);
		if (!pPOVData || m_nPOV[a] == eJoystick_None) continue;

		char *pszUp = NULL;
		char *pszLeft = NULL;
		char *pszDown = NULL;
		char *pszRight = NULL;

		if (m_nPOV[a] == eJoystick_Look)
		{
			pszUp = szJoyLookUp;
			pszLeft = szJoyLeft;
			pszDown = szJoyLookDown;
			pszRight = szJoyRight;
			
		}
		else if (m_nPOV[a] == eJoystick_Move)
		{
			pszUp = szJoyForward;
			pszLeft = szJoyStrafeLeft;
			pszDown = szJoyBackward;
			pszRight = szJoyStrafeRight;
			
		}

		if (pszUp)
		{
			g_pLTClient->AddBinding(strJoystick, pPOVData->m_sName,pszUp,0.0f,4500.0f);
			g_pLTClient->AddBinding(strJoystick, pPOVData->m_sName,pszRight,4500.0f,13500.0f);
			g_pLTClient->AddBinding(strJoystick, pPOVData->m_sName,pszDown,13500.0f,22500.0f);
			g_pLTClient->AddBinding(strJoystick, pPOVData->m_sName,pszLeft,22500.0f,31500.0f);
			g_pLTClient->AddBinding(strJoystick, pPOVData->m_sName,pszUp,31500.0f,36000.0f);

		}
	}

}


//**************************************************************************
//functions to read settings from the console and write them to the profile
//**************************************************************************

//read bindings from the game and save them in the profile
void CUserProfile::SetBindings()
{
	for (int dev = 0; dev < 3; ++dev)
	{
		for (int i = 0; i < m_nNumBindings; i++)
		{
			SAFE_STRCPY (m_bindings[i].strRealName[dev], "");
			SAFE_STRCPY (m_bindings[i].strTriggerName[dev], "");
			m_bindings[i].nCode = 0;
			m_bindings[i].nDeviceObjectId[dev] = 0;
		}
        uint32 devType = devices[dev];
        DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (devType);
		if (!pBindings)
		{
			continue;
		}
		DeviceBinding* ptr = pBindings;
		while (ptr)
		{
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
				int com = 0;
				while (com < g_kNumCommands && (pAction->nActionCode != g_CommandArray[com].nCommandID) )
					com++;
				if (com >= g_kNumCommands)
					break;


				CBindingData *pData = FindBinding(com);
				if (pData)
				{
					uint32 contType = g_pProfileMgr->GetControlType(devType, ptr->m_nObjectId );

					pData->nDeviceObjectId[dev] = ptr->m_nObjectId;

					if (contType == CONTROLTYPE_BUTTON || contType == CONTROLTYPE_KEY)							 
					{
						SAFE_STRCPY (pData->strTriggerName[dev], ptr->strTriggerName);
						SAFE_STRCPY (pData->strRealName[dev], ptr->strRealName);
					}
					else if (devType == DEVICETYPE_MOUSE && contType == CONTROLTYPE_ZAXIS)
					{
						if (pAction->nRangeHigh > 0)
						{
							strcpy(pData->strRealName[dev],"#U");
							strcpy(pData->strTriggerName[dev],szWheelUp);
						}
						else if (pAction->nRangeHigh < 0)
						{
							strcpy(pData->strRealName[dev],"#D");
							strcpy(pData->strTriggerName[dev],szWheelDown);
						}
					}
				}
	
				pAction = pAction->pNext;
			}

			ptr = ptr->pNext;
		}
        g_pLTClient->FreeDeviceBindings (pBindings);
	}


}

void CUserProfile::SetMouse()
{
	int nMin, nMax;

	m_bMouseLook = GetConsoleInt("MouseLook",1);
	m_bInvertY = GetConsoleInt("MouseInvertY",0);
	m_bInvertLCMouse = GetConsoleInt("MouseInvertLCY",0);

	m_nInputRate = GetConsoleInt("InputRate",20);
	
	nMin = int(g_vtMouseMinInputRate.GetFloat());
	nMax = int(g_vtMouseMaxInputRate.GetFloat());

	if (m_nInputRate < nMin) m_nInputRate = nMin;
	if (m_nInputRate > nMax) m_nInputRate = nMax;

	m_nSensitivity = GetConsoleInt("MouseSensitivity",5);

	nMin = int(g_vtMouseMinSensitivity.GetFloat());
	nMax = int(g_vtMouseMaxSensitivity.GetFloat());

	if (m_nSensitivity < nMin) m_nSensitivity = nMin;
	if (m_nSensitivity > nMax) m_nSensitivity = nMax;

	float fTemp = GetConsoleFloat("VehicleTurnRateScale",1.0f);
	m_nVehicleTurn = (int)(100.0f * fTemp);
}

void CUserProfile::SetKeyboard()
{
	//keyboard settings
	if (m_bMouseLook)
	{
		m_bAutoCenter = LTFALSE;
	}
	else
		m_bAutoCenter = (LTBOOL)GetConsoleInt("AutoCenter",0);

	float fTemp = GetConsoleFloat("NormalTurnRate",1.5f);
	m_nNormalTurn = (int)(10.0f * fTemp);

	fTemp = GetConsoleFloat("FastTurnRate",2.3f);
	m_nFastTurn = (int)(10.0f * fTemp);

	fTemp = GetConsoleFloat("LookUpRate",2.5f);
	m_nLookUp = (int)(10.0f * fTemp);

}

// RETAIL: empty in Tron: CScreenControls::OnFocus (0x100585d0) calls an
// empty function (0x100abb80, a bare ret) here. NOLF2's body read the
// joystick setting from the profile.
void CUserProfile::SetControls()
{
}

void CUserProfile::SetJoystick()
{
}


void CUserProfile::SetMultiplayer()
{

}

void CUserProfile::SetGameOptions()
{
	// RETAIL: 0x100f0720
	m_bGore = m_bGoreAllowed && GetConsoleInt("Gore",0);
	m_nDifficulty = g_pGameClientShell->GetDifficulty();
	m_nSubtitles = GetConsoleInt("Subtitles",0);

	m_bAlwaysRun = (LTBOOL)g_pMoveMgr->RunLock();

	SetLightCycleCamera();

	m_nLayout = GetConsoleInt("HUDLayout",0);

	m_nHeadBob = (int)(10.0f * GetConsoleFloat("HeadBob",1.0f));
	m_nWeaponSway = (int)(10.0f * GetConsoleFloat("WeaponSway",1.0f));
	m_nMsgDur = (int)(10.0f * GetConsoleFloat("MessageDuration",1.0f));
}


void CUserProfile::SetCrosshair()
{
	m_bCrosshair = g_pInterfaceMgr->IsCrosshairEnabled();

	m_CrosshairR = GetConsoleInt("CrosshairRed",0x1F);
	m_CrosshairG = GetConsoleInt("CrosshairGreen",0xFF);
	m_CrosshairB = GetConsoleInt("CrosshairBlue",0xFF);
	m_CrosshairA = GetConsoleInt("CrosshairAlpha",0xBF);

	m_nStyle = GetConsoleInt("CrosshairStyle",0);
	m_bDynamic = GetConsoleInt("CrosshairDynamic",1);
}

void CUserProfile::SetSound()
{
	m_nSoundVolume = GetConsoleInt("soundvolume",SOUND_DEFAULT_VOL);
	m_nMusicVolume = GetConsoleInt("musicvolume",MUSIC_DEFAULT_VOL);
	m_fWeaponsSoundMultiplier = GetConsoleFloat("weaponssoundmultiplier",WEAPONS_DEFAULT_MULTIPLIER);
	m_fSpeechSoundMultiplier = GetConsoleFloat("speechsoundmultiplier",SPEECH_DEFAULT_MULTIPLIER);
	m_fDefaultSoundMultiplier = GetConsoleFloat("defaultsoundmultiplier", DEFAULT_DEFAULT_MULTIPLIER);

	m_bSoundQuality = (LTBOOL)GetConsoleInt("sound16bit",0);
	m_bMusicQuality = (LTBOOL)GetConsoleInt("MusicQuality",0);
	// RETAIL: 0x100f07f0 -- EAX is the "SoundFilters" console variable
	m_bEAXEnabled = (LTBOOL)GetConsoleInt("SoundFilters",0);

	if (!GetConsoleInt("MusicEnable",0) || !GetConsoleInt("MusicActive",0))
		m_nMusicVolume = MUSIC_MIN_VOL;

	//TODO: deal with m_bMusicQuality
}

void CUserProfile::SetDisplay()
{
	m_bHardwareCursor = (GetConsoleInt("HardwareCursor",0) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);
	m_bVSync = (GetConsoleInt("VSyncOnFlip",1) > 0 );

	//get the average
	m_fGamma = (GetConsoleFloat("GammaR",1.0f) + GetConsoleFloat("GammaG",1.0f) + GetConsoleFloat("GammaB",1.0f)) / 3.0f;

	// The current render mode
	RMode currentMode;
	g_pLTClient->GetRenderMode(&currentMode);

	m_nScreenWidth = currentMode.m_Width;
	m_nScreenHeight = currentMode.m_Height;
	m_nScreenDepth = currentMode.m_BitDepth;

	g_pLTClient->CPrint("CUserProfile::SetDisplay() : %d x %d %d",m_nScreenWidth,m_nScreenHeight,m_nScreenDepth);

}

void CUserProfile::SetPerformance()
{
	g_pPerformanceMgr->GetPerformanceOptions(&m_sPerformance);
	m_bMusicQuality = (LTBOOL)GetConsoleInt("MusicQuality",0);
}

LTBOOL CProfileMgr::Init()
{

	ASSERT(g_kNumCommands <= MAX_NUM_COMMANDS);
	if (g_kNumCommands > MAX_NUM_COMMANDS) 
		return LTFALSE;


	// RETAIL: 0x100f0a00 -- Tron's default profile name is its string 1008
	// (ClientResShared.h's IDS_PLAYER is NOLF2's 1241)
	LoadString(kDefaultProfileNameStringId,szDefProfileName,sizeof(szDefProfileName));



	g_pProfileMgr = this;

	SetDeviceData();

	std::string currentProfile;
	char szProfileName[128] = "";

	//get the profile name
	CRegMgr* pRegMgr = g_pVersionMgr->GetRegMgr();
	if (pRegMgr->IsValid())
	{
		unsigned int nBufSize = sizeof(szProfileName);
		pRegMgr->Get("ProfileName", szProfileName, nBufSize, "");
	}

	// RETAIL: Tron applies "SetPerformanceLevel" in CUserProfile::Init.
	//if we have a profile name try to init it
	if (strlen(szProfileName) > 0)
	{
		currentProfile = szProfileName;
		if (m_profile.Init(currentProfile, LTFALSE))
		{
			//if we've been told to restore, do so...
			if (GetConsoleInt("RestoreDefaults",0))
			{
				m_profile.RestoreDefaults(PROFILE_ALL);
				WriteConsoleInt("RestoreDefaults",0);
			}


			//we also want to make sure to save the display configuration so that parameters passed
			//through the command line will be saved for the next run
			SaveDisplaySettings();
			return LTTRUE;
		}
	}

	//we either don't have a profile or it couldn't be initted, create one
	NewProfile(szDefProfileName);

	if (!m_profile.IsInitted()) 
		return LTFALSE;


	//we also want to make sure to save the display configuration so that parameters passed
	//through the command line will be saved for the next run
	SaveDisplaySettings();

	return LTTRUE;
}

void CProfileMgr::Term()
{
	if (m_profile.IsInitted())
	{
		m_profile.SetBindings();

		m_profile.Save();
		m_profile.Term();
	}

	g_pProfileMgr = LTNULL;
}

void CProfileMgr::GetProfileList(StringSet& profileList)
{
	struct _finddata_t file;
	long hFile;

	std::string directory = s_sProfileDir + "*" + s_sProfileExt;

	// find first file
	if((hFile = _findfirst(directory.c_str(), &file)) != -1L)
	{
		do
		{
			char *pName = strtok(file.name,".");
			profileList.insert(pName);
		}
		while(_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);
}

void CProfileMgr::ClearBindings()
{
	//step through each device
	for (int i = 0; i < 3; ++i)
	{
        DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (devices[i]);
		if (!pBindings)
		{
			continue;
		}

		//step through each binding
		DeviceBinding* ptr = pBindings;
		while (ptr)
		{
			//step through each action on that binding checking to see if we should clear it
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
				int com = 0;
				while (com < g_kNumCommands && pAction->nActionCode != g_CommandArray[com].nCommandID)
					com++;
				//if it is a bindable command, clear it
				if (com < g_kNumCommands)
				{
					uint32 contType = GetControlType(devices[i], ptr->m_nObjectId );
					if (contType == CONTROLTYPE_BUTTON || contType == CONTROLTYPE_KEY)							 
					{
						g_pLTClient->ClearBinding(ptr->strDeviceName, ptr->strRealName);
					}
					else if (devices[i] == DEVICETYPE_MOUSE && contType == CONTROLTYPE_ZAXIS)
					{
						g_pLTClient->ClearBinding(ptr->strDeviceName, ptr->strRealName);
					}

				}
				pAction = pAction->pNext;
			}

			ptr = ptr->pNext;
		}

        g_pLTClient->FreeDeviceBindings (pBindings);
	}
}



void CProfileMgr::NewProfile(const std::string& profileName)
{
	if (profileName.length() == 0)
		return;
	if (m_profile.IsInitted())
	{
		m_profile.Save();
		m_profile.Term();
	}

	// Make sure we're disconnected from server.
	if(g_pLTClient->IsConnected())
	{
		g_pInterfaceMgr->SetIntentionalDisconnect( true );
		g_pClientMultiplayerMgr->ForceDisconnect();
	}
	
	if (!m_profile.Init(profileName, LTTRUE)) return;

	m_profile.Save();
	

	CRegMgr* pRegMgr = g_pVersionMgr->GetRegMgr();
	if (pRegMgr->IsValid())
	{
		pRegMgr->Set("ProfileName", m_profile.m_sName.c_str());
	}
	else
	{
		_ASSERT("!RegMgr not valid.");
	}

}

void CProfileMgr::DeleteProfile(const std::string& profileName)
{
	LTBOOL bNeedNew = LTFALSE;
	if (m_profile.IsInitted() && m_profile.m_sName.compare(profileName) == 0)
	{
		m_profile.Term();
		bNeedNew = LTTRUE;
	}
		
	std::string fn = s_sProfileDir + profileName + s_sProfileExt;
	remove(fn.c_str());

	g_pClientSaveLoadMgr->DeleteSpecificSaveDir( profileName.c_str() );

	if (bNeedNew)
	{
		StringSet	profileList;
		GetProfileList(profileList);

		StringSet::iterator iter = profileList.begin();
		if (iter == profileList.end())
		{
			//empty list
			NewProfile(szDefProfileName);
		}
		else
			NewProfile(*iter);


	}

}
void CProfileMgr::RenameProfile(const std::string& oldName,const std::string& newName)
{
	if (newName.length() == 0)
		return;
	LTBOOL bIsCurrent = (m_profile.IsInitted() && m_profile.m_sName.compare(oldName) == 0);

	if (bIsCurrent && m_profile.IsInitted())
	{
		m_profile.Save();
		m_profile.Term();
	}
		
	std::string ofn = s_sProfileDir + oldName + s_sProfileExt;
	std::string nfn = s_sProfileDir + newName + s_sProfileExt;
	rename(ofn.c_str(),nfn.c_str());

	char oldDir[128];
	char newDir[128];
	sprintf(oldDir,"%s\\%s",SAVE_DIR,oldName.c_str());
	sprintf(newDir,"%s\\%s",SAVE_DIR,newName.c_str());
	rename(oldDir,newDir);

	if (bIsCurrent)
	{
		m_profile.Init(newName,LTFALSE);

		CRegMgr* pRegMgr = g_pVersionMgr->GetRegMgr();
		if (pRegMgr->IsValid())
		{
			pRegMgr->Set("ProfileName", m_profile.m_sName.c_str());
		}
		else
		{
			_ASSERT("!RegMgr not valid.");
		}


	}
}


LTBOOL CUserProfile::RestoreDefaults(uint8 nFlags)
{
	std::string dfn = s_sProfileDir + "default.prf";

	LTBOOL bRet = LTFALSE;

	if (m_bInitted)
		m_buteMgr.Term();

	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(dfn.c_str(), m_pCryptKey);
	}
	else
	{
		bRet = m_buteMgr.Parse(dfn.c_str());
	}

	if (!bRet) return LTFALSE;

	if (nFlags & PROFILE_CONTROLS)
		LoadControls();
	if (nFlags & PROFILE_MULTI)
		LoadMultiplayer();
	if (nFlags & PROFILE_GAME)
		LoadGameOptions();
	if (nFlags & PROFILE_SOUND)
		LoadSound(true);

	m_bInitted =  LTTRUE;

	return LTTRUE;

}

uint32 CProfileMgr::GetControlType(uint32 deviceType, uint32 nObjectId )
{
    DeviceObject* pList = g_pLTClient->GetDeviceObjects(deviceType);
	DeviceObject* pObj = pList;
    uint32 type = CONTROLTYPE_UNKNOWN;
	bool bFound = false;
	while (pObj != NULL && !bFound)
	{
		bFound = ( nObjectId == pObj->m_nObjectId );
		if (bFound)
			type = pObj->m_ObjectType;
		pObj = pObj->m_pNext;
	}

	if( pList )
	{
		g_pLTClient->FreeDeviceObjects( pList );
		pList = NULL;
	}

	return type;
}


void CProfileMgr::SetDeviceData()
{

	if (g_pGameClientShell->HasGamepad())
	{
		devices[2] = DEVICETYPE_GAMEPAD;
		g_pGameClientShell->EnableGamepad();
	}
	else if (g_pGameClientShell->HasJoystick())
	{
		devices[2] = DEVICETYPE_JOYSTICK;
		g_pGameClientShell->EnableJoystick();
	}
	else
		devices[2] = DEVICETYPE_UNKNOWN;

	for (int dev = 0; dev < 3; dev++)
	{
		g_pLTClient->GetDeviceName (devices[dev],strDeviceName[dev], sizeof(strDeviceName[dev]));
	}

	LoadString(IDS_WHEEL_UP,szWheelUp,sizeof(szWheelUp));
	LoadString(IDS_WHEEL_DOWN,szWheelDown,sizeof(szWheelDown));

	LoadString(IDS_ACTIONSTRING_LOOKUP,szJoyLookUp,sizeof(szJoyLookUp));
	LoadString(IDS_ACTIONSTRING_LOOKDOWN,szJoyLookDown,sizeof(szJoyLookDown));
	LoadString(IDS_ACTIONSTRING_FORWARD,szJoyForward,sizeof(szJoyForward));
	LoadString(IDS_ACTIONSTRING_BACKWARD,szJoyBackward,sizeof(szJoyBackward));
	LoadString(IDS_ACTIONSTRING_TURNLEFT,szJoyLeft,sizeof(szJoyLeft));
	LoadString(IDS_ACTIONSTRING_TURNRIGHT,szJoyRight,sizeof(szJoyRight));
	LoadString(IDS_ACTIONSTRING_STRAFELEFT,szJoyStrafeLeft,sizeof(szJoyStrafeLeft));
	LoadString(IDS_ACTIONSTRING_STRAFERIGHT,szJoyStrafeRight,sizeof(szJoyStrafeRight));


	m_nNumDeviceAxis = 0;
	m_nNumDevicePOV = 0;

	

	DeviceObject* pObjects = g_pLTClient->GetDeviceObjects(devices[2]);
	DeviceObject* pObj = pObjects;
    LTBOOL bFoundIt = LTFALSE;

	// loop through all joystick objects and store the axis ones with our devicename the m_aDeviceData array
	while ((pObj != NULL) && (m_nNumDeviceAxis < kMaxDeviceAxis))
	{
		if ((pObj->m_ObjectName != NULL) && (pObj->m_DeviceName != NULL))
		{
			if	(	(pObj->m_DeviceType == DEVICETYPE_JOYSTICK || pObj->m_DeviceType == DEVICETYPE_GAMEPAD) &&
					(stricmp(pObj->m_DeviceName, strDeviceName[2]) == 0) &&
					(	(pObj->m_ObjectType == CONTROLTYPE_XAXIS) ||
						(pObj->m_ObjectType == CONTROLTYPE_YAXIS) ||
						(pObj->m_ObjectType == CONTROLTYPE_ZAXIS) ||
						(pObj->m_ObjectType == CONTROLTYPE_RXAXIS) ||
						(pObj->m_ObjectType == CONTROLTYPE_RYAXIS) ||
						(pObj->m_ObjectType == CONTROLTYPE_RZAXIS) 
					)
				)
			{
				m_aDeviceAxisData[m_nNumDeviceAxis].Init(pObj->m_ObjectName, pObj->m_ObjectType, pObj->m_RangeLow, pObj->m_RangeHigh);
				m_nNumDeviceAxis++;
			}

		}
		if ((pObj->m_ObjectName != NULL) && (pObj->m_DeviceName != NULL))
		{
			if	(	(pObj->m_DeviceType == DEVICETYPE_JOYSTICK || pObj->m_DeviceType == DEVICETYPE_GAMEPAD) &&
					(stricmp(pObj->m_DeviceName, strDeviceName[2]) == 0) &&
					(pObj->m_ObjectType == CONTROLTYPE_POV)
				)
			{
				m_aDevicePOVData[m_nNumDevicePOV].Init(pObj->m_ObjectName);
				m_nNumDevicePOV++;
			}

		}
		pObj = pObj->m_pNext;
	}

	// free the device objects
	if (pObjects != NULL) g_pLTClient->FreeDeviceObjects (pObjects);

}

CDeviceAxisData* CProfileMgr::GetAxisData(int ndx)
{
	if (ndx < 0 || ndx >= m_nNumDeviceAxis) 
		return LTNULL;
	return &m_aDeviceAxisData[ndx];
}

CDevicePOVData* CProfileMgr::GetPOVData(int ndx)
{
	if (ndx < 0 || ndx >= m_nNumDevicePOV) 
		return LTNULL;
	return &m_aDevicePOVData[ndx];
}

LTBOOL IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2)
{
	_ASSERT(pRenderer1);
	_ASSERT(pRenderer2);
	if (!pRenderer1 || !pRenderer2) return LTFALSE;

	if (pRenderer1->m_Width != pRenderer2->m_Width)
	{
        return LTFALSE;
	}

	if (pRenderer1->m_Height != pRenderer2->m_Height)
	{
        return LTFALSE;
	}

	if (pRenderer1->m_BitDepth != pRenderer2->m_BitDepth)
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// Gets the default session name.
bool GetDefaultSessionName( GameType eGameType, char* pszSessionName, uint32 nSessionNameLen )
{
	uint32 nStringId = 0;

	switch( eGameType )
	{
		case eGameTypeCooperative:
			nStringId = IDS_HOST_NAME_COOP_DEFAULT;
			break;
		case eGameTypeDeathmatch:
			nStringId = IDS_HOST_NAME_DM_DEFAULT;
			break;
		case eGameTypeTeamDeathmatch:
			nStringId = IDS_HOST_NAME_TDM_DEFAULT;
			break;
		case eGameTypeDoomsDay:
			nStringId = IDS_HOST_NAME_DD_DEFAULT;
			break;
		default:
			return false;
			break;
	}

	strncpy( pszSessionName, LoadTempString( nStringId ), nSessionNameLen );
	pszSessionName[ nSessionNameLen - 1 ] = '\0';

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CUserProfile::SetLightCycleCamera
//
//	PURPOSE:	Read the light cycle camera settings back from the player
//				(RETAIL: 0x100f18b0, the TU's last function; name ours)
//
// ----------------------------------------------------------------------- //

void CUserProfile::SetLightCycleCamera()
{
	m_bLCThirdPersonCam = g_pPlayerMgr->GetTronUnknown1D7();
	m_bLCManualCam = g_pPlayerMgr->GetTronUnknown1D8();

	if (g_pPlayerMgr->IsPlayerInWorld())
	{
		m_fLCCamZoom = g_pPlayerMgr->GetPlayerCamera()->GetTronUnknown100();
		m_fLCCamPitch = RAD2DEG(g_pPlayerMgr->GetPlayerCamera()->GetTronUnknownF8());
		m_fLCCamYaw = RAD2DEG(g_pPlayerMgr->GetPlayerCamera()->GetTronUnknownFC());
		m_fLCPlayerPitch = g_pPlayerMgr->GetPitch();
	}
}
