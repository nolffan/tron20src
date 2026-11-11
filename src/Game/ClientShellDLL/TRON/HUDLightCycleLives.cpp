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
#include "HUDLightCycleLives.h"
#include "InterfaceMgr.h"

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

LTBOOL CHUDLightCycleLives::Init()
{
	// PORT: stand-in; retail 0x1002d360 sets up the background quad and the
	// text with Tron's font manager
	m_hBack = g_pInterfaceResMgr->GetTexture("interface\\hud\\armor_bg.dtx");
	g_pDrawPrim->SetRGBA(&m_Poly, argbWhite);
	return LTTRUE;
}

void CHUDLightCycleLives::Term()
{
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}
}

void CHUDLightCycleLives::Render()
{
	// PORT: stand-in for retail 0x1002d560 (which first asks Tron's interface
	// manager whether the light cycle HUD is up)
	if (!IsEnabled()) return;

	SetRenderState();

	uint8 nAlpha = 0xFF;
	if (m_nFlashCount > 0)
	{
		m_fAlpha += (g_pLTClient->GetFrameTime() / 0.5f) * m_nFlashDir;
		if (m_fAlpha > 1.0f || m_fAlpha < 0.0f)
		{
			if (m_fAlpha > 1.0f) m_fAlpha = 1.0f;
			if (m_fAlpha < 0.0f) m_fAlpha = 0.0f;
			m_nFlashDir = -m_nFlashDir;
			m_nFlashCount--;
		}
		nAlpha = (uint8)(m_fAlpha * 255.0f);
	}

	g_pDrawPrim->SetALPHA(&m_Poly, nAlpha);
	g_pDrawPrim->SetTexture(m_hBack);
	g_pDrawPrim->DrawPrim(&m_Poly, 1);

	if (m_pText)
	{
		m_pText->Render();
	}
}

void CHUDLightCycleLives::Update()
{
	UpdateText();
	UpdateLayout();
}

void CHUDLightCycleLives::UpdateText()
{
	// PORT: stand-in; retail 0x1002d890 writes "%d/%d" (lives left of the
	// player manager's lives) into the text as a wide string
}

void CHUDLightCycleLives::UpdateLayout()
{
	// PORT: stand-in; retail 0x1002d6b0 places the quad and text by the
	// HUD layout
}
