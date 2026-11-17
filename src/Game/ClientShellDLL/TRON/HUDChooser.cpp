// ----------------------------------------------------------------------- //
//
// MODULE  : HUDChooser.cpp
//
// PURPOSE : HUDItem to display weapon and ammo choices
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDChooser.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "PlayerMgr.h"
#include "ClientWeaponMgr.h"
#include "TronLayoutMgr.h"
#include "TRONPlayerStats.h"
#include "HUDWeapon.h"

/*
current weapon =		(  1, 365, 47, 404 ) show this when the chooser is closed
weapon chooser box =	( 106, 374, 156, 413 )
weapon chooser box highlight =  ( 49, 374, 99, 413 )
weapon ammo fill =		( 101, 374, 104, 413 )
divider =				( 98, 419, 156, 420 )
*/

//******************************************************************************************
//**
//** HUD Weapon Chooser display
//**
//******************************************************************************************
CHUDWpnChooser::CHUDWpnChooser()
{
	m_UpdateFlags = kHUDChooser;
	m_bDraw = LTFALSE;
	m_fScale = 0.0f;
	m_nScreenWidth = 0;
	m_pStr = LTNULL;
	m_nSelWeapon = 0;
	m_fLastTime = 0.0f;
	m_fFadeTime = 0.0f;
	m_fFadeDuration = 0.25f;
	m_hEmptyTexHi = LTNULL;
	m_hCorruptTexHi = LTNULL;
	m_hChooserTexHi = LTNULL;
	m_hEmptyTex = LTNULL;
	m_hCorruptTex = LTNULL;
	m_hChooserTex = LTNULL;
}

// RETAIL: 0x10022be0
CHUDWpnChooser::~CHUDWpnChooser()
{
 	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}

}

// RETAIL: the UVs of a quad, half a texel in from each edge (inlined twice
// in Init). The name is ours.
static inline void SetHalfTexelUVs(LT_POLYFT4* pPoly, HTEXTURE hTex)
{
	uint32 nWidth, nHeight;
	float fU = 0.0f;
	float fV = 0.0f;
	if (g_pTexInterface->GetTextureDims(hTex, nWidth, nHeight) == LT_OK)
	{
		fU = 0.5f / (float)nWidth;
		fV = 0.5f / (float)nHeight;
	}
	pPoly->verts[0].u = fU;
	pPoly->verts[0].v = fV;
	pPoly->verts[1].u = 1.0f - fU;
	pPoly->verts[1].v = fV;
	pPoly->verts[2].u = 1.0f - fU;
	pPoly->verts[2].v = 1.0f - fV;
	pPoly->verts[3].u = fU;
	pPoly->verts[3].v = 1.0f - fV;
}

