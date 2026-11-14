// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDiscScores.h
//
// PURPOSE : HUDItem showing the disc games' scores
//
// RETAIL: Tron 2.0's, retail-only: retail CShell.dll's CHUDDiscScores
// (vtable 0x101e6d7c; its TU 0x10027370..0x10028d70), 0x6d7c bytes.
// CTronHUDMgr news it (0x1009af77) and g_pDiscScores (0x1026ee08) points
// to it. A line per player of each team with players (the team's name and
// figures on its first player's line), sorted by the players' frags, and
// a legend of the three icons a player's state shows. Member names are
// ours where no string names them.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DISCSCORES_H
#define __HUD_DISCSCORES_H

#include "HUDItem.h"

class CHUDDiscScores : public CHUDItem
{
public:
	CHUDDiscScores();							// RETAIL: 0x10027370
	~CHUDDiscScores();							// RETAIL: 0x100274c0

	LTBOOL		Init();							// RETAIL: 0x10027560
	void		Term();							// RETAIL: 0x100275e0

	// RETAIL: the disc games (folded with CHUDDiscMatches's, 0x10025ae0)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		if (eGameType >= eGameTypeDiscArena && eGameType <= eGameTypeDiscTournament)
			return true;
		return false;
	}

	void		Render();						// RETAIL: 0x100276b0
	void		Update();						// RETAIL: 0x10027860
	void		UpdateLayout();					// RETAIL: 0x10028410

	// RETAIL: slot 12 (0x10027680): shows or hides the scores (when they
	// may be shown) and updates them
	virtual void	Show(bool bShow, bool bScreen);

	// RETAIL: slot 13 (0x10028d50)
	virtual bool	IsVisible();

	enum Constants
	{
		kMaxRows = 16,
		kNumLegends = 3,
	};

protected:

	CLTGUIColumnCtrl	m_Header;							// +0x10
	CLTGUIFrame			m_HeaderFrame;						// +0x5c0
	CLTGUIColumnCtrl	m_Rows[kMaxRows];					// +0xae8
	CLTGUIFrame			m_Frame;							// +0x65e8
	CLTGUITextCtrl		m_Legends[kNumLegends];				// +0x6b10
	LT_POLYFT4			m_LegendIcons[kNumLegends];			// +0x6c48
	HTEXTURE			m_hPlaying;							// +0x6d44
	HTEXTURE			m_hWaitingForMatch;					// +0x6d48
	HTEXTURE			m_hWaitingForTournament;			// +0x6d4c
	bool				m_bShow;							// +0x6d50
	bool				m_bScreen;							// +0x6d51
	LTIntPt				m_BasePos;							// +0x6d54
	uint8				m_nBaseFontSize;					// +0x6d5c
	uint32				m_nTextColor;						// +0x6d60
	uint32				m_nElimTextColor;					// +0x6d64
	bool				m_bControlsInited;					// +0x6d68
	float				m_fScale;							// +0x6d6c
	uint16				m_nFrameWidth;						// +0x6d70
	bool				m_bHasPlayers;						// +0x6d72 some team has players
	GameType			m_eGameType;						// +0x6d74
	bool				m_bInited;							// +0x6d78
};

extern CHUDDiscScores* g_pDiscScores;

#endif
