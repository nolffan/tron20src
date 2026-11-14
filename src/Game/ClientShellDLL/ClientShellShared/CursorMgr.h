// ----------------------------------------------------------------------- //
//
// MODULE  : CursorMgr.h
//
// PURPOSE : Manage all mouse cursor related functionality
//
// CREATED : 12/3/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CURSOR_MGR_H__
#define __CURSOR_MGR_H__

#include "iltcursor.h"
#include "ScreenSpriteMgr.h"

class CLightLine;


class CCursorMgr;
extern CCursorMgr* g_pCursorMgr;

class CCursorMgr
{
public:
	CCursorMgr();
	~CCursorMgr();

	LTBOOL		Init();
	void		Term();

	void		ScheduleReinit(float fDelay);
	void		CheckForReinit();
	
	void		UseHardwareCursor(LTBOOL bUseHardwareCursor, bool bForce = false);
    void		UseCursor(LTBOOL bUseCursor, LTBOOL bLockCursorToCenter = LTFALSE);
	void		Update();

	void		SetCenter(int x, int y);

	// Some functions for controlling the appearance of the cursor
	void		UseSprite(CScreenSprite * pSprite);

	void		UseSprite(char * pFile);	// can be spr or dtx, returns ID
	void		UseGlowSprite(char * pFile);
	void		UseBackgroundSprite(char * pFile);

	void		KillSprite();

	// RETAIL: Tron's (0x10143610, called by CInterfaceMgr's render
	// 0x100c8a8a): draws the two lines through the cursor when they are
	// shown. The name is ours.
	void		RenderCursorLines();

	// TODO at some point in the future, we can allow multiple sprites and other FX here.

private:

	// RETAIL: Tron's CCursorMgr is 0x3c bytes (constructor 0x10142f90,
	// Init 0x10143080): no cursor surface, a flag at 0xc showing two lines
	// through the cursor (Update 0x101434f0; nothing in CShell.dll sets
	// it), and the lines at 0x34/0x38. Their names are ours.
	LTBOOL		m_bInitialized;
    LTBOOL      m_bUseCursor;
    LTBOOL      m_bUseHardwareCursor;
	bool		m_bShowCursorLines;

	LTIntPt		m_CursorCenter;

	// old-style cursor
	HLTCURSOR	m_hCursor;

	ScreenSpriteArray m_SpriteArray;
	CScreenSprite * m_pCursorSprite;
	CScreenSprite * m_pCursorGlowSprite;
	CScreenSprite * m_pCursorBackgroundSprite;

	CLightLine *	m_pHorizontalLine;
	CLightLine *	m_pVerticalLine;
};

#endif // __CURSOR_MGR_H__