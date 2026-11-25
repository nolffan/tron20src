// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHealthEnergy.cpp
//
// PURPOSE : HUDItem to display health cache and energy
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDHealthEnergy.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "TronPlayerMgr.h"
#include "TronInterfaceMgr.h"
#include "TRONPlayerStats.h"
#include "SubroutineMgr.h"
#include "HUDCrosshair.h"

//******************************************************************************************
//**
//** HUD health cache and energy display
//**
//******************************************************************************************

// RETAIL: rewritten from retail CShell.dll 0x10029980-0x1002b3fa (Tron's
// health and energy bars, the energy plasma and the two counts). The member
// names are ours.

// RETAIL: 0x10029980
CHUDHealthEnergy::CHUDHealthEnergy()
{
	m_UpdateFlags = kHUDHealth | kHUDEnergy;
	m_hPlasmaTex = LTNULL;
	m_fScale = 0.0f;
	m_fPlasmaTime = 0.0f;
}

// RETAIL: 0x10029aa0. NEAR: register choice around GetTextureDims only
LTBOOL CHUDHealthEnergy::Init()
{
	m_nHealthWidth = 160;
	m_nEnergyWidth = 160;
	m_nHealth = 0;
	m_nMaxHealth = 0;
	m_fEnergy = 0.0f;
	m_fMaxEnergy = 0.0f;
	m_fReservedEnergy = 0.0f;
	m_nBarHeight = 12;
	m_fHealthBlinkTime = 0.0f;
	m_fLayoutHealthBlinkTime = 2.0f;
	m_bFlicker = true;
	m_fFlickerTime = 1.0f;

	UpdateLayout();

	m_hHUDTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\hud.dtx");
	m_hEnergyTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\energy_indicator.dtx");
	m_hPlasmaTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\plasma.dtx");

	g_pDrawPrim->SetRGBA(&m_Energy, argbWhite);
	g_pDrawPrim->SetRGBA(&m_HealthBG, argbWhite);
	g_pDrawPrim->SetRGBA(&m_HealthBar, 0xC0FFFFFF);
	g_pDrawPrim->SetRGBA(&m_EnergyBar, 0xC0FFFFFF);
	g_pDrawPrim->SetRGBA(&m_ReservedBar, 0xC0FFFFFF);
	g_pDrawPrim->SetRGBA(&m_EnergyTransfer, 0xC0FFFFFF);
	g_pDrawPrim->SetRGBA(&m_HealthBlinkBar, argbWhite);
	LT_POLYFT4* pCap = &m_EnergyCap;
	g_pDrawPrim->SetRGBA(pCap, argbWhite);

	// half a texel in from each edge
	uint32 nWidth, nHeight;
	float fU = 0.0f;
	float fV = 0.0f;
	if (g_pTexInterface->GetTextureDims(m_hEnergyTex, nWidth, nHeight) == LT_OK)
	{
		fU = 0.5f / (float)nWidth;
		fV = 0.5f / (float)nHeight;
	}
	pCap->verts[0].u = fU;
	pCap->verts[0].v = fV;
	pCap->verts[1].u = 1.0f - fU;
	pCap->verts[1].v = fV;
	pCap->verts[2].u = 1.0f - fU;
	pCap->verts[2].v = 1.0f - fV;
	pCap->verts[3].u = fU;
	pCap->verts[3].v = 1.0f - fV;

	g_pDrawPrim->SetRGBA(&m_Plasma[0], 0x40808080);
	g_pDrawPrim->SetRGBA(&m_Plasma[1], 0x40808080);
	g_pDrawPrim->SetRGBA(&m_Plasma[2], 0x40804000);
	g_pDrawPrim->SetRGBA(&m_Plasma[3], 0x40800000);

	m_bHealthBlink = false;

	return LTTRUE;
}

// RETAIL: 0x10029cf0
void CHUDHealthEnergy::Term()
{
	if (m_pHealthStr)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pHealthStr);
		m_pHealthStr = LTNULL;
	}
	if (m_pEnergyStr)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pEnergyStr);
		m_pEnergyStr = LTNULL;
	}
}

