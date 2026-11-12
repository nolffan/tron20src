// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponBallLauncher.cpp
//
// PURPOSE : Tron's client-side ball launcher (retail only, written from
//           CShell.dll 0x10017c20..0x1001807c; see ClientWeaponBallLauncher.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponBallLauncher.h"
#include "SubroutineMgr.h"

// RETAIL: 0x10017c60
CClientWeaponBallLauncher::CClientWeaponBallLauncher()
	: CTronClientWeapon(),
	  m_fFireAnimationRate( 1.0f )
{
}

// RETAIL: 0x10017c20
CClientWeaponBallLauncher::~CClientWeaponBallLauncher()
{
}

// RETAIL: 0x1001ad20
void CClientWeaponBallLauncher::Load( ILTMessage_Read *pMsg )
{
	m_fFireAnimationRate = pMsg->Readfloat();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x1001ad50
void CClientWeaponBallLauncher::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fFireAnimationRate );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x10017c80 (528 bytes) not yet written; PORT: CClientWeapon's
bool CClientWeaponBallLauncher::PlayFireAnimation( bool bResetAni )
{
	return CTronClientWeapon::PlayFireAnimation( bResetAni );
}

// RETAIL: 0x10017e90
float CClientWeaponBallLauncher::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_BEGIN_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[4][0] )
		{
			return fBase * m_fTronUnknown164 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Ball_launcher" ) )
	{
		BallLauncherData data = g_pSubroutineDataMgr->GetBallLauncherData( GetSubroutineDataKey( "Ball_launcher" ) );
		return data.fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x10018070.  The rate CClientWeapon keeps for the subroutine
// (retail's +0x158).
void CClientWeaponBallLauncher::UpdateFireAnimationRate()
{
	m_fFireAnimationRate = m_fTronUnknown158;
}
