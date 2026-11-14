// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEndMission.cpp
//
// PURPOSE : Interface screen for handling end of mission 
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenEndMission.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ScreenPreload.h"
#include "TronInterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"

namespace
{
	float g_fDuration = 0.0f;
	float g_fMinDelay = 3.0f;
	float g_fDelay = 30.0f;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenEndMission::CScreenEndMission()
{
	m_pMission = NULL;
	m_pAccuracy = NULL;
	m_pMarksmanship = NULL;
	m_pNonInjury = NULL;
	m_pTimesHit = NULL;
	m_pList = NULL;
}

CScreenEndMission::~CScreenEndMission()
{

}


// RETAIL: 0x1005a280. Tron's string IDs: 0x11 the title, 0x1D7 undamaged
LTBOOL CScreenEndMission::Build()
{
	CreateTitle(0x11);

	uint8 statSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_END_MISSION,"StatSize");
	if (!statSize) statSize = 12;
	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight(statSize);

	m_pMission = AddTextItem(std::wstring(),LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_nextPos.y += 8;

	m_pList = AddList(m_nextPos,(GetPageBottom() - m_nextPos.y));

	m_pAccuracy = CreateTextItem(std::wstring(),LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_pAccuracy->SetFont(pFont,statSize);
	m_pList->AddControl(m_pAccuracy);

	m_pMarksmanship = CreateTextItem(std::wstring(),LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_pMarksmanship->SetFont(pFont,statSize);
	m_pList->AddControl(m_pMarksmanship);

	m_pNonInjury = CreateTextItem(0x1D7,LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_pNonInjury->SetFont(pFont,statSize);
	m_pList->AddControl(m_pNonInjury);

	m_pTimesHit = CreateTextItem(std::wstring(),LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_pTimesHit->SetFont(pFont,statSize);
	m_pList->AddControl(m_pTimesHit);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);
	return LTTRUE;
}


// RETAIL: 0x1005a4a0. Tron's string IDs: 0x1E9 "Mission", 0x1DA accuracy,
// 0x1D9 marksmanship, 0x1D8 times hit
void CScreenEndMission::OnFocus(LTBOOL bFocus)
{
	std::wstring sText;

	if (bFocus)
	{
		g_fDuration = 0.0f;
		char szString[256] = "";

		int nMission = g_pMissionMgr->GetCurrentMission();
		MISSION* pMission = g_pMissionButeMgr->GetMission(nMission);

		if (pMission)
		{
			sText = LoadTempWString(0x1E9);
			sText += L" ";
			sText += LoadTempWString(pMission->nNameId);
			m_pMission->SetString(sText);
		}
		else
		{
			m_pMission->SetString(std::wstring());
		}

		const MissionStats *pStats = g_pPlayerStats->GetMissionStats();

		if (pStats)
		{
			if (pStats->dwNumShotsFired)
			{
				float fAccuracy = 100.0f * (float)pStats->dwNumHits / (float) pStats->dwNumShotsFired;
				FormatString(0x1DA,szString,sizeof(szString),pStats->dwNumHits,pStats->dwNumShotsFired,(int)fAccuracy);
			}
			else
			{
				FormatString(0x1DA,szString,sizeof(szString),0,0,0);
			}
			LTGUIWiden(sText,szString);
			m_pAccuracy->SetString(sText);

			LTGUIWiden(sText,FormatTempString(0x1D9,pStats->dwHitLocations[HL_HEAD]));
			m_pMarksmanship->SetString(sText);

			m_pNonInjury->Show(g_pPlayerStats->GetMissionDamage() == 0);

			LTGUIWiden(sText,FormatTempString(0x1D8,pStats->dwNumTimesHit));
			m_pTimesHit->SetString(sText);

			m_pList->SetStartIndex(0);
		}
	}
	CBaseScreen::OnFocus(bFocus);
}


// RETAIL: 0x1005a970
void CScreenEndMission::Escape()
{

	OnFocus(LTFALSE);

	if (g_pMissionMgr->IsGameOver())
	{
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
	}
	else
	{
		CScreenPreload *pPreload = (CScreenPreload *) (g_pInterfaceMgr->GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
		if (pPreload)
		{
			pPreload->SetWaitingToExit(true);
		}
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);
	}

}

LTBOOL CScreenEndMission::HandleKeyDown(int key, int rep)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;

}
LTBOOL CScreenEndMission::OnLButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}
LTBOOL CScreenEndMission::OnRButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}


LTBOOL CScreenEndMission::Render(HSURFACE hDestSurf)
{
	CBaseScreen::Render(hDestSurf);
	g_fDuration += g_pGameClientShell->GetFrameTime();
	if (g_fDuration >= g_fDelay)
		Escape();
	return LTTRUE;

}




