// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSubChooser.cpp
//
// PURPOSE : HUDItem showing the utility subroutines the player can use
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (HUDSubChooser.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDSubChooser.h"
#include "TronInterfaceMgr.h"
#include "SubroutineMgr.h"
#include "CommandIDs.h"
#include "ClientUtilities.h"

#define SUBCHOOSER_TAG	"SubroutineChooser"

// RETAIL: 0x1003b240
CHUDSubChooser::CHUDSubChooser()
{
	m_UpdateFlags = 0x00080000;	// RETAIL: Tron's flag for it
	m_fScale = 0.0f;
	m_nScreenWidth = 0;
	for (int i = 0; i < HUDSUBCHOOSER_MAX; i++)
	{
		m_pText[i] = LTNULL;
	}
	m_nTronUnknownEB4 = 0;
	m_bShow = true;
	m_nNumSubs = 0;
}

// RETAIL: 0x1003b360
CHUDSubChooser::~CHUDSubChooser()
{
	Term();
}

// RETAIL: 0x1003b3b0. Near: Tron's TronSubroutine has its function at
// +0x2c and its icon's name at +0x70, and the subroutine manager its
// subroutines at +0x111c (the tree's are build 69's)
LTBOOL CHUDSubChooser::Init()
{
	m_nFontSize = g_pLayoutMgr->GetInt(SUBCHOOSER_TAG, "FontSize", 20);
	m_nTextColor = g_pLayoutMgr->GetChooserTextColor(0, 0xFFFFFFFF);
	m_BoxSize = g_pLayoutMgr->GetPoint(SUBCHOOSER_TAG, "BoxSize");
	m_CursorSize = g_pLayoutMgr->GetPoint(SUBCHOOSER_TAG, "CursorSize");
	m_nSpacing = g_pLayoutMgr->GetInt(SUBCHOOSER_TAG, "Spacing", 30);

	char szTemp[1024];
	uint32 nBackWidth, nBackHeight;
	g_pLayoutMgr->GetString(SUBCHOOSER_TAG, "BackgroundTex", szTemp, sizeof(szTemp));
	m_hBackTex = g_pInterfaceResMgr->GetTexture(szTemp);
	g_pTexInterface->GetTextureDims(m_hBackTex, nBackWidth, nBackHeight);
	LTRect rcBack = g_pLayoutMgr->GetRect(SUBCHOOSER_TAG, "BackgroundTexUV");

	uint32 nBadWidth, nBadHeight;
	g_pLayoutMgr->GetString(SUBCHOOSER_TAG, "BadBackgroundTex", szTemp, sizeof(szTemp));
	m_hBadBackTex = g_pInterfaceResMgr->GetTexture(szTemp);
	g_pTexInterface->GetTextureDims(m_hBadBackTex, nBadWidth, nBadHeight);
	LTRect rcBad = g_pLayoutMgr->GetRect(SUBCHOOSER_TAG, "BadBackgroundTexUV");

	m_nNumSubs = 0;
	uint16 nNumSubs = g_pSubroutineMgr->GetNumSubroutines();
	for (uint16 i = 0; i < nNumSubs; i++)
	{
		TronSubroutine* pSub = g_pSubroutineMgr->GetSubroutineByIndex(i);
		if (pSub->eFunction != FUNCTION_UTILITY) continue;

		m_pSubs[m_nNumSubs] = pSub;
		m_bAvailable[m_nNumSubs] = false;

		char szIcon[1024];
		sprintf(szIcon, "%s_on.dtx", pSub->szSprite);
		m_hIconTex[m_nNumSubs] = g_pInterfaceResMgr->GetTexture(szIcon);

		g_pDrawPrim->SetUVWH(&m_Icon[m_nNumSubs], 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetRGBA(&m_Icon[m_nNumSubs], argbWhite);

		g_pDrawPrim->SetUVWH(&m_Back[m_nNumSubs],
			(float)rcBack.left / (float)nBackWidth, (float)rcBack.top / (float)nBackHeight,
			(float)(rcBack.right - rcBack.left) / (float)nBackWidth, (float)(rcBack.bottom - rcBack.top) / (float)nBackHeight);
		g_pDrawPrim->SetRGBA(&m_Back[m_nNumSubs], argbWhite);

		g_pDrawPrim->SetUVWH(&m_BadBack[m_nNumSubs],
			(float)rcBad.left / (float)nBadWidth, (float)rcBad.top / (float)nBadHeight,
			(float)(rcBad.right - rcBad.left) / (float)nBadWidth, (float)(rcBad.bottom - rcBad.top) / (float)nBadHeight);
		g_pDrawPrim->SetRGBA(&m_BadBack[m_nNumSubs], argbWhite);

		m_nNumSubs++;
	}

	uint8 nSize = (uint8)(m_nFontSize * g_pInterfaceResMgr->GetYRatio());
	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nSize);
	for (int j = 0; j < HUDSUBCHOOSER_MAX; j++)
	{
		m_pText[j] = g_pFontManager->CreateFormattedPolyString(pFont, std::wstring(L""), 0.0f, 0.0f, CUI_HALIGN_LEFT);
		m_pText[j]->SetDropShadow(true);
		m_pText[j]->SetColor(m_nTextColor);
	}

	return LTTRUE;
}

// RETAIL: 0x1003b960
void CHUDSubChooser::Term()
{
	for (int i = 0; i < HUDSUBCHOOSER_MAX; i++)
	{
		if (m_pText[i])
		{
			g_pFontManager->DestroyPolyString(m_pText[i]);
			m_pText[i] = LTNULL;
		}
	}
}

// RETAIL: 0x1003b990: the names show while the progress command is on
// (not in the revive games). Near only for CTronInterfaceMgr's layout
void CHUDSubChooser::Render()
{
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B14() || !IsEnabled()) return;

	Arrange();

	if (!m_bShow || !IsEnabled()) return;

	SetRenderState();

	for (int i = 0; i < m_nNumSubs; i++)
	{
		if (!m_bAvailable[i]) continue;

		if (m_bBad[i])
		{
			g_pDrawPrim->SetTexture(m_hBadBackTex);
			g_pDrawPrim->DrawPrim(&m_BadBack[i], 1);
		}
		else
		{
			g_pDrawPrim->SetTexture(m_hBackTex);
			g_pDrawPrim->DrawPrim(&m_Back[i], 1);
		}

		g_pDrawPrim->SetTexture(m_hIconTex[i]);
		g_pDrawPrim->DrawPrim(&m_Icon[i], 1);

		if (!IsRevivePlayerGameType() && g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_PROGRESS) && m_pText[i])
		{
			m_pText[i]->Render();
		}
	}
}

