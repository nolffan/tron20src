// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProcedurals.cpp
//
// PURPOSE : HUDItem showing the procedurals at work
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (HUDProcedurals.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDProcedurals.h"
#include "TronLayoutMgr.h"
#include "CommandIDs.h"
#include "InterfaceMgr.h"
#include "SubroutineMgr.h"
#include "ClientUtilities.h"

// RETAIL: the layout manager as a CTronLayoutMgr, without the dynamic cast
#define g_pTronLayoutMgrStatic ((CTronLayoutMgr*)g_pLayoutMgr)

#define PROCHUD_TAG		"ProceduralHUD"

// RETAIL: 0x10031b10
CHUDProcedurals::CHUDProcedurals()
{
	m_UpdateFlags = kHUDProcedurals;
	m_fScale = 0.0f;
	m_bShow = true;
	m_nNumActive = 0;
	m_nTop = 0;
	m_hFrameTex = LTNULL;
	m_nSpacing = 0;
	for (int i = 0; i < HUDPROC_MAX; i++)
	{
		m_pName[i] = LTNULL;
		m_hIconTex[i] = LTNULL;
	}
	m_nIconWidth = 0;
	m_nIconHeight = 0;
	m_nIconOffset = 0;
	m_nPBarXOffset = 0;
	m_nPBarYOffset = 0;
	m_nPBarWidth = 0;
	m_nPBarHeight = 0;
	m_nStringXOffset = 0;
	m_nStringYOffset = 0;
}

// RETAIL: 0x10031c30
void CHUDProcedurals::Term()
{
	for (int i = 0; i < HUDPROC_MAX; i++)
	{
		if (m_pName[i])
		{
			g_pFontManager->DestroyPolyString(m_pName[i]);
			m_pName[i] = LTNULL;
		}
	}
}

// RETAIL: 0x10031c60: single player only; the names show while the
// progress command is on
void CHUDProcedurals::Render()
{
	if (IsMultiplayerGame()) return;
	if (!IsEnabled() || !m_bShow || !m_nNumActive) return;

	bool bNames = g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_PROGRESS);

	SetRenderState();

	for (int i = 0; i < HUDPROC_MAX; i++)
	{
		if (!m_bActive[i]) continue;

		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

		g_pDrawPrim->SetTexture(m_hFrameTex);
		g_pDrawPrim->DrawPrim(&m_Frame[i], 1);

		g_pDrawPrim->SetTexture(m_hIconTex[i]);
		g_pDrawPrim->DrawPrim(&m_Icon[i], 1);

		g_pDrawPrim->SetTexture(m_hEmptyBarTex);
		g_pDrawPrim->DrawPrim(&m_EmptyBar[i], 1);

		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_ADD);
		g_pDrawPrim->SetTexture(m_hFullBarTex);
		g_pDrawPrim->DrawPrim(&m_FullBar[i], 1);

		if (bNames && m_pName[i])
		{
			m_pName[i]->Render();
		}
	}
}