// RETAIL: 0x10029d60. NEAR: the last SetUVWH loads the vtable later
void CHUDHealthEnergy::Render()
{
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	SetRenderState();

	if (m_fHealthBlinkTime > 0.0f)
		m_fHealthBlinkTime = 0.0f;
	m_bHealthBlink = false;

	g_pDrawPrim->SetTexture(m_hHUDTex);
	g_pDrawPrim->DrawPrim(&m_Energy);
	g_pDrawPrim->SetTexture(m_hHUDTex);
	g_pDrawPrim->DrawPrim(&m_HealthBG);
	g_pDrawPrim->SetTexture(m_hHUDTex);
	if (m_bHealthBlink)
		g_pDrawPrim->DrawPrim(&m_HealthBlinkBar);
	else
		g_pDrawPrim->DrawPrim(&m_HealthBar);

	// the energy bar flickers
	m_fFlickerTime -= g_pLTClient->GetFrameTime();
	if (m_fFlickerTime <= 0.0f)
	{
		m_bFlicker = !m_bFlicker;
		m_fFlickerTime = GetRandom(0.1f, 2.0f);
	}

	uint8 nWeaponID = g_pTronPlayerStats->GetCurWeapon();
	bool bUnlocked, bCorrupt, bCompiled;
	int nShots;
	g_pTronPlayerStats->GetWeaponStatus(nWeaponID, &bUnlocked, &bCorrupt, &bCompiled, &nShots);
	if (!bCompiled)
	{
		if (m_fFlickerTime > 0.0f)
			g_pDrawPrim->SetALPHA(&m_EnergyBar, 0xFF);
		else
			g_pDrawPrim->SetALPHA(&m_EnergyBar, 0);
		g_pDrawPrim->SetALPHA(&m_Plasma[0], 0);
		g_pDrawPrim->SetALPHA(&m_Plasma[1], 0);
	}
	else
	{
		g_pDrawPrim->SetALPHA(&m_EnergyBar, 0xFF);
		g_pDrawPrim->SetALPHA(&m_Plasma[0], 0x40);
		g_pDrawPrim->SetALPHA(&m_Plasma[1], 0x40);
	}

	g_pDrawPrim->SetTexture(m_hHUDTex);
	g_pDrawPrim->DrawPrim(&m_EnergyBar);

	if (m_fReservedEnergy > 0.0f)
	{
		g_pDrawPrim->SetTexture(m_hHUDTex);
		g_pDrawPrim->DrawPrim(&m_ReservedBar);
	}

	// the plasma moves over the energy bar
	float fEnergy = m_fEnergy / m_fMaxEnergy;
	float fHealth = (float)m_nHealth / (float)m_nMaxHealth;

	m_fPlasmaTime += g_pLTClient->GetFrameTime();
	if (m_fPlasmaTime > 20.0f)
		m_fPlasmaTime -= 20.0f;
	if (m_fPlasmaTime > 20.0f || m_fPlasmaTime < -20.0f)
		m_fPlasmaTime = GetRandom(0.0f, 20.0f);

	float fAngle = MATH_CIRCLE * (m_fPlasmaTime * 0.25f);
	float fSin = (float)sin(fAngle);
	float fCos = (float)cos(fAngle);
	float fCos2 = (float)cos(MATH_CIRCLE * (m_fPlasmaTime * 0.2f));

	g_pDrawPrim->SetUVWH(&m_Plasma[0], 0.0f, fSin * 0.4f + 0.5f, fEnergy * 0.3f, 0.05f);
	float fW = fEnergy * 0.5f;
	float fCos2u = fCos2 * 0.2f;
	g_pDrawPrim->SetUVWH(&m_Plasma[1], fCos2u + 0.25f, fCos * 0.2f + 0.5f, fW, -0.1f);
	g_pDrawPrim->SetUVWH(&m_Plasma[2], 0.0f, 0.5f - fCos * 0.4f, fHealth * 0.3f, 0.05f);
	g_pDrawPrim->SetUVWH(&m_Plasma[3], fSin * 0.2f + 0.25f, fCos2u + 0.5f, fW, -0.1f);

	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_ADD);
	g_pDrawPrim->SetTexture(m_hPlasmaTex);
	g_pDrawPrim->DrawPrim(m_Plasma, 4);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	if (m_pHealthStr)
		m_pHealthStr->Render();
	if (m_pEnergyStr)
		m_pEnergyStr->Render();

	// the energy the crosshair's target needs, marked on the energy bar
	int nRequired = g_pCrosshair->GetEnergyRequired();
	if (!nRequired) return;

	float fLeft = (m_fEnergy - nRequired) / m_fMaxEnergy * m_nEnergyWidth;
	if (fLeft < 1.0f) return;

	float x = m_EnergyBar.verts[1].x - fLeft;
	m_EnergyCap.verts[0].x = m_EnergyCap.verts[3].x = x - 16.0f;
	m_EnergyCap.verts[1].x = m_EnergyCap.verts[2].x = x + 16.0f;
	g_pDrawPrim->SetTexture(m_hEnergyTex);
	g_pDrawPrim->DrawPrim(&m_EnergyCap);

	if (g_pTronPlayerMgr->IsTransferringEnergy1())
	{
		g_pDrawPrim->SetTexture(m_hHUDTex);
		g_pDrawPrim->DrawPrim(&m_EnergyTransfer);
	}
}

