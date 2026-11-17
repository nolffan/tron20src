// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileBall.h
//
// PURPOSE : A ball that sticks to the floor it lands on and, once armed,
//			 blows up when a character in sight comes near
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10033b70..0x100353e0; vtable 0x10294368, its class def at
// 0x102daf30, 0x960 bytes). The class name is retail's (RTTI); the file
// name and the names of its members and of its non-virtual functions are
// ours.
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILEBALL_H__
#define __PROJECTILEBALL_H__

#include "ProjectileTypes.h"
#include "Timer.h"

class CProjectileBall : public CGrenade
{
	public :

		CProjectileBall();
		virtual ~CProjectileBall();

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		virtual void	HandleTouch(HOBJECT hObj);
		virtual bool	TronUnknownVirtual33(ProjectilePendingTouch *pTouch);

		// RETAIL: new virtuals (slots 51 and 52); nothing calls them
		virtual void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		virtual void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	private :

		bool	Update();
		bool	UpdateStuck();
		bool	UpdateFlight();
		void	StickTo(ProjectilePendingTouch *pTouch);
		void	StickToLastCollision();

		bool	m_bStuck;					// 0x948 On the floor
		bool	m_bArmed;					// 0x949 Looks for characters
		CTimer*	m_pArmTimer;				// 0x94c Runs from when it sticks until it is armed
		LTFLOAT	m_fProximityThreshold;		// 0x950 It sticks only this long after it is fired
		LTFLOAT	m_fProximitySecondsToArm;	// 0x954
		LTFLOAT	m_fProximityDistance;		// 0x958 RETAIL: not initialized by the constructor
		LTFLOAT	m_fRandomAngleAdjust;		// 0x95c The largest sideways turn in flight (radians)
};

#endif // __PROJECTILEBALL_H__
