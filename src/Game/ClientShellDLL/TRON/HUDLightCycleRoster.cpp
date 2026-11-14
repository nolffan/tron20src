// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCycleRoster.cpp
//
// PURPOSE : HUDItem listing the light cyclists
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1002deb0..
// 0x1002f730; HUDLightCycleRoster.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDLightCycleRoster.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "LCHUDHelper.h"
#include "ltquaduvutils.h"
#include "LightCycleMgr.h"
#include "LightCycleButeMgr.h"
#include "SharedScoring.h"

// RETAIL: two constants of this TU (retail 0x101e6f3c, 0x101e6f40: the
// entries' Update reads them from memory). The names are ours
static const LTFLOAT s_fMinAlpha = 0.0f;
static const LTFLOAT s_fFlashTime = 0.5f;

// RETAIL: 0x1026d998: set when an entry changes; Render then updates the
// roster. The name is ours
static bool s_bRosterChanged = false;

// RETAIL: the roster's search for a cyclist's entry (its find_if's
// predicate holds the cyclist by reference: retail 0x1002f6f0). The name
// is ours
struct LightCycleRosterIsCyclist
{
	LightCycleRosterIsCyclist(LIGHT_CYCLIST*& pCyclist) : m_pCyclist(pCyclist) {}
	bool operator()(CLightCycleRosterEntry& entry) const { return entry.IsCyclist(m_pCyclist); }

	LIGHT_CYCLIST*&	m_pCyclist;
};

// ----------------------------------------------------------------------- //
//	CLightCycleRosterEntry
// ----------------------------------------------------------------------- //

CLightCycleRosterEntry::CLightCycleRosterEntry(LIGHT_CYCLIST* pCyclist) :
	m_fAlpha(1.0f),
	m_nFlashCount(0),
	m_nFlashDir(-1),
	m_bAlive(true),
	m_hObject(LTNULL),
	m_hIcon(LTNULL),
	m_pText(LTNULL)
{
	g_pDrawPrim->SetRGBA(&m_Poly, argbWhite);
	SetupQuadUVs(m_Poly, m_hIcon, 0.0f, 0.0f, 1.0f, 1.0f);

	m_hObject = pCyclist->hObject;
	m_sColor = pCyclist->szCycleColor;
	m_sName = g_pLightCycleMgr->GetCyclistName(pCyclist->hObject);

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	if (!pTeamMgr)
		m_nTeam = 0xff;
	else
		m_nTeam = pTeamMgr->GetTeamOfClient(pCyclist->nClientID);

	Init();
}

CLightCycleRosterEntry::CLightCycleRosterEntry(const CLightCycleRosterEntry& other) :
	m_pText(LTNULL)
{
	m_hIcon = LTNULL;

	m_hIcon = other.m_hIcon;
	m_Poly = other.m_Poly;
	m_fAlpha = other.m_fAlpha;
	m_nFlashCount = other.m_nFlashCount;
	m_nFlashDir = other.m_nFlashDir;
	m_bAlive = other.m_bAlive;
	m_hObject = other.m_hObject;
	m_sColor = other.m_sColor;
	m_sName = other.m_sName;
	m_nTeam = other.m_nTeam;

	Init();
}

CLightCycleRosterEntry::~CLightCycleRosterEntry()
{
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
	}
}

void CLightCycleRosterEntry::Init()
{
	LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetCycleColor(m_sColor.c_str());
	m_hIcon = g_pInterfaceResMgr->GetTexture(pColor->sHUDRosterIconTex.c_str());

	std::wstring sName;
	LTGUIWiden(sName, m_sName.c_str());

	if (!m_pText)
	{
		m_pText = g_pFontManager->CreateFormattedPolyString(g_pInterfaceResMgr->GetFont(1), sName, 0.0f, 0.0f);
	}

	m_pText->SetDropShadow(true);
	m_pText->SetText(sName);
}

