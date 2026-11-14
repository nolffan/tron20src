// ----------------------------------------------------------------------- //
//
// MODULE  : TronHUDMgr.h
//
// PURPOSE : Definition of derived CHUDMgr class
//
// CREATED : 11/5/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRONHUDMGR_H
#define __TRONHUDMGR_H

#include "HUDMgr.h"		// base class

#include "TRONPlayerStats.h"

// TO2 dependencies
#include "HUDCrosshair.h"
#include "HUDDamage.h"
#include "HUDWeapons.h"

// Tron dependencies
#include "HUDPermissions.h"
#include "HUDVersion.h"
#include "HUDHealthEnergy.h"
#include "HUDArmor.h"
#include "HUDEnergyTransfer.h"
#include "HUDProgress.h"
#include "HUDChooser.h"
#include "HUDWeapon.h"

// Tron's retail-only items (RETAIL: written from retail CShell.dll)
#include "HUDDamageDir.h"
#include "HUDBuildNoteCount.h"
#include "HUDIOCamera.h"
#include "HUDSingleString.h"
#include "HUDLightCycleLives.h"
#include "HUDLightCyclePowerupIcon.h"
#include "HUDLightCycleRoster.h"
#include "HUDPings.h"
#include "HUDProcedurals.h"
#include "HUDProfiler.h"
#include "HUDSubChooser.h"
#include "HUDSecurityCamera.h"
#include "HUDScores.h"

//******************************************************************************************
//** Tron HUD Manager
//******************************************************************************************
class CTronHUDMgr: public CHUDMgr
{
public:

	CTronHUDMgr();
	~CTronHUDMgr();

	LTBOOL		Init();
	bool		AddItems();

	// RETAIL: slot 16 (0x1009c3f0): adds the scores (and the disc games'
	// items) for the game being played
	virtual void	TronUnknownVirtual40();

protected:

	CHUDCrosshair		m_Crosshair;
	CHUDWeapons			m_Weapons;

	// New Tron items
	CHUDPermissions		m_Permissions;
	CHUDVersion			m_Version;
	CHUDHealthEnergy	m_HealthEnergy;
	CHUDArmor			m_Armor;
	CHUDEnergyTransfer	m_EnergyTrans;
	CHUDProgress		m_Progress;
	CHUDWpnChooser		m_WpnChooser;
	CHUDAmmoChooser		m_AmmoChooser;
	CHUDWeapon			m_Weapon;

	// RETAIL: Tron's retail-only items. PORT: their place in the class is
	// not yet retail's (retail 0x1009b240's constructor and 0x1009ba50's
	// AddItems: the pings at +0xb418, the profiler at +0xca28)
	CHUDDamageDir				m_DamageDir;
	CHUDBuildNoteCount			m_BuildNoteCount;
	CHUDIOCamera				m_IOCamera;
	CHUDSingleString			m_SingleString;
	CHUDLightCycleLives			m_LightCycleLives;
	CHUDLightCyclePowerupIcon	m_LightCyclePowerupIcon;
	CHUDLightCycleRoster		m_LightCycleRoster;
	CHUDPings					m_Pings;
	CHUDProcedurals				m_Procedurals;
	CHUDSubChooser				m_SubChooser;
	CHUDProfiler				m_Profiler;
	CHUDSecurityCamera			m_SecurityCamera;

	// RETAIL: newed by the constructor (0x1009af48..0x1009afdf: +0x115a8,
	// +0x115ac, +0x115b0), deleted by the destructor; Init points the
	// globals at them (0x1009b9b0)
	CHUDScores*					m_pScores;
};
extern CHUDCrosshair*		g_pCrosshair;

#endif // __TRONHUDMGR_H
