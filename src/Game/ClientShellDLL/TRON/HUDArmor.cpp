// ----------------------------------------------------------------------- //
//
// MODULE  : HUDArmor.cpp
//
// PURPOSE : HUDItem to display Jet's current armor
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


//******************************************************************************************
//**
//** HUD Armor display
//**
//******************************************************************************************

CHUDArmor::CHUDArmor()
{
	m_UpdateFlags = kHUDArmor;
	m_hBaseTex = LTNULL;
	for (int i = 0; i < 6; i++)
	{
		m_hArmorPieceTex[i] = LTNULL;
		m_hCorruptPieceTex[i] = LTNULL;
	}
	m_fScale = 0.0f;
	m_fTop = 1000.0f;
}


LTBOOL CHUDArmor::Init()
{
	UpdateLayout();

	g_pDrawPrim->SetRGBA(&m_Base, argbWhite);
	m_hBaseTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\armor_bg.dtx");
	SetupQuadUVs(m_Base, m_hBaseTex, 0.0f, 0.0f, 1.0f, 1.0f);

	for (int i = 0; i < 6; i++)
	{
		g_pDrawPrim->SetRGBA(&m_ArmorPiece[i], argbWhite);
		SetupQuadUVs(m_ArmorPiece[i], m_hBaseTex, 0.0f, 0.0f, 1.0f, 1.0f);
	}

	return LTTRUE;
}

void CHUDArmor::Term()
{
	// nothing to do here
}

void CHUDArmor::Render()
{
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	SetRenderState();

	// Render the base
	g_pDrawPrim->SetTexture(m_hBaseTex);
	g_pDrawPrim->DrawPrim(&m_Base, 1);

	// Render the pieces we have
	for (int i = 0; i < 6; i++)
	{
		if (m_bHavePiece[i] && m_hArmorPieceTex[i])
		{
			HTEXTURE hTex = m_hArmorPieceTex[i];
			if (m_bCorrupt[i])
			{
				hTex = m_hCorruptPieceTex[i];
				if (!hTex) continue;
			}

			g_pDrawPrim->SetTexture(hTex);
			g_pDrawPrim->DrawPrim(&m_ArmorPiece[i], 1);
		}
	}
}

void CHUDArmor::Update()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	for (int i = 0; i < 6; i++)
	{
		m_bHavePiece[i] = false;
		m_bCorrupt[i] = false;

		PlayerSubroutine* pSub = g_pSubroutineMgr->HaveArmorPiece(i);
		if (pSub)
		{
			m_bHavePiece[i] = true;
			if (pSub->eState == SUBSTATE_CORRUPT)
				m_bCorrupt[i] = true;
		}
	}
}

// RETAIL: 0x100221b0. NEAR: register allocation only (retail keeps 0 in
// ebp and the subroutine on the stack)
void CHUDArmor::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	// RETAIL: 0x100221e9: CTronLayoutMgr::GetArmorOffset (0x100a1280)
	LTIntPt offset = ((CTronLayoutMgr*)g_pLayoutMgr)->GetArmorOffset(nCurrentLayout);

	// each piece's two textures are its armor subroutine's
	for (int i = 0; i < 6; i++)
	{
		TronSubroutine* pSub = g_pSubroutineMgr->GetArmorSubroutine(i);
		m_hArmorPieceTex[i] = g_pInterfaceResMgr->GetTexture(pSub->szArmorTex);
		m_hCorruptPieceTex[i] = g_pInterfaceResMgr->GetTexture(pSub->szCorruptTex);
	}

	// from the bottom right corner
	uint32 nWidth, nHeight;
	g_pTexInterface->GetTextureDims(m_hBaseTex, nWidth, nHeight);
	float x = (640.0f * xs) - (int)(offset.x + nWidth);
	float y = (480.0f * ys) - (int)(offset.y + nHeight);

	g_pDrawPrim->SetXYWH(&m_Base, x, y, (float)nWidth, (float)nHeight);
	for (int j = 0; j < 6; j++)
	{
		g_pDrawPrim->SetXYWH(&m_ArmorPiece[j], x, y, (float)nWidth, (float)nHeight);
	}

	// RETAIL: 0x10022336: the top of the item, which CHUDSubChooser::Arrange
	// (0x1003bad0) uses as the bottom of its column
	m_fTop = y;
}
