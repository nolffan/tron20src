// ----------------------------------------------------------------------- //
//
// MODULE  : ToolTip.cpp
//
// PURPOSE : A tool tip: text in a frame near the cursor, with an icon
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (retail CShell.dll 0x1014e230..0x1014eaf9), written from
// the retail code. The names are ours.

#include "stdafx.h"
#include "ToolTip.h"
#include "InterfaceMgr.h"
#include "ScreenSpriteMgr.h"
#include "../TRON/LightLine.h"

// RETAIL: 0x1014e230
CToolTip::CToolTip()
{
	m_bVisible = false;
	m_nTextColor = 0xFFFFFFFF;
	m_nBackColor = 0x800000C0;
	m_rcBorder.Init(8, 8, 8, 8);
	m_nWidth = 250;
	m_nOffset = 16;
	m_nFontSize = 20;
	m_fFadeTime = 0.0f;
	m_fDelay = 0.0f;

	for (int i = 0; i < kNumLines; i++)
	{
		m_pLines[i] = LTNULL;
	}

	m_hIcon = LTNULL;
}

// RETAIL: 0x1014e2c0
CToolTip::~CToolTip()
{
	Hide();
}

// RETAIL: 0x1014e310
void CToolTip::SetBackColor(uint32 nColor)
{
	m_nBackColor = nColor;
	m_Frame.SetColor(nColor);
	m_hFrame = LTNULL;
}

// RETAIL: 0x1014e340
void CToolTip::SetTextColor(uint32 nColor)
{
	m_nTextColor = nColor;
	if (m_bVisible && m_pText)
	{
		m_pText->SetColor(nColor);
	}
}

// RETAIL: 0x1014e360
void CToolTip::Hide()
{
	if (!m_bVisible) return;

	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}

	m_bVisible = false;
	g_pScreenSpriteMgr->m_pTronUnknownToolTip = LTNULL;

	for (int i = 0; i < kNumLines; i++)
	{
		m_pLines[i]->Term();
		debug_delete(m_pLines[i]);
		m_pLines[i] = LTNULL;
	}

	m_hIcon = LTNULL;
}

// RETAIL: 0x1014e3e0. After the delay the tip fades in
void CToolTip::Draw()
{
	if (!m_bVisible) return;

	uint32 nIconColor = 0xFFFFFFFF;
	float fElapsed = g_pLTClient->GetTime() - m_fStartTime;
	if (fElapsed < m_fDelay) return;

	if (m_fDelay + m_fFadeTime > fElapsed &&
		(fElapsed = (fElapsed - m_fDelay) / m_fFadeTime) != 1.0f)
	{
		m_pText->SetColor(((uint32)(uint8)(fElapsed * 255.0f) << 24) | (m_nTextColor & 0x00FFFFFF));

		uint8 nAlpha = (uint8)((float)(int)(m_nBackColor >> 24) * fElapsed);
		if (!m_hFrame)
		{
			m_Frame.SetColor(((uint32)nAlpha << 24) | (m_nBackColor & 0x00FFFFFF));
		}
		nIconColor = ((uint32)nAlpha << 24) | 0x00FFFFFF;
	}
	else
	{
		m_pText->SetColor(m_nTextColor);
	}

	m_Frame.Render();
	m_pText->Render();

	for (int i = 0; i < kNumLines; i++)
	{
		if (m_pLines[i])
		{
			m_pLines[i]->Render();
		}
	}

	if (m_hIcon)
	{
		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		g_pDrawPrim->SetTexture(m_hIcon);
		g_pDrawPrim->SetRGBA(&m_IconPoly, nIconColor);
		g_pDrawPrim->DrawPrim(&m_IconPoly);
	}
}

