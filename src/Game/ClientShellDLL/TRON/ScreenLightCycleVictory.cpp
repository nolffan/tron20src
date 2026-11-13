// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCycleVictory.cpp
//
// PURPOSE : Interface screen shown when a light cycle game is won
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (CShell.dll 0x1006de80..0x1006e420), written from the retail
// code. Its screen ID (0x27) and string ID are Tron's numbers.
#include "stdafx.h"
#include "ScreenLightCycleVictory.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ScreenPreload.h"
#include "TronInterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientMultiplayerMgr.h"

namespace
{
	const eScreenID kScreenID = SCREEN_ID_LIGHTCYCLE_VICTORY;

	float g_fDuration = 0.0f;
	float g_fMinDelay = 3.0f;
	float g_fDelay = 30.0f;
	LTRect	stringRect(20,240,620,400);
	uint8	stringSize = 24;
	LTIntPt helpPos(160,440);
	uint8	helpSize = 12;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// RETAIL: 0x1006df60
CScreenLightCycleVictory::CScreenLightCycleVictory()
{
	m_pString = LTNULL;
}

// RETAIL: 0x1006dfa0
CScreenLightCycleVictory::~CScreenLightCycleVictory()
{
	if (m_pString)
	{
		g_pFontManager->DestroyPolyString(m_pString);
		m_pString = LTNULL;
	}
}

// Build the screen
// RETAIL: 0x1006e010
LTBOOL CScreenLightCycleVictory::Build()
{
	CreateTitle(0);

	if (g_pLayoutMgr->HasCustomValue(kScreenID,"VictoryStringRect"))
	{
		stringRect = g_pLayoutMgr->GetScreenCustomRect(kScreenID,"VictoryStringRect");
	}
	if (g_pLayoutMgr->HasCustomValue(kScreenID,"VictoryStringSize"))
	{
		stringSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(kScreenID,"VictoryStringSize");
	}
	if (g_pLayoutMgr->HasCustomValue(kScreenID,"HelpStringPos"))
	{
		helpPos = g_pLayoutMgr->GetScreenCustomPoint(kScreenID,"HelpStringPos");
	}
	if (g_pLayoutMgr->HasCustomValue(kScreenID,"HelpStringSize"))
	{
		helpSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(kScreenID,"HelpStringSize");
	}

	int nFontSize = (int)(g_pInterfaceResMgr->GetXRatio() * (float)stringSize);
	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nFontSize);

	if (!m_pString)
	{
		int nStringID = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"VictoryId");
		m_pString = g_pFontManager->CreateFormattedPolyString(pFont,LoadTempWString(nStringID),0.0f,0.0f,CUI_HALIGN_LEFT);
	}
	m_pString->SetCharScreenHeight(nFontSize);
	m_pString->SetColor(argbWhite);

	if (g_pLayoutMgr->HasCustomValue(kScreenID,"Delay"))
	{
		g_fDelay = g_pLayoutMgr->GetScreenCustomFloat(kScreenID,"Delay");
	}

	ClientMultiplayerMgr *pMPMgr = g_pClientMultiplayerMgr;
	if (!g_pLTClient->IsConnected() || pMPMgr->GetStartGameRequest( ).m_Type != STARTGAME_CLIENTTCP)
	{
		CLTGUITextCtrl* pString = AddTextItem(0xA02,LTNULL,LTNULL,helpPos,LTTRUE);
		if (pString)
		{
			CUIFont* pHelpFont = g_pInterfaceResMgr->GetFontForHeight(helpSize);
			pString->SetFont(pHelpFont,helpSize);
		}
	}

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE,LTFALSE,LTFALSE);
	return LTTRUE;
}

// RETAIL: 0x1006e2c0
void CScreenLightCycleVictory::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		g_fDuration = 0.0f;

		// RETAIL: the ratio is CInterfaceResMgr +0x44 (one before m_fXRatio in
		// retail; not identified)
		float fRatio = g_pInterfaceResMgr->GetXRatio();
		int x = (int)((float)((stringRect.left + stringRect.right) / 2) * fRatio);
		int y = (int)((float)stringRect.top * fRatio);

		int32 nWidth, nHeight;
		m_pString->GetDims(&nWidth, &nHeight);
		m_pString->SetPosition(x - nWidth / 2, y);
	}

	CBaseScreen::OnFocus(bFocus);
}

// RETAIL: 0x1006e360 (folded with CScreenEndMission's, as are the two below)
LTBOOL CScreenLightCycleVictory::HandleKeyDown(int key, int rep)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}

LTBOOL CScreenLightCycleVictory::OnLButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}

LTBOOL CScreenLightCycleVictory::OnRButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}

// RETAIL: 0x1006e390
LTBOOL CScreenLightCycleVictory::Render(HSURFACE hDestSurf)
{
	CBaseScreen::Render(hDestSurf);

	if (m_pString)
		m_pString->Render();

	ClientMultiplayerMgr *pMPMgr = g_pClientMultiplayerMgr;
	g_fDuration += g_pGameClientShell->GetFrameTime();

	if (!g_pLTClient->IsConnected() || pMPMgr->GetStartGameRequest( ).m_Type != STARTGAME_CLIENTTCP)
	{
		if (g_fDuration >= g_fDelay)
			Escape();
	}

	return LTTRUE;
}
