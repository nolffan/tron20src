// ----------------------------------------------------------------------- //
//
// MODULE  : ServerDiscMatchMgr.cpp
//
// PURPOSE : The server's disc game (arena and tournament) match manager
//
// RETAIL: written from retail Object.lto 0x100493f0..0x1004f3a0 (see the
// header). The names are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerDiscMatchMgr.h"
#include "PlayerObj.h"
#include "ArenaStartPoint.h"
#include "GameServerShell.h"
#include "CommandMgr.h"
#include "MsgIDs.h"
#include "SharedScoring.h"
#include "Arena.h"
#include "CVarTrack.h"

// RETAIL: the delays before a round (retail 0x10294b38, 0x10294b3c: named
// constants, not literals). The names are ours.
static const float s_fRoundDelay = 5.0f;
static const float s_fLongRoundDelay = 7.0f;

// RETAIL: the delay before the next level and the time between two ping
// checks (retail 0x10294b4c, 0x10294b50: named constants, not literals).
// The names are ours.
static const float s_fLevelOverDelay = 10.0f;
static const float s_fPingCheckTime = 0.5f;

// RETAIL: the time of the last whole step of fStep before fTime that is
// still in the future and after fLast (retail 0x10049970)
static float GetNextTime(float fNext, float fStep, float fLast)
{
	float fNow = g_pLTServer->GetTime();
	float fTime = fNext;

	while (fNext > fNow && fNext > fLast)
	{
		fTime = fNext;
		fNext -= fStep;
	}

	return fTime;
}

// ----------------------------------------------------------------------- //
//
//	CDiscMatch
//
// ----------------------------------------------------------------------- //

CDiscMatch::CDiscMatch() : m_eState(0), m_nWinner(-1)
{
	m_nRoundB = -1;
	m_nTeam[1] = -1;
	m_nTeam[0] = -1;
	m_nRoundA = 0;
}

void CDiscMatch::Update()
{
	float fTime = g_pLTServer->GetTime();

	if ((m_eState == 1 || m_eState == 2) && fTime > m_fEndTime)
	{
		TimeOut();
		return;
	}

	if (m_eState == 1)
	{
		if (fTime > m_fRoundStartTime)
		{
			for (int i = 0; i < 2; i++)
			{
				CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(m_nTeam[i], false);
				int nNumClients = pTeam->GetNumClients();
				for (int j = 0; j < nNumClients; j++)
				{
					CPlayerObj* pPlayer = pTeam->GetPlayer(j);
					if (pPlayer)
					{
						CPlayerScore* pScore = pPlayer->GetPlayerScore();
						if (pScore->GetTronUnknown18() == 3)
						{
							pScore->SetTronUnknown18(0);
							pScore->UpdateClients();
						}
					}
				}
			}

			m_eState = 2;
			m_pArena->SendCommand(m_pArena->m_sFightCmd.c_str());

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
			cMsg.Writeuint8(kDiscMsg_RoundStart);
			CLTMsgRef_Read cRead = cMsg.Read();
			g_pTronTeamMgr->SendToTeam(cRead, m_nTeam[0], MESSAGE_GUARANTEED, true);
			g_pTronTeamMgr->SendToTeam(cRead, m_nTeam[1], MESSAGE_GUARANTEED, true);
		}
		else if (fTime > m_fNextTick)
		{
			uint16 nSeconds = (uint16)(m_fRoundStartTime - fTime + 0.1f);
			if (nSeconds > 0 && nSeconds <= 5)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
				cMsg.Writeuint8(kDiscMsg_Countdown);
				cMsg.Writeuint16(nSeconds);
				CLTMsgRef_Read cRead = cMsg.Read();
				g_pTronTeamMgr->SendToTeam(cRead, m_nTeam[0], MESSAGE_GUARANTEED, true);
				g_pTronTeamMgr->SendToTeam(cRead, m_nTeam[1], MESSAGE_GUARANTEED, true);
			}

			m_fNextTick = GetNextTime(m_fRoundStartTime, 1.0f, m_fNextTick);
		}
	}
	else if (m_eState == 3)
	{
		if (fTime > m_fRoundOverTime)
		{
			StartRound(s_fRoundDelay);
		}
	}
	else if (m_eState == 4)
	{
		if (fTime > m_fMatchOverTime)
		{
			m_eState = 5;
		}
	}
}

void CDiscMatch::Init(int nTeamA, int nTeamB, CDiscArena* pArena, float fDuration,
	int nRounds, const TournamentMatch& match, int nRoundA, int nRoundB)
{
	m_Match = match;
	m_nRoundA = nRoundA;
	m_nRoundB = nRoundB;
	Init(nTeamA, nTeamB, pArena, fDuration, nRounds, nRoundA == nRoundB);
}

void CDiscMatch::Init(int nTeamA, int nTeamB, CDiscArena* pArena, float fDuration,
	int nRounds, bool bLongDelay)
{
	for (int i = 0; i < 2; i++)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(!i ? nTeamA : nTeamB, false);
		uint32 nNumClients = pTeam->GetNumClients();
		for (uint32 j = 0; j < nNumClients; j++)
		{
			CPlayerObj* pPlayer = pTeam->GetPlayer(j);
			if (pPlayer)
			{
				CPlayerScore* pScore = pPlayer->GetPlayerScore();
				if (pScore->GetTronUnknown18() == 1)
				{
					pScore->SetTronUnknown18(3);
					pScore->UpdateClients();
				}
			}
		}
	}

	m_pArena = pArena;
	m_fDuration = fDuration;
	m_fEndTime = g_pLTServer->GetTime() + fDuration;
	m_nRounds = nRounds;
	m_nWinsNeeded = (nRounds + 1) / 2;
	m_nTeam[0] = nTeamA;
	m_nTeam[1] = nTeamB;
	m_nWins[0] = 0;
	m_nWins[1] = 0;

	m_pArena->SendCommand(m_pArena->m_sMatchStartCmd.c_str());

	StartRound(bLongDelay ? s_fLongRoundDelay : s_fRoundDelay);
	SendState(LTNULL, false, false);
}

void CDiscMatch::Term()
{
	m_pArena->SendCommand(m_pArena->m_sMatchEndCmd.c_str());
	m_pArena->m_bInUse = false;

	for (int i = 0; i < 2; i++)
	{
		if (m_nTeam[i] < 0)
			continue;

		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(m_nTeam[i], false);
		uint32 nNumClients = pTeam->GetNumClients();
		for (uint32 j = 0; j < nNumClients; j++)
		{
			CPlayerObj* pPlayer = pTeam->GetPlayer(j);
			if (pPlayer)
			{
				CPlayerScore* pScore = pPlayer->GetPlayerScore();
				uint32 nState = pScore->GetTronUnknown18();
				if (nState == 0 || nState == 3)
				{
					if (GetGameType() != eGameTypeDiscArena && pTeam->GetState() == 2)
						pScore->SetTronUnknown18(2);
					else
						pScore->SetTronUnknown18(1);
					pScore->UpdateClients();
				}
			}
		}
	}

	m_pArena = LTNULL;
	m_nTeam[1] = -1;
	m_nTeam[0] = -1;
	m_eState = 0;
}

void CDiscMatch::TimeOut()
{
	m_eState = 4;
	EndRound(-1);
}

