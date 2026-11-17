// ----------------------------------------------------------------------- //
//
// MODULE  : HUDChatInput.h
//
// PURPOSE : HUDItem to display chat input
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_CHAT_H
#define __HUD_CHAT_H

#include "HUDItem.h"
#include "LTGUIMgr.h"

const int kMaxChatLength = 64;
const int kMaxChatHistory = 25;

//******************************************************************************************
//** HUD Chat display
//******************************************************************************************
class CHUDChatInput : public CHUDItem
{
public:
	// RETAIL: every game type
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameType >= eGameTypeSingle && eGameType <= eGameTypeMultiplayerLightcycles; }

	CHUDChatInput();

    LTBOOL      Init();
	void		Term();

	void		OnExitWorld();

    void        Render();
    void        Update();

    void        UpdateLayout();

	// Handles a key press
    LTBOOL           HandleKeyDown(int key, int rep);
    LTBOOL           HandleChar(unsigned char c);


	// RETAIL: Tron's takes the chat type from SetChatType (0x10165f10)
	void		Show(LTBOOL bShow);
	void		SetChatType(uint32 nChatType);	// RETAIL: 0x101660b0 (the name is ours)
	// RETAIL: virtual in Tron, and only while the item is enabled (its last slot)
	virtual LTBOOL	IsVisible() {return m_bVisible && m_bEnabled;}

private:
	void		Send();


private:

	CLTGUIEditCtrl	m_EditCtrl;

	LTBOOL			m_bVisible;

	LTIntPt			m_BasePos;
	uint8			m_nFontSize;

	CUIPolyString*	m_pStr;

	// RETAIL: Tron's (+0x1a8; the name is ours): 2 for a team message;
	// it takes NOLF2's m_bTeamMessage's place
	uint32			m_nChatType;

	char			m_szChatStr[kMaxChatLength];
	char			m_szChatHistory[kMaxChatHistory][kMaxChatLength];

	int				m_nHistory;

};

#endif