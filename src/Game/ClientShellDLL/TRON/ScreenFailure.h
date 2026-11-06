// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenFailure.h
//
// PURPOSE : Interface screen for handling mission failure
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_FAILURE_H_)
#define _SCREEN_FAILURE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenFailure : public CBaseScreen
{
public:
	CScreenFailure();
	virtual ~CScreenFailure();

	// Build the screen
    virtual LTBOOL	Build();
    virtual void	OnFocus(LTBOOL bFocus);
	virtual void	Escape();

	// RETAIL: Tron's screen ends on a key or button going down (retail's
	// vtable overrides HandleKeyDown, OnLButtonDown and OnRButtonDown, with
	// one body: 0x1005b140); Build 69's took them going up
    virtual LTBOOL	HandleKeyDown(int key, int rep);
    virtual LTBOOL	OnLButtonDown(int x, int y);
    virtual LTBOOL	OnRButtonDown(int x, int y);

    virtual LTBOOL	Render(HSURFACE hDestSurf);


protected:

	CLTGUITextCtrl		*m_pString;
	CLTGUITextCtrl		*m_pHelp;
	
	bool				m_bSentAutoLoadMessage;


};

#endif // !defined(_SCREEN_FAILURE_H_)