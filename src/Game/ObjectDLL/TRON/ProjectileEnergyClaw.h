// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileEnergyClaw.h
//
// PURPOSE : The energy claw: seeks an AI, latches on to it and drains it,
//           then returns to its owner
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1003d960..0x1003ed70; vtable 0x102948b4, its class def at
// 0x102dbd08).  The class name is retail's (RTTI); the file name and the
// names of its members and states are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILEENERGYCLAW_H__
#define __PROJECTILEENERGYCLAW_H__

#include "TronProjectile.h"

class CProjectileEnergyClaw : public CTronProjectile
{
	public :

		enum ClawState
		{
			kClaw_None		= 0,
			kClaw_Seeking	= 1,	// looking for a target
			kClaw_Chasing	= 2,	// flying at the target
			kClaw_Attached	= 3,	// on the target, draining it
			kClaw_Returning	= 4,	// going back to the owner
		};

		CProjectileEnergyClaw();
		virtual ~CProjectileEnergyClaw();

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		uint32			ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual void	HandleTouch(HOBJECT hObj);
		virtual void	HandleImpact(HOBJECT hObj);
		virtual void	RemoveObject();

		int				m_eClawState;		// 0x918 (ClawState; saved as a byte)
		LTObjRef		m_hTarget;			// 0x91c
		LTObjRef		m_hClawFX;			// 0x92c
		LTFLOAT			m_fTronUnknown93C;	// 0x93c

	private :

		void			Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void			Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);

		void			Update();
		void			UpdateSeeking();
		void			UpdateChasing();
		void			UpdateAttached();
		void			UpdateReturning();

		void			HandleReturned();
		void			CreateClawFX(HOBJECT hTarget);
		void			UpdateClawFX();
		bool			HasClawFX() const;
		bool			CanSee(HOBJECT hFrom, HOBJECT hTo);
		void			SetClawState(int eState);
};

#endif // __PROJECTILEENERGYCLAW_H__
