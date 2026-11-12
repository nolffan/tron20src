// ----------------------------------------------------------------------- //
//
// MODULE  : Optimizer.cpp
//
// PURPOSE : An optimizer: a model a player engages (with the permission
//			 sets it asks for) to optimize his subroutines
//
// RETAIL: Tron 2.0's, written from retail Object.lto (Optimizer.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Optimizer.h"
#include "MsgIDs.h"
#include "ParsedMsg.h"
#include "ServerUtilities.h"
#include "ServerSoundMgr.h"
#include "TronPlayerObj.h"
#include "PSets.h"

// RETAIL: Tron's message ids (retail 0x10031769, 0x100308bd); the names
// are ours. A player engages us: we tell his client to start optimizing
// (MID_OPTIMIZER_BEGIN, our object); his client tells us how it went
// (MID_OPTIMIZER_RESULT, a bool)
#define MID_OPTIMIZER_BEGIN		177
#define MID_OPTIMIZER_RESULT	178

// RETAIL: Tron's special fx ids (retail 0x10030da2, 0x10031b50); the names
// are ours. SFX_OPTIMIZER_ID our special fx message; SFX_ATTACHFX_ID with
// ATTACHFX_MSG a message to the client about our attached fx
#define SFX_OPTIMIZER_ID		61
#define SFX_ATTACHFX_ID			60
#define ATTACHFX_MSG			2

// RETAIL: user flags we set on ourselves (InitialUpdate, retail 0x10030d14);
// not yet identified on the client. The names are ours
#define USRFLG_OPTIMIZER		(1<<4)
#define USRFLG_OPTIMIZER2		(1<<1)

// PORT: retail calls a ServerUtilities function (retail 0x100cdc10, not yet
// written) that asks the player's own (shared, in coop) permission sets;
// the player's are asked here (as IONode's)
static bool HasPSets(CTronPlayerObj* pPlayer, uint8 nPSets)
{
	return (pPlayer->GetPSets() & nPSets) == nPSets;
}

BEGIN_CLASS(Optimizer)
	ADD_STRINGPROP_FLAG(KeyFramer, "", PF_OBJECTLINK)
	ADD_STRINGPROP(OnActivate, "")
	ADD_STRINGPROP(OnOptimize, "")
	ADD_STRINGPROP(InvalidPSetCommand, "")
	ADD_PSETS_PROP(PF_GROUP(1))
END_CLASS_DEFAULT(Optimizer, GameBase, NULL, NULL)

