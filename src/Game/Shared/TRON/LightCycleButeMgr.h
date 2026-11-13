// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleButeMgr.h
//
// PURPOSE : The light cycles' attributes (Attributes\LightCycles.txt)
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x100257e0..0x10028110 and the template code the TU emits up
// to 0x10029c80, vtable 0x10293df0). The class name, the file name and the
// bute tags/attributes are retail's; the module, the structs, enums,
// members and functions are named by us. CShell.dll has its own copy of
// this class.
//
// RETAIL: layout. Tron's CButeMgr is 0x40 bytes smaller than ours, so every
// member of CLightCycleButeMgr is 0x40 further in our build than retail's
// offsets given below (retail sizeof(CLightCycleButeMgr) = 0x130c).
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEBUTEMGR_H__
#define __LIGHTCYCLEBUTEMGR_H__

#include "GameButeMgr.h"
#include "ButeListReader.h"
#include "CaselessString.h"
#include <vector>
#include <map>
#include <string>

#define LCBMGR_DEFAULT_FILE		"Attributes\\LightCycles.txt"

// RETAIL: string literals of the power-up sounds (retail data 0x102d9a88..,
// used by the light cycle power-up code at 0x1002a8e8, 0x1002b5b8...). The
// names are ours.
#define LCPOWERUP_SOUND_PREFIX	"WORLDS_lcpowerup_"
#define LCPOWERUP_SOUND_TOUCH	"_touch"
#define LCPOWERUP_SOUND_PICKUP	"_pickup"
#define LCPOWERUP_SOUND_ACTIVE	"_active"
#define LCPOWERUP_SOUND_REMOVE	"_remove"

class CLightCycleButeMgr;
extern CLightCycleButeMgr* g_pLightCycleButeMgr;	// RETAIL: 0x10358c40

// ----------------------------------------------------------------------- //
// The power-up types (the enum and its names are ours; the order is that of
// the retail name table g_aszLCPowerupNames, 0x102d9a5c).
// ----------------------------------------------------------------------- //
enum LCPowerupType
{
	kLCPowerup_Shield = 0,
	kLCPowerup_Turbo,
	kLCPowerup_TrailSpike,
	kLCPowerup_Missile,
	kLCPowerup_WallExtender,
	kLCPowerup_TurboCurse,
	kLCPowerup_Grab,
	kLCPowerup_TrailReset,
	kLCPowerup_ForceUse,
	kLCPowerup_Random,

	kNumLCPowerupTypes
};

// RETAIL: 0x102d9a5c, "Shield".."Random" (the name is ours)
extern const char* g_aszLCPowerupNames[kNumLCPowerupTypes];

// ----------------------------------------------------------------------- //
// The cycle dialog (AI rider chatter) situations: the enum and its names are
// ours; the order is that of the retail name table 0x102d9a14, whose names
// are also the bute attribute prefixes of a CycleDialog<n> tag.
// ----------------------------------------------------------------------- //
enum LCDialogType
{
	kLCDialog_AIStart = 0,
	kLCDialog_AICrash,
	kLCDialog_AISpeedBoost,
	kLCDialog_PlayerCrash,
	kLCDialog_PlayerSpeedBoost,
	kLCDialog_StartWallFollow,
	kLCDialog_WallFollowingPlayerInSight,
	kLCDialog_WallFollowPlayerClose,
	kLCDialog_JustPassedPlayer,
	kLCDialog_HuntingPlayer,
	kLCDialog_LostPlayerStillChasing,
	kLCDialog_LostPlayer,
	kLCDialog_Stuck,
	kLCDialog_GoingBetweenWalls,
	kLCDialog_PlayerTurnsAway,
	kLCDialog_PlayerCloseBehind,
	kLCDialog_PlayerCloseInFront,
	kLCDialog_Invalid,

	kNumLCDialogTypes
};

// RETAIL: 0x102d9a14, "AISTART".."INVALID" (the name is ours)
extern const char* g_aszLCDialogTypeNames[kNumLCDialogTypes];

// RETAIL: 0x100257e0 (the name is ours)
const char* GetLCDialogTypeName(int eType);

