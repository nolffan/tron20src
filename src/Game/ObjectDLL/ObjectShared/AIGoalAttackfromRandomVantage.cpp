// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromRandomVantage.cpp
//
// PURPOSE : AIGoalAttackFromRandomVantage implementation
//
// CREATED : 2/20/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackFromRandomVantage.h"
#include "AI.h"
#include "AIHumanState.h"
#include "AIGoalMgr.h"
#include "AINodeMgr.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromRandomVantage, kGoal_AttackFromRandomVantage);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromRandomVantage::Con/destructor
//
//	PURPOSE:	RETAIL: Tron's (0x10214cb0): sets the goal's +0x58 flag.
//
// ----------------------------------------------------------------------- //

CAIGoalAttackFromRandomVantage::CAIGoalAttackFromRandomVantage()
{
	m_bTronUnknown58 = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromRandomVantage::SetStateAttack
//
//	PURPOSE:	Set AttackFromVantage state.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromRandomVantage::SetStateAttack()
{
	super::SetStateAttack();

	// Hardcoded to target the player. This is special
	// case code for a boss fight.  Could parameterize 
	// who is being targeted.

	CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
	if( pPlayer )
	{
		m_pAI->Target( pPlayer->m_hObject );
	}

	CAIHumanStateAttackFromVantage* pAttackState = (CAIHumanStateAttackFromVantage*)m_pAI->GetState();
	// RETAIL: Tron's two flags first (0x10214dfd)
	pAttackState->SetTronUnknown8C( true );
	pAttackState->SetTronUnknown8D( true );
	pAttackState->IgnoreVisibility( LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromRandomVantage::HandleStateAttackFromVantage
//
//	PURPOSE:	Determine what to do when in state AttackFromVantage.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromRandomVantage::HandleStateAttackFromVantage()
{
	// Check for failed completion.
	// Do not allow the super to process, because we do not want AI to charge.

	// RETAIL: Tron's (0x10214e20): search for another node at once, with
	// the player as the threat when there is none.

	if( m_pAI->GetState()->GetStateStatus() == kSStat_FailedComplete )
	{
		m_hFailedNode = m_hNode;
		m_hNode = LTNULL;

		if( !m_hStimulusSource )
		{
			CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
			if( pPlayer )
			{
				m_hStimulusSource = pPlayer->m_hObject;
			}
		}

		LTBOOL bForceSearch = m_bForceSearch;
		m_bForceSearch = LTTRUE;
		HandleGoalAttractors();
		m_bForceSearch = bForceSearch;

		if( m_hNode )
		{
			SetStateAttack();
		}

		return;
	}

	// Let the superclass do default handling.

	super::HandleStateAttackFromVantage();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromRandomVantage::FindNearestAttractorNode
//
//	PURPOSE:	Find an attractor node.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAttackFromRandomVantage::FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr)
{
	// RETAIL: Tron tries the nodes in a random order (0x10214ef7).
	return g_pAINodeMgr->FindRandomNodeFromThreat( m_pAI, eNodeType, vPos, m_hStimulusSource, false );
}