bool CLightCycleRosterEntry::IsCyclist(LIGHT_CYCLIST* pCyclist)
{
	if (!pCyclist)
		return false;

	if (pCyclist->hObject == m_hObject) return true;
	return false;
}

bool CLightCycleRosterEntry::operator<(const CLightCycleRosterEntry& other) const
{
	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	if (pTeamMgr)
	{
		uint32 nNumTeams = pTeamMgr->GetNumTeams();
		if (other.m_nTeam < nNumTeams && m_nTeam < nNumTeams)
		{
			CTronTeam* pOther = pTeamMgr->GetTeam(other.m_nTeam, false);
			CTronTeam* pTeam = pTeamMgr->GetTeam(m_nTeam, false);
			if (pTeam->GetTronUnknown12() != pOther->GetTronUnknown12())
				return pTeam->GetTronUnknown12() > pOther->GetTronUnknown12();

			return m_nTeam > other.m_nTeam;
		}
	}

	return m_nTeam > other.m_nTeam;
}

LTBOOL CLightCycleRosterEntry::Update()
{
	if (!m_bAlive && m_nFlashCount > 0)
	{
		int nFlashDir = m_nFlashDir;
		m_fAlpha += g_pLTClient->GetFrameTime() / s_fFlashTime * nFlashDir;
		if (m_fAlpha > 1.0f || m_fAlpha < s_fMinAlpha)
		{
			m_fAlpha = LTMIN(1.0f, LTMAX(m_fAlpha, s_fMinAlpha));
			m_nFlashDir = -nFlashDir;
			m_nFlashCount--;
		}

		g_pDrawPrim->SetALPHA(&m_Poly, (uint8)(m_fAlpha * 255.0f));
		s_bRosterChanged = true;
	}

	return LTFALSE;
}

