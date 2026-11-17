// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponContinuousFireBase.cpp
//
// PURPOSE : Tron's base of its continuous fire weapons (retail only,
//           written from CShell.dll 0x10018330..0x100189b0; see
//           ClientWeaponContinuousFireBase.h).
//
//           Not yet written: PlayFireAnimation (0x10018590),
//           HandleFireKeyUpTransition (0x10018470; it clears two bools of
//           Tron's CPlayerMgr, +0x1a8/+0x1a9, ours lacks),
//           SendFireMessage (0x10018810) and TronUnknownVirtual164
//           (0x10018990; it needs CTronPlayerStats' float energy, 0x100a9c90).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponContinuousFireBase.h"
#include "ScreenSubroutines.h"	// RETAIL: its statics' initializers (0x100182c0..)
#include "TRONPlayerStats.h"

// RETAIL: 0x10018360
CClientWeaponContinuousFireBase::CClientWeaponContinuousFireBase()
	: CTronClientWeapon()
{
	m_bFiring = false;
	m_bKeyUpEarly = false;
}

// RETAIL: 0x10018330
CClientWeaponContinuousFireBase::~CClientWeaponContinuousFireBase()
{
}

// RETAIL: 0x10018380: while firing, whether it may fire comes from
// TronUnknownVirtual164()
WeaponState CClientWeaponContinuousFireBase::Update( bool bFire, bool bAltFire, FireType eFireType )
{
	if ( bFire )
	{
		bFire = TronUnknownVirtual164();
	}

	return CTronClientWeapon::Update( bFire, bAltFire, eFireType );
}

// RETAIL: 0x100183c0
void CClientWeaponContinuousFireBase::Load( ILTMessage_Read *pMsg )
{
	m_bFiring = pMsg->Readbool();
	m_bKeyUpEarly = pMsg->Readbool();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x10018400
void CClientWeaponContinuousFireBase::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writebool( m_bFiring );
	pMsg->Writebool( m_bKeyUpEarly );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x10018450
void CClientWeaponContinuousFireBase::HandleFireKeyDownTransition()
{
	m_bFiring = false;
	m_bKeyUpEarly = false;
	CTronClientWeapon::HandleFireKeyDownTransition();
}

namespace
{
	// RETAIL: ClientWeapon.cpp's (the same name)
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
}

// RETAIL: 0x10018590.  The fire animation keeps playing while the key is
// down and the player has energy; the energy use is noted when it starts
// and TronUnknown98B20 runs when the post-fire animation starts.  PORT:
// before the first animation retail calls CTronClientWeapon's 0x10097f20
// (a helper of four more Tron helpers), and it hands GetEnergyUse() to
// CTronClientWeapon's 0x10098b10 (it sets +0x204) when the main fire
// animation starts: neither is in our build.
bool CClientWeaponContinuousFireBase::PlayFireAnimation( bool bResetAni )
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
	bool bIsPostFireAni		= IsPostFireAni( dwAni );

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
			PlayAnimation( dwPreFireAni, true, 1.0f, false );
			return true;
		}

		if ( bHasFireAni )
		{
			PlayAnimation( dwFireAni, true, 1.0f, false );
			// PORT: 0x10098b10( GetEnergyUse() ) (see above)
			return true;
		}

		if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
			TronUnknown98B20();
			return true;
		}

		return false;
	}

	if ( !bIsCurAniDone )
	{
		// The key went up: stop at once with the post-fire animation
		if ( bIsMainFireAni && !bResetAni && bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
			TronUnknown98B20();
			return true;
		}

		return true;
	}

	if ( bIsPreFireAni )
	{
		if ( bHasFireAni && bResetAni )
		{
			PlayAnimation( dwFireAni, true, 1.0f, false );
			// PORT: 0x10098b10( GetEnergyUse() ) (see above)
			return true;
		}

		if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
			TronUnknown98B20();
			return true;
		}
	}
	else if ( bIsMainFireAni )
	{
		if ( bResetAni && TronUnknownVirtual164() )
		{
			PlayAnimation( dwFireAni, true, 1.0f, false );
			return true;
		}

		if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
			TronUnknown98B20();
			return true;
		}
	}
	else if ( bIsPostFireAni )
	{
		if ( bResetAni && TronUnknownVirtual164() )
		{
			PlayAnimation( dwPreFireAni, true, 1.0f, false );
			return true;
		}
	}

	DoSpecialEndFire();
	return false;
}

// RETAIL: 0x10018990.  Fires while the player has any energy.
bool CClientWeaponContinuousFireBase::TronUnknownVirtual164()
{
	return ( g_pTronPlayerStats->GetEnergy() > MATH_EPSILON );
}
