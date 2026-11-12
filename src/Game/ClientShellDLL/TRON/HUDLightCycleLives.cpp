// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCycleLives.cpp
//
// PURPOSE : HUDItem showing the light cycle lives left (n/m)
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (HUDLightCycleLives.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDLightCycleLives.h"
#include "InterfaceMgr.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "LCHUDHelper.h"
#include "ltquaduvutils.h"

CHUDLightCycleLives::CHUDLightCycleLives() :
	m_hBack(LTNULL),
	m_pText(LTNULL),
	m_fAlpha(1.0f),
	m_nFlashCount(0),
	m_nFlashDir(-1)
{
	// RETAIL: Tron's update flag for it (0x20)
	m_UpdateFlags = (eHUDUpdateFlag)0x20;
}

// RETAIL: two constants of this TU (retail 0x101e6ec8, 0x101e6ecc: Render
// reads them from memory). The names are ours
static const LTFLOAT s_fMinAlpha = 0.0f;
static const LTFLOAT s_fFlashTime = 0.5f;

// RETAIL: 0x1002d360
LTBOOL CHUDLightCycleLives::Init()
{
	m_hBack = g_pInterfaceResMgr->GetTexture("interface\\hud\\armor_bg.dtx");
	g_pDrawPrim->SetRGBA(&m_Poly, argbWhite);
	SetupQuadUVs(m_Poly, m_hBack, 0.0f, 0.0f, 1.0f, 1.0f);

	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}

	m_pText = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), std::wstring(L"SuperDano"), 0.0f, 0.0f, CUI_HALIGN_RIGHT);
	m_pText->SetDropShadow(true);

	return LTTRUE;
}

// RETAIL: 0x1002d520
void CHUDLightCycleLives::Term()
{
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}
}

// RETAIL: 0x1002d560: near only for CTronInterfaceMgr's layout (+0x28b14)
void CHUDLightCycleLives::Render()
{
	if (!dynamic_cast<CTronInterfaceMgr*>(g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	SetRenderState();

	uint8 nAlpha = 0xFF;
	if (m_nFlashCount > 0)
	{
		int nFlashDir = m_nFlashDir;
		m_fAlpha += g_pLTClient->GetFrameTime() / s_fFlashTime * nFlashDir;
		if (m_fAlpha > 1.0f || m_fAlpha < s_fMinAlpha)
		{
			m_fAlpha = LTMIN(1.0f, LTMAX(m_fAlpha, s_fMinAlpha));
			m_nFlashDir = -nFlashDir;
			m_nFlashCount--;
		}
		nAlpha = (uint8)(m_fAlpha * 255.0f);
	}

	g_pDrawPrim->SetALPHA(&m_Poly, nAlpha);
	g_pDrawPrim->SetTexture(m_hBack);
	g_pDrawPrim->DrawPrim(&m_Poly, 1);

	m_pText->Render();
}

// RETAIL: 0x1002d690
void CHUDLightCycleLives::Update()
{
	UpdateText();
	UpdateIcon();
	UpdateLayout();
}

// RETAIL: 0x1002d6b0: near only for CTronLayoutMgr's layout (its helper at
// +0x116c)
void CHUDLightCycleLives::UpdateLayout()
{
	uint32 nWidth, nHeight;
	g_pTexInterface->GetTextureDims(m_hBack, nWidth, nHeight);
	LTFLOAT fWidth = (LTFLOAT)nWidth;
	LTFLOAT fHeight = (LTFLOAT)nHeight;

	LTFLOAT fXRatio = g_pInterfaceResMgr->GetXRatio();
	LTFLOAT fYRatio = g_pInterfaceResMgr->GetYRatio();

	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);

	LTIntPt pos = g_pTronLayoutMgr->GetLCHUDHelper()->GetLivesIconPos(nCurrentLayout);
	LTFLOAT fX = (LTFLOAT)pos.x * fXRatio - fWidth;
	LTFLOAT fY = (LTFLOAT)pos.y * fYRatio;
	g_pDrawPrim->SetXYWH(&m_Poly, fX, fY, fWidth, fHeight);

	uint8 nTextSize = (uint8)(g_pTronLayoutMgr->GetLCHUDHelper()->GetLivesTextHeight(nCurrentLayout) * fYRatio);
	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nTextSize);
	m_pText->SetFont(pFont);
	m_pText->SetCharScreenHeight(nTextSize);

	int nOffset = (int)(g_pTronLayoutMgr->GetLCHUDHelper()->GetLivesTextOffset(nCurrentLayout) * fXRatio);
	int nX = (int)fX - nOffset;
	m_pText->SetPosition(nX, (int)(fY - m_pText->GetHeight() * 0.5f + fHeight * 0.5f));
}

// RETAIL: 0x1002d890
void CHUDLightCycleLives::UpdateText()
{
	// PORT: stand-in. Retail writes "%d/%d" into the text, as a wide string:
	// the lives left (CTronPlayerMgr's virtual at +0x88) of the current
	// light cycle game data's lives (its +0x84; the game data manager,
	// retail 0x10046f40.., is not in the tree yet)
}

// RETAIL: 0x1002da30
void CHUDLightCycleLives::UpdateIcon()
{
	// PORT: stand-in. Retail finds the player's light cycle (by the move
	// manager's object, in the client's light cycle manager, not in the
	// tree yet) and shows its color's HUDLivesIconTex
	// (g_pLightCycleButeMgr->GetCycleColor(name)->sHUDLivesIconTex)
}
