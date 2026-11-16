//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateDeflect.cpp
//              
//	PURPOSE:	CAIHumanStateDeflect implementation
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

#ifndef __AIHUMANSTATEDEFLECT_H__
#include "AIHumanStateDeflect.h"		
#endif

#ifndef __AI_HUMAN_H__
#include "AIHuman.h"
#endif 

#ifndef __AI_TARGET_H__
#include "AITarget.h"
#endif

#include "AIBrain.h"
#include "AIUtils.h"
#include "TrackedNodeContext.h"

// Forward declarations

// Globals

// Statics

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDeflect, kState_HumanDeflect);


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::CAIHumanStateDeflect() / ~
//              
//	PURPOSE:	Con/destructor
//              
//----------------------------------------------------------------------------
CAIHumanStateDeflect::CAIHumanStateDeflect()
{
	// RETAIL: 0x1022b990.
}

CAIHumanStateDeflect::~CAIHumanStateDeflect()
{
	// RETAIL: 0x1022ba10, the defense ends with the state.
	if( m_pAI )
	{
		m_pAI->SetDefending( false );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::Load() / Save()
//              
//	PURPOSE:	Restore / save data
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::Load(ILTMessage_Read *pMsg)
{
	// RETAIL: 0x1022ba80.
	CAIHumanState::Load(pMsg);

	LOAD_FLOAT( m_fDefendTime );
	LOAD_FLOAT( m_fDeflectEndTime );
	LOAD_HOBJECT( m_hObjectToDeflect );
	LOAD_bool( m_bDeflectedNamedObject );
	LOAD_BYTE_CAST( m_ePosture, EnumAnimProp );
	LOAD_BYTE_CAST( m_eDirection, EnumAnimProp );
}

void CAIHumanStateDeflect::Save(ILTMessage_Write *pMsg)
{
	// RETAIL: 0x1022bb20.
	CAIHumanState::Save(pMsg);

	SAVE_FLOAT( m_fDefendTime );
	SAVE_FLOAT( m_fDeflectEndTime );
	SAVE_HOBJECT( m_hObjectToDeflect );
	SAVE_bool( m_bDeflectedNamedObject );
	SAVE_BYTE( m_ePosture );
	SAVE_BYTE( m_eDirection );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::HandleModelString()
//              
//	PURPOSE:	A DEFLECT model string turns the defense on or off.
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::HandleModelString(ArgList* pArgList)
{
	// RETAIL: 0x1022bbb0.
	super::HandleModelString(pArgList);

	if( !pArgList || !pArgList->argv || ( pArgList->argc < 2 ) || !pArgList->argv[0] || !pArgList->argv[1] )
	{
		return;
	}

	if( !_stricmp( pArgList->argv[0], "DEFLECT" ) )
	{
		char cFlag = pArgList->argv[1][0];
		if( IsTrueChar( cFlag ) )
		{
			m_pAI->SetDefending( true );
		}
		else if( IsFalseChar( cFlag ) )
		{
			m_pAI->SetDefending( false );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::HandleTouch()
//              
//	PURPOSE:	Note when the AI touches the object it means to deflect.
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::HandleTouch(HOBJECT hObject)
{
	// RETAIL: 0x1022bc50.
	if ( hObject == m_hObjectToDeflect )
	{
		m_bDeflectedNamedObject = true;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::Init()
//              
//	PURPOSE:	Setup the deflection.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateDeflect::Init(CAIHuman* pAIHuman)
{
	// RETAIL: 0x1022bc70.
	if( !CAIHumanState::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	m_bDeflectedNamedObject = false;
	m_hObjectToDeflect = LTNULL;

	m_fDefendTime = pAIHuman->GetBrain()->GetAIData( kAIData_MaxDeflectDuration ) * 0.5f;
	m_fDeflectEndTime = g_pLTServer->GetTime() + pAIHuman->GetBrain()->GetAIData( kAIData_MaxDeflectDuration );

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::Update()
//              
//	PURPOSE:	Update the deflection.
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::Update(void)
{
	// RETAIL: 0x1022bd10.
	CAIHumanState::Update();

	// Look at the object until it is deflected.
	if ( !m_bDeflectedNamedObject )
	{
		m_pAIHuman->EnableNodeTracking( kTrack_AimAt, m_hObjectToDeflect );
	}
	else
	{
		m_pAIHuman->DisableNodeTracking();
	}

	switch( m_eStateStatus )
	{
		case kSStat_Initialized:
			SetupDeflection();
			break;

		case kSStat_StateComplete:
			if( !m_pAIHuman->GetAnimationContext()->IsTransitioning() )
			{
				m_pAI->SetDefending( false );
			}
			break;

		case kSStat_TronUnknown40:
			ShouldDropDeflection();
			break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::ShouldDropDeflection()
//              
//	PURPOSE:	Keep the defense on for a while, then end the deflection when
//				the object is gone or the time is up.
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::ShouldDropDeflection(void)
{
	// RETAIL: 0x1022bd80.
	if( m_pAIHuman->GetAnimationContext()->IsTransitioning() )
	{
		return;
	}

	if( g_pLTServer->GetTime() > m_fDefendTime )
	{
		if( ( m_hObjectToDeflect == LTNULL ) || ( g_pLTServer->GetTime() > m_fDeflectEndTime ) )
		{
			m_eStateStatus = kSStat_StateComplete;
		}
	}
	else
	{
		m_pAI->SetDefending( true );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::SetupDeflection()
//              
//	PURPOSE:	Start the deflection: pick the posture and direction.
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::SetupDeflection(void)
{
	// RETAIL: 0x1022bdf0.

	// Setting a target push speed allows the AI to knock the 
	// player back if they collide while the AI is deflecting
	GetAI()->GetTarget()->SetPushSpeed( GetAI()->GetSpeed() * 2.f );

	m_ePosture = m_pAIHuman->GetAnimationContext()->GetCurrentProp( kAPG_Posture );
	if ( ( m_ePosture == kAP_None ) || ( m_ePosture == kAP_Any ) )
	{
		m_ePosture = kAP_Stand;
	}

	m_eDirection = m_pAIHuman->GetAnimationContext()->GetCurrentProp( kAPG_Direction );
	if ( ( m_eDirection == kAP_None ) || ( m_eDirection == kAP_Any ) )
	{
		m_eDirection = ( GetRandom( 0.f, 1.f ) > 0.5 ) ? kAP_Left : kAP_Right;
	}

	m_eStateStatus = kSStat_TronUnknown40;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::UpdateAnimation()
//              
//	PURPOSE:	Set the deflection animation.
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::UpdateAnimation(void)
{
	// RETAIL: 0x1022bea0.
	CAIHumanState::UpdateAnimation();

	GetAnimationContext()->SetProp( kAPG_Posture, m_ePosture );
	GetAnimationContext()->SetProp( kAPG_WeaponPosition, kAP_Up );

	if ( m_eStateStatus != kSStat_TronUnknown40 )
	{
		GetAnimationContext()->SetProp( kAPG_WeaponAction, kAP_Aim );
	}
	else
	{
		GetAnimationContext()->SetProp( kAPG_WeaponAction, kAP_HoldDeflect );
		GetAnimationContext()->SetProp( kAPG_Direction, m_eDirection );
	}
}
