// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPings.cpp
//
// PURPOSE : HUDItem listing the pings
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (HUDPings.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDPings.h"
#include "TronLayoutMgr.h"
#include "HUDDecision.h"
#include "InterfaceMgr.h"
#include "ClientSoundMgr.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "GameClientShell.h"
#include "CommandIDs.h"

// RETAIL: the layout manager as a CTronLayoutMgr, without the dynamic cast
#define g_pTronLayoutMgrStatic ((CTronLayoutMgr*)g_pLayoutMgr)

extern CHUDDecision* g_pDecision;

// RETAIL: 0x10031100
CHUDPings::CHUDPings()
{
	m_IconSize.x = 64;
	m_IconSize.y = 64;
	m_StartPos.x = 10;
	m_nSpacing = 10;
	m_UpdateFlags = 0x00020000;	// RETAIL: Tron's flag for it
	m_fScale = 0.0f;
	m_nNumPings = 0;
	m_StartPos.y = 40;
	m_nFontNum = 0;
	m_nFontSize = 18;
}

// RETAIL: 0x100311e0. Only the first four of the pings are set up (the
// list shows four)
LTBOOL CHUDPings::Init()
{
	m_nNumPings = 0;
	UpdateLayout();

	for (int i = 0; i < 4; i++)
	{
		g_pDrawPrim->SetUVWH(&m_Pings[i].Poly, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetRGBA(&m_Pings[i].Poly, argbWhite);
		m_Pings[i].pName = LTNULL;
	}

	return LTTRUE;
}

// RETAIL: 0x10031240
void CHUDPings::Term()
{
	for (int i = 0; i < m_nNumPings; i++)
	{
		if (m_Pings[i].pName)
		{
			g_pLTClient->GetFontManager()->DestroyPolyString(m_Pings[i].pName);
			m_Pings[i].pName = LTNULL;
		}
	}
}

// RETAIL: 0x10031290
void CHUDPings::Render()
{
	if (!m_nNumPings) return;
	if (g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE) return;
	if (g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_PROGRESS)) return;
	if (g_pDecision && g_pDecision->IsVisible()) return;
	// PORT: retail also hides the list while Tron's decision HUD (its
	// global at 0x1026edec, not in the tree yet) is up
	if (!IsEnabled()) return;

	LTFLOAT fFrameTime = g_pGameClientShell->GetFrameTime();

	// The pings that expire go
	bool bChanged = false;
	int i;
	for (i = 0; i < m_nNumPings; i++)
	{
		if (m_Pings[i].bExpires)
		{
			m_Pings[i].fLifeTime -= fFrameTime;
			if (m_Pings[i].fLifeTime < 0.0f)
			{
				m_Pings[i].hIcon = LTNULL;
				// PORT: retail lets the player stats' ping manager know
				// (CTronPlayerStats' virtual at +0x164, by name: that slot is
				// not yet retail's here)
				bChanged = true;
			}
		}
	}
	if (bChanged)
	{
		Arrange();
	}

	for (i = 0; i < m_nNumPings; i++)
	{
		HUDPING* pPing = &m_Pings[i];
		if (!pPing->hIcon) continue;

		SetRenderState();

		LTFLOAT fFade = 1.0f;
		if (pPing->nBlinkCount)
		{
			pPing->fBlinkTime -= fFrameTime;
			if (pPing->fBlinkTime < 0.0f)
			{
				if (--pPing->nBlinkCount)
				{
					pPing->fBlinkTime += 0.5f;
					g_pClientSoundMgr->PlaySoundLocal("blink_obj", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f);
				}
			}
			fFade = pPing->fBlinkTime + pPing->fBlinkTime;
		}

		uint8 nAlpha = (uint8)(fFade * 255.0f);
		g_pDrawPrim->SetALPHA(&pPing->Poly, nAlpha);
		g_pDrawPrim->SetTexture(pPing->hIcon);
		g_pDrawPrim->DrawPrim(&pPing->Poly, 1);

		CUIFormattedPolyString* pName = pPing->pName;
		if (pName)
		{
			pName->SetColor(SET_ARGB(nAlpha, 0xFF, 0xFF, 0xFF));
			pName->Render();
		}
	}
}

// RETAIL: 0x10031490
void CHUDPings::Update()
{
	// PORT: stand-in. Retail rebuilds the list from the player stats' ping
	// (I/O node) manager, keeping the blink and life times of the pings
	// already shown, the new ones blinking (m_nBlinkCount, m_fBlinkTime),
	// each with its name as a wide string, then arranges it
	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}
}

// RETAIL: 0x10031840: the icons down from the start point, each name
// centred under its icon
void CHUDPings::Arrange()
{
	if (!m_nNumPings) return;

	LTFLOAT fY = (LTFLOAT)m_StartPos.y;
	for (int i = 0; i < m_nNumPings; i++)
	{
		if (!m_Pings[i].hIcon) continue;

		LTFLOAT fX = (LTFLOAT)m_StartPos.x;
		g_pDrawPrim->SetXYWH(&m_Pings[i].Poly, fX, fY, (LTFLOAT)m_IconSize.x, (LTFLOAT)m_IconSize.y);
		fY += (LTFLOAT)m_IconSize.y;

		CUIFormattedPolyString* pName = m_Pings[i].pName;
		if (pName)
		{
			int32 nWidth, nHeight;
			pName->GetDims(&nWidth, &nHeight);
			LTFLOAT fHeight = (LTFLOAT)nHeight;

			LTFLOAT fNameX = (LTFLOAT)m_IconSize.x * 0.5f + fX - (LTFLOAT)nWidth * 0.5f;
			if (fNameX < 0.0f)
				fNameX = 0.0f;
			pName->SetPosition((int)fNameX, (int)fY);

			fY += fHeight;
		}

		fY += (LTFLOAT)m_nSpacing;
	}
}

// RETAIL: 0x10031960
void CHUDPings::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);

	LTFLOAT fXRatio = g_pInterfaceResMgr->GetXRatio();
	LTFLOAT fYRatio = g_pInterfaceResMgr->GetYRatio();

	m_IconSize = g_pTronLayoutMgrStatic->GetPingIconSize(nCurrentLayout);
	m_StartPos = g_pTronLayoutMgrStatic->GetPingStartPoint(nCurrentLayout);
	m_StartPos.x = (int)(m_StartPos.x * fXRatio);
	m_StartPos.y = (int)(m_StartPos.y * fYRatio);

	m_nSpacing = (int)((int)g_pTronLayoutMgrStatic->GetPingSpacing(nCurrentLayout) * fYRatio);
	m_nFontNum = g_pTronLayoutMgrStatic->GetPingFontNum(nCurrentLayout);

	// RETAIL: twice
	m_nFontSize = (int)((int)g_pTronLayoutMgrStatic->GetPingFontSize(nCurrentLayout) * fXRatio);
	m_nFontSize = (int)((int)g_pTronLayoutMgrStatic->GetPingFontSize(nCurrentLayout) * fXRatio);

	m_nBlinkCount = g_pTronLayoutMgrStatic->GetPingBlinkCount(nCurrentLayout);
	m_fBlinkTime = g_pTronLayoutMgrStatic->GetPingBlinkTime(nCurrentLayout);
}
