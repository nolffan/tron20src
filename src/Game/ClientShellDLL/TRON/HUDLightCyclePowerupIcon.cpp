// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCyclePowerupIcon.cpp
//
// PURPOSE : HUDItem showing the icon of the light cycle's power-up
//
// RETAIL: Tron 2.0's, written from retail CShell.dll
// (HUDLightCyclePowerupIcon.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDLightCyclePowerupIcon.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "LCHUDHelper.h"
#include "ltquaduvutils.h"
#include "CMoveMgr.h"
#include "TRON/LightCycleMgr.h"

// RETAIL: 0x1002db30
CHUDLightCyclePowerupIcon::CHUDLightCyclePowerupIcon() :
	m_bShow(false),
	m_hIcon(LTNULL)
{
	m_UpdateFlags = 0x40;	// RETAIL: Tron's flag for it
}

// RETAIL: 0x1002db80
LTBOOL CHUDLightCyclePowerupIcon::Init()
{
	m_hIcon = g_pInterfaceResMgr->GetTexture("");
	g_pDrawPrim->SetRGBA(&m_Poly, argbWhite);
	SetupQuadUVs(m_Poly, m_hIcon, 0.0f, 0.0f, 1.0f, 1.0f);
	return LTTRUE;
}

// RETAIL: 0x1002dc50
void CHUDLightCyclePowerupIcon::Render()
{
	// RETAIL: near only for CTronInterfaceMgr's layout (+0x28b14 in retail)
	if (!dynamic_cast<CTronInterfaceMgr*>(g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled() || !m_bShow) return;

	SetRenderState();

	g_pDrawPrim->SetTexture(m_hIcon);
	g_pDrawPrim->DrawPrim(&m_Poly, 1);
}

// RETAIL: 0x1002dcc0
void CHUDLightCyclePowerupIcon::Update()
{
	UpdatePowerup();
	UpdateLayout();
}

// RETAIL: 0x1002dce0
void CHUDLightCyclePowerupIcon::UpdateLayout()
{
	uint32 nWidth, nHeight;
	g_pTexInterface->GetTextureDims(m_hIcon, nWidth, nHeight);
	LTFLOAT fWidth = (LTFLOAT)nWidth;
	LTFLOAT fHeight = (LTFLOAT)nHeight;

	LTFLOAT fXRatio = g_pInterfaceResMgr->GetXRatio();
	LTFLOAT fYRatio = g_pInterfaceResMgr->GetYRatio();

	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);

	// RETAIL: retail reads PowerupIconPos here (0x1002dd7c)
	LTIntPt pos = g_pTronLayoutMgr->GetLCHUDHelper()->GetPowerupIconPos(nCurrentLayout);

	// NEAR: retail keeps the four floats in other stack slots (tried the
	// products grouped and the declarations interleaved)
	fWidth *= fXRatio * 0.625f;
	fHeight *= fYRatio * 0.625f;

	g_pDrawPrim->SetXYWH(&m_Poly, (LTFLOAT)pos.x * fXRatio - fWidth * 0.5f, (LTFLOAT)pos.y * fYRatio - fHeight * 0.5f, fWidth, fHeight);
}

// RETAIL: 0x1002de10
void CHUDLightCyclePowerupIcon::UpdatePowerup()
{
	// RETAIL: the player's light cycle, while it has a power-up
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(g_pMoveMgr->GetObject());
	if (IsEnabled() && pCyclist && pCyclist->nPowerup != LC_NO_POWERUP)
	{
		m_bShow = true;
		int nCurrentLayout = GetConsoleInt("HUDLayout", 0);
		m_hIcon = g_pInterfaceResMgr->GetTexture(g_pTronLayoutMgr->GetLCHUDHelper()->GetPowerupIconTexture(nCurrentLayout, pCyclist->nPowerup).c_str());
	}
	else
	{
		m_bShow = false;
	}
}
