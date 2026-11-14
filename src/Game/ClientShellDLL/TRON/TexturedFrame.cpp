// ----------------------------------------------------------------------- //
//
// MODULE  : TexturedFrame.cpp
//
// PURPOSE : A nine-piece frame drawn from one texture
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1013e130..
// 0x1013f660; TexturedFrame.h). The names are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TexturedFrame.h"
#include "InterfaceMgr.h"
#include "ltquaduvutils.h"

CTexturedFrame* g_pTexturedFrame = LTNULL;

// RETAIL: 0x1013e130
CTexturedFrame::CTexturedFrame()
{
	m_bDirty = true;
	m_bShowPieces = false;
}

// RETAIL: an empty function (folded: 0x100abb80)
CTexturedFrame::~CTexturedFrame()
{
}

// RETAIL: 0x1013e1b0
void CTexturedFrame::Init(const char* szName)
{
	Init(g_pLayoutMgr->GetTexturedFrame(szName));
	m_bDirty = true;
	g_pTexturedFrame = this;
}

// RETAIL: 0x1013e1e0
void CTexturedFrame::Init(INT_TEXTUREDFRAME* pFrame)
{
	m_hTexture = g_pInterfaceResMgr->GetTexture(pFrame->szTexture);
	m_bHollow = pFrame->bHollow;
	m_bInscribe = pFrame->bInscribe;

	uint32 nTexWidth, nTexHeight;
	g_pTexInterface->GetTextureDims(m_hTexture, nTexWidth, nTexHeight);
	nTexWidth--;
	nTexHeight--;

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(16);
	char szTmp[128];

	for (int i = 0; i < kNumPieces; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Poly[i], argbWhite);

		m_Piece[i].x = pFrame->rcUV[i].left;
		m_Piece[i].w = pFrame->rcUV[i].right - pFrame->rcUV[i].left;
		m_Piece[i].y = pFrame->rcUV[i].top;
		m_Piece[i].h = pFrame->rcUV[i].bottom - pFrame->rcUV[i].top;

		SetupQuadUVs(m_Poly[i], m_hTexture,
			(float)m_Piece[i].x / (float)nTexWidth, (float)m_Piece[i].y / (float)nTexHeight,
			(float)m_Piece[i].w / (float)nTexWidth, (float)m_Piece[i].h / (float)nTexHeight);

		// the pieces' numbers, in two columns down the screen
		m_pPieceStr[i] = g_pFontManager->CreatePolyString(pFont, L" ", 10.0f, (float)i * 20.0f + 350.0f);
		m_pPieceStr[i]->SetColor(argbWhite);
		m_pPieceStr[i]->SetCharScreenHeight(16);

		sprintf(szTmp, "<%d %d %d %d>", m_Piece[i].x, m_Piece[i].y, m_Piece[i].x + m_Piece[i].w, m_Piece[i].y + m_Piece[i].h);

		std::wstring sText;
		sText.erase(sText.begin(), sText.end());
		uint32 nLen = strlen(szTmp);
		for (uint32 n = 0; n < nLen; n++)
			sText.push_back((wchar_t)(uint8)szTmp[n]);

		m_pUVStr[i] = g_pFontManager->CreatePolyString(pFont, sText, 320.0f, (float)i * 20.0f + 350.0f);
		m_pUVStr[i]->SetColor(argbWhite);
		m_pUVStr[i]->SetCharScreenHeight(16);
	}

	if (pFrame->bMargins)
	{
		m_rcMargins.left = pFrame->rcMargins.left;
		m_rcMargins.right = pFrame->rcMargins.right;
		m_rcMargins.top = pFrame->rcMargins.top;
		m_rcMargins.bottom = pFrame->rcMargins.bottom;
	}
	else
	{
		// the border pieces' widths
		m_rcMargins.left = m_Piece[kLeft].w;
		m_rcMargins.right = m_Piece[kRight].w;
		m_rcMargins.top = m_Piece[kTop].h;
		m_rcMargins.bottom = m_Piece[kBottom].h;
	}
}

// RETAIL: 0x1013e740 (the "Frame" console program's)
void CTexturedFrame::AdjustPiece(int nPiece, int nEdge, int nDelta)
{
	// RETAIL: lets nPiece be 9 (one past the polygons)
	if (nPiece < 0 || nPiece > 9 || nEdge < 0 || nEdge > 4) return;

	LT_POLYFT4* pPoly = &m_Poly[nPiece];
	int x = (int)m_Poly[nPiece].verts[0].x;
	int y = (int)m_Poly[nPiece].verts[0].y;
	int w = (int)m_Poly[nPiece].verts[2].x - x;
	int h = (int)m_Poly[nPiece].verts[2].y - y;

	switch (nEdge)
	{
		case 0:
			x += nDelta;
			w -= nDelta;
			break;
		case 1:
			y += nDelta;
			h -= nDelta;
			break;
		case 2:
			w += nDelta;
			break;
		case 3:
			h += nDelta;
			break;
		default:
			return;
	}

	g_pDrawPrim->SetXYWH(pPoly, (float)x, (float)y, (float)w, (float)h);

	char szTmp[128];
	sprintf(szTmp, "%d: (%d %d)->(%d %d)", nPiece, x, y, x + w, y + h);
	m_pPieceStr[nPiece]->SetText(szTmp);
}