BEGIN_CLASS(OptimizerExplosion)
END_CLASS_DEFAULT(OptimizerExplosion, Explosion, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS( Optimizer )
CMDMGR_END_REGISTER_CLASS( Optimizer, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::Optimizer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Optimizer::Optimizer() : GameBase(OT_MODEL),
	m_eState(eStateEnterLive),
	m_hKeyFramer(LTNULL),
	m_nPSets(0),
	m_hstrLoopSound(LTNULL),
	m_hLoopSound(LTNULL)
{
	m_ActivateTypeHandler.Init( this, SFX_OPTIMIZER_ID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::~Optimizer()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Optimizer::~Optimizer()
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
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Optimizer::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
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

		case MID_ALLOBJECTSCREATED:
		{
			AllObjectsCreated();
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

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
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Optimizer::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	if (messageID == MID_OPTIMIZER_RESULT)
	{
		if (pMsg->Readbool())
		{
			m_eState = eStateEnterDeath;
			m_OptimizeCommand.Execute(m_hObject, LTNULL);
		}
		else
		{
			if (m_hKeyFramer)
			{
				SendTriggerMsgToObject(this, m_hKeyFramer, LTFALSE, "RESUME");
			}

			m_eState = eStateEnterLive;
		}
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::HandleModelString
//
//	PURPOSE:	Sounds from the model's keys, while we are visible
//
// ----------------------------------------------------------------------- //

void Optimizer::HandleModelString(ArgList* pArgList)
{
	static CParsedMsg::CToken s_cTok_KEY_BUTE_SOUND("BUTE_SOUND_KEY");
	static CParsedMsg::CToken s_cTok_KEY_BUTE_SOUND_LOOPING("BUTE_SOUND_KEY_LOOPING");
	static CParsedMsg::CToken s_cTok_KILL_LOOPING_SOUND("KILL_LOOPING_SOUND");

	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	CParsedMsg::CToken tok( pKey );

	if (tok == s_cTok_KEY_BUTE_SOUND && IsVisible())
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pArgList->argv[1]);
		}
	}
	else if (tok == s_cTok_KEY_BUTE_SOUND_LOOPING && IsVisible())
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
	else if (tok == s_cTok_KILL_LOOPING_SOUND && IsVisible())
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
//	ROUTINE:	Optimizer::InitialUpdate
//
//	PURPOSE:	Size us to our animation; set our flags; our fx message
//
// ----------------------------------------------------------------------- //

LTBOOL Optimizer::InitialUpdate()
{
	LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE | USRFLG_OPTIMIZER, USRFLG_CAN_ACTIVATE | USRFLG_OPTIMIZER);

	SET_OBJECT_PSETS_USER_FLAGS(m_hObject, m_nPSets);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_OPTIMIZER2, USRFLG_OPTIMIZER2);

	SetNextUpdate(0.01f);

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_OPTIMIZER_ID);
	m_cs.Write(cMsg);
	m_ActivateTypeHandler.WriteActivateTypeMsg(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::Update
//
//	PURPOSE:	Run the state we are in
//
// ----------------------------------------------------------------------- //

void Optimizer::Update()
{
	switch (m_eState)
	{
		case eStateEnterLive:
		{
			EnterLive();
			m_eState = eStateLive;
		}
		break;

		case eStateLive:
		{
			UpdateLive();
		}
		break;

		case eStateEnterOptimizing:
		{
			EnterOptimizing();
			m_eState = eStateOptimizing;
		}
		break;

		case eStateOptimizing:
		{
			UpdateOptimizing();
		}
		break;

		case eStateEnterDeath:
		{
			EnterDeath();
			m_eState = eStateDeath;
		}
		break;

		case eStateDeath:
		{
			UpdateDeath();
		}
		break;
	}

	SetNextUpdate(0.01f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::AllObjectsCreated
//
//	PURPOSE:	Find our keyframer
//
// ----------------------------------------------------------------------- //

bool Optimizer::AllObjectsCreated()
{
	if (!m_sKeyFramer.empty())
	{
		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects(const_cast<char*>(m_sKeyFramer.c_str()), objArray);

		if (!objArray.NumObjects()) return false;

		m_hKeyFramer = objArray.GetObject(0);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL Optimizer::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (!pStruct) return LTFALSE;
	if (!g_pModelButeMgr) return LTFALSE;

	m_cs.eModelId = g_pModelButeMgr->GetModelId("Optimizer");
	SAFE_STRCPY(pStruct->m_Filename, g_pModelButeMgr->GetModelFilename(m_cs.eModelId));

	if (g_pModelButeMgr->GetNumSkins(m_cs.eModelId) == 6)
	{
		if (g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 0) &&
			g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 1) &&
			g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 2))
		{
			strcpy(pStruct->m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 0));
			strcpy(pStruct->m_SkinNames[1], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 1));
			// RETAIL: the third skin goes to the fourth slot (retail 0x10031112)
			strcpy(pStruct->m_SkinNames[3], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 2));
		}
	}

	g_pModelButeMgr->CopyRenderStyleFilenames(m_cs.eModelId, pStruct);

	pStruct->m_Flags |= FLAG_RAYHIT | FLAG_SOLID | FLAG_MODELKEYS;

	HANDLE_READ_PSETS_PROPS(m_nPSets);

	if (g_pLTServer->GetPropGeneric("KeyFramer", &genProp) == LT_OK && genProp.m_String[0])
	{
		m_sKeyFramer = genProp.m_String;
	}

	m_ActivateCommand.ReadProp(pStruct, "OnActivate");
	m_OptimizeCommand.ReadProp(pStruct, "OnOptimize");
	m_InvalidPSetCommand.ReadProp(pStruct, "InvalidPSetCommand");

	m_ActivateTypeHandler.SetActivateType( "Use Optimizer" );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool Optimizer::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_1("1");
	static CParsedMsg::CToken s_cTok_True("TRUE");
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_AddPSet("ADDPSET");
	static CParsedMsg::CToken s_cTok_RemovePSet("REMOVEPSET");
	static CParsedMsg::CToken s_cTok_Hidden("HIDDEN");

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_User, dwUserFlags);
		if (dwUserFlags & USRFLG_CAN_ACTIVATE)
		{
			CTronPlayerObj* pPlayer = dynamic_cast<CTronPlayerObj*>(g_pLTServer->HandleToObject(hSender));
			if (pPlayer && !pPlayer->IsDead())
			{
				if (HasPSets(pPlayer, m_nPSets) == true)
				{
					HCLIENT hClient = pPlayer->GetClient();
					if (hClient)
					{
						HOBJECT hObj = m_hObject;
						m_ActivateCommand.Execute(hObj, pPlayer->GetUserObject());

						m_eState = eStateEnterOptimizing;

						LTVector vPos;
						LTRotation rRot;
						g_pLTServer->GetObjectPos(m_hObject, &vPos);
						g_pLTServer->GetObjectRotation(m_hObject, &rRot);

						if (m_hKeyFramer)
						{
							SendTriggerMsgToObject(this, m_hKeyFramer, LTFALSE, "PAUSE");
						}

						CAutoMessage cClientMsg;
						cClientMsg.Writeuint8(MID_OPTIMIZER_BEGIN);
						cClientMsg.WriteObject(m_hObject);
						g_pLTServer->SendToClient(cClientMsg.Read(), hClient, MESSAGE_GUARANTEED);
					}
				}
				else
				{
					HOBJECT hObj = m_hObject;
					m_InvalidPSetCommand.Execute(hObj, pPlayer->GetUserObject());

					g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "OptimzerInvalidPSet");
				}
			}
		}
	}

	if (cMsg.GetArg(0) == s_cTok_AddPSet && cMsg.GetArgCount() > 1)
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

	if (cMsg.GetArg(0) == s_cTok_RemovePSet && cMsg.GetArgCount() > 1)
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

	if (cMsg.GetArg(0) == s_cTok_Hidden && cMsg.GetArgCount() > 1)
	{
		if (cMsg.GetArg(1) == s_cTok_1 || cMsg.GetArg(1) == s_cTok_True)
		{
			if (m_hLoopSound)
			{
				g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
				m_hLoopSound = LTNULL;
			}
		}
		else
		{
			PlayLoopSound();
		}
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::EnterLive
//
//	PURPOSE:	Loop our live animation; show our attached fx
//
// ----------------------------------------------------------------------- //

void Optimizer::EnterLive()
{
	g_pLTServer->SetModelAnimation(m_hObject, GetAnim("liveloop"));
	g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_ATTACHFX_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(ATTACHFX_MSG);
	cMsg.WriteString("SHOW_ATTACHFX");
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::UpdateLive
//
//	PURPOSE:	On to optimizing when the animation is done
//
// ----------------------------------------------------------------------- //

void Optimizer::UpdateLive()
{
	if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
	{
		m_eState = eStateEnterOptimizing;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::EnterOptimizing
//
//	PURPOSE:	Loop our optimizing animation
//
// ----------------------------------------------------------------------- //

void Optimizer::EnterOptimizing()
{
	g_pLTServer->SetModelAnimation(m_hObject, GetAnim("optimize"));
	g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::EnterDeath
//
//	PURPOSE:	No more engaging; start dying; hide our attached fx
//
// ----------------------------------------------------------------------- //

void Optimizer::EnterDeath()
{
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
	// RETAIL: a third state of Tron's (retail 0x10031c74), not in NOLF2's
	m_ActivateTypeHandler.SetState((ACTIVATETYPE::State)2);

	g_pLTServer->SetModelAnimation(m_hObject, GetAnim("startdeath"));
	g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_ATTACHFX_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(ATTACHFX_MSG);
	cMsg.WriteString("HIDE_ATTACHFX");
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::UpdateDeath
//
//	PURPOSE:	When done dying: loop dead, in our dead skins
//
// ----------------------------------------------------------------------- //

void Optimizer::UpdateDeath()
{
	HMODELANIM hAnim;
	g_pModelLT->GetCurAnim(m_hObject, MAIN_TRACKER, hAnim);

	if (GetAnim("startdeath") == hAnim && (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE))
	{
		g_pLTServer->SetModelAnimation(m_hObject, GetAnim("deathloop"));
		g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

		ObjectCreateStruct ocs;

		ModelId eModelId = g_pModelButeMgr->GetModelId("Optimizer");
		if (g_pModelButeMgr->GetNumSkins(eModelId) == 6)
		{
			if (g_pModelButeMgr->GetSkinFilename(eModelId, 3) &&
				g_pModelButeMgr->GetSkinFilename(eModelId, 4) &&
				g_pModelButeMgr->GetSkinFilename(eModelId, 5))
			{
				strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(eModelId, 3));
				strcpy(ocs.m_SkinNames[1], g_pModelButeMgr->GetSkinFilename(eModelId, 4));
				// RETAIL: the third skin goes to the fourth slot (retail 0x10032037)
				strcpy(ocs.m_SkinNames[3], g_pModelButeMgr->GetSkinFilename(eModelId, 5));

				g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::GetAnim
//
//	PURPOSE:	Our animation of that name
//
// ----------------------------------------------------------------------- //

HMODELANIM Optimizer::GetAnim(const char* szAnim)
{
	if (!szAnim)
	{
		ObjectCPrint(m_hObject, "Optimizer::GetAnim void pointer for anim name");
		return INVALID_ANI;
	}

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char*>(szAnim));
	if (hAnim == INVALID_ANI)
	{
		ObjectCPrint(m_hObject, "Optimizer missing animation %s", szAnim);
	}

	return hAnim;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Optimizer::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	pMsg->Writeint8((int8)m_eState);
	SAVE_STDSTRING(m_sKeyFramer);
	SAVE_HOBJECT(m_hKeyFramer);
	SAVE_BYTE(m_nPSets);
	m_ActivateCommand.Save(pMsg);
	m_OptimizeCommand.Save(pMsg);
	m_InvalidPSetCommand.Save(pMsg);
	m_cs.Write(pMsg);
	m_ActivateTypeHandler.Save(pMsg);
	SAVE_HSTRING(m_hstrLoopSound);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Optimizer::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_eState = pMsg->Readint8();

	char szString[1024];
	LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
	m_sKeyFramer = szString;

	LOAD_HOBJECT(m_hKeyFramer);
	LOAD_BYTE(m_nPSets);
	m_ActivateCommand.Load(pMsg);
	m_OptimizeCommand.Load(pMsg);
	m_InvalidPSetCommand.Load(pMsg);
	m_cs.Read(pMsg);
	m_ActivateTypeHandler.Load(pMsg);

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_OPTIMIZER_ID);
	m_cs.Write(cMsg);
	m_ActivateTypeHandler.WriteActivateTypeMsg(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());

	LOAD_HSTRING(m_hstrLoopSound);

	m_hLoopSound = LTNULL;
	PlayLoopSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::PlayLoopSound
//
//	PURPOSE:	(Re)start the looping sound
//
// ----------------------------------------------------------------------- //

void Optimizer::PlayLoopSound()
{
	if (!m_hstrLoopSound) return;

	if (m_hLoopSound)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
	}

	m_hLoopSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, g_pLTServer->GetStringData(m_hstrLoopSound), -1.0f,
		SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::IsVisible
//
//	PURPOSE:	Are we visible?
//
// ----------------------------------------------------------------------- //

bool Optimizer::IsVisible()
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	return !!(dwFlags & FLAG_VISIBLE);
}
