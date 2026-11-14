// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponPRod.h
//
// PURPOSE : Tron's client-side primitive charge rod (retail only: RTTI
//           .?AVCClientWeaponPRod@@, vtable 0x101e8c28).  It adds nothing
//           to CClientWeaponContinuousFireBase but its fire message type,
//           and has no code of its own: retail emits its vtable, its
//           destructor (0x10099490) and GetFireMessageType (0x10099460) in
//           the allocator's TU (TRONClientWeaponAllocator.cpp), which is
//           the only place that creates one.
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponPRod_h_INCLUDED_
#define  _ClientWeaponPRod_h_INCLUDED_

#include "ClientWeaponContinuousFireBase.h"

class CClientWeaponPRod : public CClientWeaponContinuousFireBase
{
public:
	// RETAIL: 0x10099490 (inline: it stores the vtable)
	virtual ~CClientWeaponPRod() {}

protected:
	// RETAIL: 0x10099460
	virtual uint8 GetFireMessageType() const { return kTronFire_PRod; }
};

#endif //_ClientWeaponPRod_h_INCLUDED_