void CDiscMatch::StartRound(float fDelay)
{
	m_eState = 1;
	float fStart = g_pLTServer->GetTime() + fDelay;
	m_fRoundStartTime = fStart;
	m_fNextTick = GetNextTime(fStart, 1.0f, 0.0f);
	m_nKills[1] = 0;
	m_nKills[0] = 0;

	for (int i = 0; i < 2; i++)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(m_nTeam[i], false);
		int nNumClients = pTeam->GetNumClients();
		for (int j = 0; j < nNumClients; j++)
		{
			CPlayerObj* pPlayer = pTeam->GetPlayer(j);
			if (!pPlayer)
				continue;

			CPlayerScore* pScore = pPlayer->GetPlayerScore();
			if (pScore->GetTronUnknown18() == 0)
			{
				pScore->SetTronUnknown18(3);
				pScore->UpdateClients();
			}

			if (pScore->GetTronUnknown18() == 3)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
				cMsg.Writeuint8(kDiscMsg_Spectate);
				cMsg.Writebool(true);
				g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);

				GameStartPoint* pStartPoint = (GameStartPoint*)m_pArena->GetTeamPoint(i, pPlayer);
				pPlayer->Respawn(2, false, pStartPoint, LTNULL);
				m_pArena->SendCommand(m_pArena->m_sRoundStartCmd.c_str());
			}
		}
	}
}

void CDiscMatch::EndRound(int nWinner)
{
	m_pArena->SendCommand(m_pArena->m_sRoundEndCmd.c_str());

	for (int i = 0; i < 2; i++)
	{
		if (m_nTeam[i] < 0)
			continue;

		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(m_nTeam[i], false);
		int nNumClients = pTeam->GetNumClients();
		for (int j = 0; j < nNumClients; j++)
		{
			CPlayerObj* pPlayer = pTeam->GetPlayer(j);
			if (pPlayer && pPlayer->GetPlayerScore()->GetTronUnknown18() == 0)
			{
				pPlayer->RemoveProjectiles();
			}
		}
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_MatchScore);
	cMsg.Writeuint8(m_nTeam[0]);
	cMsg.Writeuint8(m_nWins[0]);
	cMsg.Writeuint8(m_nWins[1]);
	if (nWinner >= 0)
	{
		cMsg.Writebool(true);
		cMsg.Writeuint8(m_nTeam[nWinner]);
	}
	else
	{
		cMsg.Writebool(false);
		cMsg.Writeuint8(m_nTeam[0]);
	}
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	int nMost = (m_nWins[0] > m_nWins[1]) ? m_nWins[0] : m_nWins[1];
	if (nMost < m_nWinsNeeded && m_eState != 4)
	{
		m_eState = 3;
		m_fRoundOverTime = g_pLTServer->GetTime() + 3.0f;
	}
	else
	{
		m_eState = (m_eState == 4) ? 5 : 4;
		m_fMatchOverTime = g_pLTServer->GetTime() + 4.0f;
		DecideWinner();

		CAutoMessage cOverMsg;
		cOverMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
		cOverMsg.Writeuint8(kDiscMsg_MatchOver);
		cOverMsg.Writeuint8(m_nTeam[GetWinner()]);
		g_pLTServer->SendToClient(cOverMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}

int CDiscMatch::RemovePlayer(CPlayerObj* pPlayer)
{
	int nSlot;
	if (IsOnTeam(m_nTeam[0], pPlayer))
	{
		nSlot = 0;
	}
	else if (IsOnTeam(m_nTeam[1], pPlayer))
	{
		nSlot = 1;
	}
	else
	{
		return -1;
	}

	uint32 nState = pPlayer->GetPlayerScore()->GetTronUnknown18();
	if (nState != 0 && nState != 3)
		return -1;

	int nAlive = 0;
	int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(pPlayer->GetClient()));
	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
	uint32 nNumClients = pTeam->GetNumClients();
	for (uint32 i = 0; i < nNumClients; i++)
	{
		CPlayerObj* pTeamPlayer = pTeam->GetPlayer(i);
		if (pTeamPlayer)
		{
			uint32 nTeamState = pTeamPlayer->GetPlayerScore()->GetTronUnknown18();
			if (nTeamState == 0 || nTeamState == 3)
			{
				nAlive++;
			}
		}
	}

	if (nAlive > 1)
	{
		if (nState == 0)
		{
			OnPlayerKilled(pPlayer, LTNULL);
		}
		return -1;
	}

	if (m_eState != 4 && m_eState != 5)
	{
		int nOther = (nSlot == 0);
		m_nWins[nOther] = m_nWinsNeeded;
		EndRound(nOther);
	}

	m_nTeam[nSlot] = -1;

	if (GetGameType() == eGameTypeDiscTournament)
	{
		return m_Match.nTeam[nSlot];
	}

	return -1;
}

void CDiscMatch::OnPlayerKilled(CPlayerObj* pPlayer, CPlayerObj* pKiller)
{
	CPlayerScore* pScore = pPlayer->GetPlayerScore();
	if (pScore->GetTronUnknown18() != 0)
		return;

	int nSlot, nOther;
	if (IsOnTeam(m_nTeam[0], pPlayer))
	{
		nSlot = 0;
		nOther = 1;
	}
	else if (IsOnTeam(m_nTeam[1], pPlayer))
	{
		nSlot = 1;
		nOther = 0;
	}
	else
	{
		return;
	}

	m_pArena->SendCommand(m_pArena->m_sKillCmd.c_str());

	bool bTeamDead = true;
	m_nKills[nSlot]++;

	int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(pPlayer->GetClient()));
	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
	uint32 nNumClients = pTeam->GetNumClients();
	for (uint32 i = 0; i < nNumClients; i++)
	{
		CPlayerObj* pTeamPlayer = pTeam->GetPlayer(i);
		if (pTeamPlayer && pTeamPlayer->GetPlayerScore()->GetTronUnknown18() == 0 && pTeamPlayer != pPlayer)
		{
			bTeamDead = false;
			break;
		}
	}

	pScore->SetTronUnknown18(3);
	pScore->UpdateClients();

	if (bTeamDead)
	{
		m_nWins[nOther]++;
		EndRound(nOther);
	}
}

void CDiscMatch::SendState(HCLIENT hClient, bool bTronUnknown, bool bFirst)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_AddMatch);
	cMsg.Writebool(bTronUnknown);
	cMsg.Writebool(bFirst);
	cMsg.Writeuint8(m_nTeam[0]);
	cMsg.Writeuint8(m_nTeam[1]);
	cMsg.Writeuint8(m_nWins[0]);
	cMsg.Writeuint8(m_nWins[1]);
	cMsg.Writeuint8(m_pArena->GetIndex());
	cMsg.Writeuint8(m_nRounds);
	if (GetGameType() == eGameTypeDiscArena)
	{
		cMsg.Writeuint8(0);
	}
	else
	{
		cMsg.Writeuint8(1);
	}
	cMsg.Writeint8(m_nRoundA);
	cMsg.Writeint8(m_nRoundB);
	cMsg.Writefloat(m_fEndTime - g_pLTServer->GetTime());
	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

bool CDiscMatch::HasPlayer(CPlayerObj* pPlayer)
{
	if (IsOnTeam(m_nTeam[0], pPlayer))
		return true;

	return IsOnTeam(m_nTeam[1], pPlayer) ? true : false;
}

