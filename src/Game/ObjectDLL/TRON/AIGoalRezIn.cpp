// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRezIn.cpp
//
// PURPOSE : AIGoalRezIn implementation
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (TU 0x101f6a10..
//         0x101f6f00). Names of methods that are not virtuals of the base
//         are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalRezIn.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AI.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "RelationMgr.h"
#include "ObjectRelationMgr.h"
#include "RelationButeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRezIn, kGoal_RezIn);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRezIn::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalRezIn::CAIGoalRezIn()
{
	// RETAIL: 0x101f6af0.
	m_bSeekTarget = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRezIn::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRezIn::ActivateGoal()
{
	// RETAIL: 0x101f6cc0.
	super::ActivateGoal();

	SetStateRezIn();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRezIn::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRezIn::DeactivateGoal()
{
	// RETAIL: 0x101f6ce0.
	m_pAI->HideCharacter( LTFALSE );

	super::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRezIn::SetStateRezIn
//
//	PURPOSE:	Play the rez in animation.
//
// ----------------------------------------------------------------------- //

void CAIGoalRezIn::SetStateRezIn()
{
	// RETAIL: 0x101f6d00.
	m_pGoalMgr->LockGoal( this );

	// An AI with no relation to the player yet hates the player by name.

	CPlayerObj* pPlayer = g_pCharacterMgr->FindRandomPlayer();
	if( pPlayer )
	{
		CObjectRelationMgr* pORM = CRelationMgr::GetGlobalRelationMgr()->GetObjectRelationMgr( pPlayer->m_hObject );
		if( !pORM )
		{
			m_fCurImportance = 0.f;
			return;
		}

		if( INVALID == GetAlignment( m_pAI->GetRelationMgr()->CObjectRelationMgr::GetRelationUser()->GetRelations(), pORM->GetData() ) )
		{
			RelationDescription RD;
			RD.eTrait = RelationTraits::kTrait_Name;
			pORM->GetData().GetTraitValue( RelationTraits::kTrait_Name, RD.szValue, RELATION_VALUE_LENGTH );
			RD.eAlignment = HATE;
			m_pAI->GetRelationMgr()->AddRelation( RD );
		}
	}

	m_pAI->SetState( kState_HumanAnimate );

	CAIHumanStateAnimate* pStateAnimate = dynamic_cast<CAIHumanStateAnimate*>( m_pAI->GetState() );
	if( pStateAnimate )
	{
		CAnimationProps animProps;
		animProps.Set( kAPG_Posture, kAP_Stand );
		animProps.Set( kAPG_Action, kAP_RezIn );
		pStateAnimate->SetAnimation( animProps, LTFALSE );

		m_pAI->HideCharacter( LTTRUE );
		m_pAI->SetAwareness( kAware_Alert );
	}
	else
	{
		m_fCurImportance = 1.f;

		m_pAI->HideCharacter( LTFALSE );
		m_pAI->SetAwareness( kAware_Alert );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRezIn::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRezIn::UpdateGoal()
{
	// RETAIL: 0x101f6e70.
	m_pAI->SetAwareness( kAware_Alert );

	if( m_pAI->GetState()->GetStateType() != kState_HumanAnimate )
	{
		super::UpdateGoal();
	}
	else
	{
		HandleStateAnimate();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRezIn::HandleStateAnimate
//
//	PURPOSE:	Determine what to do when in state Animate.
//
// ----------------------------------------------------------------------- //

void CAIGoalRezIn::HandleStateAnimate()
{
	// RETAIL: 0x101f6ea0.
	if( m_pAI->GetState()->GetStateStatus() == kSStat_StateComplete )
	{
		m_pAI->HideCharacter( LTFALSE );
		m_pGoalMgr->UnlockGoal( this );

		// Chase if armed.

		if( m_pAI->GetPrimaryWeapon() || m_pAI->HasHolsterString() )
		{
			m_fCurImportance = 21.f;
			ActivateChase();
		}
		else
		{
			m_fCurImportance = 1.f;
		}
	}
}
