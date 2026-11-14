// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMatchStats.cpp
//
// PURPOSE : HUDItem showing the local player's disc match
//
// RETAIL: Tron 2.0's, written from retail CShell.dll 0x1002fe20..
// 0x10030ac0 (HUDMatchStats.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDMatchStats.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "SharedScoring.h"
#include "DiscMatchMgr.h"

extern CGameClientShell* g_pGameClientShell;

CHUDMatchStats::CHUDMatchStats()
{
	m_UpdateFlags = 0x02000000;	// RETAIL: Tron's
	m_eLevel = kHUDRenderText;

	m_fScale = 0.0f;
	m_nRounds = 0;
	m_nWins[0] = 0;
	m_nWins[1] = 0;
	m_nTronUnknown14 = 0;
	m_pTime = LTNULL;
	m_pTeam[1] = LTNULL;
	m_pTeam[0] = LTNULL;
}

LTBOOL CHUDMatchStats::Init()
{
	UpdateLayout();

	m_hEmpty = g_pInterfaceResMgr->GetTexture("Interface\\hud\\circle_empty.dtx");
	m_hFull = g_pInterfaceResMgr->GetTexture("Interface\\hud\\circle_full.dtx");

	for (int i = 0; i < kMaxRounds * 2; i++)
	{
		// RETAIL: half a texel in from the circle's edges
		uint32 nWidth, nHeight;
		float fU = 0.0f;
		float fV = 0.0f;
		if (g_pTexInterface->GetTextureDims(m_hEmpty, nWidth, nHeight) == LT_OK)
		{
			fU = 0.5f / (float)nWidth;
			fV = 0.5f / (float)nHeight;
		}

		LT_POLYFT4* pPoly = &m_Circles[i];
		pPoly->verts[0].u = fU;
		pPoly->verts[0].v = fV;
		pPoly->verts[1].u = 1.0f - fU;
		pPoly->verts[1].v = fV;
		pPoly->verts[2].u = 1.0f - fU;
		pPoly->verts[2].v = 1.0f - fV;
		pPoly->verts[3].u = fU;
		pPoly->verts[3].v = 1.0f - fV;

		g_pDrawPrim->SetRGBA(pPoly, 0xFFFFFFFF);
	}

	return LTTRUE;
}

void CHUDMatchStats::Term()
{
	if (m_pTeam[0])
	{
		g_pFontManager->DestroyPolyString(m_pTeam[0]);
		m_pTeam[0] = LTNULL;
	}
	if (m_pTeam[1])
	{
		g_pFontManager->DestroyPolyString(m_pTeam[1]);
		m_pTeam[1] = LTNULL;
	}
	if (m_pTime)
	{
		g_pFontManager->DestroyPolyString(m_pTime);
		m_pTime = LTNULL;
	}
}

void CHUDMatchStats::Render()
{
	if (!IsMultiplayerGame()) return;
	if (!IsEnabled()) return;
	if (m_nRounds < 1) return;

	if (m_pTime)
	{
		int nMatch = g_pDiscMatchMgr->GetLocalMatchIndex();
		if (nMatch >= 0)
		{
			DiscMatch* pMatch = g_pDiscMatchMgr->GetMatch(nMatch);
			int nTime = (int)(pMatch->fTronUnknown14 - g_pLTClient->GetTime());
			if (nTime < 0)
				nTime = 0;

			char szTmp[128];
			int nSeconds = nTime % 60;
			int nMinutes = (nTime - nSeconds) / 60;
			sprintf(szTmp, "%d:%02d", nMinutes, nSeconds);

			std::wstring sText;
			LTGUIWiden(sText, szTmp);
			m_pTime->SetText(sText);
		}
	}

	if (m_pTeam[0])
		m_pTeam[0]->Render();
	if (m_pTeam[1])
		m_pTeam[1]->Render();
	if (m_pTime)
		m_pTime->Render();

	SetRenderState();

	for (int i = 0; i < m_nRounds; i++)
	{
		g_pDrawPrim->SetTexture(m_nWins[0] > i ? m_hFull : m_hEmpty);
		g_pDrawPrim->DrawPrim(&m_Circles[i * 2], 1);

		g_pDrawPrim->SetTexture(m_nWins[1] > i ? m_hFull : m_hEmpty);
		g_pDrawPrim->DrawPrim(&m_Circles[i * 2 + 1], 1);
	}
}