void CDiscMatch::DecideWinner()
{
	if (m_eState != 5 && m_eState != 4)
		return;

	if (m_nWins[1] > m_nWins[0])
	{
		m_nWinner = 1;
		return;
	}

	// RETAIL: 0x1004d88c, the more wins and the draw lost by team 1 share
	// the last store.
	if (m_nWins[0] <= m_nWins[1])
	{
		// RETAIL: a draw goes to the team that lost less (the damage of its
		// living players, and 100 for each death)
		float afLost[2];
		afLost[1] = 0.0f;
		afLost[0] = 0.0f;
		for (int i = 0; i < 2; i++)
		{
			CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(m_nTeam[i], false);
			uint32 nNumClients = pTeam->GetNumClients();
			for (uint32 j = 0; j < nNumClients; j++)
			{
				CPlayerObj* pPlayer = pTeam->GetPlayer(j);
				if (pPlayer && pPlayer->GetPlayerScore()->GetTronUnknown18() == 0)
				{
					CDestructible* pDamage = pPlayer->GetDestructible();
					if (pDamage)
					{
						afLost[i] += 100.0f - pDamage->GetHitPoints();
					}
				}
			}

			afLost[i] += (float)m_nKills[i] * 100.0f;
		}

		if (afLost[0] > afLost[1])
		{
			m_nWinner = 1;
			return;
		}
	}

	m_nWinner = 0;
}

int CDiscMatch::GetWinner()
{
	return (m_nWinner < 0) ? 0 : m_nWinner;
}

int CDiscMatch::GetLoser()
{
	return !GetWinner();
}

bool CDiscMatch::IsOnTeam(int nTeam, CPlayerObj* pPlayer)
{
	return nTeam == g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(pPlayer->GetClient()));
}

// ----------------------------------------------------------------------- //
//
//	CDiscArena
//
// ----------------------------------------------------------------------- //

CDiscArena::CDiscArena(int nIndex) : m_nIndex(nIndex), m_bInUse(false)
{
	m_pArena = LTNULL;

	ObjArray<HOBJECT, 128> objArray;
	g_pLTServer->FindNamedObjects("Arena", objArray);

	uint32 i;
	Arena* pArena;
	ArenaStartPoint* pPoint;
	uint32 nNumObjects = objArray.NumObjects();
	for (i = 0; i < nNumObjects; i++)
	{
		pArena = (Arena*)g_pLTServer->HandleToObject(objArray.GetObject(i));
		if (!pArena || pArena->m_nArena != nIndex)
			continue;

		if (pArena->m_hstrOnPlayerSpawnCmd)
			m_sRoundStartCmd = g_pLTServer->GetStringData(pArena->m_hstrOnPlayerSpawnCmd);
		if (pArena->m_hstrOnPlayerDeathCmd)
			m_sKillCmd = g_pLTServer->GetStringData(pArena->m_hstrOnPlayerDeathCmd);
		if (pArena->m_hstrOnMatchStartCmd)
			m_sMatchStartCmd = g_pLTServer->GetStringData(pArena->m_hstrOnMatchStartCmd);
		if (pArena->m_hstrOnMatchEndCmd)
			m_sMatchEndCmd = g_pLTServer->GetStringData(pArena->m_hstrOnMatchEndCmd);
		if (pArena->m_hstrOnPointStartCmd)
			m_sFightCmd = g_pLTServer->GetStringData(pArena->m_hstrOnPointStartCmd);
		if (pArena->m_hstrOnPointEndCmd)
			m_sRoundEndCmd = g_pLTServer->GetStringData(pArena->m_hstrOnPointEndCmd);

		m_pArena = pArena;
		break;
	}

	g_pLTServer->FindNamedObjects("ArenaStartPoint", objArray);

	nNumObjects = objArray.NumObjects();
	for (i = 0; i < nNumObjects; i++)
	{
		pPoint = (ArenaStartPoint*)g_pLTServer->HandleToObject(objArray.GetObject(i));
		if (!pPoint || pPoint->GetArena() != nIndex)
			continue;

		switch (pPoint->GetType())
		{
			case ArenaStartPoint::kType_Spectator:
				m_SpectatorPoints.push_back(pPoint);
				break;

			case ArenaStartPoint::kType_Team1:
				m_aTeamPoints[0].push_back(pPoint);
				break;

			case ArenaStartPoint::kType_Team2:
				m_aTeamPoints[1].push_back(pPoint);
				break;
		}
	}

	// RETAIL: start from random points
	uint32 nTeamPoints = m_aTeamPoints[0].size();
	if (m_aTeamPoints[1].size() < nTeamPoints)
		nTeamPoints = m_aTeamPoints[1].size();
	uint32 nNumPoints = m_SpectatorPoints.size();

	uint32 nPoint = (uint32)(((float)rand() / 32768.0f) * (float)nNumPoints);
	if (nPoint >= nNumPoints)
		nPoint = nNumPoints - 1;
	m_nSpectatorPoint = nPoint;

	nPoint = (uint32)(((float)rand() / 32768.0f) * (float)nTeamPoints);
	if (nPoint >= nTeamPoints)
		nPoint = nTeamPoints - 1;
	m_nTeamPoint = nPoint;
}

ArenaStartPoint* CDiscArena::GetSpectatorPoint(CPlayerObj* pPlayer)
{
	uint32 nNumPoints = m_SpectatorPoints.size();
	uint32 nPoint = m_nSpectatorPoint;
	bool bFree = false;
	uint32 nCount = nNumPoints;

	m_nSpectatorPoint = nPoint + 1;
	if (!nNumPoints)
		return LTNULL;

	m_nSpectatorPoint = (nPoint + 1) % nNumPoints;

	// RETAIL: tries each point once, from the one after the last
	for (; nCount; nCount--)
	{
		if (bFree)
			break;

		nPoint = (nPoint + 1) % nNumPoints;

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_SpectatorPoints[nPoint]->m_hObject, &vPos);
		bFree = !g_pGameServerShell->IsPositionOccupied(vPos, pPlayer);
	}

	return m_SpectatorPoints[nPoint];
}

ArenaStartPoint* CDiscArena::GetTeamPoint(uint32 nSlot, CPlayerObj* pPlayer)
{
	if (nSlot > 1)
		return LTNULL;

	ArenaStartPointArray& points = m_aTeamPoints[nSlot];
	uint32 nNumPoints = points.size();
	uint32 nPoint = m_nTeamPoint;
	uint32 nCount = nNumPoints;

	m_nTeamPoint = nPoint + 1;
	bool bFree = false;
	if (!nNumPoints)
		return LTNULL;

	m_nTeamPoint = (nPoint + 1) % nNumPoints;

	// RETAIL: tries each point once, from the one after the last
	for (; nCount; nCount--)
	{
		if (bFree)
			break;

		nPoint = (nPoint + 1) % nNumPoints;

		LTVector vPos;
		g_pLTServer->GetObjectPos(points[nPoint]->m_hObject, &vPos);
		bFree = !g_pGameServerShell->IsPositionOccupied(vPos, pPlayer);
	}

	return points[nPoint];
}

