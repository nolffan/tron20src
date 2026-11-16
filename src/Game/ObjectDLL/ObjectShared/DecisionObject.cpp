// ----------------------------------------------------------------------- //
//
// MODULE  : DecisionObject.cpp
//
// PURPOSE : DecisionObject - Implementation
//
// CREATED : 7/19/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DecisionObject.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "MsgIDs.h"

LINKFROM_MODULE( DecisionObject );

#pragma force_active on
BEGIN_CLASS(DecisionObject)
	// RETAIL: Tron's properties (0x100bfd80)
	ADD_LONGINTPROP_FLAG(WindowStyle, 0, 0)
	ADD_REALPROP_FLAG(Radius, 300.0f, PF_RADIUS)
	PROP_DEFINEGROUP(Choice1, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(Choice1StringID, 0, PF_GROUP(1))
		ADD_STRINGPROP_FLAG(Choice1String, LTNULL, PF_GROUP(1) | PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Choice1Cmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice2, PF_GROUP(2))
		ADD_LONGINTPROP_FLAG(Choice2StringID, 0, PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Choice2String, LTNULL, PF_GROUP(2) | PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Choice2Cmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice3, PF_GROUP(3))
		ADD_LONGINTPROP_FLAG(Choice3StringID, 0, PF_GROUP(3))
		ADD_STRINGPROP_FLAG(Choice3String, LTNULL, PF_GROUP(3) | PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Choice3Cmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice4, PF_GROUP(4))
		ADD_LONGINTPROP_FLAG(Choice4StringID, 0, PF_GROUP(4))
		ADD_STRINGPROP_FLAG(Choice4String, LTNULL, PF_GROUP(4) | PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Choice4Cmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice5, PF_GROUP(5))
		ADD_LONGINTPROP_FLAG(Choice5StringID, 0, PF_GROUP(5))
		ADD_STRINGPROP_FLAG(Choice5String, LTNULL, PF_GROUP(5) | PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Choice5Cmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice6, PF_GROUP(6))
		ADD_LONGINTPROP_FLAG(Choice6StringID, 0, PF_GROUP(6))
		ADD_STRINGPROP_FLAG(Choice6String, LTNULL, PF_GROUP(6) | PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Choice6Cmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(AbortCmd, "", PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(PlayerFiredAbortCmd, "", PF_NOTIFYCHANGE)
    ADD_BOOLPROP(RemoveAfterChoice, LTTRUE)
    ADD_BOOLPROP(Escapable, LTTRUE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(DecisionObject, BaseClass, NULL, NULL, 0, CDecisionObjectPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( DecisionObject )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( ON, 2, NULL, "ON <Radius Center>" )	// RETAIL: Tron's (0x100c08ba)
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( START, 1, NULL, "START" )
	CMDMGR_ADD_MSG( STOP, 1, NULL, "STOP" )
	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )

CMDMGR_END_REGISTER_CLASS( DecisionObject, BaseClass )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDecisionObjectPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CDecisionObjectPlugin::PreHook_PropChanged( const char *szObjName,
													   const char *szPropName, 
													   const int  nPropType, 
													   const GenericProp &gpPropValue,
													   ILTPreInterface *pInterface,
													   const char	*szModifiers)
{
	// Only our commands are marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
												szPropName, 
												nPropType, 
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::DecisionObject()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DecisionObject::DecisionObject() : GameBase(OT_NORMAL)
{
	m_hstrAbortCmd = LTNULL;
	m_hstrPlayerFiredAbortCmd = LTNULL;
	m_bVisible = false;
	m_bLock = false;
	m_bRemoveAfterChoice = true;
	m_fRadius = 300.0f;
	m_bStartOn = false;
	m_bEscapable = true;
	m_nWindowStyle = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::~DecisionObject()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

DecisionObject::~DecisionObject()
{
	FREE_HSTRING(m_hstrAbortCmd);
	FREE_HSTRING(m_hstrPlayerFiredAbortCmd);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 DecisionObject::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE;
			}

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool DecisionObject::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Start("START");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Stop("STOP");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Lock("LOCK");
	static CParsedMsg::CToken s_cTok_Unlock("UNLOCK");

	if ((cMsg.GetArg(0) == s_cTok_Start) ||
		(cMsg.GetArg(0) == s_cTok_On)
		)
	{
		// RETAIL: Tron's takes the name of the object to show the window to
		if (cMsg.GetArgCount() == 2)
		{
			HandleShow(cMsg.GetArg(1));
		}
		else
		{
			HandleShow(LTNULL);
		}
	}
	else if ((cMsg.GetArg(0) == s_cTok_Stop) ||
		(cMsg.GetArg(0) == s_cTok_Off)
		)
	{
		HandleAbort(true);
	}
	else if( cMsg.GetArg(0) == s_cTok_Lock )
	{
		m_bLock = true;
	}
	else if( cMsg.GetArg(0) == s_cTok_Unlock )
	{
		m_bLock = false;
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::ObjectMessageFn
//
//	PURPOSE:	RETAIL: Tron's (0x100c0d80): the choice a client made
//
// ----------------------------------------------------------------------- //

uint32 DecisionObject::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	if (messageID != MID_DECISION)
	{
		return GameBase::ObjectMessageFn(hSender, pMsg);
	}

	HandleChoose(pMsg->Readuint8());
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

void DecisionObject::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;
	char szProp[128];

	for (int i=1; i <= MAX_DECISION_CHOICES; i++)
	{
		sprintf(szProp, "Choice%dStringID", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_ChoiceData[i-1].nStringID = genProp.m_Long;
		}

		sprintf(szProp, "Choice%dString", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
                m_ChoiceData[i-1].hstrString = g_pLTServer->CreateString(genProp.m_String);
			}
		}

		sprintf(szProp, "Choice%dCmd", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
                m_ChoiceData[i-1].hstrCmd = g_pLTServer->CreateString(genProp.m_String);
			}
		}
	}

    if (g_pLTServer->GetPropGeneric("WindowStyle", &genProp) == LT_OK)
	{
		m_nWindowStyle = (uint8)genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("AbortCmd", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrAbortCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerFiredAbortCmd", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPlayerFiredAbortCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("RemoveAfterChoice", &genProp ) == LT_OK)
	{
		m_bRemoveAfterChoice = ( genProp.m_Bool ? true : false);
	}

    if (g_pLTServer->GetPropGeneric("Escapable", &genProp ) == LT_OK)
	{
		m_bEscapable = ( genProp.m_Bool ? true : false);
	}

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp ) == LT_OK)
	{
		m_bStartOn = ( genProp.m_Bool ? true : false);
	}

    if (g_pLTServer->GetPropGeneric("Radius", &genProp ) == LT_OK)
	{
		m_fRadius = (float)genProp.m_Long;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::InitialUpdate()
//
//	PURPOSE:	First update (RETAIL: 0x100c13b0)
//
// ----------------------------------------------------------------------- //

void DecisionObject::InitialUpdate()
{
    g_pLTServer->SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::Update()
//
//	PURPOSE:	RETAIL: 0x100c13d0. Show a StartOn window once
//
// ----------------------------------------------------------------------- //

void DecisionObject::Update()
{
	if (m_bStartOn)
	{
		m_bStartOn = false;
		HandleShow(LTNULL);
	}

    g_pLTServer->SetNextUpdate(m_hObject, UPDATE_NEVER);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::HandleShow()
//
//	PURPOSE:	RETAIL: 0x100c1410. Show the window to the named object
//
// --------------------------------------------------------------------------- //

void DecisionObject::HandleShow(const char* pszTarget)
{
	if( m_bLock )
		return;

	HOBJECT hTarget;

	if (pszTarget && pszTarget[0])
	{
		ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects((char*)pszTarget, objArray);

		switch (objArray.NumObjects())
		{
			case 0:
			{
				hTarget = m_hObject;
			}
			break;

			case 1:
			{
				hTarget = objArray.GetObject(0);
			}
			break;

			default:
			{
                g_pLTServer->CPrint("Error, %d objects named \"%s\" present in level", objArray.NumObjects(), pszTarget);
				hTarget = m_hObject;
			}
			break;
		}
	}
	else
	{
		hTarget = m_hObject;
	}

	Show(true, false, hTarget);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::HandleChoose()
//
//	PURPOSE:	RETAIL: 0x100c14b0. Handle a choice
//
// --------------------------------------------------------------------------- //

void DecisionObject::HandleChoose(uint8 nChoice)
{
	if( m_bLock )
		return;

	switch (nChoice)
	{
		// RETAIL: Tron's: the player fired while the window was open
		case 0xFD:
		{
			HandleAbort(false);

			const char* pCmd = g_pLTServer->GetStringData(m_hstrPlayerFiredAbortCmd);
			if (g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd, m_hObject, (HOBJECT)LTNULL);
			}
		}
		return;

		//special case of closing a open decision window as another one is being opened
		// in this case do not notify the client
		case 0xFE:
		case 0xFF:
		{
			HandleAbort(false);
		}
		return;
	}

	if (nChoice < MAX_DECISION_CHOICES && m_ChoiceData[nChoice].hstrCmd)
	{
		const char* pCmd = g_pLTServer->GetStringData(m_ChoiceData[nChoice].hstrCmd);
		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
		}
	}
	else
	{
		HandleAbort(false);
		return;
	}

	if (m_bRemoveAfterChoice)
		g_pLTServer->RemoveObject(m_hObject);

	// Send message to clients telling them about the DecisionObject...
	Show(false);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::HandleAbort()
//
//	PURPOSE:	RETAIL: 0x100c15f0. Handle the DecisionObject ending
//
// ----------------------------------------------------------------------- //

void DecisionObject::HandleAbort(bool bNotifyClient)
{
	if( m_bLock )
		return;

	if (m_bStartOn)
	{
		// RETAIL: a StartOn window that is not shown yet stays hidden
		m_bStartOn = false;
	}
	else if (bNotifyClient)
	{
		Show(false);
	}
	else
	{
		m_bVisible = false;
	}

	const char* pCmd = g_pLTServer->GetStringData(m_hstrAbortCmd);
	if (g_pCmdMgr->IsValidCmd(pCmd))
	{
		g_pCmdMgr->Process(pCmd, m_hObject, (HOBJECT)LTNULL);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::Show()
//
//	PURPOSE:	RETAIL: 0x100c16a0. Tell the clients about the window
//
// --------------------------------------------------------------------------- //

void DecisionObject::Show(bool bShow, bool bForceShow, HOBJECT hTarget)
{
	if( m_bLock )
		return;

	// Send message to clients telling them about the DecisionObject...

	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_DECISION);
	cMsg.Writeuint8(2);		// RETAIL: Tron's; what the 2 means is not known

	m_bVisible = bShow;

	if (bShow)
	{
		cMsg.Writeuint8(LTTRUE);
		cMsg.Writeuint8(m_nWindowStyle);
		cMsg.Writeuint8( (bForceShow ? LTTRUE : LTFALSE) );
		for (int i=0; i < MAX_DECISION_CHOICES; i++)
		{
			cMsg.Writeuint8(m_ChoiceData[i].hstrString != LTNULL);
			if (m_ChoiceData[i].hstrString)
			{
				cMsg.WriteHString(m_ChoiceData[i].hstrString);
			}
			else
			{
				cMsg.Writeuint32(m_ChoiceData[i].nStringID);
			}
		}
		cMsg.WriteObject(m_hObject);
		cMsg.WriteObject(hTarget ? hTarget : (HOBJECT)m_hObject);
		cMsg.Writefloat(m_fRadius);
		cMsg.Writebool(m_bEscapable);
	}
	else
	{
		cMsg.Writeuint8(LTFALSE);
		cMsg.WriteObject(m_hObject);
	}
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DecisionObject::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	for (int i=0; i < MAX_DECISION_CHOICES; i++)
	{
		m_ChoiceData[i].Save(pMsg);
	}

	SAVE_HSTRING(m_hstrAbortCmd);
	SAVE_HSTRING(m_hstrPlayerFiredAbortCmd);
	SAVE_bool(m_bVisible);
	SAVE_bool(m_bRemoveAfterChoice);
	SAVE_bool(m_bStartOn);
	SAVE_FLOAT(m_fRadius);
	SAVE_bool( m_bLock );
	SAVE_bool( m_bEscapable );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DecisionObject::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	for (int i=0; i < MAX_DECISION_CHOICES; i++)
	{
		m_ChoiceData[i].Load(pMsg);
	}

	LOAD_HSTRING(m_hstrAbortCmd);
	LOAD_HSTRING(m_hstrPlayerFiredAbortCmd);
	LOAD_bool(m_bVisible);
	LOAD_bool(m_bRemoveAfterChoice);
	LOAD_bool(m_bStartOn);
	LOAD_FLOAT(m_fRadius);
	LOAD_bool( m_bLock );
	LOAD_bool( m_bEscapable );

	if (m_bVisible)
		Show(true,true);
}
