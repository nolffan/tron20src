// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollow.h
//
// PURPOSE : AIGoalFollow class definition
//
// CREATED : 7/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FOLLOW_H__
#define __AIGOAL_FOLLOW_H__

#include "AIGoalAbstractTargeted.h"
#include "AnimationProp.h"
#include "ActivationObserver.h"

// RETAIL: Tron's CAIGoalFollow also derives IActivationObserver (+0x74,
// RTTI) and holds an ActivationNotifier (+0x88); size 0x94 (factory
// 0x10207430, constructor 0x102074e0).
class CAIGoalFollow : public CAIGoalAbstractTargeted, public IActivationObserver
{
	typedef CAIGoalAbstractTargeted super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFollow, kGoal_Follow);

		CAIGoalFollow( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// RETAIL: Tron's (0x102076a0): registers the activation notifier.
		virtual void	InitGoal(CAI* pAI);

		// RETAIL: Tron's IActivationObserver (0x10207700).
		virtual int		OnActivation(ActivationInfo* pInfo);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Handling.

		void HandleStateFollow();
		void HandleStatePanic();

	protected:

		LTBOOL				m_bPanicCanActivate;
		EnumAnimProp		m_eMovement;
		LTFLOAT				m_fRangeTime;
		LTFLOAT				m_fRangeSqr;
		ActivationNotifier	m_ActivationNotifier;	// RETAIL: Tron's (+0x88)
};


#endif
