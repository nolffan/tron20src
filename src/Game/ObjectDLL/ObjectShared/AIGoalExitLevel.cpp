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
		super::HandleStateGoto();
	}
}
