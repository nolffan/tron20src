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

namespace
{
	int kGap = 200;
	int kWidth = 200;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenJoystick::CScreenJoystick()
{
	memset(m_nAxis,0,sizeof(m_nAxis));
	memset(m_nPOV,0,sizeof(m_nPOV));

}

CScreenJoystick::~CScreenJoystick()
{

}

// Build the screen
// RETAIL: 0x10065030. Tron's string IDs (its string table) are retail's
// numbers.
LTBOOL CScreenJoystick::Build()
{
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
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};



// Change in focus
void CScreenJoystick::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
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