// ----------------------------------------------------------------------- //
// The text ids (TextIds tag): the enum and its names are ours; the order is
// that of the retail name table 0x10293d88.
// ----------------------------------------------------------------------- //
enum LCTextId
{
	kLCText_ActivateGrabFromCyclist = 0,
	kLCText_ActivateGrabFromSelf,
	kLCText_ActivateGrabFromFloor,
	kLCText_ActivateForceUse,
	kLCText_ActivateTrailReset,
	kLCText_ActivateTurboCurse,
	kLCText_AnnouncePowerupGrab,
	kLCText_AnnounceWallReset,
	kLCText_AnnounceTurboCurse,
	kLCText_DerezzLocalCyclist,
	kLCText_DerezzGenericCyclist,
	kLCText_AcquireTurboCurse,
	kLCText_AcquireMissileBlaster,
	kLCText_AcquireWallReset,
	kLCText_AcquirePowerupGrab,
	kLCText_AcquireWallSpike,
	kLCText_AcquireTurboBoost,
	kLCText_AcquireWallExtender,
	kLCText_AcquireShieldBreak,
	kLCText_AcquireForceUse,
	kLCText_ActivateShieldBreak,
	kLCText_ActivateTurboBoost,
	kLCText_ActivateMissileBlaster,
	kLCText_ActivateWallSpike,
	kLCText_ActivateWallExtender,

	kNumLCTextIds
};

// The number of GameSpeedScalars/TrailLengthValues values and of the
// difficulty modifiers (ours)
#define LCBMGR_NUM_VALUES		4

// ----------------------------------------------------------------------- //
// CycleMod<n>: speed modifiers (0x2c bytes; the names are ours)
// ----------------------------------------------------------------------- //
struct LIGHTCYCLEMOD
{
	LTFLOAT	fBaseSpeed;				// RETAIL: +0x00
	LTFLOAT	fMinSpeed;				// RETAIL: +0x04
	LTFLOAT	fMaxTurboSpeed;			// RETAIL: +0x08
	LTFLOAT	fTurboAccelRate;		// RETAIL: +0x0c
	LTFLOAT	fBrakeDecelRate;		// RETAIL: +0x10
	LTFLOAT	fMaxExternalSpeedAdd;	// RETAIL: +0x14
	LTFLOAT	fMaxManualSpeedAdd;		// RETAIL: +0x18
	LTFLOAT	fNormalAccelRate;		// RETAIL: +0x1c
	LTFLOAT	fNormalDecelRate;		// RETAIL: +0x20
	LTFLOAT	fMaxWallAccelRate;		// RETAIL: +0x24
	LTFLOAT	fTurnSpeedLossPercent;	// RETAIL: +0x28
};

// ----------------------------------------------------------------------- //
// CycleColor<n>: a cycle's look (0x124 bytes; the names are ours)
// ----------------------------------------------------------------------- //
struct LIGHTCYCLECOLOR
{
	char			szName[32];				// RETAIL: +0x00
	char			szModel[64];			// RETAIL: +0x20 ModelFile
	CButeListReader	blrSkins;				// RETAIL: +0x60
	CButeListReader	blrRenderStyles;		// RETAIL: +0x68
	uint8			nRed;					// RETAIL: +0x70
	uint8			nGreen;					// RETAIL: +0x71
	uint8			nBlue;					// RETAIL: +0x72
	char			szIntroFX[64];			// RETAIL: +0x73
	std::vector<std::string> aDeathFX;		// RETAIL: +0xb4 DeathFX<n>
	char			szRiderFX[64];			// RETAIL: +0xc0
	uint8			nCycleDialog;			// RETAIL: +0x100
	bool			bValidAIToSpawn;		// RETAIL: +0x101
	CaselessString	sHUDLivesIconTex;		// RETAIL: +0x104
	CaselessString	sHUDRosterIconTex;		// RETAIL: +0x110
	int				nCycleMod;				// RETAIL: +0x11c
	int				nCycleType;				// RETAIL: +0x120
};

