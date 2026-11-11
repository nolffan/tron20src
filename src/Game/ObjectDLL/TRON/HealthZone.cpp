// ----------------------------------------------------------------------- //
//
// MODULE  : HealthZone.cpp
//
// PURPOSE : A volume brush that heals (or hurts) a character
//
// RETAIL: Tron 2.0's, written from retail Object.lto (HealthZone.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HealthZone.h"
#include "Character.h"
#include "PlayerObj.h"
#include <float.h>

BEGIN_CLASS(HealthZone)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
	ADD_COLORPROP(TintColor, 200.0f, 0.0f, 0.0f)
END_CLASS_DEFAULT_FLAGS(HealthZone, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( HealthZone )
CMDMGR_END_REGISTER_CLASS( HealthZone, VolumeBrush )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HealthZone::UpdatePhysics
//
//	PURPOSE:	VolumeBrush's, with the damage given to (or, negative,
//				taken from) a character's hit points directly; a character
//				brought below one hit point is killed
//
// ----------------------------------------------------------------------- //

void HealthZone::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	if (m_bHidden || !pCPStruct || !pCPStruct->m_hObject) return;

    LTFLOAT fUpdateDelta = g_pLTServer->GetFrameTime();


	// Let the character know if they are in liquid...

	if (IsLiquid(m_eContainerCode) && IsCharacter(pCPStruct->m_hObject))
	{
        CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(pCPStruct->m_hObject);
		if (pCharacter)
		{
			pCharacter->UpdateInLiquid(this, pCPStruct);
		}
	}


	// Player container physics is done on the client...

	if (!IsPlayer(pCPStruct->m_hObject))
	{
		// Dampen velocity based on the viscosity of the container...

        LTVector vVel, vCurVel;
		vVel = vCurVel = pCPStruct->m_Velocity;

		if (m_fViscosity > 0.0f && VEC_MAG(vCurVel) > 1.0f)
		{
            LTVector vDir;
			VEC_COPY(vDir, vCurVel);
			VEC_NORM(vDir);

            LTFLOAT fAdjust = MAX_CONTAINER_VISCOSITY * m_fViscosity * fUpdateDelta;

			vVel = (vDir * fAdjust);

			if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
			{
				VEC_SUB(vVel, vCurVel, vVel);
			}
			else
			{
				VEC_INIT(vVel);
			}

			vVel += (m_vCurrent * fUpdateDelta);

			pCPStruct->m_Velocity = vVel;
		}


		// Do special liquid handling...

		if (IsLiquid(m_eContainerCode))
		{
			UpdateLiquidPhysics(pCPStruct);
		}
	}


	// Update the character's hit points...

	if (!IsCharacter(pCPStruct->m_hObject)) return;

	if (m_fDamage > FLT_EPSILON || m_fDamage < -FLT_EPSILON)
	{
		LTFLOAT fDamage = fUpdateDelta * m_fDamage;
		if (fDamage != 0.0f)
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(pCPStruct->m_hObject);

			bool bKill;
			LTFLOAT fHitPoints = pCharacter->GetDestructible()->GetHitPoints() - fDamage;
			if (fHitPoints < 0.0f)
			{
				fHitPoints = 0.0f;
				bKill = true;
			}
			else
			{
				if (pCharacter->GetDestructible()->GetMaxHitPoints() < fHitPoints)
				{
					fHitPoints = pCharacter->GetDestructible()->GetMaxHitPoints();
				}

				bKill = (fHitPoints < 1.0f);
			}

			if (IsPlayer(pCPStruct->m_hObject))
			{
				if (((CPlayerObj*)pCharacter)->InGodMode()) return;

				pCharacter->GetDestructible()->SetHitPoints(fHitPoints);
			}
			else
			{
				pCharacter->GetDestructible()->SetHitPoints(fHitPoints);
			}

			if (bKill)
			{
				DamageStruct damage;

				damage.eType	= DT_EXPLODE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager	= m_hObject;
				damage.vDir.Init(0.0f, 1.0f, 0.0f);
				damage.bTronUnknown44 = LTTRUE;

				damage.DoDamage(this, pCharacter->m_hObject);
			}
		}
	}
}
