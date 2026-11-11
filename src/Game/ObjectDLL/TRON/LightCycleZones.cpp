// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleZones.cpp
//
// PURPOSE : Volume brushes that speed up or slow down light cycles
//
// RETAIL: Tron 2.0's, written from retail Object.lto (LightCycleZones.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleZones.h"
#include "ServerUtilities.h"

BEGIN_CLASS(LightCycleSpeedZone)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(LightCycleSpeedZone, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

BEGIN_CLASS(LightCycleSlowZone)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(LightCycleSlowZone, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleSpeedZone::UpdatePhysics
//
//	PURPOSE:	Flag the light cycles in us to speed up
//
// ----------------------------------------------------------------------- //

void LightCycleSpeedZone::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	if (m_bHidden || !pCPStruct || !pCPStruct->m_hObject) return;

	if (IsKindOf(pCPStruct->m_hObject, "AILightCycle"))
	{
		LCZoneLightCycleView* pCycle = (LCZoneLightCycleView*)g_pLTServer->HandleToObject(pCPStruct->m_hObject);
		pCycle->dwFlags |= LCZONE_FLAG_SPEED;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleSlowZone::UpdatePhysics
//
//	PURPOSE:	Flag the light cycles in us to slow down
//
// ----------------------------------------------------------------------- //

void LightCycleSlowZone::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	if (m_bHidden || !pCPStruct || !pCPStruct->m_hObject) return;

	if (IsKindOf(pCPStruct->m_hObject, "AILightCycle"))
	{
		LCZoneLightCycleView* pCycle = (LCZoneLightCycleView*)g_pLTServer->HandleToObject(pCPStruct->m_hObject);
		pCycle->dwFlags |= LCZONE_FLAG_SLOW;
	}
}
