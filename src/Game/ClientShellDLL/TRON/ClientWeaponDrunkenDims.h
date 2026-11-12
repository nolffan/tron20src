// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponDrunkenDims.h
//
// PURPOSE : Tron's client-side drunken dims weapon (retail only: RTTI .?AVCClientWeaponDrunkenDims@@, vtable
//           0x101e6188; CShell.dll 0x1001a430..0x1001a5ec).  Member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponDrunkenDims_h_INCLUDED_
#define  _ClientWeaponDrunkenDims_h_INCLUDED_

#include "TronClientWeapon.h"

class CClientWeaponDrunkenDims : public CTronClientWeapon
{
public:
	CClientWeaponDrunkenDims();
	virtual ~CClientWeaponDrunkenDims();

protected:
	virtual void  AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg );

	// RETAIL: 0x1001a440
	virtual uint8 GetFireMessageType() const { return kTronFire_DrunkenDims; }

	virtual float GetEnergyUse();
};

#endif //_ClientWeaponDrunkenDims_h_INCLUDED_
