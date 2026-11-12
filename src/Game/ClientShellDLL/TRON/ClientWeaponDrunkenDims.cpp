// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponDrunkenDims.cpp
//
// PURPOSE : Tron's client-side drunken dims weapon (retail only, written from
//           CShell.dll 0x1001a430..0x1001a5ec; see ClientWeaponDrunkenDims.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponDrunkenDims.h"
#include "SubroutineMgr.h"

// RETAIL: 0x1001a470
CClientWeaponDrunkenDims::CClientWeaponDrunkenDims()
	: CTronClientWeapon()
{
}

// RETAIL: 0x1001a430
CClientWeaponDrunkenDims::~CClientWeaponDrunkenDims()
{
}

// RETAIL: 0x1001a490
float CClientWeaponDrunkenDims::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_BEGIN_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[5][0] )
		{
			return fBase * m_fTronUnknown164 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Drunken_dims" ) )
	{
		DrunkenDimsData data = g_pSubroutineDataMgr->GetDrunkenDimsData( GetSubroutineDataKey( "Drunken_dims" ) );
		return data.fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x1001a590.  The number of vectors per round.
void CClientWeaponDrunkenDims::AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg )
{
	if ( m_abTronUnknown13C[5][0] )
	{
		pMsg->Writeuint8( m_nTronUnknown158 );
		return;
	}

	pMsg->Writeuint8( m_pWeapon ? m_pWeapon->nVectorsPerRound : 1 );
}
