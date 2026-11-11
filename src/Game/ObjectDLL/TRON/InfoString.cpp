// ----------------------------------------------------------------------- //
//
// MODULE  : InfoString.cpp
//
// PURPOSE : An activatable volume the player can query for a string ID
//
// RETAIL: Tron 2.0's, written from retail Object.lto (InfoString.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "InfoString.h"
#include "MsgIDs.h"
#include "PlayerObj.h"
#include "SurfaceFunctions.h"
#include "ParsedMsg.h"

BEGIN_CLASS(InfoString)
	ADD_LONGINTPROP(ID, 0)
	ADD_VECTORPROP_VAL_FLAG(Dims, 10.0f, 10.0f, 10.0f, PF_DIMS)
END_CLASS_DEFAULT(InfoString, GameBase, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfoString::InfoString()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

InfoString::InfoString() : GameBase(OT_NORMAL)
{
	m_vDims.x = m_vDims.y = m_vDims.z = 10.0f;
	m_nID = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfoString::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 InfoString::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}

		// RETAIL: GameBase never sees the initial update, nor any message
		// but the four handled here (retail 0x10021da0 returns 0 for them)
		case MID_INITIALUPDATE:
		{
			UpdateUserFlags();
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);
			return GameBase::EngineMessageFn(messageID, pData, fData);
		}

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
			return GameBase::EngineMessageFn(messageID, pData, fData);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfoString::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 InfoString::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_QUERY_TARGET_PROPERTIES:
		{
			uint8 nNumProps = pMsg->Readuint8();
			uint8 nNumPropsProcessed = 0;

			// See what we need to send down
			if(nNumProps > 0)
			{
				// Start the message
				CAutoMessage cTempMsg;

				// Loop through the props
				uint8 iProp;
				for(int i=0;i<nNumProps;i++)
				{
					// Get the prop
					iProp = pMsg->Readuint8();
					switch(iProp)
					{
						case TARGET_PROP_INFO_STRING:
						{
							// Write the ID, the value, and inc the number of props processed
							cTempMsg.Writeuint8(iProp);
							cTempMsg.Writeuint32(m_nID);
							nNumPropsProcessed++;

							break;
						}
					}
				}

				// Build the actual message to send back down to the client
				if(nNumPropsProcessed > 0)
				{
					// Get the player
					CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
					if(pPlayer)
					{
						CAutoMessage cPropMsg;
						cPropMsg.Writeuint8(MID_QUERY_TARGET_PROPERTIES);
						cPropMsg.WriteObject(m_hObject);
						cPropMsg.Writeuint8(nNumPropsProcessed);
						cPropMsg.WriteMessage(cTempMsg.Read());
						g_pLTServer->SendToClient(cPropMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
					}
				}
			}

			return LT_OK;
		}
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfoString::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL InfoString::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return LTFALSE;

	pStruct->m_Flags |= FLAG_RAYHIT | FLAG_FORCECLIENTUPDATE;
	pStruct->m_Flags2 |= FLAG2_SERVERDIMS;

	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("ID", &genProp) == LT_OK)
	{
		m_nID = genProp.m_Long;
	}

	if (g_pLTServer->GetPropGeneric("Dims", &genProp) == LT_OK)
	{
		m_vDims = genProp.m_Vec;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfoString::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void InfoString::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_DWORD(m_nID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfoString::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void InfoString::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_DWORD(m_nID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfoString::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool InfoString::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_ID("ID");

	if (cMsg.GetArg(0) == s_cTok_ID)
	{
		if (cMsg.GetArgCount() > 1)
		{
			m_nID = atoi(cMsg.GetArg(1));
			UpdateUserFlags();
		}

		return true;
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfoString::UpdateUserFlags
//
//	PURPOSE:	Flagged for the target code while there is an ID;
//				an invisible surface; sized to the dims
//
// ----------------------------------------------------------------------- //

void InfoString::UpdateUserFlags()
{
	uint32 dwFlags = USRFLG_INFO_STRING;
	if (!m_nID)
	{
		dwFlags = 0;
	}

	g_pPhysicsLT->SetObjectDims(m_hObject, &m_vDims, 0);

	dwFlags |= SurfaceToUserFlag(ST_INVISIBLE);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwFlags, USRFLG_INFO_STRING | SurfaceToUserFlag(ST_INVISIBLE));
}
