// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileDrunkenDims.cpp
//
// PURPOSE : A corruption projectile - implementation
//
// RETAIL: Tron 2.0's, written from retail Object.lto
// (ProjectileDrunkenDims.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileDrunkenDims.h"

BEGIN_CLASS(CProjectileDrunkenDims)
END_CLASS_DEFAULT_FLAGS(CProjectileDrunkenDims, CCorrupticDims, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileDrunkenDims::~CProjectileDrunkenDims
//
//	PURPOSE:	Destructor (RETAIL: 0x1003d8a0)
//
// ----------------------------------------------------------------------- //

CProjectileDrunkenDims::~CProjectileDrunkenDims()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileDrunkenDims::CProjectileDrunkenDims
//
//	PURPOSE:	Constructor (RETAIL: 0x1003d8e0)
//
// ----------------------------------------------------------------------- //

CProjectileDrunkenDims::CProjectileDrunkenDims()
	: CCorrupticDims()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileDrunkenDims::Setup
//
//	PURPOSE:	Set up; scale the randomness (RETAIL: 0x1003d900)
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileDrunkenDims::Setup(CWeapon const* pWeapon, WeaponFireInfo const& info)
{
	LTBOOL bRet = CCorrupticDims::Setup(pWeapon, info);

	if (m_aModifiers[5].bEnabled)
	{
		m_fRandomness *= m_ModifierData.m4.fScale;
		m_fHorizontalFactor *= m_ModifierData.m4.fScale;
		m_fVerticalFactor *= m_ModifierData.m4.fScale;
	}

	return bRet;
}