// RETAIL: 0x10022c50
LTBOOL CHUDWpnChooser::Init()
{
	// get first and last weapon indices
	uint8 nFirstWeaponCommandId = g_pWeaponMgr->GetFirstWeaponCommandId();
	uint8 nLastWeaponCommandId = g_pWeaponMgr->GetLastWeaponCommandId();
	memset(m_nWeaponCount,0,sizeof(m_nWeaponCount));
	for (uint8 nWpnCommandId = nFirstWeaponCommandId; nWpnCommandId <= nLastWeaponCommandId; nWpnCommandId++)
	{
		uint8 nWpn = g_pWeaponMgr->GetWeaponId( nWpnCommandId );

		if (g_pWeaponMgr->IsPlayerWeapon(nWpn))
		{
			// get the weapon data struct
			uint8 nClass = g_pWeaponMgr->GetWeaponClass( nWpn ) - 1;
			if (nClass < kMaxClasses && m_nWeaponCount[nClass] < kMaxItemsPerClass)
			{
				m_nWeaponID[nClass][m_nWeaponCount[nClass]] = nWpn;

				WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( nWpn );
				if (pWeapon)
				{
					std::string icon;
					icon = pWeapon->GetDisabledIcon();
					m_hDisabledIcon[nClass][m_nWeaponCount[nClass]] = g_pInterfaceResMgr->GetTexture( icon.c_str() );
					icon = pWeapon->GetNormalIcon();
					m_hNormalIcon[nClass][m_nWeaponCount[nClass]] = g_pInterfaceResMgr->GetTexture( icon.c_str() );
				}

				m_nWeaponCount[nClass]++;
			}

		}
	}

	UpdateLayout();

	for( uint8 i = 0; i < kMaxClasses; ++i )
	{
		for (uint8 n = 0; n < m_nWeaponCount[i]; n++)
		{
			g_pDrawPrim->SetRGBA( &m_BoxPoly[i][n], argbWhite );
			SetHalfTexelUVs( &m_BoxPoly[i][n], m_hChooserTex );

			g_pDrawPrim->SetRGBA( &m_IconPoly[i][n], argbWhite );
			// RETAIL: the first weapon's icon sizes every icon
			SetHalfTexelUVs( &m_IconPoly[i][n], m_hNormalIcon[0][0] );
		}

		for (n = m_nWeaponCount[i]; n < kMaxItemsPerClass; n++)
		{
			m_nWeaponID[i][n] = WMGR_INVALID_ID;
			m_hNormalIcon[i][n] = LTNULL;
		}
	}

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pStr = g_pFontManager->CreateFormattedPolyString(pFont,std::wstring(L" "),0.0f, 0.0f);
	m_pStr->SetColor(m_TextColor);

	return LTTRUE;
}

// RETAIL: 0x100232c0
void CHUDWpnChooser::UpdateFade()
{
	float fTime = g_pLTClient->GetTime();
	float fDelta = fTime - m_fLastTime;
	m_fLastTime = fTime;

	m_bDraw = !!g_pInterfaceMgr->IsChoosingWeapon();

	switch (m_eFadeState)
	{
		case 0:
			if (!m_bDraw)
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
			if (!m_bDraw)
			{
				m_eFadeState = 2;
				m_fFadeTime = 0.0f;
			}
			break;
		case 2:
			if (m_bDraw)
			{
				m_eFadeState = 0;
				m_fFadeTime = m_fFadeDuration - m_fFadeTime;
				m_pWeaponPrim = &g_pHUDWeapon->m_WeaponPrim;
				m_nCurWeapon = g_pTronPlayerStats->GetCurWeapon();
			}
			else
			{
				m_fFadeTime += fDelta;
				if (m_fFadeTime >= m_fFadeDuration)
					m_eFadeState = 3;
			}
			break;
		case 3:
			if (m_bDraw)
			{
				m_eFadeState = 0;
				m_fFadeTime = 0.0f;
				m_pWeaponPrim = &g_pHUDWeapon->m_WeaponPrim;
				m_nCurWeapon = g_pTronPlayerStats->GetCurWeapon();
			}
			break;
	}
}

