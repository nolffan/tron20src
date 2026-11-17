// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectilePrankster.h
//
// PURPOSE : The prankster bit: it flies, stops, catches the characters
//           near it in a stasis and then makes them implode
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1003f0b0..0x10041b6c; vtable 0x10294984, its class def at
// 0x102dc190, 0x960 bytes).  The class name is retail's (RTTI); the file
// name and the names of its members and states are ours.  The class data
// is PRANKSTERCLASSDATA (FXButeMgr.h).
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILEPRANKSTER_H__
#define __PROJECTILEPRANKSTER_H__

#include "TronProjectile.h"
#include "Timer.h"

#pragma warning( disable : 4786 )
#include <map>
#include <string>

// RETAIL: the victim map orders its keys by their object (0x10041590
// compares the HOBJECTs at +0xc unsigned).  Ours
inline bool operator<(LTObjRefNotifier const& a, LTObjRefNotifier const& b)
{
	return (HOBJECT)a < (HOBJECT)b;
}

// RETAIL: a victim's stasis timer.  Retail saves and loads it with
// out-of-line copies of CTimer's code (0x10041780, 0x10041870) where the
// prankster's own timer is inline: its Save/Load are inline functions
// defined after their use (ProjectilePrankster.cpp).  The name is ours
class CPranksterVictimTimer : public CTimer
{
	public :

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
};

class CProjectilePrankster : public CTronProjectile
{
	public :

		enum PranksterState
		{
			kPrankster_None			= 0,
			kPrankster_Flying		= 1,	// flies in the owner's view direction
			kPrankster_Acquiring	= 2,	// stopped: catches the characters near it
			kPrankster_Done			= 3,	// removes itself
			kPrankster_NumStates	= 4,
		};

		CProjectilePrankster();
		virtual ~CProjectilePrankster();

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);

		// RETAIL: 0x1003f490 (the ILTObjRefReceiver slot): a victim is gone
		virtual void	OnLinkBroken(LTObjRefNotifier *pRef, HOBJECT hObj);

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		uint32			ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual void	TronUnknownVirtual30(HOBJECT hObj);
		virtual void	AddSpecialFX();
		virtual void	RemoveObject();

		// RETAIL: slot 39 (0x1003ff10)
		virtual void	ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj);

		// RETAIL: slot 45 (0x1003f700)
		virtual void	Detonate(HOBJECT hObj, CollisionInfo* pCollInfo = LTNULL);

		// the characters in stasis, each with the time it stays in stasis
		typedef std::map<LTObjRefNotifier, CPranksterVictimTimer> VictimMap;

		int				m_eState;				// 0x918 (PranksterState; saved as a byte)
		CTimer			m_AcquireTimer;			// 0x91c
		VictimMap		m_Victims;				// 0x92c
		LTFLOAT			m_fImplodeVelocity;		// 0x938 fBaseVelocity +- fVelocityRange
		LTFLOAT			m_fDeathRadius;			// 0x93c
		LTFLOAT			m_fMaxDistance;			// 0x940
		LTFLOAT			m_fTimeVictimStasis;	// 0x944
		LTFLOAT			m_fTimeVictimStasisRange;	// 0x948
		LTFLOAT			m_fTimeToAcquireTargets;	// 0x94c
		std::string		m_sEliminateTargetsFX;	// 0x950
		bool			m_bSentDone;			// 0x95c

	private :

		void			Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void			Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void			Update();
		void			UpdateFlying();
		void			UpdateAcquiring();
		void			SendDone();
		void			AcquireTargets(LTFLOAT fRadius);
		void			AddVictim(LTObjRefNotifier const& hVictim);
		void			UpdateVictims();
		void			SetState(int eState);
		bool			CanSee(HOBJECT hFrom, HOBJECT hTo);
		bool			IsNotSpeaker(HOBJECT hObj);
};

#endif // __PROJECTILEPRANKSTER_H__
