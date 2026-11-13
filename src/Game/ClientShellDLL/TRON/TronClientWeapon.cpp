// ----------------------------------------------------------------------- //
//
// MODULE  : TronClientWeapon.cpp
//
// PURPOSE : Tron's common base of its client weapons (retail only, written
//           from CShell.dll 0x10097060..; see TronClientWeapon.h).
//
//           Only what the derived weapons need is here so far: the
//           constructor, destructor, OnMessage, Update, Load/Save and the
//           small helpers.  The larger new virtuals are stand-ins marked
//           "not yet written" (this TU is the 0x10090000 band's).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronClientWeapon.h"
#include "MsgIDs.h"
#include "SubroutineMgr.h"
#include "TRONPlayerStats.h"
#include "RatingMgr.h"
#include "GameClientShell.h"
#include "ScreenSubroutines.h"	// RETAIL: its statics' initializers (0x10096ff0..)

// RETAIL: 0x10097090
CTronClientWeapon::CTronClientWeapon()
	: CClientWeapon(),
	  m_fTronUnknown204( 0.0f )
{
	TronUnknown97F40();
}

// RETAIL: 0x10097060
CTronClientWeapon::~CTronClientWeapon()
{
}

// RETAIL: 0x100970e0
bool CTronClientWeapon::OnMessage( uint8 messageID, ILTMessage_Read *pMsg )
{
	if ( MID_PROJECTILE == messageID )
	{
		if ( HandleMessageProjectile( messageID, pMsg ) )
		{
			return true;
		}
	}

	return false;
}

// RETAIL: 0x10097110 (three arguments there: see the header)
WeaponState CTronClientWeapon::Update( bool bFire, FireType eFireType )
{
	bool bStopped = false;

	if ( TronUnknownVirtual18C() )
	{
		bFire = false;
		eFireType = FT_NORMAL_FIRE;
		bStopped = true;
	}

	TronUnknownVirtual180();

	if ( bStopped )
	{
		return W_FIRING_NOAMMO;
	}

	return CClientWeapon::Update( bFire, eFireType );
}

// RETAIL: 0x100971e0
void CTronClientWeapon::Select()
{
	TronUnknown98B20();
	CClientWeapon::Select();
}

// RETAIL: 0x10097160
void CTronClientWeapon::Load( ILTMessage_Read *pMsg )
{
	if ( !pMsg )
	{
		return;
	}

	TronUnknown97F40();
	TronUnknownVirtual178( pMsg );
	m_fTronUnknown204 = pMsg->Readfloat();

	CClientWeapon::Load( pMsg );
}

// RETAIL: 0x100971a0.  Slot 68 (ours TronUnknownVirtual110, an int there)
// takes the message.
void CTronClientWeapon::Save( ILTMessage_Write *pMsg )
{
	if ( !pMsg )
	{
		return;
	}

	TronUnknownVirtual110( (int)pMsg );
	pMsg->Writefloat( m_fTronUnknown204 );

	CClientWeapon::Save( pMsg );
}

// RETAIL: 0x10097200: the base energy use plus what the utility
// subroutines add; while the shot is out (W_BEGIN_FIRING) those of the
// shot's data (Load's), else the player's current ones
float CTronClientWeapon::GetEnergyUse()
{
	float fBase = GetBaseEnergyUse();
	float fEnergy = fBase;

	if ( W_BEGIN_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[8][0] )
		{
			fEnergy = ( m_fTronMegaHurtzEnergyUse + 1.0f ) * fBase;
		}

		if ( m_abTronUnknown13C[9][0] )
		{
			fEnergy += fBase * m_fTronCorrodeEnergyUse;
		}

		if ( m_abTronUnknown13C[10][0] && m_pWeapon->bIsPlayerPrimitive )
		{
			fEnergy += fBase * m_fTronPrimitiveChargeEnergyUse;
		}

		return fEnergy;
	}

	if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Megahurtz" ) )
	{
		MegaHurtzData data = g_pSubroutineDataMgr->GetMegaHurtzData( GetSubroutineDataKey( "Megahurtz" ) );
		fEnergy = ( data.fEnergyUseMultiplier + 1.0f ) * fBase;
	}

	if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Corrode" ) )
	{
		CorrodeData data = g_pSubroutineDataMgr->GetCorrodeData( GetSubroutineDataKey( "Corrode" ) );
		fEnergy += data.fEnergyUseMultiplier * fBase;
	}

	if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "PrimitiveCharge" ) && m_pWeapon->bIsPlayerPrimitive )
	{
		PrimitiveChargeData data = g_pSubroutineDataMgr->GetPrimitiveChargeData( GetSubroutineDataKey( "PrimitiveCharge" ) );
		fEnergy += data.fEnergyUseMultiplier * fBase;
	}

	return fEnergy;
}

