// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponEnergyClaw.cpp
//
// PURPOSE : Tron's client-side energy claw (retail only, written from
//           CShell.dll 0x1001a5f0..0x1001ac93; see ClientWeaponEnergyClaw.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponEnergyClaw.h"
#include "SubroutineMgr.h"

// RETAIL: 0x1001a5f0
CClientWeaponEnergyClaw::CClientWeaponEnergyClaw()
	: CClientWeaponContinuousFireBase(),
	  m_bInRange( false )
{
}

// RETAIL: 0x1001a640
CClientWeaponEnergyClaw::~CClientWeaponEnergyClaw()
{
}

// RETAIL: 0x1001a650.  It skips CClientWeaponContinuousFireBase's.
WeaponState CClientWeaponEnergyClaw::Update( bool bFire, FireType eFireType )
{
	UpdateInRange();
	return CTronClientWeapon::Update( bFire, eFireType );
}

// RETAIL: 0x1001a680
float CClientWeaponEnergyClaw::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_BEGIN_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[6][0] )
		{
			return fBase * m_fTronUnknown164 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Energy_claw" ) )
	{
		EnergyClawData data = g_pSubroutineDataMgr->GetEnergyClawData( GetSubroutineDataKey( "Energy_claw" ) );
		return data.fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x1001a780
void CClientWeaponEnergyClaw::TronUnknownVirtual180()
{
	if ( !m_bInRange || !m_bFiring )
	{
		CTronClientWeapon::TronUnknownVirtual180();
	}
}

// RETAIL: 0x1001a7a0 not yet written
void CClientWeaponEnergyClaw::UpdateInRange()
{
}
