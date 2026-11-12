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
private:
	bool		m_bEnergyRequired;
	int			m_nEnergyRequired;
	// RETAIL: see IsPermissionRequired (the names are ours)
	bool		m_bPermissionRequired;
	uint8		m_nPermissionRequired;
};

#endif