// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCycleGame.cpp
//
// PURPOSE : Tron's light cycle custom game screen: the opponents, the game
//			 speed, the trail length and the power-ups allowed
//
// RETAIL: written from retail CShell.dll 0x10068420..0x10069890 (Tron's
// own class: the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenLightCycleGame.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ProfileMgr.h"
#include "CheatMgr.h"
#include "LightCycleGameDataMgr.h"
#include "TRON/LightCycleButeMgr.h"
#include "GameClientShell.h"

namespace
{
	// RETAIL: the cheat that unlocks every power-up (retail's cheat 0x38;
	// ours has no such cheat yet: the name is ours)
	const int kCheatTronUnknown38 = 0x38;
}

// RETAIL: 0x10068440 (called from CTronScreenMgr::AddScreen's new)
CScreenLightCycleGame::CScreenLightCycleGame()
{
	m_pMaxAICyclists = LTNULL;
	m_pOpponentSkill = LTNULL;
	m_pOpponentCycle = LTNULL;
	m_pGameSpeed = LTNULL;
	m_pTrailLength = LTNULL;
	m_pShield = LTNULL;
	m_pTurbo = LTNULL;
	m_pTrailSpike = LTNULL;
	m_pMissile = LTNULL;
	m_pTurboCurse = LTNULL;
	m_pGrab = LTNULL;
	m_pTrailReset = LTNULL;
	m_pForceUse = LTNULL;
	m_pWallExtender = LTNULL;
	m_pPowerupsButton = LTNULL;
	m_pPowerupList = LTNULL;
	m_pPowerupsOK = LTNULL;
	m_nMaxAICyclists = 0;
	m_nOpponentSkill = 0;
	m_nOpponentCycle = 0;
	m_nGameSpeed = 0;
	m_nTrailLength = 0;
	m_bShield = LTTRUE;
	m_bTurbo = LTTRUE;
	m_bTrailSpike = LTTRUE;
	m_bMissile = LTTRUE;
	m_bTurboCurse = LTTRUE;
	m_bGrab = LTTRUE;
	m_bTrailReset = LTTRUE;
	m_bForceUse = LTTRUE;
	m_bWallExtender = LTTRUE;
}

