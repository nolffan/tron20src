// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCycleCircuitIntro.cpp
//
// PURPOSE : HUDItem showing a light cycle circuit's name, speed, trail
//           length, wave and lives when a wave starts
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1002c180..
// 0x1002d2a0; HUDLightCycleCircuitIntro.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDLightCycleCircuitIntro.h"
#include "InterfaceMgr.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "TronPlayerMgr.h"
#include "LCHUDHelper.h"
#include "LightCycleMgr.h"
#include "LightCycleGameDataMgr.h"

// RETAIL: 0x1026ee28
CHUDLightCycleCircuitIntro* g_pLightCycleCircuitIntro = LTNULL;

// RETAIL: 0x1002c260
CHUDLightCycleCircuitIntro::CHUDLightCycleCircuitIntro() :
	m_fStartTime(0.0f),
	m_fDuration(0.0f),
	m_fPauseTime(0.0f),
	m_bStarted(false),
	m_pName(LTNULL),
	m_pSpeedLabel(LTNULL),
	m_pTrailLabel(LTNULL),
	m_pWaveLabel(LTNULL),
	m_pLivesLabel(LTNULL),
	m_pSpeed(LTNULL),
	m_pTrail(LTNULL),
	m_pWave(LTNULL),
	m_pLives(LTNULL)
{
	// RETAIL: Tron's update flag for it
	m_UpdateFlags = (eHUDUpdateFlag)0x200000;
	g_pLightCycleCircuitIntro = this;
}

// RETAIL: 0x1002c310
LTBOOL CHUDLightCycleCircuitIntro::Init()
{
	std::wstring sText;
	LTGUIWiden(sText, "");

	TermStrings();

	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);
	CaselessInlineString sFrame = dynamic_cast<CTronLayoutMgr*>(g_pLayoutMgr)->GetLCHUDHelper()->GetIntroTexturedFrameName(nCurrentLayout);
	m_Frame.Init(sFrame.c_str());

	char szText[256];

	m_pName = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pName->SetDropShadow(true);

	FormatString(9698, szText, sizeof(szText));
	LTGUIWiden(sText, szText);
	m_pSpeedLabel = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pSpeedLabel->SetDropShadow(true);

	FormatString(9695, szText, sizeof(szText));
	LTGUIWiden(sText, szText);
	m_pTrailLabel = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pTrailLabel->SetDropShadow(true);

	FormatString(9696, szText, sizeof(szText));
	LTGUIWiden(sText, szText);
	m_pWaveLabel = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pWaveLabel->SetDropShadow(true);

	FormatString(9697, szText, sizeof(szText));
	LTGUIWiden(sText, szText);
	m_pLivesLabel = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pLivesLabel->SetDropShadow(true);

	LTGUIWiden(sText, "");
	m_pSpeed = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pSpeed->SetDropShadow(true);
	m_pTrail = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pTrail->SetDropShadow(true);
	m_pWave = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pWave->SetDropShadow(true);
	m_pLives = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pLives->SetDropShadow(true);

	return LTTRUE;
}

// RETAIL: 0x1002c850
void CHUDLightCycleCircuitIntro::Term()
{
	TermStrings();
	m_Frame.Term();
}

