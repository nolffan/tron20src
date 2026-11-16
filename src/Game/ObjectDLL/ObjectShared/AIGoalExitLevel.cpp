// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalExitLevel.cpp
//
// PURPOSE : AIGoalExitLevel implementation
//
// CREATED : 3/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalExitLevel.h"
#include "AINodeGuard.h"
#include "AINodeMgr.h"
#include "AIState.h"
#include "AI.h"
#include "AIHumanState.h"
#include "AIGoalMgr.h"
#include "CVarTrack.h"
#include "..\TRON\AINodeExitLevelByRezOut.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevelAbstract::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalExitLevelAbstract::CAIGoalExitLevelAbstract()
{
	// RETAIL: 0x10208870, Tron sets the node type in InitGoal.
	m_fMinImportance = 0.f;
	m_fExitAllowedTime = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevelAbstract::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevelAbstract::InitGoal(CAI* pAI)
{
	// RETAIL: 0x102088b0.
	m_eNodeType = GetNodeType();

	super::InitGoal(pAI);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevel::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevel::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevelAbstract::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	
	SAVE_TIME( m_fExitAllowedTime );
}

void CAIGoalExitLevelAbstract::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_TIME( m_fExitAllowedTime );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevel::RecalcImportance
//
//	PURPOSE:	Bail if AI has a guard node.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevelAbstract::RecalcImportance()
{
	// Ensure that AI has been in the level for at least 5 secs before
	// trying to exit.  This gives AIs a chance to sense whatever may 
	// be going on.

	if( m_fExitAllowedTime == 0.f )
	{
		m_fExitAllowedTime = g_pLTServer->GetTime() + 5.f;
	}
	else if( g_pLTServer->GetTime() < m_fExitAllowedTime )
	{
		m_fCurImportance = 0.f;
		return;
	}

	// If we already own a Patrol node or Guard node, we will Patrol or Guard so bail.
	// An AI cannot both Patrol or Guard, and ExitLevel.

	if( g_pAINodeMgr->FindOwnedNode( kNode_Guard, m_pAI->m_hObject ) )
	{
		m_fCurImportance = 0.f;
		return;
	}

	super::RecalcImportance();
}

// ----------------------------------------------------------------------- //

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalExitLevel, kGoal_ExitLevel);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevel::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevel::HandleStateGoto()
{
	// If I made it to my ExitLevel node, remove me!
	if( m_pAI->GetState()->GetStateStatus() == kSStat_StateComplete )
	{
		SendTriggerMsgToObject(m_pAI, m_pAI->m_hObject, LTFALSE, "REMOVE");

		// RETAIL: 0x10208b00, Tron runs the node's OnUseCmd.
		AINodeExitLevel* pNode = (AINodeExitLevel*)g_pAINodeMgr->FindOwnedNode( kNode_ExitLevel, m_pAI->m_hObject );
		if( IsKindOf( pNode->m_hObject, "AINodeExitLevel" ) )
		{
			pNode->ProcessOnUseCmd( m_pAI->GetUserObject() );
		}
	}
	else
	{
		CAIGoalGuard::HandleStateGoto();
	}
}


// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalExitLevelByRezOut
//
// ----------------------------------------------------------------------- //

// RETAIL: a console variable (retail 0x1038b828) that prints when the goal
// locks; the C++ name is ours.
static CVarTrack s_vtAIGoalExitLevel;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalExitLevelByRezOut, kGoal_ExitLevelByRezOut);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevelByRezOut::HandleStateGoto
//
//	PURPOSE:	At the node, start to rez out.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevelByRezOut::HandleStateGoto()
{
	// RETAIL: 0x10208cd0
	if( m_pAI->GetState()->GetStateStatus() == kSStat_StateComplete )
	{
		StartRezOut();
	}
	else
	{
		CAIGoalGuard::HandleStateGoto();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevelByRezOut::UpdateGoal
//
//	PURPOSE:	Handle the states of the rez out.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevelByRezOut::UpdateGoal()
{
	// RETAIL: 0x10208cf0
	switch( m_pAI->GetState()->GetStateType() )
	{
		case kState_HumanHolster:
			HandleStateHolster();
			break;

		case kState_HumanUseObject:
			HandleStateUseObject();
			break;

		default:
			CAIGoalGuard::UpdateGoal();
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevelByRezOut::StartRezOut
//
//	PURPOSE:	Use the node's smart object.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevelByRezOut::StartRezOut()
{
	// RETAIL: 0x10208d30
	AINode* pOwnedNode = g_pAINodeMgr->FindOwnedNode( kNode_ExitLevelByRezOut, m_pAI->m_hObject );
	AINodeExitLevelByRezOut* pNode = dynamic_cast<AINodeExitLevelByRezOut*>( pOwnedNode );
	if( !pNode )
	{
		m_fCurImportance = 0.f;
		return;
	}

	m_pAI->SetState( kState_HumanUseObject );

	CAIHumanStateUseObject* pStateUseObject = dynamic_cast<CAIHumanStateUseObject*>( m_pAI->GetState() );
	if( pStateUseObject )
	{
		pStateUseObject->SetWeaponPosition( kAP_Up );
		pStateUseObject->SetRequireBareHands( LTTRUE );
		pStateUseObject->SetAllowDialogue( LTFALSE );
		pStateUseObject->TurnOnLights( LTFALSE );
		pStateUseObject->TurnOffLights( LTFALSE );
		pStateUseObject->CopySmartObjectCommand( pNode->GetSmartObjectCommand() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevelByRezOut::HandleStateUseObject
//
//	PURPOSE:	Determine what to do when in state UseObject.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevelByRezOut::HandleStateUseObject()
{
	// RETAIL: 0x10208e10
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_HolsterWeapon:
			m_pAI->SetState( kState_HumanHolster );
			break;

		case kSStat_PathComplete:
			// Do not let anything interrupt the rez out.
			m_fCurImportance = FLT_MAX;
			m_pGoalMgr->LockGoal( this );
			m_pAI->GetDestructible()->SetCanDamage( LTFALSE );

			s_vtAIGoalExitLevel.Init( g_pLTServer, "AIGoalExitLevel", LTNULL, 0.0f );
			if( s_vtAIGoalExitLevel.GetFloat() > 0.0f )
			{
				ObjectCPrint( m_pAI->m_hObject, "Locked the RezOut goal, should not be interuptable." );
			}
			break;

		case kSStat_StateComplete:
			{
				SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, "REMOVE" );

				AINodeExitLevelByRezOut* pNode = (AINodeExitLevelByRezOut*)g_pAINodeMgr->FindOwnedNode( kNode_ExitLevelByRezOut, m_pAI->m_hObject );
				if( IsKindOf( pNode->m_hObject, "AINodeExitLevelByRezOut" ) )
				{
					pNode->ProcessOnUseCmd( m_pAI->GetUserObject() );
				}
			}
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevelByRezOut::HandleStateHolster
//
//	PURPOSE:	Determine what to do when in state Holster.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevelByRezOut::HandleStateHolster()
{
	// RETAIL: 0x10208fa0
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_FailedComplete:
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			StartRezOut();
			break;
	}
}
