// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponBallLauncher.h
//
// PURPOSE : Tron's client-side ball launcher (retail only: RTTI .?AVCClientWeaponBallLauncher@@, vtable
//           0x101e5960; CShell.dll 0x10017c20..0x1001807c).  Member names are ours.
//           Not yet written: PlayFireAnimation (0x10017c80).
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponBallLauncher_h_INCLUDED_
#define  _ClientWeaponBallLauncher_h_INCLUDED_

#include "TronClientWeapon.h"

class CClientWeaponBallLauncher : public CTronClientWeapon
{
public:
	CClientWeaponBallLauncher();
	virtual ~CClientWeaponBallLauncher();

	// RETAIL: 0x1001ad20/0x1001ad50 (folded with CClientWeaponLOL's)
	virtual void  Load( ILTMessage_Read *pMsg );
	virtual void  Save( ILTMessage_Write *pMsg );

protected:

	// RETAIL: 0x10017c30
	virtual uint8 GetFireMessageType() const { return kTronFire_BallLauncher; }

	virtual float GetEnergyUse();

	// RETAIL: 0x10018070 (PlayFireAnimation calls it)
	void          UpdateFireAnimationRate();

	// RETAIL: +0x208 (1.0f in the ctor; BallLauncherData has a
	// fFireAnimationRateMultiplier)
	float         m_fFireAnimationRate;
};

#endif //_ClientWeaponBallLauncher_h_INCLUDED_
