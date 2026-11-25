// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCrosshair.cpp
//
// PURPOSE : HUDItem to display crosshair
//
// RETAIL: Tron's, rewritten from retail CShell.dll 0x100240f0..0x10024f80
// (HUDCrosshair.h)
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ClientRes.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDCrosshair.h"
#include "TronPlayerStats.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"
#include "BodyFX.h"
#include "TronGameClientShell.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "TargetMgr.h"
#include "TronTargetMgr.h"
#include "TronPlayerMgr.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"

namespace
{
	char * szReticleTex[NUM_RETICLES] = {
		"Interface\\HUD\\reticle.dtx",
		"Interface\\HUD\\reticle_innocent.dtx",
		"Interface\\HUD\\reticle_activate.dtx",
	};
}

//******************************************************************************************
//** HUD crosshair
//******************************************************************************************
CHUDCrosshair::CHUDCrosshair()
{
	m_UpdateFlags = kHUDFrame;
	m_bEnabled = true;
	m_pStr = LTNULL;
	m_pInfoStr = LTNULL;
	m_pDbgStr = LTNULL;
	m_fScale = 0.0f;
	m_nEnergyRequired = 0;
}

LTBOOL CHUDCrosshair::Init()
{
	int i;
	for (i = 0; i < NUM_RETICLES; i++)
	{
		m_hReticleTex[i] = g_pInterfaceResMgr->GetTexture(szReticleTex[i]);
	}
	LT_POLYFT4* pPrim = &m_ReticlePrim;
	g_pDrawPrim->SetRGBA(pPrim, argbWhite);

	// RETAIL: the reticle's texture coordinates are inset half a texel
	float fU = 0.0f;
	float fV = 0.0f;
	uint32 nWidth, nHeight;
	if (g_pTexInterface->GetTextureDims(m_hReticleTex[RETICLE_NORMAL], nWidth, nHeight) == LT_OK)
	{
		fU = 0.5f / (float)nWidth;
		fV = 0.5f / (float)nHeight;
	}
	pPrim->verts[0].u = fU;
	pPrim->verts[0].v = fV;
	pPrim->verts[1].u = 1.0f - fU;
	pPrim->verts[1].v = fV;
	pPrim->verts[2].u = 1.0f - fU;
	pPrim->verts[2].v = 1.0f - fV;
	pPrim->verts[3].u = fU;
	pPrim->verts[3].v = 1.0f - fV;

	char szTex[256];
	for (i = 0; i < NUM_PSET_ICONS; i++)
	{
		sprintf(szTex, "Interface\\HUD\\reticle_psets_empty_%d.dtx", i + 1);
		m_hPSetEmptyTex[i] = g_pInterfaceResMgr->GetTexture(szTex);
		sprintf(szTex, "Interface\\HUD\\reticle_psets_need_%d.dtx", i + 1);
		m_hPSetNeedTex[i] = g_pInterfaceResMgr->GetTexture(szTex);
		sprintf(szTex, "Interface\\HUD\\reticle_psets_full_%d.dtx", i + 1);
		m_hPSetFullTex[i] = g_pInterfaceResMgr->GetTexture(szTex);
	}

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_StrPos = LTIntPt(320,220);
	m_nStrSz = 16;
	m_nStrJust = 1;
	m_StrColor = argbWhite;
	m_StrDisColor = 0x80808080;

	m_pStr = g_pFontManager->CreateFormattedPolyString(pFont,std::wstring(),0.0f,0.0f);
	m_pStr->SetDropShadow(true);
	m_pStr->SetColor(m_StrColor);

	m_DbgPos = LTIntPt(360,200);
	m_nDbgSz = 12;
	m_nDbgWidth = 220;
	m_nDbgJust = 0;
	m_DbgColor = argbWhite;

	m_pDbgStr = g_pFontManager->CreateFormattedPolyString(pFont,std::wstring(),0.0f,0.0f);
	m_pDbgStr->SetColor(m_DbgColor);
	m_pDbgStr->SetDropShadow(true);
	m_pDbgStr->SetWrapWidth(m_nDbgWidth);

	m_InfoPos = LTIntPt(320, 260);
	m_nInfoSz = 12;
	m_nInfoWidth = 200;
	m_nInfoJust = 1;
	m_InfoColor = 0x8000FF00;
	m_InfoBadColor = 0x80FF0000;

	m_pInfoStr = g_pFontManager->CreateFormattedPolyString(pFont,std::wstring(),0.0f,0.0f);
	m_pInfoStr->SetDropShadow(true);
	m_pInfoStr->SetColor(m_InfoColor);

	UpdateLayout();

	return LTTRUE;
}

