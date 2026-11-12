// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponLOL.h
//
// PURPOSE : Tron's client-side LOL weapon (retail only: RTTI .?AVCClientWeaponLOL@@, vtable
//           0x101e64b4; CShell.dll 0x1001acc0..0x1001b09c).  Member names are ours.
//           Not yet written: PlayFireAnimation (0x1001ae80) and
//           0x1001b090 (it reads CClientWeapon+0x154, not in ours).
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponLOL_h_INCLUDED_
#define  _ClientWeaponLOL_h_INCLUDED_

#include "TronClientWeapon.h"

class CClientWeaponLOL : public CTronClientWeapon
{
public:
	CClientWeaponLOL();
	virtual ~CClientWeaponLOL();

	virtual void  Load( ILTMessage_Read *pMsg );
	virtual void  Save( ILTMessage_Write *pMsg );

protected:

	// RETAIL: 0x1001acd0
	virtual uint8 GetFireMessageType() const { return kTronFire_LOL; }

	virtual float GetEnergyUse();

	// RETAIL: +0x208 (1.0f in the ctor; LOLData has a
	// fFireAnimationRateMultiplier)
	float         m_fFireAnimationRate;
};

#endif //_ClientWeaponLOL_h_INCLUDED_
