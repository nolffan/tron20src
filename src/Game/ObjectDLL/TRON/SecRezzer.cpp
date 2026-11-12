// ----------------------------------------------------------------------- //
//
// MODULE  : SecRezzer.cpp
//
// PURPOSE : A spawner of security programs at its rez points
//
// RETAIL: Tron 2.0's, written from retail Object.lto (SecRezzer.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SecRezzer.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ServerUtilities.h"
#include "ServerSoundMgr.h"
#include "CharacterMgr.h"
#include "TronPlayerObj.h"
#include "PSets.h"
#include <algorithm>

// RETAIL: Tron's special fx id of our message (as Optimizer's, retail
// 0x100454bd); the name is ours
#define SFX_SECREZZER_ID		61

// PORT: retail calls a ServerUtilities function (retail 0x100cdc10, not yet
// written) that asks the player's own (shared, in coop) permission sets;
// the player's are asked here (as IONode's)
static bool HasPSets(CTronPlayerObj* pPlayer, uint8 nPSets)
{
	return (pPlayer->GetPSets() & nPSets) == nPSets;
}

BEGIN_CLASS(SecRezzer)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), 0)
	ADD_BOOLPROP_FLAG(CanDamage, LTFALSE, PF_GROUP(1) | PF_HIDDEN)
	ADD_PSETS_PROP(PF_GROUP(2))
	ADD_STRINGPROP_FLAG(DefaultSpawn, "", PF_HIDDEN)
	ADD_STRINGPROP(Target, "")
	ADD_STRINGPROP(SpawnSound, "")
	ADD_REALPROP(SoundRadius, 500.0f)
	ADD_STRINGPROP_FLAG(InitialCommand, "", PF_NOTIFYCHANGE)
	ADD_LONGINTPROP(MaxActive, 1)
	ADD_LONGINTPROP(NumberToSpawn, 1)
	ADD_STRINGPROP(InvalidPSetCommand, "")
	ADD_STRINGPROP_FLAG(ModelTemplate, "Secrezzer", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(SecRezzer, Spawner, NULL, NULL, 0, SecrezzerPlugin)

// RETAIL: no command class of its own (none registered in retail)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecrezzerPlugin::PreHook_EditStringList
//
//	PURPOSE:	The model templates of its kind
//
// ----------------------------------------------------------------------- //

LTRESULT SecrezzerPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	static bool s_bInitted = false;
	if (!s_bInitted)
	{
		m_ModelButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		s_bInitted = true;
	}

	if (!_stricmp("ModelTemplate", szPropName))
	{
		// PORT: retail builds a sorted list of the names of the models of
		// type 3 (retail 0x10044570, with STL helpers); a plain list here
		std::vector<std::string> lstNames;

		uint32 nNumModels = g_pModelButeMgr->GetNumModels();
		for (uint32 i = 0; i < nNumModels; i++)
		{
			if (g_pModelButeMgr->GetModelType((ModelId)i) != 3) continue;

			const char* pszName = g_pModelButeMgr->GetModelName((ModelId)i);
			if (!pszName || !pszName[0]) continue;

			std::string sName(pszName);
			if (std::find(lstNames.begin(), lstNames.end(), sName) == lstNames.end())
			{
				lstNames.push_back(sName);
			}
		}

		std::sort(lstNames.begin(), lstNames.end());

		for (std::vector<std::string>::iterator it = lstNames.begin(); it != lstNames.end() && *pcStrings < cMaxStrings; ++it)
		{
			strncpy(aszStrings[(*pcStrings)++], it->c_str(), cMaxStringLength - 1);
		}

		return LT_OK;
	}

	return CSpawnerPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::SecRezzer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SecRezzer::SecRezzer() : Spawner(),
	m_eState(eStateEnterIdle),
	m_nMaxActive(0),
	m_nToSpawn(0),
	m_nPSets(0),
	m_bLocked(false)
{
	m_nType = OT_MODEL;

	AddAggregate(&m_damage);
	m_damage.SetCanDamage(LTFALSE);
	// RETAIL: also clears a member of m_damage (retail 0x10044b58, +0x70c
	// of retail's CDestructibleModel: the one its constructor sets to 1,
	// its m_bRemoveOnDeath); not done until CDestructibleModel is retail's

	m_hHook = LTNULL;

	m_ActivateTypeHandler.Init( this, SFX_SECREZZER_ID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::~SecRezzer()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

SecRezzer::~SecRezzer()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL SecRezzer::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (!g_pModelButeMgr) return LTFALSE;

	m_cs.eModelId = g_pModelButeMgr->GetModelId("Secrezzer");

	if (g_pLTServer->GetPropGeneric("ModelTemplate", &genProp) == LT_OK && genProp.m_String[0])
	{
		ModelId eModelId = g_pModelButeMgr->GetModelId(genProp.m_String);
		if (eModelId != eModelIdInvalid)
		{
			m_cs.eModelId = eModelId;
		}
	}

	SAFE_STRCPY(pStruct->m_Filename, g_pModelButeMgr->GetModelFilename(m_cs.eModelId));
	g_pModelButeMgr->CopySkinFilenames(m_cs.eModelId, 0, pStruct->m_SkinNames[0], ARRAY_LEN(pStruct->m_SkinNames[0]));
	g_pModelButeMgr->CopyRenderStyleFilenames(m_cs.eModelId, pStruct);

	pStruct->m_Flags |= FLAG_VISIBLE | FLAG_RAYHIT | FLAG_SOLID | FLAG_MODELKEYS;

	HANDLE_READ_PSETS_PROPS(m_nPSets);

	if (g_pLTServer->GetPropGeneric("MaxActive", &genProp) == LT_OK)
	{
		m_nMaxActive = genProp.m_Long;
	}

	if (g_pLTServer->GetPropGeneric("NumberToSpawn", &genProp) == LT_OK)
	{
		m_nNumberToSpawn = genProp.m_Long;
	}

	m_InvalidPSetCommand.ReadProp(pStruct, "InvalidPSetCommand");

	m_ActivateTypeHandler.SetActivateType( "Disable Secrezzer" );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SecRezzer::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
			return LT_OK;
		}

		case MID_UPDATE:
		{
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
			ArgList* pArgList = (ArgList*)pData;
			if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc <= 0) break;

			if (!_stricmp(pArgList->argv[0], "DEATHFX"))
			{
				if (pArgList->argc > 1 && pArgList->argv[1])
				{
					PlayClientFX(pArgList->argv[1], m_hObject, (LTVector*)LTNULL, (LTRotation*)LTNULL, 0);
				}
			}
			else if (!_stricmp(pArgList->argv[0], "BUTE_SOUND_KEY"))
			{
				if (pArgList->argc > 1 && pArgList->argv[1])
				{
					g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pArgList->argv[1]);
				}
			}
			else if (!_stricmp(pArgList->argv[0], "SPAWN"))
			{
				Spawn();
			}
		}
		break;
	}

	return Spawner::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::ObjectMessageFn