void CDiscArena::SendCommand(const char* pszCmd)
{
	if (*pszCmd && g_pCmdMgr->IsValidCmd(pszCmd))
	{
		g_pCmdMgr->Process(pszCmd, (ILTBaseClass*)m_pArena, LTNULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	CDiscMatchMgr
//
// ----------------------------------------------------------------------- //

// RETAIL: the warnings before the end of an arena game (retail .rdata
// 0x10294b54)
static const float s_afWarningTimes[3] = { 30.0f, 15.0f, 5.0f };

// RETAIL: the console variable for friendly fire (retail .data 0x1037c2a0)
static CVarTrack s_vtFriendlyFire;

CDiscMatchMgr::CDiscMatchMgr()
{
	m_bStarted = false;
}

bool CDiscMatchMgr::Init()
{
	ServerGameOptions& options = g_pGameServerShell->GetServerGameOptions();
	m_nGameType = options.m_eGameType;
	m_fLastPingCheck = 0.0f;
	m_nTronUnknown28 = -1;

	if (!s_vtFriendlyFire.IsInitted())
	{
		s_vtFriendlyFire.Init(g_pLTServer, "NetFriendlyFire", LTNULL, 0.0f);
	}

	if (m_nGameType == eGameTypeDiscArena)
	{
		m_nTronUnknown39 = options.m_nArenaMaxPlayers;
		m_nTronUnknown3A = options.m_nArenaMaxTeamSize;
		m_nRounds = options.m_nArenaPointsPerMatch;
		m_fMatchTime = (float)options.m_nArenaMatchTimeLimit * 60.0f;
		m_fPreGameDelay = (float)options.m_nArenaPreGameDelay;
		m_nGamesPerMap = options.m_nArenaGamesPerMap;
		m_nTronUnknown45 = (uint8)options.m_nArenaAirControl;
		m_nTronUnknown46 = options.m_bArenaDiscsInSpectator;
		m_nNumMaps = options.m_saArenaMaps.size();
		m_nJoinTime = options.m_nArenaMaxPing;
		m_nWinsToEnd = options.m_nArenaWinLimit;
		m_fTimeLimit = (float)options.m_nArenaTimeLimit * 60.0f;
		s_vtFriendlyFire.SetFloat(options.m_bArenaFriendlyFire ? 1.0f : 0.0f);
	}
	else if (m_nGameType == eGameTypeDiscTournament)
	{
		m_nTronUnknown39 = options.m_nTournMaxPlayers;
		m_nTronUnknown3A = options.m_nTournMaxTeamSize;
		m_nRounds = options.m_nTournPointsPerMatch;
		m_fMatchTime = (float)options.m_nTournMatchTimeLimit * 60.0f;
		m_fPreGameDelay = (float)options.m_nTournPreGameDelay;
		m_nGamesPerMap = options.m_nTournGamesPerMap;
		m_nTronUnknown45 = (uint8)options.m_nTournAirControl;
		m_nTronUnknown46 = options.m_bTournDiscsInSpectator;
		m_nNumMaps = options.m_saTournMaps.size();
		m_nJoinTime = options.m_nTournMaxPing;
		m_bDoubleElimination = false;
		m_nTeamsToStart = options.m_nTournTeamsToStart;
		s_vtFriendlyFire.SetFloat(options.m_bTournFriendlyFire ? 1.0f : 0.0f);
	}
	else
	{
		return false;
	}

	g_pTronTeamMgr->SetupTeams(m_nTronUnknown3A, m_nTronUnknown39);
	return true;
}

void CDiscMatchMgr::Term()
{
	int nNumArenas = m_Arenas.size();
	for (int i = 0; i < nNumArenas; i++)
	{
		if (m_Arenas[i])
		{
			debug_delete(m_Arenas[i]);
		}
		m_Arenas[i] = LTNULL;
	}
}

void CDiscMatchMgr::Reset(int nMaxTeamSize)
{
	m_bLevelOver = false;
	m_bLevelOverWarning = false;
	m_fNextTick = 0.0f;
	m_nGamesPlayed = 0;
	m_fEndTime = -1.0f;
	m_fWarningTime = -1.0f;
	m_nWarning = 0;
	m_fLastPingCheck = 0.0f;
	m_nTronUnknown28 = -1;

	float fStart = g_pLTServer->GetTime() + m_fPreGameDelay;
	m_fStartTime = fStart;
	m_fNextTick = GetNextTime(fStart, 1.0f, m_fNextTick);

	g_pTronTeamMgr->SetupTeams((nMaxTeamSize < m_nTronUnknown3A) ? nMaxTeamSize : m_nTronUnknown3A, m_nTronUnknown39);
	g_pTronTeamMgr->ResetScores();

	m_Arenas.resize(4);
	for (int i = 0; i < 4; i++)
	{
		m_Arenas[i] = debug_new1(CDiscArena, i);
	}
}

void CDiscMatchMgr::SetNextLevel(int nLevel)
{
	// RETAIL: 0x100499c5, the switch comes first.
	if (!m_bStarted)
	{
		g_pGameServerShell->SwitchToLevel(nLevel);
	}
	else
	{
		m_nTronUnknown28 = nLevel;
	}
}

void CDiscMatchMgr::StartTournament()
{
	m_bStarted = true;

	int nNumTeams = g_pTronTeamMgr->GetNumTeams();
	for (int i = 0; i < nNumTeams; i++)
	{
		if (g_pTronTeamMgr->GetTeam(i, false)->GetNumClients() > 0)
		{
			m_Bracket.AddTeam(i);
		}
	}

	m_Bracket.Start(m_bDoubleElimination);
	ResetTeams();

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_Waiting);
	cMsg.Writeuint16(0);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

void CDiscMatchMgr::StartGame()
{
	m_bStarted = true;
	if (m_fTimeLimit > 0.0f)
	{
		m_fEndTime = g_pLTServer->GetTime() + m_fTimeLimit;
		m_fWarningTime = m_fEndTime - s_afWarningTimes[0];
		m_nWarning = 0;
	}
	else
	{
		m_nWarning = 3;
		m_fEndTime = -1.0f;
		m_fWarningTime = -1.0f;
	}

	ResetTeams();

	int nNumTeams = g_pTronTeamMgr->GetNumTeams();
	m_Queue.Clear();
	for (int i = 0; i < nNumTeams; i++)
	{
		if (g_pTronTeamMgr->GetTeam(i, false)->GetNumClients() > 0)
		{
			m_Queue.Add(i);
		}
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_Waiting);
	cMsg.Writeuint16(0);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

void CDiscMatchMgr::ResetTeams()
{
	g_pTronTeamMgr->ResetScores();

	int nNumTeams = g_pTronTeamMgr->GetNumTeams();
	for (int i = 0; i < nNumTeams; i++)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(i, false);
		uint32 nNumClients = pTeam->GetNumClients();
		if (nNumClients)
		{
			pTeam->SetState((CTronTeam::TronTeamState)0);
			pTeam->UpdateClients();
		}

		for (uint32 j = 0; j < nNumClients; j++)
		{
			CPlayerObj* pPlayer = pTeam->GetPlayer(j);
			if (pPlayer)
			{
				pPlayer->m_nTronUnknown31d4 = 0;
				CPlayerScore* pScore = pPlayer->GetPlayerScore();
				int nFrags = pScore->GetFrags();
				pScore->AddFrags(-nFrags);
				pScore->SetTronUnknown18(1);
				pScore->UpdateClients();
				if (g_pGameServerShell)
				{
					g_pGameServerShell->SetUpdateGameServ();
				}
			}
		}
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_Timer);
	if (GetGameType() == eGameTypeDiscArena)
	{
		if (m_fEndTime > 0.0f)
			cMsg.Writefloat(m_fEndTime - g_pLTServer->GetTime());
		else
			cMsg.Writefloat(-1.0f);
	}
	else
	{
		cMsg.Writeuint8(m_Bracket.GetRoundsLeft(true));
	}
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

void CDiscMatchMgr::EndGame()
{
	GameOver();

	m_fEndTime = -1.0f;
	m_fWarningTime = -1.0f;
	m_bStarted = false;
	m_nWarning = 0;

	if (m_nTronUnknown28 != -1)
	{
		m_bLevelOver = true;
		m_fStartTime = g_pLTServer->GetTime();
	}
	else if (++m_nGamesPlayed >= m_nGamesPerMap && m_nNumMaps > 1)
	{
		m_bLevelOver = true;
		m_bLevelOverWarning = true;
		m_fStartTime = g_pLTServer->GetTime() + s_fLevelOverDelay;
	}
	else
	{
		float fStart = g_pLTServer->GetTime() + m_fPreGameDelay + 5.0f;
		m_fStartTime = fStart;
		m_fNextTick = GetNextTime(fStart, 1.0f, m_fNextTick) + 5.0f;
	}

	if (m_nGameType == eGameTypeDiscArena)
	{
		m_Queue.Clear();
	}
	else
	{
		m_Bracket.Reset();
	}

	int nNumTeams = g_pTronTeamMgr->GetNumTeams();
	for (int i = 0; i < nNumTeams; i++)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(i, false);
		uint32 nNumClients = pTeam->GetNumClients();
		for (uint32 j = 0; j < nNumClients; j++)
		{
			CPlayerObj* pPlayer = pTeam->GetPlayer(j);
			if (pPlayer)
			{
				pPlayer->GetPlayerScore()->SetTronUnknown18(2);
				pPlayer->GetPlayerScore()->UpdateClients();
			}
		}
	}

	AssignWaitingPlayers();
}

void CDiscMatchMgr::Update()
{
	if (m_bLevelOver)
	{
		if (g_pLTServer->GetTime() > m_fStartTime && m_fStartTime > 0.0f)
		{
			bool bOk;
			if (m_nTronUnknown28 != -1)
			{
				bOk = g_pGameServerShell->SwitchToLevel(m_nTronUnknown28);
				m_nTronUnknown28 = -1;
			}
			else
			{
				bOk = g_pGameServerShell->NextLevel();
			}

			// RETAIL: a failed switch keeps the level going (0x1004a097)
			if (!bOk)
			{
				m_bLevelOver = m_bLevelOverWarning = false;
			}

			m_fStartTime = -1.0f;
			return;
		}

		// RETAIL: tell the clients once, halfway
		if (m_bLevelOverWarning && g_pLTServer->GetTime() > m_fStartTime - s_fLevelOverDelay * 0.5f)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
			cMsg.Writeuint8(kDiscMsg_TronUnknown101);
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
			m_bLevelOverWarning = false;
		}
		return;
	}

	// RETAIL: players over the ping limit become spectators after ten checks
	if (m_bStarted && m_nJoinTime > 0 && g_pLTServer->GetTime() > m_fLastPingCheck + s_fPingCheckTime)
	{
		uint32 nNumTeams = g_pTronTeamMgr->GetNumTeams();
		for (uint32 i = 0; i < nNumTeams; i++)
		{
			CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(i, false);
			uint32 nNumClients = pTeam->GetNumClients();
			for (uint32 j = 0; j < nNumClients; j++)
			{
				CPlayerObj* pPlayer = pTeam->GetPlayer(j);
				if (!pPlayer)
					continue;

				// RETAIL: the count is signed (0x1004a24e, 0x1004a353)
				uint16 nTime;
				if (!CanJoin(pPlayer, &nTime))
				{
					if ((int)++pPlayer->m_nTronUnknown31d4 > 10)
					{
					uint32 nClientID = g_pLTServer->GetClientID(pPlayer->GetClient());
					ChangeTeam(nClientID, TRON_MAX_TEAMS);

					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
					cMsg.Writeuint8(kDiscMsg_JoinWait);
					cMsg.Writeuint16(nTime);
					cMsg.Writeuint16(m_nJoinTime);
					g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
					}
				}
				else if ((int)pPlayer->m_nTronUnknown31d4 > 0)
				{
					pPlayer->m_nTronUnknown31d4--;
				}
			}
		}

		m_fLastPingCheck = g_pLTServer->GetTime();
	}

	if (m_nGameType == eGameTypeDiscArena)
	{
		UpdateMatches();
	}
	else
	{
		UpdateTournament();
	}
}

void CDiscMatchMgr::UpdateTournament()
{
	UpdateTeamStates();

	if (!m_bStarted)
	{
		float fTime = g_pLTServer->GetTime();
		if (GetNumActiveTeams() < m_nTeamsToStart)
		{
			m_fStartTime = fTime + m_fPreGameDelay;
		}
		else if (fTime > m_fStartTime)
		{
			StartTournament();
		}
		else if (fTime > m_fNextTick)
		{
			uint16 nSeconds = (uint16)(m_fStartTime - fTime + 0.1f);
			if (nSeconds > 0)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
				cMsg.Writeuint8(kDiscMsg_Waiting);
				cMsg.Writeuint16(nSeconds);
				g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
			}

			m_fNextTick = GetNextTime(m_fStartTime, 1.0f, m_fNextTick);
		}

		AssignWaitingPlayers();
		return;
	}

	DiscMatchList::iterator iter = m_Matches.begin();
	while (iter != m_Matches.end())
	{
		DiscMatchList::iterator next = iter;
		next++;
		CDiscMatch* pMatch = *iter;

		pMatch->Update();
		if (pMatch->m_eState == 5)
		{
			int nWinner = pMatch->GetWinner();
			int nLoser = pMatch->GetLoser();
			m_Bracket.SetWinner(&pMatch->m_Match, pMatch->m_Match.nTeam[nWinner], true);

			int nTeam = pMatch->m_nTeam[nWinner];
			if (nTeam >= 0)
			{
				CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
				pTeam->AddTronUnknownC(1);
				pTeam->UpdateClients();
				SpawnTeam(nTeam, -1);
			}

			nTeam = pMatch->m_nTeam[nLoser];
			if (nTeam >= 0)
			{
				CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
				pTeam->AddTronUnknownE(1);
				pTeam->SetState((CTronTeam::TronTeamState)2);
				pTeam->UpdateClients();
				pTeam->SetStateTime(g_pLTServer->GetTime() + 2.0f);
				SpawnTeam(nTeam, -1);
			}

			pMatch->Term();
			delete pMatch;
			m_Matches.erase(iter);
		}

		iter = next;
	}

	if (m_Bracket.IsOver())
	{
		EndGame();
		return;
	}

	if (m_nTronUnknown28 != -1)
	{
		iter = m_Matches.begin();
		while (iter != m_Matches.end())
		{
			DiscMatchList::iterator next = iter;
			next++;
			CDiscMatch* pMatch = *iter;
			pMatch->TimeOut();
			pMatch->Term();
			delete pMatch;
			m_Matches.erase(iter);
			iter = next;
		}

		EndGame();
		return;
	}

	while (GetNumFreeArenas() > 0)
	{
		TournamentMatch match;
		if (!m_Bracket.GetNextMatch(match))
			break;

		// RETAIL: a bye: the other team goes on
		if (match.nTeam[0] == -1 || match.nTeam[1] == -1)
		{
			int nSlot = (match.nTeam[0] == -1) ? 1 : 0;
			int nIndex = match.nTeam[nSlot];
			AwardWin(m_Bracket.GetTeam(nIndex)->nTeam);
			m_Bracket.SetWinner(&match, nIndex, true);
			continue;
		}

		int nTeamA = m_Bracket.GetTeam(match.nTeam[0])->nTeam;
		int nTeamB = m_Bracket.GetTeam(match.nTeam[1])->nTeam;

		// RETAIL: a team that left loses
		if (nTeamA < 0 || nTeamB < 0 ||
			!g_pTronTeamMgr->GetTeam(nTeamA, false)->GetNumClients() ||
			!g_pTronTeamMgr->GetTeam(nTeamB, false)->GetNumClients())
		{
			int nIndex;
			if (nTeamA < 0)
				nIndex = match.nTeam[1];
			else if (nTeamB < 0)
				nIndex = match.nTeam[0];
			else if (!g_pTronTeamMgr->GetTeam(nTeamA, false)->GetNumClients() &&
				g_pTronTeamMgr->GetTeam(nTeamB, false)->GetNumClients())
				nIndex = match.nTeam[1];
			else if (!g_pTronTeamMgr->GetTeam(nTeamB, false)->GetNumClients() &&
				g_pTronTeamMgr->GetTeam(nTeamA, false)->GetNumClients())
				nIndex = match.nTeam[0];
			else
				continue;

			AwardWin(m_Bracket.GetTeam(nIndex)->nTeam);
			m_Bracket.SetWinner(&match, nIndex, true);
			continue;
		}

		RemoveWaitingTeam(nTeamA);
		RemoveWaitingTeam(nTeamB);

		int nRoundsLeft = m_Bracket.GetRoundsLeft(true);
		int nRound = m_Bracket.GetRound(match.nIndex, match.bWinners) + nRoundsLeft - 3;

		CDiscArena* pArena = FindArena(nTeamA, nTeamB);
		CDiscMatch* pMatch = debug_new(CDiscMatch);
		m_Matches.push_back(pMatch);
		pMatch->Init(nTeamA, nTeamB, pArena, m_fMatchTime, m_nRounds, match, nRound, nRoundsLeft);

		// RETAIL: the final
		if (nRound == nRoundsLeft)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
			cMsg.Writeuint8(kDiscMsg_TeamMessage);
			cMsg.Writeuint8(nTeamA);
			cMsg.Writeuint8(nTeamB);
			cMsg.Writeuint8(pArena->GetIndex());
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
		}
	}

	AssignWaitingPlayers();
}


