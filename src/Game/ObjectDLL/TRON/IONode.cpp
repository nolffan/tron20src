// ----------------------------------------------------------------------- //
//
// MODULE  : IONode.cpp
//
// PURPOSE : An I/O node: engaged by a player with its ping and permission
//			 sets
//
// RETAIL: Tron 2.0's, written from retail Object.lto (IONode.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "IONode.h"
#include "MsgIDs.h"
#include "ParsedMsg.h"
#include "ServerUtilities.h"
#include "ServerSoundMgr.h"
#include "ModelButeMgr.h"
#include "TronPlayerObj.h"
#include "PSets.h"

// RETAIL: a player's pings, a list at +0x3228 of Tron's CPlayerObj (retail
// 0x10023920), whose layout is still to be written; a view of it stands in.
// The names are ours
struct IONodePing { IONodePing* pNext; char* szName; };
struct IONodePlayerView { uint8 aPad[0x3228]; IONodePing* pPings; };
#define IONODE_PLAYER_PINGS(pPlayer)	(((IONodePlayerView*)(pPlayer))->pPings)

// PORT: retail calls a ServerUtilities function (retail 0x100cdc10, not yet
// written) that asks the player's own (shared, in coop) permission sets
// (retail 0x10090720); the player's are asked here
static bool HasPSets(CTronPlayerObj* pPlayer, uint8 nPSets)
{
	return (pPlayer->GetPSets() & nPSets) == nPSets;
}

BEGIN_CLASS(IONode)
	ADD_PSETS_PROP(PF_GROUP(1))
	ADD_STRINGPROP(PingName, "")
	ADD_STRINGPROP(TriggerCommand, "")
	ADD_STRINGPROP(InvalidPSetCommand, "")
	ADD_BOOLPROP(MoveToFloor, LTFALSE)
	ADD_LONGINTPROP(StringOverrideID, 0)
END_CLASS_DEFAULT(IONode, GameBase, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS( IONode )
CMDMGR_END_REGISTER_CLASS( IONode, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::IONode()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

IONode::IONode() : GameBase(OT_MODEL),
	m_eState(eStateEnterLocked),
	m_hstrTriggerCommand(LTNULL),
	m_nPSets(0),
	m_hActivator(LTNULL),
	m_vActivatorPos(LTVector(0.0f, 0.0f, 0.0f)),
	m_bFirstUpdate(true),
	m_bMoveToFloor(false),
	m_bPinged(false),
	m_hstrLoopSound(LTNULL),
	m_hLoopSound(LTNULL)
{
	m_ActivateTypeHandler.Init( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::~IONode()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

IONode::~IONode()
{
	if (m_hstrTriggerCommand)
	{
		g_pLTServer->FreeString(m_hstrTriggerCommand);
		m_hstrTriggerCommand = LTNULL;
	}

	if (m_hLoopSound)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
		m_hLoopSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 IONode::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
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
			}

			Update();
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
		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
		}
		break;

	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::InitialUpdate
//
//	PURPOSE:	Size us to our animation; set our flags
//
// ----------------------------------------------------------------------- //

LTBOOL IONode::InitialUpdate()
{
	LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	// Activatable from the start in multiplayer; in single player once
	// the player has our ping
	if (IsMultiplayerGame())
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
	}
	else
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
	}
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

	SET_OBJECT_PSETS_USER_FLAGS(m_hObject, m_nPSets);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_IONODE, USRFLG_IONODE);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, (m_nStringOverrideID > 0) ? USRFLG_INFO_STRING : 0, USRFLG_INFO_STRING);

	SetNextUpdate(0.01f);

	m_ActivateTypeHandler.SetActivateType( "Engage IONode" );
	m_ActivateTypeHandler.CreateActivateTypeMsg();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::Update
//
//	PURPOSE:	Run the state we are in
//
// ----------------------------------------------------------------------- //

