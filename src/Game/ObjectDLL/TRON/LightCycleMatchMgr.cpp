// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMatchMgr.cpp
//
// PURPOSE : The server's multiplayer light cycle game (races and matches)
//
// RETAIL: Tron 2.0's, retail-only; written from retail Object.lto
// 0x10053b80..0x10057220 (see LightCycleMatchMgr.h). The functions are in
// retail's order.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleMatchMgr.h"
#include "TronPlayerObj.h"
#include "GameServerShell.h"
#include "GameStartPoint.h"
#include "ServerLightCycleMgr.h"
#include "SharedScoring.h"
#include "MsgIDs.h"
#include <algorithm>
#include <limits.h>

CLightCycleMatchMgr* g_pLightCycleMatchMgr = LTNULL;

// RETAIL: a time to the nearest second (the messages' countdowns; the name
// is ours)
#define LCMM_ROUND(x)		((x) + (((x) > 0.0f) ? 0.5 : -0.5))

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::CLightCycleMatchMgr
//
// ----------------------------------------------------------------------- //

CLightCycleMatchMgr::CLightCycleMatchMgr()
{
	m_eState = kState_None;
	memset(m_apRacers, 0, sizeof(m_apRacers));
	m_nNumRacers = 0;
	m_nTotalRaces = 10;
	m_fClockStartTime = 0.0f;
	m_fPausedTime = 0.0f;
	m_fPauseStart = 0.0f;
	m_fRaceOverTime = 10.0f;
	m_fGameOverTime = 10.0f;
	m_fWaitTime = 20.0f;
	m_fLagTimeout = 10.0f;
	m_nKillPoints = 1;
	m_nSelfKillPoints = 1;
	m_nGameSpeed = 1;
	m_nTrailLength = 1;
	m_nMaxTeamSize = 1;
	m_nPowerups = (uint32)-1;
	m_fRaceStartDelay = 5.0f;
	m_nWinner = -1;
	m_nNextLevel = -1;
	m_fLastSendTime = 0.0f;
	m_nWinRacePoints = 6;
	m_nAllowedCycleType = 2;
	m_nTotalPoints = 0;
	m_fTimeLimit = 0.0f;
	m_nMaxPing = 50;
	m_nGameOverReason = 0;
	m_nLiving = 0;
	m_nNumTied = 0;
	m_bClockRunning = false;
	m_nCountdown = 30;
	m_fTronUnknownA4 = 0.0f;
	m_fLastPingCheck = 0.0f;
	m_nTronUnknownAC = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::Init
//
//	PURPOSE:	The game's settings from the server options
//
// ----------------------------------------------------------------------- //

bool CLightCycleMatchMgr::Init()
{
	ServerGameOptions& options = g_pGameServerShell->GetServerGameOptions();
	g_pLightCycleMatchMgr = this;

	m_nAllowedCycleType = options.m_nLCAllowedCycleType;
	m_nGameSpeed = options.m_nLCGameSpeed;
	m_nTrailLength = options.m_nLCTrailLength;
	m_nMaxTeamSize = options.m_nLCMaxTeamSize;
	m_fWaitTime = (float)options.m_nLCWaitForPlayersTime;
	m_nTotalRaces = options.m_nLCTotalRaces;
	m_nTotalPoints = options.m_nLCTotalPoints;
	m_fTimeLimit = (float)options.m_nLCTotalTime * 60.0f;
	m_nMaxPing = options.m_nLCMaxPing;
	m_nPowerups = options.m_nLCPowerups;
	m_nNumTied = 0;
	m_nNextLevel = -1;
	m_nKillPoints = options.m_nLCKillPointValue;
	m_nSelfKillPoints = options.m_nLCSelfKillPointValue;
	m_nWinRacePoints = options.m_nLCWinRacePointValue;

	g_pServerLightCycleMgr->SetGameSpeed(m_nGameSpeed, LTNULL);
	g_pServerLightCycleMgr->SetTrailLength(m_nTrailLength, LTNULL);

	Reset(true);

	g_pTronTeamMgr->SetupTeams(m_nMaxTeamSize, g_pGameServerShell->GetServerGameOptions().m_nLCMaxPlayers);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::Term
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::Term()
{
	memset(m_apRacers, 0, sizeof(m_apRacers));
	m_nNumRacers = 0;
	g_pLightCycleMatchMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::StartRace
//
//	PURPOSE:	The racers ride
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::StartRace()
{
	m_eState = kState_Racing;
	m_nTronUnknownAC = 0;

	if (!m_bClockRunning && m_fTimeLimit > 0.0f)
	{
		m_fClockStartTime = g_pLTServer->GetTime();
		m_nCountdown = 30;
		m_bClockRunning = true;
		m_fPausedTime = 0.0f;
		m_fPauseStart = 0.0f;
	}

	if (m_fPauseStart > 0.0f)
	{
		m_fPausedTime += g_pLTServer->GetTime() - m_fPauseStart;
		m_fPauseStart = 0.0f;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(LCMM_MSG_RACESTART);
	if (m_fTimeLimit > 0.0f)
	{
		cMsg.Writefloat(m_fTimeLimit - (g_pLTServer->GetTime() - m_fClockStartTime - m_fPausedTime));
	}
	else
	{
		cMsg.Writefloat(0.0f);
	}
	cMsg.Writeuint8(m_nRace);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	for (int i = 0; i < LCMM_MAX_RACERS; i++)
	{
		if (m_apRacers[i])
		{
			RemoveSpectator(m_apRacers[i], true);
			m_apRacers[i]->StartLightCycle(false, false);
			m_apRacers[i]->m_bTronUnknown2a80 = true;
			m_apRacers[i]->m_nTronUnknown31d4 = 0;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::Update
//
//	PURPOSE:	The state's timers, then the lagged and slow racers
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::Update()
{
	switch (m_eState)
	{
		case kState_Waiting:
		{
			if (m_Timer.Stopped() || m_nNumRacers == LCMM_MAX_RACERS)
			{
				StartRace();
			}
		}
		break;

		case kState_Racing:
		{
			if (m_fTimeLimit > 0.0f)
			{
				if (g_pLTServer->GetTime() - m_fClockStartTime - m_fPausedTime > m_fTimeLimit)
				{
					EndMatch(2);
					return;
				}

				int nTimeLeft = (uint8)(m_fTimeLimit - (g_pLTServer->GetTime() - m_fClockStartTime - m_fPausedTime));
				if (nTimeLeft < m_nCountdown)
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
					cMsg.Writeuint8(LCMM_MSG_COUNTDOWN);
					cMsg.Writeuint8(m_nCountdown);
					g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

					if (m_nCountdown == 30)
					{
						m_nCountdown = 15;
					}
					else if (m_nCountdown == 15)
					{
						m_nCountdown = 5;
					}
					else if (m_nCountdown == 5)
					{
						m_nCountdown = 0;
					}
				}
			}
		}
		break;

		case kState_RaceOver:
		{
			if (!m_Timer.Stopped())
			{
				break;
			}

			if (m_nTotalRaces > 0 && m_nRace >= m_nTotalRaces)
			{
				EndMatch(0);
				break;
			}

			m_nRace++;

			if (m_nTotalRaces > 0 && m_nRace == m_nTotalRaces)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
				cMsg.Writeuint8(LCMM_MSG_LASTRACE);

				// The leading team and the teams tied with it
				int nMax = INT_MIN;
				int nTied = 0;
				int8 nLeader = -1;
				for (uint32 i = 0; i < (uint32)g_pTronTeamMgr->GetNumTeams(); i++)
				{
					CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(i, false);
					if (pTeam->GetNumClients() > 0 && i != TRON_UNASSIGNED_TEAM)
					{
						if (pTeam->GetTronUnknown12() > nMax)
						{
							nMax = pTeam->GetTronUnknown12();
							nTied = 0;
							nLeader = i;
						}
						else if (pTeam->GetTronUnknown12() == nMax)
						{
							nTied++;
						}
					}
				}

				cMsg.Writeint8(nLeader);
				cMsg.Writeuint8(nTied);
				g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
			}

			PrepareRace();
		}
		break;

		case kState_GameOver:
		{
			if (!m_Timer.Stopped() && m_nNextLevel == -1)
			{
				break;
			}

			if (!m_nNumRacers && !m_lstWaiting.size() && m_nNextLevel == -1)
			{
				Reset(false);
				return;
			}

			m_bClockRunning = false;

			int nNumMaps = g_pGameServerShell->GetServerGameOptions().GetMapList()->size();
			if (m_nNextLevel != -1)
			{
				g_pGameServerShell->SwitchToLevel(m_nNextLevel);
				m_nNextLevel = -1;
			}
			else if (nNumMaps > 1)
			{
				m_eState = kState_None;
				m_nTronUnknownAC = m_nNumRacers;
				g_pGameServerShell->NextLevel();
				m_fTronUnknownA4 = 0.0f;
			}
			else
			{
				// The same map again
				for (int i = 0; i < LCMM_MAX_RACERS; i++)
				{
					if (m_apRacers[i])
					{
						CPlayerScore* pScore = &m_apRacers[i]->m_Score;
						pScore->m_nTronUnknown0 = 0;
						pScore->m_nTronUnknown2 = 0;
						pScore->m_nTronUnknown4 = 0;
						pScore->m_nTronUnknown6 = 0;
						pScore->m_nTronUnknown8 = 0;
						pScore->m_nTronUnknownA = 0;
						pScore->m_nTronUnknownC = 0;
						pScore->m_bTronUnknownE = false;
						pScore->UpdateClients(LTNULL);
					}
				}

				if (g_pTronTeamMgr)
				{
					g_pTronTeamMgr->ResetScores();
				}

				if (g_pGameServerShell)
				{
					g_pGameServerShell->SetUpdateGameServ();
				}

				m_nRace = 1;
				m_fClockStartTime = 0.0f;
				m_fPausedTime = 0.0f;
				m_fPauseStart = 0.0f;
				m_bClockRunning = false;

				PrepareRace();
			}
		}
		break;
	}

	// Kick the racers whose client stopped updating, and move those whose
	// ping stays too high to the spectators
	float fTime = g_pLTServer->GetTime();
	m_lstLagged.clear();

	for (uint32 nTeam = 0; nTeam < (uint32)g_pTronTeamMgr->GetNumTeams(); nTeam++)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
		for (uint32 i = 0; i < pTeam->GetNumClients(); i++)
		{
			CPlayerObj* pPlayer = pTeam->GetPlayer(i);
			if (!pPlayer)
			{
				continue;
			}

			if (pPlayer->m_fLastClientUpdateTime > 0.0f && fTime - pPlayer->m_fLastClientUpdateTime > m_fLagTimeout)
			{
				m_lstLagged.push_back(pPlayer);
			}
			else if (fTime - m_fLastPingCheck > 0.5f)
			{
				uint16 nPing;
				if (!CheckPing(pPlayer, nPing))
				{
					pPlayer->m_nTronUnknown31d4++;
					if ((int)pPlayer->m_nTronUnknown31d4 > 10)
					{
						MovePlayer(pPlayer, TRON_UNASSIGNED_TEAM);

						CAutoMessage cMsg;
						cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
						cMsg.Writeuint8(LCMM_MSG_PINGTOOHIGH);
						cMsg.Writeuint16(nPing);
						cMsg.Writeuint16(m_nMaxPing);
						g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
					}
				}
				else if ((int)pPlayer->m_nTronUnknown31d4 > 0)
				{
					pPlayer->m_nTronUnknown31d4--;
				}

				m_fLastPingCheck = fTime;
			}
		}
	}

	PlayerList::iterator iter;
	for (iter = m_lstLagged.begin(); iter != m_lstLagged.end(); iter++)
	{
		g_pLTServer->CPrint("Kicking Client %d: Lagged lightcycle player", g_pLTServer->GetClientID((*iter)->GetClient()));
		g_pLTServer->KickClient((*iter)->GetClient());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::AddPlayer
//
//	PURPOSE:	A client's player came in
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::AddPlayer(CPlayerObj* pPlayer)
{
	PlacePlayer(pPlayer);
	pPlayer->GetPlayerScore()->UpdateClients(LTNULL);
	SendState(pPlayer->GetClient());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::PlacePlayer
//
//	PURPOSE:	A player without a team spectates; one with a team races
//				when it can, else waits for the next race
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::PlacePlayer(CPlayerObj* pPlayer)
{
	int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(pPlayer->GetClient()));
	if (nTeam < 0 || nTeam == TRON_UNASSIGNED_TEAM)
	{
		AddSpectator(pPlayer);
		return;
	}

	switch (m_eState)
	{
		case kState_None:
		case kState_Waiting:
		{
			if (AddRacer(pPlayer, true))
			{
				StartWaiting();
			}
			else
			{
				AddWaiting(pPlayer);
			}
		}
		break;

		case kState_Racing:
		case kState_RaceOver:
		case kState_GameOver:
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
			cMsg.Writeuint8(LCMM_MSG_GAMEINPROGRESS);
			g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);

			AddWaiting(pPlayer);
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::RemovePlayer
//
//	PURPOSE:	A player is gone from the race, the waiting and spectators
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::RemovePlayer(CPlayerObj* pPlayer)
{
	for (int i = 0; i < LCMM_MAX_RACERS; i++)
	{
		if (m_apRacers[i] == pPlayer)
		{
			if (m_eState == kState_Racing)
			{
				HandleDerez(pPlayer, LTNULL, false);
			}

			m_apRacers[i] = LTNULL;
			m_nNumRacers--;

			g_pLTServer->CPrint("Player with client ID (%d) just left the game\n", g_pLTServer->GetClientID(pPlayer->GetClient()));

			if (m_nTronUnknownAC > 0)
			{
				m_nTronUnknownAC--;
			}

			if (m_eState == kState_Waiting)
			{
				if (pPlayer->m_pLastStartPoint)
				{
					pPlayer->m_pLastStartPoint->SetInUse(false);
				}

				SendWaiting(LTNULL);
			}
		}
	}

	PlayerList::iterator iter = std::find(m_lstWaiting.begin(), m_lstWaiting.end(), pPlayer);
	if (iter != m_lstWaiting.end())
	{
		m_lstWaiting.erase(iter);
	}

	RemoveSpectator(pPlayer, false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleDerez
//
//	PURPOSE:	Score a derezzed racer (and its killer), then end the race
//				or the match when one team is left
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleDerez(CPlayerObj* pPlayer, CPlayerObj* pKiller, bool bHeadOn)
{
	if (m_eState != kState_Racing)
	{
		return;
	}

	if (pPlayer)
	{
		if (pPlayer->m_Score.m_bTronUnknownE && !bHeadOn)
		{
			return;
		}

		g_pCommonLT->SetObjectFlags(pPlayer->m_hObject, OFT_Flags, 0, FLAG_SOLID);
		pPlayer->m_bTronUnknown2a80 = false;
	}

	if (bHeadOn && pKiller && !pKiller->m_Score.m_bTronUnknownE)
	{
		g_pCommonLT->SetObjectFlags(pKiller->m_hObject, OFT_Flags, 0, FLAG_SOLID);
		pKiller->m_bTronUnknown2a80 = false;
	}

	int nOutTeam = -1;
	int nNumTeamsIn = 0;
	int8 nWinTeam = -1;
	m_nLiving = 0;
	int nSurvivor = -1;
	bool bMatchOver = false;

	for (int i = 0; i < LCMM_MAX_RACERS; i++)
	{
		if (!m_apRacers[i])
		{
			continue;
		}

		if (m_apRacers[i] == pPlayer || (bHeadOn && m_apRacers[i] == pKiller))
		{
			// A derezzed racer
			if (m_apRacers[i]->m_Score.m_bTronUnknownE)
			{
				continue;
			}

			int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(m_apRacers[i]->GetClient()));
			CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);

			CPlayerScore* pScore = &m_apRacers[i]->m_Score;
			pScore->m_bTronUnknownE = true;

			if (m_nNumRacers > 1)
			{
				if (pKiller == pPlayer && !bHeadOn)
				{
					pScore->m_nTronUnknown8++;
					pScore->m_nTronUnknown0 -= m_nSelfKillPoints;
					pScore->m_nTronUnknownA++;
					pScore->m_nTronUnknown2 -= m_nSelfKillPoints;
					pTeam->AddTronUnknown12(-m_nSelfKillPoints);
				}
			}
			else
			{
				nSurvivor = i;
			}

			int nAlive = 0;
			for (uint32 j = 0; j < pTeam->GetNumClients(); j++)
			{
				CPlayerObj* pMate = pTeam->GetPlayer(j);
				if (pMate && !pMate->m_Score.m_bTronUnknownE)
				{
					nAlive++;
				}
			}

			if (!nAlive)
			{
				pTeam->SetState(CTronTeam::kTronTeamState_Timed);
				nOutTeam = nTeam;
			}

			pTeam->UpdateClients(LTNULL);
		}
		else
		{
			// A racer still riding
			if (!g_pLightCycleMgr->GetLivingCyclist(m_apRacers[i]->m_hObject))
			{
				continue;
			}

			m_nLiving++;
			nSurvivor = i;

			CPlayerObj* pRacer = m_apRacers[i];
			int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(pRacer->GetClient()));
			CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);

			if (m_apRacers[i] == pKiller)
			{
				pRacer->m_Score.m_nTronUnknown4++;
				pRacer->m_Score.m_nTronUnknown0 += m_nKillPoints;
				pRacer->m_Score.m_nTronUnknown6++;
				pRacer->m_Score.m_nTronUnknown2 += m_nKillPoints;
				pTeam->AddTronUnknown12(m_nKillPoints);
			}

			if (nTeam != nWinTeam)
			{
				nWinTeam = nTeam;
			}

			if (m_nTotalPoints > 0)
			{
				if (pTeam->GetTronUnknown12() >= m_nTotalPoints)
				{
					bMatchOver = true;
				}
				else
				{
					int nMaxGain = LTMAX(m_nKillPoints, m_nWinRacePoints);
					int nNeeded = m_nTotalPoints - pTeam->GetTronUnknown12();
					if (nNeeded <= nMaxGain)
					{
						CAutoMessage cMsg;
						cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
						cMsg.Writeuint8(LCMM_MSG_POINTSNEEDED);
						cMsg.Writeuint8(pTeam->GetID());
						cMsg.Writeint16(nNeeded);
						g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
					}
				}
			}

			pTeam->UpdateClients(LTNULL);
		}
	}

	for (uint32 nTeam = 0; nTeam < (uint32)g_pTronTeamMgr->GetNumTeams(); nTeam++)
	{
		if (g_pTronTeamMgr->GetTeam(nTeam, false)->GetState() == CTronTeam::kTronTeamState_TronUnknown0)
		{
			nNumTeamsIn++;
		}
	}

	// The last one riding wins the race
	if (m_nLiving <= 1 && nSurvivor != -1 && m_apRacers[nSurvivor])
	{
		int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(m_apRacers[nSurvivor]->GetClient()));
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);

		CPlayerScore* pScore = &m_apRacers[nSurvivor]->m_Score;
		if (m_nLiving > 0)
		{
			pScore->m_nTronUnknown0 += m_nWinRacePoints;
			pScore->m_nTronUnknown2 += m_nWinRacePoints;
			pTeam->AddTronUnknown12(m_nWinRacePoints);
			pScore->m_nTronUnknownC++;
			pTeam->AddTronUnknownC(1);
		}

		if (nTeam != nWinTeam)
		{
			nWinTeam = nTeam;
		}

		pTeam->UpdateClients(LTNULL);

		if (m_nTotalPoints > 0)
		{
			if (pTeam->GetTronUnknown12() >= m_nTotalPoints)
			{
				bMatchOver = true;
			}
			else
			{
				int nMaxGain = LTMAX(m_nKillPoints, m_nWinRacePoints);
				int nNeeded = m_nTotalPoints - pTeam->GetTronUnknown12();
				if (nNeeded <= nMaxGain)
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
					cMsg.Writeuint8(LCMM_MSG_POINTSNEEDED);
					cMsg.Writeuint8(pTeam->GetID());
					cMsg.Writeint16(nNeeded);
					g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
				}
			}
		}
	}

	UpdateScores(LTNULL);

	if (bMatchOver)
	{
		EndMatch(1);
		return;
	}

	if (m_nLiving > 1 && nNumTeamsIn > 1)
	{
		// The race goes on without a team
		if (nOutTeam != -1)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
			cMsg.Writeuint8(LCMM_MSG_ROUNDOVER);
			cMsg.Writeuint32(nOutTeam);
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
		}

		return;
	}

	if (nWinTeam != -1)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nWinTeam, false);
		pTeam->AddTronUnknownC(1);
		if (m_nLiving > 1)
		{
			pTeam->AddTronUnknown12(m_nWinRacePoints);
		}
	}

	EndRace(nWinTeam);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::EndRace
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::EndRace(int8 nWinner)
{
	m_eState = kState_RaceOver;
	m_Timer.Start(m_fRaceOverTime);
	m_lstReady.clear();
	m_nWinner = nWinner;
	m_nNumTied = 0;

	// The match's clock stops between the races
	m_fPauseStart = g_pLTServer->GetTime();

	SendRaceOver(LTNULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::OnMessage
//
//	PURPOSE:	Message 200 from a client: ready, or a team change
//
// ----------------------------------------------------------------------- //

bool CLightCycleMatchMgr::OnMessage(ILTMessage_Read *pMsg, HCLIENT hSender)
{
	switch (pMsg->Readuint8())
	{
		case LCMM_MSG_READY:
		{
			if (m_eState == kState_Waiting || (m_eState > kState_Racing && m_eState <= kState_GameOver))
			{
				CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
				if (!pPlayer)
				{
					return true;
				}

				PlayerList::iterator iter = std::find(m_lstReady.begin(), m_lstReady.end(), pPlayer);
				if (iter == m_lstReady.end())
				{
					for (int i = 0; i < LCMM_MAX_RACERS; i++)
					{
						if (m_apRacers[i] && m_apRacers[i] == pPlayer)
						{
							m_lstReady.push_back(pPlayer);

							if (m_eState == kState_Waiting && m_nNumRacers < m_nTronUnknownAC)
							{
								return true;
							}

							// Everyone is ready: no need to wait
							if (m_lstReady.size() == m_nNumRacers)
							{
								m_Timer.Stop();
								m_lstReady.clear();
							}
						}
					}
				}
			}
		}
		return true;

		case TSM_MOVECLIENT:
		{
			HandleTeamChange(pMsg);
		}
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::AddSpectator
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::AddSpectator(CPlayerObj* pPlayer)
{
	m_lstSpectators.push_back(pPlayer);
	g_pCommonLT->SetObjectFlags(pPlayer->m_hObject, OFT_Flags, 0, FLAG_SOLID);

	g_pLTServer->CPrint("Player with client ID (%d) just joined the spectators\n", g_pLTServer->GetClientID(pPlayer->GetClient()));

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(LCMM_MSG_SPECTATE);
	cMsg.Writebool(true);
	g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);

	pPlayer->m_bTronUnknown2a98 = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::AddWaiting
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::AddWaiting(CPlayerObj* pPlayer)
{
	m_lstWaiting.push_back(pPlayer);
	g_pCommonLT->SetObjectFlags(pPlayer->m_hObject, OFT_Flags, 0, FLAG_SOLID);

	g_pLTServer->CPrint("Player with client ID (%d) just joined the waiting players\n", g_pLTServer->GetClientID(pPlayer->GetClient()));

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(LCMM_MSG_SPECTATE);
	cMsg.Writebool(true);
	g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::AddRacer
//
//	PURPOSE:	A free slot for the player, respawned when bRespawn
//
// ----------------------------------------------------------------------- //

bool CLightCycleMatchMgr::AddRacer(CPlayerObj* pPlayer, bool bRespawn)
{
	for (int i = 0; i < LCMM_MAX_RACERS; i++)
	{
		if (!m_apRacers[i])
		{
			m_apRacers[i] = (CTronPlayerObj*)pPlayer;

			CPlayerScore* pScore = &pPlayer->m_Score;
			pScore->m_nTronUnknown0 = 0;
			pScore->m_nTronUnknown4 = 0;
			pScore->m_nTronUnknown8 = 0;
			pScore->m_bTronUnknownE = false;
			pScore->UpdateClients(LTNULL);

			g_pCommonLT->SetObjectFlags(pPlayer->m_hObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID);

			g_pLTServer->CPrint("Player with client ID (%d) just joined the game\n", g_pLTServer->GetClientID(pPlayer->GetClient()));

			m_nNumRacers++;

			if (bRespawn)
			{
				pPlayer->Respawn(LOAD_NEW_LEVEL, true, LTNULL, LTNULL);

				int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(pPlayer->GetClient()));
				CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
				pTeam->SetState(CTronTeam::kTronTeamState_TronUnknown0);
				pTeam->UpdateClients(LTNULL);

				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
				cMsg.Writeuint8(LCMM_MSG_RESPAWNED);
				g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);

				pPlayer->m_bTronUnknown2a80 = false;
			}

			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::StartWaiting
//
//	PURPOSE:	Wait for more racers
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::StartWaiting()
{
	if (m_eState != kState_Waiting)
	{
		m_eState = kState_Waiting;
		g_pLightCycleMgr->RemoveAllCyclists();
	}

	m_Timer.Start(m_fWaitTime);
	SendWaiting(LTNULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::SendWaiting
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::SendWaiting(HCLIENT hClient)
{
	m_fLastSendTime = g_pLTServer->GetTime();

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(LCMM_MSG_WAITING);
	cMsg.Writeuint8(m_nNumRacers);
	uint16 nTimeLeft = (uint16)LCMM_ROUND(m_Timer.GetCountdownTime());
	cMsg.Writeuint16(nTimeLeft);
	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::SendRaceOver
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::SendRaceOver(HCLIENT hClient)
{
	m_fLastSendTime = g_pLTServer->GetTime();

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(LCMM_MSG_RACEOVER);
	uint16 nTimeLeft = (uint16)LCMM_ROUND(m_Timer.GetCountdownTime());
	cMsg.Writeuint16(nTimeLeft);
	cMsg.Writeuint8(m_nRace);
	cMsg.Writeint8(m_nWinner);
	cMsg.Writeuint8(m_nLiving);
	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::SendGameOver
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::SendGameOver(HCLIENT hClient)
{
	m_fLastSendTime = g_pLTServer->GetTime();

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(LCMM_MSG_GAMEOVER);
	uint16 nTimeLeft = (uint16)LCMM_ROUND(m_Timer.GetCountdownTime());
	cMsg.Writeuint16(nTimeLeft);
	cMsg.Writeuint8(m_nGameOverReason);
	cMsg.Writeint8(m_nWinner);
	cMsg.Writeuint8(m_nNumTied);
	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::RemoveSpectator
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::RemoveSpectator(CPlayerObj* pPlayer, bool bNotify)
{
	PlayerList::iterator iter = std::find(m_lstSpectators.begin(), m_lstSpectators.end(), pPlayer);
	if (iter != m_lstSpectators.end())
	{
		m_lstSpectators.erase(iter);

		g_pLTServer->CPrint("Player with client ID (%d) just left the spectators\n", g_pLTServer->GetClientID(pPlayer->GetClient()));

		if (bNotify)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
			cMsg.Writeuint8(LCMM_MSG_SPECTATE);
			cMsg.Writebool(false);
			g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
		}
	}

	pPlayer->m_bTronUnknown2a98 = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::UpdateScores
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::UpdateScores(HCLIENT hClients)
{
	for (int i = 0; i < LCMM_MAX_RACERS; i++)
	{
		if (m_apRacers[i])
		{
			m_apRacers[i]->m_Score.UpdateClients(hClients);
		}
	}

	if (g_pGameServerShell)
	{
		g_pGameServerShell->SetUpdateGameServ();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::PrepareRace
//
//	PURPOSE:	The waiting players fill the free slots, all respawn and
//				wait for the race to start
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::PrepareRace()
{
	g_pGameServerShell->ResetStartPoints();

	if (m_nNumRacers < LCMM_MAX_RACERS && m_lstWaiting.size() > 0)
	{
		PlayerList::iterator iter = m_lstWaiting.begin();
		while (iter != m_lstWaiting.end())
		{
			bool bAdded = false;
			for (int i = 0; i < LCMM_MAX_RACERS && !bAdded; i++)
			{
				if (!m_apRacers[i] && AddRacer(*iter, false))
				{
					bAdded = true;
				}
			}

			if (bAdded)
			{
				iter = m_lstWaiting.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	if (m_nNumRacers)
	{
		for (int i = 0; i < LCMM_MAX_RACERS; i++)
		{
			if (m_apRacers[i])
			{
				m_apRacers[i]->EndLightCycle();

				CPlayerScore* pScore = m_apRacers[i]->GetPlayerScore();
				if (pScore)
				{
					pScore->m_nTronUnknown0 = 0;
					pScore->m_nTronUnknown4 = 0;
					pScore->m_nTronUnknown8 = 0;
					pScore->m_bTronUnknownE = false;
					pScore->UpdateClients(LTNULL);
				}

				m_apRacers[i]->Respawn(LOAD_NEW_LEVEL, true, LTNULL, LTNULL);

				int nTeam = g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(m_apRacers[i]->GetClient()));
				CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nTeam, false);
				pTeam->SetState(CTronTeam::kTronTeamState_TronUnknown0);
				pTeam->UpdateClients(LTNULL);

				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
				cMsg.Writeuint8(LCMM_MSG_RESPAWNED);
				g_pLTServer->SendToClient(cMsg.Read(), m_apRacers[i]->GetClient(), MESSAGE_GUARANTEED);

				m_apRacers[i]->m_bTronUnknown2a80 = false;
			}
		}

		m_eState = kState_Waiting;
		m_Timer.Start(m_fRaceStartDelay);
		m_lstReady.clear();

		SendWaiting(LTNULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::RemoveClient
//
//	PURPOSE:	A client is gone: its team loses it, and with no team left
//				a race ends
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::RemoveClient(HCLIENT hClient)
{
	if (g_pTronTeamMgr->GetTeamOfClient(g_pLTServer->GetClientID(hClient)) < 0)
	{
		return;
	}

	g_pTronTeamMgr->RemoveClient(g_pLTServer->GetClientID(hClient));

	int nTeams = 0;
	for (uint32 i = 0; i < (uint32)g_pTronTeamMgr->GetNumTeams(); i++)
	{
		if (g_pTronTeamMgr->GetTeam(i, false)->GetNumClients() > 0)
		{
			nTeams++;
		}
	}

	if (!nTeams && m_eState == kState_Racing)
	{
		Reset(false);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleTeamChange
//
//	PURPOSE:	A client asks to move a client to a team (TSM_MOVECLIENT)
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleTeamChange(ILTMessage_Read *pMsg)
{
	uint32 nClientID = pMsg->Readuint32();
	uint8 nTeam = pMsg->Readuint8();

	if (nTeam < TRON_UNASSIGNED_TEAM && (uint32)nTeam >= (uint32)g_pTronTeamMgr->GetNumTeams())
	{
		return;
	}

	int nOldTeam = g_pTronTeamMgr->GetTeamOfClient(nClientID);
	if (nOldTeam == nTeam)
	{
		return;
	}

	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nOldTeam, false);
	CPlayerObj* pPlayer = pTeam->GetPlayer(pTeam->FindClient(nClientID));
	if (pPlayer)
	{
		MovePlayer(pPlayer, nTeam);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::OnStartWorld
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::OnStartWorld()
{
	g_pTronTeamMgr->ResetScores();
	m_nNextLevel = -1;

	if (g_pGameServerShell)
	{
		g_pGameServerShell->SetUpdateGameServ();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::SetNextLevel
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::SetNextLevel(int nLevel)
{
	m_eState = kState_GameOver;
	m_nNextLevel = nLevel;
	m_Timer.Stop();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::SendState
//
//	PURPOSE:	The settings, teams, scores, cycles and state for a client
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::SendState(HCLIENT hClient)
{
	SendSettings(hClient);
	g_pTronTeamMgr->UpdateClients(hClient);
	UpdateScores(hClient);

	g_pServerLightCycleMgr->SetGameSpeed(m_nGameSpeed, hClient);
	g_pServerLightCycleMgr->SetTrailLength(m_nTrailLength, hClient);
	g_pServerLightCycleMgr->SendCyclistsToClient(hClient);

	switch (m_eState)
	{
		case kState_Waiting:	SendWaiting(hClient);	break;
		case kState_RaceOver:	SendRaceOver(hClient);	break;
		case kState_GameOver:	SendGameOver(hClient);	break;
	}

	uint32 nClientID = g_pLTServer->GetClientID(hClient);
	int nTeam = g_pTronTeamMgr->GetTeamOfClient(nClientID);
	if (nTeam != -1)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
		cMsg.Writeuint8(TSM_MOVECLIENT);
		cMsg.Writeuint32(nClientID);
		cMsg.Writeuint8(nTeam);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::SendSettings
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::SendSettings(HCLIENT hClient)
{
	ServerGameOptions& options = g_pGameServerShell->GetServerGameOptions();

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(LCMM_MSG_SETTINGS);
	cMsg.Writefloat(m_fTronUnknownA4);
	cMsg.Writeuint8(options.m_nLCAllowedCycleType);
	cMsg.Writeuint8(m_nRace);
	cMsg.Writeuint8(m_nTotalRaces);
	cMsg.Writeuint16(m_nTotalPoints);
	cMsg.Writeint8(options.m_nLCKillPointValue);
	cMsg.Writeint8(options.m_nLCSelfKillPointValue);
	cMsg.Writeint8(options.m_nLCWinRacePointValue);

	if (m_bClockRunning)
	{
		cMsg.Writefloat(m_fTimeLimit - (g_pLTServer->GetTime() - m_fClockStartTime - m_fPausedTime));
	}
	else
	{
		cMsg.Writefloat(m_fTimeLimit);
	}

	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::EndMatch
//
//	PURPOSE:	The team with the most points wins
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::EndMatch(uint8 nReason)
{
	m_nWinner = -1;
	m_nLiving = 0;
	m_nNumTied = 0;

	int nMax = INT_MIN;
	for (uint32 i = 0; i < (uint32)g_pTronTeamMgr->GetNumTeams(); i++)
	{
		CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(i, false);
		if (pTeam->GetNumClients() > 0 && i != TRON_UNASSIGNED_TEAM)
		{
			if (pTeam->GetTronUnknown12() > nMax)
			{
				nMax = pTeam->GetTronUnknown12();
				m_nNumTied = 0;
				m_nWinner = i;
			}
			else if (pTeam->GetTronUnknown12() == nMax)
			{
				m_nNumTied++;
			}
		}
	}

	m_eState = kState_GameOver;
	m_Timer.Start(m_fGameOverTime);
	m_lstReady.clear();
	m_nGameOverReason = nReason;

	SendGameOver(LTNULL);

	m_nRace = 1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::Reset
//
//	PURPOSE:	No racers, no scores: the game starts over
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::Reset(bool bClearSpectators)
{
	m_eState = kState_None;
	memset(m_apRacers, 0, sizeof(m_apRacers));
	m_nRace = 1;
	m_nNumRacers = 0;
	m_fClockStartTime = 0.0f;
	m_fPausedTime = 0.0f;
	m_fPauseStart = 0.0f;
	m_fTimeLimit = (float)g_pGameServerShell->GetServerGameOptions().m_nLCTotalTime * 60.0f;

	m_lstReady.clear();
	if (bClearSpectators)
	{
		m_lstSpectators.clear();
	}
	m_lstWaiting.clear();

	g_pTronTeamMgr->ResetScores();

	if (g_pGameServerShell)
	{
		g_pGameServerShell->SetUpdateGameServ();
	}

	m_bClockRunning = false;
	m_nLiving = 0;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
	cMsg.Writeuint8(LCMM_MSG_RESET);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	g_pGameServerShell->ResetStartPoints();

	m_fLastPingCheck = 0.0f;
	m_nTronUnknownAC = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::CheckPing
//
//	PURPOSE:	The player's ping, and whether it is low enough
//
// ----------------------------------------------------------------------- //

bool CLightCycleMatchMgr::CheckPing(CPlayerObj* pPlayer, uint16& nPing)
{
	float fPing;
	g_pLTServer->GetClientPing(pPlayer->GetClient(), fPing);
	nPing = (uint16)(fPing + 0.5f);

	return (nPing <= m_nMaxPing);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::MovePlayer
//
//	PURPOSE:	Move a player to a team (its ping allowing), and place it
//				again
//
// ----------------------------------------------------------------------- //

bool CLightCycleMatchMgr::MovePlayer(CPlayerObj* pPlayer, uint8 nTeam)
{
	if (!pPlayer)
	{
		return false;
	}

	uint8 nNewTeam = nTeam;
	uint32 nClientID = g_pLTServer->GetClientID(pPlayer->GetClient());
	int nOldTeam = g_pTronTeamMgr->GetTeamOfClient(nClientID);
	if (nOldTeam == nTeam)
	{
		return false;
	}

	CTronTeam* pOldTeam = g_pTronTeamMgr->GetTeam(nOldTeam, false);
	pPlayer->m_nTronUnknown31d4 = 0;

	if (nTeam != TRON_UNASSIGNED_TEAM)
	{
		uint16 nPing;
		if (!CheckPing(pPlayer, nPing))
		{
			g_pLTServer->CPrint("Player with client ID (%d) had too slow of a ping for this server (%d)", g_pLTServer->GetClientID(pPlayer->GetClient()), nPing);

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
			cMsg.Writeuint8(LCMM_MSG_PINGTOOHIGH);
			cMsg.Writeuint16(nPing);
			cMsg.Writeuint16(m_nMaxPing);
			g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);

			nNewTeam = TRON_UNASSIGNED_TEAM;
		}
	}

	RemoveSpectator(pPlayer, false);
	RemovePlayer(pPlayer);

	if (!g_pTronTeamMgr->MoveClient(nClientID, nNewTeam))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
		cMsg.Writeuint8(LCMM_MSG_MOVEFAILED);
		cMsg.Writeuint32(nClientID);
		cMsg.Writeuint8(nNewTeam);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

		return false;
	}

	int nCurTeam = g_pTronTeamMgr->GetTeamOfClient(nClientID);
	CTronTeam* pTeam = g_pTronTeamMgr->GetTeam(nCurTeam, false);
	CPlayerObj* pMoved = pTeam->GetPlayer(pTeam->FindClient(nClientID));
	if (pMoved)
	{
		CPlayerScore* pScore = &pMoved->m_Score;
		pScore->Reset(true);
		pScore->UpdateClients(LTNULL);
	}

	if (nOldTeam != nCurTeam)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_TRON_SERVERMESSAGE);
		cMsg.Writeuint8(TSM_MOVECLIENT);
		cMsg.Writeuint32(nClientID);
		cMsg.Writeuint8(nCurTeam);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}

	// Its cycle derezzes
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->GetLivingCyclist(pPlayer->m_hObject);
	if (pCyclist)
	{
		g_pLightCycleMgr->DerezCyclist(pCyclist, pCyclist, true);
	}

	if (!pOldTeam->GetNumClients())
	{
		pOldTeam->SetState(CTronTeam::kTronTeamState_Reset);
		pOldTeam->UpdateClients(LTNULL);
	}

	PlayerList::iterator iter = std::find(m_lstWaiting.begin(), m_lstWaiting.end(), pPlayer);
	if (iter != m_lstWaiting.end())
	{
		m_lstWaiting.erase(iter);
	}

	PlacePlayer(pPlayer);

	if (!m_nNumRacers && m_eState == kState_Waiting)
	{
		Reset(false);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::AddClient
//
//	PURPOSE:	A new client starts without a team
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::AddClient(HCLIENT hClient)
{
	g_pTronTeamMgr->MoveClient(g_pLTServer->GetClientID(hClient), TRON_UNASSIGNED_TEAM);
}
