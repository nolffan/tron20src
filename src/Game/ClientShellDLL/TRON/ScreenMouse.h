// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMouse.h
//
// PURPOSE : Interface screen for setting mouse options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_MOUSE_H_
#define _SCREEN_MOUSE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenMouse : public CBaseScreen
{
public:
	CScreenMouse();
	virtual ~CScreenMouse();

	// Build the screen
    LTBOOL   Build();
    void OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// RETAIL: Tron's (0x7ac bytes; constructor 0x1006f5e0)
    LTBOOL	m_bInvertY;							// +0x790
    LTBOOL	m_bMouseLook;						// +0x794
	LTBOOL	m_bInvertLCMouse;					// +0x798 the light cycle's mouse
	int		m_nInputRate;						// +0x79c
	int		m_nSensitivity;						// +0x7a0

	CLTGUISlider			*m_pInputCtrl;		// +0x7a4
	CLTGUISlider			*m_pSensitivityCtrl;	// +0x7a8

};

#endif // _SCREEN_MOUSE_H_