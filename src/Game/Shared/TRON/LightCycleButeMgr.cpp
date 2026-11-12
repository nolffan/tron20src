// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleButeMgr.cpp
//
// PURPOSE : The light cycles' attributes (Attributes\LightCycles.txt)
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x100257e0..0x10029c80, vtable 0x10293df0). See
// LightCycleButeMgr.h for what is retail's and what is ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleButeMgr.h"
#include <set>

// RETAIL: tags and attributes
#define LCBMGR_CYCLEMOD_TAG				"CycleMod"
#define LCBMGR_CYCLECOLOR_TAG			"CycleColor"
#define LCBMGR_CYCLETYPE_TAG			"CycleType"
#define LCBMGR_SHADE_TAG				"Shade"
#define LCBMGR_GLOBAL_TAG				"Global"
#define LCBMGR_CYCLEDIALOGPARAMS_TAG	"CycleDialogParams"
#define LCBMGR_SOUNDS_TAG				"Sounds"
#define LCBMGR_CYCLEDIALOG_TAG			"CycleDialog"
#define LCBMGR_CYCLERACE_TAG			"CycleRace"
#define LCBMGR_GAMESPEEDSCALARS_TAG		"GameSpeedScalars"
#define LCBMGR_TRAILLENGTHVALUES_TAG	"TrailLengthValues"
#define LCBMGR_TEXTIDS_TAG				"TextIds"
#define LCBMGR_POWERUP_TAG				"Powerup%d"

#define LCBMGR_BASESPEED				"BaseSpeed"
#define LCBMGR_MINSPEED					"MinSpeed"
#define LCBMGR_MAXMANUALSPEEDADD		"MaxManualSpeedAdd"
#define LCBMGR_MAXEXTERNALSPEEDADD		"MaxExternalSpeedAdd"
#define LCBMGR_MAXTURBOSPEED			"MaxTurboSpeed"
#define LCBMGR_TURBOACCELRATE			"TurboAccelRate"
#define LCBMGR_BRAKEDECELRATE			"BrakeDecelRate"
#define LCBMGR_NORMALACCELRATE			"NormalAccelRate"
#define LCBMGR_NORMALDECELRATE			"NormalDecelRate"
#define LCBMGR_MAXWALLACCELRATE			"MaxWallAccelRate"
#define LCBMGR_TURNSPEEDLOSSPERCENT		"TurnSpeedLossPercent"

#define LCBMGR_NAME						"Name"
#define LCBMGR_NAMEID					"NameId"

// RETAIL: 0x102d9a14 (the name is ours)
const char* g_aszLCDialogTypeNames[kNumLCDialogTypes] =
{
	"AISTART",
	"AICRASH",
	"AISpeedBoost",
	"PLAYERCRASH",
	"PlayerSpeedBoost",
	"StartWallFollow",
	"WALLFOLLOWINGPLAYERINSIGHT",
	"WALLFOLLOWPLAYERCLOSE",
	"JUSTPASSEDPLAYER",
	"HUNTINGPLAYER",
	"LOSTPLAYERSTILLCHASING",
	"LOSTPLAYER",
	"STUCK",
	"GOINGBETWEENWALLS",
	"PLAYERTURNSAWAY",
	"PLAYERCLOSEBEHIND",
	"PLAYERCLOSEINFRONT",
	"INVALID",
};

// RETAIL: 0x102d9a5c (the name is ours)
const char* g_aszLCPowerupNames[kNumLCPowerupTypes] =
{
	"Shield",
	"Turbo",
	"TrailSpike",
	"Missile",
	"WallExtender",
	"TurboCurse",
	"Grab",
	"TrailReset",
	"ForceUse",
	"Random",
};

// RETAIL: 0x102d9a84, right after the power-up names; nothing reads it
// (the name is ours)
const char* g_szLCPowerupTag = "Powerup";

