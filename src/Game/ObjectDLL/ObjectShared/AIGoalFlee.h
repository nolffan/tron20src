// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlee.h
//
// PURPOSE : AIGoalFlee class definition
//
// CREATED : 7/24/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FLEE_H__
#define __AIGOAL_FLEE_H__

#include "AIGoalAbstractStimulated.h"
#include "ActivationObserver.h"


// Forward Declarations.
class AINode;



// RETAIL: Tron's CAIGoalFlee also derives IActivationObserver (+0xa0,
// RTTI) and holds an ActivationNotifier (+0xc8); size 0xd4 (factory
// 0x10207c40, constructor 0x10207cf0).
class CAIGoalFlee : public CAIGoalAbstractStimulated, public IActivationObserver
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFlee, kGoal_Flee);

		CAIGoalFlee( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// RETAIL: Tron's (0x10207fc0): registers the activation notifier.
		virtual void	InitGoal(CAI* pAI);

		// RETAIL: Tron's IActivationObserver (0x10208270).
		virtual int		OnActivation(ActivationInfo* pInfo);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Relax.

		void Relax();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateFlee();
		void HandleStatePanic();
		void HandleStateGoto();

	protected:

        LTObjRef	m_hDestNode;
		LTObjRef	m_hDangerObject;
		LTFLOAT		m_fRelaxTime;
		ActivationNotifier	m_ActivationNotifier;	// RETAIL: Tron's (+0xc8)
};


#endif
