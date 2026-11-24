// ----------------------------------------------------------------------- //
//
// MODULE  : TronClientWeapon.cpp
//
// PURPOSE : Tron's common base of its client weapons (retail only, written
//           from CShell.dll 0x10097060..; see TronClientWeapon.h).
//
//           Each function is written from the retail code (its RETAIL:
//           note gives the address).
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
#include "ClientFXMgr.h"
#include "CheatMgr.h"

#ifndef INVALID_ANI
#define INVALID_ANI			((HMODELANIM)-1)
#endif

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

// RETAIL: 0x10097110
WeaponState CTronClientWeapon::Update( bool bFire, bool bAltFire, FireType eFireType )
{
	bool bStopped = false;

	if ( TronUnknownVirtual18C() )
	{
		bFire = false;
		bAltFire = false;
		bStopped = true;
	}

	TronUnknownVirtual180();

	if ( bStopped )
	{
		return W_AUTO_SWITCH;
	}

	return CClientWeapon::Update( bFire, bAltFire, eFireType );
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

// RETAIL: 0x100971a0
void CTronClientWeapon::Save( ILTMessage_Write *pMsg )
{
	if ( !pMsg )
	{
		return;
	}

	TronUnknownVirtual110( pMsg );
	pMsg->Writefloat( m_fTronUnknown204 );

	CClientWeapon::Save( pMsg );
}

// RETAIL: 0x10097200: the base energy use plus what the utility
// subroutines add; while the shot is out (W_FIRING) those of the
// shot's data (Load's), else the player's current ones
float CTronClientWeapon::GetEnergyUse()
{
	float fBase = GetBaseEnergyUse();
	float fEnergy = fBase;

	if ( W_FIRING == GetState() )
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

// RETAIL: 0x10097520.  NOLF2's CClientWeapon::UpdateFiring as tests of the
// state one after another
void CTronClientWeapon::UpdateFiring()
{
	m_bCanSetLastFire = true;

	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_FIRING );
		}
	}

	if ( GetState() == W_IDLE )
	{
		SetState( W_FIRING );
	}

	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_FIRING );
		}
	}

	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			SetState( W_FIRING );
		}
	}

	if ( GetState() == W_FIRING || GetState() == W_FIRING_NOAMMO )
	{
		if ( PlayFireAnimation( true ) )
		{
			m_bCanSetLastFire = false;
		}
	}
}

// RETAIL: 0x100975e0.  NOLF2's CClientWeapon::UpdateNonFiring as tests of
// the state one after another
void CTronClientWeapon::UpdateNonFiring()
{
	m_bCanSetLastFire = true;

	if ( GetState() == W_FIRING )
	{
		if ( !PlayFireAnimation( false ) )
		{
			SetState( W_IDLE );
		}
		else
		{
			m_bCanSetLastFire = false;
		}
	}

	if ( GetState() == W_FIRING_NOAMMO )
	{
		SetState( W_IDLE );
	}

	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_IDLE );
		}
	}

	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_IDLE );
		}
	}

	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			m_bWeaponDeselected = true;
			SetState( W_IDLE );
		}
	}

	if ( GetState() == W_IDLE )
	{
		PlayIdleAnimation();
	}
}

// RETAIL: 0x100976c0
bool CTronClientWeapon::PlayDeselectAnimation()
{
	TronUnknown98B20();
	return CClientWeapon::PlayDeselectAnimation();
}

// RETAIL: 0x101e8854..0x101e887c: the subroutines a shot keeps the data of,
// by their index in m_abTronUnknown13C (the name is ours)
static const char* const s_aszShotSubroutines[11] =
{
	"Sequencer",
	"Cluster",
	"Suffusion",
	"LOL",
	"Ball_launcher",
	"Drunken_dims",
	"Energy_claw",
	"Prankster_bit",
	"MegaHurtz",
	"Corrode",
	"Primitive_charge",
};

