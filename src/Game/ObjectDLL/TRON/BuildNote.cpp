// ----------------------------------------------------------------------- //
//
// MODULE  : BuildNote.cpp
//
// PURPOSE : A build note: picked up, it gives the player its build points
//
// RETAIL: Tron 2.0's, written from retail Object.lto (BuildNote.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BuildNote.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ServerUtilities.h"
#include "InventoryButeMgr.h"
#include "IObjectRelationMgrUser.h"
#include "PlayerObj.h"

extern CInventoryButeMgr s_InventoryButeMgr;

LTFLOAT g_fBuildNoteRadius = 10.0f;
LTFLOAT g_fBuildNoteHeight = 40.0f;

BEGIN_CLASS(BuildNote)
	ADD_STRINGPROP_FLAG(NoteName, "<none>", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(BuildNote, GameBase, NULL, NULL, 0, CBuildNotePlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildNote::BuildNote()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

BuildNote::BuildNote() : GameBase(OT_NORMAL)
{
	m_szNoteName[0] = '\0';
	m_hFX = LTNULL;
	m_bFromWorld = true;
	m_ActivateTypeHandler.Init( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildNote::~BuildNote()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

BuildNote::~BuildNote()
{
	if( m_hFX )
	{
		g_pLTServer->RemoveObject( m_hFX );
		m_hFX = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildNote::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 BuildNote::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				m_bFromWorld = true;
			}
			else
			{
				m_bFromWorld = false;
			}

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

			InitialUpdate();

			m_ActivateTypeHandler.SetActivateType( "Pickup Build Note" );
			m_ActivateTypeHandler.SetDisabled( false, false );
			m_ActivateTypeHandler.CreateActivateTypeMsg();
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
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildNote::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

void BuildNote::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("NoteName", &genProp) == LT_OK)
	{
		strcpy(m_szNoteName, genProp.m_String);
	}

	if (g_pInventoryButeMgr->GetItemIndex(m_szNoteName) == -1)
	{
		g_pLTServer->CPrint("ERROR: Could not find note with name (%s) in the bute file.", m_szNoteName);
	}

	pStruct->m_Flags |= FLAG_RAYHIT | FLAG_FORCECLIENTUPDATE;
	pStruct->m_Flags2 |= FLAG2_SERVERDIMS;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildNote::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void BuildNote::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_CHARSTRING(m_szNoteName);
	SAVE_HOBJECT(m_hFX);
	m_ActivateTypeHandler.Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildNote::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void BuildNote::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_CHARSTRING(m_szNoteName, BUILDNOTE_MAX_NAME_LEN);
	LOAD_HOBJECT(m_hFX);
	m_ActivateTypeHandler.Load( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildNote::InitialUpdate
//
//	PURPOSE:	Size the note, put it on the floor if it was placed in
//				the level, and start its fx
//
// ----------------------------------------------------------------------- //

void BuildNote::InitialUpdate()
{
	LTVector vDims;
	vDims.y = g_fBuildNoteHeight;
	vDims.x = vDims.z = g_fBuildNoteRadius;
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	if (!m_bFromWorld) return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To = vPos;
	IQuery.m_To.y -= 10000.0f;
	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		vPos.y -= (vPos.y - IInfo.m_Point.y) - (vDims.y + 0.1f);
		g_pLTServer->SetObjectPos(m_hObject, &vPos);
	}

	char szProps[1024];
	sprintf(szProps, "FxName %s;Loop 1;SmoothShutdown 0", "WORLDS_buildnote_base");
	m_hFX = CreateClientFXObject(szProps, m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildNote::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool BuildNote::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Hidden("HIDDEN");
	static CParsedMsg::CToken s_cTok_1("1");
	static CParsedMsg::CToken s_cTok_True("TRUE");
	static CParsedMsg::CToken s_cTok_0("0");
	static CParsedMsg::CToken s_cTok_False("FALSE");
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");

	if (cMsg.GetArg(0) == s_cTok_Hidden)
	{
		if (cMsg.GetArgCount() > 1)
		{
			if ((cMsg.GetArg(1) == s_cTok_1) || (cMsg.GetArg(1) == s_cTok_True))
			{
				if (m_hFX)
				{
					SendTriggerMsgToObject(this, m_hFX, LTFALSE, "OFF");
					g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
				}
			}
			else if ((cMsg.GetArg(1) == s_cTok_0) || (cMsg.GetArg(1) == s_cTok_False))
			{
				if (m_hFX)
				{
					SendTriggerMsgToObject(this, m_hFX, LTFALSE, "ON");
					g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
				}
			}
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		if (!hSender) return true;

		if (IsPlayer(hSender))
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
			if (pPlayer && !pPlayer->IsDead())
			{
				int nIndex = g_pInventoryButeMgr->GetItemIndex(m_szNoteName);
				if (nIndex != -1)
				{
					char szMsg[32];
					sprintf(szMsg, "ADDBUILDPOINTS %d", g_pInventoryButeMgr->GetItemPoints(nIndex));

					CAutoMessage cMsg;
					cMsg.Writeuint32(MID_TRIGGER);
					cMsg.Writeuint32((uint32)szMsg);
					g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hSender, MESSAGE_GUARANTEED);

					int nTextID = g_pInventoryButeMgr->GetItemTextID(nIndex);

					cMsg.Reset();
					cMsg.Writeuint8(MID_INTEL_PICKEDUP);
					cMsg.Writeuint32(nTextID);
					cMsg.Writeuint8(1);
					cMsg.Writeuint8(1);
					cMsg.Writeuint8(1);
					cMsg.Writeuint8(0);
					cMsg.Writebool(false);
					cMsg.WriteString("");
					g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

					if (m_hFX)
					{
						g_pLTServer->RemoveObject(m_hFX);
						m_hFX = LTNULL;
					}

					LTRotation rRot;
					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);
					g_pLTServer->GetObjectRotation(m_hObject, &rRot);
					PlayClientFX("WORLDS_buildnote_remove", LTNULL, &vPos, &rRot);

					g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
					g_pLTServer->RemoveObject(m_hObject);

					return true;
				}
			}
		}
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBuildNotePlugin::PreHook_EditStringList
//
//	PURPOSE:	The notes of the inventory bute file (those with points)
//
// ----------------------------------------------------------------------- //

LTRESULT CBuildNotePlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	strcpy(aszStrings[0], "<none>");
	*pcStrings = 1;

	if (strstr(szPropName, "NoteName"))
	{
		uint32 cItems = s_InventoryButeMgr.GetNumItems();
		for (uint32 iItem = 0; iItem < cItems; iItem++)
		{
			if (s_InventoryButeMgr.GetItemPoints(iItem))
			{
				strcpy(aszStrings[(*pcStrings)++], s_InventoryButeMgr.GetItemName(iItem));
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
