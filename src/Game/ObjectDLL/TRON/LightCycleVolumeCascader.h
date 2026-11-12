// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleVolumeCascader.h
//
// PURPOSE : Turns ON the LightCycleVolumes around it in a growing circle
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1002d700..0x1002e5d0, its class def at 0x102da6d8,
// vtable 0x102940c4). The names of the class and its properties are
// retail's; the rest are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEVOLUMECASCADER_H__
#define __LIGHTCYCLEVOLUMECASCADER_H__

#include "GameBase.h"
#include <list>

class LightCycleVolumeCascader : public GameBase
{
	public :

		LightCycleVolumeCascader();
		virtual ~LightCycleVolumeCascader();

	protected :

		virtual uint32	OnPrecreate(ObjectCreateStruct* pOCS, float precreateType);
		virtual uint32	OnAllObjectsCreated();
		virtual uint32	OnUpdate();
		virtual uint32	OnLoad(ILTMessage_Read *pMsg, float dwLoadFlags);
		virtual uint32	OnSave(ILTMessage_Write *pMsg, float dwSaveFlags);

		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		typedef std::list<LTObjRef> VolumeList;

		bool		m_bCascading;		// RETAIL: +0x38
		bool		m_bDone;			// RETAIL: +0x39
		LTFLOAT		m_fCascadeRate;		// RETAIL: +0x3c
		LTFLOAT		m_fMaxRadius;		// RETAIL: +0x40
		LTFLOAT		m_fRadius;			// RETAIL: +0x44
		VolumeList	m_lstVolumes;		// RETAIL: +0x48, those not yet turned ON
};

#endif // __LIGHTCYCLEVOLUMECASCADER_H__
