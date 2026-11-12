// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFconMonster.h
//
// PURPOSE : AIGoalFconMonster class definition: the Fcon monster boss and
//           its two arms, each of which can be hurt and destroyed.
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (RTTI
//         CAIGoalFconMonster, vtable 0x1029c9a8; TU 0x101f9530..
//         0x101f9fb0). Names of members and methods are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FCONMONSTER_H__
#define __AIGOAL_FCONMONSTER_H__

#include "AIGoalAbstract.h"


class CAIGoalFconMonster : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFconMonster, kGoal_FconMonster);

		CAIGoalFconMonster( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		void	DestroyArm(uint32 iArm);

	protected:

		// RETAIL: [0] is the left arm, [1] the right.
		LTFLOAT			m_afHitPoints[2];			// RETAIL: 0x5c
		// RETAIL: near; retail's assignments check the source for NULL inline.
		std::string		m_astrCommandObject[2];		// RETAIL: 0x64
		bool			m_abDestroyed[2];			// RETAIL: 0x7c
		bool			m_abInvulnerable[2];		// RETAIL: 0x7e
};


#endif