// RETAIL: 0x10023460
void CHUDWpnChooser::Render()
{
	UpdateFade();
	if (!IsEnabled() || m_eFadeState == 3) return;

	SetRenderState();

	float fAlpha = 1.0f;
	if (m_eFadeState != 1)
	{
		fAlpha = m_fFadeTime / m_fFadeDuration;
		if (m_eFadeState == 2)
			fAlpha = 1.0f - fAlpha;
	}
	uint8 nAlpha = (uint8)(fAlpha * 255.0f);

	// RETAIL: class 0 has no row
	int nRow = 0;
	for (int i = 1; i < kMaxClasses; i++)
	{
		if (m_nWeaponCount[i] > 0)
		{
			bool abUnlocked[kMaxItemsPerClass];
			bool abCorrupt[kMaxItemsPerClass];
			bool abCompiled[kMaxItemsPerClass];
			int nShots;
			float fX = (float)(m_nScreenWidth - m_Margin.x + m_Spacing.x);
			float fY = (float)(m_Size.y + m_Margin.y) + (m_Spacing.y + m_BoxSize.y) * nRow;
			bool bAny = false;
			for (int n = 0; n < m_nWeaponCount[i]; n++)
			{
				g_pTronPlayerStats->GetWeaponStatus(m_nWeaponID[i][n], &abUnlocked[n], &abCorrupt[n], &abCompiled[n], &nShots);
				if (abUnlocked[n])
				{
					bAny = true;
					fX -= (float)(m_BoxSize.x + m_Spacing.x);
				}
			}
			int nX = (int)fX;
			float fOffset = 0.0f;
			bool bSelected = false;
			for (n = 0; n < m_nWeaponCount[i]; n++)
			{
				g_pDrawPrim->SetXYWH(&m_BoxPoly[i][n], fOffset + fX, fY, 64.0f, 64.0f);
				if ((m_nWeaponID[i][n] == m_nCurWeapon && m_eFadeState == 0) ||
					(m_nWeaponID[i][n] == m_nSelWeapon && m_eFadeState == 2))
				{
					// the icon flies between the HUD weapon and its box
					float x0 = m_pWeaponPrim->verts[0].x;
					float y0 = m_pWeaponPrim->verts[0].y;
					g_pDrawPrim->SetXYWH(&m_IconPoly[i][n],
						(m_IconOffset.x + fOffset + fX - x0) * fAlpha + x0,
						(m_IconOffset.y + fY - y0) * fAlpha + y0,
						64.0f, 64.0f);
					g_pDrawPrim->SetRGBA(&m_IconPoly[i][n], argbWhite);
				}
				else
				{
					g_pDrawPrim->SetRGBA(&m_IconPoly[i][n], SET_ARGB(nAlpha, 0xFF, 0xFF, 0xFF));
					g_pDrawPrim->SetXYWH(&m_IconPoly[i][n], m_IconOffset.x + fOffset + fX, m_IconOffset.y + fY, 64.0f, 64.0f);
				}
				if (m_nWeaponID[i][n] == m_nSelWeapon)
				{
					int nWidth, nHeight;
					m_pStr->GetDims(&nWidth, &nHeight);
					m_pStr->SetPosition(nX - 10 - nWidth, (int)fY - nHeight + m_BoxSize.y);
					bSelected = true;
				}
				if (abUnlocked[n])
					fOffset += (float)(m_BoxSize.x + m_Spacing.x);
			}
			if (bAny)
			{
				HTEXTURE hTex = bSelected ? m_hClassNumberHi[i] : m_hClassNumber[i];
				if (hTex)
				{
					g_pDrawPrim->SetTexture(hTex);
					g_pDrawPrim->SetXYWH(&m_ClassPoly[i], fX - 32.0f, fY, 32.0f, 64.0f);
					g_pDrawPrim->SetRGBA(&m_ClassPoly[i], SET_ARGB(nAlpha, 0xFF, 0xFF, 0xFF));
					g_pDrawPrim->DrawPrim(&m_ClassPoly[i], 1);
				}
			}
			for (n = 0; n < m_nWeaponCount[i]; n++)
			{
				bool bSel = (m_nWeaponID[i][n] == m_nSelWeapon);
				if (abUnlocked[n])
				{
					g_pDrawPrim->SetRGBA(&m_BoxPoly[i][n], SET_ARGB(nAlpha, 0xFF, 0xFF, 0xFF));
					HTEXTURE hTex = bSel ? m_hChooserTexHi : m_hChooserTex;
					if (abCorrupt[n])
						hTex = bSel ? m_hCorruptTexHi : m_hCorruptTex;
					if (!abCompiled[n])
						hTex = bSel ? m_hEmptyTexHi : m_hEmptyTex;
					g_pDrawPrim->SetTexture(hTex);
					g_pDrawPrim->DrawPrim(&m_BoxPoly[i][n], 1);
					g_pDrawPrim->SetTexture(bSel ? m_hDisabledIcon[i][n] : m_hNormalIcon[i][n]);
					g_pDrawPrim->DrawPrim(&m_IconPoly[i][n], 1);
				}
			}
			m_pStr->SetColor((m_TextColor & 0x00FFFFFF) | (nAlpha << 24));
			nRow++;
		}
	}
	m_pStr->Render();
}

