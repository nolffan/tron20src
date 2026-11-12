// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponContinuousFireBase.h
//
// PURPOSE : Tron's base of its continuous fire weapons (PRod, EnergyClaw;
//           retail only: RTTI .?AVCClientWeaponContinuousFireBase@@,
//           vtable 0x101e5e30; CShell.dll 0x10018330..0x100189b0).
//           Member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponContinuousFireBase_h_INCLUDED_
#define  _ClientWeaponContinuousFireBase_h_INCLUDED_

#include "TronClientWeapon.h"

class CClientWeaponContinuousFireBase : public CTronClientWeapon
{
public:
	CClientWeaponContinuousFireBase();
	virtual ~CClientWeaponContinuousFireBase();

	// RETAIL: 0x10018380 (three arguments there: see TronClientWeapon.h)
	virtual WeaponState   Update( bool bFire, FireType eFireType = FT_NORMAL_FIRE );

	virtual void  Load( ILTMessage_Read *pMsg );
	virtual void  Save( ILTMessage_Write *pMsg );

protected:
	virtual void  HandleFireKeyDownTransition();

	// RETAIL: 0x100d1580 / 0x101c8210 (the shared "return true/false")
	virtual bool  TronUnknownVirtual16C() { return true; }
	virtual bool  TronUnknownVirtual188() { return false; }

	bool          m_bFiring;		// RETAIL: +0x208 (sent "stop firing" on key up)
	bool          m_bKeyUpEarly;	// RETAIL: +0x209 (key up before it fired)
};

#endif //_ClientWeaponContinuousFireBase_h_INCLUDED_
