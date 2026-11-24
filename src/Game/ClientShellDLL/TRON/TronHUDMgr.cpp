// ----------------------------------------------------------------------- //
//
// MODULE  : TronHUDMgr.cpp
//
// PURPOSE : Implementation of CTronHUDMgr class
//
// CREATED : 11/5/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "GameClientShell.h"

#include "TRONPlayerStats.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"
// RETAIL: retail's TU runs ScreenSubroutines.h's constant initializers
#include "ScreenSubroutines.h"

extern CGameClientShell* g_pGameClientShell;

inline void CHUDItem::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
}

// RETAIL: the HUD manager's globals, in retail's order (0x1026edb8..
// 0x1026ee30). Init points them at the items; the destructor clears all
// of them, also the ones nothing sets (their names and types are not
// known). The names are ours, but for the ones already named.
CHUDCrosshair*				g_pCrosshair = LTNULL;				// 0x1026edb8
CHUDPermissions*			g_pPermissions = LTNULL;			// 0x1026edbc
CHUDVersion*				g_pVersion = LTNULL;				// 0x1026edc0
CHUDHealthEnergy*			g_pHealthEnergy = LTNULL;			// 0x1026edc4
CHUDArmor*					g_pArmor = LTNULL;					// 0x1026edc8
CHUDItem*					g_pTronUnknownHUD1026edcc = LTNULL;
CHUDItem*					g_pTronUnknownHUD1026edd0 = LTNULL;
CHUDItem*					g_pTronUnknownHUD1026edd4 = LTNULL;
CHUDWeapon*					g_pWeaponItem = LTNULL;				// 0x1026edd8
CHUDItem*					g_pTronUnknownHUD1026eddc = LTNULL;
CHUDItem*					g_pTronUnknownHUD1026ede0 = LTNULL;
CHUDSubChooser*				g_pSubChooser = LTNULL;				// 0x1026ede4
CHUDItem*					g_pTronUnknownHUD1026ede8 = LTNULL;
CHUDTronDecision*			g_pTronDecision = LTNULL;			// 0x1026edec
CHUDAnnouncementMsgQueue*	g_pAnnouncementMsgs = LTNULL;		// 0x1026edf0
CHUDMultiAnnouncementMsgQueue*	g_pMultiAnnouncementMsgs = LTNULL;	// 0x1026edf4
CHUDItem*					g_pTronUnknownHUD1026edf8 = LTNULL;
CHUDItem*					g_pTronUnknownHUD1026edfc = LTNULL;
CHUDItem*					g_pTronUnknownHUD1026ee00 = LTNULL;
CHUDScores*					g_pScores = LTNULL;					// 0x1026ee04
CHUDProcedurals*			g_pProcedurals = LTNULL;			// 0x1026ee10
CHUDItem*					g_pTronUnknownHUD1026ee14 = LTNULL;
CHUDLightCycleRoster*		g_pLightCycleRoster = LTNULL;		// 0x1026ee18
CHUDLightCyclePowerupIcon*	g_pLightCyclePowerupIcon = LTNULL;	// 0x1026ee1c
CHUDLightCycleSpeedometer*	g_pLightCycleSpeedometer = LTNULL;	// 0x1026ee20
CHUDLightCycleLives*		g_pLightCycleLives = LTNULL;		// 0x1026ee24
CHUDBuildNoteCount*			g_pBuildNoteCount = LTNULL;			// 0x1026ee2c
CHUDItem*					g_pTronUnknownHUD1026ee30 = LTNULL;
// PORT: g_pDiscScores (0x1026ee08), g_pDiscMatches (0x1026ee0c) and
// g_pLightCycleCircuitIntro (0x1026ee28) are defined with their items

