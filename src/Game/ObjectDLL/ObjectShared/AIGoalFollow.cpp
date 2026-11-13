// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollow.cpp
//
// PURPOSE : AIGoalFollow implementation
//
// CREATED : 7/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalFollow.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AI.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "AIUtils.h"

extern CCharacterMgr* g_pCharacterMgr;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFollow, kGoal_Follow);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFollow::CAIGoalFollow()
{
	m_bPanicCanActivate	= LTFALSE;
	m_eMovement			= kAP_Walk;
	m_fRangeTime		= .25f;
	m_fRangeSqr			= 100.0f*100.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bPanicCanActivate);
	SAVE_DWORD(m_eMovement);
	SAVE_FLOAT(m_fRangeTime);
	SAVE_FLOAT(m_fRangeSqr);
}

void CAIGoalFollow::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bPanicCanActivate);
	LOAD_DWORD_CAST(m_eMovement, EnumAnimProp);
	LOAD_FLOAT(m_fRangeTime);
	LOAD_FLOAT(m_fRangeSqr);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::InitGoal
//
//	PURPOSE:	RETAIL: Tron's (0x102076a0): hook the goal up to the AI's
//				activation notifications.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::InitGoal(CAI* pAI)
{
	super::InitGoal(pAI);

	m_ActivationNotifier.SetSubject( pAI );
	m_ActivationNotifier.SetObserver( this );
	m_ActivationNotifier.SetCallback( &IActivationObserver::OnActivation );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::OnActivation
//
//	PURPOSE:	RETAIL: Tron's (0x10207700): activating our AI makes it
//				follow the activator, or stop following.
//
// ----------------------------------------------------------------------- //

int CAIGoalFollow::OnActivation(ActivationInfo* pInfo)
{
	if( pInfo && ( pInfo->hAI == m_pAI->GetUserObject() ) )
	{
		m_hTarget = pInfo->hActivator;

		if( m_fCurImportance == 0.f )
		{
			SetCurToBaseImportance();
		}
		else if( m_fCurImportance == m_fBaseImportance )
		{
			m_fCurImportance = 0.f;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::ActivateGoal()
{
	super::ActivateGoal();

	m_pAI->SetState( kState_HumanIdle );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanIdle:
			// Default Target to player if not set.
			if(m_hTarget == LTNULL)
			{
				CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
				m_hTarget = pPlayer->m_hObject;
			}
			if(m_hTarget)
			{
				// Set target.
				m_pAI->Target(m_hTarget);
				m_pAI->SetState( kState_HumanFollow );
			}
			break;

		case kState_HumanFollow:
			HandleStateFollow();
			break;

		case kState_HumanPanic:

			// Ignore all senses.
			m_pAI->SetCurSenseFlags( kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

			HandleStatePanic();
			break;

		// Unexpected State.
		default: ASSERT(!"CAIGoalFollow::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::HandleStateFollow
//
//	PURPOSE:	Determine what to do when in state Follow.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::HandleStateFollow()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Following:
			break;

		case kSStat_Holding:
			break;

		case kSStat_FailedComplete:
			{
				// Ignore all senses.
				m_pAI->SetCurSenseFlags( kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

				m_pAI->SetState( kState_HumanPanic );
	
				CAIHumanStatePanic* pPanicState = (CAIHumanStatePanic*)m_pAI->GetState();
				pPanicState->SetCanActivate(m_bPanicCanActivate);
			}
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalFollow::HandleStateFollow: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::HandleStatePanic
//
//	PURPOSE:	Determine what to do when in state Panic.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::HandleStatePanic()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalFollow::HandleStatePanic: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalFollow::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// RETAIL: Tron's (0x10207890): a sense ends the goal, it never
	// triggers it (NOLF2 triggered on seeing the target). Tron calls
	// CAIGoalAbstract's directly (super does not override it).

	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		m_fCurImportance = 0.f;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalFollow::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if( super::HandleNameValuePair(szName, szValue) )
	{
		return LTTRUE;
	}

	// RETAIL: Tron's "ACTIVATE" (0x102078b0) starts following.
	if ( !_stricmp(szName, "ACTIVATE") )
	{
		SetCurToBaseImportance();
		return LTTRUE;
	}
	else if ( !_stricmp(szName, "PANICCANACTIVATE") )
	{
		m_bPanicCanActivate = IsTrueChar(*szValue);
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "MOVE") )
	{
		if ( !_stricmp(szValue, "WALK") )
		{
			m_eMovement = kAP_Walk;
			return LTTRUE;
		}
		else if ( !_stricmp(szValue, "RUN") )
		{
			m_eMovement = kAP_Run;
			return LTTRUE;
		}
		else if ( !_stricmp(szValue, "SWIM") )
		{
			m_eMovement = kAP_Swim;
			return LTTRUE;
		}
	}

	else if ( !_stricmp(szName, "RANGETIME") )
	{
		m_fRangeTime = (LTFLOAT)atof(szValue);
		return LTTRUE;
	}
	
	else if ( !_stricmp(szName, "RANGE") )
	{
		m_fRangeSqr = (LTFLOAT)atof(szValue);
		m_fRangeSqr *= m_fRangeSqr;
		return LTTRUE;
	}

	return LTFALSE;
}

