// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMatchMgr.h
//
// PURPOSE : The client's light cycle game (rounds and matches) manager
//
// RETAIL: Tron 2.0's, retail-only (no RTTI, no strings name it: the class,
// its members and its functions' names are ours). Written from retail
// CShell.dll 0x10016130..0x10017670: a 0x20-byte member of
// CTronGameClientShell (+0x3696c) that g_pLightCycleMatchMgr (0x10236df0)
// points to while initialized. CTronPlayerMgr hands it Tron's message 200
// in the light cycle game type (0x100a6e50).
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEMATCHMGR_H__
#define __LIGHTCYCLEMATCHMGR_H__

#include "ltbasedefs.h"

class CLightCycleMatchMgr
{
	public:

		CLightCycleMatchMgr();						// RETAIL: 0x100c42f0 (empty, folded)
		// RETAIL: inline (CTronGameClientShell's dtor, 0x10099e3e)
		~CLightCycleMatchMgr() { Term(); }

		bool	Init();								// RETAIL: 0x100161a0
		void	Term();								// RETAIL: 0x100161b0
		void	Reset();							// RETAIL: 0x100161c0

		bool	OnMessage(ILTMessage_Read *pMsg);	// RETAIL: 0x100161e0

		void	Update();							// RETAIL: 0x10016aa0

		// RETAIL: 0x10016ad0, empty (folded): the client light cycle
		// manager tells it a cyclist was derezzed (0x10011367)
		void	OnCyclistDerezzed(HOBJECT hObj, HOBJECT hOther);

		// RETAIL: 0x10016ae0: space bar while not playing: spectate
		bool	OnKeyDown(int nKey, int nRep);

		// RETAIL: inline accessors the HUDs read (CHUDScores reads +0xc, its
		// header part 0x10037d80 reads +0x18, +0x19, +0x1a; the names are ours)
		int		GetState() const { return m_nState; }
		uint8	GetRound() const { return m_nRound; }
		float	GetTimeLeft() const { return m_fTimeLeft; }
		uint8	GetTronUnknown18() const { return m_nTronUnknown18; }
		uint8	GetTronUnknown19() const { return m_nTronUnknown19; }
		uint16	GetTronUnknown1A() const { return m_nTronUnknown1A; }
		uint8	GetTronUnknown1C() const { return m_nTronUnknown1C; }
		uint8	GetTronUnknown1D() const { return m_nTronUnknown1D; }
		uint8	GetTronUnknown1E() const { return m_nTronUnknown1E; }
		bool	GetTronUnknown1F() const { return m_bTronUnknown1F; }

	protected:

		void	SendTronUnknown32();				// RETAIL: 0x10016ba0

		// RETAIL: the sub-messages, in retail's order (the ids are Tron's;
		// message 80 is the disc match manager's code, folded)
		void	HandleMsgSettings(ILTMessage_Read *pMsg);		// 20, 0x100163a0
		void	HandleMsgRoundStart(ILTMessage_Read *pMsg);		// 21, 0x10016440
		void	HandleMsgRound(ILTMessage_Read *pMsg);			// 22, 0x100164c0
		void	HandleMsgRoundOver(ILTMessage_Read *pMsg);		// 24, 0x100165b0
		void	HandleMsgGameOver(ILTMessage_Read *pMsg);		// 25, 0x100167f0
		void	HandleMsgTronUnknown30(ILTMessage_Read *pMsg);	// 30, 0x10016a80
		void	HandleMsgSpectate(ILTMessage_Read *pMsg);		// 33, 0x10016cd0
		void	HandleMsgTronUnknown83(ILTMessage_Read *pMsg);	// 83, 0x10016d30
		void	HandleMsgTeams(ILTMessage_Read *pMsg);			// 80, 0x1000e2c0 (folded)
		void	HandleMsgTeamChange(ILTMessage_Read *pMsg);		// 81, 0x10016ef0
		void	HandleMsgTronUnknown35(ILTMessage_Read *pMsg);	// 35, 0x10017160
		void	HandleMsgTronUnknown23(ILTMessage_Read *pMsg);	// 23, 0x10017190
		void	HandleMsgMatchOver(ILTMessage_Read *pMsg);		// 36, 0x100171d0
		void	HandleMsgTronUnknown37(ILTMessage_Read *pMsg);	// 37, 0x10017260
		void	HandleMsgTronUnknown38(ILTMessage_Read *pMsg);	// 38, 0x100172d0
		void	HandleMsgTronUnknown39(ILTMessage_Read *pMsg);	// 39, 0x10017490

		// RETAIL: 0x100175f0: a team's voice line of a kind (0 match, 1
		// round, else none) when the teams have more than one player
		const char*	GetTeamSound(int nKind, int nTeam);

		int		m_nState;			// +0x00 0 none, 1 round, 2 playing, 3 round over, 4 game over
		uint8	m_nRound;			// +0x04
		uint32	m_nTronUnknown8;	// +0x08 (2 when reset)
		float	m_fTimeLeft;		// +0x0c
		float	m_fTimeLimit;		// +0x10
		float	m_fStartTime;		// +0x14
		uint8	m_nTronUnknown18;	// +0x18
		uint8	m_nTronUnknown19;	// +0x19
		uint16	m_nTronUnknown1A;	// +0x1a
		uint8	m_nTronUnknown1C;	// +0x1c
		uint8	m_nTronUnknown1D;	// +0x1d
		uint8	m_nTronUnknown1E;	// +0x1e
		bool	m_bTronUnknown1F;	// +0x1f
};

extern CLightCycleMatchMgr* g_pLightCycleMatchMgr;	// RETAIL: 0x10236df0

#endif // __LIGHTCYCLEMATCHMGR_H__