// RETAIL: 0x10031db0: a row for each procedural working on a subroutine,
// down from the top bound at the right of the screen. Near: Tron's
// Procedural has its name id at +0x24, the icon's texture at +0xf4 and the
// subroutine at +0x138 (the tree's is build 69's), and the subroutine
// manager's procedurals are at +0x1134
void CHUDProcedurals::Update()
{
	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	m_nTop = m_nTopBound;
	m_nNumActive = 0;

	for (int i = 0; i < HUDPROC_MAX; i++)
	{
		Procedural* pProc = g_pSubroutineMgr->GetProcedural(i);
		m_bActive[i] = (pProc->pSub != LTNULL);
		if (!pProc->pSub) continue;

		m_fProgress[i] = pProc->pSub->fPercentDone;
		m_hIconTex[i] = g_pInterfaceResMgr->GetTexture(pProc->szIdleSkin);

		std::wstring sName;
		sName = LoadTempWString(pProc->nNameId);

		if (m_pName[i])
		{
			m_pName[i]->SetText(sName);
		}
		else
		{
			m_pName[i] = g_pFontManager->CreatePolyString(g_pInterfaceResMgr->GetFontForHeight(m_nFontSize), sName);
			m_pName[i]->SetCharScreenHeight(m_nFontSize);
			m_pName[i]->SetColor(m_nColor);
		}

		LTFLOAT fY = (LTFLOAT)m_nTop;
		LTFLOAT fX = (LTFLOAT)(g_pInterfaceResMgr->GetScreenWidth() - m_nIconOffset);
		LTFLOAT fW = (LTFLOAT)m_nIconWidth;
		LTFLOAT fH = (LTFLOAT)m_nIconHeight;
		g_pDrawPrim->SetXYWH(&m_Icon[i], fX, fY, fW, fH);
		g_pDrawPrim->SetXYWH(&m_Frame[i], fX, fY, fW, fH);

		fX = fX - m_nPBarXOffset;
		fY = fY - m_nPBarYOffset;
		fW = (LTFLOAT)m_nPBarWidth;
		fH = (LTFLOAT)m_nPBarHeight;
		g_pDrawPrim->SetXYWH(&m_EmptyBar[i], fX, fY, fW, fH);
		fW = fW * m_fProgress[i];
		g_pDrawPrim->SetXYWH(&m_FullBar[i], fX, fY, fW, fH);

		int32 nWidth, nHeight;
		m_pName[i]->GetDims(&nWidth, &nHeight);
		m_pName[i]->SetPosition((int)fX - m_nStringXOffset - nWidth, (int)fY + (-(nHeight / 2) - m_nStringYOffset));

		m_nTop += m_nSpacing;
		m_nNumActive++;
	}
}

// RETAIL: 0x10032190
void CHUDProcedurals::UpdateLayout()
{
	char szTemp[128];

	g_pLayoutMgr->GetString(PROCHUD_TAG, "FrameTex", szTemp, sizeof(szTemp));
	m_hFrameTex = g_pInterfaceResMgr->GetTexture(szTemp);
	g_pLayoutMgr->GetString(PROCHUD_TAG, "EmptyBarTex", szTemp, sizeof(szTemp));
	m_hEmptyBarTex = g_pInterfaceResMgr->GetTexture(szTemp);
	g_pLayoutMgr->GetString(PROCHUD_TAG, "FullBarTex", szTemp, sizeof(szTemp));
	m_hFullBarTex = g_pInterfaceResMgr->GetTexture(szTemp);

	m_nTopBound = g_pLayoutMgr->GetInt(PROCHUD_TAG, "TopBound", 60);

	m_nFont = g_pTronLayoutMgrStatic->GetProgressProceduralFont();
	m_nFontSize = g_pTronLayoutMgrStatic->GetProgressProceduralFontSize();
	m_nColor = g_pTronLayoutMgrStatic->GetProgressProceduralColor();
	m_nSpacing = g_pTronLayoutMgrStatic->GetProgressProceduralSpacing();

	m_nIconWidth = g_pLayoutMgr->GetInt(PROCHUD_TAG, "IconWidth", 32);
	m_nIconHeight = g_pLayoutMgr->GetInt(PROCHUD_TAG, "IconHeight", 32);
	m_nIconOffset = g_pLayoutMgr->GetInt(PROCHUD_TAG, "IconOffset", 42);
	m_nPBarXOffset = g_pLayoutMgr->GetInt(PROCHUD_TAG, "PBarXOffset", 68);
	m_nPBarYOffset = g_pLayoutMgr->GetInt(PROCHUD_TAG, "PBarYOffset", 0);
	m_nPBarWidth = g_pLayoutMgr->GetInt(PROCHUD_TAG, "PBarWidth", 64);
	m_nPBarHeight = g_pLayoutMgr->GetInt(PROCHUD_TAG, "PBarHeight", 32);
	m_nStringXOffset = g_pLayoutMgr->GetInt(PROCHUD_TAG, "StringXOffset", 10);
	m_nStringYOffset = g_pLayoutMgr->GetInt(PROCHUD_TAG, "StringYOffset", 16);

	for (int i = 0; i < HUDPROC_MAX; i++)
	{
		g_pDrawPrim->SetUVWH(&m_Icon[i], 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetUVWH(&m_Frame[i], 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetUVWH(&m_EmptyBar[i], 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetUVWH(&m_FullBar[i], 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetRGBA(&m_Icon[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_Frame[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_EmptyBar[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_FullBar[i], argbWhite);
	}
}
