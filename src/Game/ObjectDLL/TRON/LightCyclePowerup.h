// ----------------------------------------------------------------------- //
//
// MODULE  : LightCyclePowerup.h
//
// PURPOSE : A power-up of the light cycle game, picked up by touch
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1002ab50..0x1002bd70: its class def at 0x102da4b0, vtable
// 0x10293e90, 0x60 bytes; its plugin CLightCyclePowerupPlugin, vtable
// 0x10293e74). The names of the classes, the properties and the error
// message's function name (ObjectTouch) are retail's; the rest are ours.
// It adds itself to CServerLightCycleMgr's power-ups and removes itself.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEPOWERUP_H__
#define __LIGHTCYCLEPOWERUP_H__

#include "GameBase.h"

class LightCyclePowerup : public GameBase
{
	public :

		LightCyclePowerup();							// RETAIL: 0x1002ae50
		virtual ~LightCyclePowerup();					// RETAIL: 0x1002af10

		// The power-up it gives: a random allowed one for Random.
		// RETAIL: 0x1002ba70
		int		GetType();

		// Shown and touchable, or not. RETAIL: 0x1002b800
		void	SetEnabled(bool bEnabled);

		bool	IsEnabled() const { return m_bEnabled; }

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);	// RETAIL: 0x1002afc0
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);			// RETAIL: 0x1002b8a0 (slot 24)

		// A cycle drives through it. RETAIL: 0x1002b570 (slot 25)
		virtual void	ObjectTouch(HOBJECT hObj);

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);			// RETAIL: 0x1002b400
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);	// RETAIL: 0x1002b6b0
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);	// RETAIL: 0x1002b710
		void	CreateFX();										// RETAIL: 0x1002b790

		LTObjRef	m_hFX;						// RETAIL: +0x38 (its looping client fx)
		bool		m_bFromWorld;				// RETAIL: +0x48 (placed in the level: dropped to the floor)
		HSTRING		m_hstrPickupCommand;		// RETAIL: +0x4c PickupCommand
		LTFLOAT		m_fRespawnTime;				// RETAIL: +0x50 RespawnTime (-1 for never, 0 for always there)
		int			m_nType;					// RETAIL: +0x54 Type (LCPowerupType)
		bool		m_bEnabled;					// RETAIL: +0x58
		bool		m_bAllowInCustom;			// RETAIL: +0x59 AllowInCustom
		uint32		m_nTronUnknown5C;			// RETAIL: +0x5c (only cleared by the constructor)
};

// RETAIL: vtable 0x10293e74 (4 bytes): fills Type
class CLightCyclePowerupPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(		// RETAIL: 0x1002ade0
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);
};

#endif // __LIGHTCYCLEPOWERUP_H__