// RETAIL: 0x10293d88, the TextIds attribute prefixes (the name is ours)
static const char* const s_aszLCTextIdNames[kNumLCTextIds] =
{
	"ActivateGrabFromCyclist",
	"ActivateGrabFromSelf",
	"ActivateGrabFromFloor",
	"ActivateForceUse",
	"ActivateTrailReset",
	"ActivateTurboCurse",
	"AnnouncePowerupGrab",
	"AnnounceWallReset",
	"AnnounceTurboCurse",
	"DerezzLocalCyclist",
	"DerezzGenericCyclist",
	"AcquireTurboCurse",
	"AcquireMissileBlaster",
	"AcquireWallReset",
	"AcquirePowerupGrab",
	"AcquireWallSpike",
	"AcquireTurboBoost",
	"AcquireWallExtender",
	"AcquireShieldBreak",
	"AcquireForceUse",
	"ActivateShieldBreak",
	"ActivateTurboBoost",
	"ActivateMissileBlaster",
	"ActivateWallSpike",
	"ActivateWallExtender",
};

// RETAIL: 0x10358b78, 0x10358bdc, 0x10358c40
static char s_aAttName[100];
static char s_aTagName[100];

CLightCycleButeMgr* g_pLightCycleButeMgr = LTNULL;

typedef std::set<std::string> LCSoundSet;

static bool LCBM_SoundKeyCallback(const char* szKeyName, CButeMgr::CSymTabItem* pItem, void* pContext);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetLCDialogTypeName()
//
//	PURPOSE:	The name of a cycle dialog situation
//
// ----------------------------------------------------------------------- //

