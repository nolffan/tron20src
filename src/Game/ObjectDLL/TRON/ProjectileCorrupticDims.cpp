// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileCorrupticDims.cpp
//
// PURPOSE : The corruption projectile - implementation
//
// RETAIL: Tron 2.0's, written from retail Object.lto
// (ProjectileCorrupticDims.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileCorrupticDims.h"
#include "WeaponFireInfo.h"
#include "FXButeMgr.h"

BEGIN_CLASS(CCorrupticDims)
END_CLASS_DEFAULT_FLAGS(CCorrupticDims, CTronProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorrupticDims::~CCorrupticDims
//
//	PURPOSE:	Destructor (RETAIL: 0x100360c0, before the constructor)
//
// ----------------------------------------------------------------------- //

CCorrupticDims::~CCorrupticDims()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorrupticDims::CCorrupticDims
//
//	PURPOSE:	Constructor (RETAIL: 0x10036150)
//
// ----------------------------------------------------------------------- //

CCorrupticDims::CCorrupticDims()
	: CTronProjectile(),
	  m_fRandomness( 0.0f ),
	  m_fRandomnessRampUpTime( 0.0f ),
	  m_fHorizontalFactor( 0.0f ),
	  m_fVerticalFactor( 0.0f ),
	  m_hHomingTarget( LTNULL ),
	  m_fTurnRate( 0.0f )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorrupticDims::Setup
//
//	PURPOSE:	Set up from the ammo's class data (RETAIL: 0x100361e0)
//
// ----------------------------------------------------------------------- //

LTBOOL CCorrupticDims::Setup(CWeapon const* pWeapon, WeaponFireInfo const& info)
{
	CTronProjectile::Setup(pWeapon, info);

	// Our own damage types can't hurt us
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);

	DamageFlags nCantDamageFlags = m_damage.GetCantDamageFlags();
	if (pAmmo->eInstDamageType)
	{
		nCantDamageFlags |= DamageTypeToFlag(pAmmo->eInstDamageType);
	}
	if (pAmmo->eAreaDamageType)
	{
		nCantDamageFlags |= DamageTypeToFlag(pAmmo->eAreaDamageType);
	}
	if (pAmmo->eProgDamageType)
	{
		nCantDamageFlags |= DamageTypeToFlag(pAmmo->eProgDamageType);
	}
	m_damage.SetCantDamageFlags(nCantDamageFlags);

	PROJECTILEFX *pProjectileFX = pAmmo->pProjectileFX;
	CORRUPTICDIMSCLASSDATA *pClassData = dynamic_cast<CORRUPTICDIMSCLASSDATA*>(pProjectileFX->pClassData);

	if (!pAmmo || !pProjectileFX || !pClassData) return LTFALSE;

	m_fRandomness			= pClassData->fRandomness;
	m_fRandomnessRampUpTime	= pClassData->fRandomnessRampUpTime;
	m_fHorizontalFactor		= pClassData->fHorizontalFactor;
	m_fVerticalFactor		= pClassData->fVerticalFactor;

	if (pClassData->bHoming)
	{
		m_hHomingTarget = info.hTestObj;
		m_fTurnRate = pClassData->fTurnRate;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorrupticDims::EngineMessageFn
//
//	PURPOSE:	Handle engine messages (RETAIL: 0x10036300)
//
// ----------------------------------------------------------------------- //

uint32 CCorrupticDims::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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

	return CTronProjectile::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorrupticDims::Save
//
//	PURPOSE:	Save the object (RETAIL: 0x10036390)
//
// ----------------------------------------------------------------------- //

void CCorrupticDims::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fRandomness);
	SAVE_FLOAT(m_fRandomnessRampUpTime);
	SAVE_FLOAT(m_fHorizontalFactor);
	SAVE_FLOAT(m_fVerticalFactor);
	SAVE_HOBJECT(m_hHomingTarget);
	SAVE_FLOAT(m_fTurnRate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorrupticDims::Load
//
//	PURPOSE:	Load the object (RETAIL: 0x10036420)
//
// ----------------------------------------------------------------------- //

void CCorrupticDims::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_FLOAT(m_fRandomness);
	LOAD_FLOAT(m_fRandomnessRampUpTime);
	LOAD_FLOAT(m_fHorizontalFactor);
	LOAD_FLOAT(m_fVerticalFactor);
	LOAD_HOBJECT(m_hHomingTarget);
	LOAD_FLOAT(m_fTurnRate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorrupticDims::Update
//
//	PURPOSE:	Now and then move the projectile off its line of fire, the
//				more so the longer it flies (RETAIL: 0x100364c0)
//
// ----------------------------------------------------------------------- //

// PORT: near (the distance along the line of fire is computed differently)
void CCorrupticDims::Update()
{
	LTFLOAT fRampUp = g_pLTServer->GetTime() - m_fStartTime;
	if (MATH_EPSILON <= m_fRandomnessRampUpTime && m_fRandomnessRampUpTime > fRampUp)
	{
		fRampUp /= m_fRandomnessRampUpTime;
	}
	else
	{
		fRampUp = 1.0f;
	}

	LTFLOAT fChance = fRampUp * m_fRandomness;
	if (GetRandom(0.0f, 1.0f) <= fChance)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		// How far along the line of fire are we?
		LTFLOAT fDist;
		if ((vPos - m_vFirePos).MagSqr() <= 0.0f)
		{
			fDist = 0.0f;
		}
		else
		{
			LTVector vDelta = vPos - m_vFirePos;
			fDist = vDelta.Mag() * vDelta.Unit().Dot(m_vDir);
		}

		LTVector vOnLine = m_vFirePos + m_vDir * fDist;

		LTVector vOffset(GetRandom(-1.0f, 1.0f) * m_fHorizontalFactor * fRampUp,
						 GetRandom(-1.0f, 1.0f) * m_fVerticalFactor * fRampUp,
						 0.0f);

		LTRotation rRot(m_vDir, LTVector(0.0f, 1.0f, 0.0f));
		LTMatrix mRot;
		rRot.ConvertToMatrix(mRot);

		LTVector vNewPos = mRot * vOffset + vOnLine;
		g_pLTServer->SetObjectPos(m_hObject, &vNewPos);
	}
}
