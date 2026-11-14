// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCycleGame.h
//
// PURPOSE : Tron's light cycle custom game screen: the opponents, the game
//			 speed, the trail length and the power-ups allowed
//
// RETAIL: Tron's (RTTI CScreenLightCycleGame, vtable 0x101e7de8, sizeof
// 0xaa4; retail CShell.dll 0x10068420..0x10069890). Written from the retail
// code: the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENLIGHTCYCLEGAME_H__
#define __SCREENLIGHTCYCLEGAME_H__

#include "BaseScreenLightCycle.h"

// RETAIL: the power-up popup's commands (the names are ours)
const uint32 kCmdLCPowerupItem		= 0x447;
const uint32 kCmdLCShowPowerups		= 0x448;
const uint32 kCmdLCHidePowerups		= 0x449;

class CScreenLightCycleGame : public CBaseScreenLightCycle
{
public:
	CScreenLightCycleGame();

	// Builds the screen
	virtual LTBOOL	Build();

	virtual void	Escape();
	virtual void	OnFocus(LTBOOL bFocus);

	virtual void	LoadSettings();
	virtual void	SaveSettings();
	virtual CLTGUITextCtrl*	GetTab() { return m_pCustomGameTab; }
	virtual LTRect	GetBodyRect();

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	virtual LTBOOL	OnLeft();
	virtual LTBOOL	OnRight();

	// RETAIL: 0x10069280: only calls CBaseScreenLightCycle's
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	// Shows or hides the power-up popup (retail 0x10069480)
	void	ShowPowerups(bool bShow);
	// Whether the profile has not unlocked a power-up (retail 0x10069580)
	bool	IsPowerupLocked(int nPowerup);
	// A power-up's toggle, locked or not (retail 0x10069620)
	void	UpdatePowerup(CLTGUIToggle *pToggle, LTBOOL *pbValue, uint32 *pnPowerups, uint32 nFlag, bool bUnlocked);
	// Every power-up's toggle (retail 0x10069710)
	void	UpdatePowerups();

	CLTGUISlider*		m_pMaxAICyclists;		// +0xa28
	CLTGUICycleCtrl*	m_pOpponentSkill;		// +0xa2c
	CLTGUICycleCtrl*	m_pOpponentCycle;		// +0xa30
	CLTGUICycleCtrl*	m_pGameSpeed;			// +0xa34
	CLTGUICycleCtrl*	m_pTrailLength;			// +0xa38
	int					m_nMaxAICyclists;		// +0xa3c
	uint8				m_nOpponentSkill;		// +0xa40
	uint8				m_nOpponentCycle;		// +0xa41
	uint8				m_nGameSpeed;			// +0xa42
	uint8				m_nTrailLength;			// +0xa43
	CLTGUIToggle*		m_pShield;				// +0xa44
	CLTGUIToggle*		m_pTurbo;				// +0xa48
	CLTGUIToggle*		m_pTrailSpike;			// +0xa4c
	CLTGUIToggle*		m_pMissile;				// +0xa50
	CLTGUIToggle*		m_pWallExtender;		// +0xa54
	CLTGUIToggle*		m_pTurboCurse;			// +0xa58
	CLTGUIToggle*		m_pGrab;				// +0xa5c
	CLTGUIToggle*		m_pTrailReset;			// +0xa60
	CLTGUIToggle*		m_pForceUse;			// +0xa64
	CLTGUITextCtrl*		m_pPowerupsButton;		// +0xa68
	CLTGUIListCtrl*		m_pPowerupList;			// +0xa6c
	CLTGUITextCtrl*		m_pPowerupsOK;			// +0xa70
	LTBOOL				m_bShield;				// +0xa74
	LTBOOL				m_bTurbo;				// +0xa78
	LTBOOL				m_bTrailSpike;			// +0xa7c
	LTBOOL				m_bMissile;				// +0xa80
	LTBOOL				m_bTurboCurse;			// +0xa84
	LTBOOL				m_bGrab;				// +0xa88
	LTBOOL				m_bTrailReset;			// +0xa8c
	LTBOOL				m_bForceUse;			// +0xa90
	LTBOOL				m_bWallExtender;		// +0xa94
	ControlArray		m_OptionCtrls;			// +0xa98
};

#endif // __SCREENLIGHTCYCLEGAME_H__
