// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponPrankster.cpp
//
// PURPOSE : Tron's client-side prankster bit launcher (retail only,
//           written from CShell.dll 0x1001b090..0x1001b9a9; see
//           ClientWeaponPrankster.h).
//
//           Not yet written: UpdateModelState (0x1001b460; two arguments
//           there), PlayFireAnimation (0x1001b510) and 0x1001b090 (it reads
//           CClientWeapon+0x154, not in ours).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponPrankster.h"
#include "SubroutineMgr.h"
#include "CMoveMgr.h"
#include "MsgIDs.h"

// RETAIL: 0x1001b100
CClientWeaponPrankster::CClientWeaponPrankster()
	: CTronClientWeapon(),
	  m_nState( kPrankster_Idle )
{
	m_pTimer = new CTimer;
}

// RETAIL: 0x1001b0a0
CClientWeaponPrankster::~CClientWeaponPrankster()
{
	if ( m_pTimer )
	{
		delete m_pTimer;
		m_pTimer = LTNULL;
	}
}

// RETAIL: 0x1001b180
bool CClientWeaponPrankster::OnMessage( uint8 messageID, ILTMessage_Read *pMsg )
{
	if ( MID_PROJECTILE == messageID )
	{
		if ( HandleMessageProjectile( messageID, pMsg ) )
		{
			return true;
		}
	}

	return CTronClientWeapon::OnMessage( messageID, pMsg );
}

// RETAIL: 0x1001b1c0
WeaponState CClientWeaponPrankster::Update( bool bFire, bool bAltFire, FireType eFireType )
{
	return CTronClientWeapon::Update( bFire, bAltFire, eFireType );
}

// RETAIL: 0x1001b1e0
void CClientWeaponPrankster::Select()
{
	SetState( kPrankster_Idle );
	CTronClientWeapon::Select();
}

// RETAIL: 0x1001b210
void CClientWeaponPrankster::Load( ILTMessage_Read *pMsg )
{
	m_nState = pMsg->Readuint8();
	m_pTimer->Start( pMsg->Readfloat() );
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x1001b270
void CClientWeaponPrankster::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writeuint8( (uint8)m_nState );
	pMsg->Writefloat( m_pTimer->GetCountdownTime() );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x1001b330
float CClientWeaponPrankster::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[7][0] )
		{
			return fBase * m_fTronUnknown1DC + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Prankster_bit" ) )
	{
		PranksterBitData data = g_pSubroutineDataMgr->GetPranksterBitData( GetSubroutineDataKey( "Prankster_bit" ) );
		return data.fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x1001b460: the bit keeps the firing animations while it is
// out; its fire puts it out (and the weapon away)
WeaponState CClientWeaponPrankster::UpdateModelState( bool bFire, bool bAltFire )
{
	WeaponState eRet = W_IDLE;

	if ( bFire || kPrankster_KeyDown == m_nState || kPrankster_Out == m_nState || kPrankster_Sent == m_nState )
	{
		UpdateFiring();
	}
	else
	{
		UpdateNonFiring();
	}

	if ( m_bFire )
	{
		if ( kPrankster_KeyDown == m_nState )
		{
			bool bGadgetSpecialCase = m_pAmmo->eType != GADGET;
			eRet = UpdateAmmoFromFire( bGadgetSpecialCase );
			SetState( kPrankster_Out );
			SetVisible( false );
		}

		m_bFire = false;
	}

	// See if we just finished deselecting the weapon...
	if ( m_bWeaponDeselected )
	{
		m_bWeaponDeselected = false;

		if ( m_cbDeselect )
		{
			// call the deselect callback
			m_cbDeselect( m_nWeaponId, m_pcbData );

			// clear out the callback data
			m_cbDeselect = 0;
			m_pcbData = 0;
		}

		// deactivate the current weapon, its not used anymore
		Deactivate();
	}

	return eRet;
}

namespace
{
	// RETAIL: ClientWeapon.cpp's (the same name)
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
}

// RETAIL: 0x1001b510.  A done pre-fire animation starts again while the key
// is held down (kPrankster_KeyDown) with no fire.  PORT: before the
// animations it starts first retail calls CTronClientWeapon's 0x10097f20
// (a helper of four more Tron helpers), not in our build.
bool CClientWeaponPrankster::PlayFireAnimation( bool bResetAni )
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
			return true;
		}

		if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
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
		if ( bHasFireAni && bResetAni )
		{
			PlayAnimation( dwFireAni, true, 1.0f, false );
			return true;
		}
		else if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
			return true;
		}
		else if ( bResetAni && kPrankster_KeyDown == m_nState && !m_bFire )
		{
			PlayAnimation( dwPreFireAni, true, 1.0f, false );
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
		else if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
			return true;
		}
	}
	else if ( bIsPostFireAni )
	{
		if ( bResetAni )
		{
			PlayAnimation( dwPreFireAni, true, 1.0f, false );
			return true;
		}
	}

	DoSpecialEndFire();
	return false;
}

// RETAIL: 0x1001b760
void CClientWeaponPrankster::Fire( bool bFire )
{
	CTronClientWeapon::Fire( bFire );
}

// RETAIL: 0x1001b770
void CClientWeaponPrankster::HandleFireKeyDownTransition()
{
	if ( kPrankster_Idle == m_nState )
	{
		SetState( kPrankster_KeyDown );
	}
}

// RETAIL: 0x1001b790
void CClientWeaponPrankster::HandleFireKeyUpTransition()
{
	if ( kPrankster_KeyDown == m_nState )
	{
		// RETAIL: nothing here
	}
	else if ( kPrankster_Out == m_nState )
	{
		SendRelease();
		SetState( kPrankster_Sent );
	}
}

// RETAIL: 0x1001b7c0
bool CClientWeaponPrankster::HandleMessageProjectile( uint8 messageID, ILTMessage_Read *pMsg )
{
	switch ( pMsg->Readuint8() )
	{
		case MPROJ_PRANKSTER_DONE:
			SetState( kPrankster_Idle );
			SetVisible( true );
			// fall through

		case MPROJ_PRANKSTER_ACK:
			return true;
	}

	return false;
}

// RETAIL: 0x1001b800
void CClientWeaponPrankster::SetState( int nState )
{
	m_nState = nState;
}

// RETAIL: 0x1001b810
bool CClientWeaponPrankster::SendRelease()
{
	if ( kPrankster_Out == m_nState )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_OBJECT_MESSAGE );
		cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
		cMsg.Writeuint32( MID_PROJECTILE );
		cMsg.Writeuint8( MPROJ_PRANKSTER_RELEASE );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		return true;
	}

	return false;
}

// RETAIL: 0x1001b930.  CTronClientWeapon's, and not while the bit is out.
bool CClientWeaponPrankster::TronUnknownVirtual18C()
{
	if ( kPrankster_Idle != m_nState )
	{
		return false;
	}

	if ( TronUnknownVirtual184() && TronUnknownVirtual164() )
	{
		return false;
	}

	if ( W_FIRING == GetState() ||
	     W_AUTO_SWITCH == GetState() ||
	     W_DESELECT == GetState() ||
	     W_RELOADING == GetState() ||
	     W_SWAT_DEFENSE == GetState() )
	{
		return false;
	}

	return true;
}
