// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponContinuousFireBase.cpp
//
// PURPOSE : Tron's base of its continuous fire weapons (retail only,
//           written from CShell.dll 0x10018330..0x100189b0; see
//           ClientWeaponContinuousFireBase.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponContinuousFireBase.h"
#include "ScreenSubroutines.h"	// RETAIL: its statics' initializers (0x100182c0..)
#include "TRONPlayerStats.h"
#include "PlayerMgr.h"

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

// RETAIL: 0x10018470.  Tells the server the fire stopped, if it started;
// a key up before the first fire message stops the next one
void CClientWeaponContinuousFireBase::HandleFireKeyUpTransition()
{
	CAutoMessage cMsg;

	if ( m_bFiring )
	{
		cMsg.Writeuint8( MID_WEAPON_CONTINUOUS_FIRE_STOP );
		cMsg.Writeuint8( GetFireMessageType() );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

		m_bFiring = false;
		g_pPlayerMgr->EndServerAccurateRotation();
		g_pPlayerMgr->EndSendingCameraOffsetToServer();
	}
	else
	{
		m_bKeyUpEarly = true;
	}
}

namespace
{
	// RETAIL: ClientWeapon.cpp's (the same name)
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
}

// RETAIL: 0x10018590.  The fire animation keeps playing while the key is
// down and the player has energy; the energy use is noted when it starts
// and TronUnknown98B20 runs when the post-fire animation starts.  A new
// fire keeps the shot's subroutines (SaveShotSubroutines) before its first
// animation.  NEAR: retail keeps dwState in a register and the post-fire
// animation on the stack, ours the reverse (tried "!= 0" for the "!!").
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
			SaveShotSubroutines();
			PlayAnimation( dwPreFireAni, true, 1.0f, false );
			return true;
		}

		if ( bHasFireAni )
		{
			SaveShotSubroutines();
			PlayAnimation( dwFireAni, true, 1.0f, false );
			SetTronUnknown204( GetEnergyUse() );
			return true;
		}

		if ( bHasPostFireAni )
		{
			SaveShotSubroutines();
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
			SetTronUnknown204( GetEnergyUse() );
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
			SaveShotSubroutines();
			PlayAnimation( dwPreFireAni, true, 1.0f, false );
			return true;
		}
	}

	DoSpecialEndFire();
	return false;
}

// RETAIL: 0x10018810.  One message starts the fire (the server fires
// until MID_WEAPON_CONTINUOUS_FIRE_STOP); none of the fire's vectors are
// sent.  A key up before it cancels this one.
void CClientWeaponContinuousFireBase::SendFireMessage( bool bFire,
                                                       LTFLOAT fPerturb,
                                                       LTVector const &vFirePos,
                                                       LTVector const &vDir,
                                                       LTVector const &vImpactPos,
                                                       HOBJECT hObject )
{
	if ( m_bKeyUpEarly )
	{
		m_bFiring = false;
		m_bKeyUpEarly = false;
		return;
	}

	m_bFiring = true;
	m_bKeyUpEarly = false;
	g_pPlayerMgr->StartServerAccurateRotation();
	g_pPlayerMgr->StartSendingCameraOffsetToServer();

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_CONTINUOUS_FIRE_START );
	cMsg.Writeuint8( GetFireMessageType() );
	cMsg.Writeuint8( m_nWeaponId );
	cMsg.Writeuint8( m_nAmmoId );
	cMsg.Writefloat( m_fTronUnknown204 );
	cMsg.Writebool( true );
	TronUnknownVirtual110( cMsg );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

// RETAIL: 0x10018990.  Fires while the player has any energy.
bool CClientWeaponContinuousFireBase::TronUnknownVirtual164()
{
	return ( g_pTronPlayerStats->GetEnergy() > MATH_EPSILON );
}
