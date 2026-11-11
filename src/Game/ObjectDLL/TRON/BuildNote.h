// ----------------------------------------------------------------------- //
//
// MODULE  : BuildNote.h
//
// PURPOSE : A build note: picked up, it gives the player its build points
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10016090..0x10016ec0, its class def at 0x102d7f38). The
// names of the class, its property, its messages and its plugin are
// retail's; the rest are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __BUILDNOTE_H__
#define __BUILDNOTE_H__

#include "GameBase.h"
#include "ActivateTypeMgr.h"

#define BUILDNOTE_MAX_NAME_LEN	32

class BuildNote : public GameBase
{
	public :

		BuildNote();
		virtual ~BuildNote();

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void	InitialUpdate();

		CActivateTypeHandler	m_ActivateTypeHandler;
		char					m_szNoteName[BUILDNOTE_MAX_NAME_LEN];	// The note's inventory item
		LTObjRef				m_hFX;									// Our base fx
		bool					m_bFromWorld;							// Placed in a level (moved to the floor)
};

class CBuildNotePlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);
};

#endif // __BUILDNOTE_H__
