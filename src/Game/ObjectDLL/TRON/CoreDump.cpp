/****************************************************************************
;
;	MODULE:			CoreDump.cpp
;
;	PURPOSE:		Core Dump object class for TRON
;
;	HISTORY:		03/22/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

// RETAIL: written from retail Object.lto (CoreDump.h); Build 69's core dump
// is gone.

#include "stdafx.h"
#include "CoreDump.h"
#include "TronPlayerObj.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "MsgIDs.h"
#include "ServerUtilities.h"
#include "InventoryButeMgr.h"
#include "ModelButeMgr.h"
#include "ServerSoundMgr.h"
#include "GameServerShell.h"
#include "ServerSubroutineMgr.h"
#include "SubroutineDataMgr.h"
#include "physics/math_phys.h"

BEGIN_CLASS(CoreDump)
END_CLASS_DEFAULT_FLAGS(CoreDump, GameBase, NULL, NULL, CF_HIDDEN)

// RETAIL: Tron's message ids and values (the names are ours)
#define MID_TRON_ENERGY_GAINED			164		// Server to client
#define TARGET_PROP_COREDUMP			4		// MID_QUERY_TARGET_PROPERTIES
#define DECISION_TYPE_COREDUMP			3		// MID_DECISION's first byte

// The decision window's answers besides an item
#define CD_CHOICE_DESTROY				0xfc
#define CD_CHOICE_CANCEL				0xfd

#define CD_MAX_CHOICES					6

// How long the core dump and its ribbons last
const float g_fCoreDumpLifeTime = 50.0f;
const float g_fCoreDumpRibbonTime = 29.9f;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetRibbonColor
//
//	PURPOSE:	The tetras' color, fading through four colors over the
//				core dump's life
//
// ----------------------------------------------------------------------- //

static LTVector GetRibbonColor(float fDuration, float fTime)
{
	static LTVector s_avColors[4] =
	{
		LTVector(1.0f, 1.0f, 64.0f / 255.0f),
		LTVector(1.0f, 128.0f / 255.0f, 64.0f / 255.0f),
		LTVector(128.0f / 255.0f, 96.0f / 255.0f, 32.0f / 255.0f),
		LTVector(32.0f / 255.0f, 32.0f / 255.0f, 96.0f / 255.0f),
	};

	float fSegment = fDuration * (1.0f / 3.0f);
	int nColor = (int)(fTime / fSegment);
	float fT = fmod(fTime, fSegment) / fSegment;

	LTVector vColor;
	vColor.x = (s_avColors[nColor + 1].x - s_avColors[nColor].x) * fT + s_avColors[nColor].x;
	vColor.y = (s_avColors[nColor + 1].y - s_avColors[nColor].y) * fT + s_avColors[nColor].y;
	vColor.z = (s_avColors[nColor + 1].z - s_avColors[nColor].z) * fT + s_avColors[nColor].z;

	return vColor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::CoreDump()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //
CoreDump::CoreDump() : GameBase(OT_NORMAL)
{
	for (int i = 0; i < CD_MAX_ITEMS; i++)
	{
		m_aItems[i].Clear();
		m_aeShownCondition[i] = kAIC_Normal;
	}

	m_fTotalEnergy = 0.0f;
	m_fEnergy = 0.0f;
	m_fTotalHealth = 0.0f;
	m_fHealth = 0.0f;
	m_bActive = false;
	m_bShown = false;
	m_pPlayer = LTNULL;
	m_hRibbons = LTNULL;
	m_hTetras = LTNULL;

	m_ActivateTypeHandler.Init(this, 0x39);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::~CoreDump()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
CoreDump::~CoreDump()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //
uint32 CoreDump::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData != PRECREATE_SAVEGAME)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				pStruct->m_Flags |= FLAG_RAYHIT | FLAG_FORCECLIENTUPDATE;
				pStruct->m_Flags2 |= FLAG2_SERVERDIMS;

				m_ActivateTypeHandler.SetActivateType("Open Coredump");
				m_ActivateTypeHandler.SetState(ACTIVATETYPE::eOn);
			}

			return GameBase::EngineMessageFn(messageID, pData, fData);
		}

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

		case MID_UPDATE:
		{
			if (Update())
				return LT_OK;

			// RETAIL: no break: when Update fails it falls into
			// MID_INITIALUPDATE (it never does)
		}

		case MID_INITIALUPDATE:
		{
			LTVector vDims;
			vDims.x = vDims.y = vDims.z = 10.0f;
			g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

			m_bActive = false;

			// Pick what a virus scan shows of each item's condition
			for (int i = 0; i < CD_MAX_ITEMS; i++)
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

			m_ActivateTypeHandler.CreateActivateTypeMsg();
		}
		break;

		default: break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CoreDump::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
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
						case TARGET_PROP_COREDUMP:
						{
							cTempMsg.Writeuint8(iProp);
							cTempMsg.Writeuint32(0);
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
//	ROUTINE:	CoreDump::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CoreDump::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fTotalEnergy);
	SAVE_FLOAT(m_fEnergy);
	SAVE_FLOAT(m_fTotalHealth);
	SAVE_FLOAT(m_fHealth);

	// RETAIL: four, one past the array: m_nNumItems goes too
	int i;
	for (i = 0; i < CD_MAX_ITEMS + 1; i++)
	{
		pMsg->Writeint8(m_aeShownCondition[i]);
	}

	SAVE_HOBJECT(m_hRibbons);
	SAVE_HOBJECT(m_hTetras);

	m_LifeTimer.Save(pMsg);
	m_RibbonTimer.Save(pMsg);

	SAVE_BYTE(m_bShown);

	for (i = 0; i < CD_MAX_ITEMS; i++)
	{
		ArchiveItem& item = m_aItems[i];
		pMsg->Writeint8(item.m_eType);
		SAVE_CHARSTRING(item.m_sName.c_str());
		pMsg->Writeint8(item.m_eBuild);
		pMsg->Writeint8(item.m_eCondition);
		SAVE_CHARSTRING(item.m_sCommand.c_str());
		SAVE_BYTE(item.m_nPSet);
		SAVE_FLOAT(item.m_fCost);
	}

	m_ActivateTypeHandler.Save(pMsg);

	if (m_bShown)
	{
		SetNextUpdate(0.001f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CoreDump::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	char szString[256];

	LOAD_FLOAT(m_fTotalEnergy);
	LOAD_FLOAT(m_fEnergy);
	LOAD_FLOAT(m_fTotalHealth);
	LOAD_FLOAT(m_fHealth);

	// RETAIL: four, one past the array (see Save)
	int i;
	for (i = 0; i < CD_MAX_ITEMS + 1; i++)
	{
		LOAD_BYTE(m_aeShownCondition[i]);
	}

	LOAD_HOBJECT(m_hRibbons);
	LOAD_HOBJECT(m_hTetras);

	m_LifeTimer.Load(pMsg);
	m_RibbonTimer.Load(pMsg);

	LOAD_BYTE(m_bShown);

	for (i = 0; i < CD_MAX_ITEMS; i++)
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
	}

	m_ActivateTypeHandler.Load(pMsg);

	if (GetNumItems())
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

		// RETAIL: Tron's third activate state
		m_ActivateTypeHandler.SetState((ACTIVATETYPE::State)2);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::GetNumItems
//
//	PURPOSE:	How many items we still hold
//
// ----------------------------------------------------------------------- //

int CoreDump::GetNumItems()
{
	int nNumItems = 0;
	for (int i = 0; i < CD_MAX_ITEMS; i++)
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
//	ROUTINE:	CoreDump::CreateTetras
//
//	PURPOSE:	Create the tetras model, one tetra showing per item
//
// ----------------------------------------------------------------------- //

void CoreDump::CreateTetras()
{
	ObjectCreateStruct ocs;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	ModelId eModelId = g_pModelButeMgr->GetModelId("CoreDump");
	const char* pszFilename = g_pModelButeMgr->GetModelFilename(eModelId);
	if (pszFilename && pszFilename[0])
	{
		SAFE_STRCPY(ocs.m_Filename, pszFilename);
	}
	g_pModelButeMgr->CopySkinFilenames(eModelId, 0, ocs.m_SkinNames[0], ARRAY_LEN(ocs.m_SkinNames[0]));
	g_pModelButeMgr->CopyRenderStyleFilenames(eModelId, &ocs);

	ocs.m_Pos = vPos;
	ocs.m_Flags = FLAG_VISIBLE;
	ocs.m_ObjectType = OT_MODEL;

	HCLASS hClass = g_pLTServer->GetClass("BaseClass");
	m_hTetras = g_pLTServer->ObjectToHandle(g_pLTServer->CreateObject(hClass, &ocs));

	// Hide the tetras of the items we don't have
	char szPiece[256];
	HMODELPIECE hPiece;
	for (uint32 i = m_nNumItems; i < CD_MAX_ITEMS; i++)
	{
		sprintf(szPiece, "tetra%d", i + 1);
		g_pModelLT->GetPiece(m_hTetras, szPiece, hPiece);
		g_pModelLT->SetPieceHideStatus(m_hTetras, hPiece, true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::CreateFX
//
//	PURPOSE:	Create a looping SpecialFX object where we are
//
// ----------------------------------------------------------------------- //

HOBJECT CoreDump::CreateFX(const char* pszFXName, bool bSmoothShutdown)
{
	ObjectCreateStruct ocs;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	char szProps[512];
	sprintf(szProps, "FxName %s;Loop 1;SmoothShutdown %d", pszFXName, bSmoothShutdown ? 1 : 0);

	INIT_OBJECTCREATESTRUCT(ocs);
	ocs.m_Rotation.Init();
	ocs.m_Pos = vPos;

	HCLASS hClass = g_pLTServer->GetClass("SpecialFX");
	if (hClass)
	{
		BaseClass* pObj = (BaseClass*)g_pLTServer->CreateObjectProps(hClass, &ocs, szProps);
		return pObj->m_hObject;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool CoreDump::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_Show("SHOW");

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		HandleActivate(hSender);
		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_Show)
	{
		if (!m_bShown)
		{
			m_RibbonTimer.Start(g_fCoreDumpRibbonTime);
			m_LifeTimer.Start(g_fCoreDumpLifeTime);

			m_hRibbons = CreateFX("CoreDump_RibbonsFade", false);

			if (m_nNumItems)
			{
				CreateTetras();
			}

			LTVector vDims;
			vDims.x = vDims.y = vDims.z = 10.0f;
			g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

			m_bShown = true;
			SetNextUpdate(0.001f);
		}

		return false;
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::HandleActivate
//
//	PURPOSE:	A player opened us: give him our energy and health and show
//				him what we hold
//
// ----------------------------------------------------------------------- //
void CoreDump::HandleActivate(HOBJECT hSender)
{
	// Sanity checks
	if(m_bActive)
	{
		// We should never get an activate message when we're already activated
		g_pLTServer->CPrint("ERROR - CoreDump received an activate message when it was already being activated!\n");
		return;
	}

	if(!IsPlayer(hSender))
	{
		g_pLTServer->CPrint("ERROR - CoreDump received an activate message from an object other than a player!\n");
		return;
	}

	// We're all good
	m_pPlayer = (CTronPlayerObj*) g_pLTServer->HandleToObject(hSender);
	m_bActive = true;

	if (m_hRibbons)
	{
		m_RibbonTimer.Stop();

		SendTriggerMsgToObject(this, m_hRibbons, LTFALSE, "OFF");
		PlayClientFX("CoreDump_Ribbons_Destroy", m_hObject);
		g_pLTServer->RemoveObject(m_hRibbons);
		m_hRibbons = LTNULL;
	}

	// Stop the clock while the player looks
	m_LifeTimer.Pause();
	SetNextUpdate(0.0f);

	CDestructible* pDest = m_pPlayer->GetDestructible();

	if (m_fEnergy > 0.0f)
	{
		float fEnergy = pDest->GetEnergy() + m_fEnergy;
		float fMaxEnergy = pDest->GetMaxEnergy();
		pDest->SetEnergy(Clamp(fEnergy, 0.0f, fMaxEnergy));

		// RETAIL: measured against fEnergy, which already holds ours: never
		// more than 0, so the message is never sent
		float fGained = pDest->GetEnergy() - fEnergy;
		if (fGained > 1.0f)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_ENERGY_GAINED);
			cMsg.Writeuint8((uint8)fGained);
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
		}

		m_fEnergy = 0.0f;
	}

	if (m_fHealth > 0.0f)
	{
		float fHitPoints = pDest->GetHitPoints();
		float fMaxHitPoints = pDest->GetMaxHitPoints();

		fHitPoints += m_fHealth;
		if (fHitPoints <= 0.0f)
		{
			fHitPoints = 0.0f;
		}
		if (fMaxHitPoints < fHitPoints)
		{
			fHitPoints = fMaxHitPoints;
		}

		pDest->SetHitPoints(fHitPoints);
		m_fHealth = 0.0f;
	}

	ShowContents();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::HandleStopActivate
//
//	PURPOSE:	The player closed us
//
// ----------------------------------------------------------------------- //
void CoreDump::HandleStopActivate()
{
	m_pPlayer = LTNULL;
	m_bActive = false;

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

	m_RibbonTimer.Resume();
	m_LifeTimer.Resume();

	SetNextUpdate(0.001f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::Update
//
//	PURPOSE:	Decay, fade, and go away when our time is up
//
// ----------------------------------------------------------------------- //
LTBOOL CoreDump::Update()
{
	if (m_fTotalEnergy <= 0.0f)
	{
		SetNextUpdate(0.0f);
		return LTTRUE;
	}

	m_fEnergy = Max(0.0f, m_fEnergy - g_pLTServer->GetFrameTime());
	if (m_fTotalEnergy != 0.0f)
	{
		m_fHealth = m_fEnergy / m_fTotalEnergy * m_fTotalHealth;
	}

	if (m_RibbonTimer.Stopped() && m_hRibbons)
	{
		SendTriggerMsgToObject(this, m_hRibbons, LTFALSE, "OFF");
		g_pLTServer->RemoveObject(m_hRibbons);
		PlayClientFX("CoreDump_Ribbons_Destroy", m_hObject);
	}

	if (!m_LifeTimer.Stopped())
	{
		LTVector vColor;
		vColor = GetRibbonColor(g_fCoreDumpLifeTime, m_LifeTimer.GetElapseTime());
		if (m_hTetras)
		{
			float fTimeLeft = m_LifeTimer.GetCountdownTime();
			float fAlpha = (fTimeLeft > 5.0f) ? 1.0f : fTimeLeft * 0.2f;
			g_pLTServer->SetObjectColor(m_hTetras, vColor.x, vColor.y, vColor.z, fAlpha);
		}

		SetNextUpdate(0.001f);
		return LTTRUE;
	}

	Term();
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::Term
//
//	PURPOSE:	When it's time for us to remove ourselves
//
// ----------------------------------------------------------------------- //
void CoreDump::Term()
{
	if (m_hRibbons)
	{
		g_pLTServer->RemoveObject(m_hRibbons);
	}

	if (m_hTetras)
	{
		g_pLTServer->RemoveObject(m_hTetras);
		PlayClientFX("CoreDump_Tetras_Destroy", m_hObject);
	}

	// And remove ourselves
	g_pLTServer->RemoveObject(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::SetItems
//
//	PURPOSE:	Take the items a body dropped
//
// ----------------------------------------------------------------------- //
void CoreDump::SetItems(ArchiveItem* aItems)
{
	m_nNumItems = 0;

	for (int i = 0; i < CD_MAX_ITEMS; i++)
	{
		m_aItems[i].m_eType = aItems[i].m_eType;
		if (aItems[i].m_eType != kAIT_None)
		{
			m_nNumItems++;
		}

		m_aItems[i].m_sName = aItems[i].m_sName;
		m_aItems[i].m_eBuild = aItems[i].m_eBuild;
		m_aItems[i].m_eCondition = aItems[i].m_eCondition;
		m_aItems[i].m_nPSet = aItems[i].m_nPSet;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::ShowContents
//
//	PURPOSE:	Open the decision window on the clients with our items
//
// ----------------------------------------------------------------------- //
void CoreDump::ShowContents()
{
	CAutoMessage cMsg;

	bool bScanShowsNormal = false;
	bool bScanShowsGuess = false;

	if (!g_pServerSubroutineMgr)
	{
		g_ServerSubroutineMgr.Init(SSMGR_DEFAULT_FILE);
	}

	cMsg.Writeuint8(MID_DECISION);
	cMsg.Writeuint8(DECISION_TYPE_COREDUMP);
	cMsg.WriteObject(m_hObject);
	cMsg.Writefloat(300.0f);
	cMsg.Writeuint8(1);
	cMsg.Writeuint8(4);

	// A virus scan may tell what the subroutines' conditions are
	if (m_pPlayer)
	{
		PlayerSubroutineSlot* pSlot = m_pPlayer->FindSubroutine("virus_scan");
		if (pSlot && pSlot->nTronUnknown40 && pSlot->nTronUnknown41)
		{
			SubroutineDataKey keyA(0, pSlot->nTronUnknown41, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0);
			if (GetRandom(0.0f, 1.0f) < g_pSubroutineDataMgr->m_vecVirusScan.at(g_pSubroutineDataMgr->GetIndex(keyA)).fFailureChance)
			{
				bScanShowsNormal = true;
			}

			SubroutineDataKey keyB(1, pSlot->nTronUnknown41, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0);
			if (GetRandom(0.0f, 1.0f) < g_pSubroutineDataMgr->m_vecVirusScan.at(g_pSubroutineDataMgr->GetIndex(keyB)).fFailureChance)
			{
				bScanShowsGuess = true;
			}
		}
	}

	cMsg.Writebool(bScanShowsNormal);

	for (int i = 0; i < CD_MAX_ITEMS; i++)
	{
		ArchiveItem& item = m_aItems[i];

		bool bItem = (item.m_eType != kAIT_None);
		cMsg.Writebool(bItem);
		if (!bItem) continue;

		uint32 nNameId;
		switch (item.m_eType)
		{
			case kAIT_Subroutine:
				nNameId = g_pServerSubroutineMgr->GetNameId(item.m_sName.c_str(), kSST_Subroutine);
				break;

			case kAIT_Email:
			case kAIT_HelpFile:
			{
				int nIndex = g_pInventoryButeMgr->GetItemIndex(item.m_sName.c_str());
				if (nIndex == -1)
				{
					nNameId = 0;
				}
				else
				{
					nNameId = g_pInventoryButeMgr->GetItemTextID(nIndex);
				}
			}
			break;

			case kAIT_Procedural:
				nNameId = g_pServerSubroutineMgr->GetNameId(item.m_sName.c_str(), kSST_Procedural);
				break;

			default:
				nNameId = atoi(item.m_sName.c_str());
				break;
		}

		cMsg.Writeuint32(nNameId);
		cMsg.Writeint8(item.m_eType);

		if (item.m_eType == kAIT_Subroutine)
		{
			cMsg.Writeint8(item.m_eBuild);

			if (bScanShowsNormal)
			{
				if (item.m_eCondition == kAIC_Infected)
				{
					cMsg.Writeint8(kAIC_Normal);
				}
				else
				{
					cMsg.Writeint8(item.m_eCondition);
				}
			}
			else if (bScanShowsGuess)
			{
				cMsg.Writeint8(m_aeShownCondition[i]);
			}
			else
			{
				cMsg.Writeint8(item.m_eCondition);
			}
		}
		else if (item.m_eType == kAIT_PermissionSet)
		{
			cMsg.Writeint8(item.m_nPSet);
		}
	}

	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// PORT: Tron's PlaySoundFromPos takes one more argument (1.0f)
	g_pServerSoundMgr->PlaySoundFromPos(vPos, "coredump", 600.0f, SOUNDPRIORITY_MISC_HIGH, 0, 100, 1.0f, -1.0f, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::HandleChoice
//
//	PURPOSE:	The player took an item, destroyed us, or closed the window
//
// ----------------------------------------------------------------------- //
void CoreDump::HandleChoice(uint8 nChoice)
{
	char szMsg[256];

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

	switch (nChoice)
	{
		case CD_CHOICE_DESTROY:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
			Term();
		}
		break;

		case CD_CHOICE_CANCEL:
		case CD_CHOICE_CANCEL + 1:
		case CD_CHOICE_CANCEL + 2:
		{
			HandleStopActivate();
		}
		break;

		default:
		{
			if (nChoice >= CD_MAX_CHOICES || !m_pPlayer) break;

			// A virus scan may catch an infection
			bool bScanned = false;
			PlayerSubroutineSlot* pSlot = m_pPlayer->FindSubroutine("virus_scan");
			if (pSlot && pSlot->nTronUnknown40 && pSlot->nTronUnknown41)
			{
				SubroutineDataKey key(0, pSlot->nTronUnknown41, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0);
				if (GetRandom(0.0f, 1.0f) < g_pSubroutineDataMgr->m_vecVirusScan.at(g_pSubroutineDataMgr->GetIndex(key)).fChanceToCorrupt)
				{
					bScanned = true;
				}
			}

			// RETAIL: an item of another type (Custom) sends szMsg as it is,
			// uninitialized
			if (m_aItems[nChoice].m_eType == kAIT_Subroutine)
			{
				int eCondition = m_aItems[nChoice].m_eCondition;
				if (bScanned && eCondition != kAIC_Foreign)
				{
					eCondition = kAIC_Infected;
				}

				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %s %s %s", "ACQUIRESUB", m_aItems[nChoice].m_sName.c_str(),
					s_aszArchiveItemBuilds[m_aItems[nChoice].m_eBuild], s_aszArchiveItemConditions[eCondition]);
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_Procedural)
			{
				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %s", "ACQUIREPROC", m_aItems[nChoice].m_sName.c_str());
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_Primitive)
			{
				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %s", "ACQUIREPRIM", m_aItems[nChoice].m_sName.c_str());
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_PermissionSet)
			{
				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %d", "ACQUIREPSET", m_aItems[nChoice].m_nPSet);
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}
			else if (m_aItems[nChoice].m_eType == kAIT_Email || m_aItems[nChoice].m_eType == kAIT_HelpFile)
			{
				_snprintf(szMsg, ARRAY_LEN(szMsg), "%s %s", "ACQUIRENOTE", m_aItems[nChoice].m_sName.c_str());
				szMsg[ARRAY_LEN(szMsg) - 1] = '\0';
			}

			m_aItems[nChoice].Clear();
			m_nNumItems--;

			// Hide a tetra
			if (m_hTetras)
			{
				uint32 nNumPieces;
				g_pModelLT->GetNumPieces(m_hTetras, nNumPieces);
				for (uint32 iPiece = 0; iPiece < nNumPieces; iPiece++)
				{
					bool bHidden;
					g_pModelLT->GetPieceHideStatus(m_hTetras, iPiece, bHidden);
					if (!bHidden)
					{
						g_pModelLT->SetPieceHideStatus(m_hTetras, iPiece, true);
						break;
					}
				}
			}

			SendTriggerMsgToObject(this, m_pPlayer->m_hObject, LTFALSE, szMsg);
		}
		break;
	}
}
