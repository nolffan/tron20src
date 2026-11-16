// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponSuffusion.cpp
//
// PURPOSE : Tron's client-side suffusion polymer weapon (retail only,
//           written from CShell.dll 0x1001bc80..0x1001c72b; see
//           ClientWeaponSuffusion.h).
//
//           Not yet written: InitAnimations (0x1001bdf0), PlayFireAnimation
//           (0x1001be80), AddExtraFireMessageInfo (0x1001c430; it reads
//           CClientWeapon+0x154), GetEnergyUse (0x1001c5a0) and
//           TronUnknownVirtual164 (0x1001c6d0; CTronPlayerStats' float energy).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponSuffusion.h"
#include "SubroutineMgr.h"
#include "TRONPlayerStats.h"
#include "ScreenSubroutines.h"	// RETAIL: its statics' initializers (0x1001bc80..)

namespace
{
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
}

// RETAIL: 0x1001bd50
CClientWeaponSuffusion::CClientWeaponSuffusion()
	: CTronClientWeapon(),
	  m_nExtraFireAni( INVALID_ANI ),
	  m_fTronUnknown20C( 0.0f ),
	  m_pTimer( LTNULL ),
	  m_fTronUnknown214( 0.0f )
{
	m_pTimer = new CTimer;
}

// RETAIL: 0x1001bcf0
CClientWeaponSuffusion::~CClientWeaponSuffusion()
{
	if ( m_pTimer )
	{
		delete m_pTimer;
		m_pTimer = LTNULL;
	}
}

// RETAIL: 0x1001c3d0
bool CClientWeaponSuffusion::IsFireAni( uint32 dwAni, bool bCheckNormalOnly ) const
{
	if ( bCheckNormalOnly )
	{
		return CTronClientWeapon::IsFireAni( dwAni, bCheckNormalOnly );
	}

	if ( m_nExtraFireAni == (HMODELANIM)dwAni )
	{
		return true;
	}

	return CTronClientWeapon::IsFireAni( dwAni, false );
}

// RETAIL: 0x1001c410
bool CClientWeaponSuffusion::IsExtraFireAni( uint32 dwAni )
{
	if ( INVALID_ANI == dwAni )
	{
		return false;
	}

	return m_nExtraFireAni == dwAni;
}

// RETAIL: 0x1001c4a0
void CClientWeaponSuffusion::Load( ILTMessage_Read *pMsg )
{
	m_fTronUnknown20C = pMsg->Readfloat();
	m_pTimer->Add( pMsg->Readfloat() );
	m_fTronUnknown214 = pMsg->Readfloat();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x1001c520
void CClientWeaponSuffusion::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fTronUnknown20C );
	pMsg->Writefloat( m_pTimer->GetElapseTime() );
	pMsg->Writefloat( m_fTronUnknown214 );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x1001c5a0
float CClientWeaponSuffusion::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[2][0] )
		{
			return fBase * m_fTronUnknown160 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Suffusion" ) )
	{
		return g_pSubroutineDataMgr->GetSuffusionData( GetSubroutineDataKey( "Suffusion" ) ).fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x1001c6a0.  The perturb scaled by the subroutine's (retail's
// CClientWeapon+0x158).
void CClientWeaponSuffusion::SendFireMessage( bool bFire,
                                              LTFLOAT fPerturb,
                                              LTVector const &vFirePos,
                                              LTVector const &vDir,
                                              LTVector const &vImpactPos,
                                              HOBJECT hObject )
{
	CTronClientWeapon::SendFireMessage( bFire, fPerturb * m_fTronUnknown158, vFirePos, vDir, vImpactPos, hObject );
}

// RETAIL: 0x1001c6d0: while firing, any energy left keeps it going
bool CClientWeaponSuffusion::TronUnknownVirtual164()
{
	if ( W_FIRING == GetState() )
	{
		return g_pTronPlayerStats->GetEnergy() > 1.0e-11f;
	}

	float fEnergy = g_pTronPlayerStats->GetEnergy();
	return ( GetEnergyUse() <= fEnergy );
}
