// ----------------------------------------------------------------------- //
//
// MODULE  : ServerDiscMatchMgr.h
//
// PURPOSE : The server's disc game (arena and tournament) match manager
//
// RETAIL: Tron 2.0's, retail-only (no RTTI, no strings name it: the
// classes, their members and their functions' names are ours). Written
// from retail Object.lto 0x100493f0..0x1004f3a0 (one TU): a 0xe4-byte
// manager that CTronGameServerShell keeps at +0x19bbc in the disc game
// types, a 0x60-byte match and a 0x80-byte arena. Its sub-messages of
// MID_TRON_SERVERMESSAGE are the ones that the client's CDiscMatchMgr
// (CShell.dll 0x1000df20) reads.
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERDISCMATCHMGR_H__
#define __SERVERDISCMATCHMGR_H__

#include "ltbasedefs.h"
#include "TournamentBracket.h"
#include "DiscArenaQueue.h"
#include <list>
#include <vector>
#include <string>

class CPlayerObj;
class ArenaStartPoint;
class GameBase;

// RETAIL: the sub-message ids (the client's CDiscMatchMgr::OnMessage).
// The names are ours
enum DiscMsgID
{
	kDiscMsg_Settings		= 0,
	kDiscMsg_AddMatch		= 1,
	kDiscMsg_MatchOver		= 2,
	kDiscMsg_RoundStart		= 3,
	kDiscMsg_MatchScore		= 4,
	kDiscMsg_GameOver		= 5,
	kDiscMsg_Countdown		= 6,
	kDiscMsg_Waiting		= 7,
	kDiscMsg_Timer			= 8,
	kDiscMsg_TeamMessage	= 9,
	kDiscMsg_TeamWin		= 10,
	kDiscMsg_Spectate		= 11,
	kDiscMsg_Teams			= 80,
	kDiscMsg_TeamChange		= 81,
	kDiscMsg_JoinWait		= 83,
	kDiscMsg_TronUnknown101	= 101,
};

typedef std::vector<ArenaStartPoint*> ArenaStartPointArray;

// RETAIL: one of the four arenas (0x80 bytes)
class CDiscArena
{
	public :

		CDiscArena(int nIndex);											// RETAIL: 0x1004d9f0

		ArenaStartPoint*	GetSpectatorPoint(CPlayerObj* pPlayer);		// RETAIL: 0x1004de70
		ArenaStartPoint*	GetTeamPoint(uint32 nSlot, CPlayerObj* pPlayer);	// RETAIL: 0x1004df00
		void				SendCommand(const char* pszCmd);			// RETAIL: 0x1004dfb0

		int					m_nIndex;			// +0x00
		bool				m_bInUse;			// +0x04
		uint32				m_nSpectatorPoint;	// +0x08 the last one used
		uint32				m_nTeamPoint;		// +0x0c the last one used
		GameBase*			m_pArena;			// +0x10 the level's "Arena" object
		// RETAIL: the Arena object's six commands (its HSTRINGs +0x40..+0x54)
		std::string			m_sRoundStartCmd;	// +0x14 a player respawns for a round
		std::string			m_sKillCmd;			// +0x20 a player is killed
		std::string			m_sMatchStartCmd;	// +0x2c
		std::string			m_sMatchEndCmd;		// +0x38
		std::string			m_sFightCmd;		// +0x44 the countdown is over
		std::string			m_sRoundEndCmd;		// +0x50
		ArenaStartPointArray	m_SpectatorPoints;	// +0x5c type 0
		ArenaStartPointArray	m_aTeamPoints[2];	// +0x68 types 1 and 2
};

// RETAIL: a match between two teams in an arena (0x60 bytes)
class CDiscMatch
{
	public :

		CDiscMatch();												// RETAIL: 0x1004c8b0

		void	Update();											// RETAIL: 0x1004c8e0
		void	Init(int nTeamA, int nTeamB, CDiscArena* pArena, float fDuration,
					int nRounds, const TournamentMatch& match, int nRoundA, int nRoundB);	// RETAIL: 0x1004cc60
		void	Init(int nTeamA, int nTeamB, CDiscArena* pArena, float fDuration,
					int nRounds, bool bLongDelay);					// RETAIL: 0x1004ccb0
		void	Term();												// RETAIL: 0x1004cdd0
		void	TimeOut();											// RETAIL: 0x1004cec0
		void	StartRound(float fDelay);							// RETAIL: 0x1004ced0
		void	EndRound(int nWinner);								// RETAIL: 0x1004d0e0
		int		RemovePlayer(CPlayerObj* pPlayer);					// RETAIL: 0x1004d3d0
		void	OnPlayerKilled(CPlayerObj* pPlayer, CPlayerObj* pKiller);	// RETAIL: 0x1004d510
		void	SendState(HCLIENT hClient, bool bTronUnknown, bool bFirst);	// RETAIL: 0x1004d630
		bool	HasPlayer(CPlayerObj* pPlayer);						// RETAIL: 0x1004d820
		void	DecideWinner();										// RETAIL: 0x1004d860
		int		GetWinner();										// RETAIL: 0x1004d990
		int		GetLoser();											// RETAIL: 0x1004d9a0
		bool	IsOnTeam(int nTeam, CPlayerObj* pPlayer);			// RETAIL: 0x1004d9b0

