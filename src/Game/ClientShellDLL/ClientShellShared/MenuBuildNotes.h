// ----------------------------------------------------------------------- //
//
// MODULE  : MenuBuildNotes.h
//
// PURPOSE : In-game build notes menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (retail CShell.dll 0x101c59f0..0x101c5f9b; the class name
// from its RTTI, sizeof 0x182c: CMenuMgr's +0x13e0). It is CMenuIntel's
// pattern: a list of the build notes Jet has, and a popup for the note.

#if !defined(_MENU_BUILDNOTES_H_)
#define _MENU_BUILDNOTES_H_

#include "BaseMenu.h"
#include "PopupText.h"

class CMenuBuildNotes : public CBaseMenu
{
public:

	virtual LTBOOL	Init();
	virtual void	Term();

	virtual void OnFocus(LTBOOL bFocus);

	virtual LTBOOL  OnUp ( );
    virtual LTBOOL  OnDown ( );
    virtual LTBOOL  OnLeft ( );
    virtual LTBOOL  OnRight ( );
    virtual LTBOOL  OnEnter ( );
    virtual LTBOOL	OnEscape ();

    virtual LTBOOL  HandleKeyDown(int key, int rep);
    virtual LTBOOL  OnLButtonDown(int x, int y);

	// Render the control
	virtual void Render ();

	// Handle a command
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);
private:
	void GetIntelName(uint32 nTextId, char *pBuf, int nBufSize);

	CPopupText		m_PopupText;
	void			ClosePopup();
};

#endif //!defined(_MENU_BUILDNOTES_H_)