// RETAIL: 0x10023a00
void CHUDWpnChooser::Update()
{
	m_bDraw = !!g_pInterfaceMgr->IsChoosingWeapon();
	if (!m_bDraw) return;

	if (g_pInterfaceResMgr->GetScreenWidth() != m_nScreenWidth)
	{
		m_nScreenWidth = g_pInterfaceResMgr->GetScreenWidth();
		if (m_nScreenWidth < 1024)
			m_fScale = (float)m_nScreenWidth / 1024.0f;
		else
			m_fScale = 1.0f;
		uint8 h = (uint8)(m_fTextHeight * m_fScale);
		CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(h);
		m_pStr->SetFont(pFont);
		m_pStr->SetCharScreenHeight(h);
	}

	uint8 nClass = g_pInterfaceMgr->GetWeaponChooser()->GetCurrentClass();
	m_nSelWeapon = g_pInterfaceMgr->GetWeaponChooser()->GetCurrentSelection();

	if (nClass > 0)
		m_nClass = nClass - 1;
	else
	{
		m_nClass = g_pWeaponMgr->GetWeaponClass( m_nSelWeapon ) - 1;
	}

	if (m_nClass >= kMaxClasses)
	{
		m_bDraw = LTFALSE;
		g_pInterfaceMgr->GetWeaponChooser()->Close(true);
		return;
	}

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( m_nSelWeapon );
	if (pWeapon)
		m_pStr->SetText(LoadTempWString(pWeapon->nNameId));
	else
		m_pStr->SetText(std::wstring(L" "));
}

// RETAIL: 0x10023cc0
void CHUDWpnChooser::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	// RETAIL: the textures are read from HUDLayout0 whatever the layout
	char szTex[256];
	g_pLayoutMgr->GetString("Hudlayout0", "ChooserTex", szTex, sizeof(szTex));
	m_hChooserTex = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetString("Hudlayout0", "ChooserTexHi", szTex, sizeof(szTex));
	m_hChooserTexHi = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetString("Hudlayout0", "ChooserCorruptTex", szTex, sizeof(szTex));
	m_hCorruptTex = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetString("Hudlayout0", "ChooserCorruptTexHi", szTex, sizeof(szTex));
	m_hCorruptTexHi = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetString("Hudlayout0", "ChooserEmptyTex", szTex, sizeof(szTex));
	m_hEmptyTex = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetString("Hudlayout0", "ChooserEmptyTexHi", szTex, sizeof(szTex));
	m_hEmptyTexHi = g_pInterfaceResMgr->GetTexture(szTex);

	m_TextColor = g_pLayoutMgr->GetChooserTextColor(nCurrentLayout,argbWhite);
	m_fTextHeight = (float)g_pLayoutMgr->GetChooserTextSize(nCurrentLayout,20);

	for (int i = 0; i < kMaxClasses; i++)
	{
		char szAttr[128];
		char szName[256];
		sprintf(szAttr, "%s%d", "ClassNumber", i);
		g_pLayoutMgr->GetString("Hudlayout0", szAttr, szName, sizeof(szName));
		m_hClassNumber[i] = g_pInterfaceResMgr->GetTexture(szName);
		sprintf(szAttr, "%s%d", "ClassNumberHi", i);
		g_pLayoutMgr->GetString("Hudlayout0", szAttr, szName, sizeof(szName));
		m_hClassNumberHi[i] = g_pInterfaceResMgr->GetTexture(szName);

		g_pDrawPrim->SetRGBA(&m_ClassPoly[i], argbWhite);
		g_pDrawPrim->SetUVWH(&m_ClassPoly[i], 0.0f, 0.0f, 1.0f, 1.0f);
	}

	m_BoxSize = ((CTronLayoutMgr*)g_pLayoutMgr)->GetChooserBoxSize(nCurrentLayout);
	m_Spacing = ((CTronLayoutMgr*)g_pLayoutMgr)->GetChooserSpacing(nCurrentLayout);
	m_IconOffset = ((CTronLayoutMgr*)g_pLayoutMgr)->GetChooserIconOffset(nCurrentLayout);
	m_Margin = ((CTronLayoutMgr*)g_pLayoutMgr)->GetChooserMargin(nCurrentLayout);
	m_Size.y = ((CTronLayoutMgr*)g_pLayoutMgr)->GetChooserTitleHeight(nCurrentLayout);
	m_Size.x = 3 * m_BoxSize.x + 2 * m_Spacing.x;
	m_fIntroTime = ((CTronLayoutMgr*)g_pLayoutMgr)->GetChooserIntroTime(nCurrentLayout);
	m_fOutroTime = ((CTronLayoutMgr*)g_pLayoutMgr)->GetChooserOutroTime(nCurrentLayout);
}