void CHUDMatchStats::Update()
{
	std::wstring sText;

	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	uint8 nOldRounds = m_nRounds;

	int nMatch = g_pDiscMatchMgr->GetLocalMatchIndex();
	if (nMatch < 0)
	{
		m_nRounds = 0;
	}
	else
	{
		std::string sName;
		CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
		DiscMatch* pMatch = g_pDiscMatchMgr->GetMatch(nMatch);

		m_nRounds = (pMatch->nTronUnknown5 + 1) / 2;

		pTeamMgr->GetTeam(pMatch->nTeam[0], false)->GetName(false, sName, false);
		LTGUIWiden(sText, sName.c_str());
		m_pTeam[0]->SetText(sText);
		m_nWins[0] = pMatch->nWins[0];

		pTeamMgr->GetTeam(pMatch->nTeam[1], false)->GetName(false, sName, false);
		LTGUIWiden(sText, sName.c_str());
		m_pTeam[1]->SetText(sText);
		m_nWins[1] = pMatch->nWins[1];
	}

	int32 nWidth, nHeight;
	m_pTeam[0]->GetDims(&nWidth, &nHeight);
	m_pTeam[0]->SetPosition(m_nCenterX - nWidth / 2 - 150, m_nTop);

	m_pTeam[1]->GetDims(&nWidth, &nHeight);
	m_pTeam[1]->SetPosition(m_nCenterX - nWidth / 2 + 150, m_nTop);

	int nY = m_nTop + nHeight;
	m_pTime->GetDims(&nWidth, &nHeight);
	m_pTime->SetPosition(m_nCenterX - nWidth / 2, nY);

	if (m_nRounds != nOldRounds && m_nRounds >= 1)
	{
		int nHalf = m_nRounds * 8;
		float fY = (float)nY;
		for (int i = 0; i < m_nRounds; i++)
		{
			int nX = i * 16 - nHalf + m_nCenterX - 150;
			g_pDrawPrim->SetXYWH(&m_Circles[i * 2], (float)nX, fY, 16.0f, 16.0f);

			nX = m_nCenterX - i * 16 + nHalf + 150;
			g_pDrawPrim->SetXYWH(&m_Circles[i * 2 + 1], (float)nX, fY, 16.0f, 16.0f);
		}
	}
}

void CHUDMatchStats::UpdateLayout()
{
	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	m_nCenterX = (int)(fXRatio * 320.0f);
	m_nTronUnknown20 = (int)(fXRatio * 240.0f);
	m_nTop = (int)(fYRatio * 30.0f);

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(20);

	if (!m_pTeam[0])
	{
		m_pTeam[0] = g_pFontManager->CreateFormattedPolyString(pFont, std::wstring(L" "), 0.0f, 0.0f);
		m_pTeam[0]->SetColor(0xFFFFFFFF);
		m_pTeam[0]->SetCharScreenHeight(20);
		m_pTeam[0]->SetDropShadow(true);
	}

	if (!m_pTeam[1])
	{
		m_pTeam[1] = g_pFontManager->CreateFormattedPolyString(pFont, std::wstring(L" "), 0.0f, 0.0f);
		m_pTeam[1]->SetColor(0xFFFFFFFF);
		m_pTeam[1]->SetCharScreenHeight(20);
		m_pTeam[1]->SetDropShadow(true);
	}

	if (!m_pTime)
	{
		m_pTime = g_pFontManager->CreateFormattedPolyString(pFont, std::wstring(L" "), 0.0f, 0.0f);
		m_pTime->SetColor(0xFFFFFFFF);
		m_pTime->SetCharScreenHeight(20);
		m_pTime->SetDropShadow(true);
	}
}
