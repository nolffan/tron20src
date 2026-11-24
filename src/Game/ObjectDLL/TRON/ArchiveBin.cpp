// ----------------------------------------------------------------------- //
//
// MODULE  : ArchiveBin.cpp
//
// PURPOSE : An archive bin (ArchiveBin.h)
//
// RETAIL: written from retail Object.lto (ArchiveBin.h). Not all of it
// yet: what is not is marked PORT.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ArchiveBin.h"
#include "PSets.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ServerUtilities.h"
#include "InventoryButeMgr.h"
#include "ModelButeMgr.h"
#include "CommandMgr.h"
#include "ServerSoundMgr.h"
#include "TronPlayerObj.h"
#include "GameServerShell.h"
#include "ServerSubroutineMgr.h"
#include "SubroutineDataMgr.h"
#include <algorithm>

extern CInventoryButeMgr s_InventoryButeMgr;

// RETAIL: a user flag the bin sets on itself, as an I/O node does
// (IONode.h's USRFLG_IONODE); the name is ours
#define USRFLG_ARCHIVEBIN		(1<<1)

// RETAIL: MID_DECISION's first byte for a bin (CoreDump's is 3); the name is ours
#define DECISION_TYPE_ARCHIVEBIN		1

// RETAIL: the decision window's choices besides the items (as CoreDump's)
#define AB_CHOICE_DESTROY				0xfc
#define AB_CHOICE_CANCEL				0xfd

BEGIN_CLASS(Berry)
END_CLASS_DEFAULT_FLAGS(Berry, BaseClass, NULL, NULL, CF_HIDDEN)

