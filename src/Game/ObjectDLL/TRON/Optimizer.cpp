// ----------------------------------------------------------------------- //
//
// MODULE  : Optimizer.cpp
//
// PURPOSE : An optimizer: activated by a player with its permission sets
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
#include "ModelButeMgr.h"
#include "TronPlayerObj.h"
#include "PSets.h"

// RETAIL: Tron's message and special fx ids, not in our MsgIDs.h /
// SFXMsgIDs.h; the names are ours
#define MID_OPTIMIZER_ACTIVATED		177		// Server to the activating client (0xb1)
#define MID_OPTIMIZER_DONE			178		// To the optimizer: a bool, die or live again (0xb2)
#define SFX_OPTIMIZER_ID			61		// Our special fx message (0x3d)
#define SFX_ATTACHFX_ID				60		// With MID_SFX_MESSAGE: SHOW_ATTACHFX / HIDE_ATTACHFX (0x3c)
#define ATTACHFX_MSG_STRING			2		// Its sub-message: a string follows

// RETAIL: user flags it sets on itself (InitialUpdate); the names are ours
#define USRFLG_OPTIMIZER			(1<<1)	// As IONode's
#define USRFLG_OPTIMIZER_ACTIVATE	(USRFLG_CAN_ACTIVATE | (1<<4))

// PORT: retail calls a ServerUtilities function (retail 0x100cdc10, not yet
// written) that asks the player's own (shared, in coop) permission sets
// (retail 0x10090720); the player's are asked here (as IONode.cpp does)
static bool HasPSets(CTronPlayerObj* pPlayer, uint8 nPSets)
{
	return (pPlayer->GetPSets() & nPSets) == nPSets;
}