// ----------------------------------------------------------------------- //
// CycleType<n>: a cycle's handling, size and sounds (0x170 bytes; the names
// are ours)
// ----------------------------------------------------------------------- //
struct LIGHTCYCLETYPE
{
	char	szName[32];				// RETAIL: +0x00
	LTFLOAT	fMaxTrailLength;		// RETAIL: +0x20
	LTFLOAT	fBaseSpeed;				// RETAIL: +0x24
	LTFLOAT	fMinSpeed;				// RETAIL: +0x28
	LTFLOAT	fMaxManualSpeedAdd;		// RETAIL: +0x2c
	LTFLOAT	fMaxExternalSpeedAdd;	// RETAIL: +0x30
	LTFLOAT	fMaxTurboSpeed;			// RETAIL: +0x34
	LTFLOAT	fTurboAccelRate;		// RETAIL: +0x38
	LTFLOAT	fBrakeDecelRate;		// RETAIL: +0x3c
	LTFLOAT	fNormalAccelRate;		// RETAIL: +0x40
	LTFLOAT	fNormalDecelRate;		// RETAIL: +0x44
	LTFLOAT	fMaxWallAccelRate;		// RETAIL: +0x48
	LTFLOAT	fTurnSpeedLossPercent;	// RETAIL: +0x4c
	int		nMaxWallDist;			// RETAIL: +0x50
	LTFLOAT	fHalfLength;			// RETAIL: +0x54
	LTFLOAT	fHalfWidth;				// RETAIL: +0x58
	LTFLOAT	fHalfHeight;			// RETAIL: +0x5c
	LTFLOAT	fPlayerNoticeDist;		// RETAIL: +0x60
	LTFLOAT	fRightSensorDist;		// RETAIL: +0x64
	LTFLOAT	fLeftSensorDist;		// RETAIL: +0x68
	LTFLOAT	fFrontSensorDist;		// RETAIL: +0x6c
	char	szIntroSound[64];		// RETAIL: +0x70
	char	szTurnSound[64];		// RETAIL: +0xb0
	char	szCruiseSound[64];		// RETAIL: +0xf0
	char	szAltCruiseSound[64];	// RETAIL: +0x130
};

// ----------------------------------------------------------------------- //
// One situation of a CycleDialog<n> (0x14 bytes; the names are ours)
// ----------------------------------------------------------------------- //
struct LCDIALOGSOUNDS
{
	// RETAIL: the temporary in 0x100279e0
	LCDIALOGSOUNDS()
	{
		fPriority = 0.0f;
		eType = kLCDialog_Invalid;
	}

	std::vector<std::string>	aSounds;	// RETAIL: +0x00 "voice\<n>.wav"
	LTFLOAT						fPriority;	// RETAIL: +0x0c <Type>Priority
	int							eType;		// RETAIL: +0x10 LCDialogType
};

// CycleDialog<n>: a rider's voice (0xc bytes; the name is ours)
struct LIGHTCYCLEDIALOG
{
	// RETAIL: 0x100279e0
	LIGHTCYCLEDIALOG() : aTypes(kNumLCDialogTypes) {}

	std::vector<LCDIALOGSOUNDS>	aTypes;		// Indexed by LCDialogType
};

// CycleRace<n> (0x20 bytes; the name is ours)
struct LIGHTCYCLERACE
{
	char	szMap[32];
};

// The cycle color of a shade (indices of SHADE::szCycleColors, ours)
enum LCShadeStyle
{
	kLCShade_Regular = 0,
	kLCShade_Super,

	kNumLCShadeStyles
};

// ----------------------------------------------------------------------- //
// Shade<n>: a pair of cycle colors (0x4c bytes; the names are ours)
// ----------------------------------------------------------------------- //
struct LIGHTCYCLESHADE
{
	// RETAIL: inlined at 0x10026858, 0x1002802a
	LIGHTCYCLESHADE()
	{
		nNameId = 0;
		nLockedNameId = 0;
		// RETAIL: clears the first two chars of the Regular color, not the
		// first of each color (docs/RETAIL_BUGS.md)
		szCycleColors[0][0] = '\0';
		szCycleColors[0][1] = '\0';
		bMultiOnly = false;
	}

	int		nNameId;							// RETAIL: +0x00
	int		nLockedNameId;						// RETAIL: +0x04
	char	szCycleColors[kNumLCShadeStyles][32];// RETAIL: +0x08 Regular, +0x28 Super
	bool	bMultiOnly;							// RETAIL: +0x48
};

// Powerup<n>: (the names are ours)
struct LCPOWERUPDATA
{
	LCPOWERUPDATA(int nName, int nActivate)
	{
		nNameId = nName;
		nActivateTextId = nActivate;
	}

	int		nNameId;
	int		nActivateTextId;	// LCTextId (kNumLCTextIds if none)
};

typedef std::map<CaselessString, LCPOWERUPDATA> LCPowerupMap;
typedef std::vector<int> LCIntArray;
typedef std::vector<uint32> LCIndexArray;

// ----------------------------------------------------------------------- //
// CLightCycleButeMgr (retail RTTI .?AVCLightCycleButeMgr@@)
// ----------------------------------------------------------------------- //
class CLightCycleButeMgr : public CGameButeMgr
{
	public :

		CLightCycleButeMgr();				// RETAIL: 0x100257f0
		~CLightCycleButeMgr();				// RETAIL: 0x10025970

		LTBOOL	Init(const char* szAttributeFile=LCBMGR_DEFAULT_FILE);	// RETAIL: 0x10025b30
		void	Term();												// RETAIL: 0x10027d30

