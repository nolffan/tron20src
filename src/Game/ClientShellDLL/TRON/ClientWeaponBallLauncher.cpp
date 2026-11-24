// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponBallLauncher.cpp
//
// PURPOSE : Tron's client-side ball launcher (retail only, written from
//           CShell.dll 0x10017c20..0x1001807c; see ClientWeaponBallLauncher.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponBallLauncher.h"
#include "SubroutineMgr.h"

// RETAIL: 0x10017c60
CClientWeaponBallLauncher::CClientWeaponBallLauncher()
	: CTronClientWeapon(),
	  m_fFireAnimationRate( 1.0f )
{
}

// RETAIL: 0x10017c20
CClientWeaponBallLauncher::~CClientWeaponBallLauncher()
{
}

// RETAIL: 0x1001ad20
void CClientWeaponBallLauncher::Load( ILTMessage_Read *pMsg )
{
	m_fFireAnimationRate = pMsg->Readfloat();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x1001ad50
void CClientWeaponBallLauncher::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fFireAnimationRate );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x10017e90
float CClientWeaponBallLauncher::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[4][0] )
		{
			return fBase * m_fTronUnknown164 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Ball_launcher" ) )
	{
		BallLauncherData data = g_pSubroutineDataMgr->GetBallLauncherData( GetSubroutineDataKey( "Ball_launcher" ) );
		return data.fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x10018070.  The rate CClientWeapon keeps for the subroutine
// (retail's +0x158).
void CClientWeaponBallLauncher::UpdateFireAnimationRate()
{
	m_fFireAnimationRate = m_fTronUnknown158;
}

namespace
{
	// RETAIL: ClientWeapon.cpp's (the same name)
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
}

// RETAIL: 0x10017c80.  The fire animations play at m_fFireAnimationRate (the
// subroutine's rate).  PORT: before most of them retail calls
// CTronClientWeapon's 0x10097f20 (a helper of four more Tron helpers), not
// in our build.  A main fire animation plays again while the fire key is
// down and TronUnknownVirtual164 allows it; retail has no case for a done
// post-fire animation (IsPostFireAni is called but not used).
bool CClientWeaponBallLauncher::PlayFireAnimation( bool bResetAni )
{
	uint32 dwAni			= g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState			= g_pLTClient->GetModelPlaybackState( m_hObject ) & MS_PLAYDONE;

	uint32 dwPreFireAni		= GetPreFireAni();
	uint32 dwFireAni		= GetFireAni( m_eLastFireType );
	uint32 dwPostFireAni	= GetPostFireAni();

	bool bHasFireAni		= !!( INVALID_ANI != dwFireAni );
	bool bHasPostFireAni	= !!( INVALID_ANI != dwPostFireAni );

	bool bIsFireAni			= IsFireAni( dwAni );
	bool bIsMainFireAni		= IsFireAni( dwAni, true );
	bool bIsPreFireAni		= IsPreFireAni( dwAni );
	IsPostFireAni( dwAni );

	bool bIsCurAniDone		= !!dwState;

	if ( !bIsFireAni )
	{
		if ( !bResetAni )
		{
			DoSpecialEndFire();
			return false;
		}

		if ( INVALID_ANI != dwPreFireAni )
		{
			SaveShotSubroutines();
			UpdateFireAnimationRate();
			PlayAnimation( dwPreFireAni, true, GetFireAnimationRate(), false );
			return true;
		}

		if ( bHasFireAni )
		{
			SaveShotSubroutines();
			UpdateFireAnimationRate();
			PlayAnimation( dwFireAni, true, GetFireAnimationRate(), false );
			return true;
		}

		if ( bHasPostFireAni )
		{
			SaveShotSubroutines();
			UpdateFireAnimationRate();
			PlayAnimation( dwPostFireAni, true, GetFireAnimationRate(), false );
			return true;
		}

		return false;
	}

	if ( !bIsCurAniDone )
	{
		return true;
	}

	if ( bIsPreFireAni )
	{
		if ( bHasFireAni )
		{
			PlayAnimation( dwFireAni, true, GetFireAnimationRate(), false );
			return true;
		}
		else if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, GetFireAnimationRate(), false );
			return true;
		}
	}
	else if ( bIsMainFireAni )
	{
		if ( bResetAni && TronUnknownVirtual164() )
		{
			SaveShotSubroutines();
			PlayAnimation( dwFireAni, true, GetFireAnimationRate(), false );
			return true;
		}
		else if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, GetFireAnimationRate(), false );
			return true;
		}
	}

	DoSpecialEndFire();
	return false;
}
