// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponBall.cpp
//
// PURPOSE : Tron's client-side ball weapon (retail only, written from
//           CShell.dll 0x10017670..0x10017c1f; see ClientWeaponBall.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponBall.h"
#include "FXButeMgr.h"
#include "WeaponMgr.h"

// RETAIL: 0x10017870
CClientWeaponBall::CClientWeaponBall()
	: CTronClientWeapon(),
	  m_fFireKeyDownTime( 0.0f )
{
}

// RETAIL: 0x10017670
CClientWeaponBall::~CClientWeaponBall()
{
}

// RETAIL: 0x10017890
void CClientWeaponBall::Load( ILTMessage_Read *pMsg )
{
	m_fFireKeyDownTime = pMsg->Readfloat() + g_pLTClient->GetTime();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x100178d0
void CClientWeaponBall::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fFireKeyDownTime - g_pLTClient->GetTime() );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x10017b70
void CClientWeaponBall::HandleFireKeyDownTransition()
{
	m_fFireKeyDownTime = g_pLTClient->GetTime();
}

// RETAIL: 0x10017b90.  The throw's velocity: the min one before the min
// throw time, the max one after the max throw time, else the hold time
// times the slope between them (retail adds no offsets: probably a retail
// bug, to list in docs/RETAIL_BUGS.md when this function matches; now it
// differs only in its registers).
void CClientWeaponBall::AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg )
{
	BALLCLASSDATA *pBallData = dynamic_cast< BALLCLASSDATA* >( m_pAmmo->pProjectileFX->pClassData );

	float fHoldTime = g_pLTClient->GetTime() - m_fFireKeyDownTime;

	float fVelocity;
	if ( pBallData->fMinThrowTime > fHoldTime )
	{
		fVelocity = pBallData->fMinThrowVelocity;
	}
	else if ( pBallData->fMaxThrowTime < fHoldTime )
	{
		fVelocity = pBallData->fMaxThrowVelocity;
	}
	else
	{
		fVelocity = ( pBallData->fMaxThrowVelocity - pBallData->fMinThrowVelocity ) /
			( pBallData->fMaxThrowTime - pBallData->fMinThrowTime ) * fHoldTime;
	}

	pMsg->Writefloat( fVelocity );
}
