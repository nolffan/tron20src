// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleZones.h
//
// PURPOSE : Volume brushes that speed up or slow down light cycles
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1002c340..0x1002c6e0, their class defs at 0x102da5d0 and
// 0x102da600). The names of the classes are retail's, the module's ours.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEZONES_H__
#define __LIGHTCYCLEZONES_H__

#include "VolumeBrush.h"

// RETAIL: the flags of an AILightCycle (+0xbc) these zones set; that class
// is still to be written. The names are ours
#define LCZONE_FLAG_SPEED		(1<<20)
#define LCZONE_FLAG_SLOW		(1<<24)
struct LCZoneLightCycleView { uint8 aPad[0xbc]; uint32 dwFlags; };
#define LCZONE_LIGHTCYCLE_FLAGS(pObj)	(((LCZoneLightCycleView*)(pObj))->dwFlags)

class LightCycleSpeedZone : public VolumeBrush
{
	public :

		LightCycleSpeedZone() : VolumeBrush()
		{
			m_eContainerCode = CC_LIGHTCYCLE_SPEED_ZONE;
		}

	protected :

		virtual void	UpdatePhysics(ContainerPhysics* pCPStruct);
};

class LightCycleSlowZone : public VolumeBrush
{
	public :

		LightCycleSlowZone() : VolumeBrush()
		{
			m_eContainerCode = CC_LIGHTCYCLE_SLOW_ZONE;
		}

	protected :

		virtual void	UpdatePhysics(ContainerPhysics* pCPStruct);
};

#endif // __LIGHTCYCLEZONES_H__
