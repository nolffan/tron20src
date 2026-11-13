// ----------------------------------------------------------------------- //
//
// MODULE  : ServerLightCycleMgr.cpp
//
// PURPOSE : The server's light cycle manager
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto 0x1004f480..0x10053050 (see ServerLightCycleMgr.h). The
// functions are in retail's order.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerLightCycleMgr.h"
#include "MsgIDs.h"
#include "PlayerObj.h"
#include "TronPlayerObj.h"
#include "ServerUtilities.h"
#include "ModelButeMgr.h"
#include "LightCycleMissile.h"
#include "LightCyclePowerup.h"

CLightCycleMgr* g_pLightCycleMgr = NULL;				// RETAIL: 0x1035a54c
CServerLightCycleMgr* g_pServerLightCycleMgr = NULL;	// RETAIL: 0x1035a550

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SendPowerupGrabbed
//
//	PURPOSE:	Tells the clients pCyclist grabbed pVictim's power-up
//
//	RETAIL: 0x1004f480 (a static function; the name is ours)
//
// ----------------------------------------------------------------------- //

static void SendPowerupGrabbed(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pVictim)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_POWERUP_GRABBED);
	cMsg.WriteObject(pCyclist->hObject);
	cMsg.WriteObject(pVictim->hObject);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SendPowerupTaken
//
//	PURPOSE:	Tells the clients pCyclist took a power-up off the floor
//
//	RETAIL: 0x1004f580 (a static function; the name is ours)
//
// ----------------------------------------------------------------------- //

static void SendPowerupTaken(LIGHT_CYCLIST* pCyclist)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_POWERUP_TAKEN);
	cMsg.WriteObject(pCyclist->hObject);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::PlayDialog