BEGIN_CLASS(Optimizer)
	ADD_OBJECTPROP(KeyFramer, "")
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
			FindKeyFramer();
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

	switch(messageID)
	{
		case MID_OPTIMIZER_DONE:
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
		break;
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
//	PURPOSE:	Size us to our animation; set our flags; our special fx
//
// ----------------------------------------------------------------------- //

LTBOOL Optimizer::InitialUpdate()
{
	LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_OPTIMIZER_ACTIVATE, USRFLG_OPTIMIZER_ACTIVATE);

	SET_OBJECT_PSETS_USER_FLAGS(m_hObject, m_nPSets);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_OPTIMIZER, USRFLG_OPTIMIZER);

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

		case eStateEnterOptimize:
		{
			EnterOptimize();
			m_eState = eStateOptimize;
		}
		break;

		case eStateOptimize:
		{
			UpdateOptimize();
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
//	ROUTINE:	Optimizer::FindKeyFramer
//
//	PURPOSE:	Find the keyframer we are linked to
//
// ----------------------------------------------------------------------- //

bool Optimizer::FindKeyFramer()
{
	if (!m_sKeyFramer.empty())
	{
		ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects(m_sKeyFramer.c_str(), objArray);

		if (!objArray.NumObjects())
		{
			return false;
		}

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
	LTStrCpy(pStruct->m_Filename, g_pModelButeMgr->GetModelFilename(m_cs.eModelId), ARRAY_LEN(pStruct->m_Filename));

	// The first three skins are the live ones
	if (g_pModelButeMgr->GetNumSkins(m_cs.eModelId) == 6 &&
		g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 0) &&
		g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 1) &&
		g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 2))
	{
		strcpy(pStruct->m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 0));
		strcpy(pStruct->m_SkinNames[1], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 1));
		// RETAIL: into the fourth slot, not the third (retail 0x10031112)
		strcpy(pStruct->m_SkinNames[3], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 2));
	}

	g_pModelButeMgr->CopyRenderStyleFilenames(m_cs.eModelId, pStruct);

	pStruct->m_Flags |= FLAG_VISIBLE | FLAG_RAYHIT | FLAG_SOLID | FLAG_MODELKEYS;

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
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_User, dwFlags);

		if (dwFlags & USRFLG_CAN_ACTIVATE)
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

						m_eState = eStateEnterOptimize;

						// RETAIL: read and not used (retail 0x100316e5)
						LTVector vPos;
						LTRotation rRot;
						g_pLTServer->GetObjectPos(m_hObject, &vPos);
						g_pLTServer->GetObjectRotation(m_hObject, &rRot);

						if (m_hKeyFramer)
						{
							SendTriggerMsgToObject(this, m_hKeyFramer, LTFALSE, "PAUSE");
						}

						CAutoMessage cMsg;
						cMsg.Writeuint8(MID_OPTIMIZER_ACTIVATED);
						cMsg.WriteObject(m_hObject);
						g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
					}
				}
				else
				{
					HOBJECT hObj = m_hObject;
					m_InvalidPSetCommand.Execute(hObj, pPlayer->GetUserObject());

					// RETAIL: the sound's name is misspelt so (retail 0x102daaa8)
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

	// Our looping sound stops while we are hidden (GameBase hides us)
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
//	PURPOSE:	Loop the live animation; show the attached fx
//
// ----------------------------------------------------------------------- //

void Optimizer::EnterLive()
{
	g_pLTServer->SetModelAnimation(m_hObject, GetAnim("liveloop"));
	g_pLTServer->SetModelLooping(m_hObject, true);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_ATTACHFX_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(ATTACHFX_MSG_STRING);
	cMsg.WriteString("SHOW_ATTACHFX");
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::UpdateLive
//
//	PURPOSE:	Optimize once the animation is done
//
// ----------------------------------------------------------------------- //

void Optimizer::UpdateLive()
{
	if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
	{
		m_eState = eStateEnterOptimize;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::EnterOptimize
//
//	PURPOSE:	Loop the optimize animation
//
// ----------------------------------------------------------------------- //

void Optimizer::EnterOptimize()
{
	g_pLTServer->SetModelAnimation(m_hObject, GetAnim("optimize"));
	g_pLTServer->SetModelLooping(m_hObject, true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::UpdateOptimize
//
//	PURPOSE:	Wait to be told we are done
//
// ----------------------------------------------------------------------- //

void Optimizer::UpdateOptimize()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::EnterDeath
//
//	PURPOSE:	No more activating; play the death animation; hide the
//				attached fx
//
// ----------------------------------------------------------------------- //

void Optimizer::EnterDeath()
{
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);

	// RETAIL: Tron's third state (retail 0x10031c74), not in our
	// ACTIVATETYPE::State
	m_ActivateTypeHandler.SetState( (ACTIVATETYPE::State)2 );

	g_pLTServer->SetModelAnimation(m_hObject, GetAnim("startdeath"));
	g_pLTServer->SetModelLooping(m_hObject, false);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_ATTACHFX_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(ATTACHFX_MSG_STRING);
	cMsg.WriteString("HIDE_ATTACHFX");
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Optimizer::UpdateDeath
//
//	PURPOSE:	Once the death animation is done: loop the dead one, in the
//				dead skins
//
// ----------------------------------------------------------------------- //

void Optimizer::UpdateDeath()
{
	HMODELANIM hCurAnim;
	g_pModelLT->GetCurAnim(m_hObject, MAIN_TRACKER, hCurAnim);

	if (GetAnim("startdeath") == hCurAnim &&
		(g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE))
	{
		g_pLTServer->SetModelAnimation(m_hObject, GetAnim("deathloop"));
		g_pLTServer->SetModelLooping(m_hObject, true);

		ObjectCreateStruct ocs;

		// The last three skins are the dead ones
		ModelId eModelId = g_pModelButeMgr->GetModelId("Optimizer");
		if (g_pModelButeMgr->GetNumSkins(eModelId) == 6 &&
			g_pModelButeMgr->GetSkinFilename(eModelId, 3) &&
			g_pModelButeMgr->GetSkinFilename(eModelId, 4) &&
			g_pModelButeMgr->GetSkinFilename(eModelId, 5))
		{
			strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(eModelId, 3));
			strcpy(ocs.m_SkinNames[1], g_pModelButeMgr->GetSkinFilename(eModelId, 4));
			// RETAIL: into the fourth slot, as ReadProp (retail 0x10032037)
			strcpy(ocs.m_SkinNames[3], g_pModelButeMgr->GetSkinFilename(eModelId, 5));

			g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);
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

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, szAnim);
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

	// RETAIL: attached, not PLAYSOUND_LOOP (retail 0x10032465); Tron's sound
	// call takes one more argument (1.0f) than ours
	m_hLoopSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, g_pLTServer->GetStringData(m_hstrLoopSound), -1.0f,
		SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_ATTACHED | PLAYSOUND_GETHANDLE);
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

	return (dwFlags & FLAG_VISIBLE) != 0;
}