void IONode::Update()
{
	switch (m_eState)
	{
		case eStateEnterLocked:
		{
			EnterLocked();
			m_eState = eStateLocked;
		}
		break;

		case eStateLocked:
		{
			UpdateLocked();
		}
		break;

		case eStateEnterUnlocked:
		{
			EnterUnlocked();
			m_eState = eStateUnlocked;
		}
		break;

		case eStateUnlocked:
		{
			UpdateUnlocked();
		}
		break;

		case eStateEnterOpening:
		{
			EnterOpening();
			m_eState = eStateOpening;
		}
		break;

		case eStateOpening:
		{
			UpdateOpening();
		}
		break;

		case eStateEnterOpen:
		{
			EnterOpen();
			m_eState = eStateOpen;
		}
		break;

		case eStateOpen:
		{
			UpdateOpen();
		}
		break;

		case eStateEnterClosing:
		{
			EnterClosing();
			m_eState = eStateClosing;
		}
		break;

		case eStateClosing:
		{
			UpdateClosing();
		}
		break;
	}

	SetNextUpdate(0.01f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL IONode::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (!pStruct) return LTFALSE;
	if (!g_pModelButeMgr) return LTFALSE;

	ModelId eModelId = g_pModelButeMgr->GetModelId("IONode");
	LTStrCpy(pStruct->m_Filename, g_pModelButeMgr->GetModelFilename(eModelId), ARRAY_LEN(pStruct->m_Filename));
	g_pModelButeMgr->CopySkinFilenames(eModelId, 0, pStruct->m_SkinNames[0], ARRAY_LEN(pStruct->m_SkinNames[0]));
	g_pModelButeMgr->CopyRenderStyleFilenames(eModelId, pStruct);

	pStruct->m_Flags |= FLAG_RAYHIT | FLAG_SOLID | FLAG_MODELKEYS;

	HANDLE_READ_PSETS_PROPS(m_nPSets);

	if (g_pLTServer->GetPropGeneric("PingName", &genProp) == LT_OK && genProp.m_String[0])
	{
		m_sPingName = genProp.m_String;
	}

	if (g_pLTServer->GetPropGeneric("TriggerCommand", &genProp) == LT_OK && genProp.m_String[0])
	{
		m_hstrTriggerCommand = g_pLTServer->CreateString(genProp.m_String);
	}

	if (g_pLTServer->GetPropGeneric("StringOverrideID", &genProp) == LT_OK)
	{
		m_nStringOverrideID = genProp.m_Long;
	}

	m_InvalidPSetCommand.ReadProp(pStruct, "InvalidPSetCommand");

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool IONode::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_AddPSet("ADDPSET");
	static CParsedMsg::CToken s_cTok_RemovePSet("REMOVEPSET");
	static CParsedMsg::CToken s_cTok_InfoString("INFOSTRING");
	static CParsedMsg::CToken s_cTok_CineActivate("CINEACTIVATE");
	static CParsedMsg::CToken s_cTok_CineDeactivate("CINEDEACTIVATE");

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		CTronPlayerObj* pPlayer = dynamic_cast<CTronPlayerObj*>(g_pLTServer->HandleToObject(hSender));
		if (pPlayer && !pPlayer->IsDead())
		{
			if (HasPing(pPlayer) && HasPSets(pPlayer, m_nPSets))
			{
				// Only from in front of us
				LTRotation rRot;
				LTVector vPos, vSenderPos;
				g_pLTServer->GetObjectRotation(m_hObject, &rRot);
				g_pLTServer->GetObjectPos(m_hObject, &vPos);
				g_pLTServer->GetObjectPos(hSender, &vSenderPos);

				// RETAIL: computes the three differences before storing them
				// (retail 0x10023537); near
				LTVector vDir(vSenderPos.x - vPos.x, vSenderPos.y - vPos.y, vSenderPos.z - vPos.z);
				vDir.Normalize();

				if (vDir.Dot(rRot.Forward()) > 0.707)
				{
					m_hActivator = hSender;
					Trigger();
				}
			}
			else
			{
				HOBJECT hObj = m_hObject;
				m_InvalidPSetCommand.Execute(hObj, pPlayer->GetUserObject());

				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "IONodeInvalidPSet");
			}
		}

		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_CineActivate)
	{
		Trigger();
	}
	else if (cMsg.GetArg(0) == s_cTok_CineDeactivate)
	{
		m_eState = eStateEnterClosing;
	}
	else if (cMsg.GetArg(0) == s_cTok_AddPSet)
	{
		if (cMsg.GetArgCount() > 1)
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
		}

		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_RemovePSet)
	{
		if (cMsg.GetArgCount() > 1)
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
		}

		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_InfoString)
	{
		if (cMsg.GetArgCount() > 1)
		{
			m_nStringOverrideID = atoi(cMsg.GetArg(1));
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, (m_nStringOverrideID > 0) ? USRFLG_INFO_STRING : 0, USRFLG_INFO_STRING);
		}

		return true;
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::HasPing
//
//	PURPOSE:	Does the player have our ping?
//
// ----------------------------------------------------------------------- //