// RETAIL: 0x10068620
LTBOOL CScreenLightCycleGame::Build()
{
	LTBOOL bRet = CBaseScreenLightCycle::Build();
	CreateSettingsList();

	// RETAIL: an empty string retail makes and does not use
	std::wstring sUnused;

	int nColumnWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_BASE_LIGHTCYCLE,"ColumnWidth");

	m_pMaxAICyclists = AddSlider(0x2590,0x2591,nColumnWidth,50,-1,&m_nMaxAICyclists,
		g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_LIGHTCYCLE_CUSTOMGAME,"CommandPos"));
	m_pMaxAICyclists->SetNumericDisplay(LTTRUE);
	// RETAIL: here Tron's slider sets a member of its own (0x100c0a60 with
	// 0, +0x54): left out, our LTGUIMgr's slider has none
	m_pMaxAICyclists->SetSliderRange(1,8);
	m_pMaxAICyclists->SetSliderIncrement(1);
	m_OptionCtrls.push_back(m_pMaxAICyclists);

	m_pOpponentSkill = AddCycle(0x2592,0x2593,nColumnWidth,&m_nOpponentSkill);
	m_pOpponentSkill->AddString(LoadTempWString(0x25DA));
	m_pOpponentSkill->AddString(LoadTempWString(0x25DB));
	m_pOpponentSkill->AddString(LoadTempWString(0x25DC));
	m_pOpponentSkill->AddString(LoadTempWString(0x25DD));
	m_pOpponentSkill->AddString(LoadTempWString(0x25DE));
	m_OptionCtrls.push_back(m_pOpponentSkill);

	m_pOpponentCycle = AddCycle(0x2598,0x2599,nColumnWidth,&m_nOpponentCycle);
	m_pOpponentCycle->AddString(LoadTempWString(0x259A));
	m_pOpponentCycle->AddString(LoadTempWString(0x259B));
	m_pOpponentCycle->AddString(LoadTempWString(0x259C));
	m_OptionCtrls.push_back(m_pOpponentCycle);

	m_pGameSpeed = AddCycle(0x25AE,0x25AF,nColumnWidth,&m_nGameSpeed);
	m_pGameSpeed->AddString(LoadTempWString(0x25C0));
	m_pGameSpeed->AddString(LoadTempWString(0x25C1));
	m_pGameSpeed->AddString(LoadTempWString(0x25C2));
	m_pGameSpeed->AddString(LoadTempWString(0x25C3));
	m_OptionCtrls.push_back(m_pGameSpeed);

	m_pTrailLength = AddCycle(0x25B0,0x25B1,nColumnWidth,&m_nTrailLength);
	m_pTrailLength->AddString(LoadTempWString(0x25C4));
	m_pTrailLength->AddString(LoadTempWString(0x25C5));
	m_pTrailLength->AddString(LoadTempWString(0x25C6));
	m_pTrailLength->AddString(LoadTempWString(0x25C7));
	m_OptionCtrls.push_back(m_pTrailLength);

	m_pPowerupsButton = AddTextItem(0x26B6,kCmdLCShowPowerups,0x26B7);
	m_OptionCtrls.push_back(m_pPowerupsButton);

	// the power-up popup
	LTRect rcBody = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"BodyRect");
	int nLeft = rcBody.left + 32;
	int nTop = rcBody.top + 32;
	int nWidth = (rcBody.right - 32) - nLeft;
	int nBottom = rcBody.bottom - 32;
	m_pPowerupList = AddList(LTIntPt(nLeft,nTop),nBottom - nTop,LTTRUE,nWidth - 16);
	if (m_pPowerupList)
	{
		m_pPowerupList->SetIndent(g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LCPowerupListIndent"));
		// RETAIL: here Tron's list control takes a frame texture
		// ("interface\menu\sprtex\frame_solid.dtx", CLTGUIListCtrl 0x100b98e0):
		// left out, our LTGUIMgr's list control has none
		m_pPowerupList->Show(LTFALSE);

		m_pShield = CreateToggle(0x2580,0x2581,nColumnWidth,&m_bShield);
		m_pShield->SetOnString(LoadTempString(0x35));
		m_pShield->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pShield);

		m_pTurbo = CreateToggle(0x2582,0x2583,nColumnWidth,&m_bTurbo);
		m_pTurbo->SetOnString(LoadTempString(0x35));
		m_pTurbo->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pTurbo);

		m_pTrailSpike = CreateToggle(0x2584,0x2585,nColumnWidth,&m_bTrailSpike);
		m_pTrailSpike->SetOnString(LoadTempString(0x35));
		m_pTrailSpike->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pTrailSpike);

		m_pMissile = CreateToggle(0x2586,0x2587,nColumnWidth,&m_bMissile);
		m_pMissile->SetOnString(LoadTempString(0x35));
		m_pMissile->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pMissile);

		m_pTurboCurse = CreateToggle(0x2588,0x2589,nColumnWidth,&m_bTurboCurse);
		m_pTurboCurse->SetOnString(LoadTempString(0x35));
		m_pTurboCurse->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pTurboCurse);

		m_pGrab = CreateToggle(0x258A,0x258B,nColumnWidth,&m_bGrab);
		m_pGrab->SetOnString(LoadTempString(0x35));
		m_pGrab->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pGrab);

		m_pTrailReset = CreateToggle(0x258C,0x258D,nColumnWidth,&m_bTrailReset);
		m_pTrailReset->SetOnString(LoadTempString(0x35));
		m_pTrailReset->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pTrailReset);

		m_pForceUse = CreateToggle(0x258E,0x258F,nColumnWidth,&m_bForceUse);
		m_pForceUse->SetOnString(LoadTempString(0x35));
		m_pForceUse->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pForceUse);

		m_pWallExtender = CreateToggle(0x2576,0x2577,nColumnWidth,&m_bWallExtender);
		m_pWallExtender->SetOnString(LoadTempString(0x35));
		m_pWallExtender->SetOffString(LoadTempString(0x36));
		m_pPowerupList->AddControl(m_pWallExtender);

		int nX = nLeft + nWidth / 2;
		int nY = nBottom - 32;
		m_pPowerupsOK = AddTextItem(0x42,kCmdLCHidePowerups,0xEB,LTIntPt(nX,nY));
		m_pPowerupsOK->SetBasePos(LTIntPt(nX - m_pPowerupsOK->GetWidth() / 2,nY));
		m_pPowerupsOK->Show(LTFALSE);
	}

	return bRet;
}

