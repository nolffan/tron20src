// ----------------------------------------------------------------------- //
//
// MODULE  : PranksterProjectileFX.cpp
//
// PURPOSE : The Prankster's projectile special fx - Implementation
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x101bfe30..
// 0x101bffa7). The names of the member and the setter are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PranksterProjectileFX.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPranksterProjectileFX::~CPranksterProjectileFX
//
//	PURPOSE:	Destructor (RETAIL: 0x101bfe30)
//
// ----------------------------------------------------------------------- //

CPranksterProjectileFX::~CPranksterProjectileFX()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPranksterProjectileFX::CPranksterProjectileFX
//
//	PURPOSE:	Constructor (RETAIL: 0x101bfef0)
//
// ----------------------------------------------------------------------- //

CPranksterProjectileFX::CPranksterProjectileFX()
	: m_bTronUnknown7C(LTFALSE)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPranksterProjectileFX::Term
//
//	PURPOSE:	RETAIL: Tron's does nothing (its slot 1 folds with an
//				empty function, 0x100abb80): the object stays to the
//				CSpecialFX destructor
//
// ----------------------------------------------------------------------- //

void CPranksterProjectileFX::Term()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPranksterProjectileFX::CreateObject
//
//	PURPOSE:	Create the projectile (RETAIL: 0x101bff70)
//
// ----------------------------------------------------------------------- //

LTBOOL CPranksterProjectileFX::CreateObject(ILTClient* pClientDE)
{
	LTBOOL bRet = CProjectileFX::CreateObject(pClientDE);
	if (bRet == LTTRUE)
	{
		SetTronUnknown7C(bRet);
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPranksterProjectileFX::SetTronUnknown7C
//
//	PURPOSE:	RETAIL: 0x101bffa0 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CPranksterProjectileFX::SetTronUnknown7C(LTBOOL bValue)
{
	m_bTronUnknown7C = bValue;
}