// RETAIL: 0x100976c0
bool CTronClientWeapon::TronUnknownVirtual170()
{
	TronUnknown98B20();
	return PlayDeselectAnimation();
}

// RETAIL: 0x100978f0 (296 bytes) not yet written
bool CTronClientWeapon::HandleMessageProjectile( uint8 messageID, ILTMessage_Read *pMsg )
{
	return false;
}

// RETAIL: 0x10097c30 (358 bytes) not yet written
void CTronClientWeapon::TronUnknownVirtual178( ILTMessage_Read *pMsg )
{
}

// RETAIL: 0x10098b30: a continuous drain of m_fTronUnknown204 energy per
// second, sent to the server (CTronPlayerStats::UseEnergy)
void CTronClientWeapon::TronUnknownVirtual180()
{
	if ( m_fTronUnknown204 == 0.0f )
	{
		return;
	}

	float fEnergy = g_pGameClientShell->GetFrameTime() * m_fTronUnknown204;

	if ( g_pTronPlayerStats->GetEnergy() - fEnergy < 0.0f )
	{
		g_pTronPlayerStats->UseEnergy( fEnergy );
		return;
	}

	g_pTronPlayerStats->UseEnergy( fEnergy );
}

// RETAIL: 0x10098be0: whether the weapon's subroutines are the player's
bool CTronClientWeapon::TronUnknownVirtual184()
{
	return g_pTronPlayerStats->IsWeaponCompiled( GetWeaponId() ) ? true : false;
}

// RETAIL: 0x10098d10
bool CTronClientWeapon::TronUnknownVirtual18C()
{
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

// RETAIL: 0x10098c00: the player's subroutine's version and state, and
// the game's difficulty
SubroutineDataKey CTronClientWeapon::GetSubroutineDataKey( const char *szName )
{
	PlayerSubroutine *pSub = g_pSubroutineMgr->GetPlayerSubroutine( szName, LTNULL );

	int nVersion = kSubData_Alpha;
	switch ( pSub->eVersion )
	{
		case VERSION_ALPHA: nVersion = kSubData_Alpha; break;
		case VERSION_BETA:  nVersion = kSubData_Beta;  break;
		case VERSION_GOLD:  nVersion = kSubData_Gold;  break;
	}

	int nState = kSubData_Okay;
	switch ( pSub->eState )
	{
		case SUBSTATE_OKAY:    nState = kSubData_Okay;    break;
		case SUBSTATE_CORRUPT: nState = kSubData_Corrupt; break;
	}

	return SubroutineDataKey( nVersion, nState,
		g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy );
}

// RETAIL: 0x10098c90: the ammo's energy use (Tron's AMMO +0x18, +0x1c,
// +0x20: min, max, surge) by the accuracy rating
float CTronClientWeapon::GetBaseEnergyUse()
{
	return InterpolateRating<float>( Rating_Accuracy,
		m_pAmmo->fEnergyRequiredMin, m_pAmmo->fEnergyRequiredMax, m_pAmmo->fEnergyRequiredSurge );
}

// RETAIL: 0x10098b20
void CTronClientWeapon::TronUnknown98B20()
{
	m_fTronUnknown204 = 0.0f;
}

// RETAIL: 0x10097f40 clears eleven pairs of bools at CClientWeapon+0x13c
// (not yet at retail's offset in ours)
void CTronClientWeapon::TronUnknown97F40()
{
	for ( int i = 0; i < 11; ++i )
	{
		m_abTronUnknown13C[i][0] = false;
		m_abTronUnknown13C[i][1] = false;
	}
}