void CDiscMatchMgr::UpdateMatches()
{
	if (!m_bStarted)
	{
		float fTime = g_pLTServer->GetTime();
		if (fTime > m_fStartTime)
		{
			StartGame();
		}
		else if (fTime > m_fNextTick)
		{
			uint16 nSeconds = (uint16)(m_fStartTime - fTime + 0.1f);
			if (nSeconds > 0)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
				cMsg.Writeuint8(kDiscMsg_Waiting);
				cMsg.Writeuint16(nSeconds);
				g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
			}

			m_fNextTick = GetNextTime(m_fStartTime, 1.0f, m_fNextTick);
		}

		AssignWaitingPlayers();
		return;
	}

	bool bTimeUp = IsTimeUp();

	DiscMatchList::iterator iter = m_Matches.begin();
	while (iter != m_Matches.end())
	{
		DiscMatchList::iterator next = iter;
		next++;
		CDiscMatch* pMatch = *iter;

		if (bTimeUp)
		{
			pMatch->TimeOut();
		}
		else
		{
			pMatch->Update();
		}

		if (pMatch->m_eState == 5)
		{
			if (!bTimeUp)
			{
				int nWinner = pMatch->GetWinner();
				int nLoser = pMatch->GetLoser();

				int nTeam = pMatch->m_nTeam[nWinner];
				if (nTeam >= 0)
				{
					CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
					pTeam->AddTronUnknownC(1);
					pTeam->UpdateClients();

					// RETAIL: one more win ends the game
					if (m_nWinsToEnd - pTeam->GetTronUnknownC() == 1)
					{
						CAutoMessage cMsg;
						cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
						cMsg.Writeuint8(kDiscMsg_TeamMessage);
						cMsg.Writebool(true);
						cMsg.Writeuint8(nTeam);
						g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
					}
				}

				nTeam = pMatch->m_nTeam[nLoser];
				if (nTeam >= 0)
				{
					CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
					pTeam->AddTronUnknownE(1);
					pTeam->UpdateClients();
				}
			}

			for (int i = 0; i < 2; i++)
			{
				if (pMatch->m_nTeam[i] >= 0)
				{
					SpawnTeam(pMatch->m_nTeam[i], -1);
				}
			}

			m_Queue.Return(pMatch->m_nTeam[0], pMatch->m_nTeam[1]);
			pMatch->Term();
			delete pMatch;
			m_Matches.erase(iter);
		}

		iter = next;
	}

	if (bTimeUp)
	{
		EndGame();
		return;
	}

	if (!IsTimeUp())
	{
		UpdateWarnings();

		while (GetNumFreeArenas() > 0)
		{
			int nTeamA = -1;
			int nTeamB = -1;
			if (!m_Queue.GetNextPair(&nTeamA, &nTeamB, GetNumActiveTeams()))
				break;

			RemoveWaitingTeam(nTeamA);
			RemoveWaitingTeam(nTeamB);

			CDiscArena* pArena = FindArena(nTeamA, nTeamB);
			CDiscMatch* pMatch = debug_new(CDiscMatch);
			m_Matches.push_back(pMatch);
			pMatch->Init(nTeamA, nTeamB, pArena, m_fMatchTime, m_nRounds, false);
		}
	}

	AssignWaitingPlayers();
}

