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
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"

namespace
{
	// PORT: build 69's textures: retail takes each piece's two from its
	// subroutine (CSubroutineMgr 0x10095210; TronSubroutine +0x195, +0x1d5),
	// which this tree's SubroutineMgr does not read
	char * szPieceTex[] =
	{
		"Interface\\hud\\protection_submask.dtx",
		"Interface\\hud\\protection_truncate_plate.dtx",
		"Interface\\hud\\protection_peripheral_seal.dtx",
		"Interface\\hud\\protection_support_safeguard.dtx",
		"Interface\\hud\\protection_base_damping.dtx",
		"Interface\\hud\\protection_viral_shield.dtx",
	};
}

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
	m_fLayoutScale = 1000.0f;
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

void CHUDArmor::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	LTIntPt offset = g_pLayoutMgr->GetArmorIconOffset(nCurrentLayout);

	for (int i = 0; i < 6; i++)
	{
		// PORT: see szPieceTex
		m_hArmorPieceTex[i] = g_pInterfaceResMgr->GetTexture(szPieceTex[i]);
		m_hCorruptPieceTex[i] = m_hArmorPieceTex[i];
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

	m_fLayoutScale = xs;
}