// RETAIL: 0x1002a2c0. NEAR: register allocation
void CHUDHealthEnergy::Update()
{
	uint32 nOldHealth = m_nHealth;
	uint32 nOldMaxHealth = m_nMaxHealth;
	float fOldEnergy = m_fEnergy;
	float fOldMaxEnergy = m_fMaxEnergy;
	float fOldReserved = m_fReservedEnergy;

	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	m_nHealth = g_pPlayerStats->GetHealth();
	m_fEnergy = g_pTronPlayerStats->GetFullEnergy();
	m_nMaxHealth = g_pPlayerStats->GetMaxHealth();
	m_fMaxEnergy = g_pTronPlayerStats->GetMaxEnergy();
	m_fReservedEnergy = g_pSubroutineMgr->GetTronUnknown11A4();

	m_bHealthBlink = (nOldHealth != m_nHealth || nOldMaxHealth != m_nMaxHealth);
	bool bEnergyChanged = (fOldEnergy != m_fEnergy || fOldMaxEnergy != m_fMaxEnergy || fOldReserved != m_fReservedEnergy);

	char szBuf[12];

	if (m_bHealthBlink)
	{
		m_Plasma[2].verts[1].x = m_Plasma[2].verts[2].x =
			m_Plasma[3].verts[1].x = m_Plasma[3].verts[2].x =
			m_HealthBar.verts[1].x = m_HealthBar.verts[2].x =
			m_HealthBlinkBar.verts[1].x = m_HealthBlinkBar.verts[2].x =
				(float)m_nHealth / m_nMaxHealth * m_nHealthWidth + m_HealthBar.verts[0].x;

		sprintf(szBuf, "%d", m_nHealth);
		std::wstring sText;
		LTGUIWiden(sText, szBuf);
		if (m_pHealthStr)
		{
			m_pHealthStr->SetText(sText);
			int32 nWidth, nHeight;
			m_pHealthStr->GetDims(&nWidth, &nHeight);
			float fWidth = (float)nWidth;
			float fHeight = (float)nHeight;
			m_pHealthStr->SetPosition((int)(m_HealthTextPos.x - fWidth), (int)(m_HealthTextPos.y - fHeight * 0.5f));
		}
	}

	if (bEnergyChanged)
	{
		float fWidth = (float)m_nEnergyWidth;
		m_Plasma[1].verts[0].x = m_Plasma[1].verts[3].x =
			m_Plasma[0].verts[0].x = m_Plasma[0].verts[3].x =
			m_EnergyBar.verts[3].x = m_EnergyBar.verts[0].x =
				m_EnergyBar.verts[1].x - m_fEnergy / m_fMaxEnergy * fWidth;
		m_ReservedBar.verts[3].x = m_ReservedBar.verts[0].x =
				m_ReservedBar.verts[1].x - m_fReservedEnergy / m_fMaxEnergy * fWidth;

		sprintf(szBuf, "%d", (int)(m_fEnergy - m_fReservedEnergy));
		std::wstring sText;
		LTGUIWiden(sText, szBuf);
		if (m_pEnergyStr)
		{
			m_pEnergyStr->SetText(sText);
			int32 nWidth, nHeight;
			m_pEnergyStr->GetDims(&nWidth, &nHeight);
			m_pEnergyStr->SetPosition((int)m_EnergyTextPos.x, (int)(m_EnergyTextPos.y - (float)nHeight * 0.5f));
		}
	}

	if (g_pTronPlayerMgr->IsTransferringEnergy1())
	{
		float fRequired = (float)g_pTronPlayerMgr->GetEnergyRequired();
		float fDrain = g_pTronPlayerMgr->GetPercentEnergyTransferred() * fRequired;
		m_EnergyTransfer.verts[3].x = m_EnergyTransfer.verts[0].x = m_EnergyBar.verts[0].x;
		m_EnergyTransfer.verts[2].x = m_EnergyTransfer.verts[1].x =
			fDrain / m_fMaxEnergy * m_nEnergyWidth + m_EnergyBar.verts[0].x;
	}
}

