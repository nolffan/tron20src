// ----------------------------------------------------------------------- //
//`
// MODULE  : ClientWeaponClusterDisc.cpp
//
// PURPOSE : Tron specific client-side cluster version of the disc
//
// CREATED : 4/12/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponClusterDisc.h"
#include "FXButeMgr.h"
#include "RatingMgr.h"
#include "SubroutineMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its statics' initializers (0x100180d0..)

// RETAIL: 0x10018190
CClientWeaponClusterDisc::CClientWeaponClusterDisc()
	: CClientWeaponDisc()
{
}



// RETAIL: 0x10018140
CClientWeaponClusterDisc::~CClientWeaponClusterDisc()
{
}



// RETAIL: 0x100181b0
float CClientWeaponClusterDisc::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[1][0] )
		{
			return fBase * m_fTronUnknown1E4 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Cluster" ) )
	{
		ClusterData data = g_pSubroutineDataMgr->GetClusterData( GetSubroutineDataKey( "Cluster" ) );
		return data.fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x1001bc00
bool CClientWeaponClusterDisc::TronUnknownVirtual18C()
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
