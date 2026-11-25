// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMouse.cpp
//
// PURPOSE : Interface screen for setting mouse options
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMouse.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"

// RETAIL: this TU is 0x1006f5c0..0x1006fb8e in Tron's CShell.dll, written
// from the retail code. Tron's string IDs are not in our ClientRes headers:
// the numbers are retail's.

namespace
{
	int kGap = 200;
	int kWidth = 200;

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMouse::CScreenMouse()
{

    m_bInvertY = LTFALSE;
	m_bInvertLCMouse = LTFALSE;
    m_bMouseLook = LTFALSE;
	m_nInputRate = 0;
	m_nSensitivity = 0;

    m_pSensitivityCtrl = LTNULL;
    m_pInputCtrl = LTNULL;


}

CScreenMouse::~CScreenMouse()
{

}

// Build the screen
LTBOOL CScreenMouse::Build()
{
	CreateTitle(0xe);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_MOUSE,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_MOUSE,"SliderWidth");

	std::wstring sYes = LoadTempWString(0x35);
	std::wstring sNo = LoadTempWString(0x36);

	//Always mouse look
	CLTGUIToggle* pToggle = AddToggle(0x15e, 0xc1, kGap, &m_bMouseLook );
	pToggle->SetOnString(sYes);
	pToggle->SetOffString(sNo);

	//mouse sensitivity
	// RETAIL: Tron's ranges are constants (Build 69 read console variables)
	m_pSensitivityCtrl= AddSlider(0x161, 0xc2, kGap, kWidth, -1, &m_nSensitivity);
	m_pSensitivityCtrl->SetSliderRange(0, 10);
	m_pSensitivityCtrl->SetSliderIncrement(1);

#ifndef TRON20SRC_MODERN
	//mouse responsiveness
	m_pInputCtrl=AddSlider(0x162, 0xc3, kGap, kWidth, -1,  &m_nInputRate);
	m_pInputCtrl->SetSliderRange(0, 40);
	m_pInputCtrl->SetSliderIncrement(2);
#else
	// MODERN.md EN-23, EN-24: no "Input rate": it set how much the mouse's
	// motion was smoothed, and the mouse look has no smoothing now (the
	// value stays in the profile)
#endif

	//invert mouse
	pToggle = AddToggle(0x160, 0xc0, kGap, &m_bInvertY );
	pToggle->SetOnString(sYes);
	pToggle->SetOffString(sNo);

	// RETAIL: Tron's: invert the light cycle's mouse
	pToggle = AddToggle(0xce8, 0xce9, kGap, &m_bInvertLCMouse );
	pToggle->SetOnString(sYes);
	pToggle->SetOffString(sNo);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CScreenMouse::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};



// Change in focus
void CScreenMouse::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetMouse();
		m_bMouseLook = pProfile->m_bMouseLook;
		m_bInvertY = pProfile->m_bInvertY;
		m_bInvertLCMouse = pProfile->m_bInvertLCMouse;

		m_nInputRate = pProfile->m_nInputRate;

		m_nSensitivity = pProfile->m_nSensitivity;

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pProfile->m_bMouseLook = m_bMouseLook;
		pProfile->m_bInvertY = m_bInvertY;
		pProfile->m_bInvertLCMouse = m_bInvertLCMouse;
		pProfile->m_nInputRate = m_nInputRate;
		pProfile->m_nSensitivity = m_nSensitivity;

		pProfile->ApplyMouse();
		pProfile->Save();
	}
	CBaseScreen::OnFocus(bFocus);
}