//******************************************************************************************
//**
//** HUD Weapon Chooser display
//**
//******************************************************************************************

CHUDAmmoChooser::CHUDAmmoChooser()
{
	m_UpdateFlags = kHUDChooser;
	m_bDraw = LTFALSE;
	m_fScale = 0.0f;
	m_fTexScale = 0.0f;
	m_nScreenWidth = 0;
	m_pStr = LTNULL;
	m_pWpnStr = LTNULL;
}

CHUDAmmoChooser::~CHUDAmmoChooser()
{
 	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}
 	if (m_pWpnStr)
	{
		g_pFontManager->DestroyPolyString(m_pWpnStr);
        m_pWpnStr=LTNULL;
	}

}


LTBOOL CHUDAmmoChooser::Init()
{

	UpdateLayout();


	g_pDrawPrim->SetRGBA( &m_WeaponPoly, argbWhite );
	g_pDrawPrim->SetUVWH( &m_WeaponPoly, 0.0f, 0.0f, m_fTexScale, m_fTexScale );

	m_hWeaponIcon = NULL;

	for (int i = 0; i < kMaxAmmoTypes; ++i )
	{
		g_pDrawPrim->SetRGBA( &m_AmmoPoly[i], argbWhite );
		g_pDrawPrim->SetUVWH( &m_AmmoPoly[i], 0.0f, 0.0f, m_fTexScale, m_fTexScale );

		m_hAmmoIcon[i] = NULL;

	}

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pStr = g_pFontManager->CreateFormattedPolyString(pFont,std::wstring(),0.0f, 0.0f);
	m_pStr->SetColor(m_TextColor);

	m_pWpnStr = g_pFontManager->CreateFormattedPolyString(pFont,std::wstring(),0.0f, 0.0f);
	m_pWpnStr->SetColor(m_TextColor);


	return LTTRUE;
}


void CHUDAmmoChooser::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	g_pDrawPrim->SetTexture( m_hWeaponIcon );
	g_pDrawPrim->DrawPrim( &m_WeaponPoly, 1 );

	for (uint8 n = 0; m_hAmmoIcon[n] && n < kMaxAmmoTypes; n++)
	{
		g_pDrawPrim->SetTexture( m_hAmmoIcon[n] );
		g_pDrawPrim->DrawPrim( &m_AmmoPoly[n], 1 );
	}

	m_pStr->Render();
	m_pWpnStr->Render();
}


