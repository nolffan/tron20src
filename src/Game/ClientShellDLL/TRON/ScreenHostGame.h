// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostGame.h
//
// PURPOSE : Tron's host screen: the settings of the three multiplayer game
//			 types (disc arena, disc tournament, light cycles)
//
// RETAIL: Tron's (RTTI CScreenHostGame, vtable 0x101e7a40, sizeof 0xbc0;
// retail CShell.dll 0x1005d9e0..0x10060c20). Written from the retail code:
// the member and function names are ours, after the ServerGameOptions
// members each control edits.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENHOSTGAME_H__
#define __SCREENHOSTGAME_H__

#include "BaseScreenHost.h"

class CScreenHostGame : public CBaseScreenHost
{
public:
	CScreenHostGame();
	virtual ~CScreenHostGame();

	virtual void	Term();

	// Build the screen
	virtual LTBOOL	Build();
	virtual void	Escape();

	virtual void	OnFocus(LTBOOL bFocus);
	virtual void	ScreenDimsChanged();

	virtual void	LoadOptions();
	virtual void	SaveOptions();

	// The team sliders' range callback (retail 0x10060af0): keeps the number
	// of teams times the team size within the game type's limit
	bool	CheckTeamSize(CLTGUISlider *pSlider, int nNewPos);

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	virtual LTBOOL	OnLeft();
	virtual LTBOOL	OnRight();

	// RETAIL: 0x100618b0 (folded with CScreenHostMaps'): the base screen's
	// edit results
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	// Shows the current game type's controls (retail 0x100603c0)
	void	UpdateGameType();
	// Shows or hides the light cycle power-up list (retail 0x10060a60)
	void	ShowPowerups(bool bShow);

	CLTGUICycleCtrl*	m_pGameTypeCycle;		// +0xa14
	uint8				m_nGameType;			// +0xa18: the GameType less eGameTypeDiscArena

	// The disc arena's controls (+0xa1c) and values (+0xa50)
	CLTGUISlider*		m_pArenaTeams;			// +0xa1c
	CLTGUISlider*		m_pArenaTeamSize;		// +0xa20
	CLTGUIToggle*		m_pArenaFriendlyFire;	// +0xa24
	CLTGUIToggle*		m_pArenaDiscsInSpectator;	// +0xa28
	CLTGUIToggle*		m_pArenaAllowAutoAim;	// +0xa2c
	CLTGUISlider*		m_pArenaPointsPerMatch;	// +0xa30
	CLTGUISlider*		m_pArenaMatchTimeLimit;	// +0xa34
	CLTGUISlider*		m_pArenaWinLimit;		// +0xa38
	CLTGUISlider*		m_pArenaTimeLimit;		// +0xa3c
	CLTGUISlider*		m_pArenaGamesPerMap;	// +0xa40
	CLTGUISlider*		m_pArenaPreGameDelay;	// +0xa44
	CLTGUISlider*		m_pArenaMaxPing;		// +0xa48
	CLTGUISlider*		m_pArenaAirControl;		// +0xa4c
	int					m_nArenaTeams;			// +0xa50
	int					m_nArenaTeamSize;		// +0xa54
	LTBOOL				m_bArenaFriendlyFire;	// +0xa58
	LTBOOL				m_bArenaDiscsInSpectator;	// +0xa5c
	LTBOOL				m_bArenaAllowAutoAim;	// +0xa60
	int					m_nArenaPointsPerMatch;	// +0xa64
	int					m_nArenaMatchTimeLimit;	// +0xa68
	int					m_nArenaWinLimit;		// +0xa6c
	int					m_nArenaTimeLimit;		// +0xa70
	int					m_nArenaGamesPerMap;	// +0xa74
	int					m_nArenaPreGameDelay;	// +0xa78
	int					m_nArenaMaxPing;		// +0xa7c
	int					m_nArenaAirControl;		// +0xa80

