// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycle.cpp
//
// PURPOSE : The model of a light cycle in the light cycle game
//
// RETAIL: Tron 2.0's, written from retail Object.lto (LightCycle.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycle.h"
#include "LightCycleMgr.h"
#include "PlayerObj.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "ServerUtilities.h"
#include "CommonUtilities.h"

// RETAIL: Tron's special fx id of a light cycle (0x10025642; NOLF2 has
// SFX_DOOMSDAYPIECE_ID there). The name is ours.
#define SFX_LIGHTCYCLE_ID		58

BEGIN_CLASS(LightCycle)
	ADD_STRINGPROP_FLAG(CycleColor, "LightCycleRed", PF_STATICLIST)
	ADD_STRINGPROP(RezInFXDoneCommand, "")
END_CLASS_DEFAULT_FLAGS_PLUGIN(LightCycle, GameBase, NULL, NULL, 0, CLCPlugin)

bool				CLCPlugin::sm_bInitted = false;
CLightCycleButeMgr	CLCPlugin::sm_ButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLCPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill in the cycle colors
//
// ----------------------------------------------------------------------- //

LTRESULT CLCPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!sm_bInitted)
	{
		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, LCBMGR_DEFAULT_FILE);
		sm_ButeMgr.SetInRezFile(LTFALSE);
		sm_ButeMgr.Init(szFile);
		sm_bInitted = true;
	}

	if (_strcmpi("CycleColor", szPropName) == 0)
	{
		uint32 i;
		for (i = 0; i < (uint32)sm_ButeMgr.GetNumCycleColors(); i++)
		{
			LIGHTCYCLECOLOR* pColor = sm_ButeMgr.GetCycleColor(i);
			if (pColor && strlen(pColor->szName) < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pColor->szName);
			}
		}

		if (i > 0)
		{
			qsort(aszStrings, *pcStrings, sizeof(char *), CaseInsensitiveCompare);
		}
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::LightCycle()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

LightCycle::LightCycle() : GameBase(OT_MODEL)
{
	m_hstrRezInFXDoneCommand = LTNULL;
	m_hPlayer = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::~LightCycle()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

LightCycle::~LightCycle()
{
	FREE_HSTRING(m_hstrRezInFXDoneCommand);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 LightCycle::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
//	ROUTINE:	LightCycle::ObjectMessageFn
//
//	PURPOSE:	Handle object messages: damage derezzes the rider
//
// ----------------------------------------------------------------------- //

uint32 LightCycle::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	if (messageID == MID_DAMAGE)
	{
		LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(m_hPlayer);
		if (pCyclist)
		{
			g_pLightCycleMgr->DerezCyclist(pCyclist, LTNULL, true);
		}
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL LightCycle::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return LTFALSE;

	GenericProp genProp;

	m_szCycleColor[0] = '\0';
	if (g_pLTServer->GetPropGeneric("CycleColor", &genProp) == LT_OK)
	{
		strcpy(m_szCycleColor, genProp.m_String);
	}

	if (g_pLTServer->GetPropGeneric("RezInFXDoneCommand", &genProp) == LT_OK)
	{
		m_hstrRezInFXDoneCommand = g_pLTServer->CreateString(genProp.m_String);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::Init
//
//	PURPOSE:	Set the cycle up for its rider
//
// ----------------------------------------------------------------------- //

void LightCycle::Init(CPlayerObj* pPlayer)
{
	SetModel();

	uint8 nClientID;
	if (pPlayer)
	{
		nClientID = (uint8)g_pLTServer->GetClientID(pPlayer->GetClient());
		m_hPlayer = pPlayer->m_hObject;
	}
	else
	{
		nClientID = 0xff;
		m_hPlayer = LTNULL;
	}
	m_cs.nClientID = nClientID;

	LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	MoveObjectToFloor(m_hObject);

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::SetModel
//
//	PURPOSE:	Use the model and skins of the cycle's color
//
// ----------------------------------------------------------------------- //

void LightCycle::SetModel()
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetCycleColor(m_szCycleColor);

	// RETAIL: the type is looked up (and unused) before the color is
	// checked (0x10025503); see docs/RETAIL_BUGS.md
	g_pLightCycleButeMgr->GetCycleType(pColor->nCycleType);

	if (!pColor)
	{
		g_pLTServer->CPrint("ERROR - Couldn't find Light Cycle Color %s\n", m_szCycleColor);
		return;
	}

	SAFE_STRCPY(theStruct.m_Filename, pColor->szModel);

	pColor->blrRenderStyles.CopyList(0, theStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);
	pColor->blrSkins.CopyList(0, theStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);

	g_pCommonLT->SetObjectFilenames(m_hObject, &theStruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::InitialUpdate
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL LightCycle::InitialUpdate()
{
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE | FLAG_FULLPOSITIONRES | FLAG_TOUCH_NOTIFY | FLAG_FORCECLIENTUPDATE, FLAGMASK_ALL);
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::CreateSpecialFX
//
//	PURPOSE:	Set the special fx message and tell the clients
//
// ----------------------------------------------------------------------- //

void LightCycle::CreateSpecialFX()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_LIGHTCYCLE_ID);
	m_cs.Write(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());

	cMsg.Reset();
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_LIGHTCYCLE_ID);
	cMsg.WriteObject(m_hObject);
	m_cs.Write(cMsg);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void LightCycle::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	pMsg->WriteString(m_szCycleColor);
	pMsg->WriteHString(m_hstrRezInFXDoneCommand);
	m_cs.Write(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void LightCycle::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	pMsg->ReadString(m_szCycleColor, ARRAY_LEN(m_szCycleColor));
	m_hstrRezInFXDoneCommand = pMsg->ReadHString();
	m_cs.Read(pMsg);
}
