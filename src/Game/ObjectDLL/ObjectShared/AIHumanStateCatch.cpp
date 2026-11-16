
//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateCatch.cpp
//              
//	PURPOSE:	CAIHumanStateCatch implementation
//              
//	CREATED:	12.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIHUMANSTATECATCH_H__
#include "AIHumanStateCatch.h"		
#endif

#ifndef __AI_HUMAN_H__
#include "AIHuman.h"
#endif 

#ifndef __AI_TARGET_H__
#include "AITarget.h"
#endif

#include "TrackedNodeContext.h"

// Forward declarations

// Globals

// Statics

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCatch, kState_HumanCatch);


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::CAIHumanStateCatch()
//              
//	PURPOSE:	Handle initialization of all variables whose value is known
//				at construction of the class (without the AI present) and 
//				which are potentially not initialized later.
//              
//----------------------------------------------------------------------------
CAIHumanStateCatch::CAIHumanStateCatch()
{
	// RETAIL: 0x1022b180.

	// Reset the state tracking 
	m_bCaughtNamedObject = false;

	m_hObjectToCatch = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::Load()
//              
//	PURPOSE:	Restore data after a load
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::Load(ILTMessage_Read *pMsg)
{
	// RETAIL: 0x1022b2b0.
	CAIHumanState::Load(pMsg);

	LOAD_VECTOR( m_vIncomingPosition );
	LOAD_HOBJECT( m_hObjectToCatch );
	LOAD_bool( m_bCaughtNamedObject );
	LOAD_BYTE_CAST( m_ePosture, EnumAnimProp );
	LOAD_BYTE_CAST( m_eDirection, EnumAnimProp );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::Save()
//              
//	PURPOSE:	Save data
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::Save(ILTMessage_Write *pMsg)
{
	// RETAIL: 0x1022b360.
	CAIHumanState::Save(pMsg);

	SAVE_VECTOR( m_vIncomingPosition );
	SAVE_HOBJECT( m_hObjectToCatch );
	SAVE_bool( m_bCaughtNamedObject );
	SAVE_BYTE( m_ePosture );
	SAVE_BYTE( m_eDirection );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::HandleTouch()
//              
//	PURPOSE:	Note when the AI touches the object it means to catch.
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::HandleTouch(HOBJECT hObject)
{
	if ( hObject == m_hObjectToCatch )
	{
		m_bCaughtNamedObject = true;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::Init()
//              
//	PURPOSE:	Setup the catch.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateCatch::Init(CAIHuman* pAIHuman)
{
	// RETAIL: 0x1022b3f0.
	CAIHumanState::Init(pAIHuman);

	// Set the incoming position to a position 48 units in front of the AI.
	m_vIncomingPosition =  GetAI()->GetPosition() + GetAI()->GetEyeForward() * 48;

	// Ensure that node tracking is disabled.
	m_pAIHuman->DisableNodeTracking();

	// Set the Catching AI to an alert state/combat type
	m_pAIHuman->SetAwareness( kAware_Alert );

	m_ePosture = kAP_None;

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::Update()
//              
//	PURPOSE:	Update the catch.
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::Update(void)
{
	// RETAIL: 0x1022b4a0.
	CAIHumanState::Update();

	// Look at the object until it is caught.
	if ( !m_bCaughtNamedObject )
	{
		m_pAIHuman->EnableNodeTracking( kTrack_AimAt, m_hObjectToCatch );
	}
	else
	{
		m_pAIHuman->DisableNodeTracking();
	}

	switch( m_eStateStatus )
	{
		case kSStat_Initialized:
			HandleStartCatch();
			break;

		case kSStat_TronUnknown39:
			HandleHoldCatch();
			break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::UpdateAnimation()
//              
//	PURPOSE:	Set the catch animation.
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::UpdateAnimation(void)
{
	// RETAIL: 0x1022b4f0.
	CAIHumanState::UpdateAnimation();

	if ( m_ePosture == kAP_None )
	{
		m_ePosture = m_pAIHuman->GetAnimationContext()->GetCurrentProp( kAPG_Posture );
		if ( ( m_ePosture == kAP_None ) || ( m_ePosture == kAP_Any ) )
		{
			m_ePosture = kAP_Stand;
		}

		GetAnimationContext()->SetProp( kAPG_Posture, m_ePosture );
	}
	else
	{
		GetAnimationContext()->SetProp( kAPG_Posture, m_ePosture );
	}
	GetAnimationContext()->SetProp( kAPG_WeaponPosition, kAP_Up );

	if ( m_eStateStatus != kSStat_TronUnknown39 )
	{
		GetAnimationContext()->SetProp( kAPG_WeaponAction, kAP_Aim );
	}
	else
	{
		GetAnimationContext()->SetProp( kAPG_WeaponAction, kAP_HoldCatch );
		GetAnimationContext()->SetProp( kAPG_Direction, m_eDirection );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::ShouldDropCatch()
//              
//	PURPOSE:	Returns true if the catch should end.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateCatch::ShouldDropCatch(void)
{
	// If the AI should stop after Catching ANY object, then see
	// if we have defected one.
	if ( m_bCaughtNamedObject )
	{
		return LTTRUE;
	}

	// If the named object no longer exists, then we obviously don't want to
	// wait around for it to show up.  Stop trying to catch it now.
	return ( m_hObjectToCatch == LTNULL );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::HandleStartCatch()
//              
//	PURPOSE:	Start the catch: pick the posture and direction.
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::HandleStartCatch()
{
	// RETAIL: 0x1022b5c0.
	if ( GetAI()->HasTarget() )
	{
		// Setting a target push speed allows the AI to knock the 
		// player back if they collide while the AI Catching
		GetAI()->GetTarget()->SetPushSpeed( GetAI()->GetSpeed() * 2.f );
	}

	m_ePosture = m_pAIHuman->GetAnimationContext()->GetCurrentProp( kAPG_Posture );
	m_eDirection = m_pAIHuman->GetAnimationContext()->GetCurrentProp( kAPG_Direction );
	if ( ( m_eDirection == kAP_None ) || ( m_eDirection == kAP_Any ) )
	{
		m_eDirection = ( GetRandom( 0.f, 1.f ) > 0.5 ) ? kAP_Left : kAP_Right;
	}

	if( ShouldDropCatch() )
	{
		m_eStateStatus = kSStat_StateComplete;
	}
	else
	{
		m_eStateStatus = kSStat_TronUnknown39;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::HandleHoldCatch()
//              
//	PURPOSE:	Hold the catch until it is done.
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::HandleHoldCatch()
{
	// RETAIL: 0x1022b680.
	if( ShouldDropCatch() )
	{
		m_eStateStatus = kSStat_StateComplete;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::SetObjectToCatch()
//              
//	PURPOSE:	Set the object to catch.
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::SetObjectToCatch(HOBJECT hObject)
{
	// RETAIL: 0x1022b6a0.
	m_hObjectToCatch = hObject;
}
