// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPermissions.cpp
//
// PURPOSE : HUDItem to display Jet's current permission sets
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "ltquaduvutils.h"

// RETAIL: the layout manager as a CTronLayoutMgr, without the dynamic cast
#define g_pTronLayoutMgrStatic ((CTronLayoutMgr*)g_pLayoutMgr)

//******************************************************************************************
//**
//** HUD Permission set display
//**
//******************************************************************************************

// RETAIL: Tron's (retail 0x10030b30)
CHUDPermissions::CHUDPermissions()
{
	// Make this something new
	m_UpdateFlags = kHUDPermissions;

	m_hBaseTex = LTNULL;
	for (int i = 0; i < 8; i++)
	{
		m_hHaveTex[i] = LTNULL;
	}
	m_fScale = 0.0f;
}


// RETAIL: Tron's (retail 0x10030be0)
LTBOOL CHUDPermissions::Init()
{
	UpdateLayout();

	m_nPermissions = 0;

	// Load textures here
	m_hBaseTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\pset_base.dtx");
	g_pDrawPrim->SetRGBA(&m_Base,argbWhite);
	SetupQuadUVs(m_Base, m_hBaseTex, 0.0f, 0.0f, 1.0f, 1.0f);

	for (int i = 0; i < 8; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Have[i],argbWhite);

		char buf[256];

		sprintf(buf, "interface\\hud\\pset_blue_%d.dtx",i+1);
		m_hHaveTex[i] = g_pInterfaceResMgr->GetTexture(buf);
		SetupQuadUVs(m_Have[i], m_hHaveTex[i], 0.0f, 0.0f, 1.0f, 1.0f);
	}
	return LTTRUE;
}

void CHUDPermissions::Term()
{
	// Free the 17 textures
	// nothing to do here
}

// RETAIL: Tron's (retail 0x10030db0): only while the HUD is up, the new
// pieces flashing
void CHUDPermissions::Render()
{
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B14() || !IsEnabled()) return;

	SetRenderState();
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	if (m_nFlashCount)
	{
		m_fFlashTime -= g_pLTClient->GetFrameTime();
		if (m_fFlashTime < 0.0f)
		{
			m_fFlashTime += 1.0f;
			if (--m_nFlashCount == 0)
			{
				m_nNewPermissions = 0;
			}
		}
	}

	// Render the base
	g_pDrawPrim->SetTexture(m_hBaseTex);
	g_pDrawPrim->DrawPrim(&m_Base);

	// Render the correct pieces
	for (int i = 0; i < 8; i++)
	{
		// Do we have it?
		if (m_nPermissions & (1 << i))
		{
			if (m_nNewPermissions & (1 << i))
			{
				float fFlash = m_fFlashTime * 2.0f;
				if (fFlash > 1.0f)
					fFlash -= 1.0f;
				g_pDrawPrim->SetALPHA(&m_Have[i], (uint8)((1.0f - fFlash) * 255.0f));
			}
			else
			{
				g_pDrawPrim->SetALPHA(&m_Have[i], 0xFF);
			}

			// Render the piece
			g_pDrawPrim->SetTexture(m_hHaveTex[i]);
			g_pDrawPrim->DrawPrim(&m_Have[i]);
		}
	}
}

// RETAIL: Tron's (retail 0x10030f10): the pieces new since the last update
// flash 8 times
void CHUDPermissions::Update()
{
	// Grab the new stats from PlayerStats
	uint8 nOldPermissions = m_nPermissions;
	uint8 nRequiredPermissions;
	g_pTronPlayerStats->GetPermissions(m_nPermissions, nRequiredPermissions);

	uint8 nFlashing = m_nNewPermissions;
	m_nNewPermissions = (nOldPermissions | m_nPermissions) - nOldPermissions;
	if (m_nNewPermissions)
	{
		if (!m_nFlashCount)
		{
			m_fFlashTime = 1.0f;
		}
		m_nFlashCount = 8;
	}
	m_nNewPermissions |= nFlashing;

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}
}

// RETAIL: Tron's (retail 0x10030f80): the pieces all lie on the base, by
// the layout's Pset values
void CHUDPermissions::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	LTIntPt offset = g_pTronLayoutMgrStatic->GetPsetOffset(nCurrentLayout);
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();
	float xw = (float)(int)g_pTronLayoutMgrStatic->GetPsetWidth(nCurrentLayout);
	float yw = (float)(int)g_pTronLayoutMgrStatic->GetPsetHeight(nCurrentLayout);

	float x = (320.0f * xs) - (xw * 0.5f) + offset.x;
	float y = (480.0f * ys) - yw + offset.y;

	g_pDrawPrim->SetXYWH( &m_Base, x, y, xw, yw );
	for (int i = 0; i < 8; i++)
	{
		g_pDrawPrim->SetXYWH( &m_Have[i], x, y, xw, yw );
	}
}
