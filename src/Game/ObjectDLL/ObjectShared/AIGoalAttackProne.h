// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackProne.h
//
// PURPOSE : AIGoalAttackProne class definition
//
// CREATED : 6/25/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_PRONE_H__
#define __AIGOAL_ATTACK_PRONE_H__

#include "AIGoalAbstractStimulated.h"
#include "AITarget.h"


class CAIGoalAttackProne : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackProne, kGoal_AttackProne);

		CAIGoalAttackProne();

		virtual void	InitGoal(CAI* pAI);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Damage Handling.

		virtual HMODELANIM GetAlternateDeathAnimation();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateAttackProne();

	protected:

		LTFLOAT		m_fProneTimeLimit;
		LTFLOAT		m_fMinDistSqr;

		// RETAIL: Tron's goal keeps its own target at +0xa8 (InitGoal
		// 0x10212a30, Save 0x10212a80, Load 0x10212af0), as CAIGoalAttack.
		CGenericTarget<CAIGoalAbstract>	m_Target;
};


#endif
