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

namespace
{
	// also defined in ClientWeapon.cpp;
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
}

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

// RETAIL: 0x10017910: CTronClientWeapon's, but the main fire animation
// holds while the fire key is down (it loops after the pre-fire one), and
// a finished post-fire animation starts the next shot
bool CClientWeaponBall::PlayFireAnimation( bool bResetAni )
{
	uint32 dwAni			= g_pLTClient->GetModelAnimation(m_hObject);
	uint32 dwState			= g_pLTClient->GetModelPlaybackState(m_hObject);

	uint32 dwPreFireAni		= GetPreFireAni();
	uint32 dwFireAni		= GetFireAni( m_eLastFireType );
	uint32 dwPostFireAni	= GetPostFireAni();

	bool bHasPreFireAni		= !!(INVALID_ANI != dwPreFireAni);
	bool bHasFireAni		= !!(INVALID_ANI != dwFireAni);
	bool bHasPostFireAni	= !!(INVALID_ANI != dwPostFireAni);

	bool bIsFireAni			= IsFireAni( dwAni );

	bool bIsMainFireAni		= IsFireAni( dwAni, true );
	bool bIsPreFireAni		= IsPreFireAni( dwAni );
	bool bIsPostFireAni		= IsPostFireAni( dwAni );

	bool bIsCurAniDone		= !!(dwState & MS_PLAYDONE);

	float fRate = 1.0f;

	if ( !bIsFireAni )
	{
		if ( bResetAni )
		{
			if ( bHasPreFireAni )
			{
				SaveShotSubroutines();
				PlayAnimation( dwPreFireAni, true, fRate );
				return true;
			}

			if ( bHasFireAni )
			{
				SaveShotSubroutines();
				PlayAnimation( dwFireAni, true, fRate );
				return true;
			}

			if ( bHasPostFireAni )
			{
				SaveShotSubroutines();
				PlayAnimation( dwPostFireAni, true, fRate );
				return true;
			}

			return false;
		}
	}
	else if ( bIsPreFireAni && bIsCurAniDone )
	{
		if ( bResetAni )
		{
			if ( bHasFireAni )
			{
				PlayAnimation( dwFireAni, true, fRate, true );
				return true;
			}
		}
		else if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, fRate );
			return true;
		}
	}
	else if ( bIsMainFireAni )
	{
		if ( bResetAni )
		{
			return true;
		}

		if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, fRate );
			return true;
		}
	}
	else if ( !bIsCurAniDone )
	{
		return true;
	}

	DoSpecialEndFire();

	if ( bIsPostFireAni && bResetAni && bIsCurAniDone )
	{
		if ( bHasPreFireAni )
		{
			SaveShotSubroutines();
			PlayAnimation( dwPreFireAni, true, fRate );
			return true;
		}

		if ( bHasFireAni )
		{
			SaveShotSubroutines();
			PlayAnimation( dwFireAni, true, fRate );
			return true;
		}

		if ( bHasPostFireAni )
		{
			SaveShotSubroutines();
			PlayAnimation( dwPostFireAni, true, fRate );
			return true;
		}
	}

	return false;
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