const char* GetLCDialogTypeName(int eType)
{
	return g_aszLCDialogTypeNames[eType];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::CLightCycleButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CLightCycleButeMgr::CLightCycleButeMgr()
{
	m_nNumCycleColors = 0;
	m_pCycleColors = LTNULL;
	m_nNumCycleTypes = 0;
	m_pCycleTypes = LTNULL;

	m_fGridSize = 1.0f;
	m_fWallExtenderAdd = 1000.0f;
	m_fTurboCurseAccelRate = 1.0f;
	m_fMaxTurboCurseSpeed = 5000.0f;
	m_fTurboCurseTime = 5.0f;
	m_fTurboTime = 5.0f;
	m_fTrailSpikeSpeed = 1000.0f;
	m_fTrailSpikeDist = 400.0f;
	m_fTrailSpikeDelay = 0.0f;
	m_fTrailDerezTime = 0.5f;
	m_fTrailDerezDelay = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::~CLightCycleButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CLightCycleButeMgr::~CLightCycleButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CLightCycleButeMgr::Init(const char* szAttributeFile)
{
	if (g_pLightCycleButeMgr || !szAttributeFile) return LTFALSE;

	if (!Parse(szAttributeFile))
	{
		// RETAIL: formatted, but never shown
		char szError[512];
		LTSNPrintF(szError, sizeof(szError), "CLightCycleButeMgr::Init: Failed to parse %s", szAttributeFile);
		return LTFALSE;
	}

	g_pLightCycleButeMgr = this;

	// The cycle mods

	m_nNumCycleMods = 0;
	LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLEMOD_TAG, m_nNumCycleMods);
	while (m_buteMgr.Exist(s_aTagName))
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLEMOD_TAG, ++m_nNumCycleMods);
	}

	m_pCycleMods = debug_newa(LIGHTCYCLEMOD, m_nNumCycleMods);

	int i;
	for (i = 0; i < m_nNumCycleMods; i++)
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLEMOD_TAG, i);

		m_pCycleMods[i].fBaseSpeed				= m_buteMgr.GetFloat(s_aTagName, LCBMGR_BASESPEED, 1.0f);
		m_pCycleMods[i].fMinSpeed				= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MINSPEED, 1.0f);
		m_pCycleMods[i].fMaxManualSpeedAdd		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MAXMANUALSPEEDADD, 1.0f);
		m_pCycleMods[i].fMaxExternalSpeedAdd	= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MAXEXTERNALSPEEDADD, 1.0f);
		m_pCycleMods[i].fMaxTurboSpeed			= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MAXTURBOSPEED, 1.0f);
		m_pCycleMods[i].fTurboAccelRate			= m_buteMgr.GetFloat(s_aTagName, LCBMGR_TURBOACCELRATE, 1.0f);
		m_pCycleMods[i].fBrakeDecelRate			= m_buteMgr.GetFloat(s_aTagName, LCBMGR_BRAKEDECELRATE, 1.0f);
		m_pCycleMods[i].fNormalAccelRate		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_NORMALACCELRATE, 1.0f);
		m_pCycleMods[i].fNormalDecelRate		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_NORMALDECELRATE, 1.0f);
		m_pCycleMods[i].fMaxWallAccelRate		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MAXWALLACCELRATE, 1.0f);
		m_pCycleMods[i].fTurnSpeedLossPercent	= m_buteMgr.GetFloat(s_aTagName, LCBMGR_TURNSPEEDLOSSPERCENT, 1.0f);
	}

	// The cycle colors

	m_nNumCycleColors = 0;
	LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLECOLOR_TAG, m_nNumCycleColors);
	while (m_buteMgr.Exist(s_aTagName))
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLECOLOR_TAG, ++m_nNumCycleColors);
	}

	m_pCycleColors = debug_newa(LIGHTCYCLECOLOR, m_nNumCycleColors);

	char szTemp[256];

	for (i = 0; i < m_nNumCycleColors; i++)
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLECOLOR_TAG, i);

		LTSNPrintF(m_pCycleColors[i].szName, sizeof(m_pCycleColors[i].szName), m_buteMgr.GetString(s_aTagName, LCBMGR_NAME, ""));
		LTSNPrintF(m_pCycleColors[i].szModel, sizeof(m_pCycleColors[i].szModel), m_buteMgr.GetString(s_aTagName, "ModelFile", ""));
		m_pCycleColors[i].blrRenderStyles.Read(&m_buteMgr, s_aTagName, "RenderStyle", MAX_PATH);
		m_pCycleColors[i].blrSkins.Read(&m_buteMgr, s_aTagName, "Skin", MAX_PATH);

		m_pCycleColors[i].nRed				= (uint8)m_buteMgr.GetInt(s_aTagName, "Red", 0);
		m_pCycleColors[i].nGreen			= (uint8)m_buteMgr.GetInt(s_aTagName, "Green", 0);
		m_pCycleColors[i].nBlue				= (uint8)m_buteMgr.GetInt(s_aTagName, "Blue", 0);
		m_pCycleColors[i].nCycleDialog		= (uint8)m_buteMgr.GetInt(s_aTagName, LCBMGR_CYCLEDIALOG_TAG, 0);
		m_pCycleColors[i].bValidAIToSpawn	= m_buteMgr.GetBool(s_aTagName, "ValidAIToSpawn", false);

		m_buteMgr.GetString(s_aTagName, "HUDLivesIconTex", "", szTemp, sizeof(szTemp));
		m_pCycleColors[i].sHUDLivesIconTex = szTemp;
		m_buteMgr.GetString(s_aTagName, "HUDRosterIconTex", "", szTemp, sizeof(szTemp));
		m_pCycleColors[i].sHUDRosterIconTex = szTemp;

		m_pCycleColors[i].nCycleMod		= m_buteMgr.GetInt(s_aTagName, LCBMGR_CYCLEMOD_TAG, 0);
		m_pCycleColors[i].nCycleType	= m_buteMgr.GetInt(s_aTagName, LCBMGR_CYCLETYPE_TAG, 0);

		// DeathFX0, DeathFX1... until one is missing
		int nFX = 0;
		while (1)
		{
			char szDeathFX[128] = {0};
			LTSNPrintF(s_aAttName, sizeof(s_aAttName), "%s%d", "DeathFX", nFX);
			LTSNPrintF(szDeathFX, sizeof(szDeathFX), m_buteMgr.GetString(s_aTagName, s_aAttName, ""));
			if (!m_buteMgr.Success()) break;

			m_pCycleColors[i].aDeathFX.push_back(std::string(szDeathFX));
			nFX++;
		}

		LTSNPrintF(m_pCycleColors[i].szIntroFX, sizeof(m_pCycleColors[i].szIntroFX), m_buteMgr.GetString(s_aTagName, "IntroFX", ""));
		LTSNPrintF(m_pCycleColors[i].szRiderFX, sizeof(m_pCycleColors[i].szRiderFX), m_buteMgr.GetString(s_aTagName, "RiderFX", ""));
	}

	// The cycle types

	m_nNumCycleTypes = 0;
	LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLETYPE_TAG, m_nNumCycleTypes);
	while (m_buteMgr.Exist(s_aTagName))
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLETYPE_TAG, ++m_nNumCycleTypes);
	}

	m_pCycleTypes = debug_newa(LIGHTCYCLETYPE, m_nNumCycleTypes);

	for (i = 0; i < m_nNumCycleTypes; i++)
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLETYPE_TAG, i);

		LTSNPrintF(m_pCycleTypes[i].szName, sizeof(m_pCycleTypes[i].szName), m_buteMgr.GetString(s_aTagName, LCBMGR_NAME, ""));
		m_pCycleTypes[i].fMaxTrailLength		= m_buteMgr.GetFloat(s_aTagName, "MaxTrailLength", 0.0f);
		m_pCycleTypes[i].fBaseSpeed				= m_buteMgr.GetFloat(s_aTagName, LCBMGR_BASESPEED, 0.0f);
		m_pCycleTypes[i].fMinSpeed				= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MINSPEED, 0.0f);
		m_pCycleTypes[i].fMaxManualSpeedAdd		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MAXMANUALSPEEDADD, 0.0f);
		m_pCycleTypes[i].fMaxExternalSpeedAdd	= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MAXEXTERNALSPEEDADD, 0.0f);
		m_pCycleTypes[i].fMaxTurboSpeed			= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MAXTURBOSPEED, 0.0f);
		m_pCycleTypes[i].fTurboAccelRate		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_TURBOACCELRATE, 0.0f);
		m_pCycleTypes[i].fBrakeDecelRate		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_BRAKEDECELRATE, 0.0f);
		m_pCycleTypes[i].fNormalAccelRate		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_NORMALACCELRATE, 0.0f);
		m_pCycleTypes[i].fNormalDecelRate		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_NORMALDECELRATE, 0.0f);
		m_pCycleTypes[i].fMaxWallAccelRate		= m_buteMgr.GetFloat(s_aTagName, LCBMGR_MAXWALLACCELRATE, 0.0f);
		m_pCycleTypes[i].fTurnSpeedLossPercent	= m_buteMgr.GetFloat(s_aTagName, LCBMGR_TURNSPEEDLOSSPERCENT, 0.0f);
		m_pCycleTypes[i].nMaxWallDist			= m_buteMgr.GetInt(s_aTagName, "MaxWallDist", 0);
		m_pCycleTypes[i].fHalfLength			= m_buteMgr.GetFloat(s_aTagName, "HalfLength", 0.0f);
		m_pCycleTypes[i].fHalfWidth				= m_buteMgr.GetFloat(s_aTagName, "HalfWidth", 0.0f);
		m_pCycleTypes[i].fHalfHeight			= m_buteMgr.GetFloat(s_aTagName, "HalfHeight", 0.0f);
		m_pCycleTypes[i].fPlayerNoticeDist		= m_buteMgr.GetFloat(s_aTagName, "PlayerNoticeDist", 8000.0f);
		m_pCycleTypes[i].fRightSensorDist		= m_buteMgr.GetFloat(s_aTagName, "RightSensorDist", 65.0f);
		m_pCycleTypes[i].fLeftSensorDist		= m_buteMgr.GetFloat(s_aTagName, "LeftSensorDist", 65.0f);
		m_pCycleTypes[i].fFrontSensorDist		= m_buteMgr.GetFloat(s_aTagName, "FrontSensorDist", 65.0f);

		LTSNPrintF(m_pCycleTypes[i].szIntroSound, sizeof(m_pCycleTypes[i].szIntroSound), m_buteMgr.GetString(s_aTagName, "IntroSound", ""));
		LTSNPrintF(m_pCycleTypes[i].szTurnSound, sizeof(m_pCycleTypes[i].szTurnSound), m_buteMgr.GetString(s_aTagName, "TurnSound", ""));
		LTSNPrintF(m_pCycleTypes[i].szCruiseSound, sizeof(m_pCycleTypes[i].szCruiseSound), m_buteMgr.GetString(s_aTagName, "CruiseSound", ""));
		LTSNPrintF(m_pCycleTypes[i].szAltCruiseSound, sizeof(m_pCycleTypes[i].szAltCruiseSound), m_buteMgr.GetString(s_aTagName, "AltCruiseSound", ""));
	}

	// The shades

	m_nNumShades = 0;
	LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_SHADE_TAG, m_nNumShades);
	while (m_buteMgr.Exist(s_aTagName))
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_SHADE_TAG, ++m_nNumShades);
	}

	m_pShades = debug_newa(LIGHTCYCLESHADE, m_nNumShades);

	uint32 nShade;
	for (nShade = 0; nShade < m_nNumShades; nShade++)
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_SHADE_TAG, nShade);

		m_pShades[nShade].nNameId		= m_buteMgr.GetInt(s_aTagName, LCBMGR_NAMEID);
		m_pShades[nShade].nLockedNameId	= m_buteMgr.GetInt(s_aTagName, "LockedNameId");
		SAFE_STRCPY(m_pShades[nShade].szCycleColors[kLCShade_Regular], m_buteMgr.GetString(s_aTagName, "Regular", ""));
		SAFE_STRCPY(m_pShades[nShade].szCycleColors[kLCShade_Super], m_buteMgr.GetString(s_aTagName, "Super", ""));
		m_pShades[nShade].bMultiOnly	= !!m_buteMgr.GetInt(s_aTagName, "MultiOnly", 0);
	}

	// The globals

	m_fGridSize = m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "GridSize", 1.0f);

	m_afLCDifficultyModifiers[0]	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "LCDifficultyModifierEasy", 0.0f);
	m_afLCDifficultyModifiers[1]	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "LCDifficultyModifierNormal", 0.33f);
	m_afLCDifficultyModifiers[2]	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "LCDifficultyModifierHard", 0.66f);
	m_afLCDifficultyModifiers[3]	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "LCDifficultyModifierVeryHard", 1.0f);

	m_fWallExtenderAdd		= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "WallExtenderAdd", 1.0f);
	m_fTurboCurseAccelRate	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "TurboCurseAccelRate", 1.0f);
	m_fMaxTurboCurseSpeed	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "MaxTurboCurseSpeed", 1.0f);
	m_fTurboCurseTime		= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "TurboCurseTime", 1.0f);
	m_fTurboTime			= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "TurboTime", 1.0f);
	m_fTrailSpikeSpeed		= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "TrailSpikeSpeed", 1.0f);
	m_fTrailSpikeDist		= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "TrailSpikeDist", 1.0f);
	m_fTrailSpikeDelay		= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "TrailSpikeDelay", 1.0f);
	m_fTrailDerezTime		= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "TrailDerezTime", 1.0f);
	m_fTrailDerezDelay		= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "TrailDerezDelay", 1.0f);
	m_fSpeedZoneAccelRate	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "SpeedZoneAccelRate", 1.0f);
	m_fMaxSpeedZoneSpeed	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "MaxSpeedZoneSpeed", 1.0f);
	m_fSlowZoneDecelRate	= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "SlowZoneDecelRate", 1.0f);
	m_fMinSlowZoneSpeed		= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "MinSlowZoneSpeed", 1.0f);
	m_fMissileSpeed			= m_buteMgr.GetFloat(LCBMGR_GLOBAL_TAG, "MissileSpeed", 1.0f);

	// The cycle dialog parameters

	if (m_buteMgr.Exist(LCBMGR_CYCLEDIALOGPARAMS_TAG))
	{
		CARange rngTime = m_buteMgr.GetRange(LCBMGR_CYCLEDIALOGPARAMS_TAG, "TimeRangeBetweenCycleDialog");
		m_fMinTimeBetweenDialog = (LTFLOAT)rngTime.GetMin();
		m_fMaxTimeBetweenDialog = (LTFLOAT)rngTime.GetMax();

		m_bDialogSoundLocalToPlayer	= m_buteMgr.GetBool(LCBMGR_CYCLEDIALOGPARAMS_TAG, "SoundLocalToPlayer", true);
		m_bModifyPriorityWithDist	= m_buteMgr.GetBool(LCBMGR_CYCLEDIALOGPARAMS_TAG, "ModifyPriorityWithDist", false);

		LTFLOAT fRadius = m_buteMgr.GetFloat(LCBMGR_CYCLEDIALOGPARAMS_TAG, "SoundRadius", 10000000.0f);
		m_fDialogSoundRadius = fRadius;
		m_fDialogMaxDist = m_buteMgr.GetFloat(LCBMGR_CYCLEDIALOGPARAMS_TAG, "MaxDist", fRadius);

		m_bEnableSoundDebugInfo = m_buteMgr.GetBool(LCBMGR_CYCLEDIALOGPARAMS_TAG, "EnableSoundDebugInfo", false);
	}

	// The cycle dialogs: the sounds of the Sounds tag...

	char szSoundsTag[] = LCBMGR_SOUNDS_TAG;

	LCSoundSet setSounds;
	m_buteMgr.GetKeys(LCBMGR_SOUNDS_TAG, LCBM_SoundKeyCallback, &setSounds);

	m_nNumCycleDialogs = 0;
	char szSoundFormat[] = "voice\\%d.wav";

	LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLEDIALOG_TAG, m_nNumCycleDialogs);
	while (m_buteMgr.Exist(s_aTagName))
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLEDIALOG_TAG, ++m_nNumCycleDialogs);
	}

	m_pCycleDialogs = debug_newa(LIGHTCYCLEDIALOG, m_nNumCycleDialogs);

	for (i = 0; i < m_nNumCycleDialogs; i++)
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLEDIALOG_TAG, i);

		// ...each situation's: <Type>0, <Type>1... name sounds of the Sounds
		// tag, whose ranges give the voice file numbers
		for (int nType = 0; nType < kNumLCDialogTypes; nType++)
		{
			const char* pszType = g_aszLCDialogTypeNames[nType];

			int nSound = 0;
			LTSNPrintF(s_aAttName, sizeof(s_aAttName), "%s%d", pszType, nSound);
			while (m_buteMgr.Exist(s_aTagName, s_aAttName))
			{
				const char* pszSound = m_buteMgr.GetString(s_aTagName, s_aAttName);
				if (setSounds.find(pszSound) != setSounds.end())
				{
					CARange rngVoice = m_buteMgr.GetRange(szSoundsTag, pszSound);

					// RETAIL: the first dialog takes the range's min, the
					// others its max
					char szSoundFile[256];
					LTSNPrintF(szSoundFile, sizeof(szSoundFile), szSoundFormat, (int)(i == 0 ? rngVoice.GetMin() : rngVoice.GetMax()));

					m_pCycleDialogs[i].aTypes[nType].aSounds.push_back(std::string(szSoundFile));
					m_pCycleDialogs[i].aTypes[nType].eType = nType;
				}

				LTSNPrintF(s_aAttName, sizeof(s_aAttName), "%s%d", pszType, ++nSound);
			}

			LTSNPrintF(s_aAttName, sizeof(s_aAttName), "%s%s", pszType, "Priority");
			LTFLOAT fPriority = m_buteMgr.GetFloat(s_aTagName, s_aAttName, 1.0f);

			// RETAIL: && for ||, the error is never printed (docs/RETAIL_BUGS.md)
			if (fPriority < 0.0f && fPriority > 1.0f)
			{
				g_pLTServer->CPrint("ERROR: Priority value %f < 0 or %f > 1 for %s", fPriority, fPriority, s_aAttName);
			}

			m_pCycleDialogs[i].aTypes[nType].fPriority = fPriority;
		}
	}

	// A color's dialog must exist

	for (i = 0; i < m_nNumCycleColors; i++)
	{
		if (m_pCycleColors[i].nCycleDialog > m_nNumCycleDialogs)
		{
			m_pCycleColors[i].nCycleDialog = 0;
		}
	}

	// The cycle races

	m_nNumCycleRaces = 0;
	LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLERACE_TAG, m_nNumCycleRaces);
	while (m_buteMgr.Exist(s_aTagName))
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLERACE_TAG, ++m_nNumCycleRaces);
	}

	m_pCycleRaces = debug_newa(LIGHTCYCLERACE, m_nNumCycleRaces);

	for (i = 0; i < m_nNumCycleRaces; i++)
	{
		LTSNPrintF(s_aTagName, sizeof(s_aTagName), "%s%d", LCBMGR_CYCLERACE_TAG, i);
		LTSNPrintF(m_pCycleRaces[i].szMap, sizeof(m_pCycleRaces[i].szMap), m_buteMgr.GetString(s_aTagName, "Map", ""));
	}

	// The game speed scalars and the trail lengths

	char szValue[64];
	for (i = 0; i < LCBMGR_NUM_VALUES; i++)
	{
		sprintf(szValue, "Value%d", i);
		m_afGameSpeedScalars[i] = m_buteMgr.GetFloat(LCBMGR_GAMESPEEDSCALARS_TAG, szValue, 1.0f);
	}

	for (i = 0; i < LCBMGR_NUM_VALUES; i++)
	{
		sprintf(szValue, "Value%d", i);
		m_afTrailLengthValues[i] = m_buteMgr.GetFloat(LCBMGR_TRAILLENGTHVALUES_TAG, szValue, 30000.0f);
	}

	// The text ids: <Name>0, <Name>1... until one is missing

	for (int nText = 0; nText < kNumLCTextIds; nText++)
	{
		int nId = 0;
		while (1)
		{
			LTSNPrintF(s_aAttName, sizeof(s_aAttName), "%s%d", s_aszLCTextIdNames[nText], nId);
			int nTextId = m_buteMgr.GetInt(LCBMGR_TEXTIDS_TAG, s_aAttName);
			if (!m_buteMgr.Success()) break;

			m_aTextIds[nText].push_back(nTextId);
			nId++;
		}
	}

	// The power-ups

	int nPowerup = 0;
	LTSNPrintF(s_aTagName, sizeof(s_aTagName), LCBMGR_POWERUP_TAG, nPowerup);
	while (m_buteMgr.Exist(s_aTagName, LCBMGR_NAME) && m_buteMgr.Exist(s_aTagName, LCBMGR_NAMEID))
	{
		int nActivate = kNumLCTextIds;

		const char* pszName = m_buteMgr.GetString(s_aTagName, LCBMGR_NAME);
		int nNameId = m_buteMgr.GetInt(s_aTagName, LCBMGR_NAMEID);
		const char* pszActivate = m_buteMgr.GetString(s_aTagName, "ActivateId");

		for (int nText = 0; nText < kNumLCTextIds; nText++)
		{
			if (!_stricmp(pszActivate, s_aszLCTextIdNames[nText]))
			{
				nActivate = nText;
				break;
			}
		}

		m_mapPowerups.insert(LCPowerupMap::value_type(CaselessString(pszName), LCPOWERUPDATA(nNameId, nActivate)));

		LTSNPrintF(s_aTagName, sizeof(s_aTagName), LCBMGR_POWERUP_TAG, ++nPowerup);
	}

	m_buteMgr.Term();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LCBM_SoundKeyCallback()
