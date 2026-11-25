// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoystick.cpp
//
// PURPOSE : Interface screen for joystick/gamepad settings
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// RETAIL: NOLF2's TO2 screen (Build 69's TO2 copy, without nolf2src's
// modern blocks), which Tron's CShell.dll has (retail 0x10064fd0..0x10065a50).
// Tron's Build is larger -- its menus are wide-string (LoadTempString and
// AddString of wchar_t) -- and its CUserProfile::Save takes no argument;
// both are NOLF2's here (PORT).
#include "stdafx.h"
#include "ScreenJoystick.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#ifdef TRON20SRC_MODERN
// MODERN.md CL-23: the controller's default buttons
#include "xinput/clientpad.h"
#endif

namespace
{
	int kGap = 200;
	int kWidth = 200;
#ifdef TRON20SRC_MODERN
	void PadRestoreCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenJoystick *pThisScreen = (CScreenJoystick *)pData;
		if (pThisScreen)
			pThisScreen->ConfirmPadRestore(bReturn);
	}

	int PadPercent(char const* pVar, float fDefault)
	{
		return (int)(100.0f * GetConsoleFloat(pVar, fDefault) + 0.5f);
	}
#endif
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenJoystick::CScreenJoystick()
{
	memset(m_nAxis,0,sizeof(m_nAxis));
	memset(m_nPOV,0,sizeof(m_nPOV));
#ifdef TRON20SRC_MODERN
	m_nPadLookSpeed = 100;
	m_nPadLookCurve = 200;
	m_bPadInvert = LTFALSE;
	m_nPadLookDeadZone = 15;
	m_nPadMoveDeadZone = 20;
#endif

}

CScreenJoystick::~CScreenJoystick()
{

}

// Build the screen
// RETAIL: 0x10065030. Tron's string IDs (its string table) are retail's
// numbers.
LTBOOL CScreenJoystick::Build()
{
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-23: the controller's screen. Its axes are not bound
	// (the client reads the sticks, CL-20), so instead of the retail axis
	// assignments: the right stick's speed, response and direction, both
	// sticks' dead zones, and its default buttons back. The buttons
	// themselves are bound in Configure controls, like keys.
	CreateTitle(IDS_MODERN_PAD_SCREENTITLE);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOYSTICK,"ColumnWidth");
	kWidth = 200;

	CLTGUISlider *pSlider = AddSlider(IDS_MODERN_PAD_LOOKSPEED, IDS_HELP_MODERN_PAD_LOOKSPEED, kGap, kWidth, -1, &m_nPadLookSpeed);
	pSlider->SetSliderRange(25, 300);
	pSlider->SetSliderIncrement(25);
	pSlider->SetNumericDisplay(LTTRUE);

	pSlider = AddSlider(IDS_MODERN_PAD_LOOKCURVE, IDS_HELP_MODERN_PAD_LOOKCURVE, kGap, kWidth, -1, &m_nPadLookCurve);
	pSlider->SetSliderRange(100, 300);
	pSlider->SetSliderIncrement(25);
	pSlider->SetNumericDisplay(LTTRUE);

	CLTGUIToggle *pToggle = AddToggle(IDS_MODERN_PAD_INVERT, IDS_HELP_MODERN_PAD_INVERT, kGap, &m_bPadInvert);
	pToggle->SetOnString(LoadTempWString(IDS_YES));
	pToggle->SetOffString(LoadTempWString(IDS_NO));

	pSlider = AddSlider(IDS_MODERN_PAD_LOOKDEADZONE, IDS_HELP_MODERN_PAD_LOOKDEADZONE, kGap, kWidth, -1, &m_nPadLookDeadZone);
	pSlider->SetSliderRange(0, 50);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE);

	pSlider = AddSlider(IDS_MODERN_PAD_MOVEDEADZONE, IDS_HELP_MODERN_PAD_MOVEDEADZONE, kGap, kWidth, -1, &m_nPadMoveDeadZone);
	pSlider->SetSliderRange(0, 50);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE);

	AddTextItem(IDS_MODERN_PAD_RESTORE, CMD_RESET_DEFAULTS, IDS_HELP_MODERN_PAD_RESTORE);
