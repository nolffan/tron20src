// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenGame.cpp
//
// PURPOSE : Interface screen for setting gameplay options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

// RETAIL: CScreenGame is 0x1005b1f0..0x1005b9b3 in Tron's CShell.dll,
// written from the retail code; the string IDs are Tron's.
#include "ScreenGame.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"

#include "GameClientShell.h"
#include "VersionMgr.h"
#include "ResShared.h"

namespace
{
	int kGap = 0;
	int kWidth = 0;
}

//extern VarTrack	g_vtSubtitles;
VarTrack	g_vtSubtitles;
extern VarTrack g_vtHUDLayout;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenGame::CScreenGame()
{
	m_nDifficulty = 2;
    m_nSubtitles  = 0;
	m_bAlwaysRun = LTTRUE;
	m_nLayout = 0;
	m_nHeadBob = 0;
	m_nWeaponSway = 0;
	m_nMsgDur = 0;
	m_bLCThirdPersonCam = LTTRUE;
	m_bDiscAutoAim = LTFALSE;
	m_nLCCamSpeed = 5;
	m_nLCCamLag = 5;
}

CScreenGame::~CScreenGame()
{

}

// Build the screen
LTBOOL CScreenGame::Build()
{
	CreateTitle(0x1b);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_GAME,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_GAME,"SliderWidth");

	CLTGUICycleCtrl* pCycle = AddCycle(0xf7,0xff,kGap,&m_nSubtitles);
	pCycle->AddString(LoadTempWString(0x34));
	pCycle->AddString(LoadTempWString(0x33));

	//always run
	CLTGUIToggle* pToggle = AddToggle(0x176, 0xa0, kGap, &m_bAlwaysRun );
	pToggle->SetOnString(LoadTempWString(0x35));
	pToggle->SetOffString(LoadTempWString(0x36));

	// Add the Difficulty option
	m_pDifficultyCtrl = AddCycle(0x17e,0x96,kGap,&m_nDifficulty);
	m_pDifficultyCtrl->AddString(LoadTempWString(0x177));
	m_pDifficultyCtrl->AddString(LoadTempWString(0x178));
	m_pDifficultyCtrl->AddString(LoadTempWString(0x179));
	m_pDifficultyCtrl->AddString(LoadTempWString(0x17a));

	CLTGUISlider *pSlider=AddSlider(0x7e, 0x80, kGap, kWidth, -1, &m_nHeadBob);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(0x7f, 0x81, kGap, kWidth, -1, &m_nWeaponSway);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(0x83, 0x82, kGap, kWidth, -1, &m_nMsgDur);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	// RETAIL: Tron's disc and light cycle options
	pToggle = AddToggle(0xcad, 0xcae, kGap, &m_bDiscAutoAim );
	pToggle->SetOnString(LoadTempWString(0x35));
	pToggle->SetOffString(LoadTempWString(0x36));

	pToggle = AddToggle(0x287, 0x288, kGap, &m_bLCThirdPersonCam );
	pToggle->SetOnString(LoadTempWString(0x35));
	pToggle->SetOffString(LoadTempWString(0x36));

	pSlider=AddSlider(0xc77, 0xc78, kGap, kWidth, -1, &m_nLCCamSpeed);
	pSlider->SetSliderRange(0, 40);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(0xc79, 0xc7a, kGap, kWidth, -1, &m_nLCCamLag);
	pSlider->SetSliderRange(0, 11);
	pSlider->SetSliderIncrement(1);

	pToggle = AddToggle(0xcc3, 0xcc4, kGap, &m_bLCDialog );
	pToggle->SetOnString(LoadTempWString(0x35));
	pToggle->SetOffString(LoadTempWString(0x36));

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

// RETAIL: Tron has no crosshair screen (no CScreenCrosshair, no CROSSHAIR
// id): its CScreenGame::OnCommand (vtable slot 1) is the one-line body
// shared with CScreenDisplay's (0x1006fac0)
uint32 CScreenGame::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};

void CScreenGame::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetGameOptions();
	
		m_nDifficulty = pProfile->m_nDifficulty;
		m_nSubtitles  = pProfile->m_nSubtitles;
		m_bAlwaysRun = pProfile->m_bAlwaysRun;
		m_nLayout = pProfile->m_nLayout;
		m_nHeadBob = pProfile->m_nHeadBob;
		m_nWeaponSway = pProfile->m_nWeaponSway;
		m_nMsgDur = pProfile->m_nMsgDur;
		m_bLCThirdPersonCam = pProfile->m_bLCThirdPersonCam;
		m_bDiscAutoAim = pProfile->m_bDiscAutoAim;
		m_nLCCamSpeed = pProfile->m_nLCCamSpeed;
		m_nLCCamLag = pProfile->m_nLCCamLag;
		m_bLCDialog = pProfile->m_bLCDialog;

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pProfile->m_nDifficulty = m_nDifficulty;
		pProfile->m_nSubtitles  = m_nSubtitles;
		pProfile->m_bAlwaysRun = m_bAlwaysRun;
		pProfile->m_nLayout = m_nLayout;
		pProfile->m_nHeadBob = m_nHeadBob;
		pProfile->m_nWeaponSway = m_nWeaponSway;
		pProfile->m_nMsgDur = m_nMsgDur;
		pProfile->m_bLCThirdPersonCam = !!m_bLCThirdPersonCam;
		pProfile->m_bDiscAutoAim = m_bDiscAutoAim;
		pProfile->m_nLCCamSpeed = m_nLCCamSpeed;
		pProfile->m_nLCCamLag = m_nLCCamLag;
		pProfile->m_bLCDialog = m_bLCDialog;

		pProfile->ApplyGameOptions();
		pProfile->Save();

	}
	CBaseScreen::OnFocus(bFocus);
}


