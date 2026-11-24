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
#include "HUDMatchStats.h"

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
#include "HUDTronDecision.h"
#include "HUDLightCycleSpeedometer.h"
#include "HUDLightCycleCircuitIntro.h"
#include "HUDScores.h"
#include "HUDDiscMatches.h"
#include "HUDDiscScores.h"

#include "HUDDistance.h"

//******************************************************************************************
//** Tron HUD Manager
//******************************************************************************************
// RETAIL: Tron's layout (0x11eec bytes: constructor 0x1009adb0, destructor
// 0x1009b240). Some items are members, some are newed by the constructor.
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

	CHUDCrosshair*				m_pCrosshair;				// +0x3580
	CHUDPermissions				m_Permissions;				// +0x3584
	CHUDVersion*				m_pVersion;					// +0x38b8
	CHUDHealthEnergy			m_HealthEnergy;				// +0x38bc
	CHUDArmor					m_Armor;					// +0x3d1c
	CHUDEnergyTransfer			m_EnergyTrans;				// +0x407c
	CHUDProgress				m_Progress;					// +0x41bc (0x2c54 bytes)
	CHUDWpnChooser				m_WpnChooser;				// +0x6e10
	CHUDWeapon					m_Weapon;					// +0xb328
	CHUDPings					m_Pings;					// +0xb418
	CHUDIOCamera				m_IOCamera;					// +0xb834
	CHUDSubChooser				m_SubChooser;				// +0xbb54
	CHUDProfiler				m_Profiler;					// +0xca28
	CHUDTronDecision			m_TronDecision;				// +0xce64
	CHUDAnnouncementMsgQueue*	m_pAnnouncementMsgs;		// +0x111f0
	CHUDMultiAnnouncementMsgQueue*	m_pMultiAnnouncementMsgs;	// +0x111f4
	CHUDSecurityCamera			m_SecurityCamera;			// +0x111f8
	CHUDMatchStats				m_MatchStats;				// +0x11344
	CHUDSingleString			m_SingleString;				// +0x11578
	CHUDScores*					m_pScores;					// +0x115a8
	CHUDDiscScores*				m_pDiscScores;				// +0x115ac
	CHUDDiscMatches*			m_pDiscMatches;				// +0x115b0
	CHUDProcedurals				m_Procedurals;				// +0x115b4
	CHUDDamageDir				m_DamageDir;				// +0x11a30
	CHUDLightCycleRoster*		m_pLightCycleRoster;		// +0x11ed0
	CHUDLightCyclePowerupIcon*	m_pLightCyclePowerupIcon;	// +0x11ed4
	CHUDLightCycleSpeedometer*	m_pLightCycleSpeedometer;	// +0x11ed8
	CHUDLightCycleLives*		m_pLightCycleLives;			// +0x11edc
	CHUDLightCycleCircuitIntro*	m_pLightCycleCircuitIntro;	// +0x11ee0
	CHUDBuildNoteCount*			m_pBuildNoteCount;			// +0x11ee4
	CHUDDistance*				m_pDistance;				// +0x11ee8
};

// RETAIL: the HUD manager's globals (0x1026edb8..0x1026ee30): Init points
// them at the items, the destructor clears them. The names are ours.
extern CHUDCrosshair*				g_pCrosshair;
extern CHUDPermissions*				g_pPermissions;
extern CHUDVersion*					g_pVersion;
extern CHUDHealthEnergy*			g_pHealthEnergy;
extern CHUDArmor*					g_pArmor;
extern CHUDWeapon*					g_pWeaponItem;
extern CHUDSubChooser*				g_pSubChooser;
extern CHUDAnnouncementMsgQueue*	g_pAnnouncementMsgs;
extern CHUDMultiAnnouncementMsgQueue*	g_pMultiAnnouncementMsgs;
extern CHUDProcedurals*				g_pProcedurals;
extern CHUDLightCyclePowerupIcon*	g_pLightCyclePowerupIcon;
extern CHUDLightCycleSpeedometer*	g_pLightCycleSpeedometer;
extern CHUDBuildNoteCount*			g_pBuildNoteCount;

#endif // __TRONHUDMGR_H
