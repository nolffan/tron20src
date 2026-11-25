// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDamage.cpp
//
// PURPOSE : HUDItem to display player air meter
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "DamageFxMgr.h"
#include "InterfaceMgr.h"

//******************************************************************************************
//**
//** HUD Air display
//**
//******************************************************************************************

CHUDDamage::CHUDDamage()
{
	m_UpdateFlags = kHUDDamage;
	m_hIcon = LTNULL;
	m_Poly	= LTNULL;
	m_bDraw = LTTRUE;
	m_nIconHt = 0;
}

CHUDDamage::~CHUDDamage()
{
	debug_deletea( m_hIcon );
	debug_deletea( m_Poly );
}


LTBOOL CHUDDamage::Init()
{
	if( !g_pDamageFXMgr )
		return LTFALSE;

	int nNumDmgFX = g_pDamageFXMgr->GetNumDamageFX();
	
	// Alloc the texture and poly arrays

	m_hIcon = debug_newa( HTEXTURE, nNumDmgFX );
	m_Poly	= debug_newa( LTPoly_GT4, nNumDmgFX );

	for( int i = 0; i < nNumDmgFX; ++i )
	{
		m_hIcon[i] = g_pInterfaceResMgr->GetTexture( g_pDamageFXMgr->GetDamageFX( i )->m_szIcon );

		g_pDrawPrim->SetRGBA( &m_Poly[i], argbWhite );
		SetupQuadUVs( m_Poly[i], m_hIcon[i], 0.0f, 0.0f, 1.0f, 1.0f );
	}

	UpdateLayout();
	return LTTRUE;
}


void CHUDDamage::Render()
{
	// RETAIL: 0x10186f00 tests the item's enabled state too
	if (!IsEnabled()) return;
	if (!m_bDraw) return;

	SetRenderState();

	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstActiveFX();
	while( pDamageFX )
	{
		g_pDrawPrim->SetTexture( m_hIcon[pDamageFX->m_nID] );
		g_pDrawPrim->DrawPrim( &m_Poly[pDamageFX->m_nID], 1 );
	
		pDamageFX = g_pDamageFXMgr->GetNextActiveFX();
	}
}


void CHUDDamage::Update()
{
	if (!m_bDraw) return;

	// RETAIL: 0x10187020: Tron's stacks an icon for each active damage FX
	// (it does not skip a second FX of one type)
	float x = g_pInterfaceResMgr->GetXRatio() * m_BasePos.x;
	float y = (float)m_BasePos.y * g_pInterfaceResMgr->GetYRatio();
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: at its edge of the screen
	x += (float)g_pInterfaceResMgr->GetHUDXOffset(m_BasePos.x);
	y += (float)g_pInterfaceResMgr->GetHUDYOffset(m_BasePos.y);
#endif
	float sz = m_nIconHt * g_pInterfaceResMgr->GetYRatio();


	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstActiveFX();
	while( pDamageFX )
	{
		g_pDrawPrim->SetXYWH( &m_Poly[pDamageFX->m_nID], x, y, sz, sz );
		y += sz;
	
		pDamageFX = g_pDamageFXMgr->GetNextActiveFX();
	}
}

void CHUDDamage::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetDamageBasePos(nCurrentLayout);
	m_nIconHt		= g_pLayoutMgr->GetDamageIconSize(nCurrentLayout);
}

