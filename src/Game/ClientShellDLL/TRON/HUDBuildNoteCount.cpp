// ----------------------------------------------------------------------- //
//
// MODULE  : HUDBuildNoteCount.cpp
//
// PURPOSE : HUDItem showing how many of a level's build notes Jet has
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x100223c0..
// 0x10022a20; HUDBuildNoteCount.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "TronInterfaceMgr.h"
#include "HUDBuildNoteCount.h"
#include "MissionMgr.h"
#include "MissionButeMgr.h"
#include "IntelItemList.h"

namespace
{
	// RETAIL: a pointer variable (0x10213068)
	char* s_pszTag = "BuildNoteCount";
}

CHUDBuildNoteCount::CHUDBuildNoteCount() :
	m_fScale(0.0f),
	m_hIcon(LTNULL),
	m_pText(LTNULL)
{
	// RETAIL: Tron's update flag for it
	m_UpdateFlags = (eHUDUpdateFlag)0x20000000;
}

// RETAIL: inline: the destructor (0x10022430) has its body
inline void CHUDBuildNoteCount::Term()
{
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}
}

CHUDBuildNoteCount::~CHUDBuildNoteCount()
{
	Term();
}

LTBOOL CHUDBuildNoteCount::Init()
{
	if (!m_hIcon)
	{
		char szTmp[1024];
		g_pLayoutMgr->GetString(s_pszTag, "IconTexture", szTmp, sizeof(szTmp));
		m_hIcon = g_pInterfaceResMgr->GetTexture(szTmp);
	}

	g_pDrawPrim->SetRGBA(&m_Poly, argbWhite);
	g_pDrawPrim->SetUVWH(&m_Poly, 0.0f, 0.0f, 1.0f, 1.0f);

	if (!m_pText)
	{
		int nTextSize = g_pLayoutMgr->GetInt(s_pszTag, "TextSize");
		CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight((int)(nTextSize * m_fScale));
		m_pText = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, std::wstring(L"TempToForceValidSize"));

		LTVector vColor = g_pLayoutMgr->GetVector(s_pszTag, "BaseColor");
		m_pText->SetColor(SET_ARGB(0xFF, vColor.x, vColor.y, vColor.z));
		m_pText->SetDropShadow(true);

		UpdateLayout();
	}

	return LTTRUE;
}

void CHUDBuildNoteCount::Render()
{
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	if (m_hIcon)
	{
		SetRenderState();
		g_pDrawPrim->SetTexture(m_hIcon);
		g_pDrawPrim->DrawPrim(&m_Poly, 1);
	}

	m_pText->Render();
}

void CHUDBuildNoteCount::Update()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
	}

	int nMission = g_pMissionMgr->GetCurrentMission();
	int nLevel = g_pMissionMgr->GetCurrentLevel();

	// the build notes of this level Jet has
	int nFound = 0;
	uint16 nCount = g_pPlayerStats->GetIntelList()->GetCount();
	for (int i = 0; i < nCount; i++)
	{
		INTEL_ITEM* pItem = g_pPlayerStats->GetIntelList()->Get(i);
		// PORT: retail (0x10022737) tests Tron's INTEL_ITEM: a byte at +0x4
		// (1 for a build note), the mission at +0x8 and the level at +0x9;
		// NOLF2's item has no level, so its intel flag and mission stand in
		if (pItem && pItem->bIsIntel && pItem->nMissionNum == nMission)
		{
			nFound++;
		}
	}

	// PORT: retail reads the level's build note count (Tron's LEVEL, +0x1004),
	// which NOLF2's LEVEL has not
	int nTotal = 0;
	g_pMissionButeMgr->GetLevel(nMission, nLevel);

	// RETAIL: string 3223 (0xc97), "%d/%d"
	m_pText->SetText(FormatTempString(3223, nFound, nTotal));
}

void CHUDBuildNoteCount::UpdateLayout()
{
	m_fScale = g_pInterfaceResMgr->GetXRatio();

	LTIntPt pos = g_pLayoutMgr->GetPoint(s_pszTag, "AnchorPos");
	pos.x = (int)(pos.x * m_fScale);
	pos.y = (int)(pos.y * m_fScale);

	int nTextSize = g_pLayoutMgr->GetInt(s_pszTag, "TextSize");
	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight((int)(nTextSize * m_fScale));
	m_pText->SetFont(pFont);
	m_pText->SetCharScreenHeight((int)(nTextSize * m_fScale));

	float fSpacer = g_pLayoutMgr->GetFloat(s_pszTag, "IconTextSpacer", 10.0f) * m_fScale;

	uint32 nWidth, nHeight;
	g_pTexInterface->GetTextureDims(m_hIcon, nWidth, nHeight);
	nWidth = (uint32)(nWidth * m_fScale);
	nHeight = (uint32)(nHeight * m_fScale);

	// the icon and the text centred on the taller of them
	uint32 nTextHeight = m_pText->GetHeight();
	float fHeight = std::max((float)nHeight, (float)nTextHeight);

	g_pDrawPrim->SetXYWH(&m_Poly, (float)pos.x, (fHeight - (float)nHeight) * 0.5f + (float)pos.y, (float)nWidth, (float)nHeight);
	m_pText->SetPosition((int)((float)(nWidth + pos.x) + fSpacer), (int)((fHeight - (float)nTextHeight) * 0.5f + (float)pos.y));
}
