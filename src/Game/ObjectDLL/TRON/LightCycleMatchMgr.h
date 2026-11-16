// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMatchMgr.h
//
// PURPOSE : The server's multiplayer light cycle game (races and matches)
//
// RETAIL: Tron 2.0's, retail-only (no RTTI, no strings name it: the class,
// its members and its functions' names are ours; the client's end of the
// same messages is ClientShellDLL/TRON/LightCycleMatchMgr, hence the name).
// Written from retail Object.lto 0x10053b80..0x10057220: 0xb4 bytes that
// CTronGameServerShell news at +0x19bc0 in the light cycle game type
// (OnServerInitialized, 0x1005cba8) and that g_pLightCycleMatchMgr
// (0x1035a574) points to while initialized. Its messages are Tron's message
// 200 (MID_TRON_SERVERMESSAGE) with the sub-ids below.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEMATCHMGR_H__
#define __LIGHTCYCLEMATCHMGR_H__

#include "ltbasedefs.h"
#include "Timer.h"
#include <list>

class CPlayerObj;
class CTronPlayerObj;

// RETAIL: the sub-ids of message 200 the light cycle game sends and reads
// (the client's handlers, CShell.dll 0x100161e0; the names are ours)
#define LCMM_MSG_SETTINGS			20
#define LCMM_MSG_RACESTART			21
#define LCMM_MSG_WAITING			22
#define LCMM_MSG_RESPAWNED			23
#define LCMM_MSG_RACEOVER			24
#define LCMM_MSG_GAMEOVER			25
#define LCMM_MSG_GAMEINPROGRESS		30
#define LCMM_MSG_READY				32	// client to server
#define LCMM_MSG_SPECTATE			33
#define LCMM_MSG_RESET				35
#define LCMM_MSG_ROUNDOVER			36
#define LCMM_MSG_COUNTDOWN			37
#define LCMM_MSG_LASTRACE			38
#define LCMM_MSG_POINTSNEEDED		39
#define LCMM_MSG_MOVEFAILED			82
#define LCMM_MSG_PINGTOOHIGH		83

// RETAIL: eight light cycles race at once (the array at +0x00)
#define LCMM_MAX_RACERS				8

class CLightCycleMatchMgr
{
	public:

		// RETAIL: the states (+0x30; the client's mirror them; the names are ours)
		enum MatchState
		{
			kState_None,
			kState_Waiting,
			kState_Racing,
			kState_RaceOver,
			kState_GameOver,
		};

		typedef std::list<CPlayerObj*> PlayerList;

		CLightCycleMatchMgr();						// RETAIL: 0x10053b80
		// RETAIL: inline (CTronGameServerShell's OnServerInitialized and
		// destructor call it out of line, 0x1005ca00)
		~CLightCycleMatchMgr() { Term(); }

		bool	Init();								// RETAIL: 0x10053d10
		void	Term();								// RETAIL: 0x10053e30

		void	Update();							// RETAIL: 0x10054060

		// RETAIL: a new client starts without a team (the server shell,
		// 0x1005defa; folded with the disc game's, 0x1004b070)
		void	AddClient(HCLIENT hClient);

		// RETAIL: a client's player came in (0x10054860, from the server
		// shell 0x1005e02a) or left (0x10054a30, 0x1005e07a)
		void	AddPlayer(CPlayerObj* pPlayer);
		void	PlacePlayer(CPlayerObj* pPlayer);	// RETAIL: 0x10054890
		void	RemovePlayer(CPlayerObj* pPlayer);

		// RETAIL: 0x10054b50: the light cycle manager derezzed a cyclist
		// (CServerLightCycleMgr::DerezCyclist, HeadOnCollision with
		// bHeadOn: both cyclists die)
		void	HandleDerez(CPlayerObj* pPlayer, CPlayerObj* pKiller, bool bHeadOn);

		// RETAIL: 0x10055310: message 200 from a client (the server shell,
		// 0x1005ed94)
		bool	OnMessage(ILTMessage_Read *pMsg, HCLIENT hSender);

		// RETAIL: 0x100565b0: a client is gone (the server shell, 0x1005df5b)
		void	RemoveClient(HCLIENT hClient);

		// RETAIL: 0x100566d0: a new world started (the server shell's
		// PostStartWorld, 0x1005cf3f)
		void	OnStartWorld();
		// RETAIL: empty (retail 0x1026abb0, folded with other empty
		// one-argument functions; CTronGameServerShell 0x1005dea9). The name is ours
		void	OnPlayerRespawn(CPlayerObj* pPlayer);

		// RETAIL: 0x10056700: end the game to switch to a level (the server
		// shell, 0x1005e2c9)
		void	SetNextLevel(int nLevel);

