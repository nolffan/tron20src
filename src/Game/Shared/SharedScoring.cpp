// ----------------------------------------------------------------------- //
//
// MODULE  : SharedScoring.cpp
//
// PURPOSE : SharedScoring - shared mission summary stuff
//
// CREATED : 10/17/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SharedScoring.h"
#include "MsgIDs.h"
#ifndef _CLIENTBUILD
#include "ServerMissionMgr.h"
#include "PlayerObj.h"
#include "ServerUtilities.h"
#endif

#ifndef _CLIENTBUILD
// RETAIL: Tron's server message to a client (as TronGameServerShell.cpp's)
// and its sub-id for the teams (retail 0x1013fb00); the names are ours
#define MID_TRON_SERVERMESSAGE	200
#define TSM_TEAMS				80
#endif

CTronTeamMgr* g_pTronTeamMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::CPlayerScore
//
//	PURPOSE:	Constructor (RETAIL: retail 0x1013eed0)
//
// ----------------------------------------------------------------------- //

CPlayerScore::CPlayerScore()
{
	m_nTronUnknown0 = 0;
	m_nTronUnknown2 = 0;
	m_nTronUnknown4 = 0;
	m_nTronUnknown6 = 0;
	m_nTronUnknown8 = 0;
	m_nTronUnknownA = 0;
	m_nTronUnknownC = 0;
	m_bTronUnknownE = false;
	m_nClientID = -1;
	m_nScore = 0;
	m_nFrags = 0;
	m_nTronUnknown18 = 2;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::Init
//
//	PURPOSE:	Clear data (RETAIL: retail 0x1013ef10)
//
// ----------------------------------------------------------------------- //

void CPlayerScore::Init(uint32 nClientID)
{
	m_nClientID = nClientID;
	m_nScore = 0;
	m_nFrags = 0;
	m_nTronUnknown18 = 2;

#ifndef _CLIENTBUILD
	UpdateClients();
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::WriteData
//
//	PURPOSE:	Write the data to be sent to the client (RETAIL: retail
//				0x1013ef40)
//
// ----------------------------------------------------------------------- //

void CPlayerScore::WriteData(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	pMsg->Writeint16(m_nScore);
	pMsg->Writeint16(m_nFrags);
	pMsg->Writeuint8(m_nTronUnknown18);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::ReadData
//
//	PURPOSE:	Read the data sent to the client (RETAIL: retail 0x1013ef80)
//
// ----------------------------------------------------------------------- //

void CPlayerScore::ReadData(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	m_nScore			= pMsg->Readint16();
	m_nFrags			= pMsg->Readint16();
	m_nTronUnknown18	= pMsg->Readuint8();
}

#ifndef _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::Reset
//
//	PURPOSE:	Clear the game type's figures and, if asked, the score
//				(RETAIL: retail 0x1013efc0; the name is ours)
//
// ----------------------------------------------------------------------- //

void CPlayerScore::Reset(bool bResetScore)
{
	if (bResetScore)
		m_nScore = 0;

	m_nFrags = 0;
	m_nTronUnknown18 = 2;

	m_nTronUnknown0 = 0;
	m_nTronUnknown2 = 0;
	m_nTronUnknown4 = 0;
	m_nTronUnknown6 = 0;
	m_nTronUnknown8 = 0;
	m_nTronUnknownA = 0;
	m_nTronUnknownC = 0;
	m_bTronUnknownE = false;

	UpdateClients();
}

// RETAIL: retail 0x1013f000 (the name is ours)
void CPlayerScore::AddScore(int16 nScore)
{
	m_nScore += nScore;
}

// RETAIL: retail 0x1013f010 (the name is ours)
void CPlayerScore::AddFrags(int16 nFrags)
{
	m_nFrags += nFrags;
}

// RETAIL: retail 0x1013f020 (folded with other one-store functions)
void CPlayerScore::SetTronUnknown18(uint32 nTronUnknown)
{
	m_nTronUnknown18 = nTronUnknown;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::UpdateClients
//
//	PURPOSE:	Send the score to the clients (RETAIL: retail 0x1013f030;
//				the light cycle game type sends its figures as well)
//
// ----------------------------------------------------------------------- //

void CPlayerScore::UpdateClients(HCLIENT hClients /*= LTNULL*/)
{
	if (GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		CAutoMessage cMsg;

		cMsg.Writeuint8(MID_TRON_SCORE);
		cMsg.Writebool(true);
		cMsg.Writeuint32(m_nClientID);

		WriteTronUnknownA(cMsg);
		WriteTronUnknownB(cMsg);
		g_pLTServer->SendToClient(cMsg.Read(), hClients, MESSAGE_GUARANTEED);
	}
	else
	{
		CAutoMessage cMsg;

		cMsg.Writeuint8(MID_TRON_SCORE);
		cMsg.Writebool(true);
		cMsg.Writeuint32(m_nClientID);

		WriteData(cMsg);
		g_pLTServer->SendToClient(cMsg.Read(), hClients, MESSAGE_GUARANTEED);
	}
}

// RETAIL: retail 0x1013f230 (the name is ours)
void CPlayerScore::WriteTronUnknownB(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	pMsg->Writeint16(m_nTronUnknown2);
	pMsg->Writeint16(m_nTronUnknown6);
	pMsg->Writeint16(m_nTronUnknownA);
	pMsg->Writeint16(m_nTronUnknownC);
}

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeam::CTronTeam
//
//	PURPOSE:	Constructor (RETAIL: retail 0x1013f280)
//
// ----------------------------------------------------------------------- //

CTronTeam::CTronTeam()
{
	m_nID = 0xff;
	m_nTeamSize = 0;
	Reset();
}

// RETAIL: retail 0x1013f2e0
void CTronTeam::Init(uint8 nID)
{
	m_nID = nID;
	Reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeam::Reset
//
//	PURPOSE:	Clear the team (RETAIL: retail 0x1013f2f0)
//
// ----------------------------------------------------------------------- //

void CTronTeam::Reset()
{
	m_eState = kTronTeamState_Reset;
	m_nTronUnknownC = 0;
	m_nTronUnknownE = 0;
	m_fStateTime = 0;
	m_TronUnknown14.clear();
	m_nTronUnknown12 = 0;
	m_nMaxClients = 0;
	m_Clients.clear();
}

#ifndef _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeam::WriteData
//
//	PURPOSE:	Write the team for the clients (RETAIL: retail 0x1013f360)
//
// ----------------------------------------------------------------------- //

void CTronTeam::WriteData(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	uint32 nNumClients = m_Clients.size();

	pMsg->Writeuint8(m_nID);
	pMsg->Writeuint8(GetState());
	pMsg->Writeint16(m_nTronUnknownC);
	pMsg->Writeint16(m_nTronUnknownE);
	pMsg->Writeuint8(m_nMaxClients);
	pMsg->Writeuint8(m_nTeamSize);
	pMsg->Writeint16(m_nTronUnknown12);
	pMsg->Writeuint8(nNumClients);

	for (uint32 i = 0; i < nNumClients; i++)
	{
		pMsg->Writeuint32(GetClient(i));
	}
}

// RETAIL: retail 0x1013f420
void CTronTeam::ResetScores()
{
	m_eState = kTronTeamState_Reset;
	m_nTronUnknownC = 0;
	m_nTronUnknownE = 0;
	m_nTronUnknown12 = 0;
	UpdateClients();
}

// RETAIL: retail 0x1013f440 (folded with other one-store functions)
void CTronTeam::SetState(TronTeamState eState)
{
	m_eState = eState;
}

// RETAIL: retail 0x1013f450
void CTronTeam::AddTronUnknownC(int16 nAmount)
{
	m_nTronUnknownC += nAmount;
}

// RETAIL: retail 0x1013f460
void CTronTeam::AddTronUnknownE(int16 nAmount)
{
	m_nTronUnknownE += nAmount;
}

// RETAIL: retail 0x1013f470
void CTronTeam::AddTronUnknown12(int16 nAmount)
{
	m_nTronUnknown12 += nAmount;
}

// RETAIL: retail 0x1013f480 (folded with other one-store functions)
void CTronTeam::SetStateTime(LTFLOAT fTime)
{
	m_fStateTime = fTime;
}

// RETAIL: retail 0x1013f490
bool CTronTeam::IsStateTimeUp()
{
	if (g_pLTServer->GetTime() > m_fStateTime)
		return true;

	return false;
}

// RETAIL: retail 0x1013f4c0
std::vector<uint32>* CTronTeam::GetTronUnknown14()
{
	return &m_TronUnknown14;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeam::AddClient
//
//	PURPOSE:	Add a client to the team if it has room (RETAIL: retail
//				0x1013f4d0)
//
// ----------------------------------------------------------------------- //

bool CTronTeam::AddClient(uint32 nClientID)
{
	if (FindClient(nClientID) >= 0)
		return false;

	if (m_Clients.size() >= m_nMaxClients)
		return false;

	m_Clients.push_back(nClientID);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeam::RemoveClient
//
//	PURPOSE:	Remove a client; an emptied team loses its scores (RETAIL:
//				retail 0x1013f540)
//
// ----------------------------------------------------------------------- //

bool CTronTeam::RemoveClient(uint32 nClientID)
{
	std::vector<uint32>::iterator iter;
	for (iter = m_Clients.begin(); iter != m_Clients.end(); iter++)
	{
		if (*iter == nClientID)
		{
			m_Clients.erase(iter);

			if (m_Clients.size() == 0)
				ResetScores();

			return true;
		}
	}

	return false;
}

// RETAIL: retail 0x1013f5b0
void CTronTeam::SetMaxClients(uint8 nMaxClients, uint8 nTeamSize)
{
	m_nMaxClients = nMaxClients;
	m_nTeamSize = nTeamSize;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeam::UpdateClients
//
//	PURPOSE:	Send the team to the clients (RETAIL: retail 0x1013f5d0)
//
// ----------------------------------------------------------------------- //

void CTronTeam::UpdateClients(HCLIENT hClients /*= LTNULL*/)
{
	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_TRON_SCORE);
	cMsg.Writebool(false);

	WriteData(cMsg);
	g_pLTServer->SendToClient(cMsg.Read(), hClients, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeam::GetPlayer
//
//	PURPOSE:	The player of the team's nIndex'th client (RETAIL: retail
//				0x1013f6c0)
//
// ----------------------------------------------------------------------- //

CPlayerObj* CTronTeam::GetPlayer(uint32 nIndex)
{
	if (nIndex >= m_Clients.size())
		return LTNULL;

	CPlayerObj* pPlayer = LTNULL;

	HCLIENT hClient = g_pGameServerShell->GetClientFromID(m_Clients[nIndex]);
	if (hClient)
	{
		pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hClient);
	}

	return pPlayer;
}

// RETAIL: retail 0x1013f710 (the meaning, and the name, are ours)
uint8 CTronTeam::GetTronUnknownIndex()
{
	if (m_nID == TRON_UNASSIGNED_TEAM || m_nID > 8)
		return 7;

	return m_nID;
}

// RETAIL: retail 0x1013f720: the client's index in the team, or -1
int CTronTeam::FindClient(uint32 nClientID)
{
	int nIndex = 0;

	std::vector<uint32>::iterator iter;
	for (iter = m_Clients.begin(); iter != m_Clients.end(); iter++, nIndex++)
	{
		if (*iter == nClientID)
			break;
	}

	if (iter == m_Clients.end())
		return -1;

	return nIndex;
}

// RETAIL: retail 0x1013f750
uint32 CTronTeam::GetClient(uint32 nIndex)
{
	if (nIndex >= m_Clients.size())
		return 0;

	return m_Clients[nIndex];
}

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeamMgr::Init
//
//	PURPOSE:	Clear the teams (RETAIL: retail 0x1013f780)
//
// ----------------------------------------------------------------------- //

void CTronTeamMgr::Init()
{
	for (uint32 i = 0; i < TRON_MAX_TEAMS + 1; i++)
	{
		m_aTeams[i].Init(i);
		m_aTeams[i].Reset();

		if (i < TRON_MAX_TEAMS)
			m_aTeamIndex[i] = i;
	}

	m_nNumTeams = 0;
	m_nTeamSize = -1;
	m_nMaxPlayers = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeamMgr::GetTeam
//
//	PURPOSE:	A team by its index, or by the index map (RETAIL: retail
//				0x1013f7e0)
//
// ----------------------------------------------------------------------- //

CTronTeam* CTronTeamMgr::GetTeam(uint32 nTeam, bool bMapped)
{
	if (bMapped && nTeam < TRON_MAX_TEAMS)
	{
		nTeam = m_aTeamIndex[nTeam];
		if (nTeam >= TRON_MAX_TEAMS)
			nTeam = 0;
	}

	return &m_aTeams[nTeam];
}

#ifndef _CLIENTBUILD

// RETAIL: retail 0x1013f810: the team holding the client, or -1
int CTronTeamMgr::GetTeamOfClient(uint32 nClientID)
{
	for (int i = 0; i <= TRON_UNASSIGNED_TEAM; i++)
	{
		if (m_aTeams[i].FindClient(nClientID) >= 0)
			return i;
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeamMgr::SetupTeams
//
//	PURPOSE:	Split nMaxPlayers into teams of nTeamSize; the clients
//				that no longer fit go to the unassigned team (RETAIL:
//				retail 0x1013f850)
//
// ----------------------------------------------------------------------- //

bool CTronTeamMgr::SetupTeams(uint32 nTeamSize, uint32 nMaxPlayers)
{
	uint32 i;

	if (nMaxPlayers < m_nMaxPlayers)
		return false;

	m_nTeamSize = nTeamSize;
	m_aTeams[TRON_UNASSIGNED_TEAM].SetMaxClients(nMaxPlayers, nTeamSize);

	m_nNumTeams = nMaxPlayers / nTeamSize;
	m_nMaxPlayers = nMaxPlayers;

	for (i = 0; i < m_nNumTeams; i++)
	{
		CTronTeam* pTeam = &m_aTeams[i];
		uint32 nNumClients = pTeam->m_Clients.size();
		if (nNumClients > nTeamSize)
		{
			for (int j = nNumClients - 1; j >= (int)nTeamSize; j--)
			{
				MoveClient(pTeam->GetClient(j), TRON_UNASSIGNED_TEAM);
			}
		}

		pTeam->SetMaxClients(nTeamSize, nTeamSize);
	}

	uint32 nRemainder = nMaxPlayers % nTeamSize;
	if (nRemainder > 0)
	{
		m_nNumTeams++;

		CTronTeam* pTeam = &m_aTeams[i];
		if (pTeam->m_Clients.size() > nRemainder)
		{
			for (int j = pTeam->m_Clients.size() - 1; j >= (int)nRemainder; j--)
			{
				MoveClient(pTeam->GetClient(j), TRON_UNASSIGNED_TEAM);
			}
		}

		pTeam->SetMaxClients(nRemainder, nTeamSize);
		i++;
	}

	for (; i < TRON_MAX_TEAMS; i++)
	{
		CTronTeam* pTeam = &m_aTeams[i];
		for (int j = pTeam->m_Clients.size() - 1; j >= 0; j--)
		{
			MoveClient(pTeam->GetClient(j), TRON_UNASSIGNED_TEAM);
		}

		pTeam->SetMaxClients(0, nTeamSize);
	}

	UpdateClients();

	return true;
}

// RETAIL: retail 0x1013f9e0: take the client out of its team
bool CTronTeamMgr::RemoveClient(uint32 nClientID)
{
	for (uint32 i = 0; i < TRON_MAX_TEAMS + 1; i++)
	{
		if (m_aTeams[i].FindClient(nClientID) >= 0)
		{
			m_aTeams[i].RemoveClient(nClientID);
			m_aTeams[i].UpdateClients();
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeamMgr::MoveClient
//
//	PURPOSE:	Put the client in team nTeam, or, if that is full, in the
//				unassigned team (RETAIL: retail 0x1013fa40)
//
// ----------------------------------------------------------------------- //

bool CTronTeamMgr::MoveClient(uint32 nClientID, uint32 nTeam)
{
	RemoveClient(nClientID);

	if (m_aTeams[nTeam].m_Clients.size() >= m_aTeams[nTeam].m_nMaxClients)
	{
		m_aTeams[TRON_UNASSIGNED_TEAM].AddClient(nClientID);
		m_aTeams[TRON_UNASSIGNED_TEAM].UpdateClients();
		return false;
	}

	m_aTeams[nTeam].AddClient(nClientID);
	m_aTeams[nTeam].UpdateClients();
	return true;
}

// RETAIL: retail 0x1013fab0: the 16 teams' scores and start points
void CTronTeamMgr::ResetScores()
{
	for (uint32 i = 0; i < TRON_MAX_TEAMS; i++)
	{
		m_aTeams[i].ResetScores();
		m_aTeams[i].GetTronUnknown14()->clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeamMgr::UpdateClients
//
//	PURPOSE:	Send the team setup and every team to the clients (RETAIL:
//				retail 0x1013fb00)
//
// ----------------------------------------------------------------------- //

void CTronTeamMgr::UpdateClients(HCLIENT hClients /*= LTNULL*/)
{
	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(TSM_TEAMS);
	cMsg.Writeuint8(GetNumTeams());
	cMsg.Writeuint8(m_nTeamSize);
	g_pLTServer->SendToClient(cMsg.Read(), hClients, MESSAGE_GUARANTEED);

	for (uint32 i = 0; i < TRON_MAX_TEAMS + 1; i++)
	{
		m_aTeams[i].UpdateClients();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTeamMgr::SendToTeam
//
//	PURPOSE:	Send a message to the team's clients (RETAIL: retail
//				0x1013fc30; bTronUnknown limits it to the players whose
//				score's +0x18 state is 0 or 3)
//
// ----------------------------------------------------------------------- //

LTRESULT CTronTeamMgr::SendToTeam(ILTMessage_Read *pMsg, int nTeam, uint32 nFlags, bool bTronUnknown)
{
	LTRESULT res = LT_OK;

	if (nTeam < 0)
		return LT_ERROR;

	CTronTeam* pTeam = GetTeam(nTeam, false);
	int nNumClients = pTeam->m_Clients.size();
	for (int i = 0; i < nNumClients; i++)
	{
		CPlayerObj* pPlayer = pTeam->GetPlayer(i);
		if (!pPlayer)
			continue;

		if (bTronUnknown)
		{
			uint32 nTronUnknown = pPlayer->GetPlayerScore()->GetTronUnknown18();
			if (nTronUnknown != 0 && nTronUnknown != 3)
				continue;
		}

		LTRESULT resSend = g_pLTServer->SendToClient(pMsg, pPlayer->GetClient(), nFlags);
		if (resSend != LT_OK)
			res = resSend;
	}

	return res;
}

#endif // _CLIENTBUILD
