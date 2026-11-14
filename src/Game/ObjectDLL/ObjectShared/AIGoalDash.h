//----------------------------------------------------------------------------
//
//	MODULE:		AIGoalDash.h
//
//	PURPOSE:	CAIGoalDash declaration
//
//	RETAIL:		Tron 2.0 only (no source anywhere): written from the retail
//				Object.lto, 0x1020bcf0..0x1020c170 (vtable 0x1029d76c,
//				goal type 19). The AI dashes at a stimulus seen between
//				its brain's DashMinDist and DashMaxDist. Member names ours.
//
//----------------------------------------------------------------------------

#ifndef __AIGOALDASH_H__
#define __AIGOALDASH_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "AIGoalAbstractStimulated.h"


class CAIGoalDash : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDash, kGoal_Dash);

		CAIGoalDash( );
		virtual ~CAIGoalDash( );

		virtual void InitGoal(CAI* pAI);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Sense Handling.

		virtual LTBOOL	HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateDash();

	protected:

		// RETAIL: doubles (0xa0, 0xa8: InitGoal 0x1020beb0 stores them as
		// QWORDs; Save/Load narrow them to floats).
		double	m_fDashMinDist;
		double	m_fDashMaxDist;
};


#endif // __AIGOALDASH_H__