// RETAIL: 0x100976e0: NOLF2's CClientWeapon::PlayFireAnimation; each fire
// animation that starts a shot first keeps the shot's subroutine data, and
// the energy (slot 89), not the ammo, lets the main fire animation repeat
bool CTronClientWeapon::PlayFireAnimation( bool bResetAni )
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

	if ( !bIsFireAni && bResetAni )
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

	if ( bIsFireAni )
	{
		if ( !bIsCurAniDone )
		{
			return true;
		}

		if ( bIsPreFireAni )
		{
			if ( bHasFireAni )
			{
				PlayAnimation( dwFireAni, true, fRate );
				return true;
			}
			else if ( bHasPostFireAni )
			{
				PlayAnimation( dwPostFireAni, true, fRate );
				return true;
			}
		}
		else if ( bIsMainFireAni )
		{
			if ( bResetAni && TronUnknownVirtual164() )
			{
				SaveShotSubroutines();
				PlayAnimation( dwFireAni, true, fRate );
				return true;
			}
			else if ( bHasPostFireAni )
			{
				PlayAnimation( dwPostFireAni, true, fRate );
				return true;
			}
		}
		else if( bIsPostFireAni )
		{
			// RETAIL: the cycle starts again with the pre-fire animation (if
			// there is none too), and the end of the firing follows
			if( bResetAni )
			{
				SaveShotSubroutines();
				PlayAnimation( dwPreFireAni, true, fRate );
			}
		}
	}

	DoSpecialEndFire();
	return false;
}

// RETAIL: 0x10097f10: false; retail also loads the argument into eax first
// (mov eax,[esp+4]; xor al,al), a shape not yet found
bool CTronClientWeapon::TronUnknownVirtual160( int nArg )
{
	return false;
}

// RETAIL: 0x10097f20
void CTronClientWeapon::SaveShotSubroutines()
{
	SaveShotWeaponSubroutine();
	SaveShotMegaHurtz();
	SaveShotCorrode();
	SaveShotPrimitiveCharge();
}

// RETAIL: 0x10097f60: the data of the weapon's subroutine goes to the union
// at +0x154 (the active one's data: the inactive one's is Alpha, not corrupt)
void CTronClientWeapon::SaveShotWeaponSubroutine()
{
	if ( !_stricmp( m_pWeapon->szSubroutineNeeded, s_aszShotSubroutines[0] ) )
	{
		m_abTronUnknown13C[0][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[0] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[0] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;
		if ( !g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[0] ) )
		{
			nVersion = 0;
			bCorrupt = false;
			nState = 0;
		}

		*(SequencerData*)&m_fTronUnknown154 = g_pSubroutineDataMgr->GetSequencerData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[0][1] = true;
		}
	}
	if ( !_stricmp( m_pWeapon->szSubroutineNeeded, s_aszShotSubroutines[1] ) )
	{
		m_abTronUnknown13C[1][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[1] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[1] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;
		if ( !g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[1] ) )
		{
			nVersion = 0;
			bCorrupt = false;
			nState = 0;
		}

		*(ClusterData*)&m_fTronUnknown154 = g_pSubroutineDataMgr->GetClusterData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[1][1] = true;
		}
	}
	if ( !_stricmp( m_pWeapon->szSubroutineNeeded, s_aszShotSubroutines[2] ) )
	{
		m_abTronUnknown13C[2][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[2] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[2] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;
		if ( !g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[2] ) )
		{
			nVersion = 0;
			bCorrupt = false;
			nState = 0;
		}

		*(SuffusionData*)&m_fTronUnknown154 = g_pSubroutineDataMgr->GetSuffusionData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[2][1] = true;
		}
	}
	if ( !_stricmp( m_pWeapon->szSubroutineNeeded, s_aszShotSubroutines[3] ) )
	{
		m_abTronUnknown13C[3][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[3] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[3] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;
		if ( !g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[3] ) )
		{
			nVersion = 0;
			bCorrupt = false;
			nState = 0;
		}

		*(LOLData*)&m_fTronUnknown154 = g_pSubroutineDataMgr->GetLOLData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[3][1] = true;
		}
	}
	if ( !_stricmp( m_pWeapon->szSubroutineNeeded, s_aszShotSubroutines[4] ) )
	{
		m_abTronUnknown13C[4][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[4] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[4] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;
		if ( !g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[4] ) )
		{
			nVersion = 0;
			bCorrupt = false;
			nState = 0;
		}

		*(BallLauncherData*)&m_fTronUnknown154 = g_pSubroutineDataMgr->GetBallLauncherData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[4][1] = true;
		}
	}
	if ( !_stricmp( m_pWeapon->szSubroutineNeeded, s_aszShotSubroutines[5] ) )
	{
		m_abTronUnknown13C[5][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[5] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[5] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;
		if ( !g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[5] ) )
		{
			nVersion = 0;
			bCorrupt = false;
			nState = 0;
		}

		*(DrunkenDimsData*)&m_fTronUnknown154 = g_pSubroutineDataMgr->GetDrunkenDimsData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[5][1] = true;
		}
	}
	if ( !_stricmp( m_pWeapon->szSubroutineNeeded, s_aszShotSubroutines[6] ) )
	{
		m_abTronUnknown13C[6][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[6] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[6] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;
		if ( !g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[6] ) )
		{
			nVersion = 0;
			bCorrupt = false;
			nState = 0;
		}

		*(EnergyClawData*)&m_fTronUnknown154 = g_pSubroutineDataMgr->GetEnergyClawData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[6][1] = true;
		}
	}
	if ( !_stricmp( m_pWeapon->szSubroutineNeeded, s_aszShotSubroutines[7] ) )
	{
		m_abTronUnknown13C[7][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[7] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[7] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;
		if ( !g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[7] ) )
		{
			nVersion = 0;
			bCorrupt = false;
			nState = 0;
		}

		*(PranksterBitData*)&m_fTronUnknown154 = g_pSubroutineDataMgr->GetPranksterBitData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[7][1] = true;
		}
	}
}

