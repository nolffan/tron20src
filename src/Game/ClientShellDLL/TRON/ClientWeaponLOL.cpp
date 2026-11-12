// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponLOL.cpp
//
// PURPOSE : Tron's client-side LOL weapon (retail only, written from
//           CShell.dll 0x1001acc0..0x1001b09c; see ClientWeaponLOL.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponLOL.h"
#include "SubroutineMgr.h"

// RETAIL: 0x1001ad00
CClientWeaponLOL::CClientWeaponLOL()
	: CTronClientWeapon(),
	  m_fFireAnimationRate( 1.0f )
{
}

// RETAIL: 0x1001acc0
CClientWeaponLOL::~CClientWeaponLOL()
{
}

// RETAIL: 0x1001ad20
void CClientWeaponLOL::Load( ILTMessage_Read *pMsg )
{
	m_fFireAnimationRate = pMsg->Readfloat();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x1001ad50
void CClientWeaponLOL::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fFireAnimationRate );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x1001ad80
float CClientWeaponLOL::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_BEGIN_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[3][0] )
		{
			return fBase * m_fTronUnknown15C + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "LOL" ) )
	{
		return g_pSubroutineDataMgr->GetLOLData( GetSubroutineDataKey( "LOL" ) ).fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}
