// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSingleString.cpp
//
// PURPOSE : HUDItem showing the one string the server asks for
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1003ad60..
// 0x1003b1d0; HUDSingleString.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "TronInterfaceMgr.h"
#include "HUDSingleString.h"
#include "HUDDiscScores.h"

CHUDSingleString::CHUDSingleString()
{
	LTIntPt kNullPt(0,0);

	m_BasePos = kNullPt;
	// RETAIL: Tron's update flag for it
	m_UpdateFlags = (eHUDUpdateFlag)0x4000000;
	m_pString = LTNULL;
	m_vColor = LTVector(32.0f,32.0f,255.0f);
	m_nStringID = 0;
	m_fScale = 0.0f;
}

void CHUDSingleString::Term()
{
	if (m_pString)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pString);
		m_pString = LTNULL;
	}
}

void CHUDSingleString::Render()
{
	if (!IsMultiplayerGame()) return;
	if (!IsEnabled()) return;

	// not over the disc games' scores
	if (g_pDiscScores && g_pDiscScores->IsVisible()) return;

	if (m_nStringID && m_pString)
	{
		m_pString->Render();
	}
}

void CHUDSingleString::Update()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	m_nStringID = ((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B18();
	if (m_nStringID)
	{
		m_pString->SetText(LoadTempWString(m_nStringID));

		int32 nWidth, nHeight;
		m_pString->GetDims(&nWidth, &nHeight);
		m_pString->SetPosition(m_BasePos.x - nWidth / 2, m_BasePos.y);
	}
	else
	{
		m_pString->SetText(std::wstring(L" "));
	}
}

void CHUDSingleString::UpdateLayout()
{
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	LTIntPt pos;
	if (IsRevivePlayerGameType())
		pos = g_pLayoutMgr->GetPoint("SingleString", "DiscMPPosition");
	else
		pos = g_pLayoutMgr->GetPoint("SingleString", "Position");
	m_BasePos.x = (int)(pos.x * xs);
	m_BasePos.y = (int)(pos.y * ys);

	g_pLayoutMgr->GetInt("SingleString", "Font");
	int nFontSize = (int)(g_pLayoutMgr->GetInt("SingleString", "FontSize") * xs);
	m_vColor = g_pLayoutMgr->GetVector("SingleString", "FontColor");

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nFontSize);
	if (!m_pString)
	{
		m_pString = g_pFontManager->CreatePolyString(pFont, std::wstring(L" "));
		m_pString->SetDropShadow(true);
	}
	else
	{
		m_pString->SetFont(pFont);
	}

	m_pString->SetCharScreenHeight(nFontSize);
	m_pString->SetColor(SET_ARGB(0xFF, m_vColor.x, m_vColor.y, m_vColor.z));
}