// RETAIL: 0x1002c870
void CHUDLightCycleCircuitIntro::Render()
{
	if (!dynamic_cast<CTronInterfaceMgr*>(g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	if (m_fPauseTime > 0.0f)
	{
		if (m_fPauseTime - m_fStartTime >= m_fDuration) return;
	}
	else
	{
		if (g_pLTClient->GetTime() - m_fStartTime >= m_fDuration) return;
	}

	SetRenderState();

	m_Frame.Render();
	m_pName->Render();
	m_pSpeedLabel->Render();
	m_pTrailLabel->Render();
	m_pWaveLabel->Render();
	m_pLivesLabel->Render();
	m_pSpeed->Render();
	m_pTrail->Render();
	m_pWave->Render();
	m_pLives->Render();
}

// RETAIL: 0x1002c990
void CHUDLightCycleCircuitIntro::Update()
{
	UpdateText();
	UpdateLayout();
}

// RETAIL: 0x1002c9b0
void CHUDLightCycleCircuitIntro::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);

	LTFLOAT fXRatio = g_pInterfaceResMgr->GetXRatio();
	LTFLOAT fYRatio = g_pInterfaceResMgr->GetYRatio();
	int nMaxWidth = 0;

	LCHUDHelper* pHelper = dynamic_cast<CTronLayoutMgr*>(g_pLayoutMgr)->GetLCHUDHelper();

	// RETAIL: 0x1002ca0c..0x1002cb0c: the intro's own layout values
	LTIntPt pos = pHelper->GetIntroMasterPos(nCurrentLayout);
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: on the centred page
	int x = (int)g_pInterfaceResMgr->PageX((LTFLOAT)pos.x);
	int y = (int)g_pInterfaceResMgr->PageY((LTFLOAT)pos.y);
#else
	int x = (int)(pos.x * fXRatio);
	int y = (int)(pos.y * fYRatio);
#endif


	int nNameOffset = (int)(pHelper->GetIntroVFrameBorder(nCurrentLayout) * fYRatio);
	int nHBorder = (int)(pHelper->GetIntroHFrameBorder(nCurrentLayout) * fXRatio);
	int nNameHeight = (int)(pHelper->GetIntroTitleHeight(nCurrentLayout) * fYRatio);
	int nTextHeight = (int)(pHelper->GetIntroEntryHeight(nCurrentLayout) * fYRatio);
	int nVBorder = (int)(pHelper->GetIntroTitleSpace(nCurrentLayout) * fYRatio);
	int nLineGap = (int)(pHelper->GetIntroEntrySpace(nCurrentLayout) * fYRatio);
	int nLabelOffset = (int)(pHelper->GetIntroHeaderOffset(nCurrentLayout) * fXRatio);
	int nValueOffset = (int)(pHelper->GetIntroValueOffset(nCurrentLayout) * fXRatio);

	m_pName->SetFont(g_pInterfaceResMgr->GetFontForHeight(nNameHeight));
	m_pName->SetCharScreenHeight(nNameHeight);
	int nLineY = nNameOffset + y;
	m_pName->SetPosition(x - m_pName->GetWidth() / 2, nLineY);

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nTextHeight);

	m_pSpeedLabel->SetFont(pFont);
	m_pSpeed->SetFont(pFont);
	m_pSpeedLabel->SetCharScreenHeight(nTextHeight);
	m_pSpeed->SetCharScreenHeight(nTextHeight);
	nMaxWidth = LTMAX(nMaxWidth, (int)m_pSpeed->GetWidth());
	nLineY += nLineGap + nTextHeight;
	int nLabelX = nLabelOffset + x;
	int nValueX = nValueOffset + x;
	m_pSpeedLabel->SetPosition(nLabelX, nLineY);
	m_pSpeed->SetPosition(nValueX, nLineY);

	m_pTrailLabel->SetFont(pFont);
	m_pTrail->SetFont(pFont);
	m_pTrailLabel->SetCharScreenHeight(nTextHeight);
	m_pTrail->SetCharScreenHeight(nTextHeight);
	nMaxWidth = LTMAX(nMaxWidth, (int)m_pTrail->GetWidth());
	nLineY += nLineGap + nTextHeight;
	m_pTrailLabel->SetPosition(nLabelX, nLineY);
	m_pTrail->SetPosition(nValueX, nLineY);

	m_pWaveLabel->SetFont(pFont);
	m_pWave->SetFont(pFont);
	m_pWaveLabel->SetCharScreenHeight(nTextHeight);
	m_pWave->SetCharScreenHeight(nTextHeight);
	nMaxWidth = LTMAX(nMaxWidth, (int)m_pWave->GetWidth());
	nLineY += nLineGap + nTextHeight;
	m_pWaveLabel->SetPosition(nLabelX, nLineY);
	m_pWave->SetPosition(nValueX, nLineY);

	m_pLivesLabel->SetFont(pFont);
	m_pLives->SetFont(pFont);
	m_pLivesLabel->SetCharScreenHeight(nTextHeight);
	m_pLives->SetCharScreenHeight(nTextHeight);
	nMaxWidth = LTMAX(nMaxWidth, (int)m_pLives->GetWidth());
	nLineY += nLineGap + nTextHeight;
	m_pLivesLabel->SetPosition(nLabelX, nLineY);
	m_pLives->SetPosition(nValueX, nLineY);

	int nWidth = LTMAX((nHBorder - nLabelOffset) * 2, (nValueOffset + nHBorder + nMaxWidth) * 2);
	nWidth = LTMAX(nWidth, (int)m_pName->GetWidth() + nHBorder * 2);

	int nLeft = x - nWidth / 2;
	m_Frame.SetRect(LTRect(nLeft, y, nLeft + nWidth,
		y + nNameHeight + nVBorder + (nNameOffset + (nLineGap + nTextHeight) * 2) * 2));
}

