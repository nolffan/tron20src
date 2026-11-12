// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileDrunkenDims.h
//
// PURPOSE : A corruption projectile
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1003d7d0..0x1003d960; vtable 0x102947dc, its class def at
// 0x102dbc78).  The class name is retail's (RTTI); the file name is ours.
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILEDRUNKENDIMS_H__
#define __PROJECTILEDRUNKENDIMS_H__

#include "ProjectileCorrupticDims.h"

class CProjectileDrunkenDims : public CCorrupticDims
{
	public :

		CProjectileDrunkenDims();
		virtual ~CProjectileDrunkenDims();

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);
};

#endif // __PROJECTILEDRUNKENDIMS_H__
