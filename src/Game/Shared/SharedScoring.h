// ----------------------------------------------------------------------- //
//
// MODULE  : SharedScoring.h
//
// PURPOSE : SharedScoring - shared mission summary stuff
//
// CREATED : 10/17/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHARED_SCORE_H__
#define __SHARED_SCORE_H__

#include "ltbasetypes.h"
#include "ClientServerShared.h"

#include <vector>
#include <string>

// RETAIL: Tron's own score message (retail Object.lto 0x1013f030,
// 0x1013f5d0): a bool, true for a player's score, false for a team's. The
// name is ours
#define MID_TRON_SCORE				122

// RETAIL: Tron's message between the server and the host's client, and its
// sub-ids for the teams (Object.lto 0x1013fb00; CShell.dll 0x100fb954). The
// names are ours
#define MID_TRON_SERVERMESSAGE		200
#define TSM_TEAMS					80
#define TSM_MOVECLIENT				81

// RETAIL: Tron's CPlayerScore, not NOLF2's (Tron forked before NOLF2's was
// written): retail Object.lto 0x1013eed0 to 0x1013f230, 0x1c bytes, CPlayerObj's
// m_Score (+0x2b40 in retail; its constructor 0x10076f10, Save 0x100811d0
// and Load 0x10081570 call it). Seven 16-bit values and a bool the light
// cycle game type keeps, the client id, the score and the frags. Retail has
// no AddFrag/AddTag/AddBonus: Tron's game types add to the score themselves.
// Member names but m_nClientID are ours
class CPlayerScore
{
public:
	CPlayerScore();

	void	Init(uint32 nClientID);
	void	WriteData(ILTMessage_Write *pMsg);
	void	ReadData(ILTMessage_Read *pMsg);

#ifndef _CLIENTBUILD
	void	Reset(bool bResetScore);
	void	AddScore(int16 nScore);
	void	AddFrags(int16 nFrags);
	void	SetTronUnknown18(uint32 nTronUnknown);
	void	UpdateClients(HCLIENT hClients = LTNULL);
	void	WriteTronUnknownB(ILTMessage_Write *pMsg);

	// RETAIL: inline in retail 0x1013f030
	void	WriteTronUnknownA(ILTMessage_Write *pMsg)
	{
		if (!pMsg) return;

		pMsg->Writeint16(GetTronUnknown0());
		pMsg->Writeint16(GetTronUnknown4());
		pMsg->Writeint16(GetTronUnknown8());
		pMsg->Writebool(m_bTronUnknownE);
	}
#endif

	// RETAIL: the score is what GameSpy's "score" key and the server app
	// report (retail 0x1005d730, 0x100e3ab0); the frags are what retail
	// CDestructible::ProcessPlayerDeath (0x100e8e70) adds a kill to
	int16	GetScore() {return m_nScore;}
	int16	GetFrags() {return m_nFrags;}
	int16	GetTronUnknown0() {return m_nTronUnknown0;}
	int16	GetTronUnknown4() {return m_nTronUnknown4;}
	int16	GetTronUnknown6() {return m_nTronUnknown6;}	// RETAIL: the light cycle game type's server app figures (0x100e3b6a)
	int16	GetTronUnknown8() {return m_nTronUnknown8;}
	int16	GetTronUnknownC() {return m_nTronUnknownC;}	// RETAIL: the light cycle game type's server app figures (0x100e3b80)
	uint32	GetTronUnknown18() {return m_nTronUnknown18;}	// RETAIL: 0 or 3 for the players CTronTeamMgr::SendToTeam may send to (0x1013fc75)

private:
	int16	m_nTronUnknown0;	// +0x00
	int16	m_nTronUnknown2;	// +0x02
	int16	m_nTronUnknown4;	// +0x04
	int16	m_nTronUnknown6;	// +0x06
	int16	m_nTronUnknown8;	// +0x08
	int16	m_nTronUnknownA;	// +0x0a
	int16	m_nTronUnknownC;	// +0x0c
	bool	m_bTronUnknownE;	// +0x0e
	uint32	m_nClientID;		// +0x10
	int16	m_nScore;			// +0x14
	int16	m_nFrags;			// +0x16
	uint32	m_nTronUnknown18;	// +0x18 (2 at start; sent as 8 bits)
};

// RETAIL: a team's record, Tron's (no RTTI, no strings: the name and the
// member names are ours): retail Object.lto 0x1013f280 to 0x1013f750, 0x2c
// bytes. Its constructor builds 17 of them in CTronTeamMgr
class CTronTeam
{
public:
	// RETAIL: 1 when reset, 2 while a timer (m_fStateTime) runs (retail
	// 0x1004bfa0 sets 1 when it is up); the names are ours
	enum TronTeamState
	{
		kTronTeamState_TronUnknown0,
		kTronTeamState_Reset,
		kTronTeamState_Timed,
	};

	CTronTeam();

