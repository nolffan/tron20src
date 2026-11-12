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

	if ( W_BEGIN_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[0][0] )
		{
			return fBase * m_fTronUnknown160 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Sequencer" ) )
	{
		SequencerData data = g_pSubroutineDataMgr->GetSequencerData( GetSubroutineDataKey( "Sequencer" ) );
		return data.fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x1001bc00 (CClientWeaponClusterDisc's slot 99 too)
bool CClientWeaponSequencerDisc::TronUnknownVirtual18C()
{
	if ( TronUnknownVirtual184() && TronUnknownVirtual164() && TronUnknownVirtual19C() )
	{
		return false;
	}

	if ( W_FIRING_NOAMMO == GetState() ||
	     W_RELOADING == GetState() ||
	     W_SWAT_DEFENSE == GetState() ||
	     W_FIRED == GetState() ||
	     TronUnknownVirtual1A0() )
	{
		return false;
	}

	return true;
}
