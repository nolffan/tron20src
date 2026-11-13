// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenServerInfo.h
//
// PURPOSE : Tron's server info tab: the players and the game settings of
//			 the server chosen in the server list
//
// RETAIL: Tron's (RTTI CScreenServerInfo, vtable 0x101e85bc, sizeof 0xb80;
// retail CShell.dll 0x10077ec0..0x1007bcd0). Written from the retail code:
// the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENSERVERINFO_H__
#define __SCREENSERVERINFO_H__

#include "BaseScreenJoin.h"

class CScreenServerInfo : public CBaseScreenJoin
{
public:
	CScreenServerInfo();
	virtual ~CScreenServerInfo();

	virtual LTBOOL	Build();
	virtual void	OnFocus(LTBOOL bFocus);
	virtual void	ScreenDimsChanged();
	virtual LTBOOL	Render(HSURFACE hDestSurf);

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Fills the tab from g_hJoinServer's keys, asking the server for them
	// first when it has not all of them or bRefresh is set (retail
	// 0x1007a500)
	void	UpdateInfo(bool bRefresh);

	// The refresh button (retail 0x1007bc40)
	void	Refresh();

	typedef std::vector<CLTGUICtrl*> CtrlArray;

	void*				m_xTronUnknownA74;	// +0xa74: zeroed by the constructor only
	CLTGUITextCtrl*		m_pNoInfo;			// +0xa78: shown while there is no server
	CLTGUIButton*		m_pRefresh;			// +0xa7c
	CLTGUIListCtrl*		m_pPlayerList;		// +0xa80
	CLTGUIColumnCtrl*	m_pPlayersHeader;	// +0xa84: "players (n/max)"
	CLTGUIColumnCtrl*	m_apPlayers[16];	// +0xa88: name and score
	uint8				m_nGameType;		// +0xac8: the server's GameType

	CLTGUIColumnCtrl*	m_pName;			// +0xacc
	CLTGUIColumnCtrl*	m_pMap;				// +0xad0
	CLTGUIColumnCtrl*	m_pPassword;		// +0xad4
	CLTGUIColumnCtrl*	m_pDedicated;		// +0xad8
	CLTGUIColumnCtrl*	m_pGameTypeName;	// +0xadc

	// disc tournament settings
	CLTGUIColumnCtrl*	m_pTournamentTeamSize;		// +0xae0
	CLTGUIColumnCtrl*	m_pTournamentFriendlyFire;
	CLTGUIColumnCtrl*	m_pTournamentWaitWeapons;
	CLTGUIColumnCtrl*	m_pTournamentAutoAim;
	CLTGUIColumnCtrl*	m_pTournamentMatchPoints;
	CLTGUIColumnCtrl*	m_pTournamentMatchTime;
	CLTGUIColumnCtrl*	m_pTournamentGamesPerMap;
	CLTGUIColumnCtrl*	m_pTournamentMaxPing;
	CLTGUIColumnCtrl*	m_pTournamentAirControl;	// +0xb00

	// disc arena settings
	CLTGUIColumnCtrl*	m_pArenaTeamSize;			// +0xb04
	CLTGUIColumnCtrl*	m_pArenaFriendlyFire;
	CLTGUIColumnCtrl*	m_pArenaWaitWeapons;
	CLTGUIColumnCtrl*	m_pArenaAutoAim;
	CLTGUIColumnCtrl*	m_pArenaMatchPoints;
	CLTGUIColumnCtrl*	m_pArenaFragLimit;
	CLTGUIColumnCtrl*	m_pArenaTimeLimit;
	CLTGUIColumnCtrl*	m_pArenaMatchTime;
	CLTGUIColumnCtrl*	m_pArenaGamesPerMap;
	CLTGUIColumnCtrl*	m_pArenaMaxPing;
	CLTGUIColumnCtrl*	m_pArenaAirControl;			// +0xb2c

	// light cycle settings
	CLTGUIColumnCtrl*	m_pCycleTeamSize;			// +0xb30
	CLTGUIColumnCtrl*	m_pCycleType;
	CLTGUIColumnCtrl*	m_pCycleTrailLength;
	CLTGUIColumnCtrl*	m_pCycleGamesPerMap;
	CLTGUIColumnCtrl*	m_pCycleFragLimit;
	CLTGUIColumnCtrl*	m_pCycleTimeLimit;
	CLTGUIColumnCtrl*	m_pCycleMaxPing;
	CLTGUIColumnCtrl*	m_pCycleKillPoints;
	CLTGUIColumnCtrl*	m_pCycleDeathPoints;
	CLTGUIColumnCtrl*	m_pCycleWinPoints;			// +0xb54

	CtrlArray			m_TournamentCtrls;	// +0xb58
	CtrlArray			m_ArenaCtrls;		// +0xb64
	CtrlArray			m_CycleCtrls;		// +0xb70

	float				m_fRefreshTimer;	// +0xb7c: the refresh button waits this long
};

#endif // __SCREENSERVERINFO_H__
