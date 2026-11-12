// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileCorrupticDims.h
//
// PURPOSE : The corruption projectile: it jitters about its line of fire
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10035f90..0x10036970; vtable 0x10294608, its class def at
// 0x102db190).  The class name is retail's (RTTI); the file name and the
// names of its members are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILECORRUPTICDIMS_H__
#define __PROJECTILECORRUPTICDIMS_H__

#include "TronProjectile.h"

class CCorrupticDims : public CTronProjectile
{
	public :

		CCorrupticDims();
		virtual ~CCorrupticDims();

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		// from CORRUPTICDIMSCLASSDATA
		LTFLOAT			m_fRandomness;				// 0x918
		LTFLOAT			m_fRandomnessRampUpTime;	// 0x91c
		LTFLOAT			m_fHorizontalFactor;		// 0x920
		LTFLOAT			m_fVerticalFactor;			// 0x924
		LTObjRef		m_hHomingTarget;			// 0x928 (set when homing)
		LTFLOAT			m_fTurnRate;				// 0x938

	private :

		void			Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void			Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void			Update();
};

#endif // __PROJECTILECORRUPTICDIMS_H__