LTBOOL IONode::HasPing(CPlayerObj* pPlayer)
{
	for (IONodePing* pPing = IONODE_PLAYER_PINGS(pPlayer); pPing; pPing = pPing->pNext)
	{
		if (stricmp(pPing->szName, m_sPingName.c_str()) == 0)
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::Trigger
//
//	PURPOSE:	Engaged: open and run our command
//
// ----------------------------------------------------------------------- //

void IONode::Trigger()
{
	m_eState = eStateEnterOpening;

	if (m_hstrTriggerCommand)
	{
		const char* pCmd = g_pLTServer->GetStringData(m_hstrTriggerCommand);
		if (g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, (HOBJECT)LTNULL);
		}
		else
		{
			g_pLTServer->CPrint("IONode::Trigger - ERROR - Invalid command: %s\n", pCmd);
		}
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::EnterLocked
//
//	PURPOSE:	The locked skin
//
// ----------------------------------------------------------------------- //

void IONode::EnterLocked()
{
	ObjectCreateStruct ocs;

	ModelId eModelId = g_pModelButeMgr->GetModelId("IONode");
	strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(eModelId, 4));
	strcpy(ocs.m_SkinNames[1], g_pModelButeMgr->GetSkinFilename(eModelId, 1));
	strcpy(ocs.m_SkinNames[2], g_pModelButeMgr->GetSkinFilename(eModelId, 2));
	strcpy(ocs.m_SkinNames[3], g_pModelButeMgr->GetSkinFilename(eModelId, 3));

	g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::UpdateLocked
//
//	PURPOSE:	Wait for the player to have our ping
//
// ----------------------------------------------------------------------- //

void IONode::UpdateLocked()
{
	if (!IsMultiplayerGame())
	{
		CPlayerObj* pPlayer = *CPlayerObj::GetPlayerObjList().begin();
		if (pPlayer && !m_bPinged && IONODE_PLAYER_PINGS(pPlayer) && HasPing(pPlayer))
		{
			m_bPinged = true;
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
		}
	}

	if (m_bPinged)
	{
		m_eState = eStateEnterUnlocked;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::EnterUnlocked
//
//	PURPOSE:	The unlocked skin
//
// ----------------------------------------------------------------------- //

void IONode::EnterUnlocked()
{
	ObjectCreateStruct ocs;

	ModelId eModelId = g_pModelButeMgr->GetModelId("IONode");
	strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(eModelId, 0));
	strcpy(ocs.m_SkinNames[1], g_pModelButeMgr->GetSkinFilename(eModelId, 1));
	strcpy(ocs.m_SkinNames[2], g_pModelButeMgr->GetSkinFilename(eModelId, 2));
	strcpy(ocs.m_SkinNames[3], g_pModelButeMgr->GetSkinFilename(eModelId, 3));

	g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::UpdateUnlocked
//
//	PURPOSE:	Back to locked when the ping is gone
//
// ----------------------------------------------------------------------- //

void IONode::UpdateUnlocked()
{
	if (!m_bPinged)
	{
		m_eState = eStateEnterLocked;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::EnterOpen
//
//	PURPOSE:	Open: idle, and remember where the activator is
//
// ----------------------------------------------------------------------- //

void IONode::EnterOpen()
{
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Idle");
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	g_pLTServer->SetModelLooping(m_hObject, true);

	g_pLTServer->GetObjectPos(m_hActivator, &m_vActivatorPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::UpdateOpen
//
//	PURPOSE:	Close once the activator walks away
//
// ----------------------------------------------------------------------- //

void IONode::UpdateOpen()
{
	if (m_hActivator)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hActivator, &vPos);

		if ((m_vActivatorPos - vPos).MagSqr() > 64.0f)
		{
			m_eState = eStateEnterClosing;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::EnterOpening
//
//	PURPOSE:	Play the open animation
//
// ----------------------------------------------------------------------- //

void IONode::EnterOpening()
{
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Open");
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	g_pLTServer->SetModelLooping(m_hObject, false);

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pServerSoundMgr->PlaySoundFromPos(vPos, "IONode_open", 1000.0f, SOUNDPRIORITY_MISC_HIGH);

	m_bPinged = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::UpdateOpening
//
//	PURPOSE:	Open once the animation is done
//
// ----------------------------------------------------------------------- //

void IONode::UpdateOpening()
{
	if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
	{
		m_eState = eStateEnterOpen;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::EnterClosing
//
//	PURPOSE:	Play the close animation
//
// ----------------------------------------------------------------------- //

void IONode::EnterClosing()
{
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Close");
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	g_pLTServer->SetModelLooping(m_hObject, false);

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pServerSoundMgr->PlaySoundFromPos(vPos, "IONode_close", 1000.0f, SOUNDPRIORITY_MISC_HIGH);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::UpdateClosing
//
//	PURPOSE:	Locked again once the animation is done
//
// ----------------------------------------------------------------------- //

void IONode::UpdateClosing()
{
	if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
	{
		HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Default");
		g_pLTServer->SetModelAnimation(m_hObject, hAnim);
		g_pLTServer->SetModelLooping(m_hObject, true);

		ObjectCreateStruct ocs;

		ModelId eModelId = g_pModelButeMgr->GetModelId("IONode");
		strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(eModelId, 4));
		strcpy(ocs.m_SkinNames[1], g_pModelButeMgr->GetSkinFilename(eModelId, 1));
		strcpy(ocs.m_SkinNames[2], g_pModelButeMgr->GetSkinFilename(eModelId, 2));
		strcpy(ocs.m_SkinNames[3], g_pModelButeMgr->GetSkinFilename(eModelId, 3));

		g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);

		m_eState = eStateLocked;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void IONode::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	pMsg->Writeint8((int8)m_eState);
	SAVE_STDSTRING(m_sPingName);
	SAVE_HSTRING(m_hstrTriggerCommand);
	SAVE_BYTE(m_nPSets);
	SAVE_HOBJECT(m_hActivator);
	pMsg->WriteType(m_vActivatorPos);
	SAVE_bool(m_bFirstUpdate);
	SAVE_bool(m_bMoveToFloor);
	SAVE_bool(m_bPinged);
	SAVE_INT(m_nStringOverrideID);
	m_InvalidPSetCommand.Save(pMsg);
	m_ActivateTypeHandler.Save(pMsg);
	SAVE_HSTRING(m_hstrLoopSound);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void IONode::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_eState = pMsg->Readint8();

	char szString[1024];
	LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
	m_sPingName = szString;

	LOAD_HSTRING(m_hstrTriggerCommand);
	LOAD_BYTE(m_nPSets);
	LOAD_HOBJECT(m_hActivator);
	LOAD_VECTOR(m_vActivatorPos);
	LOAD_bool(m_bFirstUpdate);
	LOAD_bool(m_bMoveToFloor);
	LOAD_bool(m_bPinged);
	LOAD_INT(m_nStringOverrideID);
	m_InvalidPSetCommand.Load(pMsg);
	m_ActivateTypeHandler.Load(pMsg);
	LOAD_HSTRING(m_hstrLoopSound);

	m_hLoopSound = LTNULL;
	PlayLoopSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IONode::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 IONode::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
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
							if (m_nStringOverrideID > 0)
							{
								// Write the ID, the value, and inc the number of props processed
								cTempMsg.Writeuint8(iProp);
								cTempMsg.Writeint32(m_nStringOverrideID);
								nNumPropsProcessed++;
							}

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
//	ROUTINE:	IONode::HandleModelString
//
//	PURPOSE:	Sounds from the model's keys
//
// ----------------------------------------------------------------------- //

void IONode::HandleModelString(ArgList* pArgList)
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
//	ROUTINE:	IONode::PlayLoopSound
//
//	PURPOSE:	(Re)start the looping sound
//
// ----------------------------------------------------------------------- //

void IONode::PlayLoopSound()
{
	if (!m_hstrLoopSound) return;

	if (m_hLoopSound)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
	}

	m_hLoopSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, g_pLTServer->GetStringData(m_hstrLoopSound), -1.0f,
		SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
}