#else
	CreateTitle(15);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOYSTICK,"ColumnWidth");
	uint8 nLarge = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOYSTICK,"HeaderFontSize");

	CLTGUICycleCtrl *pCtrl = AddCycle(0x16c, NULL, kGap, NULL, kDefaultPos, LTTRUE );
	pCtrl->AddString(LoadTempWString(0x16d));
	pCtrl->SetFont(NULL,nLarge);
	m_nextPos.y += 8;

	for (int axis = 0; axis < g_pProfileMgr->GetNumAxis(); axis++)
	{
		CDeviceAxisData *pAxisData = g_pProfileMgr->GetAxisData(axis);
		if (!pAxisData || !strlen(pAxisData->m_sName)) continue;

		std::wstring sName;
		LTGUIWiden(sName, pAxisData->m_sName);

		CLTGUICycleCtrl *pCtrl = AddCycle(sName, 0x173, kGap, &m_nAxis[axis] );
		pCtrl->SetParam1(axis);
		pCtrl->AddString(LoadTempWString(0x172));

		switch (pAxisData->m_nType)
		{
			case CONTROLTYPE_XAXIS:
			case CONTROLTYPE_RXAXIS:
			case CONTROLTYPE_ZAXIS:
				pCtrl->AddString(LoadTempWString(0x167));
				pCtrl->AddString(LoadTempWString(0x16a));
				break;
			case CONTROLTYPE_YAXIS:
			case CONTROLTYPE_RYAXIS:
			case CONTROLTYPE_RZAXIS:
				pCtrl->AddString(LoadTempWString(0x168));
				pCtrl->AddString(LoadTempWString(0x169));
				pCtrl->AddString(LoadTempWString(0x170));
				break;
		}
	}

	for (int POV = 0; POV < g_pProfileMgr->GetNumPOV(); POV++)
	{
		CDevicePOVData *pPOVData = g_pProfileMgr->GetPOVData(POV);
		if (!pPOVData || !strlen(pPOVData->m_sName)) continue;

		std::wstring sName;
		sName.erase(sName.begin(), sName.end());
		uint32 nLen = strlen(pPOVData->m_sName);
		for (uint32 i = 0; i < nLen; i++)
			sName.push_back((wchar_t)(uint8)pPOVData->m_sName[i]);

		CLTGUICycleCtrl *pCtrl = AddCycle(sName, 0x214, kGap, &m_nPOV[POV] );
		pCtrl->SetParam1(POV);
		pCtrl->AddString(LoadTempWString(0x172));

		pCtrl->AddString(LoadTempWString(0x171));
		pCtrl->AddString(LoadTempWString(0x16f));
	}
#endif


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CScreenJoystick::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_UPDATE:
		{
			UpdateData();

		
			break;
		}
#ifdef TRON20SRC_MODERN
	case CMD_RESET_DEFAULTS:
		{
			// CL-23: the controller's default buttons back
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = PadRestoreCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_MODERN_PAD_CONFIRMRESTORE,&mb);
			break;
		}
#endif
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};



#ifdef TRON20SRC_MODERN
// CL-23: the controller's column of the bindings back to its defaults; the
// keyboard's and the mouse's are left as they are
void CScreenJoystick::ConfirmPadRestore(LTBOOL bConfirm)
{
	if (!bConfirm) return;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->SetBindings();
	Pad_DefaultBindings(pProfile, true);
	pProfile->ApplyBindings();
	pProfile->Save();
}
#endif

// Change in focus
void CScreenJoystick::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
#ifdef TRON20SRC_MODERN
	// CL-23: the controller's console variables (clientpad.cpp), kept in
	// autoexec.cfg (WriteConsoleFloat sets them with "+")
	if (bFocus)
	{
		m_nPadLookSpeed = PadPercent("PadLookSpeed", 1.0f);
		m_nPadLookCurve = PadPercent("PadLookCurve", 2.0f);
		m_bPadInvert = (GetConsoleInt("PadInvertY", 0) != 0);
		m_nPadLookDeadZone = PadPercent("PadLookDeadZone", 0.15f);
		m_nPadMoveDeadZone = PadPercent("PadMoveDeadZone", 0.2f);

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		WriteConsoleFloat("PadLookSpeed", (LTFLOAT)m_nPadLookSpeed / 100.0f);
		WriteConsoleFloat("PadLookCurve", (LTFLOAT)m_nPadLookCurve / 100.0f);
		WriteConsoleInt("PadInvertY", m_bPadInvert ? 1 : 0);
		WriteConsoleFloat("PadLookDeadZone", (LTFLOAT)m_nPadLookDeadZone / 100.0f);
		WriteConsoleFloat("PadMoveDeadZone", (LTFLOAT)m_nPadMoveDeadZone / 100.0f);
	}
	CBaseScreen::OnFocus(bFocus);
	return;
#endif
	if (bFocus)
	{
		pProfile->SetJoystick();
		for (int axis = 0; axis < g_pProfileMgr->GetNumAxis(); axis++)
		{
			m_nAxis[axis] = pProfile->m_nAxis[axis];
		}
		for (int POV = 0; POV < g_pProfileMgr->GetNumPOV(); POV++)
		{
			m_nPOV[POV] = pProfile->m_nPOV[POV];
		}

	
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		for (int axis = 0; axis < g_pProfileMgr->GetNumAxis(); axis++)
		{
			pProfile->m_nAxis[axis] = m_nAxis[axis];
		};
		for (int POV = 0; POV < g_pProfileMgr->GetNumPOV(); POV++)
		{
			pProfile->m_nPOV[POV] = m_nPOV[POV];
		};

		pProfile->ApplyJoystick();
		pProfile->Save();
	}
	CBaseScreen::OnFocus(bFocus);
}