// RETAIL: 0x100691e0
uint32 CScreenLightCycleGame::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		ShowPowerups(false);
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}

	switch (dwCommand)
	{
	case kCmdLCPowerupItem:
		break;
	case kCmdLCShowPowerups:
		ShowPowerups(true);
		break;
	case kCmdLCHidePowerups:
		ShowPowerups(false);
		break;
	default:
		return CBaseScreenLightCycle::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// RETAIL: 0x10069260: overrides that only call CBaseScreenLightCycle's
LTBOOL CScreenLightCycleGame::OnLeft()
{
	return CBaseScreenLightCycle::OnLeft();
}

// RETAIL: 0x10069270
LTBOOL CScreenLightCycleGame::OnRight()
{
	return CBaseScreenLightCycle::OnRight();
}

// RETAIL: 0x10069280
void CScreenLightCycleGame::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	CBaseScreenLightCycle::HandleCallback(dwParam1,dwParam2);
}

// RETAIL: 0x100692a0
void CScreenLightCycleGame::LoadSettings()
{
	CBaseScreenLightCycle::LoadSettings();

	CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
	m_nMaxAICyclists = pData->m_Custom.nMaxAICyclists;
	m_nOpponentSkill = pData->m_Custom.nOpponentSkill - 1;
	m_nOpponentCycle = pData->m_Custom.nOpponentCycleType;
	m_nGameSpeed = pData->m_Custom.nGameSpeed;
	m_nTrailLength = pData->m_Custom.nTrailLength;
	m_bShield = pData->m_Custom.bShield;
	m_bTurbo = pData->m_Custom.bTurbo;
	m_bTrailSpike = pData->m_Custom.bTrailSpike;
	m_bMissile = pData->m_Custom.bMissile;
	m_bTurboCurse = pData->m_Custom.bTurboCurse;
	m_bGrab = pData->m_Custom.bGrab;
	m_bTrailReset = pData->m_Custom.bTrailReset;
	m_bForceUse = pData->m_Custom.bForceUse;
	m_bWallExtender = pData->m_Custom.bWallExtender;

	UpdateData(LTFALSE);
}

// RETAIL: 0x10069380
void CScreenLightCycleGame::SaveSettings()
{
	UpdateData(LTTRUE);

	CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
	pData->m_Custom.nMaxAICyclists = m_nMaxAICyclists;
	pData->m_Custom.nOpponentSkill = m_nOpponentSkill + 1;
	pData->m_Custom.nOpponentCycleType = m_nOpponentCycle;
	pData->m_Custom.nGameSpeed = m_nGameSpeed;
	pData->m_Custom.nTrailLength = m_nTrailLength;
	pData->m_Custom.nPowerups = 0;
	if (m_bShield) pData->m_Custom.nPowerups |= 0x1;
	if (m_bTurbo) pData->m_Custom.nPowerups |= 0x2;
	if (m_bTrailSpike) pData->m_Custom.nPowerups |= 0x4;
	if (m_bMissile) pData->m_Custom.nPowerups |= 0x8;
	if (m_bTurboCurse) pData->m_Custom.nPowerups |= 0x20;
	if (m_bGrab) pData->m_Custom.nPowerups |= 0x40;
	if (m_bTrailReset) pData->m_Custom.nPowerups |= 0x80;
	if (m_bForceUse) pData->m_Custom.nPowerups |= 0x100;
	if (m_bWallExtender) pData->m_Custom.nPowerups |= 0x10;
	g_pLightCycleGameDataMgr->SaveCurrent();

	CBaseScreenLightCycle::SaveSettings();
}

// RETAIL: 0x10069480: the popup's controls take the input while shown
void CScreenLightCycleGame::ShowPowerups(bool bShow)
{
	m_pPowerupList->Show(bShow);
	if (bShow)
	{
		// RETAIL: CBaseScreen's 0x100d38d0 (out of line; ours inline)
		m_xTronUnknown50.push_back(m_pPowerupList);
		m_xTronUnknown50.push_back(m_pPowerupsOK);
		SetSelection(GetIndex(m_pPowerupList));
	}
	else
	{
		// RETAIL: CBaseScreen's 0x100d3900 (out of line; ours inline)
		m_xTronUnknown50.clear();
		SetSelection(kNoSelection);
	}
	m_pPowerupsOK->Show(bShow);
}