// RETAIL: 0x1003bad0
void CHUDSubChooser::Arrange()
{
	// PORT: stand-in. Retail lays the available subroutines out in a column
	// at the right of the screen (its width less 42), from below one HUD
	// item (the item at retail's 0x1026ee10, its +0x40) down to above
	// another (0x1026edc8, its +0x14), each name left of its box, starting
	// a new column when one is full: neither item is in the tree yet
}

// RETAIL: 0x1003bd10
void CHUDSubChooser::Update()
{
	uint32 nScreenWidth = g_pInterfaceResMgr->GetScreenWidth();
	if (nScreenWidth != m_nScreenWidth)
	{
		m_nScreenWidth = nScreenWidth;
		m_fScale = g_pInterfaceResMgr->GetXRatio();

		uint8 nSize = (uint8)(m_nFontSize * m_fScale);
		CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nSize);
		for (int i = 0; i < HUDSUBCHOOSER_MAX; i++)
		{
			m_pText[i]->SetFont(pFont);
			m_pText[i]->SetCharScreenSize(nSize);
		}
	}

	for (int i = 0; i < m_nNumSubs; i++)
	{
		TronSubroutine* pSub = m_pSubs[i];
		m_bBad[i] = false;
		// PORT: retail asks the subroutine manager whether the player has the
		// subroutine compiled (retail 0x10094e70) and whether it is a bad one
		// (0x10094fb0); neither is in the tree yet
		m_bAvailable[i] = false;
		if (m_bAvailable[i])
		{
			m_pText[i]->SetText(LoadTempString(pSub->nNameId));
		}
	}
}