// RETAIL: 0x1013e8c0 (the "FrameUV" console program's)
void CTexturedFrame::AdjustPieceUV(int nPiece, int nEdge, int nDelta)
{
	if (nPiece < 0 || nPiece > 9 || nEdge < 0 || nEdge > 4) return;

	switch (nEdge)
	{
		case 0:
			m_Piece[nPiece].x += nDelta;
			break;
		case 1:
			m_Piece[nPiece].y += nDelta;
			break;
		case 2:
			m_Piece[nPiece].w += nDelta;
			break;
		case 3:
			m_Piece[nPiece].h += nDelta;
			break;
		default:
			return;
	}

	char szTmp[128];
	sprintf(szTmp, "<%d %d %d %d>", m_Piece[nPiece].x, m_Piece[nPiece].y, m_Piece[nPiece].x + m_Piece[nPiece].w, m_Piece[nPiece].y + m_Piece[nPiece].h);
	m_pUVStr[nPiece]->SetText(szTmp);

	uint32 nTexWidth, nTexHeight;
	g_pTexInterface->GetTextureDims(m_hTexture, nTexWidth, nTexHeight);
	nTexWidth--;
	nTexHeight--;

	SetupQuadUVs(m_Poly[nPiece], m_hTexture,
		(float)m_Piece[nPiece].x / (float)nTexWidth, (float)m_Piece[nPiece].y / (float)nTexHeight,
		(float)m_Piece[nPiece].w / (float)nTexWidth, (float)m_Piece[nPiece].h / (float)nTexHeight);
}

// RETAIL: 0x1013eb20
void CTexturedFrame::Render()
{
	if (m_bDirty)
	{
		Update();
	}

	if (!m_hTexture) return;

	g_pDrawPrim->SetTexture(m_hTexture);
	g_pDrawPrim->DrawPrim(m_Poly, m_bHollow ? kNumPieces - 1 : kNumPieces);

	if (m_bShowPieces)
	{
		for (int i = 0; i < kNumPieces; i++)
		{
			if (m_pPieceStr[i])
				m_pPieceStr[i]->Render();
			if (m_pUVStr[i])
				m_pUVStr[i]->Render();
		}
	}
}

// RETAIL: 0x1013eba0
LTIntPt CTexturedFrame::GetPosition()
{
	return LTIntPt(m_Pos.x, m_Pos.y);
}

// RETAIL: 0x1013ebc0
LTIntPt CTexturedFrame::GetSize()
{
	return LTIntPt(m_Size.x, m_Size.y);
}

// RETAIL: 0x1013ebe0
LTRect CTexturedFrame::GetRect()
{
	return LTRect(m_Pos.x, m_Pos.y, m_Pos.x + m_Size.x, m_Pos.y + m_Size.y);
}

// RETAIL: 0x1013ec10
void CTexturedFrame::SetPosition(int x, int y)
{
	if (x != m_Pos.x || y != m_Pos.y)
	{
		m_Pos.x = x;
		m_Pos.y = y;
		m_bDirty = true;
	}
}

// RETAIL: 0x1013ec40
void CTexturedFrame::SetSize(int w, int h)
{
	if (w != m_Size.x || h != m_Size.y)
	{
		m_Size.x = w;
		m_Size.y = h;
		m_bDirty = true;
	}
}

// RETAIL: 0x1013ec70
void CTexturedFrame::SetRect(LTRect rect)
{
	m_Pos = LTIntPt(rect.left, rect.top);
	m_Size.x = rect.right - rect.left;
	m_Size.y = rect.bottom - rect.top;
	m_bDirty = true;
}

