// ----------------------------------------------------------------------- //
//
// MODULE  : Skills.cpp
//
// PURPOSE : Player skills implementation
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MsgIds.h"
#include "Skills.h"
#include "MissionButeMgr.h"
#include "GameServerShell.h"
#include "ParsedMsg.h"
#include "ServerMissionMgr.h"
#include "PlayerObj.h"

// Constructor
CSkills::CSkills()
{
	Init( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkills::Init
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void CSkills::Init()
{
	m_nTotalPoints = 0;
	m_nAvailPoints = 0;
	memset(m_nLevel,0,sizeof(m_nLevel));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkills::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CSkills::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;


	SAVE_DWORD(m_nTotalPoints);
	SAVE_DWORD(m_nAvailPoints);

	for (uint8 i = 0; i < kNumSkills; i++)
	{
		SAVE_BYTE(m_nLevel[i]);
	}

	SAVE_HOBJECT( m_hOwner );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkills::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CSkills::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;


	LOAD_DWORD(m_nTotalPoints);
	LOAD_DWORD(m_nAvailPoints);

	uint8 nTmp;
	for (uint8 i = 0; i < kNumSkills; i++)
	{
		LOAD_BYTE(nTmp);
		m_nLevel[i] = (eSkillLevel)nTmp;
	}

	LOAD_HOBJECT( m_hOwner );

	UpdateClient();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkills::MultiplayerInit
//
//	PURPOSE:	Init multiplayer values
//
// ----------------------------------------------------------------------- //

void CSkills::MultiplayerInit(ILTMessage_Read *pMsg)
{
	uint32 nLevels[kNumSkills];

	for (uint8 nSkill = 0; nSkill < kNumSkills; nSkill++)
	{
		nLevels[nSkill] = pMsg->Readuint8();
	}

	m_nTotalPoints = 0;
	if (g_pServerMissionMgr->UseSkills( ))
	{
		m_nTotalPoints = g_pSkillsButeMgr->GetMultiplayerPool();
	}
	
	m_nAvailPoints = m_nTotalPoints;


	for (nSkill = 0; nSkill < kNumSkills; nSkill++)
	{
		m_nLevel[nSkill] = SKL_NOVICE;
		uint8 nTgtLevel = 1;
		while (nTgtLevel <= nLevels[nSkill] && m_nAvailPoints >= g_pSkillsButeMgr->GetCostToUpgrade((eSkill)nSkill,(eSkillLevel)nTgtLevel))
		{
			m_nAvailPoints -= g_pSkillsButeMgr->GetCostToUpgrade((eSkill)nSkill,(eSkillLevel)nTgtLevel);
			m_nLevel[nSkill] = (eSkillLevel)nTgtLevel;
			nTgtLevel++;
		}
	}



	UpdateClient();

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Skills::HandleRewardMessage()
//
//	PURPOSE:	Handle reward message
//
// --------------------------------------------------------------------------- //

bool CSkills::HandleRewardMessage(const CParsedMsg &cMsg)
{
	int nCurrentMission = g_pServerMissionMgr->GetCurrentMission( );

	if (nCurrentMission < 0 || nCurrentMission >= g_pMissionButeMgr->GetNumMissions()) return false;

	MISSION *pMission = g_pMissionButeMgr->GetMission(nCurrentMission);

	char sRewardMsg[256];
	cMsg.ReCreateMsg(sRewardMsg, sizeof(sRewardMsg), 1);

	int n = 0;
	while (n < pMission->nNumRewards && (stricmp(sRewardMsg,pMission->aRewards[n].szName) != 0))
		n++;

	if (n >= pMission->nNumRewards)
	{
		g_pLTServer->CPrint("Unknown reward: %s", sRewardMsg);
		return false;
	}

//	g_pLTServer->CPrint("Reward %s: %d", pMsg, pMission->aRewards[n].nVal);

	m_nTotalPoints += pMission->aRewards[n].nVal;
	m_nAvailPoints += pMission->aRewards[n].nVal;

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_SKILLS_ID);
	cClientMsg.Writeuint8(n);
    cClientMsg.Writeuint8(0);
    cClientMsg.Writefloat(0.0f); // (LTFLOAT)pMission->aRewards[n].nVal);
	g_pLTServer->SendToClient(cClientMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

    return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Skills::UpdateClient()
//
//	PURPOSE:	Give the client our current values
//
// --------------------------------------------------------------------------- //

void CSkills::UpdateClient()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_SKILLS);
	cMsg.Writeuint32(m_nTotalPoints);
	cMsg.Writeuint32(m_nAvailPoints);

	for (uint8 i = 0; i < kNumSkills; i++)
	{
		cMsg.Writeuint8(m_nLevel[i]);
	}

	CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_hOwner ));
	if( !pPlayerObj )
		return;

	HCLIENT hClient = pPlayerObj->GetClient( );
	if( !hClient )
		return;

	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkills::HandleSkillUpdate
//
//	PURPOSE:	Handle upgrading skills
//
// ----------------------------------------------------------------------- //

void CSkills::HandleSkillUpdate(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	uint8 nSkill = pMsg->Readuint8();
	if (nSkill >= kNumSkills) return;

	uint8 nTgtLevel = m_nLevel[nSkill] + 1;
	if (nTgtLevel >= kNumSkillLevels) return;
	
	uint32 nCost = g_pSkillsButeMgr->GetCostToUpgrade((eSkill)nSkill,(eSkillLevel)nTgtLevel);
	if (nCost > m_nAvailPoints) return;

	m_nAvailPoints -= nCost;
	m_nLevel[nSkill] = (eSkillLevel)nTgtLevel;

	UpdateClient();


}


// RETAIL: 0x101852b0 (CPlayerObj::SkillsCheat, 0x1007d8d0, jumps to it):
// Tron's cheat gives the current mission's rewards, one message each, not
// NOLF2's rewards of every mission before it.
void CSkills::SkillsCheat()
{
	int nCurrentMission = g_pServerMissionMgr->GetCurrentMission( );

	if (nCurrentMission < 0 || nCurrentMission >= g_pMissionButeMgr->GetNumMissions()) return;

	MISSION *pMission = g_pMissionButeMgr->GetMission(nCurrentMission);

	for (int n = 0; n < pMission->nNumRewards; ++n)
	{
		m_nTotalPoints += pMission->aRewards[n].nVal;
		m_nAvailPoints += pMission->aRewards[n].nVal;

		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cClientMsg.Writeuint8(IC_SKILLS_ID);
		cClientMsg.Writeuint8(n);
		cClientMsg.Writeuint8(0);
		cClientMsg.Writefloat(0.0f);
		g_pLTServer->SendToClient(cClientMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}