	// The disc tournament's controls (+0xa84) and values (+0xab4)
	CLTGUISlider*		m_pTournTeams;			// +0xa84
	CLTGUISlider*		m_pTournTeamSize;		// +0xa88
	CLTGUIToggle*		m_pTournFriendlyFire;	// +0xa8c
	CLTGUIToggle*		m_pTournDiscsInSpectator;	// +0xa90
	CLTGUIToggle*		m_pTournAllowAutoAim;	// +0xa94
	CLTGUISlider*		m_pTournPointsPerMatch;	// +0xa98
	CLTGUISlider*		m_pTournMatchTimeLimit;	// +0xa9c
	CLTGUISlider*		m_pTournTeamsToStart;	// +0xaa0
	CLTGUISlider*		m_pTournGamesPerMap;	// +0xaa4
	CLTGUISlider*		m_pTournPreGameDelay;	// +0xaa8
	CLTGUISlider*		m_pTournMaxPing;		// +0xaac
	CLTGUISlider*		m_pTournAirControl;		// +0xab0
	int					m_nTournTeams;			// +0xab4
	int					m_nTournTeamSize;		// +0xab8
	LTBOOL				m_bTournFriendlyFire;	// +0xabc
	LTBOOL				m_bTournDiscsInSpectator;	// +0xac0
	LTBOOL				m_bTournAllowAutoAim;	// +0xac4
	int					m_nTournPointsPerMatch;	// +0xac8
	int					m_nTournMatchTimeLimit;	// +0xacc
	int					m_nTournTeamsToStart;	// +0xad0
	uint8				m_nTournMode;			// +0xad4: kept, no control
	int					m_nTournGamesPerMap;	// +0xad8
	int					m_nTournPreGameDelay;	// +0xadc
	int					m_nTournMaxPing;		// +0xae0
	int					m_nTournAirControl;		// +0xae4

	// The light cycles' controls (+0xae8) and values (+0xb4c)
	CLTGUISlider*		m_pLCTeams;				// +0xae8
	CLTGUISlider*		m_pLCTeamSize;			// +0xaec
	CLTGUICycleCtrl*	m_pLCTrailLength;		// +0xaf0
	CLTGUICycleCtrl*	m_pLCGameSpeed;			// +0xaf4
	CLTGUICycleCtrl*	m_pLCAllowedCycleType;	// +0xaf8
	CLTGUISlider*		m_pLCWaitForPlayersTime;	// +0xafc
	CLTGUISlider*		m_pLCTotalRaces;		// +0xb00
	CLTGUISlider*		m_pLCTotalPoints;		// +0xb04
	CLTGUISlider*		m_pLCTotalTime;			// +0xb08
	CLTGUISlider*		m_pLCMaxPing;			// +0xb0c
	CLTGUIToggle*		m_pLCShield;			// +0xb10: the power-ups (LCPowerupBits)
	CLTGUIToggle*		m_pLCTurbo;				// +0xb14
	CLTGUIToggle*		m_pLCTrailSpike;		// +0xb18
	CLTGUIToggle*		m_pLCMissile;			// +0xb1c
	CLTGUIToggle*		m_pLCTurboCurse;		// +0xb20
	CLTGUIToggle*		m_pLCGrab;				// +0xb24
	CLTGUIToggle*		m_pLCTrailReset;		// +0xb28
	CLTGUIToggle*		m_pLCForceUse;			// +0xb2c
	CLTGUIToggle*		m_pLCWallExtender;		// +0xb30
	CLTGUITextCtrl*		m_pPowerups;			// +0xb34: opens the power-up list
	CLTGUIListCtrl*		m_pPowerupList;			// +0xb38
	CLTGUITextCtrl*		m_pPowerupsDone;		// +0xb3c: closes it
	CLTGUISlider*		m_pLCKillPointValue;	// +0xb40
	CLTGUISlider*		m_pLCSelfKillPointValue;	// +0xb44
	CLTGUISlider*		m_pLCWinRacePointValue;	// +0xb48
	int					m_nLCTeams;				// +0xb4c
	int					m_nLCTeamSize;			// +0xb50
	uint8				m_nLCTrailLength;		// +0xb54
	uint8				m_nLCGameSpeed;			// +0xb55
	uint8				m_nLCAllowedCycleType;	// +0xb56
	int					m_nLCWaitForPlayersTime;	// +0xb58
	int					m_nLCTotalRaces;		// +0xb5c
	int					m_nLCTotalPoints;		// +0xb60
	int					m_nLCTotalTime;			// +0xb64
	int					m_nLCMaxPing;			// +0xb68
	LTBOOL				m_bLCShield;			// +0xb6c
	LTBOOL				m_bLCTurbo;				// +0xb70
	LTBOOL				m_bLCTrailSpike;		// +0xb74
	LTBOOL				m_bLCMissile;			// +0xb78
	LTBOOL				m_bLCTurboCurse;		// +0xb7c
	LTBOOL				m_bLCGrab;				// +0xb80
	LTBOOL				m_bLCTrailReset;		// +0xb84
	LTBOOL				m_bLCForceUse;			// +0xb88
	LTBOOL				m_bLCWallExtender;		// +0xb8c
	int					m_nLCKillPointValue;	// +0xb90
	int					m_nLCSelfKillPointValue;	// +0xb94: shown as a penalty (negated)
	int					m_nLCWinRacePointValue;	// +0xb98

	// Each game type's controls, shown when it is selected
	ControlArray		m_ArenaCtrls;			// +0xb9c
	ControlArray		m_TournCtrls;			// +0xba8
	ControlArray		m_LCCtrls;				// +0xbb4
};

#endif // __SCREENHOSTGAME_H__
