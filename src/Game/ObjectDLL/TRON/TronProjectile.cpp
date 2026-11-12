// ----------------------------------------------------------------------- //
//
// MODULE  : TronProjectile.cpp
//
// PURPOSE : Tron's projectile base - implementation
//
// RETAIL: Tron 2.0's, written from retail Object.lto (TronProjectile.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronProjectile.h"
#include "WeaponFireInfo.h"
#include "FXButeMgr.h"
#include "TronPlayerObj.h"
#include "Character.h"

BEGIN_CLASS(CTronProjectile)
END_CLASS_DEFAULT_FLAGS(CTronProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronProjectile::CTronProjectile
//
//	PURPOSE:	Constructor (RETAIL: 0x10068b10)
//
// ----------------------------------------------------------------------- //

CTronProjectile::CTronProjectile()
	: CProjectile(),
	  m_fEnergyDrain( 0.0f )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronProjectile::Setup
//
//	PURPOSE:	Set up the projectile; a player's projectile drains
//				energy at a rate set by his rating (RETAIL: 0x10068b40)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronProjectile::Setup(CWeapon const* pWeapon, WeaponFireInfo const& info)
{
	LTBOOL bRet = CProjectile::Setup(pWeapon, info);

	if (m_pAmmoData && m_pAmmoData->pProjectileFX && IsPlayer(info.hFiredFrom))
	{
		CTronPlayerObj *pPlayer = dynamic_cast<CTronPlayerObj*>(g_pLTServer->HandleToObject(info.hFiredFrom));

		// RETAIL: rating 2 (Ratings.h calls it Rating_Accuracy)
		if (pPlayer->IsSurgePerformanceRating(Rating_Accuracy))
		{
			m_fEnergyDrain = m_pAmmoData->pProjectileFX->fEnergyDrainSurge;
			return bRet;
		}

		PROJECTILEFX *pFX = m_pAmmoData->pProjectileFX;
		m_fEnergyDrain = pFX->fEnergyDrainMin + (pFX->fEnergyDrainMax - pFX->fEnergyDrainMin) *
			(LTFLOAT)pPlayer->GetPerformanceRating(Rating_Accuracy);
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages (RETAIL: 0x10068c20)
//
// ----------------------------------------------------------------------- //

uint32 CTronProjectile::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronProjectile::Save
//
//	PURPOSE:	Save the object (RETAIL: 0x10068cb0)
//
// ----------------------------------------------------------------------- //

void CTronProjectile::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fEnergyDrain);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronProjectile::Load
//
//	PURPOSE:	Load the object (RETAIL: 0x10068cd0)
//
// ----------------------------------------------------------------------- //

void CTronProjectile::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_FLOAT(m_fEnergyDrain);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronProjectile::UpdateEnergyDrain
//
//	PURPOSE:	Drain the firing character's energy; when a player runs
//				out the projectile goes (RETAIL: 0x10068d00)
//
// ----------------------------------------------------------------------- //

void CTronProjectile::UpdateEnergyDrain()
{
	if (IsPlayer(m_hFiredFrom))
	{
		CTronPlayerObj *pPlayer = dynamic_cast<CTronPlayerObj*>(g_pLTServer->HandleToObject(m_hFiredFrom));
		if (!pPlayer || pPlayer->InGodMode()) return;
	}

	if (MATH_EPSILON >= m_fEnergyDrain) return;

	CTronPlayerObj *pPlayer = dynamic_cast<CTronPlayerObj*>(g_pLTServer->HandleToObject(m_hFiredFrom));
	if (pPlayer && pPlayer->InGodMode()) return;

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hFiredFrom));
	if (!pChar) return;

	CDestructible *pDest = pChar->GetDestructible();
	if (!pDest) return;

	LTFLOAT fEnergy = pDest->GetEnergy();
	LTFLOAT fMinEnergy = 0.0f;
	pDest->GetMaxEnergy();

	if (pPlayer)
	{
		fMinEnergy = pPlayer->TronUnknownVirtual135();
	}

	LTFLOAT fNewEnergy = fEnergy - (g_pLTServer->GetTime() - m_xTronUnknownB0) * m_fEnergyDrain;
	if (fNewEnergy < fMinEnergy)
	{
		fNewEnergy = LTMIN(fEnergy, fMinEnergy);
		HandleOutOfEnergy();
	}

	pDest->SetEnergy(fNewEnergy);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronProjectile::HandleOutOfEnergy
//
//	PURPOSE:	The owner is out of energy (RETAIL: 0x10068e90)
//
// ----------------------------------------------------------------------- //

void CTronProjectile::HandleOutOfEnergy()
{
	Detonate(LTNULL, false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronProjectile::Update
//
//	PURPOSE:	MID_UPDATE (RETAIL: 0x10068ea0)
//
// ----------------------------------------------------------------------- //

void CTronProjectile::Update()
{
	UpdateEnergyDrain();
}