	void	Init(uint8 nID);
	void	Reset();

#ifndef _CLIENTBUILD
	void	WriteData(ILTMessage_Write *pMsg);
	void	ResetScores();
	void	SetState(TronTeamState eState);
	void	AddTronUnknownC(int16 nAmount);
	void	AddTronUnknownE(int16 nAmount);
	void	AddTronUnknown12(int16 nAmount);
	void	SetStateTime(LTFLOAT fTime);
	bool	IsStateTimeUp();
	std::vector<uint32>* GetTronUnknown14();
	bool	AddClient(uint32 nClientID);
	bool	RemoveClient(uint32 nClientID);
	void	SetMaxClients(uint8 nMaxClients, uint8 nTeamSize);
	void	UpdateClients(HCLIENT hClients = LTNULL);
	class CPlayerObj* GetPlayer(uint32 nIndex);
#else
	// RETAIL: the client's copy (CShell.dll 0x100fae60, 0x100fafc0)
	void	ReadData(ILTMessage_Read *pMsg);
	void	GetName(bool bPlayerName, std::string& sName, bool bTronUnknown);
#endif
	uint8	GetTronUnknownIndex();

	TronTeamState GetState() { return m_eState; }
	int		FindClient(uint32 nClientID);
	uint32	GetClient(uint32 nIndex);

	// RETAIL: inline accessors the client's HUDs use (the names are ours)
	uint8	GetID() { return m_nID; }
	uint8	GetMaxClients() { return m_nMaxClients; }
	uint8	GetTeamSize() { return m_nTeamSize; }
	int16	GetTronUnknownC() { return m_nTronUnknownC; }
	int16	GetTronUnknownE() { return m_nTronUnknownE; }
	int16	GetTronUnknown12() { return m_nTronUnknown12; }
	uint32	GetNumClients() { return m_Clients.size(); }

private:
	uint8	m_nID;				// +0x00 (0xff until Init)
	TronTeamState	m_eState;	// +0x04 (1 when reset)
	LTFLOAT	m_fStateTime;		// +0x08
	int16	m_nTronUnknownC;	// +0x0c
	int16	m_nTronUnknownE;	// +0x0e
	uint8	m_nMaxClients;		// +0x10
	uint8	m_nTeamSize;		// +0x11
	int16	m_nTronUnknown12;	// +0x12
	std::vector<uint32>	m_TronUnknown14;	// +0x14 (start points? retail CGameServerShell::FindStartPoint, 0x1004c020, reads it)
	std::vector<uint32>	m_Clients;		// +0x20 the team's client ids

	friend class CTronTeamMgr;
};

// RETAIL: Tron's team manager (the name is ours): retail Object.lto
// 0x1013f780 to 0x1013fcc0, 0x338 bytes, CTronGameServerShell's member at
// +0x1d0ec, which g_pTronTeamMgr (0x1035a7cc) points to. 16 teams and
// the unassigned (the 17th), an index map of the 16
#define TRON_MAX_TEAMS			16
#define TRON_UNASSIGNED_TEAM	16

class CTronTeamMgr
{
public:
	// RETAIL: both inline (retail 0x1005c2c0, 0x1005c9a0)
	CTronTeamMgr() { Init(); }
	~CTronTeamMgr() { Init(); }

	void	Init();
	CTronTeam* GetTeam(uint32 nTeam, bool bMapped);

	int		GetNumTeams() { return m_nNumTeams; }

	int		GetTeamOfClient(uint32 nClientID);

#ifdef _CLIENTBUILD
	// RETAIL: the client asks the server to move a client to a team
	// (CShell.dll 0x100fb900); the name is ours
	void	MoveClient(uint32 nClientID, uint32 nTeam);
	// RETAIL: CShell.dll 0x100fb330, 0x100fba70 (the names are ours)
	void	SortTeams();
	uint8	GetTeamColor(uint8 nIndex, bool bRandom);
#else
	bool	SetupTeams(uint32 nTeamSize, uint32 nMaxPlayers);
	bool	RemoveClient(uint32 nClientID);
	bool	MoveClient(uint32 nClientID, uint32 nTeam);
	void	ResetScores();
	void	UpdateClients(HCLIENT hClients = LTNULL);
	LTRESULT SendToTeam(ILTMessage_Read *pMsg, int nTeam, uint32 nFlags, bool bTronUnknown);
#endif

private:
	int			m_nNumTeams;					// +0x000
	CTronTeam	m_aTeams[TRON_MAX_TEAMS + 1];	// +0x004
	uint32		m_aTeamIndex[TRON_MAX_TEAMS];	// +0x2f0
	uint32		m_nTeamSize;					// +0x330 (-1 until set up)
#ifndef _CLIENTBUILD
	// RETAIL: the server's only: the client's copy is 0x334 bytes
	// (CTronInterfaceMgr +0xe408 to +0xe73c; its Init, 0x100fb2e0)
	uint32		m_nMaxPlayers;					// +0x334
#endif
};

extern CTronTeamMgr* g_pTronTeamMgr;

#endif // __SHARED_SCORE_H__