//
//	PURPOSE:	Collects the keys of the Sounds tag (RETAIL: 0x10027b20)
//
// ----------------------------------------------------------------------- //

static bool LCBM_SoundKeyCallback(const char* szKeyName, CButeMgr::CSymTabItem* pItem, void* pContext)
{
	LCSoundSet* pSet = (LCSoundSet*)pContext;
	pSet->insert(std::string(szKeyName));
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::Term()
//
//	PURPOSE:	Clean up
//
// ----------------------------------------------------------------------- //

void CLightCycleButeMgr::Term()
{
	if (m_pCycleColors)
	{
		debug_deletea(m_pCycleColors);
		m_pCycleColors = LTNULL;
	}
	m_nNumCycleColors = 0;

	if (m_pCycleTypes)
	{
		debug_deletea(m_pCycleTypes);
		m_pCycleTypes = LTNULL;
	}
	m_nNumCycleTypes = 0;

	if (m_pCycleDialogs)
	{
		debug_deletea(m_pCycleDialogs);
		m_pCycleDialogs = LTNULL;
	}
	m_nNumCycleDialogs = 0;

	if (m_pCycleRaces)
	{
		debug_deletea(m_pCycleRaces);
		m_pCycleRaces = LTNULL;
	}
	m_nNumCycleRaces = 0;

	if (m_pCycleMods)
	{
		debug_deletea(m_pCycleMods);
		m_pCycleMods = LTNULL;
	}
	m_nNumCycleMods = 0;

	if (m_pShades)
	{
		debug_deletea(m_pShades);
		m_pShades = LTNULL;
	}
	m_nNumShades = 0;

	g_pLightCycleButeMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetCycleType()
//
// ----------------------------------------------------------------------- //

LIGHTCYCLETYPE* CLightCycleButeMgr::GetCycleType(int nIndex)
{
	return &m_pCycleTypes[nIndex];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetCycleColor()
//
//	PURPOSE:	The color of that name
//
// ----------------------------------------------------------------------- //

LIGHTCYCLECOLOR* CLightCycleButeMgr::GetCycleColor(const char* szName)
{
	for (int i = 0; i < m_nNumCycleColors; i++)
	{
		if (!_stricmp(m_pCycleColors[i].szName, szName))
		{
			return &m_pCycleColors[i];
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetCycleMod()
//
// ----------------------------------------------------------------------- //

LIGHTCYCLEMOD* CLightCycleButeMgr::GetCycleMod(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_nNumCycleMods) return LTNULL;

	return &m_pCycleMods[nIndex];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetNumCycleColors()
//
// ----------------------------------------------------------------------- //

int CLightCycleButeMgr::GetNumCycleColors()
{
	return m_nNumCycleColors;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetCycleColor()
//
//	PURPOSE:	The color of that index
//
// ----------------------------------------------------------------------- //

LIGHTCYCLECOLOR* CLightCycleButeMgr::GetCycleColor(uint32 nIndex)
{
	if (nIndex < (uint32)GetNumCycleColors())
	{
		return &m_pCycleColors[nIndex];
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetCycleDialog()
//
// ----------------------------------------------------------------------- //

LIGHTCYCLEDIALOG* CLightCycleButeMgr::GetCycleDialog(uint8 nIndex)
{
	return &m_pCycleDialogs[nIndex];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetLCDifficultyModifier()
//
// ----------------------------------------------------------------------- //

LTFLOAT CLightCycleButeMgr::GetLCDifficultyModifier(int nDifficulty)
{
	return m_afLCDifficultyModifiers[nDifficulty];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetValidAICycleColors()
//
//	PURPOSE:	The colors an AI may spawn with
//
// ----------------------------------------------------------------------- //

int CLightCycleButeMgr::GetValidAICycleColors(int nCycleType, int nCycleMod, LCIndexArray & aColors)
{
	for (int i = 0; i < m_nNumCycleColors; i++)
	{
		if (m_pCycleColors[i].bValidAIToSpawn &&
			m_pCycleColors[i].nCycleMod == nCycleMod &&
			m_pCycleColors[i].nCycleType == nCycleType)
		{
			aColors.push_back(i);
		}
	}

	return aColors.size();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetNumShades()
//
// ----------------------------------------------------------------------- //

uint32 CLightCycleButeMgr::GetNumShades()
{
	return m_nNumShades;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetShade()
//
//	PURPOSE:	The shade of that index (an empty one if there is none)
//
// ----------------------------------------------------------------------- //

LIGHTCYCLESHADE* CLightCycleButeMgr::GetShade(uint32 nShade)
{
	if (nShade >= m_nNumShades)
	{
		static LIGHTCYCLESHADE s_Shade;
		return &s_Shade;
	}

	return &m_pShades[nShade];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetShadeCycleColor()
//
//	PURPOSE:	The regular or super color of a shade
//
// ----------------------------------------------------------------------- //

LIGHTCYCLECOLOR* CLightCycleButeMgr::GetShadeCycleColor(uint8 nStyle, uint8 nShade)
{
	if (nShade >= GetNumShades()) return LTNULL;
	if (nStyle >= kNumLCShadeStyles) return LTNULL;

	return GetCycleColor(GetShade(nShade)->szCycleColors[nStyle]);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetGameSpeedScalar()
//
// ----------------------------------------------------------------------- //

LTFLOAT CLightCycleButeMgr::GetGameSpeedScalar(int nIndex)
{
	return m_afGameSpeedScalars[nIndex];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleButeMgr::GetTrailLengthValue()
//
// ----------------------------------------------------------------------- //

LTFLOAT CLightCycleButeMgr::GetTrailLengthValue(int nIndex)
{
	return m_afTrailLengthValues[nIndex];
}
