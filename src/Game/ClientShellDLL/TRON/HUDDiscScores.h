// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDiscScores.h
//
// PURPOSE : HUDItem showing the disc games' scores
//
// RETAIL: Tron 2.0's (retail CShell.dll's CHUDDiscScores, vtable
// 0x101e6d7c; its code 0x100274a0..0x10028d70). Only the parts the other
// HUD items use are written yet: its two virtuals past CHUDItem's (the
// names are ours) and the flags they read. The members before them are
// not yet known: the filler keeps their retail offsets.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DISCSCORES_H
#define __HUD_DISCSCORES_H

#include "HUDItem.h"

class CHUDDiscScores : public CHUDItem
{
public:
	// RETAIL: slot 12 (0x10027680): shows or hides the scores (when they
	// may be shown) and updates them
	virtual void	Show(bool bShow, bool bTronUnknown);

	// RETAIL: slot 13 (0x10028d50)
	virtual bool	IsVisible();

protected:
	uint8		m_aTronUnknown10[0x6d50 - 0x10];	// not yet known
	bool		m_bShow;							// +0x6d50
	bool		m_bTronUnknown6D51;					// +0x6d51
	uint8		m_aTronUnknown6D52[0x6d78 - 0x6d52];	// not yet known
	bool		m_bTronUnknown6D78;					// +0x6d78: whether it may be shown
};

extern CHUDDiscScores* g_pDiscScores;

#endif
