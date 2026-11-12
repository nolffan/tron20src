// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileBallLauncher.cpp
//
// PURPOSE : A corruption projectile that takes no damage - implementation
//
// RETAIL: Tron 2.0's, written from retail Object.lto
// (ProjectileBallLauncher.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileBallLauncher.h"

BEGIN_CLASS(CProjectileBallLauncher)
END_CLASS_DEFAULT_FLAGS(CProjectileBallLauncher, CCorrupticDims, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBallLauncher::DamageFilterHook
//
//	PURPOSE:	m_damage's filter (RETAIL: 0x100354b0)
//
// ----------------------------------------------------------------------- //

bool CProjectileBallLauncher::DamageFilterHook(GameBase *pObject, DamageStruct *pDamageStruct)
{
	CProjectileBallLauncher *pLauncher = (CProjectileBallLauncher*)pObject;
	if (pLauncher)
	{
		return pLauncher->FilterDamage(pDamageStruct);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBallLauncher::FilterDamage
//
//	PURPOSE:	Take no damage (RETAIL: 0x100354d0)
//
// ----------------------------------------------------------------------- //

bool CProjectileBallLauncher::FilterDamage(DamageStruct *pDamageStruct)
{
	pDamageStruct->fDamage = 0.0f;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBallLauncher::~CProjectileBallLauncher
//
//	PURPOSE:	Destructor (RETAIL: 0x100354e0)
//
// ----------------------------------------------------------------------- //

CProjectileBallLauncher::~CProjectileBallLauncher()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBallLauncher::CProjectileBallLauncher
//
//	PURPOSE:	Constructor (RETAIL: 0x10035520)
//
// ----------------------------------------------------------------------- //

CProjectileBallLauncher::CProjectileBallLauncher()
	: CCorrupticDims()
{
	m_damage.RegisterFilterFunction(DamageFilterHook, this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBallLauncher::Setup
//
//	PURPOSE:	Set up; scale the randomness (RETAIL: 0x10035580)
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileBallLauncher::Setup(CWeapon const* pWeapon, WeaponFireInfo const& info)
{
	LTBOOL bRet = CCorrupticDims::Setup(pWeapon, info);

	if (m_bTronUnknownF9)
	{
		m_fRandomness *= m_fTronUnknown108;
		m_fHorizontalFactor *= m_fTronUnknown108;
		m_fVerticalFactor *= m_fTronUnknown108;
	}

	return bRet;
}
