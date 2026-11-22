// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeapon.cpp
//
// PURPOSE : HUDItem to display Jet's current Weapon
//
// RETAIL: rewritten from retail CShell.dll 0x1003e9a0-0x1003f3e0 (Tron's
// indicator fades out while the weapon chooser is open and shows the ammo)
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDWeapon.h"
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"


//******************************************************************************************
//**
//** HUD Weapon display
//**
//******************************************************************************************

CHUDWeapon* g_pHUDWeapon = LTNULL;

// RETAIL: 0x1003e9a0
CHUDWeapon::CHUDWeapon()
{
	m_hHUDTex = LTNULL;
	m_hCorruptTex = LTNULL;
	m_hWeaponTex = LTNULL;

	m_fScale = 0.0f;
	m_nWeaponID = 0;
	m_bVisible = LTFALSE;
	m_pAmmoStr = LTNULL;

	m_UpdateFlags = kHUDWeapons;
	m_eFadeState = 3;
	m_fFadeDuration = 0.25f;
}

// RETAIL: 0x1003ea30
LTBOOL CHUDWeapon::Init()
{
	UpdateLayout();

	g_pDrawPrim->SetRGBA(&m_BasePrim, argbWhite);
	g_pDrawPrim->SetRGBA(&m_WeaponPrim, argbWhite);
	g_pDrawPrim->SetUVWH(&m_WeaponPrim, 0.0f, 0.0f, 1.0f, 1.0f);

	m_eFadeState = 3;
	m_fLastTime = 0.0f;
	g_pHUDWeapon = this;
	m_nAmmo = -10;

	return LTTRUE;
}

// RETAIL: 0x1003eab0
void CHUDWeapon::Term()
{
	g_pHUDWeapon = LTNULL;
	if (m_pAmmoStr)
	{
		g_pFontManager->DestroyPolyString(m_pAmmoStr);
		m_pAmmoStr = LTNULL;
	}
}

// RETAIL: 0x1003eae0
void CHUDWeapon::UpdateFade()
{
	float fTime = g_pLTClient->GetTime();
	float fDelta = fTime - m_fLastTime;
	m_fLastTime = fTime;

	m_bVisible = !g_pInterfaceMgr->GetWeaponChooser()->IsOpen();

	switch (m_eFadeState)
	{
		case 0:
			if (!m_bVisible)
			{
				m_eFadeState = 2;
				m_fFadeTime = m_fFadeDuration - m_fFadeTime;
			}
			else
			{
				m_fFadeTime += fDelta;
				if (m_fFadeTime >= m_fFadeDuration)
					m_eFadeState = 1;
			}
			break;
		case 1:
			if (!m_bVisible)
			{
				m_eFadeState = 2;
				m_fFadeTime = 0.0f;
			}
			break;
		case 2:
			if (m_bVisible)
			{
				m_eFadeState = 0;
				m_fFadeTime = m_fFadeDuration - m_fFadeTime;
			}
			else
			{
				m_fFadeTime += fDelta;
				if (m_fFadeTime >= m_fFadeDuration)
					m_eFadeState = 3;
			}
			break;
		case 3:
			if (m_bVisible)
			{
				m_eFadeState = 0;
				m_fFadeTime = 0.0f;
			}
			break;
	}
}

