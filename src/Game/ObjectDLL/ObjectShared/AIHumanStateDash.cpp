//----------------------------------------------------------------------------
//
//	MODULE:		AIHumanStateDash.cpp
//
//	PURPOSE:	CAIHumanStateDash implementation
//
//	RETAIL:		Tron 2.0 only: written from the retail Object.lto
//				(0x1022cc90..0x1022d490). See AIHumanStateDash.h.
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "AIHumanStateDash.h"
#include "AIHuman.h"
#include "AIHumanStrategy.h"
#include "AITarget.h"
#include "AIMovement.h"
#include "AIVolume.h"
#include "TrackedNodeContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDash, kState_HumanDash);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateDash::Con/destructor (RETAIL: 0x1022cd40,
//				0x1022cde0)
//
// ----------------------------------------------------------------------- //

CAIHumanStateDash::CAIHumanStateDash()
{
	m_fTronUnknown7C = 0.f;
	m_bDashing = false;

	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
}

CAIHumanStateDash::~CAIHumanStateDash()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateDash::Init (RETAIL: 0x1022ce50)
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateDash::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	GetAI()->SetAwareness( kAware_Alert );

	HOBJECT hTarget = GetAI()->GetTarget()->GetObject();
	GetAI()->SetNodeTrackingTarget( kTrack_AimAt, hTarget, "Head" );
	GetAI()->EnableNodeTracking( kTrack_AimAt, LTNULL );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateDash::Update (RETAIL: 0x1022ced0)
//
// ----------------------------------------------------------------------- //

void CAIHumanStateDash::Update()
{
	super::Update();

	if( GetAI()->HasTarget() && GetAI()->GetCurrentWeapon() )
	{
		CAITarget* pTarget = GetAI()->GetTarget();
		HOBJECT hTarget = pTarget->GetObject();

		if( pTarget->GetCharacter()->GetLastVolume() )
		{
		LTVector vTargetPos = pTarget->GetCharacter()->GetLastVolumePos();

		// Close enough already: just aim.

		if( !m_bDashing &&
			( VEC_DISTSQR( vTargetPos, GetAI()->GetPosition() ) < m_fTronUnknown7C ) &&
			GetAI()->GetAIMovement()->IsUnset() )
		{
			GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
			m_eStateStatus = kSStat_StateComplete;
			return;
		}

		if( !m_bDashing && GetAI()->GetAIMovement()->IsUnset() )
		{
			const LTVector& vDest = m_pStrategyFollowPath->GetDest();
			const LTVector& vPos = GetAI()->GetPosition();
			LTVector vToDest = LTVector( vDest.x - vPos.x, vDest.y - vPos.y, vDest.z - vPos.z );

			if( vToDest.MagSqr() < 25.f )
			{
				// Reached the end of the path: dash on past the target.

				LTVector vDir = vTargetPos - GetAI()->GetPosition();
				vDir = vDir / vDir.Mag();

				LTVector vDash = GetAI()->GetPosition() + ( vDir * 128.f );
				m_pStrategyFollowPath->Set( vDash, LTFALSE );
				m_bDashing = true;
			}
			else if( m_pStrategyFollowPath->IsUnset() || m_pStrategyFollowPath->IsDone() )
			{
				if( !m_pStrategyFollowPath->Set( vTargetPos, LTFALSE ) )
				{
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Cannot find path to target!" ) );
					GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
					m_eStateStatus = kSStat_StateComplete;
					return;
				}
			}
		}

		if( !( m_bDashing == true &&
			   ( m_pStrategyFollowPath->IsDone() || m_pStrategyFollowPath->IsUnset() ) ) )
		{
			if( m_pStrategyFollowPath->IsSet() )
			{
				m_pStrategyFollowPath->Update();
			}

			GetAI()->GetTarget()->SetPushSpeed( 800.f );
			GetAI()->GetTarget()->SetPushMinDist( m_pAI->GetDims().z * 2.f );
			return;
		}
		}
		else
		{
			AITRACE( AIShowStates, ( GetAI()->m_hObject, "Cannot find path to target!" ) );
			GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
		}
	}

	m_eStateStatus = kSStat_StateComplete;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateDash::UpdateAnimation (RETAIL: 0x1022d3c0)
//
// ----------------------------------------------------------------------- //

void CAIHumanStateDash::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	m_pStrategyFollowPath->UpdateAnimation();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateDash::Load/Save (RETAIL: 0x1022d400,
//				0x1022d440)
//
// ----------------------------------------------------------------------- //

void CAIHumanStateDash::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	LOAD_FLOAT(m_fTronUnknown7C);
	LOAD_bool(m_bDashing);
}

void CAIHumanStateDash::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	SAVE_FLOAT(m_fTronUnknown7C);
	SAVE_bool(m_bDashing);
}