//
//	PURPOSE:	Queues a line of the cyclist's rider
//
//	RETAIL: 0x1004f660
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::PlayDialog(LIGHT_CYCLIST* pCyclist, int eType)
{
	static float s_fMaxDist = g_pLightCycleButeMgr->GetDialogMaxDist();
	static bool s_bDebug = g_pLightCycleButeMgr->GetEnableSoundDebugInfo();

	if((GetDialogTime() - m_fLastDialogTime < m_fDialogDelay) || !m_bDialogEnabled || !m_bTronUnknown1365)
	{
		if(s_bDebug)
		{
			g_pLTServer->CPrint("in-time-interval-queue : %s %d", GetLCDialogTypeName(eType), eType);
		}
		return;
	}

	LCDIALOGSOUNDS* pSounds = &g_pLightCycleButeMgr->GetCycleDialog((uint8)pCyclist->nCycleDialog)->aTypes[eType];
	bool bModifyPriority = g_pLightCycleButeMgr->GetModifyPriorityWithDist();

	LTVector vPos;
	g_pLTServer->GetObjectPos(pCyclist->hObject, &vPos);

	LTVector vListener;
	if(GetPlayerCyclist())
	{
		g_pLTServer->GetObjectPos(GetPlayerCyclist()->hObject, &vListener);
	}
	else
	{
		vListener = vPos;
	}

	float fPriority;
	if(bModifyPriority)
	{
		float fDist = (vPos - vListener).Mag();
		if(fDist >= s_fMaxDist)
			return;

		fPriority = (s_fMaxDist - fDist) * pSounds->fPriority;
	}
	else
	{
		fPriority = pSounds->fPriority;
	}

	if(s_bDebug)
	{
		g_pLTServer->CPrint(" queuesound : %s %f", GetLCDialogTypeName(eType), fPriority);
	}

	LCDialogQueueEntry entry;
	entry.pSounds = pSounds;
	entry.fPriority = fPriority;
	entry.vPos = vPos;
	entry.pCyclist = pCyclist;
	m_DialogQueue.push(entry);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::PlayDialogEntry
//
//	PURPOSE:	Tells the clients to play a queued line
//
//	RETAIL: 0x1004f8c0
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::PlayDialogEntry(const LCDialogQueueEntry& entry)
{
	// RETAIL: computed and never used
	static uint32 s_dwFlags = 0x1010;
	static float s_fRadius = g_pLightCycleButeMgr->GetDialogSoundRadius();
	static bool s_bDebug = g_pLightCycleButeMgr->GetEnableSoundDebugInfo();

	char szDebug[512];
	uint32 nSounds = entry.pSounds->aSounds.size();

	if(s_bDebug)
	{
		sprintf(szDebug, "play-sound -%x %d %s %f", entry.pCyclist, nSounds,
			GetLCDialogTypeName(entry.pSounds->eType), entry.fPriority);
	}

	if(nSounds)
	{
		uint32 nSound = (uint32)(((float)rand() / 32768.0f) * (int)nSounds);
		const char* szSound = entry.pSounds->aSounds.at(nSound).c_str();
		strcat(szDebug, szSound);
		strcat(szDebug, " ");

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cMsg.Writeuint8(LCI_DIALOG);
		cMsg.WriteLTVector(entry.vPos);
		cMsg.WriteString(szSound);
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}

	if(s_bDebug)
	{
		g_pLTServer->CPrint(szDebug);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::CServerLightCycleMgr
//
//	PURPOSE:	Constructor
//
//	RETAIL: 0x1004fb50
//
// ----------------------------------------------------------------------- //

CServerLightCycleMgr::CServerLightCycleMgr()
{
	m_fMinTimeBetweenDialog = 10.0f;
	m_fMaxTimeBetweenDialog = 10.0f;
	m_fLastDialogTime = 0.0f;
	m_fDialogDelay = 0.0f;
	m_fDialogPauseTime = 0.0f;
	m_bDialogPlayed = false;
	m_bDialogEnabled = true;
	m_bTronUnknown1365 = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::Init
//
//	PURPOSE:	Initialization
//
//	RETAIL: 0x100500c0
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::Init()
{
	if(!CLightCycleMgr::Init())
		return false;

	g_pLightCycleMgr = this;
	g_pServerLightCycleMgr = this;

	m_fMinTimeBetweenDialog = g_pLightCycleButeMgr->GetMinTimeBetweenDialog();
	m_fMaxTimeBetweenDialog = g_pLightCycleButeMgr->GetMaxTimeBetweenDialog();
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::Term
//
//	PURPOSE:	Cleanup
//
//	RETAIL: 0x10050100
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::Term()
{
	LCPositionUpdateList::iterator iter;
	for(iter = m_lstPositionUpdates.begin(); iter != m_lstPositionUpdates.end(); iter++)
	{
		debug_delete(*iter);
	}
	m_lstPositionUpdates.clear();

	for(iter = m_lstFreePositionUpdates.begin(); iter != m_lstFreePositionUpdates.end(); iter++)
	{
		debug_delete(*iter);
	}
	m_lstFreePositionUpdates.clear();

	CLightCycleMgr::Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::Update
//
//	PURPOSE:	Updates the cyclists, tells the clients where the AI's are
//				and plays the queued rider dialog
//
//	RETAIL: 0x100501d0
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::Update()
{
	if(!m_bUpdating)
		return;

	UpdateDerez();

	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter = m_collCyclists.begin(); iter != m_collCyclists.end(); iter++)
	{
		LIGHT_CYCLIST* pCyclist = (*iter);

		// Done derezzing?
		if((pCyclist->fStartDerezTime > 0.0f) && pCyclist->collTrails.empty())
		{
			RemoveCyclist(pCyclist->hObject);
		}

		if((pCyclist->nFlags & LCF_TURBO) || (pCyclist->nFlags & LCF_TURBOCURSED))
		{
			if(pCyclist->fPowerupDuration + pCyclist->fPowerupStartTime <= g_pLTServer->GetTime())
			{
				pCyclist->nFlags &= LCF_TIMED_MASK;
			}
		}

		if(pCyclist->nFlags & LCF_TRAILSPIKE)
		{
			UpdateTrailSpike(pCyclist);
		}

		if(!pCyclist->bUpdating)
			continue;

		if(!pCyclist->bIsPlayer)
		{
			// The AI: follow the object and tell the clients
			LTVector vForward = pCyclist->vForward;
			g_pLTServer->GetObjectPos(pCyclist->hObject, &pCyclist->vLastPos);

			LIGHT_CYCLE_TRAIL* pTrail = pCyclist->pCurTrail;
			if(pTrail && (pTrail->collTrailPoints.size() > 1))
			{
				LIGHT_CYCLE_TRAIL_POINT &pt = pTrail->collTrailPoints[pTrail->collTrailPoints.size() - 1];
				if(pt.vPoint.x < pCyclist->vLastPos.x)
				{
					vForward.x = 1.0f;
					vForward.z = 0.0f;
				}
				else if(pt.vPoint.x > pCyclist->vLastPos.x)
				{
					vForward.x = -1.0f;
					vForward.z = 0.0f;
				}
				else if(pt.vPoint.z < pCyclist->vLastPos.z)
				{
					vForward.x = 0.0f;
					vForward.z = 1.0f;
				}
				else if(pt.vPoint.z > pCyclist->vLastPos.z)
				{
					vForward.x = 0.0f;
					vForward.z = -1.0f;
				}
			}

			UpdateLightCycleTrail(pCyclist, true, &vForward, true);

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
			cMsg.Writeuint8(LCI_POSITION);
			cMsg.WriteObject(pCyclist->hObject);
			cMsg.Writefloat(pCyclist->vLastPos.x);
			cMsg.Writefloat(pCyclist->vLastPos.z);
			cMsg.Writeint8((int8)pCyclist->vForward.x);
			cMsg.Writeint8((int8)pCyclist->vForward.z);
			g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
		}
		else if(pCyclist->fTrailCheckTime != 0.0f)
		{
			// A player: apply what its client told us
			LTVector vOldForward = pCyclist->vForward;
			LTVector vObjPos;
			g_pLTServer->GetObjectPos(pCyclist->hObject, &vObjPos);

			bool bSend = true;
			LCPositionUpdateList::iterator iterUpdate;
			for(iterUpdate = m_lstPositionUpdates.begin(); iterUpdate != m_lstPositionUpdates.end(); iterUpdate++)
			{
				LCPositionUpdate* pUpdate = (*iterUpdate);
				if(pUpdate->hObject != pCyclist->hObject)
					continue;

				pCyclist->vLastPos.y = vObjPos.y;
				pCyclist->vLastPos.x = pUpdate->fX;
				pCyclist->vLastPos.z = pUpdate->fZ;

				LTVector vForward(pUpdate->fForwardX, 0.0f, pUpdate->fForwardZ);

				if(pCyclist->bIsPlayer)
				{
					CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(pCyclist->hObject);
					g_pLTServer->MoveObject(pPlayer->m_hObject, &pCyclist->vLastPos);

					LTVector vCurPos;
					g_pLTServer->GetObjectPos(pPlayer->m_hObject, &vCurPos);
					if(vCurPos != pCyclist->vLastPos)
					{
						g_pLTServer->SetObjectPos(pPlayer->m_hObject, &pCyclist->vLastPos);
					}
				}

				UpdateLightCycleTrail(pCyclist, true, &vForward, true);

				if(vForward != vOldForward)
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
					cMsg.Writeuint8(LCI_POSITION);
					cMsg.WriteObject(pCyclist->hObject);
					cMsg.Writefloat(pCyclist->vLastPos.x);
					cMsg.Writefloat(pCyclist->vLastPos.z);
					cMsg.Writeint8((int8)pCyclist->vForward.x);
					cMsg.Writeint8((int8)pCyclist->vForward.z);
					g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
					bSend = false;
				}
				else
				{
					bSend = true;
				}
			}

			if(bSend)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
				cMsg.Writeuint8(LCI_POSITION);
				cMsg.WriteObject(pCyclist->hObject);
				cMsg.Writefloat(pCyclist->vLastPos.x);
				cMsg.Writefloat(pCyclist->vLastPos.z);
				cMsg.Writeint8((int8)pCyclist->vForward.x);
				cMsg.Writeint8((int8)pCyclist->vForward.z);
				g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
			}

			pCyclist->fTrailCheckTime = 0.0f;
		}
	}

	// Recycle the position reports
	LCPositionUpdateList::iterator iterUpdate;
	for(iterUpdate = m_lstPositionUpdates.begin(); iterUpdate != m_lstPositionUpdates.end(); iterUpdate++)
	{
		m_lstFreePositionUpdates.push_back(*iterUpdate);
	}
	m_lstPositionUpdates.clear();

	// Time for a line?
	if(GetDialogTime() - m_fLastDialogTime < m_fDialogDelay)
		return;

	if(m_DialogQueue.empty())
		return;

	// RETAIL: keeps a pointer to the top of the queue across pop(), so it
	// plays whatever pop() moved there (docs/RETAIL_BUGS.md)
	bool bSkip = false;
	const LCDialogQueueEntry* pEntry;
	do
	{
		pEntry = &m_DialogQueue.top();
		m_DialogQueue.pop();

		if(!GetLivingCyclist(pEntry->pCyclist->hObject))
		{
			bSkip = true;
			if(pEntry->pSounds->eType == kLCDialog_AICrash)
				bSkip = false;
		}
	}
	while(!m_DialogQueue.empty() && bSkip);

	if(pEntry)
	{
		PlayDialogEntry(*pEntry);
	}

	ClearDialogQueue();

	if(GetPlayerCyclist())
	{
		float fMin = m_fMinTimeBetweenDialog;
		float fMax = m_fMaxTimeBetweenDialog;
		m_fDialogDelay = GetRandom(0.0f, 1.0f) * (fMax - fMin) + fMin;
	}
	else
	{
		m_fDialogDelay = 300.0f;
	}

	m_fLastDialogTime = g_pLTServer->GetTime();
	m_bDialogPlayed = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::AddCyclist
//
//	PURPOSE:	Creates a new cyclist and tells the clients
//
//	RETAIL: 0x10050aa0
//
// ----------------------------------------------------------------------- //

LIGHT_CYCLIST* CServerLightCycleMgr::AddCyclist(HOBJECT hObj, const char* szCycleColor, bool bUpdate,
	const LTVector* pvDims, uint32 nNameId, bool bTronUnknown)
{
	LIGHT_CYCLIST* pCyclist = CLightCycleMgr::AddCyclist(hObj, szCycleColor, bUpdate, pvDims, 0, true);
	if(!pCyclist)
		return NULL;

	if(!pvDims)
	{
		// An AI
		ClearDialogQueue();
		if(m_collCyclists.size() <= 2)
		{
			m_fDialogDelay = 0.0f;
			m_fDialogPauseTime = 0.0f;
			m_bDialogPlayed = false;
		}
		PlayDialog(pCyclist, kLCDialog_AIStart);
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_ADD_CYCLIST);
	cMsg.WriteObject(hObj);
	cMsg.WriteString(szCycleColor);
	cMsg.Writebool(bUpdate);
	cMsg.Writeint8((int8)pCyclist->vForward.x);
	cMsg.Writeint8((int8)pCyclist->vForward.z);
	cMsg.Writebool(bTronUnknown);

	if(IsPlayer(hObj))
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObj);
		if(!pPlayer)
			return NULL;

		pCyclist->nNameId = 0;
		pCyclist->nClientID = g_pLTServer->GetClientID(pPlayer->GetClient());
		cMsg.Writeuint32(pCyclist->nClientID);
		cMsg.Writeuint32(pCyclist->nNameId);
		cMsg.Writebool(true);
		cMsg.WriteLTVector(*pvDims);
	}
	else
	{
		ModelId eModelId = g_pModelButeMgr->GetModelId("GridCyclist");
		if(nNameId)
		{
			pCyclist->nNameId = nNameId;
		}
		else
		{
			pCyclist->nNameId = g_pModelButeMgr->GetModelNameId(eModelId);
		}

		pCyclist->nClientID = -1;
		cMsg.Writeuint32(pCyclist->nClientID);
		cMsg.Writeuint32(pCyclist->nNameId);
		cMsg.Writebool(false);
	}

	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	return pCyclist;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::RemoveCyclist
//
//	PURPOSE:	Marks a cyclist for removal and tells its object and the
//				clients
//
//	RETAIL: 0x10050d30
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::RemoveCyclist(HOBJECT hObj)
{
	if(CLightCycleMgr::RemoveCyclist(hObj))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint32(MID_LIGHT_CYCLE_INFO);
		cMsg.Writeuint8(LCI_REMOVE_CYCLIST);
		g_pLTServer->SendToObject(cMsg.Read(), hObj, hObj, MESSAGE_GUARANTEED);

		cMsg.Reset();
		cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cMsg.Writeuint8(LCI_REMOVE_CYCLIST);
		cMsg.WriteObject(hObj);
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::DerezCyclist
//
//	PURPOSE:	Starts a cyclist's derez
//
//	RETAIL: 0x10050e90
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::DerezCyclist(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pKiller, bool bTronUnknown)
{
	return DerezCyclist(pCyclist, pKiller, false, true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::DerezCyclist
//
//	PURPOSE:	Starts a cyclist's derez
//
//	RETAIL: 0x10050eb0
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::DerezCyclist(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pKiller, bool bTronUnknown, bool bNotifyMission)
{
	// Already derezzing?
	if(pCyclist->fStartDerezTime > 0.0f)
		return false;

	if(!pCyclist->bIsPlayer)
	{
		PlayDialog(pCyclist, kLCDialog_AICrash);
		SendTriggerMsgToObjects(NULL, "OnAICrash", "TRIGGER");
	}
	else
	{
		LIGHT_CYCLIST* pAI = FindNearestCyclist(pCyclist, true, false);
		if(pAI)
		{
			ClearDialogQueue();
			m_fDialogDelay = 0.0f;
			m_fDialogPauseTime = 0.0f;
			m_bDialogPlayed = false;
			PlayDialog(pAI, kLCDialog_PlayerCrash);
		}

		if(!IsMultiplayerGame())
		{
			SendTriggerMsgToObjects(NULL, "OnPlayerCrash", "TRIGGER");
		}
		else if(bNotifyMission)
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(pCyclist->hObject);
			CPlayerObj* pKillerPlayer = NULL;
			if(pKiller)
			{
				pKillerPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(pKiller->hObject);
			}

			// PORT: retail tells the light cycle mission's manager
			// (0x1035a574) here: 0x10054b50(pPlayer, pKillerPlayer, false).
			// That class is not written yet.
		}
	}

	pCyclist->fStartDerezTime = g_pLTServer->GetTime();
	pCyclist->bUpdating = false;

	std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter;
	for(iter = pCyclist->collTrails.begin(); iter != pCyclist->collTrails.end(); iter++)
	{
		if((*iter)->fDerezStartTime == 0.0f)
		{
			(*iter)->fDerezStartTime = g_pLTServer->GetTime();
		}
	}

	pCyclist->pCurTrail = NULL;
	pCyclist->fTrailCheckTime = 0.0f;
	pCyclist->m_vTronUnknownE4.Init();

	HOBJECT hObj = pCyclist->hObject;
	HOBJECT hKiller = NULL;
	if(pKiller)
	{
		hKiller = pKiller->hObject;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_DEREZ_CYCLIST);
	g_pLTServer->SendToObject(cMsg.Read(), hObj, hObj, MESSAGE_GUARANTEED);

	cMsg.Reset();
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_DEREZ_CYCLIST);
	cMsg.WriteObject(hObj);
	cMsg.WriteObject(hKiller);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::OnMessage
//
//	PURPOSE:	Handles a client's light cycle message
//
//	RETAIL: 0x10051150
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::OnMessage(ILTMessage_Read* pMsg, HCLIENT hSender)
{
	uint8 nType = pMsg->Readuint8();
	HOBJECT hObj = pMsg->ReadObject();

	switch(nType)
	{
		case LCI_POSITION:
		{
			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if(pCyclist)
			{
				LCPositionUpdate* pUpdate = NewPositionUpdate();
				pUpdate->hObject = hObj;
				pUpdate->fX = pMsg->Readfloat();
				pUpdate->fZ = pMsg->Readfloat();
				pUpdate->fForwardX = (float)pMsg->Readint8();
				pUpdate->fForwardZ = (float)pMsg->Readint8();
				m_lstPositionUpdates.push_back(pUpdate);
				pCyclist->fTrailCheckTime = g_pLTServer->GetTime();
			}
			return true;
		}

		case LCI_TURBO:
		{
			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if(pCyclist)
			{
				float fTronUnknown1 = pMsg->Readfloat();
				float fTronUnknown2 = pMsg->Readfloat();
				bool bTronUnknown = pMsg->Readbool();
				float fTronUnknown3 = pMsg->Readfloat();
				float fTronUnknown4 = pMsg->Readfloat();
				uint32 nTronUnknown = pMsg->Readuint32();
				SendCyclistMovement(pCyclist, bTronUnknown, fTronUnknown1, fTronUnknown2, fTronUnknown3, fTronUnknown4, nTronUnknown, hSender);
			}
			return true;
		}

		case LCI_DEREZ_CYCLIST:
		{
			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			LIGHT_CYCLIST* pKiller = FindCyclist(pMsg->ReadObject());
			if(pCyclist)
			{
				DerezCyclist(pCyclist, pKiller, (bool)(hSender != NULL), true);
			}
			return true;
		}

		case LCI_TRONUNKNOWN_6:
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
			if(pPlayer)
			{
				char szTronUnknown[32];
				pMsg->ReadString(szTronUnknown, sizeof(szTronUnknown));

				// PORT: retail passes the string to CTronPlayerObj's
				// 0x10067b90, not written yet.
			}
			return true;
		}

		case LCI_START_LIGHT_CYCLE:
		{
			CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->HandleToObject(hObj);
			if(pPlayer)
			{
				pPlayer->StartLightCycle(false, false);
			}
			return true;
		}

		case LCI_END_LIGHT_CYCLE:
		{
			CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->HandleToObject(hObj);
			if(pPlayer)
			{
				pPlayer->EndLightCycle();
			}
			return true;
		}

		case LCI_TRIM_TRAIL:
		{
			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if(pCyclist)
			{
				uint32 nTronUnknown1 = pMsg->Readuint32();
				uint32 nTronUnknown2 = pMsg->Readuint32();
				TrimTrailExcept(pCyclist, nTronUnknown1, nTronUnknown2, hSender);
			}
			return true;
		}

		case LCI_ACTIVATE_POWERUP:
		{
			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if(pCyclist)
			{
				LTVector vPos;
				pMsg->ReadType(&vPos);
				LTVector vCopy = vPos;
				ActivatePowerup(pCyclist, &vCopy);
			}
			return true;
		}

		case LCI_START_UPDATING:
		{
			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if(!pCyclist)
				return true;

			if(pCyclist->fStartDerezTime != 0.0f)
				return true;

			pCyclist->bUpdating = true;

			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_LIGHT_CYCLE_INFO);
			cMsg.Writeuint8(LCI_START_UPDATING);
			g_pLTServer->SendToObject(cMsg.Read(), hObj, hObj, MESSAGE_GUARANTEED);

			cMsg.Reset();
			cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
			cMsg.Writeuint8(LCI_UPDATING_STARTED);
			cMsg.WriteObject(hObj);
			cMsg.Writeuint8(1);
			g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
			return true;
		}

		default:
			break;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::CyclistMovementChanged
//
//	PURPOSE:	A cycle used its turbo: some dialog, and the clients
//
//	RETAIL: 0x10051620
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::CyclistMovementChanged(LIGHT_CYCLIST* pCyclist, bool bTurned, float fAccel, float fWallAccel, uint32 nFlags)
{
	if(!pCyclist->bIsPlayer)
	{
		PlayDialog(pCyclist, kLCDialog_AISpeedBoost);
	}
	else
	{
		LIGHT_CYCLIST* pListener = m_collCyclists.begin()[GetRandom(0, m_collCyclists.size())];
		PlayDialog(pListener, kLCDialog_PlayerSpeedBoost);
	}

	SendCyclistMovement(pCyclist, bTurned, pCyclist->fAccelRate, pCyclist->fSpeed,
		fAccel, fWallAccel, nFlags, NULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::SendCyclistMovement
//
//	PURPOSE:	Tells the clients (but hExclude) about a turbo
//
//	RETAIL: 0x10051690
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::SendCyclistMovement(LIGHT_CYCLIST* pCyclist, bool bTronUnknown, float fTronUnknown1, float fTronUnknown2,
	float fTronUnknown3, float fTronUnknown4, uint32 nTronUnknown, HCLIENT hExclude)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_TURBO);
	cMsg.WriteObject(pCyclist->hObject);
	cMsg.Writefloat(fTronUnknown1);
	cMsg.Writefloat(fTronUnknown2);
	cMsg.Writebool(bTronUnknown);
	cMsg.Writefloat(fTronUnknown3);
	cMsg.Writefloat(fTronUnknown4);
	cMsg.Writeuint32(nTronUnknown);

	CLTMsgRef_Read cRead = cMsg.Read();
	HCLIENT hNext = g_pLTServer->GetNextClient(NULL);
	while(hNext)
	{
		HCLIENT hClient = hNext;
		hNext = g_pLTServer->GetNextClient(hNext);
		if(hClient != hExclude)
		{
			g_pLTServer->SendToClient(cRead, hClient, MESSAGE_GUARANTEED);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::GetPlayerCyclist
//
//	PURPOSE:	The cyclist of the first player (the listener of the
//				rider dialog)
//
//	RETAIL: 0x10051810
//
// ----------------------------------------------------------------------- //

LIGHT_CYCLIST* CServerLightCycleMgr::GetPlayerCyclist()
{
	if(CPlayerObj::GetPlayerObjList().size() > 0)
	{
		CPlayerObj* pPlayer = CPlayerObj::GetPlayerObjList()[0];
		return FindCyclist(pPlayer->m_hObject);
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::SetCyclistPowerup
//
//	PURPOSE:	Gives a cyclist a power-up
//
//	RETAIL: 0x10051840
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::SetCyclistPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup)
{
	if(nPowerup == kLCPowerup_Random)
	{
		nPowerup = TronUnknownGetRandomPowerup();
	}

	if(CLightCycleMgr::SetCyclistPowerup(pCyclist, nPowerup))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cMsg.Writeuint8(LCI_SET_POWERUP);
		cMsg.WriteObject(pCyclist->hObject);
		cMsg.Writeuint8((uint8)pCyclist->nPowerup);
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::ActivatePowerup
//
//	PURPOSE:	A cyclist uses its power-up
//
//	RETAIL: 0x10051980
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::ActivatePowerup(LIGHT_CYCLIST* pCyclist, LTVector* pvPos)
{
	int nPowerup = pCyclist->nPowerup;

	// Nothing to grab?
	if(nPowerup == kLCPowerup_Grab)
	{
		if(!FindNearestCyclist(pCyclist, false, true) && !FindNearestPowerup(pCyclist->hObject, false))
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
			cMsg.Writeuint8(LCI_GRAB_FAILED);
			cMsg.WriteObject(pCyclist->hObject);
			g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
			return false;
		}
	}

	if(!CLightCycleMgr::ActivatePowerup(pCyclist, pvPos))
		return false;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_ACTIVATE_POWERUP);
	cMsg.WriteObject(pCyclist->hObject);
	cMsg.WriteLTVector(*pvPos);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	switch(nPowerup)
	{
		case kLCPowerup_Missile:
		{
			ObjectCreateStruct theStruct;
			INIT_OBJECTCREATESTRUCT(theStruct);

			g_pLTServer->GetObjectPos(pCyclist->hObject, &theStruct.m_Pos);
			g_pLTServer->GetObjectRotation(pCyclist->hObject, &theStruct.m_Rotation);

			HCLASS hClass = g_pLTServer->GetClass("LightCycleMissile");
			LightCycleMissile* pMissile = (LightCycleMissile*)g_pLTServer->CreateObject(hClass, &theStruct);
			if(pMissile)
			{
				HOBJECT hOwner = pCyclist->hObject;
				pMissile->m_hOwner = hOwner;
				pMissile->m_vStartPos = theStruct.m_Pos;
				pMissile->m_vDir = pCyclist->vForward;
				pMissile->m_pOwnerCyclist = FindCyclist(hOwner);
			}
			else
			{
				g_pLTServer->CPrint("ERROR - Could not create the light cycle object!\n");
			}
		}
		break;

		case kLCPowerup_Grab:
		{
			int nGrabbed;
			LIGHT_CYCLIST* pVictim = FindNearestCyclist(pCyclist, false, true);
			if(pVictim)
			{
				nGrabbed = pVictim->nPowerup;
				SetCyclistPowerup(pVictim, LC_NO_POWERUP);
				SendPowerupGrabbed(pCyclist, pVictim);
			}
			else
			{
				LightCyclePowerup* pPowerup = FindNearestPowerup(pCyclist->hObject, false);
				if(!pPowerup)
					break;

				nGrabbed = pPowerup->GetType();
				pPowerup->SetEnabled(false);
				SendPowerupTaken(pCyclist);
			}

			if(nGrabbed != LC_NO_POWERUP)
			{
				SetCyclistPowerup(pCyclist, nGrabbed);
			}
		}
		break;

		case kLCPowerup_TrailReset:
		{
			DerezOldTrails();
		}
		break;

		case kLCPowerup_ForceUse:
		{
			// Everyone else uses theirs
			std::vector<LIGHT_CYCLIST*>::iterator iter;
			for(iter = m_collCyclists.begin(); iter != m_collCyclists.end(); iter++)
			{
				LIGHT_CYCLIST* pOther = (*iter);
				if(pOther->nPowerup != LC_NO_POWERUP)
				{
					ActivatePowerup(pOther, &pOther->vLastPos);
				}
			}
		}
		break;

		default:
			break;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::TrimTrailExcept
//
//	RETAIL: 0x10051e50
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::TrimTrailExcept(LIGHT_CYCLIST* pCyclist, int nTrail, int nPoint, HCLIENT hExclude)
{
	if(CLightCycleMgr::TrimTrail(pCyclist, nTrail, nPoint, true))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cMsg.Writeuint8(LCI_TRIM_TRAIL);
		cMsg.WriteObject(pCyclist->hObject);
		cMsg.Writeuint32(nTrail);
		cMsg.Writeuint32(nPoint);

		CLTMsgRef_Read cRead = cMsg.Read();
		HCLIENT hNext = g_pLTServer->GetNextClient(NULL);
		while(hNext)
		{
			HCLIENT hClient = hNext;
			hNext = g_pLTServer->GetNextClient(hNext);
			if(hClient != hExclude)
			{
				g_pLTServer->SendToClient(cRead, hClient, MESSAGE_GUARANTEED);
			}
		}
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::TrimTrail
//
//	RETAIL: 0x10051fc0
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::TrimTrail(LIGHT_CYCLIST* pCyclist, int nTrail, int nPoint, bool bNotify)
{
	return TrimTrailExcept(pCyclist, nTrail, nPoint, NULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::DeactivateShield
//
//	RETAIL: 0x10051fe0
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::DeactivateShield(LIGHT_CYCLIST* pCyclist, bool bNotify)
{
	CLightCycleMgr::DeactivateShield(pCyclist, bNotify);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_TRONUNKNOWN_13);
	cMsg.WriteObject(pCyclist->hObject);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::HandleShieldCollision
//
//	PURPOSE:	A cycle ran into a trail: tell the clients
//
//	RETAIL: 0x100520d0
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::HandleShieldCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo* pInfo, bool bNotify, bool bCurrentTrail)
{
	if(pInfo->nTrail == -1)
		return true;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_COLLISION);
	cMsg.WriteObject(pCyclist->hObject);
	cMsg.Writeuint32(pInfo->nSegment);
	cMsg.Writeuint32(pInfo->nTrail);
	cMsg.WriteObject(pInfo->pCyclist->hObject);
	cMsg.Writefloat(pInfo->vCollisionPoint.x);
	cMsg.Writefloat(pInfo->vCollisionPoint.z);
	cMsg.Writebool(bNotify);
	cMsg.Writebool(bCurrentTrail);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	return CLightCycleMgr::HandleShieldCollision(pCyclist, pInfo, bNotify, bCurrentTrail);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::FindNearestCyclist
//
//	PURPOSE:	The live cyclist nearest pCyclist
//
//	RETAIL: 0x10052250
//
// ----------------------------------------------------------------------- //

LIGHT_CYCLIST* CServerLightCycleMgr::FindNearestCyclist(LIGHT_CYCLIST* pCyclist, bool bAIOnly, bool bWithPowerup)
{
	LIGHT_CYCLIST* pNearest = NULL;
	float fNearestDistSqr = (float)HUGE_VAL;

	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter = m_collCyclists.begin(); iter != m_collCyclists.end(); iter++)
	{
		if((*iter) == pCyclist)
			continue;

		if(bAIOnly && (*iter)->bIsPlayer)
			continue;

		if(bWithPowerup && ((*iter)->nPowerup == LC_NO_POWERUP))
			continue;

		LTVector vPos;
		g_pLTServer->GetObjectPos((*iter)->hObject, &vPos);

		float fDistSqr = vPos.DistSqr(pCyclist->vLastPos);
		if((fDistSqr < fNearestDistSqr) && ((*iter)->fStartDerezTime <= 0.0f))
		{
			fNearestDistSqr = fDistSqr;
			pNearest = (*iter);
		}
	}

	return pNearest;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::ClearDialogQueue
//
//	RETAIL: 0x10052330
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::ClearDialogQueue()
{
	while(!m_DialogQueue.empty())
	{
		m_DialogQueue.pop();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::EnableDialog
//
//	RETAIL: 0x10052360
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::EnableDialog(bool bEnable)
{
	if(g_pServerLightCycleMgr)
	{
		g_pServerLightCycleMgr->m_bDialogEnabled = bEnable;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::RemovePowerup
//
//	RETAIL: 0x10052380
//
// ----------------------------------------------------------------------- //

bool CServerLightCycleMgr::RemovePowerup(HOBJECT hPowerup)
{
	LCPowerupList::iterator iter;
	for(iter = m_collPowerups.begin(); iter != m_collPowerups.end(); iter++)
	{
		if(((LPBASECLASS)(*iter))->m_hObject == hPowerup)
		{
			m_collPowerups.erase(iter);
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::FindNearestPowerup
//
//	PURPOSE:	The power-up nearest an object
//
//	RETAIL: 0x100523f0
//
// ----------------------------------------------------------------------- //

LightCyclePowerup* CServerLightCycleMgr::FindNearestPowerup(HOBJECT hObj, bool bAny)
{
	float fNearestDistSqr = (float)HUGE_VAL;
	LightCyclePowerup* pNearest = NULL;

	LTVector vPos;
	g_pLTServer->GetObjectPos(hObj, &vPos);

	LCPowerupList::iterator iter;
	for(iter = m_collPowerups.begin(); iter != m_collPowerups.end(); iter++)
	{
		LTVector vPowerupPos;
		g_pLTServer->GetObjectPos(((LPBASECLASS)(*iter))->m_hObject, &vPowerupPos);

		float fDistSqr = (vPowerupPos - vPos).MagSqr();
		if(fDistSqr < fNearestDistSqr)
		{
			if(bAny || (*iter)->IsEnabled())
			{
				fNearestDistSqr = fDistSqr;
				pNearest = (*iter);
			}
		}
	}

	return pNearest;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::DerezOldTrails
//
//	PURPOSE:	Makes every live cycle's trail end at its last point, then
//				the base's work, and tells the clients
//
//	RETAIL: 0x100524c0
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::DerezOldTrails()
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter = m_collCyclists.begin(); iter != m_collCyclists.end(); iter++)
	{
		LIGHT_CYCLIST* pCyclist = (*iter);
		if(pCyclist->fStartDerezTime != 0.0f)
			continue;

		LIGHT_CYCLE_TRAIL* pTrail = pCyclist->pCurTrail;
		if(!pTrail || (pTrail->collTrailPoints.size() <= 1))
			continue;

		LightCycleCollisionInfo info;
		info.pCyclist = pCyclist;
		info.nTrail = 0;
		info.vCollisionPoint = pTrail->collTrailPoints[pTrail->collTrailPoints.size() - 1].vPoint;
		info.nSegment = pTrail->collTrailPoints.size() - 2;
		HandleShieldCollision(pCyclist, &info, true, true);
	}

	CLightCycleMgr::DerezOldTrails();

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_TRONUNKNOWN_26);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::DerezAllAICyclists
//
//	RETAIL: 0x10052660
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::DerezAllAICyclists()
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter = m_collCyclists.begin(); iter != m_collCyclists.end(); iter++)
	{
		if(!(*iter)->bIsPlayer)
		{
			DerezCyclist(*iter, NULL, true);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::SetGameSpeed
//
//	RETAIL: 0x100526a0
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::SetGameSpeed(int nIndex, HCLIENT hClient)
{
	m_fGameSpeedScalar = g_pLightCycleButeMgr->GetGameSpeedScalar(nIndex);
	SendGameSpeed(hClient);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::SetTrailLength
//
//	RETAIL: 0x100526d0
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::SetTrailLength(int nIndex, HCLIENT hClient)
{
	m_fTrailLength = g_pLightCycleButeMgr->GetTrailLengthValue(nIndex);
	SendTrailLength(hClient);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::SendGameSpeed
//
//	RETAIL: 0x10052700
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::SendGameSpeed(HCLIENT hClient)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_GAME_SPEED);
	cMsg.Writefloat(m_fGameSpeedScalar);
	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::SendTrailLength
//
//	RETAIL: 0x10052800
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::SendTrailLength(HCLIENT hClient)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_TRAIL_LENGTH);
	cMsg.Writefloat(m_fTrailLength);
	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::SendTronUnknown21
//
//	RETAIL: 0x10052900
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::SendTronUnknown21(HCLIENT hClient, int8 nTronUnknown)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_TRONUNKNOWN_33);
	cMsg.Writeint8(nTronUnknown);
	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::SendCyclistsToClient
//
//	PURPOSE:	Tells a (new) client about all the cyclists
//
//	RETAIL: 0x100529f0
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::SendCyclistsToClient(HCLIENT hClient)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_CYCLISTS);
	cMsg.Writeuint8((uint8)m_collCyclists.size());

	for(uint8 i = 0; i < m_collCyclists.size(); i++)
	{
		WriteCyclist(m_collCyclists[i], cMsg);
	}

	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::WriteCyclist
//
//	RETAIL: 0x10052b40
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::WriteCyclist(LIGHT_CYCLIST* pCyclist, ILTMessage_Write* pMsg)
{
	pMsg->WriteObject(pCyclist->hObject);
	pMsg->Writefloat(pCyclist->fStartDerezTime);
	pMsg->Writebool(pCyclist->bUpdating);
	pMsg->Writebool(pCyclist->bRemove);
	pMsg->WriteLTVector(pCyclist->vForward);
	pMsg->Writefloat(pCyclist->fMaxTrailLength);
	pMsg->Writeuint8((uint8)pCyclist->nCycleType);
	pMsg->WriteString(pCyclist->szCycleColor);
	pMsg->Writeuint8(pCyclist->nRed);
	pMsg->Writeuint8(pCyclist->nGreen);
	pMsg->Writeuint8(pCyclist->nBlue);
	pMsg->Writefloat(pCyclist->fAccelRate);
	pMsg->Writefloat(pCyclist->fBaseSpeed);
	pMsg->Writefloat(pCyclist->fSpeed);
	pMsg->Writefloat(pCyclist->fMaxSpeed);
	pMsg->Writefloat(pCyclist->fTurboAccelRate);
	pMsg->Writefloat(pCyclist->fBrakeDecelRate);
	pMsg->Writefloat(pCyclist->fMaxManualSpeedAdd);
	pMsg->Writefloat(pCyclist->fMaxExternalSpeedAdd);
	pMsg->Writefloat(pCyclist->fMaxTurboSpeed);
	pMsg->Writefloat(pCyclist->fMinSpeed);
	pMsg->Writefloat(pCyclist->fNormalAccelRate);
	pMsg->Writefloat(pCyclist->fNormalDecelRate);
	pMsg->Writefloat(pCyclist->fMaxWallAccelRate);
	pMsg->Writefloat(pCyclist->fTurnSpeedLossPercent);
	pMsg->Writeuint32(pCyclist->nMaxWallDist);
	pMsg->Writefloat(pCyclist->fHeight);
	pMsg->Writefloat(pCyclist->fWidth);
	pMsg->Writefloat(pCyclist->fLength);
	pMsg->Writebool(pCyclist->bIsPlayer);
	pMsg->Writeuint32(pCyclist->nCycleDialog);
	pMsg->Writeuint8((uint8)pCyclist->nPowerup);
	pMsg->Writeuint32(pCyclist->nFlags);
	pMsg->Writefloat(pCyclist->fPowerupStartTime);
	pMsg->Writefloat(pCyclist->fPowerupDuration);
	pMsg->Writeuint32(pCyclist->nClientID);
	pMsg->Writebool(pCyclist->m_bTronUnknownAC);

	if(pCyclist->pSpikeTrail && pCyclist->pSpikeTrail2)
	{
		pMsg->Writebool(true);
		pMsg->WriteType(pCyclist->vSpikeDir);
		pMsg->WriteType(pCyclist->vSpikeDir2);
		pMsg->WriteType(pCyclist->vSpikePos);
		pMsg->Writefloat(pCyclist->fSpikeFullTime);
	}
	else
	{
		pMsg->Writebool(false);
	}

	pMsg->Writeuint8((uint8)pCyclist->collTrails.size());
	for(uint8 i = 0; i < pCyclist->collTrails.size(); i++)
	{
		LIGHT_CYCLE_TRAIL* pTrail = pCyclist->collTrails[i];
		if(pTrail == pCyclist->pCurTrail)
		{
			pMsg->Writeuint8(0);
		}
		else if(pTrail == pCyclist->pSpikeTrail)
		{
			pMsg->Writeuint8(1);
		}
		else if(pTrail == pCyclist->pSpikeTrail2)
		{
			pMsg->Writeuint8(2);
		}
		else
		{
			pMsg->Writeuint8(3);
		}

		WriteTrail(pCyclist->collTrails[i], pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::WriteTrail
//
//	RETAIL: 0x10052f20
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::WriteTrail(LIGHT_CYCLE_TRAIL* pTrail, ILTMessage_Write* pMsg)
{
	pMsg->Writebool(pTrail->m_bTronUnknownC);
	pMsg->Writefloat(pTrail->fDerezStartTime);
	pMsg->Writeuint32(pTrail->collTrailPoints.size());

	for(uint32 i = 0; i < pTrail->collTrailPoints.size(); i++)
	{
		pMsg->WriteType(pTrail->collTrailPoints[i]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::NewPositionUpdate
//
//	PURPOSE:	A position report from the free list, or a new one
//
//	RETAIL: 0x10052fe0
//
// ----------------------------------------------------------------------- //

LCPositionUpdate* CServerLightCycleMgr::NewPositionUpdate()
{
	if(m_lstFreePositionUpdates.size() > 0)
	{
		LCPositionUpdate* pUpdate = m_lstFreePositionUpdates.front();
		m_lstFreePositionUpdates.erase(m_lstFreePositionUpdates.begin());
		return pUpdate;
	}

	return debug_new(LCPositionUpdate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerLightCycleMgr::HeadOnCollision
//
//	PURPOSE:	Two cycles ran into each other: both derez
//
//	RETAIL: 0x10053050
//
// ----------------------------------------------------------------------- //

void CServerLightCycleMgr::HeadOnCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther)
{
	DerezCyclist(pCyclist, pOther, false, false);
	DerezCyclist(pOther, pCyclist, false, false);

	if(IsMultiplayerGame())
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(pCyclist->hObject);
		CPlayerObj* pOtherPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(pOther->hObject);

		// PORT: retail tells the light cycle mission's manager
		// (0x1035a574) here: 0x10054b50(pPlayer, pOtherPlayer, true). That
		// class is not written yet.
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronUnknownGetRandomPowerup
//
//	PURPOSE:	PORT: retail's 0x1013c350 (it returns 0) is in a TU not
//				identified yet
//
// ----------------------------------------------------------------------- //

int TronUnknownGetRandomPowerup()
{
	return 0;
}