		// RETAIL: 0x10056720: all a client needs to join (the server shell)
		void	SendState(HCLIENT hClient);

		// RETAIL: inline: the power-ups the game allows (LightCyclePowerup,
		// 0x1002b0c1, 0x1002bab1)
		uint32	GetAllowedPowerups() const { return m_nPowerups; }

	protected:

		void	StartRace();						// RETAIL: 0x10053e50
		void	EndRace(int8 nWinner);				// RETAIL: 0x10055280
		void	AddSpectator(CPlayerObj* pPlayer);	// RETAIL: 0x100554b0
		void	AddWaiting(CPlayerObj* pPlayer);	// RETAIL: 0x10055620
		bool	AddRacer(CPlayerObj* pPlayer, bool bRespawn);	// RETAIL: 0x10055780
		void	StartWaiting();						// RETAIL: 0x10055940
		void	SendWaiting(HCLIENT hClient);		// RETAIL: 0x10055990
		void	SendRaceOver(HCLIENT hClient);		// RETAIL: 0x10055bc0
		void	SendGameOver(HCLIENT hClient);		// RETAIL: 0x10055dd0
		void	RemoveSpectator(CPlayerObj* pPlayer, bool bNotify);	// RETAIL: 0x10055ff0
		void	UpdateScores(HCLIENT hClients);		// RETAIL: 0x100561c0
		void	PrepareRace();						// RETAIL: 0x10056200
		void	HandleTeamChange(ILTMessage_Read *pMsg);	// RETAIL: 0x10056640
		void	SendSettings(HCLIENT hClient);		// RETAIL: 0x100568c0
		void	EndMatch(uint8 nReason);			// RETAIL: 0x10056aa0
		void	Reset(bool bClearSpectators);		// RETAIL: 0x10056b80
		bool	CheckPing(CPlayerObj* pPlayer, uint16& nPing);	// RETAIL: 0x10056d40
		bool	MovePlayer(CPlayerObj* pPlayer, uint8 nTeam);	// RETAIL: 0x10056d90

		CTronPlayerObj*	m_apRacers[LCMM_MAX_RACERS];	// +0x00
		PlayerList	m_lstSpectators;		// +0x20
		PlayerList	m_lstReady;				// +0x24 the racers that sent LCMM_MSG_READY
		PlayerList	m_lstWaiting;			// +0x28 for the next race
		PlayerList	m_lstLagged;			// +0x2c (Update's, kicked)
		int			m_eState;				// +0x30 a MatchState
		float		m_fWaitTime;			// +0x34 "WaitForPlayersTime"
		uint8		m_nRace;				// +0x38
		CTimer		m_Timer;				// +0x3c
		float		m_fRaceStartDelay;		// +0x4c
		float		m_fClockStartTime;		// +0x50
		float		m_fRaceOverTime;		// +0x54
		float		m_fGameOverTime;		// +0x58
		float		m_fLastSendTime;		// +0x5c
		int			m_nKillPoints;			// +0x60
		int			m_nSelfKillPoints;		// +0x64
		int			m_nWinRacePoints;		// +0x68
		uint8		m_nNumRacers;			// +0x6c
		uint32		m_nAllowedCycleType;	// +0x70
		uint32		m_nGameSpeed;			// +0x74
		uint32		m_nTrailLength;			// +0x78
		uint8		m_nTotalRaces;			// +0x7c
		uint8		m_nMaxTeamSize;			// +0x7d
		uint16		m_nTotalPoints;			// +0x7e
		float		m_fTimeLimit;			// +0x80 seconds
		uint16		m_nMaxPing;				// +0x84
		uint32		m_nPowerups;			// +0x88 the server options' m_nLCPowerups
		uint8		m_nGameOverReason;		// +0x8c 0 races, 1 points, 2 time
		int8		m_nWinner;				// +0x8d the winning team (-1 none)
		uint8		m_nNumTied;				// +0x8e
		bool		m_bClockRunning;		// +0x8f
		uint8		m_nCountdown;			// +0x90 the next time warning (30, 15, 5 seconds)
		float		m_fPausedTime;			// +0x94 the clock's pauses
		float		m_fPauseStart;			// +0x98
		float		m_fLagTimeout;			// +0x9c
		uint8		m_nLiving;				// +0xa0
		float		m_fTronUnknownA4;		// +0xa4 (sent with the settings)
		float		m_fLastPingCheck;		// +0xa8
		uint8		m_nTronUnknownAC;		// +0xac the racers of the last map
		int			m_nNextLevel;			// +0xb0 (-1 none)
};

extern CLightCycleMatchMgr* g_pLightCycleMatchMgr;	// RETAIL: 0x1035a574

#endif // __LIGHTCYCLEMATCHMGR_H__
