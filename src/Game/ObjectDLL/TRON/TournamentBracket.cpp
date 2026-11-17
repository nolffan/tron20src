// ----------------------------------------------------------------------- //
//
// MODULE  : TournamentBracket.cpp
//
// PURPOSE : The 16-team single or double elimination bracket of the disc
//			 game types' tournament
//
// RETAIL: Tron 2.0's, written from retail Object.lto (TournamentBracket.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TournamentBracket.h"

// RETAIL: 0x10294e00: the seed order of the first round's slots
static const int s_aSeeds[BRACKET_TEAMS] = { 0, 15, 8, 7, 5, 10, 13, 2, 3, 12, 11, 4, 6, 9, 14, 1 };

// RETAIL: 0x10294e40, 0x10294e50: the matches in each round
static const int s_aWinRoundMatches[BRACKET_WIN_ROUNDS] = { 8, 4, 2, 1 };
static const int s_aLoseRoundMatches[BRACKET_LOSE_ROUNDS] = { 4, 4, 2, 2, 1, 1 };

// RETAIL: 0x10294e68: the losers' match each winners' match's loser goes to
static const int s_aLoserMatch[BRACKET_TEAMS - 1] = { 0, 0, 1, 1, 2, 2, 3, 3, 7, 6, 5, 4, 10, 11, 13 };

TournamentMatch::TournamentMatch()
{
	nWinner = nTeam[0] = nTeam[1] = BRACKET_EMPTY;
}

CTournamentBracket::CTournamentBracket()
{
	Reset();
}

CTournamentBracket::~CTournamentBracket()
{
}

void CTournamentBracket::Reset()
{
	m_WinMatches.clear();
	m_LoseMatches.clear();
	m_Teams.clear();

	m_bOver = true;
	m_bDoubleElimination = true;
	m_nByeMatches = 0;

	int i;

	m_nNumWinMatches = 0;
	for (i = 0; i < BRACKET_WIN_ROUNDS; i++)
	{
		m_aWinRoundStart[i] = m_nNumWinMatches;
		m_nNumWinMatches += s_aWinRoundMatches[i];
	}

	m_WinMatches.reserve(m_nNumWinMatches);
	for (i = 0; i < m_nNumWinMatches; i++)
	{
		TournamentMatch match;
		match.nIndex = i;
		match.bWinners = true;
		m_WinMatches.push_back(match);
	}

	m_nNumLoseMatches = 0;
	for (i = 0; i < BRACKET_LOSE_ROUNDS; i++)
	{
		m_aLoseRoundStart[i] = m_nNumLoseMatches;
		m_nNumLoseMatches += s_aLoseRoundMatches[i];
	}

	m_LoseMatches.reserve(m_nNumLoseMatches);
	for (i = 0; i < m_nNumLoseMatches; i++)
	{
		TournamentMatch match;
		match.nIndex = i;
		match.bWinners = false;
		m_LoseMatches.push_back(match);
	}
}

void CTournamentBracket::AddTeam(int nTeam)
{
	TournamentTeam team;
	team.nIndex = m_Teams.size();
	team.nTeam = nTeam;
	m_Teams.push_back(team);
}

void CTournamentBracket::Start(bool bDoubleElimination)
{
	int nTeams = m_Teams.size();

	m_bDoubleElimination = bDoubleElimination;
	m_bOver = false;

	int aOrder[BRACKET_TEAMS];
	int i;

	for (i = 0; i < nTeams; i++)
	{
		aOrder[i] = i;
	}

	// Shuffle the teams...
	for (i = 0; i < nTeams; i++)
	{
		float fRand = (float)rand() / 32768.0f;
		uint32 nSwap = (uint32)(fRand * (float)(uint32)nTeams);
		if (nSwap >= (uint32)nTeams)
			nSwap = nTeams - 1;

		int nTemp = aOrder[i];
		aOrder[i] = aOrder[nSwap];
		aOrder[nSwap] = nTemp;
	}

	for (i = 0; i < nTeams; i++)
	{
		PlaceTeam(m_Teams[aOrder[i]].nIndex, s_aSeeds[i] / 2, true);
	}

	for (; i < BRACKET_TEAMS; i++)
	{
		PlaceTeam(BRACKET_BYE, s_aSeeds[i] / 2, true);
	}

	AdvanceByes(true);
}

int CTournamentBracket::GetWinnersChampion()
{
	return m_WinMatches[m_nNumWinMatches - 1].nWinner;
}

int CTournamentBracket::GetLosersChampion()
{
	return m_LoseMatches[m_nNumLoseMatches - 1].nWinner;
}

TournamentTeam* CTournamentBracket::GetTeam(int nIndex)
{
	return &m_Teams[nIndex];
}

void CTournamentBracket::SetTeam(int nIndex, const TournamentTeam& team)
{
	m_Teams[nIndex] = team;
}

bool CTournamentBracket::GetNextMatch(TournamentMatch& match)
{
	if (m_Ready.empty())
		return false;

	TournamentMatch next = m_Ready.front();
	m_Ready.pop();

	if (next.nTeam[0] == BRACKET_BYE || next.nTeam[0] == BRACKET_EMPTY ||
		next.nTeam[1] == BRACKET_BYE || next.nTeam[1] == BRACKET_EMPTY)
	{
		// A bye: only the winners' matches past the first byes count
		if (!next.bWinners || next.nIndex < m_nByeMatches)
			return GetNextMatch(match);
	}

	match = next;
	return true;
}