// RETAIL: 0x1014e5c0. Places the tip above the point (below it, or beside
// it, when it does not fit), with its icon at the top right
void CToolTip::Show(int x, int y, std::wstring sText, HTEXTURE hIcon)
{
	if (m_bVisible)
	{
		Hide();
	}

	if (!sText.size()) return;

	m_bVisible = true;
	m_fStartTime = g_pLTClient->GetTime();

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(m_nFontSize);
	m_pText = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pText->SetColor(m_nTextColor);
	m_pText->SetCharScreenHeight(m_nFontSize);
	m_pText->SetWrapWidth(m_nWidth - m_rcBorder.right - m_rcBorder.left);

	int32 nTextWidth, nTextHeight;
	m_pText->GetDims(&nTextWidth, &nTextHeight);

	int nWidth = m_nWidth + m_rcBorder.right + m_rcBorder.left;
	int nHeight = m_rcBorder.bottom + m_rcBorder.top + nTextHeight;

	if (m_hFrame)
	{
		m_Frame.Create(m_hFrame, nWidth, nHeight);
	}
	else
	{
		m_Frame.Create(m_nBackColor, nWidth, nHeight);
	}

	int nScreenWidth = g_pInterfaceResMgr->GetScreenWidth();
	int nScreenHeight = g_pInterfaceResMgr->GetScreenHeight();

	int nX = x - (int)((float)nWidth * 0.5f);
	if (nX < 10)
	{
		nX = 10;
	}
	else if (nX + nWidth > nScreenWidth - 10)
	{
		nX = nScreenWidth - nWidth - 10;
	}

	if (nHeight + 20 > nScreenHeight)
	{
		g_pLTClient->CPrint("Tool tip is too tall to fit on screen");
	}

	int nSideOffset = (nScreenWidth / nScreenHeight) * m_nOffset;

	int nY = y - m_nOffset - nHeight;
	if (nY < 10)
	{
		nY = y + m_nOffset;
		if (nHeight + nY > nScreenHeight - 10)
		{
			int nHalf = nHeight / 2;
			nY = y - nHalf;
			nX = x - nSideOffset - nWidth;
			if (nX < 10)
			{
				nX = nSideOffset + x;
			}

			if (nHalf + nY < nScreenHeight - 10 || nY - nHalf < 10)
			{
				nY = nScreenHeight / 2 - nHalf;
			}
		}
	}

	m_Frame.SetBasePos(LTIntPt(nX, nY));
	m_pText->SetPosition(m_rcBorder.left + nX, m_rcBorder.top + nY);

	for (int i = 0; i < kNumLines; i++)
	{
		if (m_pLines[i])
		{
			debug_delete(m_pLines[i]);
		}
		m_pLines[i] = debug_new(CLightLine);
	}

	float fLeft = (float)nX;
	float fTop = (float)nY;
	float fRight = (float)(nX + nWidth);
	float fBottom = (float)(nHeight + nY);
	m_pLines[0]->Create(fLeft, fTop, fRight, fTop);
	m_pLines[1]->Create(fRight, fTop, fRight, fBottom);
	m_pLines[2]->Create(fRight, fBottom, fLeft, fBottom);
	m_pLines[3]->Create(fLeft, fTop, fLeft, fBottom);

	if (hIcon)
	{
		m_hIcon = hIcon;

		uint32 nIconWidth, nIconHeight;
		g_pTexInterface->GetTextureDims(hIcon, nIconWidth, nIconHeight);
		g_pDrawPrim->SetXYWH(&m_IconPoly,
			(float)(nX - m_rcBorder.right - nIconWidth + nWidth),
			(float)(m_rcBorder.top + nY),
			(float)nIconWidth, (float)nIconHeight);
		g_pDrawPrim->SetRGBA(&m_IconPoly, 0xFFFFFFFF);

		// Half a texel in from each edge
		float fU = 0.0f;
		float fV = 0.0f;
		uint32 nTexWidth, nTexHeight;
		if (g_pTexInterface->GetTextureDims(m_hIcon, nTexWidth, nTexHeight) == LT_OK)
		{
			fU = 0.5f / (float)nTexWidth;
			fV = 0.5f / (float)nTexHeight;
		}

		m_IconPoly.verts[0].u = fU;
		m_IconPoly.verts[0].v = fV;
		m_IconPoly.verts[1].u = 1.0f - fU;
		m_IconPoly.verts[1].v = fV;
		m_IconPoly.verts[2].u = 1.0f - fU;
		m_IconPoly.verts[2].v = 1.0f - fV;
		m_IconPoly.verts[3].u = fU;
		m_IconPoly.verts[3].v = 1.0f - fV;
	}

	g_pScreenSpriteMgr->m_pTronUnknownToolTip = this;
}