LTBOOL CLightCycleRosterEntry::Render()
{
	// RETAIL: CHUDItem::SetRenderState's, inline
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	g_pDrawPrim->SetTexture(m_hIcon);
	g_pDrawPrim->DrawPrim(&m_Poly, 1);

	m_pText->Render();

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//	CHUDLightCycleRoster
// ----------------------------------------------------------------------- //

CHUDLightCycleRoster::CHUDLightCycleRoster()
{
	// RETAIL: Tron's update flag for it
	m_UpdateFlags = (eHUDUpdateFlag)0x800;
}

void CHUDLightCycleRoster::Term()
{
	m_Entries.clear();
}

LTBOOL CHUDLightCycleRoster::Init()
{
	Update();
	return LTTRUE;
}

void CHUDLightCycleRoster::Render()
{
	if (s_bRosterChanged)
	{
		s_bRosterChanged = false;
		Update();
	}

	if (!dynamic_cast<CTronInterfaceMgr*>(g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	m_Entries.sort(std::less<CLightCycleRosterEntry>());
	UpdateLayout();

	std::for_each(m_Entries.begin(), m_Entries.end(), std::mem_fun_ref(&CLightCycleRosterEntry::Render));
}

void CHUDLightCycleRoster::Update()
{
	UpdateEntries();
	UpdateLayout();
}

void CHUDLightCycleRoster::UpdateLayout()
{
	LTFLOAT fXRatio = g_pInterfaceResMgr->GetXRatio();
	LTFLOAT fYRatio = g_pInterfaceResMgr->GetYRatio();

	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);

	LTIntPt pos = g_pTronLayoutMgr->GetLCHUDHelper()->GetIntroMasterPos(nCurrentLayout);
	pos.x = (int)(pos.x * fXRatio);
	pos.y = (int)(pos.y * fYRatio);
	int nTextHeight = (int)(g_pTronLayoutMgr->GetLCHUDHelper()->GetRosterTextHeight(nCurrentLayout) * fYRatio);
	int nTextOffset = (int)(g_pTronLayoutMgr->GetLCHUDHelper()->GetIntroEntrySpace(nCurrentLayout) * fXRatio);
	int nLineOffset = (int)(g_pTronLayoutMgr->GetLCHUDHelper()->GetIntroHeaderOffset(nCurrentLayout) * fYRatio);
	bool bScaleIcons = (g_pTronLayoutMgr->GetLCHUDHelper()->GetIntroTitleSpace(nCurrentLayout) != 0);

	LTFLOAT fX = (LTFLOAT)pos.x;
	LTFLOAT fY = (LTFLOAT)pos.y;

	LightCycleRosterList::iterator iter = m_Entries.begin();
	while (iter != m_Entries.end())
	{
		CLightCycleRosterEntry& entry = *iter;

		LTFLOAT fWidth, fHeight;
		if (bScaleIcons)
		{
			LTIntPt size = g_pTronLayoutMgr->GetLCHUDHelper()->GetRosterIconBaseSizeIfScalingIsEnabled(nCurrentLayout);
			fWidth = size.x * fXRatio;
			fHeight = size.y * fYRatio;
		}
		else
		{
			uint32 nWidth, nHeight;
			g_pTexInterface->GetTextureDims(entry.m_hIcon, nWidth, nHeight);
			fWidth = (LTFLOAT)nWidth;
			fHeight = (LTFLOAT)nHeight;
		}

		g_pDrawPrim->SetXYWH(&entry.m_Poly, fX, fY, fWidth, fHeight);

		entry.m_pText->SetFont(g_pInterfaceResMgr->GetFontForHeight(nTextHeight));
		entry.m_pText->SetCharScreenHeight(nTextHeight);

		LTFLOAT fHalfText = (LTFLOAT)entry.m_pText->GetHeight() * 0.5f;
		LTFLOAT fHalfIcon = fHeight * 0.5f;
		entry.m_pText->SetPosition((int)(fX + nTextOffset + fWidth), (int)(fY - fHalfText + fHalfIcon));

		fY += LTMAX(fHalfIcon + fHalfText, fHeight) + nLineOffset;

		iter++;
	}
}

void CHUDLightCycleRoster::DerezCyclist(LIGHT_CYCLIST* pCyclist)
{
	if (!pCyclist) return;

	LightCycleRosterList::iterator iter = std::find_if(m_Entries.begin(), m_Entries.end(), LightCycleRosterIsCyclist(pCyclist));
	if (iter != m_Entries.end())
	{
		(*iter).m_bAlive = false;
		(*iter).m_fAlpha = 1.0f;
		(*iter).m_nFlashCount = 7;
		(*iter).m_nFlashDir = -1;
		s_bRosterChanged = true;
	}
}

void CHUDLightCycleRoster::AddCyclist(LIGHT_CYCLIST* pCyclist)
{
	LightCycleRosterList::iterator iter = std::find_if(m_Entries.begin(), m_Entries.end(), LightCycleRosterIsCyclist(pCyclist));
	if (m_Entries.end() != iter)
	{
		m_Entries.erase(iter);
	}

	m_Entries.push_back(CLightCycleRosterEntry(pCyclist));
	s_bRosterChanged = true;
}

void CHUDLightCycleRoster::Reset()
{
	m_Entries.clear();
	s_bRosterChanged = true;
}

void CHUDLightCycleRoster::UpdateEntries()
{
	std::for_each(m_Entries.begin(), m_Entries.end(), std::mem_fun_ref(&CLightCycleRosterEntry::Update));

	LightCycleRosterList::iterator iterEnd = m_Entries.end();
	LightCycleRosterList::iterator iter = m_Entries.begin();
	while (iter != iterEnd)
	{
		LightCycleRosterList::iterator iterNext = iter;
		iterNext++;

		if ((*iter).IsDone())
		{
			m_Entries.erase(iter);
		}

		iter = iterNext;
	}
}

bool CLightCycleRosterEntry::IsDone()
{
	return !m_bAlive && !m_nFlashCount;
}
