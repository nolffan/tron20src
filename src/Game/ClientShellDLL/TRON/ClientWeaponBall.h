// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponBall.h
//
// PURPOSE : Tron's client-side ball weapon (retail only: RTTI
//           .?AVCClientWeaponBall@@, vtable 0x101e57cc; CShell.dll
//           0x10017670..0x10017c1f).  Member names are ours.
//           Not yet written: PlayFireAnimation (0x10017910) and
//           AddExtraFireMessageInfo (0x10017b90; it needs the retail-only
//           BALLCLASSDATA and WEAPON/AMMO's layout).
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponBall_h_INCLUDED_
#define  _ClientWeaponBall_h_INCLUDED_

#include "TronClientWeapon.h"

class CClientWeaponBall : public CTronClientWeapon
{
public:
	CClientWeaponBall();
	virtual ~CClientWeaponBall();

	virtual void  Load( ILTMessage_Read *pMsg );
	virtual void  Save( ILTMessage_Write *pMsg );

protected:
	virtual void  HandleFireKeyDownTransition();

	// RETAIL: 0x10017840
	virtual uint8 GetFireMessageType() const { return kTronFire_Ball; }

	// RETAIL: +0x208, the time the fire key went down (saved relative)
	float         m_fFireKeyDownTime;
};

#endif //_ClientWeaponBall_h_INCLUDED_