void CHUDAmmoChooser::Update()
{
	m_bDraw = !!g_pInterfaceMgr->IsChoosingAmmo();
	if (!m_bDraw) return;
	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	if ( !pClientWeapon )
	{
		m_bDraw = LTFALSE;
		g_pInterfaceMgr->GetAmmoChooser()->Close();
		return;
	}
	
	WEAPON const *pWeapon = pClientWeapon->GetWeapon();
	if (!pWeapon || pWeapon->nNumAmmoIds <= 1)
	{
		m_bDraw = LTFALSE;
		g_pInterfaceMgr->GetAmmoChooser()->Close();
		return;
	}

	if (g_pInterfaceResMgr->GetScreenWidth() != m_nScreenWidth)
	{
		m_nScreenWidth = g_pInterfaceResMgr->GetScreenWidth();
		if (m_nScreenWidth < 1024)
			m_fScale = (float)m_nScreenWidth / 1024.0f;
		else
			m_fScale = 1.0f;

		uint8 h = (uint8)(m_fTextHeight * m_fScale);
		m_pStr->SetCharScreenHeight(h);
		m_pWpnStr->SetCharScreenHeight(h);

		uint16 w = ((uint16)(m_nScreenWidth) / 2) - 20;
		m_pStr->SetWrapWidth(w);
		m_pWpnStr->SetWrapWidth(w);

	}


	float fHeight = m_fIconHt * m_fScale;

	std::string icon = pWeapon->GetNormalIcon();
	m_hWeaponIcon = g_pInterfaceResMgr->GetTexture( icon.c_str() );

	uint32 tw,th;
	g_pTexInterface->GetTextureDims(m_hWeaponIcon,tw,th);

	float fWidth = (float)tw * ( fHeight / (float)th );
	float x = ((float)m_nScreenWidth / 2.0f);
	g_pDrawPrim->SetXYWH( &m_WeaponPoly, x-fWidth, 0.0f, fWidth, fHeight );

	uint8 nSlot = 0;
	uint8 nCurrAmmo = g_pInterfaceMgr->GetAmmoChooser()->GetCurrentSelection();
	float y = fHeight;
	

	m_pWpnStr->SetText(LoadTempString(pWeapon->nNameId));
	m_pWpnStr->SetPosition(x+2.0f,2.0f);

	for (uint8 n = 0; n < pWeapon->nNumAmmoIds && n < kMaxAmmoTypes; n++)
	{
		uint8 nAmmoID = pWeapon->aAmmoIds[n];
		if (g_pPlayerStats->GetAmmoCount(nAmmoID) <= 0) continue;
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoID);
		if (!pAmmo) continue;

	
		icon = "";
		if (nAmmoID == nCurrAmmo)
		{
			icon = pAmmo->GetNormalIcon();
		}
		else
		{
			icon = pAmmo->GetUnselectedIcon();
		}

		m_hAmmoIcon[nSlot] = g_pInterfaceResMgr->GetTexture( icon.c_str() );
		if (!m_hAmmoIcon[nSlot]) continue;

		uint32 tw,th;
		g_pTexInterface->GetTextureDims(m_hAmmoIcon[nSlot],tw,th);
		float fWidth = (float)tw * ( fHeight / (float)th );

		g_pDrawPrim->SetXYWH( &m_AmmoPoly[nSlot], x-fWidth, y, fWidth, fHeight );

		
		char szTmp[256] = "";

		if (pWeapon->bInfiniteAmmo)
			LoadString(pAmmo->nNameId,szTmp,sizeof(szTmp));
		else
			sprintf(szTmp,"(%d/%d) %s",g_pPlayerStats->GetAmmoCount(nAmmoID), pAmmo->GetMaxAmount(LTNULL), LoadTempString(pAmmo->nNameId) );
		if (nAmmoID == nCurrAmmo)
		{
			m_pStr->SetText( szTmp);
			m_pStr->SetPosition(x+2.0f,y+2.0f);
		}

		nSlot++;
		y += fHeight;

	}
	m_hAmmoIcon[nSlot] = NULL;
		
}

void CHUDAmmoChooser::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);


	m_fIconHt = (float)g_pLayoutMgr->GetChooserIconHeight(nCurrentLayout,48);
	m_TextColor = g_pLayoutMgr->GetChooserTextColor(nCurrentLayout,argbWhite);
	m_fTextHeight = (float)g_pLayoutMgr->GetChooserTextSize(nCurrentLayout,20);
	m_fTexScale = g_pLayoutMgr->GetChooserTextureScale(nCurrentLayout,0.75f);


}

