// ----------------------------------------------------------------------- //
//
// MODULE  : DiscArenaQueue.cpp
//
// PURPOSE : The queue of teams that wait to fight in the disc arena
//
// RETAIL: Tron 2.0's, written from retail Object.lto (DiscArenaQueue.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DiscArenaQueue.h"

CDiscArenaQueue::CDiscArenaQueue()
{
	Clear();
}

void CDiscArenaQueue::Clear()
{
	m_Teams.clear();
	m_bFresh = true;
}

void CDiscArenaQueue::Add(int nTeam)
{
	m_Teams.push_back(nTeam);
}

void CDiscArenaQueue::Remove(int nTeam)
{
	m_Teams.remove(nTeam);
}

bool CDiscArenaQueue::GetNextPair(int* pTeamA, int* pTeamB, int nNumTeams)
{
	int nSize = m_Teams.size();
	if (nSize < 2)
		return false;

	if (nSize == 2)
	{
		// The same two teams again only when there are no others...
		if (nNumTeams > 2 && !m_bFresh)
			return false;

		*pTeamA = m_Teams.front();
		m_Teams.erase(m_Teams.begin());
		*pTeamB = m_Teams.front();
		m_Teams.erase(m_Teams.begin());
		m_bFresh = true;
		return true;
	}

	*pTeamA = m_Teams.front();

	if (nSize == 3)
	{
		m_Teams.erase(m_Teams.begin());
		*pTeamB = m_Teams.back();
		m_Teams.pop_back();
		m_bFresh = true;
		return true;
	}

	m_Teams.erase(m_Teams.begin());

	uint32 nCount = m_Teams.size();
	uint32 nIndex = (uint32)((float)rand() / 32768.0f * nCount);
	if (nIndex >= nCount)
		nIndex = nCount - 1;

	if (!m_bFresh && nIndex == 0)
	{
		nIndex = 1;
		m_bFresh = true;
	}

	DiscTeamList::iterator iter = m_Teams.begin();
	std::advance(iter, nIndex);

	*pTeamB = *iter;
	m_Teams.erase(iter);
	return true;
}

void CDiscArenaQueue::Return(int nTeamA, int nTeamB)
{
	int nOldSize = m_Teams.size();

	if (nTeamA >= 0)
		m_Teams.push_back(nTeamA);

	if (nTeamB >= 0)
		m_Teams.push_back(nTeamB);

	if (m_Teams.size() == 2 && nOldSize == 0)
		m_bFresh = false;
}
