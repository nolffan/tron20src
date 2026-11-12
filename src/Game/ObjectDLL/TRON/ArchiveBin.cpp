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

extern CInventoryButeMgr s_InventoryButeMgr;

// RETAIL: a user flag the bin sets on itself, as an I/O node does
// (IONode.h's USRFLG_IONODE); the name is ours
#define USRFLG_ARCHIVEBIN		(1<<1)

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

CMDMGR_BEGIN_REGISTER_CLASS( ArchiveBin )

	CMDMGR_ADD_MSG( SELECT, 1, NULL, "SELECT" )
	CMDMGR_ADD_MSG( DECISIONABORTED, 0, NULL, "DECISIONABORTED" )

CMDMGR_END_REGISTER_CLASS( ArchiveBin, GameBase )

// RETAIL: in .rdata (const arrays), unlike the other strings
static const char s_szModelTemplateProp[] = "ModelTemplate";
static const char s_szArchiveBinModel[] = "ArchiveBin";

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
	m_hFrame			( LTNULL ),
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

			m_aBerries[i].m_nItem = -1;
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

	SAVE_HOBJECT(m_hFrame);
	SAVE_BYTE(m_nPSets);
	SAVE_bool(m_bMoveToFloor);
	SAVE_bool(m_bFirstUpdate);
	SAVE_FLOAT(m_fRadius);
	SAVE_bool(m_bStartCompressed);

	for (i = 0; i < AB_MAX_BERRIES; i++)
	{
		SAVE_HOBJECT(m_aBerries[i].m_hObject);
		SAVE_BYTE(m_aBerries[i].m_nItem);
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

	LOAD_HOBJECT(m_hFrame);
	LOAD_BYTE(m_nPSets);
	LOAD_bool(m_bMoveToFloor);
	LOAD_bool(m_bFirstUpdate);
	LOAD_FLOAT(m_fRadius);
	LOAD_bool(m_bStartCompressed);

	for (i = 0; i < AB_MAX_BERRIES; i++)
	{
		LOAD_HOBJECT(m_aBerries[i].m_hObject);
		LOAD_BYTE(m_aBerries[i].m_nItem);
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

// PORT: not written yet from retail; these stand in so the class links

// RETAIL: 0x1000de10 (with 0x1000ed46): reads the model template, the
// permission sets, the flags and the six items' properties
LTBOOL ArchiveBin::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return LTFALSE;

	HANDLE_READ_PSETS_PROPS(m_nPSets);

	m_lstOnWithdraw.resize(AB_MAX_ITEMS);
	m_OnUseCommand.ReadProp(pStruct, "OnUseCmd");
	m_InvalidPSetCommand.ReadProp(pStruct, "InvalidPSetCommand");

	return LTTRUE;
}

// RETAIL: 0x1000f100: SELECT and DECISIONABORTED
bool ArchiveBin::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	return GameBase::OnTrigger(hSender, cMsg);
}

// RETAIL: 0x1000f520: the decision window
void ArchiveBin::ShowContents()
{
}

// RETAIL: 0x1000fa00: withdraw an item
void ArchiveBin::HandleChoice(uint8 nChoice)
{
}

// RETAIL: 0x1000fef0 (with 0x10010000): the "ArchiveBin_Frame" model on
// the "Frame" socket
void ArchiveBin::CreateFrame()
{
}

// RETAIL: 0x10010390 (with 0x100102c0): a Berry on each "Berry%d" socket
void ArchiveBin::CreateBerries()
{
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
		SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
}