// RETAIL: 0x10069510
void CScreenLightCycleGame::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		ShowPowerups(false);
		UpdatePowerups();
	}
	else
	{
		ShowPowerups(false);
	}
	CBaseScreenLightCycle::OnFocus(bFocus);
}

// RETAIL: 0x10069550: escape closes the popup first
void CScreenLightCycleGame::Escape()
{
	if (m_pPowerupList->IsVisible())
	{
		ShowPowerups(false);
		PlayInterfaceSound(2);
	}
	else
	{
		CBaseScreen::Escape();
	}
}

// RETAIL: 0x10069580. PORT: retail's power-up names run to 11 (a table of
// 12, 0x102162d4); ours to kNumLCPowerupTypes
bool CScreenLightCycleGame::IsPowerupLocked(int nPowerup)
{
	if (g_pCheatMgr->IsCheatActive(kCheatTronUnknown38))
		return false;

	if (nPowerup < 0 || nPowerup >= kNumLCPowerupTypes)
		return true;

	const char *pszName = g_aszLCPowerupNames[nPowerup];
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	StringArray::iterator it;
	for (it = pProfile->m_UnlockedPowerups.begin(); it != pProfile->m_UnlockedPowerups.end(); ++it)
	{
		if (LCNameEquals(*it,pszName))
			break;
	}
	return (it == pProfile->m_UnlockedPowerups.end());
}

// RETAIL: 0x10069620: a power-up newly unlocked is turned on
void CScreenLightCycleGame::UpdatePowerup(CLTGUIToggle *pToggle, LTBOOL *pbValue, uint32 *pnPowerups, uint32 nFlag, bool bUnlocked)
{
	if (bUnlocked)
	{
		pToggle->Enable(LTTRUE);
		if (!(*pnPowerups & nFlag))
		{
			*pbValue = LTTRUE;
			pToggle->SetOnString(LoadTempString(0x35));
			pToggle->SetOffString(LoadTempString(0x36));
			*pnPowerups |= nFlag;
		}
	}
	else
	{
		pToggle->SetOnString(LoadTempString(0x402));
		pToggle->SetOffString(LoadTempString(0x402));
		pToggle->SetOn(LTFALSE);
		pToggle->Enable(LTFALSE);
		*pbValue = LTFALSE;
		*pnPowerups &= ~nFlag;
	}
}

// RETAIL: 0x10069710
void CScreenLightCycleGame::UpdatePowerups()
{
	CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
	uint32 *pnPowerups = &pData->m_Custom.nPowerups;
	UpdatePowerup(m_pShield,&m_bShield,pnPowerups,0x1,!IsPowerupLocked(kLCPowerup_Shield));
	UpdatePowerup(m_pTurbo,&m_bTurbo,pnPowerups,0x2,!IsPowerupLocked(kLCPowerup_Turbo));
	UpdatePowerup(m_pTrailSpike,&m_bTrailSpike,pnPowerups,0x4,!IsPowerupLocked(kLCPowerup_TrailSpike));
	UpdatePowerup(m_pMissile,&m_bMissile,pnPowerups,0x8,!IsPowerupLocked(kLCPowerup_Missile));
	UpdatePowerup(m_pWallExtender,&m_bWallExtender,pnPowerups,0x10,!IsPowerupLocked(kLCPowerup_WallExtender));
	UpdatePowerup(m_pTurboCurse,&m_bTurboCurse,pnPowerups,0x20,!IsPowerupLocked(kLCPowerup_TurboCurse));
	UpdatePowerup(m_pGrab,&m_bGrab,pnPowerups,0x40,!IsPowerupLocked(kLCPowerup_Grab));
	UpdatePowerup(m_pTrailReset,&m_bTrailReset,pnPowerups,0x80,!IsPowerupLocked(kLCPowerup_TrailReset));
	UpdatePowerup(m_pForceUse,&m_bForceUse,pnPowerups,0x100,!IsPowerupLocked(kLCPowerup_ForceUse));
}

// RETAIL: 0x1006d860 (folded with the other light cycle screens')
LTRect CScreenLightCycleGame::GetBodyRect()
{
	return g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"TabBodyRect");
}
