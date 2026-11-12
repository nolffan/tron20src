// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGlitch.cpp
//
// PURPOSE : AIGoalGlitch implementation
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (TU 0x101f8ef0..
//         0x101f94f0). Names of members and methods are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalGlitch.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AINode.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalGlitch, kGoal_Glitch);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGlitch::Save / Load
//
//	PURPOSE:	Save / Load (RETAIL: 0x101f9470 / 0x101f94b0)
//
// ----------------------------------------------------------------------- //

void CAIGoalGlitch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bGlitching);
	SAVE_FLOAT(m_fNextGlitchTime);
}

void CAIGoalGlitch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bGlitching);
	LOAD_FLOAT(m_fNextGlitchTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGlitch::ActivateGoal / DeactivateGoal
//
//	PURPOSE:	(RETAIL: 0x101f9200 / 0x101f9220)
//
// ----------------------------------------------------------------------- //

void CAIGoalGlitch::ActivateGoal()
{
	super::ActivateGoal();

	m_bGlitching = false;
	m_fNextGlitchTime = 0.f;
}

void CAIGoalGlitch::DeactivateGoal()
{
	super::DeactivateGoal();

	m_bGlitching = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGlitch::UpdateGoal
//
//	PURPOSE:	Update goal (RETAIL: 0x101f9240).
//
// ----------------------------------------------------------------------- //

void CAIGoalGlitch::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanUseObject:
			// Glitch while walking to the node, once it is time to.
			if( ( m_pAI->GetState()->GetStateStatus() == kSStat_Moving ) &&
				( g_pLTServer->GetTime() > m_fNextGlitchTime ) )
			{
				m_pAI->SetState( kState_HumanAnimate );
				CAIHumanStateAnimate* pAnimate = dynamic_cast<CAIHumanStateAnimate*>(m_pAI->GetState());
				if( pAnimate )
				{
					CAnimationProps animProps;
					animProps.Set( kAPG_Posture, kAP_Stand );
					animProps.Set( kAPG_Awareness, kAP_Glitch );
					pAnimate->SetAnimation( animProps, LTFALSE );

					m_bGlitching = true;
					return;
				}
			}
			break;

		case kState_HumanAnimate:
			// Done glitching: back to work.
			if( m_pAI->GetState()->GetStateStatus() == kSStat_StateComplete )
			{
				m_bGlitching = false;
				m_fNextGlitchTime = g_pLTServer->GetTime() + GetRandom( 2.f, 3.f );
				SetStateUseObject();
			}
			return;
	}

	super::UpdateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGlitch::RecalcImportance
//
//	PURPOSE:	Forget the last node used once far enough from it
//				(RETAIL: 0x101f9370).
//
// ----------------------------------------------------------------------- //

void CAIGoalGlitch::RecalcImportance()
{
	if( m_hLastNodeUseObject )
	{
		AINode* pNode = dynamic_cast<AINode*>(g_pLTServer->HandleToObject(m_hLastNodeUseObject));
		if( !pNode )
		{
			return;
		}

		LTFLOAT fDist = m_pAI->GetPosition().Dist( pNode->GetPos() );
		ObjectCPrint( ToString( m_pAI->m_hObject ), "Is %f far away from last node", fDist );

		if( fDist > 512.f )
		{
			m_hLastNodeUseObject = LTNULL;
		}
	}

	super::RecalcImportance();
}
