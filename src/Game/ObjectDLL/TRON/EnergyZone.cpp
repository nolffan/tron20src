// ----------------------------------------------------------------------- //
//
// MODULE  : EnergyZone.cpp
//
// PURPOSE : A volume brush that drains (or gives) a character's energy
//
// RETAIL: Tron 2.0's, written from retail Object.lto (EnergyZone.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "EnergyZone.h"
#include "Character.h"
#include "PlayerObj.h"
#include <float.h>

BEGIN_CLASS(EnergyZone)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(SoundFilter, "energy_up", PF_STATICLIST)
	ADD_REALPROP(EnergyDrain, 0.0f)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
	ADD_COLORPROP(TintColor, 0.0f, 0.0f, 200.0f)
END_CLASS_DEFAULT_FLAGS(EnergyZone, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( EnergyZone )
CMDMGR_END_REGISTER_CLASS( EnergyZone, VolumeBrush )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnergyZone::ReadProp
//
//	PURPOSE:	Read in the properties
//
// ----------------------------------------------------------------------- //

void EnergyZone::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	g_pLTServer->GetPropReal("EnergyDrain", &m_fEnergyDrain);

	VolumeBrush::ReadProp(pStruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnergyZone::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 EnergyZone::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
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
	}

	return VolumeBrush::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnergyZone::UpdatePhysics
//
//	PURPOSE:	Take a character's energy, then what any volume does
//
// ----------------------------------------------------------------------- //

void EnergyZone::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	if (m_bHidden || !pCPStruct || !pCPStruct->m_hObject) return;

	if (IsCharacter(pCPStruct->m_hObject))
	{
		LTFLOAT fUpdateDelta = g_pLTServer->GetFrameTime();

		if (m_fEnergyDrain > FLT_EPSILON || m_fEnergyDrain < -FLT_EPSILON)
		{
			LTFLOAT fDrain = fUpdateDelta * m_fEnergyDrain;
			if (fDrain != 0.0f)
			{
				CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(pCPStruct->m_hObject);

				LTFLOAT fEnergy = pCharacter->GetDestructible()->GetEnergy() - fDrain;
				if (fEnergy < 0.0f)
				{
					fEnergy = 0.0f;
				}
				else if (pCharacter->GetDestructible()->GetMaxEnergy() < fEnergy)
				{
					fEnergy = pCharacter->GetDestructible()->GetMaxEnergy();
				}

				if (IsPlayer(pCPStruct->m_hObject))
				{
					if (!((CPlayerObj*)pCharacter)->InGodMode())
					{
						pCharacter->GetDestructible()->SetEnergy(fEnergy);
					}
				}
				else
				{
					pCharacter->GetDestructible()->SetEnergy(fEnergy);
				}
			}
		}
	}

	VolumeBrush::UpdatePhysics(pCPStruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnergyZone::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void EnergyZone::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fEnergyDrain);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnergyZone::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void EnergyZone::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_FLOAT(m_fEnergyDrain);
}
