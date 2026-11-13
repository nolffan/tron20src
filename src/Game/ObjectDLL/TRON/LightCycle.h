// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycle.h
//
// PURPOSE : The model of a light cycle in the light cycle game
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10024c40..0x100257e0: its class def at 0x102d98f8, vtable
// 0x10293cdc, 0x6c bytes; its plugin CLCPlugin, vtable 0x10293cc0). The
// names of the classes, the properties and the special fx struct are
// retail's (RTTI, strings); the rest are ours. AILightCycle derives from it.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLE_H__
#define __LIGHTCYCLE_H__

#include "GameBase.h"
#include "SharedFXStructs.h"
#include "LightCycleButeMgr.h"

class CPlayerObj;

class LightCycle : public GameBase
{
	public :

		LightCycle();									// RETAIL: 0x10024fb0
		virtual ~LightCycle();							// RETAIL: 0x10025070

		// Sets the cycle up for its rider (NULL for none): its model, its
		// dims, on the floor, and its special fx. RETAIL: 0x10025310 (slot 25)
		virtual void	Init(CPlayerObj* pPlayer);

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);	// RETAIL: 0x100250f0
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);		// RETAIL: 0x100251d0

		virtual void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);	// RETAIL: 0x10025760 (slot 26)
		virtual void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);	// RETAIL: 0x100257a0 (slot 27)
		virtual LTBOOL	InitialUpdate();									// RETAIL: 0x100255f0 (slot 28)

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);		// RETAIL: 0x10025230
		void	SetModel();									// RETAIL: 0x100253b0
		void	CreateSpecialFX();							// RETAIL: 0x10025610

		LIGHTCYCLECREATESTRUCT	m_cs;						// RETAIL: +0x38
		char		m_szCycleColor[32];						// RETAIL: +0x44 CycleColor
		HSTRING		m_hstrRezInFXDoneCommand;				// RETAIL: +0x64 RezInFXDoneCommand
		HOBJECT		m_hPlayer;								// RETAIL: +0x68 (the rider)
};

// RETAIL: vtable 0x10293cc0 (4 bytes): fills CycleColor
class CLCPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(		// RETAIL: 0x10024e00
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

	protected :

		static bool					sm_bInitted;		// RETAIL: 0x10358b14
		static CLightCycleButeMgr	sm_ButeMgr;			// RETAIL: 0x103577e8
};

#endif // __LIGHTCYCLE_H__