BEGIN_CLASS(ArchiveBin)
	ADD_BOOLPROP_FLAG(Template, LTFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ModelTemplate, "<none>", PF_STATICLIST | PF_DIMS)
	ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, 0)
	ADD_REALPROP_FLAG(Radius, 300.0f, PF_RADIUS)
	ADD_BOOLPROP_FLAG(StartCompressed, LTFALSE, 0)
	ADD_STRINGPROP_FLAG(OnUseCmd, "", 0)
	ADD_STRINGPROP_FLAG(InvalidPSetCommand, "", 0)
	PROP_DEFINEGROUP(Item1, PF_GROUP(1))
	ADD_STRINGPROP_FLAG(Type1, "", PF_GROUP(1) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineName1, "<none>", PF_GROUP(1) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineBuild1, "", PF_GROUP(1) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineCondition1, "", PF_GROUP(1) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(ProceduralName1, "<none>", PF_GROUP(1) | PF_STATICLIST)
	ADD_LONGINTPROP_FLAG(PSetNumber1, 0, PF_GROUP(1))
	ADD_STRINGPROP_FLAG(PrimitiveName1, "<none>", PF_GROUP(1) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(EmailName1, "<none>", PF_GROUP(1) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(HelpFile1, "<none>", PF_GROUP(1) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CustomCmdName1, "<none>", PF_GROUP(1))
	ADD_STRINGPROP_FLAG(CustomCmd1, "<none>", PF_GROUP(1))
	ADD_REALPROP_FLAG(Cost1, 0.0f, PF_GROUP(1))
	ADD_STRINGPROP_FLAG(OnWithdraw1, "", PF_GROUP(1))
	PROP_DEFINEGROUP(Item2, PF_GROUP(2))
	ADD_STRINGPROP_FLAG(Type2, "", PF_GROUP(2) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineName2, "<none>", PF_GROUP(2) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineBuild2, "", PF_GROUP(2) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineCondition2, "", PF_GROUP(2) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(ProceduralName2, "<none>", PF_GROUP(2) | PF_STATICLIST)
	ADD_LONGINTPROP_FLAG(PSetNumber2, 0, PF_GROUP(2))
	ADD_STRINGPROP_FLAG(PrimitiveName2, "<none>", PF_GROUP(2) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(EmailName2, "<none>", PF_GROUP(2) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(HelpFile2, "<none>", PF_GROUP(2) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CustomCmdName2, "<none>", PF_GROUP(2))
	ADD_STRINGPROP_FLAG(CustomCmd2, "<none>", PF_GROUP(2))
	ADD_REALPROP_FLAG(Cost2, 0.0f, PF_GROUP(2))
	ADD_STRINGPROP_FLAG(OnWithdraw2, "", PF_GROUP(2))
	PROP_DEFINEGROUP(Item3, PF_GROUP(3))
	ADD_STRINGPROP_FLAG(Type3, "", PF_GROUP(3) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineName3, "<none>", PF_GROUP(3) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineBuild3, "", PF_GROUP(3) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineCondition3, "", PF_GROUP(3) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(ProceduralName3, "<none>", PF_GROUP(3) | PF_STATICLIST)
	ADD_LONGINTPROP_FLAG(PSetNumber3, 0, PF_GROUP(3))
	ADD_STRINGPROP_FLAG(PrimitiveName3, "<none>", PF_GROUP(3) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(EmailName3, "<none>", PF_GROUP(3) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(HelpFile3, "<none>", PF_GROUP(3) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CustomCmdName3, "<none>", PF_GROUP(3))
	ADD_STRINGPROP_FLAG(CustomCmd3, "<none>", PF_GROUP(3))
	ADD_REALPROP_FLAG(Cost3, 0.0f, PF_GROUP(3))
	ADD_STRINGPROP_FLAG(OnWithdraw3, "", PF_GROUP(3))
	PROP_DEFINEGROUP(Item4, PF_GROUP(4))
	ADD_STRINGPROP_FLAG(Type4, "", PF_GROUP(4) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineName4, "<none>", PF_GROUP(4) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineBuild4, "", PF_GROUP(4) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineCondition4, "", PF_GROUP(4) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(ProceduralName4, "<none>", PF_GROUP(4) | PF_STATICLIST)
	ADD_LONGINTPROP_FLAG(PSetNumber4, 0, PF_GROUP(4))
	ADD_STRINGPROP_FLAG(PrimitiveName4, "<none>", PF_GROUP(4) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(EmailName4, "<none>", PF_GROUP(4) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(HelpFile4, "<none>", PF_GROUP(4) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CustomCmdName4, "<none>", PF_GROUP(4))
	ADD_STRINGPROP_FLAG(CustomCmd4, "<none>", PF_GROUP(4))
	ADD_REALPROP_FLAG(Cost4, 0.0f, PF_GROUP(4))
	ADD_STRINGPROP_FLAG(OnWithdraw4, "", PF_GROUP(4))
	PROP_DEFINEGROUP(Item5, PF_GROUP(5))
	ADD_STRINGPROP_FLAG(Type5, "", PF_GROUP(5) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineName5, "<none>", PF_GROUP(5) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineBuild5, "", PF_GROUP(5) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineCondition5, "", PF_GROUP(5) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(ProceduralName5, "<none>", PF_GROUP(5) | PF_STATICLIST)
	ADD_LONGINTPROP_FLAG(PSetNumber5, 0, PF_GROUP(5))
	ADD_STRINGPROP_FLAG(PrimitiveName5, "<none>", PF_GROUP(5) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(EmailName5, "<none>", PF_GROUP(5) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(HelpFile5, "<none>", PF_GROUP(5) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CustomCmdName5, "<none>", PF_GROUP(5))
	ADD_STRINGPROP_FLAG(CustomCmd5, "<none>", PF_GROUP(5))
	ADD_REALPROP_FLAG(Cost5, 0.0f, PF_GROUP(5))
	ADD_STRINGPROP_FLAG(OnWithdraw5, "", PF_GROUP(5))
	PROP_DEFINEGROUP(Item6, PF_GROUP(6))
	ADD_STRINGPROP_FLAG(Type6, "", PF_GROUP(6) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineName6, "<none>", PF_GROUP(6) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineBuild6, "", PF_GROUP(6) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SubroutineCondition6, "", PF_GROUP(6) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(ProceduralName6, "<none>", PF_GROUP(6) | PF_STATICLIST)
	ADD_LONGINTPROP_FLAG(PSetNumber6, 0, PF_GROUP(6))
	ADD_STRINGPROP_FLAG(PrimitiveName6, "<none>", PF_GROUP(6) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(EmailName6, "<none>", PF_GROUP(6) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(HelpFile6, "<none>", PF_GROUP(6) | PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CustomCmdName6, "<none>", PF_GROUP(6))
	ADD_STRINGPROP_FLAG(CustomCmd6, "<none>", PF_GROUP(6))
	ADD_REALPROP_FLAG(Cost6, 0.0f, PF_GROUP(6))
	ADD_STRINGPROP_FLAG(OnWithdraw6, "", PF_GROUP(6))
	ADD_PSETS_PROP(PF_GROUP(7))
END_CLASS_DEFAULT_FLAGS_PLUGIN(ArchiveBin, GameBase, NULL, NULL, 0, CArchiveBinPlugin)

// RETAIL: in .rdata (const arrays), unlike the other strings
static const char s_szModelTemplateProp[] = "ModelTemplate";
static const char s_szArchiveBinModel[] = "ArchiveBin";
static const char s_szSocket[] = "Socket";
static const char s_szBerryClass[] = "Berry";
static const char s_szPSetBerryModel[] = "PSetBerry";
static const char s_szSubroutineBerryModel[] = "SubroutineBerry";
static const char s_szDocumentBerryModel[] = "DocumentBerry";
static const char s_szFrameModel[] = "ArchiveBin_Frame";
static const char s_szSelectMsg[] = "SELECT";
static const char s_szDecisionAbortedMsg[] = "DECISIONABORTED";

CMDMGR_BEGIN_REGISTER_CLASS( ArchiveBin )

	CMDMGR_ADD_MSG( SELECT, 1, NULL, (char*)s_szSelectMsg )
	CMDMGR_ADD_MSG( DECISIONABORTED, 0, NULL, (char*)s_szDecisionAbortedMsg )

CMDMGR_END_REGISTER_CLASS( ArchiveBin, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArchiveBinPlugin::PreHook_PropChanged
//
//	PURPOSE:	Check our command strings (RETAIL: Spawner's, as it is)
//
// ----------------------------------------------------------------------- //

LTRESULT CArchiveBinPlugin::PreHook_PropChanged( const char *szObjName,
											  const char *szPropName,
											  const int  nPropType,
											  const GenericProp &gpPropValue,
											  ILTPreInterface *pInterface,
											  const char *szModifiers )
{
	if( !_stricmp( "InitialCommand", szPropName ))
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													szPropName,
													nPropType,
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArchiveBinPlugin::PreHook_EditStringList
//
//	PURPOSE:	The archive bin models, emails and help files
//
// ----------------------------------------------------------------------- //

LTRESULT CArchiveBinPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	if (!g_pModelButeMgr) return LT_OK;

	if (!strncmp(szPropName, s_szModelTemplateProp, strlen(s_szModelTemplateProp)))
	{
		uint32 cModels = g_pModelButeMgr->GetNumModels();
		for (uint32 iModel = 0; iModel < cModels; iModel++)
		{
			const char* pszModel = g_pModelButeMgr->GetModelName((ModelId)iModel);
			if (strstr(pszModel, s_szArchiveBinModel))
			{
				strcpy(aszStrings[(*pcStrings)++], pszModel);
			}
		}

		return LT_OK;
	}

	LTRESULT res = m_CharacterPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
	if (res != LT_OK) return res;

	if (!strncmp(szPropName, "EmailName", strlen("EmailName")))
	{
		uint32 cItems = s_InventoryButeMgr.GetNumItems();
		for (uint32 iItem = 0; iItem < cItems; iItem++)
		{
			if (s_InventoryButeMgr.GetItemType(iItem) == 0)
			{
				strcpy(aszStrings[(*pcStrings)++], s_InventoryButeMgr.GetItemName(iItem));
			}
		}
	}

	if (!strncmp(szPropName, "HelpFile", strlen("HelpFile")))
	{
		uint32 cItems = s_InventoryButeMgr.GetNumItems();
		for (uint32 iItem = 0; iItem < cItems; iItem++)
		{
			if (s_InventoryButeMgr.GetItemType(iItem) >= 2)
			{
				strcpy(aszStrings[(*pcStrings)++], s_InventoryButeMgr.GetItemName(iItem));
			}
		}
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArchiveBinPlugin::PreHook_Dims
//
//	PURPOSE:	The model template's dims
//
// ----------------------------------------------------------------------- //

LTRESULT CArchiveBinPlugin::PreHook_Dims(
	const char* szRezPath,
	const char* szPropValue,
	char* szModelFilenameBuf,
	int nModelFilenameBufLen,
	LTVector & vDims)
{
	if (!g_pModelButeMgr) return LT_OK;

	ModelId eModelId = g_pModelButeMgr->GetModelId(szPropValue);
	if (eModelId == eModelIdInvalid) return LT_UNSUPPORTED;

	const char* pszFilename = g_pModelButeMgr->GetModelFilename(eModelId);
	if (!pszFilename) return LT_UNSUPPORTED;

	LTStrCpy(szModelFilenameBuf, pszFilename, nModelFilenameBufLen);

	// The dims are in the .ltc
	if (szModelFilenameBuf && szModelFilenameBuf[0])
	{
		int nLen = strlen(szModelFilenameBuf);
		if (nLen > 0)
		{
			szModelFilenameBuf[nLen - 1] = 'c';
		}
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::ArchiveBin()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ArchiveBin::ArchiveBin()
:	GameBase			( OT_MODEL ),
	m_hPlayer			( LTNULL ),
	m_nPSets			( 0 ),
	m_bMoveToFloor		( true ),
	m_bFirstUpdate		( true ),
	m_fRadius			( 300.0f ),
	m_bStartCompressed	( false ),
	m_hTronUnknown3a4	( LTNULL ),
	m_hstrLoopSound		( LTNULL ),
	m_hLoopSound		( LTNULL )
{
	for (int i = 0; i < AB_MAX_ITEMS; i++)
	{
		m_aItems[i].Clear();
		m_aeShownCondition[i] = kAIC_Normal;
	}

	m_hstrLoopSound = LTNULL;
	m_hLoopSound = LTNULL;

	m_ActivateTypeHandler.Init(this, 0x39);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::~ArchiveBin()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ArchiveBin::~ArchiveBin()
{
	if (m_hstrLoopSound)
	{
		g_pLTServer->FreeString(m_hstrLoopSound);
	}

	if (m_hLoopSound)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
		m_hLoopSound = LTNULL;
	}

	for (int i = 0; i < AB_MAX_BERRIES; i++)
	{
		if (m_aBerries[i].m_hObject)
		{
			HATTACHMENT hAttachment;
			g_pLTServer->FindAttachment(m_hObject, m_aBerries[i].m_hObject, &hAttachment);
			g_pLTServer->RemoveAttachment(hAttachment);

			if (m_aBerries[i].m_hObject)
			{
				g_pLTServer->RemoveObject(m_aBerries[i].m_hObject);
				m_aBerries[i].m_hObject = LTNULL;
			}

			m_aBerries[i].m_eItemType = -1;
		}
	}

	if (m_hTronUnknown3a4)
	{
		HATTACHMENT hAttachment;
		g_pLTServer->FindAttachment(m_hObject, m_hTronUnknown3a4, &hAttachment);
		g_pLTServer->RemoveAttachment(hAttachment);
		g_pLTServer->RemoveObject(m_hTronUnknown3a4);
		m_hTronUnknown3a4 = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ArchiveBin::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
				ReadProp((ObjectCreateStruct*)pData);
				return dwRet;
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
		}
		break;

		case MID_UPDATE:
		{
			if (m_bFirstUpdate && m_bMoveToFloor)
			{
				m_bFirstUpdate = false;
				MoveObjectToFloor(m_hObject);
				SetNextUpdate(0.0f);
			}

			// RETAIL: no break: falls into MID_MODELSTRINGKEY, with no
			// arguments
		}

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 ArchiveBin::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DECISION:
		{
			uint8 nChoice = pMsg->Readuint8();
			HandleChoice(nChoice);
			return LT_OK;
		}
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::InitialUpdate
//
//	PURPOSE:	Size us, set our flags, and build the frame and berries
//
// ----------------------------------------------------------------------- //

LTBOOL ArchiveBin::InitialUpdate()
{
	LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, (GetNumItems() > 0) ? USRFLG_CAN_ACTIVATE : 0, USRFLG_CAN_ACTIVATE);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

	SET_OBJECT_PSETS_USER_FLAGS(m_hObject, m_nPSets);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_ARCHIVEBIN, USRFLG_ARCHIVEBIN);

	// Pick what a virus scan shows of each item's condition
	for (int i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (GetRandom(0.0f, 1.0f) < 0.5f)
		{
			if (m_aItems[i].m_eCondition == kAIC_Normal)
			{
				m_aeShownCondition[i] = kAIC_Infected;
			}
			else if (m_aItems[i].m_eCondition == kAIC_Infected)
			{
				m_aeShownCondition[i] = kAIC_Normal;
			}
		}
		else
		{
			m_aeShownCondition[i] = m_aItems[i].m_eCondition;
		}
	}

	CreateFrame();
	CreateBerries();

	SetNextUpdate(0.01f);

	m_ActivateTypeHandler.CreateActivateTypeMsg();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::GetNumItems
//
//	PURPOSE:	How many items we still hold
//
// ----------------------------------------------------------------------- //

int ArchiveBin::GetNumItems()
{
	int nNumItems = 0;
	for (int i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_None)
		{
			nNumItems++;
		}
	}

	return nNumItems;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ArchiveBin::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	int i;
	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		ArchiveItem& item = m_aItems[i];
		pMsg->Writeint8(item.m_eType);
		SAVE_CHARSTRING(item.m_sName.c_str());
		pMsg->Writeint8(item.m_eBuild);
		pMsg->Writeint8(item.m_eCondition);
		SAVE_CHARSTRING(item.m_sCommand.c_str());
		SAVE_BYTE(item.m_nPSet);
		SAVE_FLOAT(item.m_fCost);
		pMsg->Writeint8(m_aeShownCondition[i]);
	}

	SAVE_HOBJECT(m_hPlayer);
	SAVE_BYTE(m_nPSets);
	SAVE_bool(m_bMoveToFloor);
	SAVE_bool(m_bFirstUpdate);
	SAVE_FLOAT(m_fRadius);
	SAVE_bool(m_bStartCompressed);

	for (i = 0; i < AB_MAX_BERRIES; i++)
	{
		SAVE_HOBJECT(m_aBerries[i].m_hObject);
		SAVE_BYTE(m_aBerries[i].m_eItemType);
	}

	SAVE_HOBJECT(m_hTronUnknown3a4);
	SAVE_HSTRING(m_hstrLoopSound);

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		m_lstOnWithdraw.at(i).Save(pMsg);
	}

	m_OnUseCommand.Save(pMsg);
	m_InvalidPSetCommand.Save(pMsg);

	m_ActivateTypeHandler.Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ArchiveBin::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	char szString[256];

	int i;
	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		ArchiveItem& item = m_aItems[i];
		item.m_eType = pMsg->Readint8();
		LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
		item.m_sName = szString;
		item.m_eBuild = pMsg->Readint8();
		item.m_eCondition = pMsg->Readint8();
		LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
		item.m_sCommand = szString;
		LOAD_BYTE(item.m_nPSet);
		LOAD_FLOAT(item.m_fCost);
		LOAD_BYTE(m_aeShownCondition[i]);
	}

	LOAD_HOBJECT(m_hPlayer);
	LOAD_BYTE(m_nPSets);
	LOAD_bool(m_bMoveToFloor);
	LOAD_bool(m_bFirstUpdate);
	LOAD_FLOAT(m_fRadius);
	LOAD_bool(m_bStartCompressed);

	for (i = 0; i < AB_MAX_BERRIES; i++)
	{
		LOAD_HOBJECT(m_aBerries[i].m_hObject);
		LOAD_BYTE(m_aBerries[i].m_eItemType);
	}

	LOAD_HOBJECT(m_hTronUnknown3a4);
	LOAD_HSTRING(m_hstrLoopSound);
	m_hLoopSound = LTNULL;
	PlayLoopSound();

	m_lstOnWithdraw.resize(AB_MAX_ITEMS);
	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		m_lstOnWithdraw.at(i).Load(pMsg);
	}

	m_OnUseCommand.Load(pMsg);
	m_InvalidPSetCommand.Load(pMsg);

	m_ActivateTypeHandler.Load(pMsg);

	if (GetNumItems())
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
	}
}

// RETAIL: 0x1000de10 (with 0x1000ed46): reads the model template, the
// permission sets, the flags and the six items' properties, one property
// at a time
LTBOOL ArchiveBin::ReadProp(ObjectCreateStruct *pStruct)
{
	ArchiveItem itemUnused;		// RETAIL: an item that the function does not use
	GenericProp genProp;

	if (!pStruct) return LTFALSE;
	if (!g_pModelButeMgr) return LTFALSE;

	if (g_pLTServer->GetPropGeneric(s_szModelTemplateProp, &genProp) == LT_OK && genProp.m_String[0])
	{
		ModelId eModelId = g_pModelButeMgr->GetModelId(genProp.m_String);
		if (eModelId < g_pModelButeMgr->GetNumModels())
		{
			SAFE_STRCPY(pStruct->m_Filename, g_pModelButeMgr->GetMultiModelFilename(eModelId));
			g_pModelButeMgr->CopySkinFilenames(eModelId, 0, pStruct->m_SkinNames[0], ARRAY_LEN(pStruct->m_SkinNames[0]));
			g_pModelButeMgr->CopyRenderStyleFilenames(eModelId, pStruct);

			m_bTronUnknown171 = pStruct->m_nRenderGroup;

			if (g_pModelButeMgr->IsModelTranslucent(eModelId))
			{
				pStruct->m_Flags2 |= FLAG2_FORCETRANSLUCENT;
			}
		}
	}

	pStruct->m_Flags |= FLAG_MODELKEYS | FLAG_SOLID | FLAG_RAYHIT;

	HANDLE_READ_PSETS_PROPS(m_nPSets);

	if (g_pLTServer->GetPropGeneric("MoveToFloor", &genProp) == LT_OK)
	{
		m_bMoveToFloor = genProp.m_Bool;
	}

	if (g_pLTServer->GetPropGeneric("Radius", &genProp) == LT_OK)
	{
		m_fRadius = genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("StartCompressed", &genProp) == LT_OK)
	{
		m_bStartCompressed = genProp.m_Bool;
	}

	// RETAIL BUG: the buffer is 32 bytes, but the calls below let _snprintf
	// write up to 100 (docs/RETAIL_BUGS.md, as CCharacter::ReadProp)
#ifdef TRON20SRC_MODERN
	char szProp[100];
#else
	char szProp[32];
#endif
	int i;
	uint32 nNoneLen = strlen("<none>");
	uint8 j;

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		_snprintf(szProp, 100, "%s%d", "Type", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0])
		{
			for (j = 0; j < s_nNumArchiveItemTypes; j++)
			{
				if (!strcmp(genProp.m_String, s_aszArchiveItemTypes[j]))
					m_aItems[i].m_eType = j;
			}
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_Subroutine) continue;

		_snprintf(szProp, 100, "%s%d", "SubroutineName", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0] &&
			strncmp(genProp.m_String, "<none>", nNoneLen))
		{
			m_aItems[i].m_sName = genProp.m_String;
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_Subroutine) continue;

		_snprintf(szProp, 100, "%s%d", "SubroutineBuild", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0] &&
			strncmp(genProp.m_String, "<none>", nNoneLen))
		{
			for (j = 0; j < s_nNumArchiveItemBuilds; j++)
			{
				if (!strcmp(genProp.m_String, s_aszArchiveItemBuilds[j]))
					m_aItems[i].m_eBuild = j;
			}
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_Subroutine) continue;

		_snprintf(szProp, 100, "%s%d", "SubroutineCondition", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0] &&
			strncmp(genProp.m_String, "<none>", nNoneLen))
		{
			for (j = 0; j < s_nNumArchiveItemConditions; j++)
			{
				if (!strcmp(genProp.m_String, s_aszArchiveItemConditions[j]))
					m_aItems[i].m_eCondition = j;
			}
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_Procedural) continue;

		_snprintf(szProp, 100, "%s%d", "ProceduralName", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0] &&
			strncmp(genProp.m_String, "<none>", nNoneLen))
		{
			m_aItems[i].m_sName = genProp.m_String;
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_Primitive) continue;

		_snprintf(szProp, 100, "%s%d", "PrimitiveName", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0] &&
			strncmp(genProp.m_String, "<none>", nNoneLen))
		{
			m_aItems[i].m_sName = genProp.m_String;
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_PermissionSet) continue;

		_snprintf(szProp, 100, "%s%d", "PSetNumber", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_aItems[i].m_nPSet = (uint8)genProp.m_Long;
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_Email) continue;

		_snprintf(szProp, 100, "%s%d", "EmailName", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0] &&
			strncmp(genProp.m_String, "<none>", nNoneLen))
		{
			m_aItems[i].m_sName = genProp.m_String;
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_HelpFile) continue;

		_snprintf(szProp, 100, "%s%d", "HelpFile", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0] &&
			strncmp(genProp.m_String, "<none>", nNoneLen))
		{
			m_aItems[i].m_sName = genProp.m_String;
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_Custom) continue;

		_snprintf(szProp, 100, "%s%d", "CustomCmdName", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0] &&
			strncmp(genProp.m_String, "<none>", nNoneLen))
		{
			m_aItems[i].m_sName = genProp.m_String;
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (m_aItems[i].m_eType != kAIT_Custom) continue;

		_snprintf(szProp, 100, "%s%d", "CustomCmd", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK && genProp.m_String[0])
		{
			m_aItems[i].m_sCommand = genProp.m_String;
		}
	}

	for (i = 0; i < AB_MAX_ITEMS; i++)
	{
		_snprintf(szProp, 100, "%s%d", "Cost", i + 1);
		if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_aItems[i].m_fCost = genProp.m_Float;
		}
	}

	m_lstOnWithdraw.resize(AB_MAX_ITEMS);

	for (uint8 iCmd = 0; iCmd < AB_MAX_ITEMS; iCmd++)
	{
		_snprintf(szProp, 100, "%s%d", "OnWithdraw", iCmd + 1);
		m_lstOnWithdraw.at(iCmd).ReadProp(pStruct, szProp);
	}

	if (!GetNumItems())
	{
		g_pLTServer->CPrint("%s : Archive bin is empty on initial update.", pStruct->m_Name);
	}

	m_OnUseCommand.ReadProp(pStruct, "OnUseCmd");
	m_InvalidPSetCommand.ReadProp(pStruct, "InvalidPSetCommand");

	m_ActivateTypeHandler.SetActivateType("Open Archivebin");
	m_ActivateTypeHandler.SetState(ACTIVATETYPE::eOn, true, LTNULL);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::OnTrigger
//
//	PURPOSE:	Handle trigger messages (RETAIL: 0x1000f100)
//
// ----------------------------------------------------------------------- //

bool ArchiveBin::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_AddPSet("ADDPSET");
	static CParsedMsg::CToken s_cTok_RemovePSet("REMOVEPSET");

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_User, dwFlags);
		if (dwFlags & USRFLG_CAN_ACTIVATE)
		{
			CTronPlayerObj* pPlayer = dynamic_cast<CTronPlayerObj*>(g_pLTServer->HandleToObject(hSender));
			if (pPlayer && !pPlayer->IsDead())
			{
				if (!HasPSets(pPlayer, m_nPSets))
				{
					HOBJECT hObj = m_hObject;
					m_InvalidPSetCommand.Execute(hObj, pPlayer->GetUserObject());

					g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "ArchiveBinInvalidPSet");
					return true;
				}

				HOBJECT hObj = m_hObject;
				m_OnUseCommand.Execute(hObj, pPlayer->GetUserObject());

				if (pPlayer->GetClient())
				{
					m_hPlayer = hSender;
					g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
					ShowContents();
				}

				return true;
			}
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_AddPSet && cMsg.GetArgCount() > 1)
	{
		const char* pPSets = cMsg.GetArg(1);
		int nLen = strlen(pPSets);
		if (nLen > 8) nLen = 8;

		for (int i = 0; i < nLen; i++)
		{
			if (pPSets[i] >= '1' && pPSets[i] <= '8')
			{
				m_nPSets |= 1 << (pPSets[i] - '1');
			}
		}

		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_RemovePSet && cMsg.GetArgCount() > 1)
	{
		const char* pPSets = cMsg.GetArg(1);
		int nLen = strlen(pPSets);
		if (nLen > 8) nLen = 8;

		for (int i = 0; i < nLen; i++)
		{
			if (pPSets[i] >= '1' && pPSets[i] <= '8')
			{
				m_nPSets &= ~(1 << (pPSets[i] - '1'));
			}
		}

		return true;
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::ShowContents
//
//	PURPOSE:	Open the decision window on the clients with our items, or
//				tell the player we are empty (RETAIL: 0x1000f520)
//
// ----------------------------------------------------------------------- //

void ArchiveBin::ShowContents()
{
	CAutoMessage cMsg;

	bool bScanShowsNormal = false;
	bool bScanShowsGuess = false;

	if (!g_pServerSubroutineMgr)
	{
		g_ServerSubroutineMgr.Init(SSMGR_DEFAULT_FILE);
	}

	cMsg.Writeuint8(MID_DECISION);
	cMsg.Writeuint8(DECISION_TYPE_ARCHIVEBIN);
	cMsg.WriteObject(m_hObject);
	cMsg.Writefloat(m_fRadius);
	cMsg.Writeuint8(1);
	cMsg.Writeuint8(2);

	// A virus scan may tell what the subroutines' conditions are
	CTronPlayerObj* pPlayer = dynamic_cast<CTronPlayerObj*>(g_pLTServer->HandleToObject(m_hPlayer));
	if (pPlayer)
	{
		PlayerSubroutineSlot* pSlot = pPlayer->FindSubroutine("virus_scan");
		if (pSlot && pSlot->nTronUnknown40 && pSlot->nTronUnknown41)
		{
			SubroutineDataKey keyA(0, pSlot->nTronUnknown41, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0);
			if (GetRandom(0.0f, 1.0f) < g_pSubroutineDataMgr->GetVirusScanData(keyA).fFailureChance)
			{
				bScanShowsNormal = true;
			}

			SubroutineDataKey keyB(1, pSlot->nTronUnknown41, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0);
			if (GetRandom(0.0f, 1.0f) < g_pSubroutineDataMgr->GetVirusScanData(keyB).fFailureChance)
			{
				bScanShowsGuess = true;
			}
		}
	}

	cMsg.Writebool(bScanShowsNormal);

	bool bEmpty = true;
	uint32 nNameId;

	for (int i = 0; i < AB_MAX_ITEMS; i++)
	{
		ArchiveItem& item = m_aItems[i];

		bool bItem = (item.m_eType != kAIT_None);
		if (bItem)
		{
			switch (item.m_eType)
			{
				case kAIT_Subroutine:
				{
					nNameId = g_pServerSubroutineMgr->GetNameId(item.m_sName.c_str(), kSST_Subroutine);
					if (!nNameId)
					{
						ObjectCPrint(m_hObject, "Item %d has a type but no valid name.  Items must be named to exist.", i);
						bItem = false;
					}
				}
				break;

				case kAIT_Email:
				case kAIT_HelpFile:
					nNameId = g_pInventoryButeMgr->GetItemTextID(g_pInventoryButeMgr->GetItemIndex(item.m_sName.c_str()));
					break;

				case kAIT_Procedural:
					nNameId = g_pServerSubroutineMgr->GetNameId(item.m_sName.c_str(), kSST_Procedural);
					break;

				case kAIT_Primitive:
					nNameId = g_pServerSubroutineMgr->GetNameId(item.m_sName.c_str(), kSST_Primitive);
					break;

				default:
					nNameId = atoi(item.m_sName.c_str());
					break;
			}
		}

		cMsg.Writebool(bItem);
		if (!bItem) continue;

		bEmpty = false;

		cMsg.Writeuint32(nNameId);
		cMsg.Writeint8(item.m_eType);

		if (item.m_eType == kAIT_Subroutine)
		{
			cMsg.Writeint8(item.m_eBuild);

			if (bScanShowsNormal)
			{
				if (item.m_eCondition == kAIC_Infected)
				{
					cMsg.Writeint8(kAIC_Normal);
				}
				else
				{
					cMsg.Writeint8(item.m_eCondition);
				}
			}
			else if (bScanShowsGuess)
			{
				cMsg.Writeint8(m_aeShownCondition[i]);
			}
			else
			{
				cMsg.Writeint8(item.m_eCondition);
			}
		}
		else if (item.m_eType == kAIT_PermissionSet)
		{
			cMsg.Writeint8(item.m_nPSet);
		}

		cMsg.Writefloat(item.m_fCost);
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (bEmpty)
	{
		g_pServerSoundMgr->PlaySoundFromPos(vPos, "Empty_archloop_01", 600.0f, SOUNDPRIORITY_MISC_HIGH);
	}
	else
	{
		g_pServerSoundMgr->PlaySoundFromPos(vPos, "Interface\\snd\\archive_up.wav", 600.0f, SOUNDPRIORITY_MISC_HIGH);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::HandleChoice
//
//	PURPOSE:	The player withdrew an item or closed the window
//				(RETAIL: 0x1000fa00)
//
// ----------------------------------------------------------------------- //

void ArchiveBin::HandleChoice(uint8 nChoice)
{
	char szMsg[256];

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

	switch (nChoice)
	{
		case AB_CHOICE_DESTROY:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, GetNumItems() ? USRFLG_CAN_ACTIVATE : 0, USRFLG_CAN_ACTIVATE);
		}
		break;

		case AB_CHOICE_CANCEL:
		case AB_CHOICE_CANCEL + 1:
		case AB_CHOICE_CANCEL + 2:
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pServerSoundMgr->PlaySoundFromPos(vPos, "Interface\\snd\\archive_dn.wav");
		}
		break;

		default:
		{
			if (nChoice >= AB_MAX_ITEMS || !m_hPlayer) break;

			CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->HandleToObject(m_hPlayer);
			if (!pPlayer) break;

			CDestructible* pDest = pPlayer->GetDestructible();
			if (pDest->GetEnergy() < m_aItems[nChoice].m_fCost) break;
			pDest->SetEnergy(pDest->GetEnergy() - m_aItems[nChoice].m_fCost);

			// A virus scan may catch an infection
			bool bScanned = false;
			PlayerSubroutineSlot* pSlot = pPlayer->FindSubroutine("virus_scan");
			if (pSlot && pSlot->nTronUnknown40 && pSlot->nTronUnknown41)
			{
				SubroutineDataKey key(0, pSlot->nTronUnknown41, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0);
				if (GetRandom(0.0f, 1.0f) < g_pSubroutineDataMgr->GetVirusScanData(key).fChanceToCorrupt)
				{
					bScanned = true;
				}
			}

			// RETAIL: a Custom item sends szMsg as it is, uninitialized (as
			// CoreDump's; not listed in docs/RETAIL_BUGS.md while this is
			// near)
			if (m_aItems[nChoice].m_eType == kAIT_Subroutine)
			{
				int eCondition = m_aItems[nChoice].m_eCondition;
				if (bScanned && eCondition != kAIC_Foreign)
				{
					eCondition = kAIC_Infected;
				}

				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %s %s %s", "ACQUIRESUB", m_aItems[nChoice].m_sName.c_str(),
					s_aszArchiveItemBuilds[m_aItems[nChoice].m_eBuild], s_aszArchiveItemConditions[eCondition]);
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_Procedural)
			{
				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %s", "ACQUIREPROC", m_aItems[nChoice].m_sName.c_str());
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_Primitive)
			{
				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %s", "ACQUIREPRIM", m_aItems[nChoice].m_sName.c_str());
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_PermissionSet)
			{
				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %d", "ACQUIREPSET", m_aItems[nChoice].m_nPSet);
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_Email || m_aItems[nChoice].m_eType == kAIT_HelpFile)
			{
				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %s", "ACQUIRENOTE", m_aItems[nChoice].m_sName.c_str());
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_Custom)
			{
				if (g_pCmdMgr->IsValidCmd(m_aItems[nChoice].m_sCommand.c_str()))
				{
					g_pCmdMgr->Process(m_aItems[nChoice].m_sCommand.c_str(), m_hObject, m_hObject);
				}
			}

			// Take down the first berry of the item's type
			uint32 iBerry;
			for (iBerry = 0; iBerry < AB_MAX_BERRIES; iBerry++)
			{
				if (m_aBerries[iBerry].m_hObject && m_aBerries[iBerry].m_eItemType == m_aItems[nChoice].m_eType)
				{
					break;
				}
			}

			if (iBerry < AB_MAX_BERRIES)
			{
				BerryRef& berry = m_aBerries[iBerry];

				HATTACHMENT hAttachment;
				g_pLTServer->FindAttachment(m_hObject, berry.m_hObject, &hAttachment);
				g_pLTServer->RemoveAttachment(hAttachment);

				if (berry.m_hObject)
				{
					g_pLTServer->RemoveObject(berry.m_hObject);
					berry.m_hObject = LTNULL;
				}

				berry.m_eItemType = -1;
			}

			m_aItems[nChoice].Clear();

			if (!GetNumItems())
			{
				g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
				m_ActivateTypeHandler.SetState(ACTIVATETYPE::eTronUnknown2, true, LTNULL);
			}

			SendTriggerMsgToObject(this, m_hPlayer, LTFALSE, szMsg);

			// The item's command runs on its first withdrawal only
			m_lstOnWithdraw.at(nChoice).Execute(m_hObject, m_hPlayer);
		}
		break;
	}
}

// RETAIL: 0x1000fef0 (with 0x10010000): the "ArchiveBin_Frame" model on
// the "Frame" socket (kept at 0x3a4)
void ArchiveBin::CreateFrame()
{
	ObjectCreateStruct ocs;
	LTransform tf;
	INIT_OBJECTCREATESTRUCT(ocs);

	HMODELSOCKET hSocket;
	if (g_pModelLT->GetSocket(m_hObject, "Frame", hSocket) == LT_OK)
	{
		g_pModelLT->GetSocketTransform(m_hObject, hSocket, tf, LTTRUE);

		SAFE_STRCPY(ocs.m_Name, s_szFrameModel);

		ModelId eModelId = g_pModelButeMgr->GetModelId(s_szFrameModel);
		if (eModelId < g_pModelButeMgr->GetNumModels())
		{
			SAFE_STRCPY(ocs.m_Filename, g_pModelButeMgr->GetMultiModelFilename(eModelId));
			g_pModelButeMgr->CopySkinFilenames(eModelId, 0, ocs.m_SkinNames[0], ARRAY_LEN(ocs.m_SkinNames[0]));
			g_pModelButeMgr->CopyRenderStyleFilenames(eModelId, &ocs);
		}

		ocs.m_Pos = tf.m_Pos;
		ocs.m_Flags = FLAG_VISIBLE;
		ocs.m_ObjectType = OT_MODEL;
		ocs.m_nRenderGroup = m_bTronUnknown171;

		if (g_pModelButeMgr->IsModelTranslucent(eModelId))
		{
			ocs.m_Flags2 = FLAG2_FORCETRANSLUCENT;
		}

		HCLASS hClass = g_pLTServer->GetClass("BaseClass");
		LPBASECLASS pFrame = g_pLTServer->CreateObject(hClass, &ocs);
		if (pFrame)
		{
			m_hTronUnknown3a4 = pFrame->m_hObject;

			if (m_hTronUnknown3a4)
			{
				g_pLTServer->SetNextUpdate(m_hTronUnknown3a4, 0.0f);
				g_pLTServer->SetObjectState(m_hTronUnknown3a4, OBJSTATE_INACTIVE);
			}

			LTVector vOffset(0.0f, 0.0f, 0.0f);
			LTRotation rOffset;
			HATTACHMENT hAttachment;
			g_pLTServer->CreateAttachment(m_hObject, pFrame->m_hObject, "Frame", &vOffset, &rOffset, &hAttachment);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::AssignBerries
//
//	PURPOSE:	Give each item a berry socket, at random (RETAIL: 0x100102c0;
//				the name is ours). The last socket is the frame's.
//
// ----------------------------------------------------------------------- //

void ArchiveBin::AssignBerries()
{
	uint32 nSockets = 0;
	g_pModelLT->GetNumSockets(m_hObject, nSockets);
	nSockets--;

	BerryRef* apBerries[AB_MAX_BERRIES];
	for (uint32 iBerry = 0; iBerry < AB_MAX_BERRIES; iBerry++)
	{
		apBerries[iBerry] = (iBerry < nSockets) ? &m_aBerries[iBerry] : LTNULL;
	}

	std::random_shuffle(apBerries, apBerries + nSockets);

	for (uint32 i = 0; i < AB_MAX_ITEMS; i++)
	{
		if (i < nSockets)
		{
			apBerries[i]->m_eItemType = m_aItems[i].m_eType;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::CreateBerries
//
//	PURPOSE:	Put a berry of its item's type on each assigned socket
//				(RETAIL: 0x10010390)
//
// ----------------------------------------------------------------------- //

void ArchiveBin::CreateBerries()
{
	AssignBerries();

	for (uint32 i = 0; i < AB_MAX_BERRIES; i++)
	{
		if (m_aBerries[i].m_eItemType == -1) continue;

		ObjectCreateStruct ocs;
		LTransform tf;
		INIT_OBJECTCREATESTRUCT(ocs);

		g_pModelLT->GetSocketTransform(m_hObject, i, tf, LTTRUE);

		char szName[256];
		_snprintf(szName, ARRAY_LEN(szName), "%s%d", s_szBerryClass, i + 1);
		SAFE_STRCPY(ocs.m_Name, szName);

		ModelId eModelId;
		switch (m_aBerries[i].m_eItemType)
		{
			case kAIT_Subroutine:
				eModelId = g_pModelButeMgr->GetModelId(s_szSubroutineBerryModel);
				break;

			case kAIT_PermissionSet:
				eModelId = g_pModelButeMgr->GetModelId(s_szPSetBerryModel);
				break;

			default:
				eModelId = g_pModelButeMgr->GetModelId(s_szDocumentBerryModel);
				break;
		}
		if (eModelId < g_pModelButeMgr->GetNumModels())
		{
			SAFE_STRCPY(ocs.m_Filename, g_pModelButeMgr->GetModelFilename(eModelId));
			g_pModelButeMgr->CopySkinFilenames(eModelId, 0, ocs.m_SkinNames[0], ARRAY_LEN(ocs.m_SkinNames[0]));
			g_pModelButeMgr->CopyRenderStyleFilenames(eModelId, &ocs);
		}

		ocs.m_Pos = tf.m_Pos;
		ocs.m_Flags = FLAG_VISIBLE;
		ocs.m_ObjectType = OT_MODEL;
		ocs.m_nRenderGroup = m_bTronUnknown171;

		if (g_pModelButeMgr->IsModelTranslucent(eModelId))
		{
			ocs.m_Flags2 = FLAG2_FORCETRANSLUCENT;
		}

		HCLASS hClass = g_pLTServer->GetClass(s_szBerryClass);
		LPBASECLASS pBerry = g_pLTServer->CreateObject(hClass, &ocs);
		if (!pBerry) continue;

		m_aBerries[i].m_hObject = pBerry->m_hObject;

		HOBJECT hBerry = pBerry->m_hObject;
		if (hBerry)
		{
			g_pLTServer->SetNextUpdate(hBerry, 0.0f);
			g_pLTServer->SetObjectState(hBerry, OBJSTATE_INACTIVE);
		}

		LTVector vOffset(0.0f, 0.0f, 0.0f);
		LTRotation rOffset;
		_snprintf(szName, ARRAY_LEN(szName), "%s%d", s_szSocket, i + 1);

		HATTACHMENT hAttachment;
		g_pLTServer->CreateAttachment(m_hObject, pBerry->m_hObject, szName, &vOffset, &rOffset, &hAttachment);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::HandleModelString
//
//	PURPOSE:	Handle the model's sound keys (RETAIL: as IONode's; PORT: Tron's PlaySoundFromObject
//				takes one more argument)
//
// ----------------------------------------------------------------------- //

void ArchiveBin::HandleModelString(ArgList* pArgList)
{
	static CParsedMsg::CToken s_cTok_KEY_BUTE_SOUND("BUTE_SOUND_KEY");
	static CParsedMsg::CToken s_cTok_KEY_BUTE_SOUND_LOOPING("BUTE_SOUND_KEY_LOOPING");
	static CParsedMsg::CToken s_cTok_KILL_LOOPING_SOUND("KILL_LOOPING_SOUND");

	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	CParsedMsg::CToken tok( pKey );

	if (tok == s_cTok_KEY_BUTE_SOUND)
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pArgList->argv[1]);
		}
	}
	else if (tok == s_cTok_KEY_BUTE_SOUND_LOOPING)
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			if (m_hstrLoopSound)
			{
				g_pLTServer->FreeString(m_hstrLoopSound);
				m_hstrLoopSound = LTNULL;
			}

			m_hstrLoopSound = g_pLTServer->CreateString(pArgList->argv[1]);
			PlayLoopSound();
		}
	}
	else if (tok == s_cTok_KILL_LOOPING_SOUND)
	{
		bool bLoop = false;
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			if (atoi(pArgList->argv[1]) == 1)
			{
				bLoop = true;
			}
		}

		if (m_hLoopSound)
		{
			if (bLoop)
			{
				g_pLTServer->SoundMgr()->KillSoundLoop(m_hLoopSound);
			}
			else
			{
				g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
			}

			m_hLoopSound = LTNULL;
		}

		if (m_hstrLoopSound)
		{
			g_pLTServer->FreeString(m_hstrLoopSound);
			m_hstrLoopSound = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::PlayLoopSound
//
//	PURPOSE:	(Re)start the looping sound
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArchiveBin::PlayLoopSound
//
//	PURPOSE:	(Re)start the looping sound (RETAIL: as IONode's; PORT: Tron's PlaySoundFromObject
//				takes one more argument)
//
// ----------------------------------------------------------------------- //

void ArchiveBin::PlayLoopSound()
{
	if (!m_hstrLoopSound) return;

	if (m_hLoopSound)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
	}

	m_hLoopSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, g_pLTServer->GetStringData(m_hstrLoopSound), -1.0f,
		SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_ATTACHED | PLAYSOUND_GETHANDLE);
	// RETAIL: PLAYSOUND_ATTACHED, not PLAYSOUND_LOOP (retail pushes 0x18), so
	// the "loop" sound plays once (docs/RETAIL_BUGS.md)
}
