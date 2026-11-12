// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponSequencerDisc.h
//
// PURPOSE : Tron's client-side sequencer disc (retail only: RTTI
//           .?AVCClientWeaponSequencerDisc@@, vtable 0x101e67dc; CShell.dll
//           0x1001b9b0..0x1001bc7c).
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponSequencerDisc_h_INCLUDED_
#define  _ClientWeaponSequencerDisc_h_INCLUDED_

#include "ClientWeaponDisc.h"

class CClientWeaponSequencerDisc : public CClientWeaponDisc
{
public:
	CClientWeaponSequencerDisc();
	virtual ~CClientWeaponSequencerDisc();

protected:
	virtual float GetEnergyUse();
	virtual bool  TronUnknownVirtual18C();
};

#endif //_ClientWeaponSequencerDisc_h_INCLUDED_
