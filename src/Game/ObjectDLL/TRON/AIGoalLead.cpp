// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalLead.cpp
//
// PURPOSE : AIGoalLead implementation
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (TU 0x101f8780..
//         0x101f8eb0). Names of members and methods are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalLead.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AI.h"
#include "CharacterMgr.h"

// RETAIL: the TU reads the player's HOBJECT through the virtual
// GetUserObject() (IObjectRelationMgrUser, at 0x40). It does not include
// PlayerObj.h (none of PlayerShared.h's static initializers), so the player
// is seen as the CCharacter it is (at offset 0).
#define LEAD_PLAYER() ((CCharacter*)g_pCharacterMgr->FindPlayer())
// RETAIL: the TU does not include TrackedNodeContext.h (it has none of that
// header's static initializers), so kTrack_LookAt is spelled out.
#define LEAD_TRACK_LOOKAT ((EnumTrackedNodeGroup)0)	// kTrack_LookAt

extern CCharacterMgr* g_pCharacterMgr;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalLead, kGoal_Lead);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::CAIGoalLead
//
//	PURPOSE:	Constructor (RETAIL: 0x101f89a0)
//
// ----------------------------------------------------------------------- //

CAIGoalLead::CAIGoalLead()
{
	m_fWaitDist = 524.f;
	m_fContinueDist = 128.f;
	m_bLookAround = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::ActivateGoal
//
//	PURPOSE:	Activate goal (RETAIL: 0x101f8a70). Skips CAIGoalGoto's
//				activation: the AI only resumes walking if it was already
//				in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalLead::ActivateGoal()
{
	CAIGoalAbstract::ActivateGoal();

	if( m_pAI->GetState()->GetStateType() == kState_HumanGoto )
	{
		CAIHumanStateGoto* pGoto = (CAIHumanStateGoto*)m_pAI->GetState();
		pGoto->SetDestNode(m_hDestNode);
		pGoto->SetMovement(m_eMovement);
		pGoto->SetAwareness(m_eAwareness);

		m_pGoalMgr->LockGoal(this);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::UpdateGoal
//
//	PURPOSE:	Update goal (RETAIL: 0x101f8ad0).
//
// ----------------------------------------------------------------------- //

void CAIGoalLead::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();
	LTFLOAT fDist = GetPlayerDistance();

	switch(pState->GetStateType())
	{
		case kState_HumanIdle:
			{
				m_pAI->SetState( kState_HumanGoto );
				CAIHumanStateGoto* pGoto = (CAIHumanStateGoto*)m_pAI->GetState();
				pGoto->SetDestNode(m_hDestNode);
				pGoto->SetMovement(m_eMovement);
				pGoto->SetAwareness(m_eAwareness);

				m_pAI->DisableNodeTracking();
				m_pGoalMgr->LockGoal(this);
			}
			break;

		case kState_HumanGoto:
			{
				// Wait for the player when he falls behind.
				if( fDist >= m_fWaitDist )
				{
					m_pAI->SetState( kState_HumanAnimate );
					CAIHumanStateAnimate* pAnimate = dynamic_cast<CAIHumanStateAnimate*>(m_pAI->GetState());
					if( !m_bLookAround )
					{
						CAnimationProps animProps;
						animProps.Set( kAPG_Posture, kAP_Stand );
						animProps.Set( kAPG_WeaponPosition, kAP_Up );
						animProps.Set( kAPG_Action, kAP_LookAround );
						pAnimate->SetAnimation( animProps, LTFALSE );

						m_pAI->EnableNodeTracking( LEAD_TRACK_LOOKAT, LEAD_PLAYER()->GetUserObject() );
						m_bLookAround = false;
					}
				}
				else
				{
					HandleStateGoto();
				}
			}
			break;

		case kState_HumanAnimate:
			{
				// Start walking again when the player has caught up.
				if( fDist < m_fContinueDist )
				{
					m_pAI->SetState( kState_HumanGoto );
					CAIHumanStateGoto* pGoto = (CAIHumanStateGoto*)m_pAI->GetState();
					pGoto->SetDestNode(m_hDestNode);
					pGoto->SetMovement(m_eMovement);
					pGoto->SetAwareness(m_eAwareness);

					m_pAI->DisableNodeTracking();
					m_pGoalMgr->LockGoal(this);
				}
				else
				{
					HandleStateAnimate();
				}
			}
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::HandleStateAnimate
//
//	PURPOSE:	Alternate looking around and beckoning while waiting
//				(RETAIL: 0x101f8c80).
//
// ----------------------------------------------------------------------- //

void CAIGoalLead::HandleStateAnimate()
{
	CAIHumanStateAnimate* pAnimate = (CAIHumanStateAnimate*)m_pAI->GetState();

	if( m_pAI->IsNodeTrackingEnabled() )
	{
		m_pAI->SetNodeTrackingTarget( LEAD_TRACK_LOOKAT, LEAD_PLAYER()->GetUserObject(), "Head" );
	}

	if( pAnimate->GetStateStatus() == kSStat_StateComplete )
	{
		if( m_bLookAround )
		{
			CAnimationProps animProps;
			animProps.Set( kAPG_Posture, kAP_Stand );
			animProps.Set( kAPG_WeaponPosition, kAP_Lower );
			animProps.Set( kAPG_Action, kAP_LookAround );
			pAnimate->SetAnimation( animProps, LTFALSE );

			m_pAI->EnableNodeTracking( LEAD_TRACK_LOOKAT, LEAD_PLAYER()->GetUserObject() );
			m_bLookAround = false;
		}
		else
		{
			CAnimationProps animProps;
			animProps.Set( kAPG_Posture, kAP_Stand );
			animProps.Set( kAPG_WeaponPosition, kAP_Up );
			animProps.Set( kAPG_Action, kAP_Taunt );
			pAnimate->SetAnimation( animProps, LTFALSE );

			m_pAI->EnableNodeTracking( LEAD_TRACK_LOOKAT, LEAD_PLAYER()->GetUserObject() );
			m_bLookAround = true;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::GetPlayerDistance
//
//	PURPOSE:	Distance from the AI to the player (RETAIL: 0x101f8db0).
//
// ----------------------------------------------------------------------- //

LTFLOAT CAIGoalLead::GetPlayerDistance()
{
	CCharacter* pPlayer = LEAD_PLAYER();

	LTVector vPlayerPos;
	g_pLTServer->GetObjectPos( pPlayer->GetUserObject(), &vPlayerPos );

	// RETAIL: near; retail copies the AI's position's y and z to the stack
	// first (an inlined by-value vector?) and subtracts it from the player's.
	return m_pAI->GetPosition().Dist( vPlayerPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::HandleNameValuePair
//
//	PURPOSE:	Handles updating the goal's parameters (RETAIL: 0x101f8e30).
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalLead::HandleNameValuePair(const char *szName, const char *szValue)
{
	// RETAIL: returns LTFALSE when CAIGoalGoto took the pair (NODE,
	// MOVEMENT, AWARENESS); harmless, CAIGoalMgr ignores the result.
	if( !super::HandleNameValuePair(szName, szValue) )
	{
		if ( !_stricmp(szName, "WAIT_DIST") )
		{
			m_fWaitDist = (LTFLOAT)atof(szValue);
			return LTTRUE;
		}

		else if ( !_stricmp(szName, "CONTINUE_DIST") )
		{
			m_fContinueDist = (LTFLOAT)atof(szValue);
			return LTTRUE;
		}
	}

	return LTFALSE;
}
