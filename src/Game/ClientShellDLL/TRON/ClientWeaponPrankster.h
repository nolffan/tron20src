// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponPrankster.h
//
// PURPOSE : Tron's client-side prankster bit launcher (retail only: RTTI
//           .?AVCClientWeaponPrankster@@, vtable 0x101e6648; CShell.dll
//           0x1001b090..0x1001b9a9).  Member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponPrankster_h_INCLUDED_
#define  _ClientWeaponPrankster_h_INCLUDED_

#include "TronClientWeapon.h"
#include "Timer.h"

// RETAIL: the prankster's state (+0x208); names ours
enum PranksterState
{
	kPrankster_Idle		= 0,
	kPrankster_KeyDown	= 1,
	kPrankster_Out		= 3,	// the bit is out: key up sends it off
	kPrankster_Sent		= 4,
};

// RETAIL: its MID_PROJECTILE subtypes (names ours)
#define MPROJ_PRANKSTER_ACK			200
#define MPROJ_PRANKSTER_DONE		201
#define MPROJ_PRANKSTER_RELEASE		202

class CClientWeaponPrankster : public CTronClientWeapon
{
public:
	CClientWeaponPrankster();
	virtual ~CClientWeaponPrankster();

	virtual bool  OnMessage( uint8 messageID, ILTMessage_Read *pMsg );

	// RETAIL: slot 8 (0x1001b200)
	virtual void  TronUnknownVirtual20() { SendRelease(); }

	virtual void  Select();

	virtual void  Load( ILTMessage_Read *pMsg );
	virtual void  Save( ILTMessage_Write *pMsg );

	// RETAIL: slot 50 (0x1001b450, the shared "return true")
	virtual bool  TronUnknownVirtualC8() { return true; }

protected:
	virtual void  HandleFireKeyDownTransition();
	virtual void  HandleFireKeyUpTransition();

	// RETAIL: 0x1001b0d0
	virtual uint8 GetFireMessageType() const { return kTronFire_Prankster; }

	// RETAIL: slot 71 (0x101c9040, the shared empty function)
	virtual void  DoSpecialEndFire() {}

	virtual float GetEnergyUse();
	virtual bool  HandleMessageProjectile( uint8 messageID, ILTMessage_Read *pMsg );
	virtual bool  TronUnknownVirtual18C();

	// RETAIL: 0x1001b800
	void          SetState( int nState );

	// RETAIL: 0x1001b810: tell the server to let the bit go
	bool          SendRelease();

	int           m_nState;		// RETAIL: +0x208 (PranksterState)
	CTimer       *m_pTimer;		// RETAIL: +0x20c
};

#endif //_ClientWeaponPrankster_h_INCLUDED_