		LIGHTCYCLETYPE*		GetCycleType(int nIndex);			// RETAIL: 0x10027e30
		LIGHTCYCLECOLOR*	GetCycleColor(const char* szName);	// RETAIL: 0x10027e50
		LIGHTCYCLEMOD*		GetCycleMod(int nIndex);			// RETAIL: 0x10027ec0
		int					GetNumCycleColors();				// RETAIL: 0x10027ef0
		LIGHTCYCLECOLOR*	GetCycleColor(uint32 nIndex);		// RETAIL: 0x10027f00
		LIGHTCYCLEDIALOG*	GetCycleDialog(uint8 nIndex);		// RETAIL: 0x10027f30
		LTFLOAT				GetLCDifficultyModifier(int nDifficulty);	// RETAIL: 0x10027f50

		// Fills aColors with the indices of the colors an AI may spawn with
		// for the type and the mod; returns their number. RETAIL: 0x10027f60
		int					GetValidAICycleColors(int nCycleType, int nCycleMod, LCIndexArray & aColors);

		uint32				GetNumShades();						// RETAIL: 0x10028000
		LIGHTCYCLESHADE*	GetShade(uint32 nShade);			// RETAIL: 0x10028010
		LIGHTCYCLECOLOR*	GetShadeCycleColor(uint8 nStyle, uint8 nShade);	// RETAIL: 0x10028090
		LTFLOAT				GetGameSpeedScalar(int nIndex);		// RETAIL: 0x100280f0
		LTFLOAT				GetTrailLengthValue(int nIndex);	// RETAIL: 0x10028100

#ifdef _CLIENTBUILD
		// The client's (CShell.dll only; the names are ours):
		// a random string id of the TextIds list, 0 if none. RETAIL: CShell 0x10045310
		int					GetRandomTextId(int eTextId);
		// The power-up's data (a default one if the bute file has none).
		// RETAIL: CShell 0x10045350, 0x10045440
		LCPOWERUPDATA*		GetPowerupData(int nPowerup);
		LCPOWERUPDATA*		GetPowerupData(CaselessString sName);
#endif

		int					GetNumCycleTypes()	const { return m_nNumCycleTypes; }
		int					GetNumCycleMods()	const { return m_nNumCycleMods; }
		int					GetNumCycleDialogs()const { return m_nNumCycleDialogs; }
		int					GetNumCycleRaces()	const { return m_nNumCycleRaces; }
		LIGHTCYCLERACE*		GetCycleRace(int nIndex) { return &m_pCycleRaces[nIndex]; }
		LCIntArray &		GetTextIds(int eTextId) { return m_aTextIds[eTextId]; }
		LCPowerupMap &		GetPowerups() { return m_mapPowerups; }

		LTFLOAT	GetGridSize()				const { return m_fGridSize; }
		LTFLOAT	GetWallExtenderAdd()		const { return m_fWallExtenderAdd; }
		LTFLOAT	GetTurboCurseAccelRate()	const { return m_fTurboCurseAccelRate; }
		LTFLOAT	GetMaxTurboCurseSpeed()		const { return m_fMaxTurboCurseSpeed; }
		LTFLOAT	GetTurboCurseTime()			const { return m_fTurboCurseTime; }
		LTFLOAT	GetTurboTime()				const { return m_fTurboTime; }
		LTFLOAT	GetTrailSpikeSpeed()		const { return m_fTrailSpikeSpeed; }
		LTFLOAT	GetTrailSpikeDist()			const { return m_fTrailSpikeDist; }
		LTFLOAT	GetTrailSpikeDelay()		const { return m_fTrailSpikeDelay; }
		LTFLOAT	GetTrailDerezTime()			const { return m_fTrailDerezTime; }
		LTFLOAT	GetTrailDerezDelay()		const { return m_fTrailDerezDelay; }
		LTFLOAT	GetMaxSpeedZoneSpeed()		const { return m_fMaxSpeedZoneSpeed; }
		LTFLOAT	GetMinSlowZoneSpeed()		const { return m_fMinSlowZoneSpeed; }
		LTFLOAT	GetMissileSpeed()			const { return m_fMissileSpeed; }
		LTFLOAT	GetSpeedZoneAccelRate()		const { return m_fSpeedZoneAccelRate; }
		LTFLOAT	GetSlowZoneDecelRate()		const { return m_fSlowZoneDecelRate; }

