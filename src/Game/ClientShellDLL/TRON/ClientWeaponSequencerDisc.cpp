// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponSequencerDisc.cpp
//
// PURPOSE : Tron's client-side sequencer disc (retail only, written from
//           CShell.dll 0x1001b9b0..0x1001bc7c; see ClientWeaponSequencerDisc.h).
//
//           Not yet written: UpdateModelState (0x1001bb00; two arguments
//           there) and TronUnknownVirtual19C (0x1001bb40; the subroutine's
//           ammo bonus).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponSequencerDisc.h"
#include "SubroutineMgr.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// RETAIL: 0x1001b9e0
CClientWeaponSequencerDisc::CClientWeaponSequencerDisc()
	: CClientWeaponDisc()
{
}

// RETAIL: 0x1001b9b0
CClientWeaponSequencerDisc::~CClientWeaponSequencerDisc()
{
}

// RETAIL: 0x1001ba00
float CClientWeaponSequencerDisc::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[0][0] )
		{
			return fBase * m_fTronUnknown160 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Sequencer" ) )
	{
		SubroutineDataKey key = GetSubroutineDataKey( "Sequencer" );
		return g_pSubroutineDataMgr->GetSequencerData( key ).fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x1001bb00: no alternate fire while the discs are full
WeaponState CClientWeaponSequencerDisc::UpdateModelState( bool bFire, bool bAltFire )
{
	if ( bAltFire && TronUnknownVirtual1A0() )
	{
		bAltFire = false;
	}

	return CClientWeaponDisc::UpdateModelState( bFire, bAltFire );
}

// RETAIL: 0x1001bb40: more discs than the subroutine's ammo override
// allows below the maximum
bool CClientWeaponSequencerDisc::TronUnknownVirtual19C()
{
	char *szName = m_pWeapon->szSubroutineNeeded;
	bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt( szName );
	SubVersion eVersion = g_pSubroutineMgr->GetActiveSubroutineVersion( szName );

	int nOverride = g_pSubroutineDataMgr->GetSequencerData( SubroutineDataKey( (uint8)eVersion, (uint8)bCorrupt,
		g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy ) ).nMaxAmmoOverride;

	int nMax = m_pAmmo->GetMaxAmount( LTNULL ) - nOverride;
	return (int)g_pPlayerStats->GetAmmoCount( m_nAmmoId ) > nMax;
}

// RETAIL: 0x1001bc00 (CClientWeaponClusterDisc's slot 99 too)
bool CClientWeaponSequencerDisc::TronUnknownVirtual18C()
{
	if ( TronUnknownVirtual184() && TronUnknownVirtual164() && TronUnknownVirtual19C() )
	{
		return false;
	}

	if ( W_AUTO_SWITCH == GetState() ||
	     W_DESELECT == GetState() ||
	     W_RELOADING == GetState() ||
	     W_SWAT_DEFENSE == GetState() ||
	     TronUnknownVirtual1A0() )
	{
		return false;
	}

	return true;
}
