// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayer.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_PLAYER_H_
#define _SCREEN_PLAYER_H_

#include "BaseScreen.h"
#include "LayoutMgr.h"

// RETAIL: Tron's player screen (CShell.dll 0x10072880..0x10075190, vtable
// 0x101e83e0, sizeof 0x7e8): a game type cycle (on foot or light cycle), a
// model and a color for each, name and bandwidth. Member names are ours
// where retail gives none.
class CScreenPlayer : public CBaseScreen
{
public:
	CScreenPlayer();
	virtual ~CScreenPlayer();


	// Build the screen
    LTBOOL   Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);
	LTBOOL	Render(HSURFACE hDestSurf);

	virtual LTBOOL OnUp();
	virtual LTBOOL OnDown();
	virtual LTBOOL OnLeft();
	virtual LTBOOL OnRight();
    LTBOOL   OnLButtonUp(int x, int y);
	// RETAIL: slots 18 and 21 are a "return LTFALSE" (0x100bef10)
    LTBOOL   OnLButtonDblClick(int x, int y) {return LTFALSE;}
    LTBOOL   OnRButtonUp(int x, int y);
    LTBOOL   OnRButtonDblClick(int x, int y) {return LTFALSE;}
    virtual LTBOOL OnMouseMove(int x, int y);

	void	NextColor(bool bSound);
	void	PrevColor(bool bSound);
	void	NextModel(bool bSound);
	void	PrevModel(bool bSound);

protected:
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	void	UpdateBandwidth();

	void	UpdateChar();
	void	UpdatePlayerChar();
	void	UpdateCycleChar();
	void	UpdateGameType();

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	CLTGUICycleCtrl*	m_pGameType;		// 0x790
	uint8				m_nGameType;		// 0x794: 0 on foot, 1 light cycle
	uint8				m_nCycleColor;		// 0x795
	uint32				m_nCycleTeam;		// 0x798
	CLTGUIColumnCtrl*	m_pName;			// 0x79c
	CLTGUITextCtrl*		m_pModel;			// 0x7a0
	CLTGUIButton*		m_pLeft;			// 0x7a4
	CLTGUIButton*		m_pRight;			// 0x7a8
	CLTGUITextCtrl*		m_pColor;			// 0x7ac
	CLTGUIButton*		m_pColorLeft;		// 0x7b0
	CLTGUIButton*		m_pColorRight;		// 0x7b4

	int			m_nCurrentModel;			// 0x7b8
	std::string m_sPlayerName;				// 0x7bc
	int			m_nCurrentColor;			// 0x7c8

	CLTGUICycleCtrl*	m_pBandwidthCycle;	// 0x7cc
	CLTGUIColumnCtrl*	m_pBandwidth;		// 0x7d0

	uint8				m_nBandwidth;		// 0x7d4
	CString				m_sBandwidth;		// 0x7d8
	bool				m_bCanChangeColor;	// 0x7dc
	bool				m_bCanChangeModel;	// 0x7dd

	LTFLOAT				m_fRotation;		// 0x7e0
	LTFLOAT				m_fLastTime;		// 0x7e4
};


#endif // _SCREEN_PLAYER_H_
