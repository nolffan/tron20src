// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponSuffusion.cpp
//
// PURPOSE : Tron's client-side suffusion polymer weapon (retail only,
//           written from CShell.dll 0x1001bc80..0x1001c72b; see
//           ClientWeaponSuffusion.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponSuffusion.h"
#include "SubroutineMgr.h"
#include "TRONPlayerStats.h"
#include "GameClientShell.h"
#include "ScreenSubroutines.h"	// RETAIL: its statics' initializers (0x1001bc80..)

namespace
{
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );

	// RETAIL: 0x101e6980 (the name is ours; ClientWeapon.cpp's style)
	char *ns_szExtraFireAnimationName = "PostFullChargeFire";
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

// RETAIL: 0x1001bdf0.  The energy a second of the pre-fire (charge)
// animation uses.
void CClientWeaponSuffusion::InitAnimations( bool bAllowSelectOverride )
{
	m_nExtraFireAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szExtraFireAnimationName );

	CTronClientWeapon::InitAnimations( bAllowSelectOverride );

	uint32 nLength = 0;
	g_pModelLT->GetAnimLength( m_hObject, m_nPreFireAni, nLength );
	if ( nLength > 0 )
	{
		m_fTronUnknown214 = GetEnergyUse() / ( nLength * 0.001f );
	}
}

