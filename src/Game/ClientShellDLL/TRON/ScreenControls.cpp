// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenControls.cpp
//
// PURPOSE : Interface screen for setting control options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenControls.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "GameSettings.h"
#include "GameClientShell.h"

// RETAIL: Tron's "restore the default controls?" string
#define IDS_TRON_CONFIRM_RESTORE	0x243

namespace
{
	int kGap = 200;
	int kWidth = 200;

	void ConfirmCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenControls *pThisScreen = (CScreenControls *)pData;
		if (pThisScreen)
		{
			pThisScreen->ConfirmSetting(bReturn);
		}
	}


	bool bHasJoystick = true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenControls::CScreenControls()
{
    m_pJoystickCtrl = LTNULL;

    m_bUseJoystick=LTFALSE;
	m_nConfirm = 0;

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
	bHasJoystick = (dwAdvancedOptions & AO_JOYSTICK) != 0;
	if (bHasJoystick)
	{
		bHasJoystick = g_pGameClientShell->HasJoystick() || g_pGameClientShell->HasGamepad();
	}
}

CScreenControls::~CScreenControls()
{

}

// Build the screen
LTBOOL CScreenControls::Build()
{
	// RETAIL: Tron's string IDs (0x10058220)
	CreateTitle(0x15);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CONTROLS,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CONTROLS,"SliderWidth");

	//customize
	AddTextItem(0x5a, CMD_CONFIGURE, 0xaf);

	// Mouse
	AddTextItem(0x57, CMD_MOUSE, 0xb0);

	// keyboard
	AddTextItem(0x56, CMD_KEYBOARD, 0xf3);

#ifdef TRON20SRC_MODERN
	// MODERN.md CL-23: no "Use joystick/gamepad" and "Configure
	// joystick/gamepad": the controller's sticks are the client's (CL-20),
	// and an axis bound there would move a second time. "Controller" is the
	// controller's own screen instead, always there (a pad can be plugged
	// in later); its buttons are bound in Configure controls.
	m_pJoystickCtrl = AddTextItem(IDS_MODERN_PAD_TITLE, CMD_JOYSTICK, IDS_HELP_MODERN_PAD);
#else
	// use joystick
	CLTGUIToggle* pToggle = AddToggle(0x166, 0xba, kGap, &m_bUseJoystick);
	pToggle->NotifyOnChange(CMD_UPDATE,this);
	pToggle->SetOnString(LoadTempWString(0x35));
	pToggle->SetOffString(LoadTempWString(0x36));
	pToggle->Enable( bHasJoystick ? LTTRUE : LTFALSE );

	// joystick
	m_pJoystickCtrl = AddTextItem(0x58, CMD_JOYSTICK, 0xb1);
	m_pJoystickCtrl->Enable( bHasJoystick );
#endif

	//restore
	AddTextItem(0x59, CMD_RESET_DEFAULTS, 0xb3);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CScreenControls::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_CONFIGURE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_CONFIGURE);
			break;
		}
	case CMD_UPDATE:
		{

			UpdateData();
#ifndef TRON20SRC_MODERN
			m_pJoystickCtrl->Enable(bHasJoystick && m_bUseJoystick);
#endif
			break;

		}
	case CMD_MOUSE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MOUSE);
			break;
		}
	case CMD_KEYBOARD:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_KEYBOARD);
			break;
		}
	case CMD_JOYSTICK:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOYSTICK);
			break;
		}
	case CMD_RESET_DEFAULTS:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = ConfirmCallBack;
			mb.pData = this;
			m_nConfirm = IDS_TRON_CONFIRM_RESTORE;
			g_pInterfaceMgr->ShowMessageBox(IDS_TRON_CONFIRM_RESTORE,&mb);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};


// Change in focus
void CScreenControls::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{

		pProfile->SetControls();

		m_bUseJoystick = pProfile->m_bUseJoystick;

#ifndef TRON20SRC_MODERN
		m_pJoystickCtrl->Enable(bHasJoystick && m_bUseJoystick);
#endif

        UpdateData(LTFALSE);
	}
	else
	{
		bool bJoystickChanged = !(pProfile->m_bUseJoystick == m_bUseJoystick);
		pProfile->m_bUseJoystick = m_bUseJoystick;

		// RETAIL: 0x10058643: Tron calls ApplyJoystick here, not ApplyControls
		// (0x100efeb0), then again when the joystick setting changed
		pProfile->ApplyJoystick();
		if (bJoystickChanged)
			pProfile->ApplyJoystick();
		pProfile->Save();

		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
}


void CScreenControls::ConfirmSetting(LTBOOL bConfirm)
{
	switch (m_nConfirm)
	{

	case IDS_JOYSTICK_UNBOUND:
/*
		if (bConfirm)
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOYSTICK);
		else
		{
			m_bUseJoystick = LTFALSE;
			UpdateData(LTFALSE);
		}
*/
		break;

	case IDS_TRON_CONFIRM_RESTORE:
		if (bConfirm)
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			pProfile->RestoreDefaults(PROFILE_CONTROLS);
		}
		break;
	}

}