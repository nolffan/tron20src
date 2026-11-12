// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCycleVictory.h
//
// PURPOSE : Interface screen shown when a light cycle game is won
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_LIGHTCYCLE_VICTORY_H_
#define _SCREEN_LIGHTCYCLE_VICTORY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

// RETAIL: Tron's (CShell.dll 0x1006de80..0x1006e420, vtable 0x101e7fd4,
// sizeof 0x794), written from the retail code: CScreenEndMission's pattern
// with a formatted string of its own. The name is RTTI's.
class CScreenLightCycleVictory : public CBaseScreen
{
public:
	CScreenLightCycleVictory();
	virtual ~CScreenLightCycleVictory();

	// Build the screen
    virtual LTBOOL	Build();
    virtual void	OnFocus(LTBOOL bFocus);

    virtual LTBOOL	HandleKeyDown(int key, int rep);
    virtual LTBOOL	OnLButtonDown(int x, int y);
    virtual LTBOOL	OnRButtonDown(int x, int y);

    virtual LTBOOL	Render(HSURFACE hDestSurf);

protected:

	CUIFormattedPolyString	*m_pString;
};

#endif // _SCREEN_LIGHTCYCLE_VICTORY_H_
