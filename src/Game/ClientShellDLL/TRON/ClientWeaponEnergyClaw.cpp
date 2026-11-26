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
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "CharacterFX.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;
bool CheckVectorNodeIntersect(ModelSkeleton eModelSkeleton, HOBJECT hObject, const LTVector &vStartPos, const LTVector &vEndPos);

// RETAIL: 0x101e6318. RETAIL_BUGS.md: the data the claw looks up is the
// subroutine "EnergyClaw"'s, not "Energy_claw"'s
#ifdef TRON20SRC_MODERN
// MODERN.md GAME-121: the subroutine's name, as the claw's energy use has it
const char* const CClientWeaponEnergyClaw::s_szClawName = "Energy_claw";
#else
const char* const CClientWeaponEnergyClaw::s_szClawName = "EnergyClaw";
#endif

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
WeaponState CClientWeaponEnergyClaw::Update( bool bFire, bool bAltFire, FireType eFireType )
{
	UpdateTarget();
	return CTronClientWeapon::Update( bFire, bAltFire, eFireType );
}

// RETAIL: 0x1001a680
float CClientWeaponEnergyClaw::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_FIRING == GetState() )
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


// RETAIL: what the claw's intersect filter is given (as
// CW_VOFF_Params in ClientWeapon.cpp)
struct ClawFilterParams
{
	CClientWeaponEnergyClaw*	m_pWeapon;
	LTVector					m_vFirePos;
	LTVector					m_vEndPos;
};

static bool ClawFilterFn( HOBJECT hTest, void *pUserData );

// RETAIL: 0x1001a7a0: a target in the claw's range in front of the camera
// shows the in-range FX
void CClientWeaponEnergyClaw::UpdateTarget()
{
	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	if ( !hCamera )
	{
		HideInRangeFX();
		return;
	}

	float fRange = GetClawRange();

	LTVector vPos;
	g_pLTClient->GetObjectPos( hCamera, &vPos );

	LTRotation rRot;
	g_pLTClient->GetObjectRotation( hCamera, &rRot );

	LTVector vEnd = vPos + rRot.Forward() * fRange;

	ClientIntersectQuery iQuery;
	ClientIntersectInfo iInfo;

	ClawFilterParams params;
	params.m_pWeapon = this;
	params.m_vFirePos = vPos;
	params.m_vEndPos = vEnd;

	iQuery.m_From = vPos;
	iQuery.m_To = vEnd;
	iQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iQuery.m_FilterFn = ClawFilterFn;
	iQuery.m_pUserData = &params;

	if ( g_pLTClient->IntersectSegment( &iQuery, &iInfo ) )
	{
		ShowInRangeFX();
		m_bInRange = true;
	}
	else
	{
		HideInRangeFX();
		m_bInRange = false;
	}
}

// RETAIL: 0x1001aa70
EnergyClawData CClientWeaponEnergyClaw::GetClawData()
{
	int nVersion = kSubData_Alpha;
	int nState = kSubData_Okay;

	if ( g_pSubroutineMgr->IsUtilitySubroutineActive( (char*)s_szClawName ) )
	{
		switch ( g_pSubroutineMgr->GetActiveSubroutineVersion( (char*)s_szClawName ) )
		{
			case VERSION_ALPHA: nVersion = kSubData_Alpha; break;
			case VERSION_BETA:  nVersion = kSubData_Beta;  break;
			case VERSION_GOLD:  nVersion = kSubData_Gold;  break;
		}

		if ( g_pSubroutineMgr->IsSubroutineCorrupt( s_szClawName ) )
		{
			nState = kSubData_Corrupt;
		}
	}

	return g_pSubroutineDataMgr->GetEnergyClawData( SubroutineDataKey( nVersion, nState,
		g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy ) );
}

// RETAIL: 0x1001ab60
float CClientWeaponEnergyClaw::GetClawRange()
{
	EnergyClawData data = GetClawData();
	return data.fRange;
}

// RETAIL: 0x1001ab90
void CClientWeaponEnergyClaw::ShowInRangeFX()
{
	EnergyClawData data = GetClawData();
	ShowPVAttachClientFX( data.nInRangePVAttachFX );
}

// RETAIL: 0x1001abd0
void CClientWeaponEnergyClaw::HideInRangeFX()
{
	EnergyClawData data = GetClawData();
	HidePVAttachClientFX( data.nInRangePVAttachFX );
}

// RETAIL: 0x1001ac10: characters' hit boxes only, not the player's
static bool ClawFilterFn( HOBJECT hTest, void *pUserData )
{
	// Don't hit the local player.
	if ( hTest == g_pPlayerMgr->GetMoveMgr()->GetObject() )
		return false;

	// Don't hit the server player.
	HOBJECT hServerPlayer = g_pLTClient->GetClientObject();
	if ( hTest == hServerPlayer )
		return false;

	uint32 nObjUserFlags;
	g_pCommonLT->GetObjectFlags( hTest, OFT_User, nObjUserFlags );
	if ( (nObjUserFlags & USRFLG_HITBOX) != 0 )
	{
		ClawFilterParams *pParams = reinterpret_cast<ClawFilterParams *>(pUserData);

		CCharacterFX *pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox( hTest );
		if ( pCharacter )
		{
			HOBJECT hCharacterObj = pCharacter->GetServerObj();
			if ( hCharacterObj == hServerPlayer )
				return false;

			return CheckVectorNodeIntersect( pCharacter->GetModelSkeleton(), pCharacter->GetServerObj(), pParams->m_vFirePos, pParams->m_vEndPos );
		}

		return true;
	}

	return false;
}