bool CDiscMatchMgr::IsTimeUp()
{
	if (m_nTronUnknown28 != -1)
		return true;

	if (m_fEndTime > 0.0f && g_pLTServer->GetTime() > m_fEndTime)
		return true;

	int nNumTeams = g_pTronTeamMgr->GetNumTeams();
	for (int i = 0; i < nNumTeams; i++)
	{
		if (g_pTronTeamMgr->GetTeam(i, false)->GetTronUnknownC() >= m_nWinsToEnd)
			return true;
	}

	return false;
}

void CDiscMatchMgr::UpdateWarnings()
{
	if (g_pLTServer->GetTime() < m_fWarningTime || m_fWarningTime == -1.0f)
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_TeamMessage);
	cMsg.Writebool(false);
	cMsg.Writeuint8((uint8)s_afWarningTimes[m_nWarning]);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	m_nWarning++;
	if (m_nWarning < 3)
	{
		m_fWarningTime = m_fEndTime - s_afWarningTimes[m_nWarning];
	}
	else
	{
		m_fWarningTime = -1.0f;
	}
}

void CDiscMatchMgr::MakeSpectator(HCLIENT hClient)
{
	g_pTronTeamMgr->MoveClient(g_pLTServer->GetClientID(hClient), TRON_MAX_TEAMS);
}

void CDiscMatchMgr::OnPlayerAdded(CPlayerObj* pPlayer)
{
	SpawnPlayer(pPlayer, -1);
	pPlayer->GetPlayerScore()->UpdateClients();
	SendState(pPlayer->GetClient());
}

void CDiscMatchMgr::OnClientRemoved(HCLIENT hClient)
{
	int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(hClient));
	if (nTeam >= 0)
	{
		g_pTronTeamMgr->RemoveClient(g_pLTServer->GetClientID(hClient));
	}

	// RETAIL: 0x1004b13e: the sar of the client count stays, which VC6
	// gives for "<= 0" or "> 0" of the uint32, not for "== 0" or "!= 0"
	if (m_nGameType == eGameTypeDiscArena && nTeam >= 0 && nTeam < TRON_MAX_TEAMS &&
		g_pTronTeamMgr->GetTeam(nTeam, false)->GetNumClients() <= 0)
	{
		m_Queue.Remove(nTeam);
	}
}

void CDiscMatchMgr::OnPlayerRemoved(CPlayerObj* pPlayer)
{
	m_Waiting.remove(pPlayer);

	for (DiscMatchList::iterator iter = m_Matches.begin(); iter != m_Matches.end(); iter++)
	{
		int nIndex = (*iter)->RemovePlayer(pPlayer);
		if (m_nGameType == eGameTypeDiscTournament && nIndex >= 0)
		{
			TournamentTeam team;
			team.nTeam = -1;
			team.nIndex = nIndex;
			m_Bracket.SetTeam(nIndex, team);
		}
	}
}

void CDiscMatchMgr::OnPlayerKilled(CPlayerObj* pPlayer, CPlayerObj* pKiller)
{
	DiscMatchList::iterator iter = m_Matches.begin();
	pPlayer->RemoveProjectiles();

	for (; iter != m_Matches.end(); iter++)
	{
		(*iter)->OnPlayerKilled(pPlayer, LTNULL);
	}
}

