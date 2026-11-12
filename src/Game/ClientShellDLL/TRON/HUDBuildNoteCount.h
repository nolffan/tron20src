// ----------------------------------------------------------------------- //
//
// MODULE  : HUDBuildNoteCount.h
//
// PURPOSE : HUDItem showing how many of a level's build notes Jet has
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x100223c0..
// 0x10022a20, vtable 0x101e6c50). The class name is retail's (its RTTI);
// the member names are ours. An icon and "found/total" beside it, from
// the layout's "BuildNoteCount".
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_BUILDNOTECOUNT_H
#define __HUD_BUILDNOTECOUNT_H

#include "HUDItem.h"

class CHUDBuildNoteCount : public CHUDItem
{
public:
	CHUDBuildNoteCount();
	~CHUDBuildNoteCount();

    LTBOOL      Init();
	void		Term();

	// RETAIL: single player only (0x100226c0)
	virtual bool	IsUsedInGameType(GameType eGameType) { return (eGameTypeSingle == eGameType) ? true : false; }

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	float			m_fScale;	// +0x10
	LT_POLYFT4		m_Poly;		// +0x14
	HTEXTURE		m_hIcon;	// +0x68
	CUIPolyString*	m_pText;	// +0x6c
};

#endif
