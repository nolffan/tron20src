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

CHUDCrosshair*		g_pCrosshair = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::CTronHUDMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronHUDMgr::CTronHUDMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::~CTronHUDMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronHUDMgr::~CTronHUDMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CTronHUDMgr::Init()
{
	g_pCrosshair = &m_Crosshair;

    return CHUDMgr::Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::AddItems()
//
//	PURPOSE:	Tron's items first, then the base's (RETAIL: retail 0x1009ba50
//				adds and inits its own, then calls CHUDMgr's)
//
// ----------------------------------------------------------------------- //

bool CTronHUDMgr::AddItems()
{
	GameType eGameType = g_pGameClientShell->GetGameType();

	//crosshair should be first so that it is rendered first (potential overlap)
	AddItem(&m_Crosshair, eGameType);

	AddItem(&m_Weapons, eGameType);
	AddItem(&m_Version, eGameType);

	// Tron-specific items
	AddItem(&m_Permissions, eGameType);
	AddItem(&m_HealthEnergy, eGameType);

	AddItem(&m_Armor, eGameType);
	AddItem(&m_EnergyTrans, eGameType);
	AddItem(&m_Progress, eGameType);

	AddItem(&m_Weapon, eGameType);
	AddItem(&m_WpnChooser, eGameType);
	AddItem(&m_AmmoChooser, eGameType);

	// RETAIL: Tron's retail-only items (PORT: retail's order is by their
	// place in the class, which is not yet retail's: the weapon, the
	// pings, ..., the subroutine chooser, the profiler, Tron's decision)
	AddItem(&m_Pings, eGameType);
	AddItem(&m_DamageDir, eGameType);
	AddItem(&m_BuildNoteCount, eGameType);
	AddItem(&m_IOCamera, eGameType);
	AddItem(&m_SingleString, eGameType);
	AddItem(&m_LightCycleLives, eGameType);
	AddItem(&m_LightCyclePowerupIcon, eGameType);
	AddItem(&m_Procedurals, eGameType);
	AddItem(&m_Profiler, eGameType);

	m_Crosshair.Init();
	m_Weapons.Init();
	m_Version.Init();
	m_Permissions.Init();
	m_HealthEnergy.Init();
	m_Armor.Init();
	m_EnergyTrans.Init();
	m_Progress.Init();
	m_Weapon.Init();
	m_WpnChooser.Init();
	m_AmmoChooser.Init();
	m_Pings.Init();
	m_DamageDir.Init();
	m_BuildNoteCount.Init();
	m_IOCamera.Init();
	m_SingleString.Init();
	m_LightCycleLives.Init();
	m_LightCyclePowerupIcon.Init();
	m_Procedurals.Init();
	m_Profiler.Init();

    return CHUDMgr::AddItems();
}