// RETAIL: 0x1002cdf0
void CHUDLightCycleCircuitIntro::Start()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);
	m_fDuration = dynamic_cast<CTronLayoutMgr*>(g_pLayoutMgr)->GetLCHUDHelper()->GetIntroValueTime(nCurrentLayout);
	m_fStartTime = g_pLTClient->GetTime();
	m_bStarted = true;
}

// RETAIL: 0x1002ce50. RETAIL_BUGS.md: it destroys the trail strings twice
// and the lives strings never
void CHUDLightCycleCircuitIntro::TermStrings()
{
	DestroyString(&m_pName);
	DestroyString(&m_pSpeedLabel);
	DestroyString(&m_pTrailLabel);
	DestroyString(&m_pWaveLabel);
#ifdef TRON20SRC_MODERN
	// MODERN.md GAME-123: the lives strings, not the trail strings again
	DestroyString(&m_pLivesLabel);
#else
	DestroyString(&m_pTrailLabel);
#endif
	DestroyString(&m_pSpeed);
	DestroyString(&m_pTrail);
	DestroyString(&m_pWave);
#ifdef TRON20SRC_MODERN
	DestroyString(&m_pLives);
#else
	DestroyString(&m_pTrail);
#endif
}

// RETAIL: 0x1002ced0
void CHUDLightCycleCircuitIntro::DestroyString(CUIFormattedPolyString** ppStr)
{
	if (*ppStr)
	{
		g_pFontManager->DestroyPolyString(*ppStr);
		*ppStr = LTNULL;
	}
}

// RETAIL: 0x1002cf00
void CHUDLightCycleCircuitIntro::UpdateText()
{
	CLightCycleGameData* pData = g_pLightCycleGameDataMgr->GetCurrent();

	std::wstring sText;
	char szText[80];

	FormatString(pData->m_Circuit.nNameId, szText, sizeof(szText));
	LTGUIWiden(sText, szText);
	m_pName->SetText(sText);

	// RETAIL_BUGS.md: an unknown index leaves the string's id unset
#ifdef TRON20SRC_MODERN
	// MODERN.md GAME-124: an unknown index gives the first string
	int nStringId = 9664;
#else
	int nStringId;
#endif
	switch (g_pLightCycleMgr->GetGameSpeedIndex())
	{
		case 0: nStringId = 9664; break;
		case 1: nStringId = 9665; break;
		case 2: nStringId = 9666; break;
		case 3: nStringId = 9667; break;
	}
	FormatString(nStringId, szText, sizeof(szText));
	LTGUIWiden(sText, szText);
	m_pSpeed->SetText(sText);

#ifdef TRON20SRC_MODERN
	nStringId = 9668;	// MODERN.md GAME-124
#endif
	switch (g_pLightCycleMgr->GetTrailLengthIndex())
	{
		case 0: nStringId = 9668; break;
		case 1: nStringId = 9669; break;
		case 2: nStringId = 9670; break;
		case 3: nStringId = 9671; break;
	}
	FormatString(nStringId, szText, sizeof(szText));
	LTGUIWiden(sText, szText);
	m_pTrail->SetText(sText);

	sprintf(szText, "%d/%d", g_pLightCycleMgr->GetTronUnknown1338() + 1, pData->m_Circuit.aWaves.size());
	LTGUIWiden(sText, szText);
	m_pWave->SetText(sText);

	sprintf(szText, "%d/%d", (uint8)dynamic_cast<CTronPlayerMgr*>(g_pPlayerMgr)->TronUnknownVirtual88(), pData->m_Circuit.nInitialLives);
	LTGUIWiden(sText, szText);
	m_pLives->SetText(sText);
}
