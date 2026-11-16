// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponEnergyClaw.h
//
// PURPOSE : Tron's client-side energy claw (retail only: RTTI
//           .?AVCClientWeaponEnergyClaw@@, vtable 0x101e6320; CShell.dll
//           0x1001a5f0..0x1001ac93).  Member names are ours.
//           Not yet written: 0x1001a7a0 (the claw's target and in-range
//           FX, which Update calls first) and its helpers 0x1001ab60..0x1001ac10.
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponEnergyClaw_h_INCLUDED_
#define  _ClientWeaponEnergyClaw_h_INCLUDED_

#include "ClientWeaponContinuousFireBase.h"
#include "SubroutineDataMgr.h"

class CClientWeaponEnergyClaw : public CClientWeaponContinuousFireBase
{
public:
	CClientWeaponEnergyClaw();
	virtual ~CClientWeaponEnergyClaw();

	// RETAIL: 0x1001a650 (three arguments there: see TronClientWeapon.h)
	virtual WeaponState   Update( bool bFire, bool bAltFire, FireType eFireType = FT_NORMAL_FIRE );

protected:
	// RETAIL: 0x1001a610
	virtual uint8 GetFireMessageType() const { return kTronFire_EnergyClaw; }

	virtual float GetEnergyUse();
	virtual void  TronUnknownVirtual180();

	// RETAIL: 0x1001a7a0..0x1001abd0 (the names are ours)
	void           UpdateTarget();
	EnergyClawData GetClawData();
	float          GetClawRange();
	void           ShowInRangeFX();
	void           HideInRangeFX();

	// RETAIL: the subroutine's name read from memory (retail 0x101e6318)
	static const char* const s_szClawName;

	bool          m_bInRange;		// RETAIL: +0x20c
};

#endif //_ClientWeaponEnergyClaw_h_INCLUDED_