void CTournamentBracket::SetWinner(TournamentMatch* pMatch, int nWinner, bool bAdvanceByes)
{
	if (pMatch->bWinners)
	{
		m_WinMatches[pMatch->nIndex].nWinner = nWinner;

		// The loser drops to the losers' bracket
		int nLoserMatch = s_aLoserMatch[pMatch->nIndex];
		PlaceTeam(pMatch->nTeam[0] != nWinner ? pMatch->nTeam[0] : pMatch->nTeam[1], nLoserMatch, false);
	}
	else
	{
		m_LoseMatches[pMatch->nIndex].nWinner = nWinner;
	}

	int nNext = GetNextMatchIndex(pMatch->nIndex, pMatch->bWinners);
	if (nNext >= 0)
	{
		PlaceTeam(nWinner, nNext, pMatch->bWinners);
	}

	if (m_WinMatches[m_nNumWinMatches - 1].nWinner != BRACKET_EMPTY &&
		(!m_bDoubleElimination || m_LoseMatches[m_nNumLoseMatches - 1].nWinner != BRACKET_EMPTY))
	{
		m_bOver = true;
	}

	if (bAdvanceByes)
	{
		AdvanceByes(false);
	}
}

void CTournamentBracket::AdvanceByes(bool bFirst)
{
	int i;
	for (i = 0; i < m_nNumWinMatches; i++)
	{
		TournamentMatch* pMatch = m_WinMatches.begin() + i;
		if (pMatch->nWinner == BRACKET_EMPTY && pMatch->nTeam[0] != BRACKET_EMPTY && pMatch->nTeam[1] != BRACKET_EMPTY)
		{
			if (pMatch->nTeam[0] == BRACKET_BYE)
				SetWinner(pMatch, pMatch->nTeam[1], false);
			else if (pMatch->nTeam[1] == BRACKET_BYE)
				SetWinner(pMatch, pMatch->nTeam[0], false);
		}
	}

	if (bFirst)
	{
		int nRounds = GetRound(GetFirstRealMatch(), true);
		for (i = 0; i < nRounds; i++)
		{
			m_nByeMatches += s_aWinRoundMatches[i];
		}
	}

	if (m_bDoubleElimination)
	{
		for (i = 0; i < m_nNumLoseMatches; i++)
		{
			TournamentMatch* pMatch = m_LoseMatches.begin() + i;
			if (pMatch->nWinner == BRACKET_EMPTY && pMatch->nTeam[0] != BRACKET_EMPTY && pMatch->nTeam[1] != BRACKET_EMPTY)
			{
				if (pMatch->nTeam[0] == BRACKET_BYE)
					SetWinner(pMatch, pMatch->nTeam[1], false);
				else if (pMatch->nTeam[1] == BRACKET_BYE)
					SetWinner(pMatch, pMatch->nTeam[0], false);
			}
		}
	}
}

void CTournamentBracket::PlaceTeam(int nTeam, int nMatch, bool bWinners)
{
	if (!bWinners && !m_bDoubleElimination)
		return;

	TournamentMatchArray* pMatches = bWinners ? &m_WinMatches : &m_LoseMatches;

	if ((*pMatches)[nMatch].nTeam[0] == BRACKET_EMPTY)
	{
		(*pMatches)[nMatch].nTeam[0] = nTeam;
	}
	else if ((*pMatches)[nMatch].nTeam[1] == BRACKET_EMPTY)
	{
		(*pMatches)[nMatch].nTeam[1] = nTeam;
		if (nTeam != BRACKET_BYE || (*pMatches)[nMatch].nTeam[0] != BRACKET_BYE)
		{
			m_Ready.push((*pMatches)[nMatch]);
		}
	}
}

int CTournamentBracket::GetNextMatchIndex(int nIndex, bool bWinners)
{
	int nMatches = bWinners ? m_nNumWinMatches : m_nNumLoseMatches;
	if (nIndex >= nMatches - 1)
		return -1;

	const int* pRoundMatches = bWinners ? s_aWinRoundMatches : s_aLoseRoundMatches;
	const int* pRoundStart = bWinners ? m_aWinRoundStart : m_aLoseRoundStart;

	int nRound = GetRound(nIndex, bWinners);
	int nPerMatch = pRoundMatches[nRound] / pRoundMatches[nRound + 1];
	return (nIndex - pRoundStart[nRound]) / nPerMatch + pRoundStart[nRound + 1];
}

int CTournamentBracket::GetRound(int nIndex, bool bWinners)
{
	const int* pRoundMatches;
	int* pRoundStart;
	if (bWinners)
	{
		pRoundMatches = s_aWinRoundMatches;
		pRoundStart = m_aWinRoundStart;
	}
	else
	{
		pRoundMatches = s_aLoseRoundMatches;
		pRoundStart = m_aLoseRoundStart;
	}

	int nRounds = bWinners ? BRACKET_WIN_ROUNDS : BRACKET_LOSE_ROUNDS;
	int i;
	for (i = 0; i < nRounds; i++)
	{
		if (nIndex < pRoundStart[i] + pRoundMatches[i])
			break;
	}

	return i;
}

int CTournamentBracket::GetRoundsLeft(bool bWinners)
{
	if (!bWinners)
		return 0;

	return BRACKET_WIN_ROUNDS - GetRound(GetFirstRealMatch(), bWinners);
}
