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

	if ( W_BEGIN_FIRING == GetState() )
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

	if ( W_BEGIN_FIRING == GetState() ||
	     W_FIRING_NOAMMO == GetState() ||
	     W_RELOADING == GetState() ||
	     W_SWAT_DEFENSE == GetState() ||
	     W_FIRED == GetState() )
	{
		return false;
	}

	return true;
}
