// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponClusterDisc.h
//
// PURPOSE : Tron specific client-side cluster version of the disc
//
// CREATED : 4/12/02 (was WeaponModel.h)
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponClusterDisc_h_INCLUDED_
#define  _ClientWeaponClusterDisc_h_INCLUDED_

#include "ClientWeaponDisc.h"

class CClientWeaponClusterDisc : public CClientWeaponDisc
{
public:
	CClientWeaponClusterDisc();
	virtual ~CClientWeaponClusterDisc();

	// return the "type" identifier (1st part of a fire message)
	// NOTE: currently the server handles VECTOR and PROJECTILE the same
	virtual uint8 GetFireMessageType() const { return MWEAPFIRE_DISC; }

protected:
	// RETAIL: 0x100181b0
	virtual float GetEnergyUse();

	// RETAIL: 0x1001bc00 (folded with CClientWeaponSequencerDisc's)
	virtual bool  TronUnknownVirtual18C();

	// PORT: b69's; retail's cluster disc overrides only the destructor,
	// GetEnergyUse and TronUnknownVirtual18C
	// add disc firing information to the fire message
	virtual void  AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg );
};

#endif //_ClientWeaponClusterDisc_h_INCLUDED_
