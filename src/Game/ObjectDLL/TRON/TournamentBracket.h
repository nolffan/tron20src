// ----------------------------------------------------------------------- //
//
// MODULE  : TournamentBracket.h
//
// PURPOSE : The 16-team single or double elimination bracket of the disc
//			 game types' tournament
//
// RETAIL: Tron 2.0's, retail-only (no RTTI, no strings name it: the class,
// its members and its functions' names are ours). Written from retail
// Object.lto 0x1005a2d0..0x1005bd50 (its own TU, between Thorne and
// TronGameServerShell): a 0x84-byte member (+0x58) of the disc game types'
// manager (ServerDiscMatchMgr.h).
//
// ----------------------------------------------------------------------- //

#ifndef __TOURNAMENTBRACKET_H__
#define __TOURNAMENTBRACKET_H__

#include "ltbasetypes.h"
#include <vector>
#include <deque>
#include <queue>

// RETAIL: the slot values of a match (retail compares with -1 and -2)
#define BRACKET_BYE			-1		// no team: the other team goes on
#define BRACKET_EMPTY		-2		// not yet known

#define BRACKET_TEAMS		16
#define BRACKET_WIN_ROUNDS	4
#define BRACKET_LOSE_ROUNDS	6

// RETAIL: one match of the bracket (0x14 bytes)
struct TournamentMatch
{
	TournamentMatch();						// RETAIL: 0x1005a2d0

	int		nTeam[2];		// +0x00 the bracket's team indices
	bool	bWinners;		// +0x08 in the winners' bracket
	int		nIndex;			// +0x0c in its bracket
	int		nWinner;		// +0x10
};

// RETAIL: a team of the bracket (8 bytes)
struct TournamentTeam
{
	int		nIndex;			// +0x00 its index in the bracket's teams
	int		nTeam;			// +0x04 the team's id
};

typedef std::vector<TournamentMatch>	TournamentMatchArray;
typedef std::queue<TournamentMatch>		TournamentMatchQueue;
typedef std::vector<TournamentTeam>		TournamentTeamArray;

class CTournamentBracket
{
	public :

		CTournamentBracket();				// RETAIL: 0x1005a2e0
		~CTournamentBracket();			// RETAIL: 0x1005a470

		void	Reset();					// RETAIL: 0x1005a5d0
		void	AddTeam(int nTeam);			// RETAIL: 0x1005a800
		void	Start(bool bDoubleElimination);	// RETAIL: 0x1005a860

		int		GetWinnersChampion();		// RETAIL: 0x1005a960
		int		GetLosersChampion();		// RETAIL: 0x1005a970
		TournamentTeam*	GetTeam(int nIndex);	// RETAIL: 0x1005a980
		void	SetTeam(int nIndex, const TournamentTeam& team);	// RETAIL: 0x1005a990

		bool	GetNextMatch(TournamentMatch& match);	// RETAIL: 0x1005a9b0
		void	SetWinner(TournamentMatch* pMatch, int nWinner, bool bAdvanceByes);	// RETAIL: 0x1005aa80

		int		GetRoundsLeft(bool bWinners);	// RETAIL: 0x1005ad80

		bool	IsDoubleElimination() const { return m_bDoubleElimination; }
		bool	IsOver() const { return m_bOver; }
		int		GetNumTeams() const { return m_Teams.size(); }

	protected :

		void	AdvanceByes(bool bFirst);	// RETAIL: 0x1005ab30
		void	PlaceTeam(int nTeam, int nMatch, bool bWinners);	// RETAIL: 0x1005ac30
		int		GetNextMatchIndex(int nIndex, bool bWinners);	// RETAIL: 0x1005acc0
		int		GetRound(int nIndex, bool bWinners);	// RETAIL: 0x1005ad20

		// The first winners' match that is not a bye (inline in retail)
		int		GetFirstRealMatch()
		{
			int i;
			for (i = 0; i < m_nNumWinMatches; i++)
			{
				if (m_WinMatches[i].nTeam[0] != BRACKET_BYE && m_WinMatches[i].nTeam[1] != BRACKET_BYE)
					break;
			}
			return i;
		}

		bool					m_bDoubleElimination;	// +0x00
		bool					m_bOver;				// +0x01
		TournamentMatchArray	m_WinMatches;			// +0x04
		TournamentMatchArray	m_LoseMatches;			// +0x10
		TournamentMatchQueue	m_Ready;				// +0x1c the matches to play
		int						m_aWinRoundStart[BRACKET_WIN_ROUNDS];	// +0x44
		int						m_aLoseRoundStart[BRACKET_LOSE_ROUNDS];	// +0x54
		int						m_nNumWinMatches;		// +0x6c
		int						m_nNumLoseMatches;		// +0x70
		int						m_nByeMatches;			// +0x74 the first matches all byes passed
		TournamentTeamArray		m_Teams;				// +0x78
};

#endif // __TOURNAMENTBRACKET_H__