		// CycleDialogParams (RETAIL: read by the server's light cycle
		// manager, 0x100500ce, 0x1004f67f, 0x1004f906)
		LTFLOAT	GetMinTimeBetweenDialog()	const { return m_fMinTimeBetweenDialog; }
		LTFLOAT	GetMaxTimeBetweenDialog()	const { return m_fMaxTimeBetweenDialog; }
		bool	GetModifyPriorityWithDist()	const { return m_bModifyPriorityWithDist; }
		LTFLOAT	GetDialogMaxDist()			const { return m_fDialogMaxDist; }
		LTFLOAT	GetDialogSoundRadius()		const { return m_fDialogSoundRadius; }
		bool	GetDialogSoundLocalToPlayer() const { return m_bDialogSoundLocalToPlayer; }
		bool	GetEnableSoundDebugInfo()	const { return m_bEnableSoundDebugInfo; }

	protected :

		// RETAIL: offsets are retail's (ours are 0x40 more, see above)
		int					m_nNumCycleColors;			// RETAIL: +0x111c
		LIGHTCYCLECOLOR*	m_pCycleColors;				// RETAIL: +0x1120
		int					m_nNumCycleTypes;			// RETAIL: +0x1124
		LIGHTCYCLETYPE*		m_pCycleTypes;				// RETAIL: +0x1128
		int					m_nNumCycleMods;			// RETAIL: +0x112c
		LIGHTCYCLEMOD*		m_pCycleMods;				// RETAIL: +0x1130

		// Global
		LTFLOAT				m_fGridSize;				// RETAIL: +0x1134
		LTFLOAT				m_fWallExtenderAdd;			// RETAIL: +0x1138
		LTFLOAT				m_fTurboCurseAccelRate;		// RETAIL: +0x113c
		LTFLOAT				m_fMaxTurboCurseSpeed;		// RETAIL: +0x1140
		LTFLOAT				m_fTurboCurseTime;			// RETAIL: +0x1144
		LTFLOAT				m_fTurboTime;				// RETAIL: +0x1148
		LTFLOAT				m_fTrailSpikeSpeed;			// RETAIL: +0x114c
		LTFLOAT				m_fTrailSpikeDist;			// RETAIL: +0x1150
		LTFLOAT				m_fTrailSpikeDelay;			// RETAIL: +0x1154
		LTFLOAT				m_fTrailDerezTime;			// RETAIL: +0x1158
		LTFLOAT				m_fTrailDerezDelay;			// RETAIL: +0x115c
		LTFLOAT				m_fMaxSpeedZoneSpeed;		// RETAIL: +0x1160
		LTFLOAT				m_fMinSlowZoneSpeed;		// RETAIL: +0x1164
		LTFLOAT				m_fMissileSpeed;			// RETAIL: +0x1168
		LTFLOAT				m_fSpeedZoneAccelRate;		// RETAIL: +0x116c
		LTFLOAT				m_fSlowZoneDecelRate;		// RETAIL: +0x1170

		int					m_nNumCycleDialogs;			// RETAIL: +0x1174
		LIGHTCYCLEDIALOG*	m_pCycleDialogs;			// RETAIL: +0x1178
		int					m_nNumCycleRaces;			// RETAIL: +0x117c
		LIGHTCYCLERACE*		m_pCycleRaces;				// RETAIL: +0x1180

		// CycleDialogParams
		LTFLOAT				m_fMinTimeBetweenDialog;	// RETAIL: +0x1184
		LTFLOAT				m_fMaxTimeBetweenDialog;	// RETAIL: +0x1188
		bool				m_bModifyPriorityWithDist;	// RETAIL: +0x118c
		LTFLOAT				m_fDialogMaxDist;			// RETAIL: +0x1190
		LTFLOAT				m_fDialogSoundRadius;		// RETAIL: +0x1194
		bool				m_bDialogSoundLocalToPlayer;// RETAIL: +0x1198
		bool				m_bEnableSoundDebugInfo;	// RETAIL: +0x1199

		LTFLOAT				m_afLCDifficultyModifiers[LCBMGR_NUM_VALUES];	// RETAIL: +0x119c
		LTFLOAT				m_afGameSpeedScalars[LCBMGR_NUM_VALUES];		// RETAIL: +0x11ac
		LTFLOAT				m_afTrailLengthValues[LCBMGR_NUM_VALUES];		// RETAIL: +0x11bc

		uint32				m_nNumShades;				// RETAIL: +0x11cc
		LIGHTCYCLESHADE*	m_pShades;					// RETAIL: +0x11d0

		LCIntArray			m_aTextIds[kNumLCTextIds];	// RETAIL: +0x11d4
		LCPowerupMap		m_mapPowerups;				// RETAIL: +0x1300
};

#endif // __LIGHTCYCLEBUTEMGR_H__