void CDiscMatchMgr::OnPlayerRespawn(CPlayerObj* pPlayer)
{
	for (DiscMatchList::iterator iter = m_Matches.begin(); iter != m_Matches.end(); iter++)
	{
		if ((*iter)->HasPlayer(pPlayer))
		{
			SpawnPlayer(pPlayer, (*iter)->m_pArena->m_nIndex);
			return;
		}
	}

	SpawnPlayer(pPlayer, -1);
}

void CDiscMatchMgr::SendState(HCLIENT hClient)
{
	SendSettings(hClient);
	g_pTronTeamMgr->UpdateClients(hClient);

	bool bFirst = true;
	for (DiscMatchList::iterator iter = m_Matches.begin(); iter != m_Matches.end(); ++iter)
	{
		(*iter)->SendState(hClient, true, bFirst);
		bFirst = false;
	}

	if (m_bLevelOver)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
		cMsg.Writeuint8(kDiscMsg_TronUnknown101);
		g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
	}
}

void CDiscMatchMgr::SendSettings(HCLIENT hClient)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_Settings);
	cMsg.Writeuint8(m_nTronUnknown45);

	if (m_nGameType == eGameTypeDiscArena)
	{
		cMsg.Writeuint8(m_nWinsToEnd);
		cMsg.Writefloat((m_fEndTime > 0.0f) ? m_fEndTime - g_pLTServer->GetTime() : -1.0f);
	}
	else
	{
		cMsg.Writeuint8(m_bStarted ? m_Bracket.GetRoundsLeft(true) : 0);
	}

	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

bool CDiscMatchMgr::OnMessage(ILTMessage_Read* pMsg, HCLIENT hSender)
{
	if (pMsg->Peekuint8() != kDiscMsg_TeamChange)
		return false;

	pMsg->Readuint8();
	HandleTeamChange(pMsg);
	return true;
}

void CDiscMatchMgr::HandleTeamChange(ILTMessage_Read* pMsg)
{
	uint32 nClientID = pMsg->Readuint32();
	uint8 nTeam = pMsg->Readuint8();

	int nOldTeam = g_pTronTeamMgr->GetTeamOfClient(nClientID);
	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nOldTeam, false);
	CPlayerObj* pPlayer = pTeam->GetPlayer(pTeam->FindClient(nClientID));
	if (!pPlayer)
		return;

	uint16 nTime;
	if (nTeam != TRON_MAX_TEAMS && !CanJoin(pPlayer, &nTime))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
		cMsg.Writeuint8(kDiscMsg_JoinWait);
		cMsg.Writeuint16(nTime);
		cMsg.Writeuint16(m_nJoinTime);
		g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
		return;
	}

	ChangeTeam(nClientID, nTeam);
}

void CDiscMatchMgr::ChangeTeam(uint32 nClientID, uint8 nTeam)
{
	if (nTeam < TRON_MAX_TEAMS && nTeam >= (uint32)g_pTronTeamMgr->GetNumTeams())
		return;

	int nOldTeam = g_pTronTeamMgr->GetTeamOfClient(nClientID);
	if (nOldTeam == nTeam)
		return;

	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nOldTeam, false);
	CPlayerObj* pPlayer = pTeam->GetPlayer(pTeam->FindClient(nClientID));
	if (!pPlayer)
		return;

	pPlayer->m_nTronUnknown31d4 = 0;
	OnPlayerRemoved(pPlayer);
	OnClientRemoved(pPlayer->GetClient());
	SpawnPlayer(pPlayer, -1);
	g_pTronTeamMgr->MoveClient(nClientID, nTeam);

	int nNewTeam = g_pTronTeamMgr->GetTeamOfClient(nClientID);
	pTeam = g_pTronTeamMgr->GetTeam(nNewTeam, false);
	pPlayer = pTeam->GetPlayer(pTeam->FindClient(nClientID));
	if (pPlayer)
	{
		CPlayerScore* pScore = pPlayer->GetPlayerScore();
		int nFrags = pScore->GetFrags();
		pScore->AddFrags(-nFrags);

		if (m_nGameType == eGameTypeDiscArena)
		{
			if (m_bStarted && nNewTeam != TRON_MAX_TEAMS)
			{
				pPlayer->GetPlayerScore()->SetTronUnknown18(1);
			}
			else
			{
				pPlayer->GetPlayerScore()->SetTronUnknown18(2);
			}
		}
		else
		{
			pPlayer->GetPlayerScore()->SetTronUnknown18(2);
		}
		pPlayer->GetPlayerScore()->UpdateClients();

		if (g_pGameServerShell)
		{
			g_pGameServerShell->SetUpdateGameServ();
		}
	}

	if (m_bStarted && m_nGameType == eGameTypeDiscArena && pTeam->GetNumClients() == 1)
	{
		// RETAIL: the team's index goes in nTeam (0x1004b894)
		nTeam = pTeam->GetID();
		if (nTeam < TRON_MAX_TEAMS)
		{
			m_Queue.Add(nTeam);
			pTeam->SetState((CTronTeam::TronTeamState)0);
			pTeam->UpdateClients();
		}
	}

	if (nOldTeam != nNewTeam)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
		cMsg.Writeuint8(kDiscMsg_TeamChange);
		cMsg.Writeuint32(nClientID);
		cMsg.Writeuint8(nNewTeam);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

		// RETAIL: the player is not checked on the second path (a retail
		// bug, see RETAIL_BUGS.md)
		pPlayer->m_bTronUnknown31c9 = (pPlayer && pTeam->GetTeamSize() > 1 && pTeam->GetID() < TRON_MAX_TEAMS) ?
			pTeam->GetTronUnknownIndex() : pPlayer->m_bTronUnknown31ca;
	}
}

void CDiscMatchMgr::GameOver()
{
	DiscTeamList aWinners[3];

	if (m_nGameType == eGameTypeDiscArena)
	{
		int nNumTeams = g_pTronTeamMgr->GetNumTeams();
		int nBest = -1;
		int i;
		for (i = 0; i < nNumTeams; i++)
		{
			CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(i, false);
			if (pTeam->GetNumClients() > 0 && pTeam->GetTronUnknownC() > nBest)
			{
				nBest = pTeam->GetTronUnknownC();
			}
		}

		for (i = 0; i < nNumTeams; i++)
		{
			CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(i, false);
			if (pTeam->GetNumClients() > 0 && pTeam->GetTronUnknownC() >= nBest)
			{
				aWinners[0].push_back(i);
			}
		}
	}
	else if (m_nTronUnknown28 == -1)
	{
		int nSecond = -1;
		int nFirst = m_Bracket.GetTeam(m_Bracket.GetWinnersChampion())->nTeam;
		if (m_bDoubleElimination)
		{
			nSecond = m_Bracket.GetTeam(m_Bracket.GetLosersChampion())->nTeam;
		}

		if (nFirst < 0)
		{
			nFirst = nSecond;
			nSecond = -1;
		}

		if (nFirst >= 0)
		{
			aWinners[0].push_back(nFirst);
			if (nSecond >= 0)
			{
				aWinners[1].push_back(nSecond);
			}
		}
	}

	DiscTeamList::iterator iter;
	for (iter = aWinners[0].begin(); iter != aWinners[0].end(); iter++)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(*iter, false);
		int nNumClients = pTeam->GetNumClients();
		for (int j = 0; j < nNumClients; j++)
		{
			CPlayerObj* pPlayer = pTeam->GetPlayer(j);
			if (pPlayer)
			{
				CPlayerScore* pScore = pPlayer->GetPlayerScore();
				if (pScore->GetTronUnknown18() != 2)
				{
					pScore->AddScore(1);
					pScore->UpdateClients();
					if (g_pGameServerShell)
					{
						g_pGameServerShell->SetUpdateGameServ();
					}
				}
			}
		}
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_GameOver);

	int i;
	for (i = 0; i < 3; i++)
	{
		cMsg.Writeuint8(aWinners[i].size());
	}

	for (i = 0; i < 3; i++)
	{
		for (iter = aWinners[i].begin(); iter != aWinners[i].end(); iter++)
		{
			cMsg.Writeuint8(*iter);
		}
	}

	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