//
//	PURPOSE:	Damaged to death: disable
//
// ----------------------------------------------------------------------- //

uint32 SecRezzer::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	if (messageID != MID_DAMAGE)
	{
		return GameBase::ObjectMessageFn(hSender, pMsg);
	}

	uint32 dwRet = GameBase::ObjectMessageFn(hSender, pMsg);

	if (m_eState != eStateDisabled && m_eState != eStateEnterDisable && m_eState != eStateDisable && m_damage.IsDead())
	{
		m_eState = eStateEnterDisable;
		SetNextUpdate(0.001f);

		LTVector vPos;
		LTRotation rRot;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		if (m_hHook && !m_bLocked)
		{
			m_bLocked = true;
			SendTriggerMsgToObject(this, m_hHook, LTFALSE, "LOCK");
		}
	}

	return dwRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::InitialUpdate
//
//	PURPOSE:	Size us to our animation; set our flags; our fx message
//
// ----------------------------------------------------------------------- //

LTBOOL SecRezzer::InitialUpdate()
{
	m_damage.SetCanDamage(LTFALSE);

	LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	SET_OBJECT_PSETS_USER_FLAGS(m_hObject, m_nPSets);

	// RETAIL: as Optimizer's and IONode's (retail 0x1004546a); not yet
	// identified on the client
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, (1<<1), (1<<1));

	SetNextUpdate(0.001f);

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_SECREZZER_ID);
	m_cs.Write(cMsg);
	m_ActivateTypeHandler.WriteActivateTypeMsg(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::Update
//
//	PURPOSE:	Run the state we are in
//
// ----------------------------------------------------------------------- //

void SecRezzer::Update()
{
	switch (m_eState)
	{
		case eStateIdle:
		{
			// Room for more: unlock, and let a player disable us
			if (CountActive() <= m_nMaxActive - m_nNumberToSpawn)
			{
				if (m_hHook && m_bLocked == true)
				{
					m_bLocked = false;
					SendTriggerMsgToObject(this, m_hHook, LTFALSE, "UNLOCK");
				}

				g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
			}
		}
		break;

		case eStateEnterIdle:
		{
			EnterIdle();
			m_eState = eStateIdle;
		}
		break;

		case eStateEnterSelect:
		{
			EnterSelect();
			m_eState = eStateSelect;
		}
		break;

		case eStateSelect:
		{
			UpdateSelect();
		}
		break;

		case eStateEnterRun:
		{
			EnterRun();
			m_eState = eStateRun;
		}
		break;

		case eStateRun:
		{
			UpdateRun();
		}
		break;

		case eStateLock:
		{
			UpdateLock();
		}
		break;

		case eStateEnterDeselect:
		{
			EnterDeselect();
			m_eState = eStateDeselect;
		}
		break;

		case eStateDeselect:
		{
			UpdateDeselect();
		}
		break;

		case eStateEnterDisable:
		{
			EnterDisable();
			m_eState = eStateDisable;
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
			// RETAIL: a third state of Tron's (retail 0x10045704)
			m_ActivateTypeHandler.SetState((ACTIVATETYPE::State)2);
		}
		break;

		case eStateDisable:
		{
			UpdateDisable();
		}
		break;

		case eStateDisabled:
		{
			SetNextUpdate(0.0f);
		}
		break;
	}

	SetNextUpdate(0.001f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool SecRezzer::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_AddPSet("ADDPSET");
	static CParsedMsg::CToken s_cTok_RemovePSet("REMOVEPSET");
	static CParsedMsg::CToken s_cTok_Idle("IDLE");
	static CParsedMsg::CToken s_cTok_HookSender("HOOKSENDER");

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_User, dwUserFlags);
		if (dwUserFlags & USRFLG_CAN_ACTIVATE)
		{
			CTronPlayerObj* pPlayer = dynamic_cast<CTronPlayerObj*>(g_pLTServer->HandleToObject(hSender));
			if (pPlayer && !pPlayer->IsDead())
			{
				if (HasPSets(pPlayer, m_nPSets))
				{
					if (pPlayer->GetClient())
					{
						if (m_eState >= eStateEnterIdle && m_eState <= eStateIdle)
						{
							m_eState = eStateEnterDisable;
							SetNextUpdate(0.001f);

							CDestructible* pDest = &m_damage;
							pDest->HandleDestruction(pPlayer->GetUserObject());
						}

						return true;
					}
				}
				else
				{
					HOBJECT hObj = m_hObject;
					m_InvalidPSetCommand.Execute(hObj, pPlayer->GetUserObject());

					g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "SecrezzerInvalidPSet");
				}
			}
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_On)
	{
		switch (m_eState)
		{
			case eStateEnterIdle:
			case eStateIdle:
			{
				if (CountActive() <= m_nMaxActive - m_nNumberToSpawn)
				{
					m_nToSpawn = m_nNumberToSpawn;
					m_eState = eStateEnterSelect;
				}
			}
			break;

			case eStateEnterDeselect:
			case eStateDeselect:
			case eStateEnterDisable:
			case eStateDisabled:
				return true;
		}

		if (cMsg.GetArgCount() > 1)
		{
			m_sSpawnName = cMsg.GetArg(1);
		}

		SetNextUpdate(0.001f);
		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
		m_nToSpawn = 0;
		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_Idle)
	{
		if (m_eState >= eStateEnterSelect && m_eState <= eStateLock)
		{
			m_eState = eStateEnterDeselect;
			SetNextUpdate(0.001f);
		}

		return true;
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
	else if (cMsg.GetArg(0) == s_cTok_HookSender)
	{
		m_hHook = hSender;
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::CountActive
//
//	PURPOSE:	How many of our programs are about (forgetting the gone)
//
// ----------------------------------------------------------------------- //

int SecRezzer::CountActive()
{
	int nCount = 0;

	std::slist<LTObjRef>::iterator it = m_lstSpawned.begin();
	while (it != m_lstSpawned.end())
	{
		if (!(*it))
		{
			std::slist<LTObjRef>::iterator itGone = it;
			++it;
			m_lstSpawned.erase(itGone);
		}
		else
		{
			++it;
			nCount++;
		}
	}

	return nCount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::EnterIdle
//
//	PURPOSE:	Idle, closed
//
// ----------------------------------------------------------------------- //

void SecRezzer::EnterIdle()
{
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Idle");
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	g_pLTServer->SetModelLooping(m_hObject, false);

	ObjectCreateStruct ocs;
	strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 0));
	g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::UpdateLock
//
//	PURPOSE:	Rez the rest, or close (locking our hook when full)
//
// ----------------------------------------------------------------------- //

void SecRezzer::UpdateLock()
{
	int nActive = CountActive();

	if (m_nToSpawn > 0)
	{
		m_eState = eStateEnterRun;
		return;
	}

	m_damage.SetCanDamage(LTFALSE);
	m_eState = eStateEnterDeselect;

	if (nActive > m_nMaxActive - m_nNumberToSpawn && m_hHook && !m_bLocked)
	{
		m_bLocked = true;
		SendTriggerMsgToObject(this, m_hHook, LTFALSE, "LOCK");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::EnterSelect
//
//	PURPOSE:	No more disabling; the select animation
//
// ----------------------------------------------------------------------- //

void SecRezzer::EnterSelect()
{
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);

	ObjectCreateStruct ocs;
	strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 2));
	g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Select");
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::UpdateSelect
//
//	PURPOSE:	On to running when selected
//
// ----------------------------------------------------------------------- //

void SecRezzer::UpdateSelect()
{
	if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
	{
		m_eState = eStateEnterRun;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::EnterRun
//
//	PURPOSE:	The run animation; damageable
//
// ----------------------------------------------------------------------- //

void SecRezzer::EnterRun()
{
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Run");
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);

	ObjectCreateStruct ocs;
	strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 2));
	g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);

	m_damage.SetCanDamage(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::UpdateRun
//
//	PURPOSE:	Open when run
//
// ----------------------------------------------------------------------- //

void SecRezzer::UpdateRun()
{
	if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
	{
		m_eState = eStateLock;
		HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Open");
		g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::EnterDeselect
//
//	PURPOSE:	The deselect animation; no more rezzing
//
// ----------------------------------------------------------------------- //

void SecRezzer::EnterDeselect()
{
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Deselect");
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);

	m_nToSpawn = 0;

	ObjectCreateStruct ocs;
	strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 0));
	g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::UpdateDeselect
//
//	PURPOSE:	Idle again when deselected
//
// ----------------------------------------------------------------------- //

void SecRezzer::UpdateDeselect()
{
	if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
	{
		m_eState = eStateIdle;
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::EnterDisable
//
//	PURPOSE:	Disabled: no more activating or damage; tell our hook
//
// ----------------------------------------------------------------------- //

void SecRezzer::EnterDisable()
{
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
	m_damage.SetCanDamage(LTFALSE);

	ObjectCreateStruct ocs;
	strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 1));
	g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Disable");
	g_pLTServer->SetModelAnimation(m_hObject, hAnim);

	if (m_hHook)
	{
		SendTriggerMsgToObject(this, m_hHook, LTFALSE, "DISABLE");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::UpdateDisable
//
//	PURPOSE:	Stay disabled when the animation is done
//
// ----------------------------------------------------------------------- //

void SecRezzer::UpdateDisable()
{
	if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
	{
		HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Disabled");
		g_pLTServer->SetModelAnimation(m_hObject, hAnim);
		m_eState = eStateDisabled;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::Spawn
//
//	PURPOSE:	Rez a program at a free rez point
//
// ----------------------------------------------------------------------- //

void SecRezzer::Spawn()
{
	char szTarget[1024] = "";
	strncpy(szTarget, m_sTarget.c_str(), ARRAY_LEN(szTarget) - 1);
	szTarget[ARRAY_LEN(szTarget) - 1] = '\0';

	const char* pszName = m_sSpawnName.c_str();

	if (m_sSpawnSound.c_str())
	{
		g_pServerSoundMgr->PlaySoundFromObject(m_hObject, m_sSpawnSound.c_str(), m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
	}

	uint32 nPoints = m_lstPoints.size();
	if (m_lstFreePoints.size() != nPoints)
	{
		m_lstFreePoints.resize(nPoints, SecRezzerPointInfo());
	}

	// The points no one stands at
	uint32 nFree = 0;
	uint32 nIndex = 0;
	for (uint32 i = 0; i < nPoints; i++)
	{
		if (!g_pCharacterMgr->FindCharactersWithinRadius(LTNULL, m_lstPoints[i].vPos, 36.0f))
		{
			m_lstFreePoints[nFree] = m_lstPoints[i];
			nFree++;
		}
	}

	if (nFree > 1)
	{
		nIndex = GetRandom(0, nFree - 1);
	}

	if (nFree > 0)
	{
		BaseClass* pObj = SpawnObjectTemplate(szTarget, pszName, m_lstFreePoints[nIndex].vPos, m_lstFreePoints[nIndex].rRot);

		ObjectCreateStruct ocs;
		strcpy(ocs.m_SkinNames[0], g_pModelButeMgr->GetSkinFilename(m_cs.eModelId, 3));
		g_pCommonLT->SetObjectFilenames(m_hObject, &ocs);

		if (pObj)
		{
			m_lstSpawned.push_front(LTObjRef(pObj->m_hObject));

			if (m_sInitialCommand.c_str() && g_pCmdMgr->IsValidCmd(m_sInitialCommand.c_str()))
			{
				g_pCmdMgr->Process(m_sInitialCommand.c_str(), m_hObject, pObj->m_hObject);
			}

			m_nToSpawn--;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SecRezzer::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	pMsg->Writeint8((int8)m_eState);
	SAVE_INT(m_nToSpawn);
	SAVE_INT(m_nMaxActive);
	SAVE_STDSTRING(m_sSpawnName);
	pMsg->Writeint8((int8)m_nNumberToSpawn);
	SAVE_BYTE(m_nPSets);
	SAVE_HOBJECT(m_hHook);
	SAVE_bool(m_bLocked);

	pMsg->Writeuint32(m_lstSpawned.size());
	for (std::slist<LTObjRef>::iterator itSpawned = m_lstSpawned.begin(); itSpawned != m_lstSpawned.end(); ++itSpawned)
	{
		SAVE_HOBJECT(*itSpawned);
	}

	pMsg->Writeuint32(m_lstPoints.size());
	SecRezzerPointList::iterator it;
	for (it = m_lstPoints.begin(); it != m_lstPoints.end(); ++it)
	{
		SAVE_VECTOR(it->vPos);
		SAVE_ROTATION(it->rRot);
	}

	pMsg->Writeuint32(m_lstFreePoints.size());
	for (it = m_lstFreePoints.begin(); it != m_lstFreePoints.end(); ++it)
	{
		pMsg->WriteType(it->vPos);
		pMsg->WriteType(it->rRot);
	}

	m_cs.Write(pMsg);
	m_InvalidPSetCommand.Save(pMsg);
	m_ActivateTypeHandler.Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SecRezzer::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_eState = pMsg->Readint8();
	LOAD_INT(m_nToSpawn);
	LOAD_INT(m_nMaxActive);

	char szString[1024];
	LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
	m_sSpawnName = szString;

	m_nNumberToSpawn = pMsg->Readint8();
	LOAD_BYTE(m_nPSets);
	LOAD_HOBJECT(m_hHook);
	LOAD_bool(m_bLocked);

	uint32 nCount = pMsg->Readuint32();
	uint32 i;
	for (i = 0; i < nCount; i++)
	{
		m_lstSpawned.push_front(LTObjRef(pMsg->ReadObject()));
	}

	nCount = pMsg->Readuint32();
	m_lstPoints.resize(nCount, SecRezzerPointInfo());
	for (i = 0; i < nCount; i++)
	{
		pMsg->ReadType(&m_lstPoints[i].vPos);
		LTRotation rRot;
		pMsg->ReadType(&rRot);
		m_lstPoints[i].rRot = rRot;
	}

	nCount = pMsg->Readuint32();
	m_lstFreePoints.resize(nCount, SecRezzerPointInfo());
	for (i = 0; i < nCount; i++)
	{
		pMsg->ReadType(&m_lstFreePoints[i].vPos);
		LTRotation rRot;
		pMsg->ReadType(&rRot);
		m_lstFreePoints[i].rRot = rRot;
	}

	m_cs.Read(pMsg);
	m_InvalidPSetCommand.Load(pMsg);
	m_ActivateTypeHandler.Load(pMsg);

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_SECREZZER_ID);
	m_cs.Write(cMsg);
	m_ActivateTypeHandler.WriteActivateTypeMsg(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}
