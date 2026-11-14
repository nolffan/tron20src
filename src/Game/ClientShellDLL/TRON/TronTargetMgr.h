// ----------------------------------------------------------------------- //
//
// MODULE  : TronTargetMgr.h
//
// PURPOSE : Definition of class to handle tracking what the player is aimed at.
//           Derived from CTargetMgr
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_TARGET_MGR_H__
#define __TRON_TARGET_MGR_H__

#include "TargetMgr.h"

class CTronTargetMgr : public CTargetMgr
{
public:
	CTronTargetMgr();
	virtual ~CTronTargetMgr();

	void Update();
	void HandleTargetPropertiesMessage(ILTMessage_Read *pMsg);

	bool IsEnergyRequired() {return m_bEnergyRequired;}
	int		GetEnergyRequired() {return m_nEnergyRequired;}

	// RETAIL: Tron's (retail +0x874, +0x875: a permission set the target
	// needs, CTronPlayerMgr::DoActivate, 0x100a7a20; the names are ours)
	bool	IsPermissionRequired() {return m_bPermissionRequired;}
	uint8	GetPermissionRequired() {return m_nPermissionRequired;}

	// RETAIL: retail 0x100afa90 (the name is ours): the target's
	// properties (+0x878..+0x895) go
	void	ClearTargetProperties();

	// RETAIL: retail's +0x876: the crosshair shows its "innocent" reticle
	// while it is set (CHUDCrosshair::RenderArmed, 0x10024aae); the name is
	// ours, and the member is at the end of ours until the class's layout
	// is retail's
	bool	IsTronUnknown876() {return m_bTronUnknown876;}
private:
	bool		m_bEnergyRequired;
	int			m_nEnergyRequired;
	// RETAIL: see IsPermissionRequired (the names are ours)
	bool		m_bPermissionRequired;
	uint8		m_nPermissionRequired;
	// RETAIL: retail's +0x878 (eight words), +0x888 (three), +0x895: more
	// target properties, not yet named
	uint16		m_anTronUnknown878[8];
	uint32		m_anTronUnknown888[3];
	bool		m_bTronUnknown895;
	bool		m_bTronUnknown876;		// RETAIL: +0x876 (see IsTronUnknown876)
};

#endif