// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCycleSpeedometer.cpp
//
// PURPOSE : HUDItem showing the speed of the player's light cycle
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1002f770..
// 0x1002fe15; HUDLightCycleSpeedometer.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDLightCycleSpeedometer.h"
#include "InterfaceMgr.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "LCHUDHelper.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "LightCycleMgr.h"

// RETAIL: 0x1002f850
CHUDLightCycleSpeedometer::CHUDLightCycleSpeedometer() :
	m_pText(LTNULL)
{
	// RETAIL: Tron's update flag for it
	m_UpdateFlags = (eHUDUpdateFlag)0x40000000;
}

// RETAIL: 0x1002f8c0
LTBOOL CHUDLightCycleSpeedometer::Init()
{
	LCHUDHelper* pHelper = dynamic_cast<CTronLayoutMgr*>(g_pLayoutMgr)->GetLCHUDHelper();
	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);

	std::string sFrame = pHelper->GetSpeedometerTexturedFrameName(nCurrentLayout);
	m_Frame.Init(sFrame.c_str());

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(pHelper->GetLivesTextOffset(nCurrentLayout));
	m_pText = g_pFontManager->CreateFormattedPolyString(pFont, std::wstring(L"LightSpeed"), 0.0f, 0.0f, CUI_HALIGN_RIGHT);
	m_pText->SetDropShadow(true);

	m_bTronUnknown42C = false;

	return LTTRUE;
}

// RETAIL: 0x1002fa50
void CHUDLightCycleSpeedometer::Render()
{
	if (!dynamic_cast<CTronInterfaceMgr*>(g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	m_Frame.Render();
	m_pText->Render();
}

// RETAIL: 0x1002faa0
void CHUDLightCycleSpeedometer::Update()
{
	UpdateText();
	UpdateLayout();
}

// RETAIL: 0x1002fac0
void CHUDLightCycleSpeedometer::UpdateLayout()
{
	LCHUDHelper* pHelper = dynamic_cast<CTronLayoutMgr*>(g_pLayoutMgr)->GetLCHUDHelper();
	int nCurrentLayout = GetConsoleInt("HUDLayout", 0);

	LTFLOAT fXRatio = g_pInterfaceResMgr->GetXRatio();
	LTFLOAT fYRatio = g_pInterfaceResMgr->GetYRatio();

	// RETAIL: the y position is scaled by the x ratio too
	// RETAIL: 0x1002fb14 reads SpeedometerPos
	LTIntPt pos = pHelper->GetSpeedometerPos(nCurrentLayout);
	pos.x = (int)(pos.x * fXRatio);
	pos.y = (int)(pos.y * fXRatio);

	int nTextHeight = (int)(pHelper->GetRosterLineOffset(nCurrentLayout) * fYRatio);
	int nTextOffset = (int)(pHelper->GetSpeedometerFrameHeight(nCurrentLayout) * fYRatio);
	int nTextBorder = (int)(pHelper->GetSpeedometerHBorder(nCurrentLayout) * fXRatio);
	int nHalfWidth = (int)(pHelper->GetSpeedometerTextHeight(nCurrentLayout) * fXRatio);
	int nHeight = (int)(pHelper->GetSpeedometerVBorder(nCurrentLayout) * fXRatio);

	m_pText->SetCharScreenHeight(nTextHeight);
	m_pText->SetPosition(pos.x - nTextBorder, pos.y + nTextOffset);

	m_Frame.SetRect(LTRect(pos.x - nHalfWidth, pos.y, pos.x + nHalfWidth, pos.y + nHeight));
}

// RETAIL: 0x1002fc30: the speed, red while turbo is on or in a speed zone
void CHUDLightCycleSpeedometer::UpdateText()
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->GetLivingCyclist(g_pMoveMgr->GetObject());

	char szText[256];
	if (pCyclist && !g_pMoveMgr->GetVehicleMgr()->GetTronUnknown2158())
	{
		sprintf(szText, "%d", (int)pCyclist->fSpeed);
	}
	else
	{
		sprintf(szText, "0");
	}

	std::wstring sText;
	LTGUIWiden(sText, szText);
	m_pText->SetText(sText);

	if (pCyclist && !g_pMoveMgr->GetVehicleMgr()->GetTronUnknown2158() &&
		(pCyclist->bTurboCursed || pCyclist->bTurbo ||
		 g_pPlayerMgr->GetCurContainerCode() == CC_LIGHTCYCLE_SPEED_ZONE ||
		 g_pPlayerMgr->GetCurContainerCode() == CC_LIGHTCYCLE_SLOW_ZONE))
	{
		m_pText->SetColor(0xFFFF0000);
	}
	else
	{
		m_pText->SetColor(argbWhite);
	}
}
