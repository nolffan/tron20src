// ----------------------------------------------------------------------- //
//
// MODULE  : EnergyZone.h
//
// PURPOSE : A volume brush that drains (or gives) a character's energy
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1001a7e0..0x1001adb0, its class def at 0x102d8268). The
// names of the class and its properties are retail's; the rest are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __ENERGYZONE_H__
#define __ENERGYZONE_H__

#include "VolumeBrush.h"

class EnergyZone : public VolumeBrush
{
	public :

		EnergyZone() : VolumeBrush()
		{
			m_fEnergyDrain = 0.0f;
		}

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		virtual void	ReadProp(ObjectCreateStruct *pStruct);
		virtual void	UpdatePhysics(ContainerPhysics* pCPStruct);

	private :

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		LTFLOAT		m_fEnergyDrain;		// Energy a second taken from a character
};

#endif // __ENERGYZONE_H__
