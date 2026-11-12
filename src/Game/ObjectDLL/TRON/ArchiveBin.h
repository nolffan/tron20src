// ----------------------------------------------------------------------- //
//
// MODULE  : ArchiveBin.h
//
// PURPOSE : An archive bin: a model holding up to six archive items a
//			 player can withdraw through the decision window, shown as
//			 berries on its frame
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto 0x1000ae60..0x10012b10 (ArchiveBin, vtable 0x1029354c, 0x400
// bytes; Berry, vtable 0x102934c4; CArchiveBinPlugin, 0x10293508). The
// class and property names are retail's; the members and functions are
// ours.
//
// ----------------------------------------------------------------------- //

#ifndef __ARCHIVEBIN_H__
#define __ARCHIVEBIN_H__

#include "GameBase.h"
#include "ActivateTypeMgr.h"
#include "AutoCommand.h"
#include "ArchiveItem.h"
#include "Character.h"
#include <vector>


#define AB_MAX_ITEMS		6
#define AB_MAX_BERRIES		28

// A berry: a model attached to the bin, one per item (RETAIL: Berry has
// no members of its own; the class name is retail's)
class Berry : public BaseClass
{
};

class ArchiveBin : public GameBase
{
	public :

		ArchiveBin();
		~ArchiveBin();

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		// A berry on the frame and the item it shows
		struct BerryRef
		{
			BerryRef() : m_hObject(LTNULL), m_nItem(-1) {}

			LTObjRef	m_hObject;
			int			m_nItem;
		};

		typedef std::vector<AutoCommand<SingleDispatch> > OnWithdrawList;

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		LTBOOL	InitialUpdate();
		void	CreateFrame();
		void	CreateBerries();
		void	ShowContents();
		void	HandleChoice(uint8 nChoice);
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		int		GetNumItems();
		void	HandleModelString(ArgList* pArgList);
		void	PlayLoopSound();

		ArchiveItem					m_aItems[AB_MAX_ITEMS];
		int							m_aeShownCondition[AB_MAX_ITEMS];	// What a virus scan may show of each condition
		LTObjRef					m_hFrame;
		uint8						m_nPSets;				// The permission sets a player needs
		bool						m_bMoveToFloor;
		bool						m_bFirstUpdate;
		float						m_fRadius;				// The decision window's radius
		bool						m_bStartCompressed;
		bool						m_bTronUnknown171;		// From the model's bute (ReadProp, retail 0x1000e04d)
		BerryRef					m_aBerries[AB_MAX_BERRIES];
		LTObjRef					m_hTronUnknown3a4;
		HSTRING						m_hstrLoopSound;		// A looping sound from the model's keys
		HLTSOUND					m_hLoopSound;
		OnWithdrawList				m_lstOnWithdraw;		// Run when an item is withdrawn, once
		AutoCommand<MultiDispatch>	m_OnUseCommand;
		AutoCommand<MultiDispatch>	m_InvalidPSetCommand;	// Run when a player without the permission sets tries
		CActivateTypeHandler		m_ActivateTypeHandler;
};

class CArchiveBinPlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int nModelFilenameBufLen,
			LTVector & vDims);

		virtual LTRESULT PreHook_PropChanged(
			const char *szObjName,
			const char *szPropName,
			const int nPropType,
			const GenericProp &gpPropValue,
			ILTPreInterface *pInterface,
			const char *szModifiers);

	protected :

		CCommandMgrPlugin	m_CommandMgrPlugin;
		CCharacterPlugin	m_CharacterPlugin;
};

#endif // __ARCHIVEBIN_H__