// RETAIL: 0x10098840
void CTronClientWeapon::SaveShotMegaHurtz()
{
	if ( g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[8] ) )
	{
		m_abTronUnknown13C[8][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[8] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[8] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;

		*(MegaHurtzData*)&m_fTronMegaHurtzDamage = g_pSubroutineDataMgr->GetMegaHurtzData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[8][1] = true;
		}
	}
}

// RETAIL: 0x10098920
void CTronClientWeapon::SaveShotCorrode()
{
	if ( g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[9] ) )
	{
		m_abTronUnknown13C[9][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[9] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[9] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;

		*(CorrodeData*)&m_fTronCorrodeDuration = g_pSubroutineDataMgr->GetCorrodeData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[9][1] = true;
		}
	}
}

// RETAIL: 0x10098a20: only for the player's primitive weapons
void CTronClientWeapon::SaveShotPrimitiveCharge()
{
	if ( !m_pWeapon->bIsPlayerPrimitive )
	{
		return;
	}

	if ( g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_aszShotSubroutines[10] ) )
	{
		m_abTronUnknown13C[10][0] = true;

		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( s_aszShotSubroutines[10] );
		int nVersion = (uint8)g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_aszShotSubroutines[10] );
		int nState = (uint8)bCorrupt;
		int nDifficulty = g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy;

		*(PrimitiveChargeData*)&m_fTronPrimitiveChargeDamage = g_pSubroutineDataMgr->GetPrimitiveChargeData( SubroutineDataKey( nVersion, nState, nDifficulty ) );

		if ( bCorrupt )
		{
			m_abTronUnknown13C[10][1] = true;
		}
	}
}

// RETAIL: 0x100978f0: a blocked shot shows the weapon's defense class
// effect on the player view
bool CTronClientWeapon::HandleMessageProjectile( uint8 messageID, ILTMessage_Read *pMsg )
{
	DEFENSECLASSDATA const *pDefense = LTNULL;
	if ( MPROJ_BLOCKED == pMsg->Readuint8() )
	{
		pDefense = g_pWeaponMgr->GetDefenseClass( m_pWeapon->nDefenseClassId );
	}

	if ( !pDefense )
	{
		return false;
	}

	CLIENTFX_LINK clientFXLink;
	CLIENTFX_CREATESTRUCT fxInit( pDefense->szDefendPVFXName, FXFLAG_REALLYCLOSE, m_hObject );
	g_pClientFXMgr->CreateClientFX( &clientFXLink, fxInit, LTTRUE );
	if ( clientFXLink.GetInstance() )
	{
		clientFXLink.GetInstance()->m_bPlayerView = LTTRUE;
	}

	return true;
}

// RETAIL: 0x10097a70: without energy (slot 89) no shot, unless the cheat
bool CTronClientWeapon::SpecialOverrideFire()
{
	if ( g_pCheatMgr && !g_pCheatMgr->IsCheatActive( CHEAT_GOD ) && !TronUnknownVirtual164() )
	{
		return false;
	}

	return CClientWeapon::SpecialOverrideFire();
}

