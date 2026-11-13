// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenKeyboard.cpp
//
// PURPOSE : Interface screen for keyboard settings
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenKeyboard.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"

namespace
{
	int kGap = 200;
	int kWidth = 200;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// RETAIL: 0x100657c0
CScreenKeyboard::CScreenKeyboard()
{
    m_pLookCtrl = LTNULL;


    m_bAutoCenter = LTFALSE;
	m_nNormalTurn	= 15;
	m_nFastTurn		= 23;
	m_nLookUp		= 25;

}

CScreenKeyboard::~CScreenKeyboard()
{

}

// Build the screen
// RETAIL: 0x10065830. Tron's has no frame and no vehicle turn speed.
LTBOOL CScreenKeyboard::Build()
{
	CreateTitle(0xD);
	
	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_KEYBOARD,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_KEYBOARD,"SliderWidth");

	//turn speed
	CLTGUISlider *pSlider = AddSlider(0x5D, 0xC5, kGap, kWidth, -1, &m_nNormalTurn);
	pSlider->SetSliderRange(5, 55);
	pSlider->SetSliderIncrement(5);

	pSlider = AddSlider(0x5E, 0xC6, kGap, kWidth, -1, &m_nFastTurn);
	pSlider->SetSliderRange(8, 58);
	pSlider->SetSliderIncrement(5);

	// look speed
	pSlider = AddSlider(0x5F, 0xC7, kGap, kWidth, -1, &m_nLookUp);
	pSlider->SetSliderRange(5, 50);
	pSlider->SetSliderIncrement(5);

	// auto center
	// RETAIL: Tron's AddToggle also sets the toggle from the value (every
	// AddToggle in retail does: SetOn, vtable +0xc4); written out here
	m_pLookCtrl = CreateToggle(0x15F, 0x9F, kGap, &m_bAutoCenter );
	if (m_pLookCtrl)
	{
		AddControl(m_pLookCtrl);
		m_pLookCtrl->SetOn(m_bAutoCenter);
	}
	// PORT: retail's toggle strings are wide (CLTGUIToggle 0x100c01b0,
	// 0x100c0260), which our LTGUIMgr's toggle has not yet
	m_pLookCtrl->SetOnString(LoadTempString(0x35));
	m_pLookCtrl->SetOffString(LoadTempString(0x36));


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

// RETAIL: 0x10065a20
uint32 CScreenKeyboard::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_UPDATE:
		{
			UpdateData();
//			m_pLookCtrl->Enable(!m_bMouseLook);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};



// Change in focus
// RETAIL: 0x10065a50
void CScreenKeyboard::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetKeyboard();
		m_pLookCtrl->Enable(!pProfile->m_bMouseLook);
		m_bAutoCenter = pProfile->m_bAutoCenter;
		m_nNormalTurn = pProfile->m_nNormalTurn;
		m_nFastTurn = pProfile->m_nFastTurn;
		m_nLookUp = pProfile->m_nLookUp;
		
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pProfile->m_bAutoCenter = m_bAutoCenter;
		pProfile->m_nNormalTurn = m_nNormalTurn;
		pProfile->m_nFastTurn = m_nFastTurn;
		pProfile->m_nLookUp	= m_nLookUp;

		pProfile->ApplyKeyboard();
		pProfile->Save();
	}
	CBaseScreen::OnFocus(bFocus);
}
