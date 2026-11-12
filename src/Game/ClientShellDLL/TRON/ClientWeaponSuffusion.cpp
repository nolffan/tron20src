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
bool CClientWeaponSuffusion::TronUnknownVirtualE0( int nArg1, int nArg2 )
{
	if ( nArg2 )
	{
		return CTronClientWeapon::TronUnknownVirtualE0( nArg1, nArg2 );
	}

	if ( m_nExtraFireAni == (HMODELANIM)nArg1 )
	{
		return true;
	}

	return CTronClientWeapon::TronUnknownVirtualE0( nArg1, 0 );
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
