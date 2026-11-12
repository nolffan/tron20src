// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponEnergyClaw.h
//
// PURPOSE : Tron's client-side energy claw (retail only: RTTI
//           .?AVCClientWeaponEnergyClaw@@, vtable 0x101e6320; CShell.dll
//           0x1001a5f0..0x1001ac93).  Member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponEnergyClaw_h_INCLUDED_
#define  _ClientWeaponEnergyClaw_h_INCLUDED_

#include "ClientWeaponContinuousFireBase.h"

class CClientWeaponEnergyClaw : public CClientWeaponContinuousFireBase
{
public:
	CClientWeaponEnergyClaw();
	virtual ~CClientWeaponEnergyClaw();

	// RETAIL: 0x1001a650 (three arguments there: see TronClientWeapon.h)
	virtual WeaponState   Update( bool bFire, FireType eFireType = FT_NORMAL_FIRE );

protected:
	// RETAIL: 0x1001a610
	virtual uint8 GetFireMessageType() const { return kTronFire_EnergyClaw; }

	virtual float GetEnergyUse();
	virtual void  TronUnknownVirtual180();

	// RETAIL: 0x1001a7a0 (707 bytes: the claw's target and its in-range FX)
	// not yet written
	void          UpdateInRange();

	bool          m_bInRange;		// RETAIL: +0x20c
};

#endif //_ClientWeaponEnergyClaw_h_INCLUDED_
