// ----------------------------------------------------------------------- //
//
// MODULE  : LightCyclePowerup.cpp
//
// PURPOSE : A power-up of the light cycle game, picked up by touch
//
// RETAIL: Tron 2.0's, written from retail Object.lto (LightCyclePowerup.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCyclePowerup.h"
#include "ServerLightCycleMgr.h"
#include "ServerUtilities.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "GameServerShell.h"

extern CGameServerShell* g_pGameServerShell;

static uint32 GetPowerupFlag(int nType);
static int GetPowerupType(const char* szName);

BEGIN_CLASS(LightCyclePowerup)
	ADD_VECTORPROP_VAL_FLAG(Dims, 97.0f, 32.0f, 97.0f, PF_HIDDEN | PF_DIMS)
	// RETAIL: with PF_LOCALDIMS too (0x1002ac2f)
	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST | PF_LOCALDIMS)
	ADD_REALPROP(RespawnTime, -1.0f)
	ADD_STRINGPROP(PickupCommand, "")
	ADD_BOOLPROP(AllowInCustom, LTTRUE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(LightCyclePowerup, GameBase, NULL, NULL, 0, CLightCyclePowerupPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCyclePowerupPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill in the power-up types
//
// ----------------------------------------------------------------------- //

LTRESULT CLightCyclePowerupPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (_strcmpi("Type", szPropName) == 0)
	{
		for (int i = 0; i < kNumLCPowerupTypes; i++)
		{
			strcpy(aszStrings[(*pcStrings)++], g_aszLCPowerupNames[i]);
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::LightCyclePowerup()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

LightCyclePowerup::LightCyclePowerup() : GameBase(OT_NORMAL)
{
	m_hFX = LTNULL;
	m_bFromWorld = true;
	m_hstrPickupCommand = LTNULL;
	m_fRespawnTime = -1.0f;
	m_nType = kLCPowerup_Random;
	m_bAllowInCustom = true;
	m_nTronUnknown5C = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::~LightCyclePowerup()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

LightCyclePowerup::~LightCyclePowerup()
{
	if (m_hFX)
	{
		if (g_pLightCycleMgr)
		{
			((CServerLightCycleMgr*)g_pLightCycleMgr)->RemovePowerup(m_hObject);
		}

		g_pLTServer->RemoveObject(m_hFX);
		m_hFX = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetAllowedPowerups
//
//	PURPOSE:	The power-ups the game allows, a bit each (GetPowerupFlag)
//
//	RETAIL: inline (0x1002b0c1, 0x1002bab1; the name is ours). PORT: in a
//	multiplayer light cycle game retail reads its light cycle game's copy
//	(Object.lto 0x1035a574, +0x88; that class is not written yet) of the
//	server options' m_nLCPowerups, read here instead.
//
// ----------------------------------------------------------------------- //

inline uint32 GetAllowedPowerups()
{
	uint32 dwAllowed = (uint32)-1;

	switch (GetGameType())
	{
		case eGameTypeSingleLightCycleCustom:
		{
			// RETAIL: the custom game's settings, their power-ups at +0x2c
			ServerGameOptions& options = g_pGameServerShell->GetServerGameOptions();
			if (options.GetTronUnknownCC())
			{
				dwAllowed = ((uint32*)options.GetTronUnknownCC())[0x2c / sizeof(uint32)];
			}
			else
			{
				dwAllowed = (uint32)-1;
			}
		}
		break;

		case eGameTypeMultiplayerLightcycles:
		{
			dwAllowed = g_pGameServerShell->GetServerGameOptions().m_nLCPowerups;
		}
		break;
	}

	return dwAllowed;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 LightCyclePowerup::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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

			((CServerLightCycleMgr*)g_pLightCycleMgr)->AddPowerup(this);

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwAllowed = GetAllowedPowerups();

			if (m_nType != kLCPowerup_Random || !dwAllowed)
			{
				uint32 dwFlag = GetPowerupFlag(m_nType);

				if ((GetGameType() == eGameTypeSingleLightCycleCustom && !m_bAllowInCustom) ||
					!(dwAllowed & dwFlag) || (m_nType == kLCPowerup_Random && !dwAllowed))
				{
					// Not in this game
					((CServerLightCycleMgr*)g_pLightCycleMgr)->RemovePowerup(m_hObject);
					g_pLTServer->RemoveObject(m_hObject);
					return 0;
				}
			}

			if (m_bFromWorld)
			{
				// Drop it to the floor
				LTVector vDims(97.0f, 32.0f, 97.0f);
				g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject, &vPos);

				IntersectQuery IQuery;
				IntersectInfo IInfo;

				IQuery.m_From = vPos;
				IQuery.m_To = LTVector(vPos.x, vPos.y - 10000.0f, vPos.z);
				IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

				if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
				{
					vPos.y -= (vPos.y - IInfo.m_Point.y) - (vDims.y + 0.1f);
					g_pLTServer->SetObjectPos(m_hObject, &vPos);
				}
			}

			CreateFX();
			return 0;
		}
		break;

		case MID_UPDATE:
		{
			// Respawn
			SetEnabled(true);
			return 0;
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			ObjectTouch((HOBJECT)pData);
			return 0;
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
//	ROUTINE:	GetPowerupFlag
//
//	PURPOSE:	A power-up type's bit in the allowed power-ups (none for
//				Random)
//
//	RETAIL: 0x1002b390 (the name is ours)
//
// ----------------------------------------------------------------------- //

static uint32 GetPowerupFlag(int nType)
{
	switch (nType)
	{
		case kLCPowerup_Shield:			return (1<<0);
		case kLCPowerup_Turbo:			return (1<<1);
		case kLCPowerup_TrailSpike:		return (1<<2);
		case kLCPowerup_Missile:		return (1<<3);
		case kLCPowerup_WallExtender:	return (1<<4);
		case kLCPowerup_TurboCurse:		return (1<<5);
		case kLCPowerup_Grab:			return (1<<6);
		case kLCPowerup_TrailReset:		return (1<<7);
		case kLCPowerup_ForceUse:		return (1<<8);
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

void LightCyclePowerup::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("Type", &genProp) == LT_OK && genProp.m_String[0])
	{
		int nType = GetPowerupType(genProp.m_String);
		if (nType != -1)
		{
			m_nType = nType;
		}
	}

	if (g_pLTServer->GetPropGeneric("PickupCommand", &genProp) == LT_OK && genProp.m_String[0])
	{
		m_hstrPickupCommand = g_pLTServer->CreateString(genProp.m_String);
	}

	if (g_pLTServer->GetPropGeneric("RespawnTime", &genProp) == LT_OK && genProp.m_Float != 0.0f)
	{
		m_fRespawnTime = genProp.m_Float;
	}

	// RETAIL: only ever set when true, so AllowInCustom cannot be turned
	// off (see docs/RETAIL_BUGS.md)
	if (g_pLTServer->GetPropGeneric("AllowInCustom", &genProp) == LT_OK && genProp.m_Bool)
	{
		m_bAllowInCustom = genProp.m_Bool;
	}

	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetPowerupType
//
//	PURPOSE:	A power-up type from its name (-1 if none)
//
//	RETAIL: 0x1002b530 (the name is ours)
//
// ----------------------------------------------------------------------- //

static int GetPowerupType(const char* szName)
{
	for (int i = 0; i < kNumLCPowerupTypes; i++)
	{
		if (_stricmp(szName, g_aszLCPowerupNames[i]) == 0)
			return i;
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::ObjectTouch
//
//	PURPOSE:	Give the power-up to the cycle
//
// ----------------------------------------------------------------------- //

void LightCyclePowerup::ObjectTouch(HOBJECT hObj)
{
	if (!hObj) return;

	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(hObj);
	if (!pCyclist) return;

	int nType = GetType();
	if (!g_pLightCycleMgr->SetPowerup(pCyclist, nType)) return;

	char szFX[64];
	sprintf(szFX, "%s%s%s", LCPOWERUP_SOUND_PREFIX, g_aszLCPowerupNames[m_nType], LCPOWERUP_SOUND_TOUCH);
	PlayClientFX(szFX, m_hObject);

	if (m_hstrPickupCommand)
	{
		const char* szCmd = g_pLTServer->GetStringData(m_hstrPickupCommand);
		if (g_pCmdMgr->IsValidCmd(szCmd))
		{
			g_pCmdMgr->Process(szCmd, m_hObject, (HOBJECT)LTNULL);
		}
		else
		{
			g_pLTServer->CPrint("LightCyclePowerup::ObjectTouch - ERROR - Invalid command: %s\n", szCmd);
		}
	}

	if (m_fRespawnTime != 0.0f)
	{
		SetEnabled(false);

		if (m_fRespawnTime > 0.0f)
		{
			SetNextUpdate(m_fRespawnTime);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void LightCyclePowerup::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_HOBJECT(m_hFX);
	SAVE_HSTRING(m_hstrPickupCommand);
	SAVE_FLOAT(m_fRespawnTime);
	SAVE_BYTE(m_nType);
	SAVE_bool(m_bEnabled);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void LightCyclePowerup::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_HOBJECT(m_hFX);
	LOAD_HSTRING(m_hstrPickupCommand);
	LOAD_FLOAT(m_fRespawnTime);
	LOAD_BYTE(m_nType);
	LOAD_bool(m_bEnabled);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::CreateFX
//
//	PURPOSE:	The power-up's looping client fx
//
// ----------------------------------------------------------------------- //

void LightCyclePowerup::CreateFX()
{
	char szFX[1024];
	sprintf(szFX, "FxName %s%s%s;Loop 1;SmoothShutdown 0", LCPOWERUP_SOUND_PREFIX, g_aszLCPowerupNames[m_nType], LCPOWERUP_SOUND_PICKUP);
	m_hFX = CreateClientFXObject(szFX, m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::SetEnabled
//
//	PURPOSE:	Show the power-up and make it touchable, or not
//
// ----------------------------------------------------------------------- //

void LightCyclePowerup::SetEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		SendTriggerMsgToObject(this, m_hFX, LTTRUE, "ON");
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY);
	}
	else
	{
		SendTriggerMsgToObject(this, m_hFX, LTTRUE, "OFF");
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY);
	}

	m_bEnabled = bEnabled;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool LightCyclePowerup::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if (cMsg.GetArg(0) == s_cTok_On)
	{
		SetEnabled(true);
		SetNextUpdate(0.0f);
		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
		SetEnabled(false);
		SetNextUpdate(0.0f);
		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_Remove)
	{
		((CServerLightCycleMgr*)g_pLightCycleMgr)->RemovePowerup(m_hObject);
		g_pLTServer->RemoveObject(m_hObject);
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCyclePowerup::GetType
//
//	PURPOSE:	The power-up it gives
//
// ----------------------------------------------------------------------- //

int LightCyclePowerup::GetType()
{
	if (m_nType != kLCPowerup_Random)
		return m_nType;

	std::list<int> lstTypes;

	uint32 dwAllowed = GetAllowedPowerups();

	if (dwAllowed & (1<<kLCPowerup_Shield))			lstTypes.push_back(kLCPowerup_Shield);
	if (dwAllowed & (1<<kLCPowerup_Turbo))			lstTypes.push_back(kLCPowerup_Turbo);
	if (dwAllowed & (1<<kLCPowerup_TrailSpike))		lstTypes.push_back(kLCPowerup_TrailSpike);
	if (dwAllowed & (1<<kLCPowerup_Missile))			lstTypes.push_back(kLCPowerup_Missile);
	if (dwAllowed & (1<<kLCPowerup_TurboCurse))		lstTypes.push_back(kLCPowerup_TurboCurse);
	if (dwAllowed & (1<<kLCPowerup_Grab))			lstTypes.push_back(kLCPowerup_Grab);
	if (dwAllowed & (1<<kLCPowerup_ForceUse))		lstTypes.push_back(kLCPowerup_ForceUse);
	if (dwAllowed & (1<<kLCPowerup_TrailReset))		lstTypes.push_back(kLCPowerup_TrailReset);
	if (dwAllowed & (1<<kLCPowerup_WallExtender))	lstTypes.push_back(kLCPowerup_WallExtender);

	std::list<int>::iterator iter = lstTypes.begin();
	std::advance(iter, GetRandom(0, lstTypes.size() - 1));
	return *iter;
}
