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

	// RETAIL: the speedometer's position. Near: CTronLayoutMgr's helper is
	// at +0x116c in retail (the layout manager is larger here)
	LTIntPt pos = g_pTronLayoutMgr->GetLCHUDHelper()->GetSpeedometerPos(nCurrentLayout);

	fWidth = fWidth * 0.625f * fXRatio;
	fHeight = fHeight * 0.625f * fYRatio;

	g_pDrawPrim->SetXYWH(&m_Poly, (LTFLOAT)pos.x * fXRatio - fWidth * 0.5f, (LTFLOAT)pos.y * fYRatio - fHeight * 0.5f, fWidth, fHeight);
}

// RETAIL: 0x1002de10
void CHUDLightCyclePowerupIcon::UpdatePowerup()
{
	// PORT: stand-in. Retail finds the player's light cycle (by the move
	// manager's object, in the client's light cycle manager: 0x100ff280,
	// not in this tree yet) and, while it has a power-up (its +0xf0 is not
	// 12), shows that power-up's icon (LCHUDHelper::GetPowerupIconTexture)
	m_bShow = false;
}
