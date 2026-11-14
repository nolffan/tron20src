// ----------------------------------------------------------------------- //
//
// MODULE  : HUDScores.h
//
// PURPOSE : Definition of CHUDScores to display the light cycle game's
//			 scores
//
// RETAIL: Tron 2.0's, a rewrite of Build 69's (which is NOLF2's): retail
// CShell.dll's CHUDScores (vtable 0x101e70f4; its TU 0x10034800..
// 0x1003a620), 0xb9d0 bytes. CTronHUDMgr news it (0x1009af48) and
// g_pScores (0x1026ee04) points to it. It shows the light cycle match
// manager's round, time and match figures above the players' (or the
// teams' and their players') scores. Member names are ours where no
// string names them.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SCORES_H
#define __HUD_SCORES_H

#include "HUDItem.h"

class CTronTeamMgr;
class CClientInfoMgr;

// RETAIL: a player's line of the scores, 0x20 bytes: a name, three
// figures (the player score's +0x0, +0x2 and +0xc), its +0xe flag (drawn
// in the "ElimTextColor") and the client's ping (not set by the
// constructor; retail 0x10034e40 builds one out of line). The names are
// ours
struct HUDScorePlayer
{
	HUDScorePlayer() : sName(""), nTronUnknownC(0), nTronUnknown10(0), nTronUnknown14(0), bTronUnknown18(false) {}

	std::string	sName;				// +0x00
	int			nTronUnknownC;		// +0x0c
	int			nTronUnknown10;		// +0x10
	int			nTronUnknown14;		// +0x14
	bool		bTronUnknown18;		// +0x18
	uint32		nPing;				// +0x1c
};

typedef std::vector<HUDScorePlayer> HUDScorePlayerArray;

// RETAIL: a team's line, 0x30 bytes: the totals of its players' and its
// players (sorted by their +0x2 figure, highest first); its ping is not
// used
struct HUDScoreTeam : public HUDScorePlayer
{
	HUDScoreTeam() : bTronUnknown2C(false) {}

	HUDScorePlayerArray	Players;	// +0x20
	bool		bTronUnknown2C;		// +0x2c: set when all its players are flagged
};

typedef std::vector<HUDScoreTeam> HUDScoreTeamArray;

//******************************************************************************************
//** HUD Scores
//******************************************************************************************
class CHUDScores : public CHUDItem
{
public:
	CHUDScores();									// RETAIL: 0x100351a0
	~CHUDScores();									// RETAIL: 0x100352e0

	virtual LTBOOL		Init();						// RETAIL: 0x100353d0
	virtual void		Term();						// RETAIL: 0x10035500

	// RETAIL: the multiplayer light cycles (0x10035550)
	virtual bool		IsUsedInGameType(GameType eGameType) { return (eGameTypeMultiplayerLightcycles == eGameType) ? true : false; }

	virtual void		Render();					// RETAIL: 0x10035560
	virtual void		Update();					// RETAIL: 0x10035960
	virtual void		UpdateLayout();				// RETAIL: 0x10035a60

	// RETAIL: slot 12 (0x10036540): the light cycle match manager shows
	// them at a round's end and hides them again
	virtual	void		Show(bool bDraw, bool bScreen = false);

	enum Constants
	{
		kMaxRows = 24,
	};

protected:

	void		UpdateScores();						// RETAIL: 0x10036570
	void		UpdateTeamScores();					// RETAIL: 0x100365a0
	void		UpdatePlayerScores();				// RETAIL: 0x10036640

	// RETAIL: each lays out its part from the position given and returns
	// the position below it
	LTIntPt		UpdatePlayers(const LTIntPt& pos);	// RETAIL: 0x100366e0
	LTIntPt		UpdateTeams(const LTIntPt& pos);	// RETAIL: 0x10037120
	LTIntPt		UpdateHeader(const LTIntPt& pos);	// RETAIL: 0x10037d80
	LTIntPt		UpdateFooter(const LTIntPt& pos);	// RETAIL: 0x100381e0

	int					m_nFrameHBorder;			// +0x10
	int					m_nFrameVBorder;			// +0x14
	bool				m_bInited;					// +0x18
	bool				m_bControlsInited;			// +0x19

	CLTGUIFrame			m_HeaderFrame;				// +0x1c
	CLTGUIColumnCtrl	m_HeaderTitles;				// +0x544
	CLTGUIColumnCtrl	m_HeaderValues;				// +0xaf4
	CLTGUIFrame			m_Frame;					// +0x10a4
	CLTGUIColumnCtrl	m_ScoreTitles;				// +0x15cc
	CLTGUIFrame			m_ScoreFrame;				// +0x1b7c
	CLTGUIColumnCtrl	m_Scores[kMaxRows];			// +0x20a4
	CLTGUIFrame			m_FooterFrame;				// +0xa924
	CLTGUIColumnCtrl	m_FooterTitles;				// +0xae4c
	CLTGUIColumnCtrl	m_FooterValues;				// +0xb3fc

	uint16		m_nFrameWidth;						// +0xb9ac
	float		m_fScale;							// +0xb9b0
	LTIntPt		m_BasePos;							// +0xb9b4
	uint8		m_nBaseFontSize;					// +0xb9bc
	uint32		m_nTextColor;						// +0xb9c0
	uint32		m_nElimTextColor;					// +0xb9c4
	bool		m_bDraw;							// +0xb9c8
	bool		m_bScreen;							// +0xb9c9
	float		m_fDrawTime;						// +0xb9cc (counted down while above 0)
};

#endif
