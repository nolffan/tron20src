// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProfiler.cpp
//
// PURPOSE : HUDItem showing the name of what profiles the player
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (HUDProfiler.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"
#include "HUDProfiler.h"
#include "TronPlayerMgr.h"
#include "TargetMgr.h"
#include "InterfaceResMgr.h"
#include "LayoutMgr.h"

// RETAIL: 0x10032670
CHUDProfiler::CHUDProfiler()
{
	m_UpdateFlags = 0x00100000;	// RETAIL: Tron's flag for it
	m_fScale = 0.0f;
	m_fJitter = 5.0f;
	m_bTronUnknown14 = false;
	m_pText = LTNULL;
	m_pFlicker = LTNULL;
	m_szName[0] = '\0';
}

// RETAIL: 0x100326d0
CHUDProfiler::~CHUDProfiler()
{
	Term();
}

// RETAIL: 0x10032720
LTBOOL CHUDProfiler::Init()
{
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(0);

	m_pText = g_pFontManager->CreateFormattedPolyString(pFont, std::wstring(), 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pText->SetDropShadow(true);

	m_pFlicker = g_pFontManager->CreateFormattedPolyString(pFont, std::wstring(), 0.0f, 0.0f, CUI_HALIGN_LEFT);

	UpdateLayout();

	m_fFlickerTime = 0.1f;

	return LTTRUE;
}

// RETAIL: empty (folded with the other HUD items' empty Term, 0x100abb80)
void CHUDProfiler::Term()
{
}

// RETAIL: inlined in Render (the name is ours): the flicker jumps about
// and fades, its letters scrambled
inline void CHUDProfiler::Flicker()
{
	LTFLOAT fX = GetRandom(0.0f, m_fJitter);
	LTFLOAT fY = GetRandom(-m_fJitter, m_fJitter);
	m_pFlicker->SetPosition(m_Pos.x + (int)fX, m_Pos.y + (int)fY);

	uint8 nAlpha = (uint8)GetRandom(128, 255);
	m_pFlicker->SetColor(SET_ARGB(nAlpha, 0xFF, 0xFF, 0xFF));

	Scramble();
}

// RETAIL: 0x10032870
void CHUDProfiler::Render()
{
	if (!m_szName[0] || !IsEnabled()) return;

	SetRenderState();

	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fDelta = fTime - m_fLastTime;
	m_fLastTime = fTime;

	bool bWasProfiling = m_bProfiling;
	m_bProfiling = g_pTronPlayerMgr->IsTronUnknown205();
	if (m_bProfiling)
	{
		if (!bWasProfiling)
		{
			m_fFlickerTimer = m_fFlickerTime;
			g_pLTClient->CPrint("New profile");
		}
		else
		{
			m_fFlickerTimer -= fDelta;
			if (m_fFlickerTimer < 0.0f)
			{
				m_fFlickerTimer = m_fFlickerTime;
				g_pLTClient->CPrint("New flicker");
			}
			else
			{
				goto RenderFlicker;
			}
		}

		Flicker();

RenderFlicker:
		m_pFlicker->Render();
	}
	else
	{
		m_pText->Render();
	}
}

// RETAIL: 0x100329c0: the flicker's letters each show a random piece of
// the font's texture, now and then their own
void CHUDProfiler::Scramble()
{
	LT_POLYGT4* pDest = m_pFlicker->GetPolys();
	LT_POLYGT4* pSrc = m_pText->GetPolys();
	int nNum = m_pFlicker->GetLength();
	for (int i = 0; i < nNum; i++, pSrc++, pDest++)
	{
		if (pSrc->verts[3].v == pSrc->verts[0].v) continue;

		LTFLOAT fU, fV;
		if (GetRandom(0.0f, 1.0f) > 0.95f)
		{
			fU = pSrc->verts[0].u;
			fV = pSrc->verts[0].v;
		}
		else
		{
			fU = GetRandom(0.0f, 0.9f);
			fV = GetRandom(0.0f, 0.9f);
		}
		g_pDrawPrim->SetUVWH(pDest, fU, fV, pSrc->verts[1].u - pSrc->verts[0].u, pSrc->verts[3].v - pSrc->verts[0].v);
	}
}

// RETAIL: 0x10032a90
void CHUDProfiler::Update()
{
	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
	}

	// RETAIL: the name is Tron's target manager's, at +0x895 there (its
	// layout is not yet retail's here)
	const char* pszName = (const char*)g_pTronPlayerMgr->GetTargetMgr() + 0x895;
	if (stricmp(pszName, m_szName) == 0) return;

	SAFE_STRCPY(m_szName, pszName);

	std::wstring sName;
	LTGUIWiden(sName, pszName);

	m_pText->SetText(sName);
	m_pFlicker->SetText(sName);

	int32 nWidth, nHeight;
	m_pText->GetDims(&nWidth, &nHeight);
	m_pText->SetPosition(m_Pos.x, m_Pos.y - nHeight);

	Scramble();
}

// RETAIL: 0x10032c70
void CHUDProfiler::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);

	LTFLOAT fXRatio = g_pInterfaceResMgr->GetXRatio();
	LTFLOAT fYRatio = g_pInterfaceResMgr->GetYRatio();
	m_fScale = fXRatio;
	m_fJitter = fXRatio * 5.0f;

	int nTextSize = (int)(g_pLayoutMgr->GetDisplayMeterHeight(nCurrentLayout) * fXRatio);
	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nTextSize);

	m_pText->SetFont(pFont);
	m_pText->SetCharScreenHeight(nTextSize);
	m_pFlicker->SetFont(pFont);
	m_pFlicker->SetCharScreenHeight(nTextSize);

	m_Pos = g_pLayoutMgr->GetAmmoBarOffset(nCurrentLayout);
	m_Pos.x = (int)(m_Pos.x * fXRatio);
	m_Pos.y = (int)(m_Pos.y * fYRatio);

	int32 nWidth, nHeight;
	m_pText->GetDims(&nWidth, &nHeight);
	m_pText->SetPosition(m_Pos.x, m_Pos.y - nHeight);
}