void CDiscMatchMgr::AwardWin(int nTeam)
{
	if (nTeam < 0)
		return;

	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
	pTeam->AddTronUnknownC(1);
	pTeam->UpdateClients();

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_TeamWin);
	CLTMsgRef_Read cRead = cMsg.Read();
	g_pTronTeamMgr->SendToTeam(cRead, nTeam, MESSAGE_GUARANTEED, false);
}

int CDiscMatchMgr::GetNumActiveTeams()
{
	int nActive = 0;
	uint32 nNumTeams = g_pTronTeamMgr->GetNumTeams();
	for (uint32 i = 0; i < nNumTeams; i++)
	{
		if (g_pTronTeamMgr->GetTeam(i, false)->GetNumClients() > 0)
		{
			nActive++;
		}
	}

	return nActive;
}

void CDiscMatchMgr::UpdateTeamStates()
{
	uint32 nNumTeams = g_pTronTeamMgr->GetNumTeams();
	for (uint32 i = 0; i < nNumTeams; i++)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(i, false);
		if (pTeam->GetState() == 2 && pTeam->IsStateTimeUp())
		{
			pTeam->SetState((CTronTeam::TronTeamState)1);
			pTeam->UpdateClients();
		}
	}
}

int CDiscMatchMgr::GetNumFreeArenas()
{
	int nFree = 0;
	uint32 nNumArenas = m_Arenas.size();
	for (uint32 i = 0; i < nNumArenas; i++)
	{
		if (!m_Arenas[i]->m_bInUse)
		{
			nFree++;
		}
	}

	return nFree;
}

// RETAIL: an arena to rank for a match (the nodes of a sorted std::list;
// the name is ours)
struct DiscArenaChoice
{
	float	fPlayed;	// how long ago both teams played in it
	uint32	nRandom;
	uint32	nArena;

	bool operator<(const DiscArenaChoice& other) const
	{
		if (fPlayed < other.fPlayed)
			return true;
		if (other.fPlayed < fPlayed)
			return false;
		return (int)nRandom < (int)other.nRandom;
	}
};

CDiscArena* CDiscMatchMgr::FindArena(int nTeamA, int nTeamB)
{
	uint32 nNumArenas = m_Arenas.size();
	std::vector<uint32>* pPlayedA = g_pTronTeamMgr->GetTeam(nTeamA, false)->GetTronUnknown14();
	std::vector<uint32>* pPlayedB = g_pTronTeamMgr->GetTeam(nTeamB, false)->GetTronUnknown14();

	uint32 i;
	if (pPlayedA->size() < nNumArenas)
	{
		pPlayedA->clear();
		pPlayedA->reserve(nNumArenas);
		for (i = 0; i < nNumArenas; i++)
		{
			pPlayedA->push_back(1000);
		}
	}

	if (pPlayedB->size() < nNumArenas)
	{
		pPlayedB->clear();
		pPlayedB->reserve(nNumArenas);
		for (i = 0; i < nNumArenas; i++)
		{
			pPlayedB->push_back(1000);
		}
	}

	std::list<DiscArenaChoice> lstChoices;
	for (i = 0; i < nNumArenas; i++)
	{
		DiscArenaChoice choice;
		choice.fPlayed = 0.0f;
		choice.nRandom = 0;
		choice.fPlayed = (float)(int)((*pPlayedA)[i] + (*pPlayedB)[i]) * 0.5f;
		float fRandom = (float)rand() * (1.0f / 32768.0f);
		uint32 nRandom = (uint32)(fRandom * 1024.0f);
		if (nRandom >= 1024)
			nRandom = 1023;
		choice.nRandom = nRandom;
		choice.nArena = i;
		lstChoices.push_back(choice);
	}

	lstChoices.sort();

	for (std::list<DiscArenaChoice>::iterator iter = lstChoices.begin(); iter != lstChoices.end(); iter++)
	{
		uint32 nArena = iter->nArena;
		if (m_Arenas[nArena]->m_bInUse)
			continue;

		for (i = 0; i < nNumArenas; i++)
		{
			if (i == nArena)
			{
				(*pPlayedA)[i] = 0;
				(*pPlayedB)[i] = 0;
			}
			else
			{
				(*pPlayedA)[i]++;
				(*pPlayedB)[i]++;
			}
		}

		m_Arenas[nArena]->m_bInUse = true;
		return m_Arenas[nArena];
	}

	return LTNULL;
}

void CDiscMatchMgr::RemoveWaitingTeam(int nTeam)
{
	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
	uint32 nNumClients = pTeam->GetNumClients();
	for (uint32 i = 0; i < nNumClients; i++)
	{
		CPlayerObj* pPlayer = pTeam->GetPlayer(i);
		if (pPlayer)
		{
			m_Waiting.remove(pPlayer);
		}
	}
}

void CDiscMatchMgr::AssignWaitingPlayers()
{
	for (DiscPlayerList::iterator iter = m_Waiting.begin(); iter != m_Waiting.end(); iter++)
	{
		// RETAIL: an arena with a match to watch, from a random one on
		// RETAIL: the random number is a variable of its own: retail multiplies
		// twice (0x1004c5e0), where one expression folds the two constants.
		float fRandom = (float)rand() * (1.0f / 32768.0f);
		uint32 nArena = (uint32)(fRandom * 4.0f);
		if (nArena >= 4)
			nArena = 3;

		for (int i = 0; i < 4; i++, nArena++)
		{
			if ((int)nArena >= 4)
				nArena = 0;

			if (m_Arenas[nArena]->m_bInUse)
				break;
		}

		if ((int)nArena >= 4)
			nArena = 0;

		SpawnPlayer(*iter, nArena);
	}

	m_Waiting.clear();
}

void CDiscMatchMgr::SpawnPlayer(CPlayerObj* pPlayer, int nArena)
{
	if (nArena < 0)
	{
		m_Waiting.push_back(pPlayer);
		return;
	}

	GameStartPoint* pPoint = (GameStartPoint*)m_Arenas[nArena]->GetSpectatorPoint(pPlayer);
	pPlayer->Respawn(2, false, pPoint, LTNULL);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(kDiscMsg_Spectate);
	cMsg.Writebool(false);
	g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
}

void CDiscMatchMgr::SpawnTeam(int nTeam, int nArena)
{
	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
	uint32 nNumClients = pTeam->GetNumClients();
	for (uint32 i = 0; i < nNumClients; i++)
	{
		CPlayerObj* pPlayer = pTeam->GetPlayer(i);
		if (pPlayer)
		{
			SpawnPlayer(pPlayer, nArena);
		}
	}
}

bool CDiscMatchMgr::CanJoin(CPlayerObj* pPlayer, uint16* pnTime)
{
	*pnTime = 0;
	if (!pPlayer)
		return true;

	if (m_nJoinTime <= 0)
		return true;

	float fTime;
	g_pLTServer->GetClientPing(pPlayer->GetClient(), fTime);
	if (fTime < 0.0f)
	{
		fTime = 0.0f;
	}

	*pnTime = (uint16)(fTime + 0.5f);
	return m_nJoinTime >= *pnTime;
}