// RETAIL: inlined for each item the constructor news (the destructor):
// out of the item list, then deleted
#define TRON_DELETE_HUD_ITEM(pItem) \
	if (pItem) \
	{ \
		ItemArray::iterator iter = std::find(m_itemArray.begin(), m_itemArray.end(), pItem); \
		if (iter != m_itemArray.end()) \
			m_itemArray.erase(iter); \
		debug_delete(pItem); \
		pItem = LTNULL; \
	}

// RETAIL: AddItems adds each item for the game type being played (as
// CHUDMgr::AddItem, inlined each time), then inits each through its vtable
#define TRON_ADD_HUD_ITEM(pItem) \
	if (((CHUDItem*)(pItem))->IsUsedInGameType(eGameType)) \
		m_itemArray.push_back(pItem);
#define TRON_INIT_HUD_ITEM(pItem) ((CHUDItem*)(pItem))->Init();

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::CTronHUDMgr()
//
//	PURPOSE:	Constructor (RETAIL: 0x1009adb0)
//
// ----------------------------------------------------------------------- //

CTronHUDMgr::CTronHUDMgr()
{
	m_pCrosshair = debug_new(CHUDCrosshair);
	m_pVersion = debug_new(CHUDVersion);
	m_pScores = debug_new(CHUDScores);
	m_pDiscScores = debug_new(CHUDDiscScores);
	m_pDiscMatches = debug_new(CHUDDiscMatches);
	m_pAnnouncementMsgs = debug_new(CHUDAnnouncementMsgQueue);
	m_pMultiAnnouncementMsgs = debug_new(CHUDMultiAnnouncementMsgQueue);
	m_pLightCycleRoster = debug_new(CHUDLightCycleRoster);
	m_pLightCyclePowerupIcon = debug_new(CHUDLightCyclePowerupIcon);
	m_pLightCycleSpeedometer = debug_new(CHUDLightCycleSpeedometer);
	m_pLightCycleLives = debug_new(CHUDLightCycleLives);
	m_pLightCycleCircuitIntro = debug_new(CHUDLightCycleCircuitIntro);
	m_pBuildNoteCount = debug_new(CHUDBuildNoteCount);
	m_pDistance = debug_new(CHUDDistance);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::~CTronHUDMgr()
//
//	PURPOSE:	Destructor (RETAIL: 0x1009b240)
//
// ----------------------------------------------------------------------- //

CTronHUDMgr::~CTronHUDMgr()
{
	g_pCrosshair = LTNULL;
	g_pPermissions = LTNULL;
	g_pVersion = LTNULL;
	g_pHealthEnergy = LTNULL;
	g_pArmor = LTNULL;
	g_pTronUnknownHUD1026edcc = LTNULL;
	g_pTronUnknownHUD1026edd0 = LTNULL;
	g_pTronUnknownHUD1026edd4 = LTNULL;
	g_pWeaponItem = LTNULL;
	g_pTronUnknownHUD1026eddc = LTNULL;
	g_pTronUnknownHUD1026ede0 = LTNULL;
	g_pSubChooser = LTNULL;
	g_pTronUnknownHUD1026ede8 = LTNULL;
	g_pTronDecision = LTNULL;
	g_pAnnouncementMsgs = LTNULL;
	g_pMultiAnnouncementMsgs = LTNULL;
	g_pTronUnknownHUD1026edf8 = LTNULL;
	g_pTronUnknownHUD1026edfc = LTNULL;
	g_pTronUnknownHUD1026ee00 = LTNULL;
	g_pScores = LTNULL;
	g_pDiscScores = LTNULL;
	g_pDiscMatches = LTNULL;
	g_pProcedurals = LTNULL;
	g_pTronUnknownHUD1026ee14 = LTNULL;
	g_pLightCycleRoster = LTNULL;
	g_pLightCyclePowerupIcon = LTNULL;
	g_pLightCycleSpeedometer = LTNULL;
	g_pLightCycleLives = LTNULL;
	g_pLightCycleCircuitIntro = LTNULL;
	g_pBuildNoteCount = LTNULL;
	g_pTronUnknownHUD1026ee30 = LTNULL;

	TRON_DELETE_HUD_ITEM(m_pCrosshair);
	TRON_DELETE_HUD_ITEM(m_pScores);
	TRON_DELETE_HUD_ITEM(m_pDiscScores);
	TRON_DELETE_HUD_ITEM(m_pDiscMatches);
	// RETAIL: m_pVersion is not deleted here (0x1009b240; docs/RETAIL_BUGS.md)
	TRON_DELETE_HUD_ITEM(m_pAnnouncementMsgs);
	TRON_DELETE_HUD_ITEM(m_pMultiAnnouncementMsgs);
	TRON_DELETE_HUD_ITEM(m_pLightCycleRoster);
	TRON_DELETE_HUD_ITEM(m_pLightCyclePowerupIcon);
	TRON_DELETE_HUD_ITEM(m_pLightCycleSpeedometer);
	TRON_DELETE_HUD_ITEM(m_pLightCycleLives);
	TRON_DELETE_HUD_ITEM(m_pLightCycleCircuitIntro);
	TRON_DELETE_HUD_ITEM(m_pBuildNoteCount);
	TRON_DELETE_HUD_ITEM(m_pDistance);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::Init()
//
//	PURPOSE:	Initialize (RETAIL: 0x1009b960)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronHUDMgr::Init()
{
	g_pCrosshair = m_pCrosshair;
	g_pPermissions = &m_Permissions;
	g_pVersion = m_pVersion;
	g_pSubChooser = &m_SubChooser;
	g_pTronDecision = &m_TronDecision;
	g_pAnnouncementMsgs = m_pAnnouncementMsgs;
	g_pMultiAnnouncementMsgs = m_pMultiAnnouncementMsgs;
	g_pScores = m_pScores;
	g_pWeaponItem = &m_Weapon;
	g_pProcedurals = &m_Procedurals;
	g_pHealthEnergy = &m_HealthEnergy;
	g_pArmor = &m_Armor;
	g_pDiscScores = m_pDiscScores;
	g_pDiscMatches = m_pDiscMatches;
	g_pLightCycleCircuitIntro = m_pLightCycleCircuitIntro;
	g_pLightCyclePowerupIcon = m_pLightCyclePowerupIcon;
	g_pLightCycleSpeedometer = m_pLightCycleSpeedometer;
	g_pLightCycleLives = m_pLightCycleLives;
	g_pLightCycleRoster = m_pLightCycleRoster;
	g_pBuildNoteCount = m_pBuildNoteCount;

    return CHUDMgr::Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::AddItems()
//
//	PURPOSE:	Tron's items first, then the base's (RETAIL: 0x1009ba50)
//
// ----------------------------------------------------------------------- //

bool CTronHUDMgr::AddItems()
{
	GameType eGameType = g_pGameClientShell->GetGameType();

	//crosshair should be first so that it is rendered first (potential overlap)
	TRON_ADD_HUD_ITEM(m_pCrosshair);
	TRON_ADD_HUD_ITEM(m_pVersion);
	TRON_ADD_HUD_ITEM(&m_Weapon);
	TRON_ADD_HUD_ITEM(&m_Pings);
	TRON_ADD_HUD_ITEM(&m_IOCamera);
	TRON_ADD_HUD_ITEM(&m_HealthEnergy);
	TRON_ADD_HUD_ITEM(&m_Permissions);
	TRON_ADD_HUD_ITEM(&m_Armor);
	TRON_ADD_HUD_ITEM(&m_EnergyTrans);
	TRON_ADD_HUD_ITEM(&m_Progress);
	TRON_ADD_HUD_ITEM(&m_WpnChooser);
	TRON_ADD_HUD_ITEM(&m_SubChooser);
	TRON_ADD_HUD_ITEM(&m_Profiler);
	TRON_ADD_HUD_ITEM(&m_TronDecision);
	TRON_ADD_HUD_ITEM(m_pAnnouncementMsgs);
	TRON_ADD_HUD_ITEM(m_pMultiAnnouncementMsgs);
	TRON_ADD_HUD_ITEM(&m_SecurityCamera);
	TRON_ADD_HUD_ITEM(&m_MatchStats);
	TRON_ADD_HUD_ITEM(&m_SingleString);
	TRON_ADD_HUD_ITEM(&m_Procedurals);
	TRON_ADD_HUD_ITEM(&m_DamageDir);
	TRON_ADD_HUD_ITEM(m_pLightCycleRoster);
	TRON_ADD_HUD_ITEM(m_pLightCyclePowerupIcon);
	TRON_ADD_HUD_ITEM(m_pLightCycleSpeedometer);
	TRON_ADD_HUD_ITEM(m_pLightCycleLives);
	TRON_ADD_HUD_ITEM(m_pLightCycleCircuitIntro);
	TRON_ADD_HUD_ITEM(m_pBuildNoteCount);
	TRON_ADD_HUD_ITEM(m_pDistance);

	TRON_INIT_HUD_ITEM(m_pCrosshair);
	TRON_INIT_HUD_ITEM(&m_Permissions);
	TRON_INIT_HUD_ITEM(m_pVersion);
	TRON_INIT_HUD_ITEM(&m_HealthEnergy);
	TRON_INIT_HUD_ITEM(&m_Armor);
	TRON_INIT_HUD_ITEM(&m_EnergyTrans);
	TRON_INIT_HUD_ITEM(&m_Progress);
	TRON_INIT_HUD_ITEM(&m_WpnChooser);
	TRON_INIT_HUD_ITEM(&m_Weapon);
	TRON_INIT_HUD_ITEM(&m_Pings);
	TRON_INIT_HUD_ITEM(&m_IOCamera);
	TRON_INIT_HUD_ITEM(&m_SubChooser);
	TRON_INIT_HUD_ITEM(&m_Profiler);
	TRON_INIT_HUD_ITEM(&m_TronDecision);
	TRON_INIT_HUD_ITEM(m_pAnnouncementMsgs);
	TRON_INIT_HUD_ITEM(m_pMultiAnnouncementMsgs);
	TRON_INIT_HUD_ITEM(&m_SecurityCamera);
	TRON_INIT_HUD_ITEM(&m_MatchStats);
	TRON_INIT_HUD_ITEM(&m_SingleString);
	TRON_INIT_HUD_ITEM(m_pScores);
	TRON_INIT_HUD_ITEM(m_pDiscScores);
	TRON_INIT_HUD_ITEM(m_pDiscMatches);
	TRON_INIT_HUD_ITEM(&m_Procedurals);
	TRON_INIT_HUD_ITEM(&m_DamageDir);
	TRON_INIT_HUD_ITEM(m_pLightCycleRoster);
	TRON_INIT_HUD_ITEM(m_pLightCyclePowerupIcon);
	TRON_INIT_HUD_ITEM(m_pLightCycleSpeedometer);
	TRON_INIT_HUD_ITEM(m_pLightCycleLives);
	TRON_INIT_HUD_ITEM(m_pLightCycleCircuitIntro);
	TRON_INIT_HUD_ITEM(m_pBuildNoteCount);
	TRON_INIT_HUD_ITEM(m_pDistance);

    return CHUDMgr::AddItems();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::TronUnknownVirtual40()
//
//	PURPOSE:	Adds the items CTronHUDMgr news for the game type being
//				played (RETAIL: 0x1009c3f0)
//
// ----------------------------------------------------------------------- //

void CTronHUDMgr::TronUnknownVirtual40()
{
	GameType eGameType = g_pGameClientShell->GetGameType();

	TRON_ADD_HUD_ITEM(m_pScores);
	TRON_ADD_HUD_ITEM(m_pDiscScores);
	TRON_ADD_HUD_ITEM(m_pDiscMatches);
}
