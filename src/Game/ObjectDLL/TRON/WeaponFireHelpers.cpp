// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFireHelpers.cpp
//
// PURPOSE : Tron's two continuous fire helpers that CCharacter owns
//           (CPRodFireHelper, CEnergyClawFireHelper; declared in
//           ContinuousFireHelper.h). Retail only (Object.lto
//           0x10194160..0x10194335), written from the retail code. The
//           file name and the names of the non-virtual functions are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ContinuousFireHelper.h"
#include "Character.h"
#include "WeaponMgr.h"

// RETAIL: the fire data's field 6 holds two floats this file reads (a
// range at +0, an energy rate at +8).
static inline float FireDataFloat(const ContinuousFireBlob20b &Blob, int nIndex)
{
	return ((const float*)Blob.m_aData)[nIndex];
}

// ----------------------------------------------------------------------- //

uint8 CEnergyClawFireHelper::GetMessageID()
{
	// RETAIL: 0x10194160
	return 13;
}

// ----------------------------------------------------------------------- //

CEnergyClawFireHelper::CEnergyClawFireHelper()
{
	// RETAIL: 0x10194170
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyClawFireHelper::Update()
//
//	PURPOSE:	While the claw holds targets, give energy to the firing
//				character (clamped to its maximum).
//
// ----------------------------------------------------------------------- //

void CEnergyClawFireHelper::Update()
{
	// RETAIL: 0x10194190
	int nTargets = m_lstTargets.size();
	if( nTargets > 0 )
	{
		CCharacter *pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject( m_hObject ));
		if( pChar )
		{
			float fDelta = GetEnergyRate() * g_pLTServer->GetFrameTime();
			float fEnergy = pChar->GetDestructible()->GetEnergy();
			float fMax = pChar->GetDestructible()->GetMaxEnergy();
			fEnergy += fDelta;
			if( fEnergy < 0.0f ) fEnergy = 0.0f;
			else if( fEnergy > fMax ) fEnergy = fMax;
			pChar->GetDestructible()->SetEnergy( fEnergy );
		}
	}

	CContinuousFireHelper::Update();
}

// ----------------------------------------------------------------------- //

float CEnergyClawFireHelper::GetRange()
{
	// RETAIL: 0x10194280
	if( m_FireData.m_aFlags[6].m_bSet )
		return FireDataFloat( m_FireData.m_Field6, 0 );

	if( m_pWeapon )
		return (float)m_pWeapon->nRange;

	return 0.0f;
}

// ----------------------------------------------------------------------- //

float CEnergyClawFireHelper::GetEnergyRate()
{
	// RETAIL: 0x101942a0
	float fRate = 0.0f;
	if( m_FireData.m_aFlags[6].m_bSet )
		fRate = FireDataFloat( m_FireData.m_Field6, 2 );

	return fRate + m_fTronUnknown24;
}

// ----------------------------------------------------------------------- //

void CEnergyClawFireHelper::WriteExtra(ILTMessage_Write *pMsg)
{
	// RETAIL: 0x101942c0
	float fRange = GetRange();
	pMsg->WriteBits( *(uint32*)&fRange, 32 );
}

// ----------------------------------------------------------------------- //

CPRodFireHelper::~CPRodFireHelper()
{
	// RETAIL: 0x101942e0
}

// ----------------------------------------------------------------------- //

uint8 CPRodFireHelper::GetMessageID()
{
	// RETAIL: 0x101942f0
	return 6;
}

// ----------------------------------------------------------------------- //

CPRodFireHelper::CPRodFireHelper()
{
	// RETAIL: 0x10194300
}

// ----------------------------------------------------------------------- //

float CPRodFireHelper::GetRange()
{
	// RETAIL: 0x10194320
	if( m_pWeapon )
		return (float)m_pWeapon->nRange;

	return 0.0f;
}