// RETAIL: 0x10097ab0: Save writes the shot's subroutine data
void CTronClientWeapon::TronUnknownVirtual110( ILTMessage_Write *pMsg )
{
	bool (*pFlags)[2] = m_abTronUnknown13C;
	for ( int i = 0; i < 11; ++i, ++pFlags )
	{
		pMsg->Writebool( (*pFlags)[0] );
		pMsg->Writebool( (*pFlags)[1] );
	}

	if ( m_abTronUnknown13C[0][0] ) pMsg->WriteType( *(SequencerData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[1][0] ) pMsg->WriteType( *(ClusterData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[2][0] ) pMsg->WriteType( *(SuffusionData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[3][0] ) pMsg->WriteType( *(LOLData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[4][0] ) pMsg->WriteType( *(BallLauncherData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[5][0] ) pMsg->WriteType( *(DrunkenDimsData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[6][0] ) pMsg->WriteType( *(EnergyClawData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[7][0] ) pMsg->WriteType( *(PranksterBitData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[8][0] ) pMsg->WriteType( *(MegaHurtzData*)&m_fTronMegaHurtzDamage );
	if ( m_abTronUnknown13C[9][0] ) pMsg->WriteType( *(CorrodeData*)&m_fTronCorrodeDuration );
	if ( m_abTronUnknown13C[10][0] ) pMsg->WriteType( *(PrimitiveChargeData*)&m_fTronPrimitiveChargeDamage );
}

// RETAIL: 0x10097c30: Load reads the shot's subroutine data
void CTronClientWeapon::TronUnknownVirtual178( ILTMessage_Read *pMsg )
{
	bool (*pFlags)[2] = m_abTronUnknown13C;
	for ( int i = 0; i < 11; ++i, ++pFlags )
	{
		(*pFlags)[0] = pMsg->Readbool();
		(*pFlags)[1] = pMsg->Readbool();
	}

	if ( m_abTronUnknown13C[0][0] ) pMsg->ReadType( (SequencerData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[1][0] ) pMsg->ReadType( (ClusterData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[2][0] ) pMsg->ReadType( (SuffusionData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[3][0] ) pMsg->ReadType( (LOLData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[4][0] ) pMsg->ReadType( (BallLauncherData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[5][0] ) pMsg->ReadType( (DrunkenDimsData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[6][0] ) pMsg->ReadType( (EnergyClawData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[7][0] ) pMsg->ReadType( (PranksterBitData*)&m_fTronUnknown154 );
	if ( m_abTronUnknown13C[8][0] ) pMsg->ReadType( (MegaHurtzData*)&m_fTronMegaHurtzDamage );
	if ( m_abTronUnknown13C[9][0] ) pMsg->ReadType( (CorrodeData*)&m_fTronCorrodeDuration );
	if ( m_abTronUnknown13C[10][0] ) pMsg->ReadType( (PrimitiveChargeData*)&m_fTronPrimitiveChargeDamage );
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

// RETAIL: 0x10098b10
void CTronClientWeapon::SetTronUnknown204( float fTronUnknown204 )
{
	m_fTronUnknown204 = fTronUnknown204;
}

// RETAIL: 0x10098b20
void CTronClientWeapon::TronUnknown98B20()
{
	m_fTronUnknown204 = 0.0f;
}

// RETAIL: 0x10097a20
void CTronClientWeapon::Fire( bool bFire )
{
	CClientWeapon::Fire( bFire );

	if ( TronUnknownVirtual188() )
	{
		float fEnergy = GetEnergyUse();
		g_pTronPlayerStats->UseEnergy( fEnergy );
	}

	TronUnknown97F40();
}

// RETAIL: 0x10098ba0
bool CTronClientWeapon::TronUnknownVirtual164()
{
	float fEnergy = g_pTronPlayerStats->GetEnergy();
	return ( GetEnergyUse() <= fEnergy );
}

// RETAIL: 0x10097f40 clears eleven pairs of bools at CClientWeapon+0x13c
// (not yet at retail's offset in ours)
void CTronClientWeapon::TronUnknown97F40()
{
	bool (*pFlags)[2] = m_abTronUnknown13C;
	for ( int i = 0; i < 11; ++i, ++pFlags )
	{
		(*pFlags)[0] = false;
		(*pFlags)[1] = false;
	}
}
