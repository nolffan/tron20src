// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEnergyTransfer.cpp
//
// PURPOSE : HUDItem to display an energy transfer in progress
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"
#include "TronPlayerMgr.h"

// RETAIL: Tron 2.0's energy transfer meter, written from retail CShell.dll
// (0x10029350..0x10029910). Build 69's arc of quads is gone: the meter is
// a texture and its fill up to three triangles fanned from its center, the
// angle going from -90 (empty) to 24 degrees (full).

namespace
{
	const float kfEmptyAngle = -90.0f;	// RETAIL: 0x101e6dc0
	const float kfFullAngle = 24.0f;	// RETAIL: 0x101e6dc4
	const float kfCenterU = 0.015625f;	// RETAIL: 0x101e6dc8 (the meter's center's u)
}

//******************************************************************************************
//**
//** HUD Energy transfer display
//**
//******************************************************************************************

CHUDEnergyTransfer::CHUDEnergyTransfer()
{
	m_UpdateFlags = kHUDEnergyTrans;
	m_fScale = 0.0f;
	m_bShow = false;
}


LTBOOL CHUDEnergyTransfer::Init()
{
	UpdateLayout();

	m_hBack = g_pInterfaceResMgr->GetTexture("interface\\hud\\transfer_meter_bg.dtx");
	m_hFill = g_pInterfaceResMgr->GetTexture("interface\\hud\\transfer_meter_fg.dtx");

	return LTTRUE;
}

void CHUDEnergyTransfer::Term()
{
	// nothing to do here
}

void CHUDEnergyTransfer::Render()
{
	if (!IsEnabled()) return;

	m_bShow = g_pTronPlayerMgr->IsShowingEnergyTransfer();
	if (!m_bShow) return;

	SetRenderState();
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	g_pDrawPrim->SetTexture(m_hBack);
	g_pDrawPrim->DrawPrim(&m_Back, 1);

	if (m_nFill)
	{
		g_pDrawPrim->SetTexture(m_hFill);
		g_pDrawPrim->DrawPrim(m_Fill, m_nFill);
	}
}

void CHUDEnergyTransfer::Update()
{
	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	float fPercentDone = g_pTronPlayerMgr->GetPercentEnergyTransferred();
	if (fPercentDone >= 1.0f)
		fPercentDone = 1.0f;
	else if (fPercentDone < 0.0f)
		fPercentDone = 0.0f;

	float fAngle = (kfFullAngle - kfEmptyAngle) * fPercentDone + kfEmptyAngle;

	if (fAngle < -45.0f)
	{
		m_nFill = 1;
		BuildFill(&m_Fill[0], -90.0f, fAngle);
	}
	else if (fAngle < 0.0f)
	{
		m_nFill = 2;
		BuildFill(&m_Fill[0], -90.0f, -45.0f);
		BuildFill(&m_Fill[1], -45.0f, fAngle);
	}
	else
	{
		m_nFill = 3;
		BuildFill(&m_Fill[0], -90.0f, -45.0f);
		BuildFill(&m_Fill[1], -45.0f, 0.0f);
		// RETAIL: from -45 again, not 0 (the second triangle is drawn twice
		// over: docs/RETAIL_BUGS.md)
		BuildFill(&m_Fill[2], -45.0f, fAngle);
	}
}

// RETAIL: 0x10029610: the triangle from the center to the meter's edge at
// the two angles (the meter is 64x128, its center at the left middle).
// NEAR: retail multiplies by -0.5 and adds 0.5 for the v's; VC6 makes
// 0.5 - y * 0.5 of every form of it tried here (fsubr for fadd)
void CHUDEnergyTransfer::BuildFill(LT_POLYFT3* pPoly, float fStartAngle, float fEndAngle)
{
	float x, y;

	GetSquarePoint(fStartAngle, &x, &y);
	pPoly->verts[1].u = kfCenterU + x;
	pPoly->verts[1].v = y * -0.5f + 0.5f;
	pPoly->verts[1].x = x * 64.0f + m_fCenterX;
	pPoly->verts[1].y = m_fCenterY - y * 64.0f;

	GetSquarePoint(fEndAngle, &x, &y);
	pPoly->verts[2].u = kfCenterU + x;
	pPoly->verts[2].v = y * -0.5f + 0.5f;
	pPoly->verts[2].x = x * 64.0f + m_fCenterX;
	pPoly->verts[2].y = m_fCenterY - y * 64.0f;
}

// RETAIL: 0x100296d0: where the ray at the angle (degrees) leaves the unit
// square around the origin
void CHUDEnergyTransfer::GetSquarePoint(float fAngle, float* pX, float* pY)
{
	while (fAngle < 0.0f)
		fAngle += 360.0f;
	while (fAngle > 360.0f)
		fAngle -= 360.0f;

	bool bNegY = false;
	if (fAngle > 180.0f)
	{
		fAngle = 360.0f - fAngle;
		bNegY = true;
	}

	bool bNegX = false;
	if (fAngle > 90.0f)
	{
		fAngle = 180.0f - fAngle;
		bNegX = true;
	}

	fAngle *= MATH_PI / 180.0f;
	float fCos = (float)cos(fAngle);
	float fSin = (float)sin(fAngle);

	if (fCos > fSin)
	{
		*pX = 1.0f;
		*pY = 1.0f / fCos * fSin;
	}
	else
	{
		*pY = 1.0f;
		*pX = 1.0f / fSin * fCos;
	}

	if (bNegX)
		*pX *= -1.0f;
	if (bNegY)
		*pY *= -1.0f;
}

void CHUDEnergyTransfer::UpdateLayout()
{
	m_nFill = 0;

	m_fCenterX = (float)(g_pInterfaceResMgr->GetScreenWidth() / 2);
	m_fCenterY = (float)(g_pInterfaceResMgr->GetScreenHeight() / 2);

	g_pDrawPrim->SetRGBA(&m_Back, argbWhite);
	g_pDrawPrim->SetXYWH(&m_Back, m_fCenterX, m_fCenterY - 64.0f, 64.0f, 128.0f);
	SetupQuadUVs(m_Back, m_hBack, 0.0f, 0.0f, 1.0f, 1.0f);

	for (int i = 0; i < 3; i++)
	{
		m_Fill[i].rgba.r = 0xFF;
		m_Fill[i].rgba.g = 0xFF;
		m_Fill[i].rgba.b = 0xFF;
		m_Fill[i].rgba.a = 0xFF;
		m_Fill[i].verts[0].x = m_fCenterX;
		m_Fill[i].verts[0].y = m_fCenterY;
		m_Fill[i].verts[0].u = kfCenterU;
		m_Fill[i].verts[0].v = 0.5f;
	}
}
