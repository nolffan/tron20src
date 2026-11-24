//----------------------------------------------------------------------------
//
//	MODULE:		AIGoalDash.cpp
//
//	PURPOSE:	CAIGoalDash implementation
//
//	RETAIL:		Tron 2.0 only: written from the retail Object.lto
//				(0x1020bcf0..0x1020c170). See AIGoalDash.h.
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "AIGoalDash.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AITarget.h"
#include "AnimationMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDash, kGoal_Dash);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDash::Con/destructor
//
//	PURPOSE:	Factory Con/destructor (RETAIL: 0x1020bdc0)
//
// ----------------------------------------------------------------------- //

CAIGoalDash::CAIGoalDash()
{
}

CAIGoalDash::~CAIGoalDash()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDash::InitGoal
//
//	PURPOSE:	Initialize goal (RETAIL: 0x1020beb0)
//
// ----------------------------------------------------------------------- //

void CAIGoalDash::InitGoal(CAI* pAI)
{
	super::InitGoal(pAI);

	double fDashMaxDist = pAI->GetBrain()->GetAIData( kAIData_DashMaxDist );
	m_fDashMinDist = pAI->GetBrain()->GetAIData( kAIData_DashMinDist );
	m_fDashMaxDist = fDashMaxDist;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDash::Save / Load
//
//	PURPOSE:	Save / Load (RETAIL: 0x1020bf10 / 0x1020bf60)
//
// ----------------------------------------------------------------------- //

void CAIGoalDash::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	// RETAIL: 0x1020bf10 narrows each double before it fetches pMsg's
	// vtable. VC6 does that only for a parenthesized argument: without
	// the inner parentheses (or with a (LTFLOAT) cast) it narrows after.
	SAVE_FLOAT( (m_fDashMinDist) );
	SAVE_FLOAT( (m_fDashMaxDist) );
}

void CAIGoalDash::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LTFLOAT fDashMinDist, fDashMaxDist;
	LOAD_FLOAT( fDashMinDist );
	LOAD_FLOAT( fDashMaxDist );
	m_fDashMinDist = fDashMinDist;
	m_fDashMaxDist = fDashMaxDist;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDash::ActivateGoal
//
//	PURPOSE:	Activate goal (RETAIL: 0x1020bfb0)
//
// ----------------------------------------------------------------------- //

void CAIGoalDash::ActivateGoal()
{
	super::ActivateGoal();

	// Ignore senses other than SeeEnemy.

	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pAI->SetState( kState_HumanDash );
	m_pAI->Target( m_hStimulusSource );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDash::DeactivateGoal
//
//	PURPOSE:	Deactivate goal (RETAIL: 0x1020bff0; skips
//				CAIGoalAbstractStimulated's)
//
// ----------------------------------------------------------------------- //

void CAIGoalDash::DeactivateGoal()
{
	CAIGoalAbstract::DeactivateGoal();

	m_pAI->GetAnimationContext()->SetAnimRate( 1.f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDash::UpdateGoal
//
//	PURPOSE:	Update goal (RETAIL: 0x1020c010)
//
// ----------------------------------------------------------------------- //

void CAIGoalDash::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch( pState->GetStateType() )
	{
		case kState_HumanIdle:
			m_fCurImportance = 0.f;
			break;

		case kState_HumanDash:
			HandleStateDash();
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDash::HandleStateDash
//
//	PURPOSE:	Determine what to do when in state Dash (RETAIL: 0x1020c040)
//
// ----------------------------------------------------------------------- //

void CAIGoalDash::HandleStateDash()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			m_pAI->GetAnimationContext()->SetAnimRate( 2.5f );
			break;

		case kSStat_StateComplete:
			{
				m_pAI->GetTarget()->SetPushSpeed( 800.f );
				m_pAI->GetTarget()->SetPushMinDist( m_pAI->GetDims().z * 2.f );

				m_pAI->GetAnimationContext()->SetAnimRate( 1.f );
				m_pAI->ResetBaseSenseFlags();
				m_pAI->SetState( kState_HumanIdle );

				m_fCurImportance = 0.f;
			}
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDash::HandleGoalSenseTrigger
//
//	PURPOSE:	React to a sense (RETAIL: 0x1020c0f0)
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDash::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		LTFLOAT fDist = ( pSenseRecord->vLastStimulusPos - m_pAI->GetPosition() ).Mag();
		if( ( m_fDashMinDist < fDist ) && ( m_fDashMaxDist > fDist ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}