void CHUDCrosshair::Term()
{
	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}
	if (m_pDbgStr)
	{
		g_pFontManager->DestroyPolyString(m_pDbgStr);
        m_pDbgStr=LTNULL;
	}
	if (m_pInfoStr)
	{
		g_pFontManager->DestroyPolyString(m_pInfoStr);
        m_pInfoStr=LTNULL;
	}
}

void CHUDCrosshair::Render()
{
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	SetRenderState();
	if (g_pInterfaceMgr->IsOverlayActive(OVM_SCOPE))
	{
		RenderScope();
		return;
	}

	if (!m_bEnabled)
		return;

	RenderArmed();
	RenderPermissions();

	if (m_pStr)
	{
		m_pStr->SetPosition(m_StrScreenPos.x,m_StrScreenPos.y);
		m_pStr->SetColor(m_StrColor);
		m_pStr->Render();
	}

	if (m_pDbgStr)
	{
		m_pDbgStr->SetColor(m_DbgColor);
		m_pDbgStr->SetPosition(m_DbgScreenPos.x,m_DbgScreenPos.y);
		m_pDbgStr->Render();
	}

	if (m_pInfoStr)
	{
		m_pInfoStr->SetColor(m_InfoColor);
		m_pInfoStr->SetPosition(m_InfoScreenPos.x,m_InfoScreenPos.y);
		m_pInfoStr->Render();
	}
}