// RETAIL: 0x1013eca0: lays the pieces' screen quads out around the rect,
// or inside it if inscribed. The numbers drawn are 0..8 left to right, top
// to bottom (the center is 4).
void CTexturedFrame::Update()
{
	if (!m_bDirty) return;
	m_bDirty = false;

	char szTmp[128];
	int x, y, w, h;

	if (m_bInscribe)
	{
		x = m_Pos.x;
		y = m_Pos.y;
		w = m_Piece[kTopLeft].w + 1;
		h = m_Piece[kTopLeft].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kTopLeft], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "0: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[0]->SetText(szTmp);

		x += w;
		w = m_Size.x - m_Piece[kTopLeft].w - m_Piece[kTopRight].w - 1;
		h = m_Piece[kTop].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kTop], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "1: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[1]->SetText(szTmp);

		x += w;
		w = m_Piece[kTopRight].w;
		h = m_Piece[kTopRight].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kTopRight], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "2: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[2]->SetText(szTmp);

		x = m_Pos.x;
		y = m_Pos.y + m_Piece[kTopLeft].h;
		w = m_Piece[kLeft].w + 1;
		h = m_Size.y - m_Piece[kBottomLeft].h - m_Piece[kTopLeft].h + 1;
		g_pDrawPrim->SetXYWH(&m_Poly[kLeft], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "3: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[3]->SetText(szTmp);

		x += w;
		y = m_Pos.y + m_Piece[kTop].h;
		w = m_Size.x - m_Piece[kLeft].w - m_Piece[kRight].w - 1;
		h = m_Size.y - m_Piece[kTop].h - m_Piece[kBottom].h + 1;
		g_pDrawPrim->SetXYWH(&m_Poly[kCenter], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "4: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[4]->SetText(szTmp);

		x += w;
		y = m_Pos.y + m_Piece[kTopRight].h;
		w = m_Piece[kRight].w;
		h = m_Size.y - m_Piece[kTopRight].h - m_Piece[kBottomRight].h + 1;
		g_pDrawPrim->SetXYWH(&m_Poly[kRight], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "5: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[5]->SetText(szTmp);

		x = m_Pos.x;
		y = m_Pos.y - m_Piece[kBottomLeft].h + m_Size.y + 1;
		w = m_Piece[kBottomLeft].w + 1;
		h = m_Piece[kBottomLeft].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kBottomLeft], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "6: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[6]->SetText(szTmp);

		x += w;
		y = m_Pos.y - m_Piece[kBottom].h + m_Size.y + 1;
		w = m_Size.x - m_Piece[kBottomLeft].w - m_Piece[kBottomRight].w - 1;
		h = m_Piece[kBottom].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kBottom], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "7: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[7]->SetText(szTmp);

		x += w;
		y = m_Pos.y - m_Piece[kBottomRight].h + m_Size.y + 1;
		w = m_Piece[kBottomRight].w;
		h = m_Piece[kBottomRight].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kBottomRight], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "8: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[8]->SetText(szTmp);
	}
	else
	{
		x = m_Pos.x - m_Piece[kTopLeft].w;
		y = m_Pos.y - m_Piece[kTopLeft].h;
		w = m_Piece[kTopLeft].w + 1;
		h = m_Piece[kTopLeft].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kTopLeft], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "0: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[0]->SetText(szTmp);

		x = m_Pos.x;
		y = m_Pos.y - m_Piece[kTop].h;
		w = m_Size.x;
		h = m_Piece[kTop].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kTop], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "1: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[1]->SetText(szTmp);

		x = m_Pos.x + m_Size.x;
		y = m_Pos.y - m_Piece[kTopRight].h;
		w = m_Piece[kTopRight].w;
		h = m_Piece[kTopRight].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kTopRight], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "2: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[2]->SetText(szTmp);

		x = m_Pos.x - m_Piece[kLeft].w;
		y = m_Pos.y;
		w = m_Piece[kLeft].w;
		h = m_Size.y;
		g_pDrawPrim->SetXYWH(&m_Poly[kLeft], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "3: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[3]->SetText(szTmp);

		x = m_Pos.x;
		w = m_Size.x;
		h = m_Size.y;
		g_pDrawPrim->SetXYWH(&m_Poly[kCenter], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "4: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[4]->SetText(szTmp);

		x = m_Pos.x + m_Size.x;
		w = m_Piece[kRight].w;
		g_pDrawPrim->SetXYWH(&m_Poly[kRight], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "5: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[5]->SetText(szTmp);

		x = m_Pos.x - m_Piece[kBottomLeft].w;
		y = m_Pos.y + m_Size.y;
		w = m_Piece[kBottomLeft].w;
		h = m_Piece[kBottomLeft].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kBottomLeft], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "6: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[6]->SetText(szTmp);

		x = m_Pos.x;
		w = m_Size.x;
		h = m_Piece[kBottom].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kBottom], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "7: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[7]->SetText(szTmp);

		x = m_Pos.x + m_Size.x;
		w = m_Piece[kBottomRight].w;
		h = m_Piece[kBottomRight].h;
		g_pDrawPrim->SetXYWH(&m_Poly[kBottomRight], (float)x, (float)y, (float)w, (float)h);
		sprintf(szTmp, "8: (%d %d)->(%d %d)", x, y, x + w, y + h);
		m_pPieceStr[8]->SetText(szTmp);
	}
}