// RETAIL: 0x1001be80.  The pre-fire animation charges the shot
// (m_fTronUnknown20C: the part of it played, 1 when it is done) while it
// uses energy; the extra fire animation follows a full charge.  PORT:
// before the animations it starts first retail calls CTronClientWeapon's
// 0x10097f20 (a helper of four more Tron helpers), and it hands
// m_fTronUnknown214 to CTronClientWeapon's 0x10098b10 (it sets +0x204) when
// the charge starts: neither is in our build.
bool CClientWeaponSuffusion::PlayFireAnimation( bool bResetAni )
{
	uint32 dwAni			= g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState			= g_pLTClient->GetModelPlaybackState( m_hObject );

	uint32 dwPreFireAni		= GetPreFireAni();
	uint32 dwFireAni		= GetFireAni( m_eLastFireType );
	uint32 dwPostFireAni	= GetPostFireAni();
	uint32 dwExtraFireAni	= GetExtraFireAni();

	bool bHasPreFireAni		= !!( INVALID_ANI != dwPreFireAni );
	bool bHasFireAni		= !!( INVALID_ANI != dwFireAni );
	bool bHasPostFireAni	= !!( INVALID_ANI != dwPostFireAni );
	bool bHasExtraFireAni	= !!( INVALID_ANI != dwExtraFireAni );

	bool bIsFireAni			= IsFireAni( dwAni );
	bool bIsMainFireAni		= IsFireAni( dwAni, true );
	bool bIsPreFireAni		= IsPreFireAni( dwAni );
	bool bIsPostFireAni		= IsPostFireAni( dwAni );
	bool bIsExtraFireAni	= IsExtraFireAni( dwAni );

	bool bIsCurAniDone		= !!( dwState & MS_PLAYDONE );

	if ( !bIsFireAni && bResetAni )
	{
		if ( bHasPreFireAni )
		{
			m_fTronUnknown20C = 0.0f;
			m_pTimer->Start();
			PlayAnimation( dwPreFireAni, true, 1.0f, false );
			g_pTronPlayerStats->UseEnergy( GetEnergyUse() );
			// PORT: 0x10098b10( m_fTronUnknown214 ) (see above)
			return true;
		}

		if ( bHasFireAni )
		{
			m_fTronUnknown20C = 0.0f;
			m_pTimer->Stop();
			TronUnknown98B20();
			PlayAnimation( dwFireAni, true, 1.0f, true );
			return true;
		}

		if ( bHasPostFireAni )
		{
			m_fTronUnknown20C = 0.0f;
			m_pTimer->Stop();
			TronUnknown98B20();
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
			return true;
		}

		return false;
	}

	if ( bIsFireAni )
	{
		if ( bIsPreFireAni )
		{
			if ( bIsCurAniDone )
			{
				// Fully charged
				if ( bHasFireAni )
				{
					m_fTronUnknown20C = 1.0f;
					m_pTimer->Stop();
					TronUnknown98B20();
					PlayAnimation( dwFireAni, true, 1.0f, true );
					return true;
				}

				if ( bHasPostFireAni )
				{
					uint32 nLength;
					g_pModelLT->GetCurAnimLength( m_hObject, MAIN_TRACKER, nLength );
					m_fTronUnknown20C = m_pTimer->GetElapseTime() / ( nLength * 0.001f );
					m_pTimer->Stop();
					TronUnknown98B20();
					PlayAnimation( dwPostFireAni, true, 1.0f, false );
					return true;
				}

				return true;
			}

			if ( !bResetAni )
			{
				// Let go while charging
				if ( bHasPostFireAni )
				{
					uint32 nLength;
					g_pModelLT->GetCurAnimLength( m_hObject, MAIN_TRACKER, nLength );
					m_fTronUnknown20C = m_pTimer->GetElapseTime() / ( nLength * 0.001f );
					m_pTimer->Stop();
					TronUnknown98B20();
					PlayAnimation( dwPostFireAni, true, 1.0f, false );
					return true;
				}

				return true;
			}

			// Out of energy while charging
			float fEnergyUse = g_pGameClientShell->GetFrameTime() * m_fTronUnknown214;
			if ( g_pTronPlayerStats->GetEnergy() - fEnergyUse < 0.0f )
			{
				m_pTimer->Stop();
				TronUnknown98B20();
				PlayAnimation( dwPostFireAni, true, 1.0f, false );
				return true;
			}

			return true;
		}

		if ( bIsMainFireAni )
		{
			if ( bResetAni )
			{
				return true;
			}

			if ( bHasExtraFireAni )
			{
				PlayAnimation( dwExtraFireAni, true, 1.0f, false );
				return true;
			}

			if ( bHasPostFireAni )
			{
				PlayAnimation( dwPostFireAni, true, 1.0f, false );
				return true;
			}

			return true;
		}

		if ( ( bIsPostFireAni || bIsExtraFireAni ) && !bIsCurAniDone )
		{
			return true;
		}
	}

	// If we got here, it means we're done firing...

	DoSpecialEndFire();

	if ( bResetAni )
	{
		if ( bHasPreFireAni )
		{
			m_fTronUnknown20C = 0.0f;
			m_pTimer->Start();
			// PORT: 0x10098b10( m_fTronUnknown214 ) (see above)
			PlayAnimation( dwPreFireAni, true, 1.0f, false );
			g_pTronPlayerStats->UseEnergy( GetEnergyUse() );
			return true;
		}

		if ( bHasFireAni )
		{
			m_fTronUnknown20C = 0.0f;
			m_pTimer->Stop();
			TronUnknown98B20();
			PlayAnimation( dwFireAni, true, 1.0f, true );
			return true;
		}

		if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, 1.0f, false );
			return true;
		}
	}

	return false;
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
bool CClientWeaponSuffusion::IsExtraFireAni( uint32 dwAni ) const
{
	if ( INVALID_ANI == dwAni )
	{
		return false;
	}

	return ( dwAni == m_nExtraFireAni ? true : false );
}

// RETAIL: 0x10019210 (folded with the disc weapons' getter of their
// +0x208)
uint32 CClientWeaponSuffusion::GetExtraFireAni() const
{
	return m_nExtraFireAni;
}

// RETAIL: 0x1001c430.  The charge, and the vectors of the shot scaled by
// it (the subroutine's count while subroutine 2 is active: retail reads
// its +0x154 as an int).
void CClientWeaponSuffusion::AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fTronUnknown20C );

	float fScale = m_fTronUnknown20C + 1.0f;
	int nVectors = ( m_abTronUnknown13C[2][0] ? *(int*)&m_fTronUnknown154 : m_pWeapon->nVectorsPerRound );
	pMsg->Writeint8( (int8)( nVectors * fScale ) );
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