void CHUDCrosshair::Update()
{
	if (!m_bEnabled)
		return;

	float fScale = g_pInterfaceResMgr->GetXRatio();
	if (fScale != m_fScale)
	{
		m_fScale = fScale;
		UpdateLayout();
	}

	HOBJECT hObj = g_pPlayerMgr->GetTargetMgr()->GetTargetObject();
	uint16  nID = g_pPlayerMgr->GetTargetMgr()->GetTargetStringID();

	std::wstring sDbg;
	LTGUIWiden(sDbg, g_pPlayerMgr->GetTargetMgr()->GetDebugString());
	if (sDbg != m_pDbgStr->GetText())
	{
		m_pDbgStr->SetText(sDbg);
		PositionString(m_pDbgStr, m_nDbgJust, m_DbgPos, m_nDbgSz, &m_DbgScreenPos);
	}

	if (hObj == m_hObj && nID == m_nString && nID)
		return;

	m_nEnergyRequired = 0;
	m_hObj = hObj;
	m_nString = nID;
	m_pStr->SetText(g_pPlayerMgr->GetTargetMgr()->GetTargetString());
	PositionString(m_pStr, m_nStrJust, m_StrPos, m_nStrSz, &m_StrScreenPos);

	if (g_pTronPlayerMgr->GetTronTargetMgr()->IsEnergyRequired())
	{
		m_nEnergyRequired = g_pTronPlayerMgr->GetTronTargetMgr()->GetEnergyRequired();
		m_pInfoStr->SetText(FormatTempString(IDS_REQUIRES_ENERGY,m_nEnergyRequired));
		PositionString(m_pInfoStr, m_nInfoJust, m_InfoPos, m_nInfoSz, &m_InfoScreenPos);
		if ((int)g_pTronPlayerStats->GetEnergy() > m_nEnergyRequired)
			m_pInfoStr->SetColor(m_InfoColor);
		else
			m_pInfoStr->SetColor(m_InfoBadColor);
	}
	else
	{
		m_pInfoStr->SetText(std::wstring(L" "));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDCrosshair::RenderArmed()
//
//	PURPOSE:	Draw the reticle at the screen's centre, spun about it once
//				a second while the target can be activated
//
// ----------------------------------------------------------------------- //
void CHUDCrosshair::RenderArmed()
{
	float fCX = (float)m_ScreenCenter.x;
	float fCY = (float)m_ScreenCenter.y;
	float fSize = m_fScale * 0.625f;
	float fWidth = fSize * 64.0f;
	float fRadius = fSize * 32.0f;
	float fTop = fCY - fRadius;
	float fLeft = fCX - fRadius;
	g_pDrawPrim->SetXYWH(&m_ReticlePrim, fLeft, fTop, fWidth, fWidth);

	uint32 dwUserFlags = 0;
	if (m_hObj)
		g_pCommonLT->GetObjectFlags(m_hObj, OFT_User, dwUserFlags);

	if (g_pTronPlayerMgr->GetTronTargetMgr()->IsTronUnknown876())
	{
		g_pDrawPrim->SetTexture(m_hReticleTex[RETICLE_INNOCENT]);
		g_pDrawPrim->DrawPrim(&m_ReticlePrim, 1);
		return;
	}

	if (dwUserFlags & USRFLG_CAN_ACTIVATE)
	{
		g_pDrawPrim->SetTexture(m_hReticleTex[RETICLE_ACTIVATE]);

		float fTime = g_pLTClient->GetTime();
		float fAngle = (fTime - (float)floor(fTime)) * MATH_CIRCLE;
		float fCos = (float)cos(fAngle) * 32.0f;
		float fSin = (float)sin(fAngle) * 32.0f;

		m_ReticlePrim.verts[0].x = fCos + fCX;
		m_ReticlePrim.verts[0].y = fCY - fSin;
		m_ReticlePrim.verts[1].x = fSin + fCX;
		m_ReticlePrim.verts[1].y = fCos + fCY;
		m_ReticlePrim.verts[2].x = fCX - fCos;
		m_ReticlePrim.verts[2].y = fSin + fCY;
		m_ReticlePrim.verts[3].x = fCX - fSin;
		m_ReticlePrim.verts[3].y = fCY - fCos;
		g_pDrawPrim->DrawPrim(&m_ReticlePrim, 1);

		g_pDrawPrim->SetXYWH(&m_ReticlePrim, fLeft, fTop, fWidth, fWidth);
		return;
	}

	g_pDrawPrim->SetTexture(m_hReticleTex[RETICLE_NORMAL]);
	g_pDrawPrim->DrawPrim(&m_ReticlePrim, 1);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDCrosshair::RenderPermissions()
//
//	PURPOSE:	Draw the permission sets the target needs over the reticle:
//				each needed set held or not (RETAIL: 0x10024c00; the name
//				is ours)
//
// ----------------------------------------------------------------------- //
void CHUDCrosshair::RenderPermissions()
{
	CTronTargetMgr* pTargetMgr = g_pTronPlayerMgr->GetTronTargetMgr();
	if (!pTargetMgr->IsPermissionRequired()) return;

	uint8 nRequired = pTargetMgr->GetPermissionRequired();
	uint8 nPermissions, nRequiredPermissions;
	g_pTronPlayerStats->GetPermissions(nPermissions, nRequiredPermissions);

	for (int i = 0; i < NUM_PSET_ICONS; i++)
	{
		int nBit = 1 << i;
		if (nRequired & nBit)
		{
			if (nPermissions & nBit)
				g_pDrawPrim->SetTexture(m_hPSetFullTex[i]);
			else
				g_pDrawPrim->SetTexture(m_hPSetNeedTex[i]);
		}
		else
		{
			g_pDrawPrim->SetTexture(m_hPSetEmptyTex[i]);
		}
		g_pDrawPrim->DrawPrim(&m_ReticlePrim, 1);
	}
}

// RETAIL: 0x10024c00 is near only for CTronTargetMgr's layout (+0x874)

// RETAIL: empty (folded with other empty functions, 0x100abb80)
void CHUDCrosshair::RenderScope()
{
}

void CHUDCrosshair::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_ScreenCenter.x = g_pInterfaceResMgr->GetScreenWidth() / 2;
	m_ScreenCenter.y = g_pInterfaceResMgr->GetScreenHeight() / 2;
	m_fScale = g_pInterfaceResMgr->GetXRatio();

	LTIntPt pos = g_pLayoutMgr->GetActivationTextPos(nCurrentLayout);
	if (pos.x > 0)
		m_StrPos = pos;

	uint8 nTmp = g_pLayoutMgr->GetActivationTextSize(nCurrentLayout);
	if (nTmp > 0)
		m_nStrSz = nTmp;

	m_nStrJust = g_pLayoutMgr->GetActivationTextJustify(nCurrentLayout);

	uint32 color = g_pLayoutMgr->GetActivationTextColor(nCurrentLayout);
	if (color > 0)
		m_StrColor = color;

	color =	g_pLayoutMgr->GetActivationTextDisabledColor(nCurrentLayout);
	if (color > 0)
		m_StrDisColor = color;

	pos = g_pLayoutMgr->GetDebugTextPos(nCurrentLayout);
	if (pos.x > 0)
		m_DbgPos = pos;

	nTmp = g_pLayoutMgr->GetDebugTextSize(nCurrentLayout);
	if (nTmp > 0)
		m_nDbgSz = nTmp;

	uint16 nWid = g_pLayoutMgr->GetDebugTextWidth(nCurrentLayout);
	if (nWid > 0)
		m_nDbgWidth = nWid;

	m_nDbgJust = g_pLayoutMgr->GetDebugTextJustify(nCurrentLayout);

	color = g_pLayoutMgr->GetDebugTextColor(nCurrentLayout);
	if (color > 0)
		m_DbgColor = color;

	pos = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextPos(nCurrentLayout);
	if (pos.x > 0)
		m_InfoPos = pos;

	nTmp = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextSize(nCurrentLayout);
	if (nTmp > 0)
		m_nInfoSz = nTmp;

	nWid = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextWidth(nCurrentLayout);
	if (nWid > 0)
		m_nInfoWidth = nWid;

	m_nInfoJust = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextJustify(nCurrentLayout);

	color = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextColor(nCurrentLayout);
	if (color > 0)
		m_InfoColor = color;

	color = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextBadColor(nCurrentLayout);
	if (color > 0)
		m_InfoBadColor = color;
}

void CHUDCrosshair::PositionString(CUIFormattedPolyString* pStr, uint8 nJust, LTIntPt pos, uint8 nSize, LTIntPt* pScreenPos)
{
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: the strings round the crosshair, on the centred page
	float fX = g_pInterfaceResMgr->PageX((float)pos.x);
	float fY = g_pInterfaceResMgr->PageY((float)pos.y);
#else
	float fX = (float)pos.x * m_fScale;
	float fY = (float)pos.y * m_fScale;
#endif
	float fSize = (float)nSize * m_fScale;


	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight((int)fSize);
	pStr->SetFont(pFont);
	pStr->SetCharScreenHeight((uint8)fSize);

	int32 nWidth, nHeight;
	pStr->GetDims(&nWidth, &nHeight);

	pScreenPos->y = (int)fY;
	switch (nJust)
	{
	case 0:
		pScreenPos->x = (int)fX;
		pStr->SetPosition(pScreenPos->x, pScreenPos->y);
		break;
	case 1:
		pScreenPos->x = (int)fX - nWidth;
		pStr->SetPosition((int)(fX - (float)nWidth), pScreenPos->y);
		break;
	case 2:
		pScreenPos->x = (int)(fX - (float)(nWidth / 2));
		pStr->SetPosition(pScreenPos->x, pScreenPos->y);
		break;
	}
}
