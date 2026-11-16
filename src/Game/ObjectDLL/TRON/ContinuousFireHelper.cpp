// ----------------------------------------------------------------------- //
//
// MODULE  : ContinuousFireHelper.cpp
//
// PURPOSE : Tron's server helper for the continuous fire weapons.
//           Retail only (Object.lto 0x101606f0..0x10161670), written from
//           the retail code. Names are ours where not said otherwise.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ContinuousFireHelper.h"
#include "WeaponMgr.h"
#include "PlayerObj.h"
#include "AI.h"
#include "ServerUtilities.h"
#include "AutoMessage.h"
#include "CommonUtilities.h"
#include "AITarget.h"
#include "AnimationMgr.h"
#include "Weapon.h"
#include "WeaponFireInfo.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CContinuousFireHelper::~CContinuousFireHelper()
//
// ----------------------------------------------------------------------- //

CContinuousFireHelper::~CContinuousFireHelper()
{
	// RETAIL: 0x101606f0
	Stop();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CContinuousFireHelper::CContinuousFireHelper()
//
// ----------------------------------------------------------------------- //

CContinuousFireHelper::CContinuousFireHelper()
:	m_hObject			( LTNULL ),
	m_lstTargets		( ),
	m_nTronUnknown18	( 0 ),
	m_bOn				( false ),
	m_pWeapon			( LTNULL ),
	m_pAmmo				( LTNULL ),
	m_fTronUnknown24	( 0 )
{
	// RETAIL: 0x101607a0
	for( int i = 0; i < ContinuousFireData::kNumFields; ++i )
	{
		ContinuousFireData::FieldFlags &Flags = m_FireData.m_aFlags[i];
		Flags.m_bSet = false;
		Flags.m_bTronUnknown1 = false;
	}
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::Init(HOBJECT hObject)
{
	// RETAIL: 0x10160870
	m_hObject = hObject;
	m_bOn = false;
	m_pWeapon = LTNULL;
	m_pAmmo = LTNULL;
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::Save(ILTMessage_Write *pMsg)
{
	// RETAIL: 0x101608b0
	pMsg->WriteObject( m_hObject );

	pMsg->Writeuint32( m_lstTargets.size() );
	for( TargetList::iterator it = m_lstTargets.begin(); it != m_lstTargets.end(); ++it )
	{
		pMsg->WriteObject( *it );
	}

	pMsg->Writeuint8( m_nTronUnknown18 );

	pMsg->Writebool( m_pWeapon != LTNULL );
	if( m_pWeapon )
	{
		pMsg->Writeuint8( (uint8)m_pWeapon->nId );
	}

	pMsg->Writebool( m_pAmmo != LTNULL );
	if( m_pAmmo )
	{
		pMsg->Writeuint8( (uint8)m_pAmmo->nId );
	}

	pMsg->Writefloat( m_fTronUnknown24 );

	WriteFireData( pMsg );
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::Load(ILTMessage_Read *pMsg)
{
	// RETAIL: 0x101609b0
	m_hObject = pMsg->ReadObject();

	uint32 nTargets = pMsg->Readuint32();
	for( ; nTargets; --nTargets )
	{
		m_lstTargets.push_back( LTObjRef( pMsg->ReadObject() ) );
	}

	m_nTronUnknown18 = pMsg->Readuint8();

	if( pMsg->Readbool() )
	{
		SetWeapon( pMsg->Readuint8() );
	}

	if( pMsg->Readbool() )
	{
		SetAmmo( pMsg->Readuint8() );
	}

	m_fTronUnknown24 = pMsg->Readfloat();

	ReadFireData( pMsg );
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::SetTronUnknown18(uint8 nValue)
{
	// RETAIL: 0x10160b00
	m_nTronUnknown18 = nValue;
}

void CContinuousFireHelper::SetWeapon(uint8 nWeaponId)
{
	// RETAIL: 0x10160b10
	m_pWeapon = g_pWeaponMgr->GetWeapon( nWeaponId );
}

void CContinuousFireHelper::SetAmmo(uint8 nAmmoId)
{
	// RETAIL: 0x10160b30
	m_pAmmo = g_pWeaponMgr->GetAmmo( nAmmoId );
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::Update()
{
	// RETAIL: 0x10160b50
	RemoveDeadTargets();

	if( m_bOn )
	{
		if( dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject( m_hObject ) ) )
		{
			UpdatePlayer();
		}
		else
		{
			UpdateAI();
		}
	}
}

// ----------------------------------------------------------------------- //

bool CContinuousFireHelper::AddTarget(HOBJECT hTarget)
{
	// RETAIL: 0x10160ba0
	for( TargetList::iterator it = m_lstTargets.begin(); it != m_lstTargets.end(); ++it )
	{
		if( hTarget == *it )
			return false;
	}

	m_lstTargets.push_back( LTObjRef( hTarget ) );
	return true;
}

// ----------------------------------------------------------------------- //

bool CContinuousFireHelper::RemoveTarget(HOBJECT hTarget)
{
	// RETAIL: 0x10160c70
	for( TargetList::iterator it = m_lstTargets.begin(); it != m_lstTargets.end(); ++it )
	{
		if( hTarget == *it )
		{
			m_lstTargets.erase( it );
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //

bool CContinuousFireHelper::GetFirePos(LTVector *pvPos)
{
	// RETAIL: 0x10160cb0
	if( m_bOn )
	{
		g_pLTServer->GetObjectPos( m_hObject, pvPos );

		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject( m_hObject ) );
		if( pPlayer )
		{
			*pvPos += pPlayer->GetClientCameraOffset();
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //

bool CContinuousFireHelper::Start(ILTMessage_Read *pMsg)
{
	// RETAIL: 0x10160d50
	SetTronUnknown18( pMsg->Readuint8() );
	SetWeapon( pMsg->Readuint8() );
	SetAmmo( pMsg->Readuint8() );
	m_fTronUnknown24 = pMsg->Readfloat();

	if( pMsg->Readbool() )
	{
		ReadFireData( pMsg );
	}

	m_bOn = true;
	return true;
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::Stop()
{
	// RETAIL: 0x10160de0
	m_bOn = false;

	TargetList::iterator it = m_lstTargets.begin();
	while( it != m_lstTargets.end() )
	{
		if( *it )
		{
			ReleaseTarget( *it++ );
		}
		else
		{
			++it;
		}
	}

	m_lstTargets.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CContinuousFireHelper::UpdatePlayer()
//
//	PURPOSE:	Makes a fire message as the client would and gives it to
//				the player (CPlayerObj::HandleWeaponFireMessage).
//
// ----------------------------------------------------------------------- //

bool CContinuousFireHelper::UpdatePlayer()
{
	// RETAIL: 0x10160e50
	CAutoMessage cMsg;

	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject( m_hObject ) );

	cMsg.Writeuint8( GetMessageID() );
	cMsg.Writeuint8( (uint8)m_pWeapon->nId );
	cMsg.Writeuint8( (uint8)m_pAmmo->nId );

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	vPos += pPlayer->GetClientCameraOffset();

	cMsg.WriteLTVector( vPos );		// fire position
	cMsg.WriteLTVector( vPos );		// flash position

	LTRotation rRot = pPlayer->GetRotationWithPitch();
	cMsg.WriteLTVector( rRot.Forward() );

	cMsg.Writeuint8( GetRandom( 2, 255 ) );
	cMsg.Writeuint8( 0 );
	cMsg.Writefloat( g_pLTServer->GetTime() );

	WriteExtra( cMsg );
	WriteFireData( cMsg );

	cMsg.Writebool( false );

	pPlayer->HandleWeaponFireMessage( CLTMsgRef_Read( cMsg.Read() ) );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CContinuousFireHelper::UpdateAI()
//
//	PURPOSE:	Fires the AI's weapon at its target while its fire
//				animation plays.
//
// ----------------------------------------------------------------------- //

bool CContinuousFireHelper::UpdateAI()
{
	// RETAIL: 0x101610f0
	CAI *pAI = dynamic_cast<CAI*>( g_pLTServer->HandleToObject( m_hObject ) );
	if( !pAI || !pAI->HasTarget() || !pAI->GetCurrentWeapon() )
	{
		Stop();
		return false;
	}

	if( !pAI->GetAnimationContext()->IsPropSet( kAPG_WeaponAction, kAP_Fire ) &&
		pAI->GetAnimationContext()->GetCurrentProp( kAPG_WeaponAction ) != kAP_Fire )
	{
		Stop();
		pAI->GetAnimationContext()->SetAnimRate( 1.0f );
		return false;
	}

	LTVector vFirePos = pAI->GetWeaponPosition( pAI->GetCurrentWeapon() );

	CAITarget *pTarget = pAI->GetTarget();
	pTarget->UpdateVisibility();
	LTVector vDir = pTarget->GetVisiblePosition() - vFirePos;

	// PORT: Tron's WeaponFireInfo is larger than NOLF2's (0x16c bytes,
	// with a ContinuousFireData at +0x68); retail also sets its +0x50 to
	// the AI's GetRelationData(). Ours has no such member.
	WeaponFireInfo weaponFireInfo;
	weaponFireInfo.hSocket		= pAI->GetWeaponSocket( pAI->GetCurrentWeapon() );
	weaponFireInfo.hFiredFrom	= pAI->m_hObject;
	weaponFireInfo.vPath		= vDir;
	weaponFireInfo.vFirePos		= vFirePos;
	weaponFireInfo.vFlashPos	= vFirePos;
	weaponFireInfo.fPerturbR	= 0.0f;
	weaponFireInfo.fPerturbU	= 0.0f;

	if( pAI->GetCurrentWeapon()->UpdateWeapon( weaponFireInfo, LTTRUE ) == W_FIRED )
	{
		CWeapon *pWeapon = pAI->GetCurrentWeapon();
		pWeapon->PlayAnimation( pWeapon->GetFireAni(), false, true, false );
	}

	return true;
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::RemoveDeadTargets()
{
	// RETAIL: 0x10161380
	TargetList::iterator it = m_lstTargets.begin();
	while( it != m_lstTargets.end() )
	{
		if( !*it )
		{
			TargetList::iterator itDead = it;
			++it;
			m_lstTargets.erase( itDead );
		}
		else
		{
			++it;
		}
	}
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::ReleaseTarget(HOBJECT hTarget)
{
	// RETAIL: 0x101613c0. The target sends the message to itself.
	LPBASECLASS pTarget = g_pLTServer->HandleToObject( hTarget );
	SendTriggerMsgToObject( pTarget, g_pLTServer->HandleToObject( hTarget ), LTTRUE,
		"GOAL_SpecialDamage CONTINUOUSFIRERELEASE=0" );
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::ReadFireData(ILTMessage_Read *pMsg)
{
	// RETAIL: 0x10161400
	for( int i = 0; i < ContinuousFireData::kNumFields; ++i )
	{
		ContinuousFireData::FieldFlags &Flags = m_FireData.m_aFlags[i];
		Flags.m_bSet = pMsg->Readbool();
		Flags.m_bTronUnknown1 = pMsg->Readbool();
	}

	if( m_FireData.m_aFlags[0].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field0 );
	if( m_FireData.m_aFlags[1].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field1 );
	if( m_FireData.m_aFlags[2].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field2 );
	if( m_FireData.m_aFlags[3].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field3 );
	if( m_FireData.m_aFlags[4].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field4 );
	if( m_FireData.m_aFlags[5].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field5 );
	if( m_FireData.m_aFlags[6].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field6 );
	if( m_FireData.m_aFlags[7].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field7 );
	if( m_FireData.m_aFlags[8].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field8 );
	if( m_FireData.m_aFlags[9].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field9 );
	if( m_FireData.m_aFlags[10].m_bSet )
		pMsg->ReadType( &m_FireData.m_Field10 );
}

// ----------------------------------------------------------------------- //

void CContinuousFireHelper::WriteFireData(ILTMessage_Write *pMsg)
{
	// RETAIL: 0x10161530
	for( int i = 0; i < ContinuousFireData::kNumFields; ++i )
	{
		ContinuousFireData::FieldFlags &Flags = m_FireData.m_aFlags[i];
		pMsg->Writebool( Flags.m_bSet );
		pMsg->Writebool( Flags.m_bTronUnknown1 );
	}

	if( m_FireData.m_aFlags[0].m_bSet )
		pMsg->WriteType( m_FireData.m_Field0 );
	if( m_FireData.m_aFlags[1].m_bSet )
		pMsg->WriteType( m_FireData.m_Field1 );
	if( m_FireData.m_aFlags[2].m_bSet )
		pMsg->WriteType( m_FireData.m_Field2 );
	if( m_FireData.m_aFlags[3].m_bSet )
		pMsg->WriteType( m_FireData.m_Field3 );
	if( m_FireData.m_aFlags[4].m_bSet )
		pMsg->WriteType( m_FireData.m_Field4 );
	if( m_FireData.m_aFlags[5].m_bSet )
		pMsg->WriteType( m_FireData.m_Field5 );
	if( m_FireData.m_aFlags[6].m_bSet )
		pMsg->WriteType( m_FireData.m_Field6 );
	if( m_FireData.m_aFlags[7].m_bSet )
		pMsg->WriteType( m_FireData.m_Field7 );
	if( m_FireData.m_aFlags[8].m_bSet )
		pMsg->WriteType( m_FireData.m_Field8 );
	if( m_FireData.m_aFlags[9].m_bSet )
		pMsg->WriteType( m_FireData.m_Field9 );
	if( m_FireData.m_aFlags[10].m_bSet )
		pMsg->WriteType( m_FireData.m_Field10 );
}
