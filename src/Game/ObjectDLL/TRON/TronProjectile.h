// ----------------------------------------------------------------------- //
//
// MODULE  : TronProjectile.h
//
// PURPOSE : Tron's projectile base: a projectile that drains its owner's
//           energy while it is out
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10068a00..0x10068eb0; vtable 0x10295294, its class def at
// 0x102dec50).  The class name is retail's (RTTI); the file name and the
// names of its members are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __TRONPROJECTILE_H__
#define __TRONPROJECTILE_H__

#include "Projectile.h"

class CTronProjectile : public CProjectile
{
	public :

		CTronProjectile();
		virtual ~CTronProjectile();

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		// RETAIL: slot 46 (0x10068d00): drains the firing player's energy
		// while the projectile is out
		virtual void	UpdateEnergyDrain();

		// RETAIL: slot 47 (0x10068e90): the firing player is out of energy
		virtual void	HandleOutOfEnergy();

		LTFLOAT			m_fEnergyDrain;		// 0x910 energy per second

	private :

		void			Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void			Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void			Update();
};

#endif // __TRONPROJECTILE_H__
