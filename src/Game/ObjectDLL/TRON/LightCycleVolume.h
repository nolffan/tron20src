// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleVolume.h
//
// PURPOSE : A model (a wall, a ramp) on a light cycle grid that raises
//           itself when it is turned ON
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1002c6e0..0x1002d700, its class def at 0x102da6a8,
// vtable 0x10294054). The names of the classes, the properties, the
// animations and the plugin are retail's; the rest are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEVOLUME_H__
#define __LIGHTCYCLEVOLUME_H__

#include "GameBase.h"
#include "ModelButeMgr.h"

class LightCycleVolume : public GameBase
{
	public :

		LightCycleVolume();
		virtual ~LightCycleVolume();

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		virtual uint32	OnPrecreate(ObjectCreateStruct* pOCS, float precreateType);
		virtual uint32	OnObjectCreated(float createType);
		virtual uint32	OnUpdate();
		virtual uint32	OnLoad(ILTMessage_Read *pMsg, float dwLoadFlags);
		virtual uint32	OnSave(ILTMessage_Write *pMsg, float dwSaveFlags);

		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		// RETAIL: 0 plays the Raise animation, 1 the Up one (0x1002d530)
		void	SetAnimation(uint32 nState);

		bool		m_bRaised;			// RETAIL: +0x38
		HMODELANIM	m_hAnim;			// RETAIL: +0x3c
		ModelId		m_eModelId;			// RETAIL: +0x40
		uint32		m_bEastWest;		// RETAIL: +0x44, facing east-west
};

class LightCycleVolumeObjectPlugin : public IObjectPlugin
{
	public :

		LightCycleVolumeObjectPlugin();

		virtual ~LightCycleVolumeObjectPlugin()
		{
			if (m_pModelButeMgrPlugin)
			{
				delete m_pModelButeMgrPlugin;
				m_pModelButeMgrPlugin = LTNULL;
			}
		}

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

	private :

		CModelButeMgrPlugin*	m_pModelButeMgrPlugin;

		static bool				sm_bInitted;
};

#endif // __LIGHTCYCLEVOLUME_H__
