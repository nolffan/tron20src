// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenTeamSelect.h
//
// PURPOSE : Tron's multiplayer team selection screen: the teams and their
//			 players, the spectators, and moving the local player
//
// RETAIL: Tron's (RTTI CScreenTeamSelect, vtable 0x101e8784, 36 slots,
// sizeof 0x884; retail CShell.dll 0x10082510..0x10084080, SCREEN_ID_TEAMSELECT).
// Written from the retail code: the member and function names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENTEAMSELECT_H__
#define __SCREENTEAMSELECT_H__

#include "BaseScreen.h"

class CLTGUIFrame;

class CScreenTeamSelect : public CBaseScreen
{
public:
	CScreenTeamSelect();
	virtual ~CScreenTeamSelect();

	// Build the screen
	virtual LTBOOL	Build();
	virtual LTBOOL	Render(HSURFACE hDestSurf);
	virtual void	OnFocus(LTBOOL bFocus);
	virtual LTBOOL	HandleKeyDown(int key, int rep);

	// The lists are filled again at the next Render
	void	SetNeedUpdate()		{ m_bNeedUpdate = true; }

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Fills the team and the spectator lists (retail 0x10083330)
	void	Update();

	int					m_nListFontSize;		// +0x790 "ListFontSize"
	int					m_nHeaderFontSize;		// +0x794 "HeaderFontSize"
	bool				m_bNeedUpdate;			// +0x798
	uint32				m_nTextColor;			// +0x79c "TextColor"
	uint32				m_nPlayerTextColor;		// +0x7a0 "PlayerTextColor": the local player
	uint32				m_xTronUnknown7A4[2];	// +0x7a4: not used
	CLTGUIColumnCtrl*	m_pTeamHeader;			// +0x7ac
	CLTGUIColumnCtrl*	m_pTeamRows[16];		// +0x7b0: commands 0x433..0x442
	uint32				m_anRowTeam[16];		// +0x7f0: the team of each row
	CLTGUIColumnCtrl*	m_pSpecHeader;			// +0x830
	CLTGUIColumnCtrl*	m_pSpecRows[16];		// +0x834: command 0x447
	CLTGUIFrame*		m_pTeamFrame;			// +0x874
	CLTGUIFrame*		m_pSpecFrame;			// +0x878
	CLTGUIFrame*		m_pTeamHeaderBar;		// +0x87c
	CLTGUIFrame*		m_pSpecHeaderBar;		// +0x880
	// RETAIL: the screen is 0x888 bytes (debug_new_fn 0x100adfb6); not
	// identified
	uint32				m_xTronUnknown884;		// +0x884
};

#endif // __SCREENTEAMSELECT_H__
