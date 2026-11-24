// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponLOL.cpp
//
// PURPOSE : Tron's client-side LOL weapon (retail only, written from
//           CShell.dll 0x1001acc0..0x1001b09c; see ClientWeaponLOL.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponLOL.h"
#include "SubroutineMgr.h"

// RETAIL: 0x1001ad00
CClientWeaponLOL::CClientWeaponLOL()
	: CTronClientWeapon(),
	  m_fFireAnimationRate( 1.0f )
{
}

// RETAIL: 0x1001acc0
CClientWeaponLOL::~CClientWeaponLOL()
{
}

// RETAIL: 0x1001ad20
void CClientWeaponLOL::Load( ILTMessage_Read *pMsg )
{
	m_fFireAnimationRate = pMsg->Readfloat();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x1001ad50
void CClientWeaponLOL::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fFireAnimationRate );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x1001ad80
float CClientWeaponLOL::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[3][0] )
		{
			return fBase * m_fTronUnknown15C + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "LOL" ) )
	{
		SubroutineDataKey key = GetSubroutineDataKey( "LOL" );
		return g_pSubroutineDataMgr->GetLOLData( key ).fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

namespace
{
	// RETAIL: ClientWeapon.cpp's (the same name)
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
}

// RETAIL: 0x1001ae80.  The fire animations play at m_fFireAnimationRate (the
// subroutine's rate).  PORT: before most of them retail calls
// CTronClientWeapon's 0x10097f20 (a helper of four more Tron helpers), not
// in our build.  A main fire animation plays again while the fire key is
// down and TronUnknownVirtual164 allows it; retail has no case for a done
// post-fire animation (IsPostFireAni is called but not used).
bool CClientWeaponLOL::PlayFireAnimation( bool bResetAni )
{
	uint32 dwAni			= g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState			= g_pLTClient->GetModelPlaybackState( m_hObject );

	uint32 dwPreFireAni		= GetPreFireAni();
	uint32 dwFireAni		= GetFireAni( m_eLastFireType );
	uint32 dwPostFireAni	= GetPostFireAni();

	bool bHasFireAni		= !!( INVALID_ANI != dwFireAni );
	bool bHasPostFireAni	= !!( INVALID_ANI != dwPostFireAni );

	bool bIsFireAni			= IsFireAni( dwAni );
	bool bIsMainFireAni		= IsFireAni( dwAni, true );
	bool bIsPreFireAni		= IsPreFireAni( dwAni );
	IsPostFireAni( dwAni );

	bool bIsCurAniDone		= !!( dwState & MS_PLAYDONE );

	if ( !bIsFireAni )
	{
		if ( !bResetAni )
		{
			DoSpecialEndFire();
			return false;
		}

		if ( INVALID_ANI != dwPreFireAni )
		{
			UpdateFireAnimationRate();
			PlayAnimation( dwPreFireAni, true, m_fFireAnimationRate, false );
			return true;
		}

		if ( bHasFireAni )
		{
			UpdateFireAnimationRate();
			PlayAnimation( dwFireAni, true, m_fFireAnimationRate, false );
			return true;
		}

		if ( bHasPostFireAni )
		{
			UpdateFireAnimationRate();
			PlayAnimation( dwPostFireAni, true, m_fFireAnimationRate, false );
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
			PlayAnimation( dwFireAni, true, m_fFireAnimationRate, false );
			return true;
		}
		else if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, m_fFireAnimationRate, false );
			return true;
		}
	}
	else if ( bIsMainFireAni )
	{
		if ( bResetAni && TronUnknownVirtual164() )
		{
			PlayAnimation( dwFireAni, true, m_fFireAnimationRate, false );
			return true;
		}
		else if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, m_fFireAnimationRate, false );
			return true;
		}
	}

	DoSpecialEndFire();
	return false;
}

// RETAIL: 0x1001b090.  The rate CClientWeapon keeps for the subroutine
// (retail's +0x154).
void CClientWeaponLOL::UpdateFireAnimationRate()
{
	m_fFireAnimationRate = m_fTronUnknown154;
}
