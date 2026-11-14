// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDecision.h
//
// PURPOSE : Definition of CHUDDecision to display transmission messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// TRON: Tron's decision window is its own: numbered boxes in textured
// frames stacked up from the bottom of the screen (retail CShell.dll
// 0x1013f660..0x10140be0). The names of what NOLF2 lacks are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DECISION_H
#define __HUD_DECISION_H

#include "HUDItem.h"
#include "ClientServerShared.h"
#include "TexturedFrame.h"

class CUIFormattedPolyString;

//******************************************************************************************
//** One choice: its number on a tab and its text, in a "chooserbox" frame
//** RETAIL: size 0x480; the destructor is inline (retail 0x1003d200)
//******************************************************************************************
class CHUDDecisionChoice
{
public:
	CHUDDecisionChoice();
	~CHUDDecisionChoice() { Term(); }

	void	Reset();
	void	Term();

	// the choice's text from the string table, or a string the server sent
	void	Init(int nNumber, uint32 nStringID, int nFontSize, int nWidth);
	void	Init(int nNumber, HSTRING hString, int nFontSize, int nWidth);

	void	GetSize(int &nWidth, int &nHeight);
	void	SetPosition(int x, int y);

	void	Render();

	bool	IsVisible() { return m_bVisible; }

protected:
	bool			m_bVisible;		// +0x0
	HTEXTURE		m_hNumberTex;	// +0x4
	LT_POLYFT4		m_Poly;			// +0x8: the number's tab
	CTexturedFrame	m_Frame;		// +0x5c
	int				m_nWidth;		// +0x474
	CUIFormattedPolyString*	m_pNumber;	// +0x478
	CUIFormattedPolyString*	m_pText;	// +0x47c
};

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDDecision : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDDecision();


    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual void        UpdateLayout();

	void	OnObjectRemove(HLOCALOBJ hObj);

	// hide or show based on messages from the server (RETAIL: 0x10140230
	// reads a sub-message type: 2 is ShowDecision's)
	void	Show(ILTMessage_Read *pMsg);
	void	ShowDecision(ILTMessage_Read *pMsg);

	// RETAIL: 0x10140810, called directly (CInterfaceMgr::OnCommandOn):
	// tells the server, and closes the window when no choice is left or the
	// layout says so
	void	Choose(uint8 nChoice);

	void	SetScale(float fScale);

	// RETAIL: virtual in Tron (its vtable's slot 12, retail 0x10140120):
	// NOLF2's Hide with the result sent: 0xFF when a new decision replaces
	// the window, 0xFE when the player walks out of range
	virtual void	Hide(uint8 nChoice);

	// RETAIL: virtual in Tron, and only while the item is enabled (its last slot)
	virtual bool	IsVisible() {return m_bVisible && m_bEnabled;}

	HOBJECT GetObject() { return m_hObject; }
	float	GetRadius() { return m_fRadius; }

protected:

    LTIntPt			m_BasePos;			// +0x10

	CUIFont*		m_pFont;			// +0x18
	uint8			m_nLayout;			// +0x1c: "DecisionWindow%d"
	uint8			m_nFontSize;		// +0x1d
	uint32			m_nTextColor;		// +0x20
	float			m_fScale;			// +0x24
	LTBOOL			m_bVisible;			// +0x28
	uint16			m_nWidth;			// +0x2c
	LTIntPt			m_Offset;			// +0x30

	LTBOOL			m_bRemoveItems;		// +0x38: "RemoveItems"
	LTBOOL			m_bRemoveWindow;	// +0x3c: "RemoveWindow"
	HOBJECT			m_hObject;			// +0x40
	HOBJECT			m_hRangeObject;		// +0x44: the radius is measured from it
	float			m_fRadius;			// +0x48
	bool			m_bTronUnknown4c;	// +0x4c: set while shown
	bool			m_bTronUnknown4d;	// +0x4d: from the message, true when hidden

	CUIPolyString*	m_pTitle;			// +0x50
	CHUDDecisionChoice	m_Choices[MAX_DECISION_CHOICES];	// +0x54
	CTexturedFrame	m_Frame;			// +0x1b54
};



#endif
