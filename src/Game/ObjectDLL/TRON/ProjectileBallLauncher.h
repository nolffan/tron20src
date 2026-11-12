// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileBallLauncher.h
//
// PURPOSE : A corruption projectile that takes no damage
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10035440..0x100355e0; vtable 0x10294454, its class def at
// 0x102db018).  The class name is retail's (RTTI); the file name and the
// names of its members are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILEBALLLAUNCHER_H__
#define __PROJECTILEBALLLAUNCHER_H__

#include "ProjectileCorrupticDims.h"

class CProjectileBallLauncher : public CCorrupticDims
{
	public :

		CProjectileBallLauncher();
		virtual ~CProjectileBallLauncher();

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);

		bool			FilterDamage(DamageStruct *pDamageStruct);

	protected :

		static bool		DamageFilterHook(GameBase *pObject, DamageStruct *pDamageStruct);
};

#endif // __PROJECTILEBALLLAUNCHER_H__
