// ----------------------------------------------------------------------- //
//
// MODULE  : DiscMatchMgr.h
//
// PURPOSE : The client's disc game (arena and tournament) match manager
//
// RETAIL: Tron 2.0's, retail-only (no RTTI, no strings name it: the class,
// its members and its functions' names are ours). Written from retail
// CShell.dll 0x1000dd50..0x10010650: a 0x10-byte member of
// CTronGameClientShell (+0x3695c) that g_pDiscMatchMgr (0x10236d70) points
// to while initialized. CTronPlayerMgr hands it Tron's message 200 in the
// disc game types (0x100a6e50).
//
// ----------------------------------------------------------------------- //

#ifndef __DISCMATCHMGR_H__
#define __DISCMATCHMGR_H__

#include "ltbasedefs.h"
#include <list>

// RETAIL: a match between two teams (0x18 bytes, the nodes of a std::list:
// 0x1000e390 builds them). The names are ours
struct DiscMatch
{
	uint8	nTeam[2];			// +0x00 the local team second (when in it)
	uint8	nOrder;				// +0x02 the sort key (0x100104b0)
	uint8	nWins[2];			// +0x03 per team (0x1000eb00)
	uint8	nTronUnknown5;		// +0x05
	uint32	nTronUnknown8;		// +0x08
	int		nTronUnknownC;		// +0x0c (signed 8 bits on the wire)
	int		nTronUnknown10;		// +0x10 (signed 8 bits on the wire)
	float	fTronUnknown14;		// +0x14 a time (now + the message's)

	bool operator<(const DiscMatch& other) const { return nOrder < other.nOrder; }
};

typedef std::list<DiscMatch> DiscMatchList;

class CDiscMatchMgr
{
	public:

		CDiscMatchMgr();							// RETAIL: 0x1000ddc0
		// RETAIL: inline (CTronGameClientShell's dtor, 0x10099e54)
		~CDiscMatchMgr() { Term(); }

		bool	Init();								// RETAIL: 0x1000de90
		void	Term();								// RETAIL: 0x1000deb0
		void	Reset();							// RETAIL: 0x1000dec0

		bool	OnMessage(ILTMessage_Read *pMsg);	// RETAIL: 0x1000df20

		int			GetNumMatches();				// RETAIL: 0x1000fb50
		int			GetLocalMatchIndex();			// RETAIL: 0x1000fb70
		DiscMatch*	GetMatch(uint32 nIndex);			// RETAIL: 0x1000fbc0
		int			FindMatch(uint8 nTeam, DiscMatchList::iterator& iter);	// RETAIL: 0x1000fbf0
		bool		IsLocalInMatch(uint8 nTeam);	// RETAIL: 0x1000fc40

		int		GetState() const { return m_nState; }
		float	GetTronUnknownC() const { return m_fTronUnknownC; }

	protected:

		// RETAIL: the sub-messages, in retail's order (the ids are Tron's)
		void	HandleMsgSettings(ILTMessage_Read *pMsg);		// 0, 0x1000e190
		void	HandleMsgTeams(ILTMessage_Read *pMsg);			// 80, 0x1000e2c0
		void	HandleMsgTimer(ILTMessage_Read *pMsg);			// 8, 0x1000e310
		void	HandleMsgAddMatch(ILTMessage_Read *pMsg);		// 1, 0x1000e390
		void	HandleMsgMatchOver(ILTMessage_Read *pMsg);		// 2, 0x1000e880
		void	HandleMsgRoundOver(ILTMessage_Read *pMsg);		// 3, 0x1000eac0
		void	HandleMsgMatchScore(ILTMessage_Read *pMsg);		// 4, 0x1000eb00
		void	HandleMsgCountdown(ILTMessage_Read *pMsg);		// 6, 0x1000ecf0
		void	HandleMsgWaiting(ILTMessage_Read *pMsg);		// 7, 0x1000ed60
		void	HandleMsgGameOver(ILTMessage_Read *pMsg);		// 5, 0x1000ee30
		void	HandleMsgTeamChange(ILTMessage_Read *pMsg);		// 81, 0x1000f330
		void	HandleMsgTeamMessage(ILTMessage_Read *pMsg);	// 9, 0x1000f560
		void	HandleMsgTronUnknown10(ILTMessage_Read *pMsg);	// 10, 0x1000f860
		void	HandleMsgTronUnknown11(ILTMessage_Read *pMsg);	// 11, 0x1000f880
		void	HandleMsgTronUnknown101(ILTMessage_Read *pMsg);	// 101, 0x1000f8a0
		void	HandleMsgTronUnknown83(ILTMessage_Read *pMsg);	// 83, 0x1000f8c0

		// RETAIL: 0x1000fcf0: a team's voice line ("voice\<nBase + team>.wav")
		// when the teams have more than one player
		const char*	GetTeamSound(int nBase, int nTeam);

		DiscMatchList	m_Matches;			// +0x00
		int				m_nState;			// +0x04 0 none, 1 playing, 2 round over
		uint32			m_nTronUnknown8;	// +0x08 (-1 when reset)
		float			m_fTronUnknownC;	// +0x0c a time (-1 when none)
};

extern CDiscMatchMgr* g_pDiscMatchMgr;		// RETAIL: 0x10236d70

#endif // __DISCMATCHMGR_H__
