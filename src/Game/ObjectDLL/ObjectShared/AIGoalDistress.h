// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDistress.h
//
// PURPOSE : AIGoalDistress class definition
//
// CREATED : 7/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_DISTRESS_H__
#define __AIGOAL_DISTESS_H__

#include "AIGoalAbstractStimulated.h"
#include "ActivationObserver.h"


// RETAIL: Tron's CAIGoalDistress also derives IActivationObserver (+0xa0,
// RTTI) and holds an ActivationNotifier (+0xa8); size 0xb4 (factory
// 0x102097c0, constructor 0x10209870).
class CAIGoalDistress : public CAIGoalAbstractStimulated, public IActivationObserver
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDistress, kGoal_Distress);

		CAIGoalDistress( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// RETAIL: Tron's (0x102099f0): registers the activation notifier.
		virtual void	InitGoal(CAI* pAI);

		// RETAIL: Tron's IActivationObserver (0x10209a50).
		virtual int		OnActivation(ActivationInfo* pInfo);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateDistress();
		void HandleStatePanic();

	protected:

		LTBOOL				m_bCanActivate;
		ActivationNotifier	m_ActivationNotifier;	// RETAIL: Tron's (+0xa8)
};


#endif
