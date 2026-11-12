// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponBlaster.h
//
// PURPOSE : Tron's client-side blaster (retail only: RTTI .?AVCClientWeaponBlaster@@, vtable
//           0x101e5af4; CShell.dll 0x10018080..0x100180c1; it overrides only the destructor).  Member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponBlaster_h_INCLUDED_
#define  _ClientWeaponBlaster_h_INCLUDED_

#include "TronClientWeapon.h"

class CClientWeaponBlaster : public CTronClientWeapon
{
public:
	CClientWeaponBlaster();
	virtual ~CClientWeaponBlaster();
};

#endif //_ClientWeaponBlaster_h_INCLUDED_
