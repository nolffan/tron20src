// ----------------------------------------------------------------------- //
//
// MODULE  : HealthZone.h
//
// PURPOSE : A volume brush that heals (or hurts) a character
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x100214c0..0x10021b40, its class def at 0x102d96f0). The
// names of the class and its properties are retail's; the rest are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HEALTHZONE_H__
#define __HEALTHZONE_H__

#include "VolumeBrush.h"

class HealthZone : public VolumeBrush
{
	public :

		HealthZone() : VolumeBrush() {}

	protected :

		virtual void	UpdatePhysics(ContainerPhysics* pCPStruct);
};

#endif // __HEALTHZONE_H__