		int				m_eState;			// +0x00 0 none, 1 countdown, 2 fight, 3 round over, 4 timed out, 5 over
		CDiscArena*		m_pArena;			// +0x04
		float			m_fDuration;		// +0x08
		float			m_fEndTime;			// +0x0c
		int				m_nWinsNeeded;		// +0x10
		int				m_nRounds;			// +0x14
		int				m_nTeam[2];			// +0x18
		uint32			m_nWins[2];			// +0x20
		uint32			m_nKills[2];		// +0x28 the deaths of each team this round
		int				m_nWinner;			// +0x30
		TournamentMatch	m_Match;			// +0x34
		int				m_nRoundA;			// +0x48
		int				m_nRoundB;			// +0x4c
		float			m_fRoundStartTime;	// +0x50
		float			m_fRoundOverTime;	// +0x54
		float			m_fMatchOverTime;	// +0x58
		float			m_fNextTick;		// +0x5c
};

typedef std::list<CDiscMatch*>		DiscMatchList;
typedef std::vector<CDiscArena*>	DiscArenaArray;
typedef std::list<CPlayerObj*>		DiscPlayerList;

class CDiscMatchMgr
{
	public :

		CDiscMatchMgr();									// RETAIL: 0x100493f0

		bool	Init();										// RETAIL: 0x100494c0
		void	Term();										// RETAIL: 0x10049750
		void	Reset(int nMaxTeamSize);					// RETAIL: 0x100497a0
		void	SetNextLevel(int nLevel);					// RETAIL: 0x100499c0
		void	StartTournament();							// RETAIL: 0x100499f0
		void	StartGame();								// RETAIL: 0x10049b30
		void	ResetTeams();								// RETAIL: 0x10049ca0
		void	EndGame();									// RETAIL: 0x10049ec0
		void	Update();									// RETAIL: 0x1004a020
		void	UpdateTournament();							// RETAIL: 0x1004a3d0
		void	UpdateMatches();							// RETAIL: 0x1004aa20
		bool	IsTimeUp();									// RETAIL: 0x1004ae90
		void	UpdateWarnings();							// RETAIL: 0x1004af10
		void	MakeSpectator(HCLIENT hClient);				// RETAIL: 0x1004b070
		void	OnPlayerAdded(CPlayerObj* pPlayer);			// RETAIL: 0x1004b0a0
		void	OnClientRemoved(HCLIENT hClient);			// RETAIL: 0x1004b0d0
		void	OnPlayerRemoved(CPlayerObj* pPlayer);		// RETAIL: 0x1004b160
		void	OnPlayerKilled(CPlayerObj* pPlayer, CPlayerObj* pKiller);	// RETAIL: 0x1004b1f0
		void	OnPlayerRespawn(CPlayerObj* pPlayer);		// RETAIL: 0x1004b230
		void	SendState(HCLIENT hClient);					// RETAIL: 0x1004b280
		void	SendSettings(HCLIENT hClient);				// RETAIL: 0x1004b3a0
		bool	OnMessage(ILTMessage_Read* pMsg, HCLIENT hSender);	// RETAIL: 0x1004b520
		void	HandleTeamChange(ILTMessage_Read* pMsg);	// RETAIL: 0x1004b560
		void	ChangeTeam(uint32 nClientID, uint8 nTeam);	// RETAIL: 0x1004b700
		void	GameOver();									// RETAIL: 0x1004b9e0
		void	AwardWin(int nTeam);						// RETAIL: 0x1004be60
		int		GetNumActiveTeams();						// RETAIL: 0x1004bf60
		void	UpdateTeamStates();							// RETAIL: 0x1004bfa0
		int		GetNumFreeArenas();							// RETAIL: 0x1004bff0
		CDiscArena*	FindArena(int nTeamA, int nTeamB);		// RETAIL: 0x1004c020
		void	RemoveWaitingTeam(int nTeam);				// RETAIL: 0x1004c520
		void	AssignWaitingPlayers();						// RETAIL: 0x1004c5c0
		void	SpawnPlayer(CPlayerObj* pPlayer, int nArena);	// RETAIL: 0x1004c670
		void	SpawnTeam(int nTeam, int nArena);			// RETAIL: 0x1004c7d0
		bool	CanJoin(CPlayerObj* pPlayer, uint16* pnTime);	// RETAIL: 0x1004c830

	protected :

		DiscMatchList	m_Matches;			// +0x00
		DiscArenaArray	m_Arenas;			// +0x04
		DiscPlayerList	m_Waiting;			// +0x10 players without a match
		bool			m_bStarted;			// +0x14
		bool			m_bLevelOver;		// +0x15 the level ends at m_fStartTime
		bool			m_bLevelOverWarning;	// +0x16
		float			m_fStartTime;		// +0x18
		float			m_fNextTick;		// +0x1c
		int				m_nGamesPlayed;		// +0x20 on this level
		float			m_fLastPingCheck;	// +0x24
		int				m_nTronUnknown28;	// +0x28 -1 while the game goes on
		float			m_fEndTime;			// +0x2c
		float			m_fWarningTime;		// +0x30 -1 when no warning is left
		int				m_nWarning;			// +0x34
		uint8			m_nGameType;		// +0x38
		uint8			m_nTronUnknown39;	// +0x39
		uint8			m_nTronUnknown3A;	// +0x3a
		uint8			m_nRounds;			// +0x3b
		float			m_fMatchTime;		// +0x3c
		float			m_fPreGameDelay;	// +0x40
		uint8			m_nGamesPerMap;		// +0x44
		uint8			m_nTronUnknown45;	// +0x45 the air control
		uint8			m_nTronUnknown46;	// +0x46 discs in spectator
		int				m_nNumMaps;			// +0x48
		uint16			m_nJoinTime;		// +0x4c the most ping a player may join with
		uint8			m_nWinsToEnd;		// +0x4e
		float			m_fTimeLimit;		// +0x50
		bool			m_bDoubleElimination;	// +0x54
		uint8			m_nTeamsToStart;	// +0x55
		CTournamentBracket	m_Bracket;		// +0x58
		CDiscArenaQueue	m_Queue;			// +0xdc
};

#endif // __SERVERDISCMATCHMGR_H__
