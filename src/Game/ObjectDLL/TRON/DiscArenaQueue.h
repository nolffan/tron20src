// ----------------------------------------------------------------------- //
//
// MODULE  : DiscArenaQueue.h
//
// PURPOSE : The queue of teams that wait to fight in the disc arena
//
// RETAIL: Tron 2.0's, retail-only (no RTTI, no strings name it: the class,
// its members and its functions' names are ours). Written from retail
// Object.lto 0x1001a200..0x1001a640 (its own TU, after CoreDump): an
// 8-byte member (+0xdc) of the disc game types' manager
// (ServerDiscMatchMgr.h).
//
// ----------------------------------------------------------------------- //

#ifndef __DISCARENAQUEUE_H__
#define __DISCARENAQUEUE_H__

#include "ltbasetypes.h"
#include <list>

typedef std::list<int> DiscTeamList;

class CDiscArenaQueue
{
	public :

		CDiscArenaQueue();						// RETAIL: 0x1001a200

		void	Clear();						// RETAIL: 0x1001a2e0
		void	Add(int nTeam);					// RETAIL: 0x1001a320
		void	Remove(int nTeam);				// RETAIL: 0x1001a360

		// Takes the next two teams to fight (RETAIL: 0x1001a3a0)
		bool	GetNextPair(int* pTeamA, int* pTeamB, int nNumTeams);

		// Puts a match's teams back (RETAIL: 0x1001a5a0)
		void	Return(int nTeamA, int nTeamB);

	protected :

		DiscTeamList	m_Teams;		// +0x00
		bool			m_bFresh;		// +0x04 no pair taken since the queue had two teams
};

#endif // __DISCARENAQUEUE_H__