// RETAIL: 0x1002a830. NEAR: retail builds the std::wstring from L"100"
// with STLport helpers out of line, and some float scheduling differs
void CHUDHealthEnergy::UpdateLayout()
{
	// RETAIL: 0x1002a830: CTronLayoutMgr's getters (0x100a0600..), in this
	// order; the bar height is CLayoutMgr's (0x100dae00)
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthEnergyFont(nCurrentLayout);
	int nFontSize = ((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthEnergyFontSize(nCurrentLayout);

	LTIntPt pos = ((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthTextOffset(nCurrentLayout);
	float fCenterX = 320.0f * xs;
	m_HealthTextPos.x = (float)pos.x + fCenterX;
	float fBottom = 480.0f * ys;
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: the screen's centre and bottom
	fCenterX = g_pInterfaceResMgr->HUDX(320.0f, 320);
	m_HealthTextPos.x = (float)pos.x + fCenterX;
	fBottom = g_pInterfaceResMgr->HUDY(480.0f, 480);
#endif

	m_HealthTextPos.y = (float)pos.y + fBottom;
	pos = ((CTronLayoutMgr*)g_pLayoutMgr)->GetEnergyTextOffset(nCurrentLayout);
	m_EnergyTextPos.x = (float)pos.x + fCenterX;
	m_EnergyTextPos.y = (float)pos.y + fBottom;

	m_nBarHeight = ((CTronLayoutMgr*)g_pLayoutMgr)->GetBarHeight(nCurrentLayout);
	m_nHealthWidth = ((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthBarWidth(nCurrentLayout);
	m_nEnergyWidth = ((CTronLayoutMgr*)g_pLayoutMgr)->GetEnergyBarWidth(nCurrentLayout);
	m_fLayoutHealthBlinkTime = (float)(int)((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthBlinkTime(nCurrentLayout);

	LTIntPt base = ((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthBGOffset(nCurrentLayout);
	LTRect rc = ((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthBGTex(nCurrentLayout);
	g_pDrawPrim->SetXYWH(&m_Energy, (float)base.x + fCenterX, (float)base.y + fBottom, (float)(rc.right - rc.left), (float)(rc.bottom - rc.top));
	SetUVWH(&m_Energy, rc);

	rc = ((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthTex(nCurrentLayout);
	LTIntPt off = ((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthFGOffset(nCurrentLayout);
	SetUVWH(&m_HealthBar, rc);
	SetUVWH(&m_EnergyTransfer, rc);

	float x = (float)off.x + fCenterX;
	float y = (float)off.y + fBottom;
	g_pDrawPrim->SetXYWH(&m_HealthBar, x, y, (float)m_nHealthWidth, (float)m_nBarHeight);
	g_pDrawPrim->SetXYWH(&m_HealthBlinkBar, x, y, (float)m_nHealthWidth, (float)m_nBarHeight);
	g_pDrawPrim->SetXYWH(&m_Plasma[2], x, y, (float)m_nHealthWidth, (float)m_nBarHeight);
	g_pDrawPrim->SetXYWH(&m_Plasma[3], x, y, (float)m_nHealthWidth, (float)m_nBarHeight);
	SetUVWH(&m_HealthBlinkBar, ((CTronLayoutMgr*)g_pLayoutMgr)->GetHealthBlinkTex(nCurrentLayout));

	off = ((CTronLayoutMgr*)g_pLayoutMgr)->GetEnergyBGOffset(nCurrentLayout);
	rc = ((CTronLayoutMgr*)g_pLayoutMgr)->GetEnergyBGTex(nCurrentLayout);
	float fBGWidth = (float)(rc.right - rc.left);
	g_pDrawPrim->SetXYWH(&m_HealthBG, fCenterX - fBGWidth + off.x, (float)off.y + fBottom, fBGWidth, (float)(rc.bottom - rc.top));
	SetUVWH(&m_HealthBG, rc);

	off = ((CTronLayoutMgr*)g_pLayoutMgr)->GetEnergyFGOffset(nCurrentLayout);
	// RETAIL: 0x1002ad04: each bar's x and width are worked out again
	float fOffY = (float)off.y;
	y = fOffY + fBottom;
	float fOffX = (float)off.x;
	g_pDrawPrim->SetXYWH(&m_EnergyBar, fOffX - (float)m_nEnergyWidth + fCenterX, y, (float)m_nEnergyWidth, (float)m_nBarHeight);
	g_pDrawPrim->SetXYWH(&m_Plasma[0], fOffX - (float)m_nEnergyWidth + fCenterX, y, (float)m_nEnergyWidth, (float)m_nBarHeight);
	g_pDrawPrim->SetXYWH(&m_Plasma[1], fOffX - (float)m_nEnergyWidth + fCenterX, y, (float)m_nEnergyWidth, (float)m_nBarHeight);
	g_pDrawPrim->SetXYWH(&m_EnergyTransfer, fOffX - (float)m_nEnergyWidth + fCenterX, y, (float)m_nEnergyWidth, (float)m_nBarHeight);
	g_pDrawPrim->SetXYWH(&m_EnergyCap, 0.0f, (float)m_nBarHeight + fOffY + fBottom - 32.0f, 32.0f, 32.0f);
	SetUVWH(&m_EnergyBar, ((CTronLayoutMgr*)g_pLayoutMgr)->GetEnergyTex(nCurrentLayout));
	g_pDrawPrim->SetXYWH(&m_ReservedBar, fOffX - (float)m_nEnergyWidth + fCenterX, y, (float)m_nEnergyWidth, (float)m_nBarHeight);
	SetUVWH(&m_ReservedBar, ((CTronLayoutMgr*)g_pLayoutMgr)->GetEnergyDeficitTex(nCurrentLayout));

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nFontSize);

	if (m_pHealthStr)
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pHealthStr);
	m_pHealthStr = g_pLTClient->GetFontManager()->CreatePolyString(pFont, std::wstring(L"100"));
	m_pHealthStr->SetCharScreenHeight(nFontSize);
	int32 nWidth, nHeight;
	m_pHealthStr->GetDims(&nWidth, &nHeight);
	m_pHealthStr->SetPosition((int)(m_HealthTextPos.x - (float)nWidth), (int)(m_HealthTextPos.y - (float)nHeight * 0.5f));
	m_pHealthStr->SetColor(0xC0FFFFFF);

	if (m_pEnergyStr)
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pEnergyStr);
	m_pEnergyStr = g_pLTClient->GetFontManager()->CreatePolyString(pFont, std::wstring(L"100"));
	m_pEnergyStr->SetCharScreenHeight(nFontSize);
	m_pEnergyStr->GetDims(&nWidth, &nHeight);
	m_pEnergyStr->SetPosition((int)m_EnergyTextPos.x, (int)(m_EnergyTextPos.y - (float)nHeight * 0.5f));
	m_pEnergyStr->SetColor(0xC0FFFFFF);
}

// RETAIL: 0x1002b390
void CHUDHealthEnergy::SetUVWH(LT_POLYFT4 * pPrim, LTRect rect)
{
	rect.right -= rect.left;
	rect.bottom -= rect.top;

	g_pDrawPrim->SetUVWH(pPrim, rect.left / 256.0f, rect.top / 256.0f, rect.right / 256.0f, rect.bottom / 256.0f);
}