// RETAIL: 0x1003ec00
void CHUDWeapon::Render()
{
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	UpdateFade();
	if (m_eFadeState == 3) return;

	float fAlpha = 1.0f;
	if (m_eFadeState == 2)
		fAlpha = 1.0f - m_fFadeTime / m_fFadeDuration;
	else if (m_eFadeState == 0)
		fAlpha = m_fFadeTime / m_fFadeDuration;

	uint8 nAlpha = (uint8)(fAlpha * 255.0f);
	g_pDrawPrim->SetRGBA(&m_BasePrim, SET_ARGB(nAlpha, 0xFF, 0xFF, 0xFF));

	if (m_eFadeState != 1) return;

	int nOldAmmo = m_nAmmo;
	uint8 nWeaponID = g_pTronPlayerStats->GetCurWeapon();
	bool bUnlocked, bCorrupt, bCompiled;
	g_pTronPlayerStats->GetWeaponStatus(nWeaponID, &bUnlocked, &bCorrupt, &bCompiled, &m_nAmmo);

	if (m_hWeaponTex)
	{
		SetRenderState();
		if (bCorrupt)
			g_pDrawPrim->SetTexture(m_hCorruptTex);
		else
			g_pDrawPrim->SetTexture(m_hHUDTex);
		g_pDrawPrim->DrawPrim(&m_BasePrim);
		g_pDrawPrim->SetTexture(m_hWeaponTex);
		g_pDrawPrim->DrawPrim(&m_WeaponPrim);
	}

	if (m_nAmmo != nOldAmmo)
	{
		std::wstring sAmmo;
		char szAmmo[10];
		if (m_nAmmo > 0)
			sprintf(szAmmo, "%d", m_nAmmo);
		else
			sprintf(szAmmo, " ");
		LTGUIWiden(sAmmo, szAmmo);

		if (!m_pAmmoStr)
		{
			CUIFont* pFont = g_pInterfaceResMgr->GetFont(0);
			m_pAmmoStr = g_pFontManager->CreatePolyString(pFont, sAmmo);
			m_pAmmoStr->SetCharScreenSize(20);
			m_pAmmoStr->SetColor(argbWhite);
			m_pAmmoStr->SetDropShadow(true);
		}
		else
			m_pAmmoStr->SetText(sAmmo);

		m_pAmmoStr->SetPosition((int)m_BasePrim.verts[3].x + m_AmmoOffset.x, (int)m_BasePrim.verts[3].y + m_AmmoOffset.y);
	}

	if (m_hWeaponTex && m_pAmmoStr)
		m_pAmmoStr->Render();
}

// RETAIL: 0x1003efb0: SetWeapon's code is repeated here (not a call)
void CHUDWeapon::Update()
{
	m_bVisible = !g_pInterfaceMgr->GetWeaponChooser()->IsOpen();
	if (!m_bVisible) return;

	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	uint8 nWeaponID = g_pTronPlayerStats->GetCurWeapon();
	if (m_nWeaponID == nWeaponID) return;

	m_nWeaponID = nWeaponID;
	m_hWeaponTex = LTNULL;
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if (pWeapon)
	{
		std::string sIcon = pWeapon->GetUnselectedIcon();
		m_hWeaponTex = g_pInterfaceResMgr->GetTexture(sIcon.c_str());
	}
}

// RETAIL: 0x1003f100
void CHUDWeapon::SetWeapon(uint8 nWeaponID)
{
	if (m_nWeaponID == nWeaponID) return;

	m_nWeaponID = nWeaponID;
	m_hWeaponTex = LTNULL;
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if (pWeapon)
	{
		std::string sIcon = pWeapon->GetUnselectedIcon();
		m_hWeaponTex = g_pInterfaceResMgr->GetTexture(sIcon.c_str());
	}
}

// RETAIL: 0x1003f1f0. NEAR: retail reads the texture height later (the
// same code; VC6 schedules the load of nHeight earlier here)
void CHUDWeapon::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	char szTex[256];
	g_pLayoutMgr->GetString("Hudlayout0", "ChooserTexHi", szTex, sizeof(szTex));
	m_hHUDTex = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetString("Hudlayout0", "ChooserCorruptTexHi", szTex, sizeof(szTex));
	m_hCorruptTex = g_pInterfaceResMgr->GetTexture(szTex);

	uint32 nWidth, nHeight;
	g_pTexInterface->GetTextureDims(m_hHUDTex, nWidth, nHeight);
	g_pDrawPrim->SetUVWH(&m_BasePrim, 0.0f, 0.0f, 0.99f, 0.99f);

	m_fFadeDuration = ((CTronLayoutMgr*)g_pLayoutMgr)->GetWeaponIndicatorAnimTime(nCurrentLayout);

	// RETAIL: 0x1003f2d0, 0x1003f35f: CTronLayoutMgr's weapon indicator
	// offsets (0x100a1cb0, 0x100a1d30), not CLayoutMgr's NOLF2 getters
	LTIntPt offset = ((CTronLayoutMgr*)g_pLayoutMgr)->GetWeaponIndicatorOffset(nCurrentLayout);
	float x = 640.0f * g_pInterfaceResMgr->GetXRatio() - (float)nWidth - offset.x;
	float y = (float)offset.y;
	g_pDrawPrim->SetXYWH(&m_BasePrim, x, y, (float)nWidth, (float)nHeight);

	offset = ((CTronLayoutMgr*)g_pLayoutMgr)->GetWeaponIndicatorIconOffset(nCurrentLayout);
	g_pDrawPrim->SetXYWH(&m_WeaponPrim, x + (float)offset.x, y + (float)offset.y, 64.0f, 64.0f);

	m_AmmoOffset = g_pLayoutMgr->GetPoint("HUDLayout0", "AmmoCountOffset");
}
