// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDiscMatches.h
//
// PURPOSE : HUDItem listing the disc games' matches
//
// RETAIL: Tron 2.0's, retail-only: retail CShell.dll's CHUDDiscMatches
// (vtable 0x101e6d44; its TU 0x100257a0..0x10027370), 0x36d4 bytes.
// CTronHUDMgr news it (0x1009afa6) and g_pDiscMatches (0x1026ee0c)
// points to it. For each of the disc match manager's matches (up to four)
// a line with its number, time left and teams, and a circle per round
// each team won. Member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DISCMATCHES_H
#define __HUD_DISCMATCHES_H

#include "HUDItem.h"

class CHUDDiscMatches : public CHUDItem
{
public:
	CHUDDiscMatches();							// RETAIL: 0x100257a0
	~CHUDDiscMatches();							// RETAIL: 0x100258f0

	LTBOOL		Init();							// RETAIL: 0x10025990
	void		Term();							// RETAIL: 0x10025a70

	// RETAIL: the disc games (0x10025ae0)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		if (eGameType >= eGameTypeDiscArena && eGameType <= eGameTypeDiscTournament)
			return true;
		return false;
	}

	void		Render();						// RETAIL: 0x10025b20
	void		Update();						// RETAIL: 0x10026180
	void		UpdateLayout();					// RETAIL: 0x10026b30

	// RETAIL: slot 12 (0x10025b00)
	virtual void	Show(bool bDraw, bool bScreen);

	enum Constants
	{
		kMaxMatches = 4,
		kMaxRounds = 3,
	};

protected:

	CLTGUIFrame			m_Frame;							// +0x10
	CLTGUIColumnCtrl	m_HeaderTitles;						// +0x538
	CLTGUIColumnCtrl	m_HeaderValues;						// +0xae8 the time left, the round
	CLTGUIColumnCtrl	m_Titles;							// +0x1098
	CLTGUIColumnCtrl	m_Rows[kMaxMatches];				// +0x1648
	uint8				m_nRounds[kMaxMatches];				// +0x2d08 rounds to win
	uint8				m_nWins[kMaxMatches][2];			// +0x2d0c per team
	LT_POLYFT4			m_Circles[kMaxMatches][kMaxRounds * 2];	// +0x2d14 a round's circle per team
	LT_POLYF4			m_Boxes[kMaxMatches][2];			// +0x34f4 behind the teams' names
	HTEXTURE			m_hEmpty;							// +0x3694
	HTEXTURE			m_hFull;							// +0x3698
	bool				m_bDraw;							// +0x369c
	bool				m_bScreen;							// +0x369d
	LTIntPt				m_BasePos;							// +0x36a0
	uint8				m_nBaseFontSize;					// +0x36a8
	uint32				m_nTextColor;						// +0x36ac
	uint8				m_aTronUnknown36B0[0x36bc - 0x36b0];	// not used
	bool				m_bControlsInited;					// +0x36bc
	float				m_fScale;							// +0x36c0
	uint16				m_nTronUnknown36C4;					// +0x36c4 not used
	uint16				m_nFrameWidth;						// +0x36c6
	int					m_nNumMatches;						// +0x36c8
	GameType			m_eGameType;						// +0x36cc
	bool				m_bInited;							// +0x36d0
};

extern CHUDDiscMatches* g_pDiscMatches;

#endif
