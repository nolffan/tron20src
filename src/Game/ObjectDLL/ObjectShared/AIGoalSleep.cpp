// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSleep.cpp
//
// PURPOSE : AIGoalSleep implementation
//
// CREATED : 7/11/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalSleep.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSleep, kGoal_Sleep);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSleep::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSleep::CAIGoalSleep()
{
	m_bRequireBareHands = LTTRUE;
	m_bAllowDialogue = LTFALSE;
	m_bTurnOffLights = LTTRUE;
	m_bTurnOnLights = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSleep::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalSleep::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalSleep::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSleep::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSleep::ActivateGoal()
{
	super::ActivateGoal();

	m_pAI->SetAwareness( kAware_Relaxed );

	// RETAIL: 0x101fbe85, a sleeping AI in Tron does not see, and does not
	// hear footsteps (mask 0xff83f620).
	m_pAI->SetCurSenseFlags( ~( kSense_SeeEnemy | kSense_SeeEnemyLean | kSense_SeeEnemyWeaponImpact |
		kSense_SeeEnemyDanger | kSense_SeeEnemyDisturbance | kSense_SeeEnemyFootprint |
		kSense_SeeAllyDeath | kSense_SeeAllyDisturbance | kSense_HearEnemyFootstep |
		kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile | kSense_SeeAllyDistress |
		kSense_SeeAllySpecialDamage | kSense_SeeInappropriateBehavior ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSleep::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSleep::DeactivateGoal()
{
	super::DeactivateGoal();

	// Remove Zzz when done sleeping...
	
	if( m_pAI->HasZzz() )
		m_pAI->DestroyZzz();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSleep::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSleep::UpdateGoal()
{
	super::UpdateGoal();

	CAIState* pState = m_pAI->GetState();
	if( pState->GetStateType() != kState_HumanUseObject )
	{
		return;
	}

	if( pState->GetStateStatus() == kSStat_StateComplete )
	{
		// Remove Zzz when done sleeping...
	
		if( m_pAI->HasZzz() )
			m_pAI->DestroyZzz();
	}

	if( pState->GetStateStatus() != kSStat_PathComplete )
	{
		return;
	}

	// RETAIL: 0x101fbefe, Tron's sleeper keeps its senses and does not
	// snore; the Zzz's come and go with the fidgets.

	CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)pState;
	pStateUseObject->SetVulnerable( LTTRUE );

	// No Zzz's while fidgeting.

	AIASSERT( m_pAI->GetAnimationContext(), m_pAI->m_hObject, "CAIGoalSleep::UpdateGoal: NULL animation context." );
	if( m_pAI->GetAnimationContext()->IsPropSet( kAPG_Action, kAP_Fidget ) )
	{
		// Remove Zzz when fidgeting...
	
		if( m_pAI->HasZzz() )
			m_pAI->DestroyZzz();
	}
	else
	{		
		// Create Zzz when begining to sleep...

		if( !m_pAI->HasZzz() )
			m_pAI->CreateZzz();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSleep::HandleDamage
//
//	PURPOSE:	Handle damage...
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalSleep::HandleDamage(const DamageStruct& damage)
{
	if( m_pGoalMgr->IsCurGoal( this ))
	{
		// Make sure the AI can be damaged by the damagetype...

		if( m_pAI->GetDestructible()->IsCantDamageType( damage.eType ) || !m_pAI->GetDestructible()->GetCanDamage() || damage.fDamage <= 0.0f)
		{
			return LTFALSE;
		}

		// If the AI is fidgetting don't do one shot one kill...

		if( m_pAI->GetAnimationContext()->IsPropSet( kAPG_Action, kAP_Fidget ) )
		{
			return LTFALSE;
		}
	
		// Kill it...

		m_pAI->GetDestructible()->HandleDestruction( damage.hDamager );

		return LTTRUE;
	}

	return LTFALSE;
}
