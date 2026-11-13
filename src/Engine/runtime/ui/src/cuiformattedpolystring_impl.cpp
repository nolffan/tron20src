//-------------------------------------------------------------------
//
//   MODULE    : CUIFORMATTEDPOLYSTRING_IMPL.CPP
//
//   PURPOSE   : implements the CUIFormattedPolyString_Impl Class
//
//   CREATED   : 1/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------
//
// TRON: written from retail Lithtech.exe (0x568550..0x569920); see
// cuiformattedpolystring_impl.h.


#ifndef __CUIFORMATTEDPOLYSTRING_IMPL_H__
#include "cuiformattedpolystring_impl.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#include <math.h>
#include <string.h>


// RETAIL: the holder at 0x5ecd78 (its registration 0x568510): the default
// drawprim, where the polystring's is the internal one
static ILTDrawPrim *pDrawPrim = NULL;
define_holder(ILTDrawPrim, pDrawPrim);


// the colour tag
static const wchar_t sColorTag[] = L"<color ";


//  ---------------------------------------------------------------------------
// RETAIL: 0x568550
CUIFormattedPolyString_Impl::CUIFormattedPolyString_Impl()
{
	m_WrapWidth = 0;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x568640
CUIFormattedPolyString_Impl::~CUIFormattedPolyString_Impl()
{
	m_Lines.clear();
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x568750
CUI_RESULTTYPE CUIFormattedPolyString_Impl::SetWrapWidth(uint32 wrap)
{
	m_WrapWidth = wrap;

	Format();
	Layout();

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x568780
CUI_RESULTTYPE CUIFormattedPolyString_Impl::SetAlignmentH(CUI_ALIGNMENTTYPE halign)
{
	m_Alignment = halign;

	Format();
	Layout();

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5687b0: the polystring's, again
CUI_RESULTTYPE CUIFormattedPolyString_Impl::SetText(const char* pBuffer)
{
	if (!pBuffer)
		return CUIPS_ERROR;

	std::wstring sText;
	sText.erase(sText.begin(), sText.end());

	uint32 nLen = strlen(pBuffer);
	for (uint32 i = 0; i < nLen; i++)
	{
		sText += (wchar_t)(uint8)pBuffer[i];
	}

	return SetText(sText);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x568930: keeps the text, and gives the polystring the text without
// its colour tags (at most 1,021 characters)
CUI_RESULTTYPE CUIFormattedPolyString_Impl::SetText(const std::wstring& sText)
{
	wchar_t sBuffer[1024];

	m_RawText = sText;

	int32 nLen = m_RawText.size() + 1;
	if (nLen > 1024)
	{
		nLen = 1022;
		m_RawText.resize(nLen);
	}

	memset(sBuffer, 0, nLen * sizeof(wchar_t));

	const wchar_t* pSrc = m_RawText.data();
	wchar_t* pDest = sBuffer;
	bool bInTag = false;

	while (*pSrc)
	{
		if (bInTag)
		{
			if (*pSrc == '>')
				bInTag = false;
		}
		else if (*pSrc == '<')
		{
			std::wstring sTag(sColorTag);
			std::wstring sHere(pSrc);

			if (sHere.compare(0, 7, sTag) == 0)
				bInTag = true;

			if (!bInTag)
				*pDest++ = *pSrc;
		}
		else
		{
			*pDest++ = *pSrc;
		}

		pSrc++;
	}

	std::wstring sStripped(sBuffer);
	return CUIPolyString_Impl::SetText(sStripped);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x568bb0 (slot 37): breaks the text into lines: at a line feed, and
// where a character would pass the wrap width, at the last space (or before
// the character, when the line has none)
void CUIFormattedPolyString_Impl::Format()
{
	if (m_Text.empty() || !m_pFont)
		return;

	int32 nLen			= m_Text.size();
	int32 nWordStart	= 0;
	bool bAtBreak		= true;		// the last character was white space
	bool bNoBreak		= true;		// the line has no white space yet
	int32 nFontHeight	= m_pFont->GetDefCharScreenHeight();
	int32 nHeight		= m_ScaledHeight;
	CUIFormatLine line;

	m_Lines.clear();

	int32 x = 0;
	int32 i;
	for (i = 0; i < nLen; i++)
	{
		uint16 c = m_Text[i];
		int32 nWidth;

		switch (c)
		{
			case '\n':
				line.m_LastChar	= i;
				line.m_Width	= x;
				m_Lines.push_back(line);
				nWidth			= 0;
				x				= 0;
				line.m_Width	= 0;
				bAtBreak		= false;
				bNoBreak		= true;
				break;

			case '\r':
				if (!bAtBreak)
				{
					line.m_Width	= x;
					line.m_LastChar	= i;
					bAtBreak		= true;
					bNoBreak		= false;
				}
				nWidth = 0;
				break;

			case ' ':
				if (!bAtBreak)
				{
					line.m_Width	= x;
					line.m_LastChar	= i;
					bAtBreak		= true;
					bNoBreak		= false;
				}
				nWidth = m_pFont->GetSpaceWidth() * nHeight / nFontHeight;
				break;

			default:
			{
				CUIWideFontChar* pChar = CUIGetFontChar(m_pFont, c);
				nWidth = (int32)ceil((float)pChar->m_Width * nHeight / nFontHeight);

				if (bAtBreak)
				{
					bAtBreak	= false;
					nWordStart	= i;
				}

				if (i && i > nWordStart && m_WrapWidth > 0 &&
					(int32)(m_pFont->GetCharSpacing() + nWidth + x) > (int32)m_WrapWidth)
				{
					if (bNoBreak)
					{
						// no white space on the line: break before this
						line.m_Width	= x;
						line.m_LastChar	= --i;
					}
					else
					{
						// back to the start of the word
						line.m_LastChar	= nWordStart - 1;
						i				= nWordStart - 1;
					}

					bAtBreak = true;
					bNoBreak = true;
					m_Lines.push_back(line);

					x = -(m_pFont->GetCharSpacing() + nWidth);
				}
				break;
			}
		}

		x += m_pFont->GetCharSpacing() + nWidth;
	}

	line.m_LastChar	= i - 1;
	line.m_Width	= x;
	m_Lines.push_back(line);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x568f00 (slot 34): the polystring's, line by line
void CUIFormattedPolyString_Impl::Layout()
{
	if (m_Text.empty() || !m_pFont)
		return;

	uint32 nLen			= m_Text.size();
	int32 nFontHeight	= m_pFont->GetDefCharScreenHeight();
	std::list<CUIFormatLine>::iterator iLine = m_Lines.begin();

	int32 x				= (int32)m_Rect.x + GetLineOffset(*iLine);
	int32 y				= (int32)m_Rect.y;
	int32 nHeight		= m_ScaledHeight;

	for (uint32 i = 0; i < nLen; i++)
	{
		uint16 c = m_Text[i];
		int32 nWidth;

		switch (c)
		{
			case '\n':
				nWidth = 0;
				pDrawPrim->SetXYWH(&m_pPolys[i], (float)x, (float)y, 0, 0);
				if (m_pShadowPolys)
					pDrawPrim->SetXYWH(&m_pShadowPolys[i], (float)x, (float)y, 0, 0);
				break;

			case '\r':
				nWidth = 0;
				pDrawPrim->SetXYWH(&m_pPolys[i], (float)x, (float)y, 0, 0);
				if (m_pShadowPolys)
					pDrawPrim->SetXYWH(&m_pShadowPolys[i], (float)x, (float)y, 0, 0);
				break;

			case ' ':
				nWidth = (int32)ceil((float)m_pFont->GetSpaceWidth() * (float)nHeight / nFontHeight);
				pDrawPrim->SetXYWH(&m_pPolys[i], (float)x, (float)y, (float)nWidth, (float)nHeight);
				if (m_pShadowPolys)
					pDrawPrim->SetXYWH(&m_pShadowPolys[i], (float)x, (float)y, 0, 0);
				break;

			default:
			{
				CUIWideFontChar* pChar = CUIGetFontChar(m_pFont, c);

				nWidth = (int32)ceil((float)pChar->m_Width * (float)nHeight / nFontHeight);
				pDrawPrim->SetXYWH(&m_pPolys[i], (float)x, (float)y, (float)nWidth, (float)nHeight);
				if (m_pShadowPolys)
					pDrawPrim->SetXYWH(&m_pShadowPolys[i],
									   (float)(x + m_ShadowOffsetX),
									   (float)(y + m_ShadowOffsetY),
									   (float)nWidth, (float)nHeight);
				break;
			}
		}

		x += m_pFont->GetCharSpacing() + nWidth;

		if (i == (uint32)(*iLine).m_LastChar)
		{
			++iLine;
			x = (int32)m_Rect.x + GetLineOffset(*iLine);
			y += m_pFont->GetLineSpacing() + m_CharScreenHeight;
		}
	}

	GetExtents();
	ClearClipped();
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5692e0 (slot 38): with a wrap width, the string is as wide as it
void CUIFormattedPolyString_Impl::GetExtents()
{
	if (!m_bGetExtents)
		return;

	int32 minx = (int32)m_Rect.x;
	int32 maxx = minx + m_WrapWidth;
	int32 miny = 0;
	int32 maxy = 0;

	LT_POLYGT4* pPolys = m_pPolys;
	if (pPolys)
	{
		uint32 i;
		LT_POLYGT4* pPoly;

		miny = maxy = (int32)pPolys->verts[0].y;

		for (i = 0, pPoly = pPolys; i < m_Text.size(); i++, pPoly++)
		{
			maxy = LTMAX(maxy, pPoly->verts[0].y);
			maxy = LTMAX(maxy, pPoly->verts[1].y);
			maxy = LTMAX(maxy, pPoly->verts[2].y);
			maxy = LTMAX(maxy, pPoly->verts[3].y);
			miny = LTMIN(miny, pPoly->verts[0].y);
			miny = LTMIN(miny, pPoly->verts[1].y);
			miny = LTMIN(miny, pPoly->verts[2].y);
			miny = LTMIN(miny, pPoly->verts[3].y);
		}

		if (!m_WrapWidth)
		{
			minx = maxx = (int32)pPolys->verts[0].x;

			for (i = 0, pPoly = pPolys; i < m_Text.size(); i++, pPoly++)
			{
				maxx = LTMAX(maxx, pPoly->verts[0].x);
				maxx = LTMAX(maxx, pPoly->verts[1].x);
				maxx = LTMAX(maxx, pPoly->verts[2].x);
				maxx = LTMAX(maxx, pPoly->verts[3].x);
				minx = LTMIN(minx, pPoly->verts[0].x);
				minx = LTMIN(minx, pPoly->verts[1].x);
				minx = LTMIN(minx, pPoly->verts[2].x);
				minx = LTMIN(minx, pPoly->verts[3].x);
			}
		}
	}

	m_Rect.x		= (float)minx;
	m_Rect.y		= (float)miny;
	m_Rect.width	= (float)(maxx - minx);
	m_Rect.height	= (float)(maxy - miny);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x569500 (slot 36): a text with colour tags takes its colours from
// them
void CUIFormattedPolyString_Impl::ApplyColors(uint32 start, int32 count)
{
	if (m_Text.empty() || !m_pFont)
		return;

	if (m_Text.size() != m_RawText.size())
		ApplyFormattedColors(start, count);
	else
		CUIPolyString_Impl::ApplyColors(start, count);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x569550: every character's colour, from the tags before it (the
// first colour until the first tag); the range is not used. The shadow is
// black, at the text's alpha.
void CUIFormattedPolyString_Impl::ApplyFormattedColors(uint32 start, int32 count)
{
	uint16 nLen = m_Text.size();
	const wchar_t* pRaw = m_RawText.data();
	std::wstring sTag(sColorTag);
	uint32 color = m_pColors[0];
	int32 nPoly = 0;

	for (uint16 i = nLen; i; i--)
	{
		if (*pRaw == '<')
		{
			std::wstring sHere(pRaw);

			if (sHere.compare(0, 7, sTag) == 0)
			{
				pRaw += 7;
				color = ParseColor(pRaw, color);

				while (*pRaw != '>' && *pRaw)
					pRaw++;

				if (*pRaw == '>')
					pRaw++;
			}
		}

		int32 v;
		for (v = 0; v < 4; v++)
		{
			m_pPolys[nPoly].verts[v].rgba.b = (uint8)color;
			m_pPolys[nPoly].verts[v].rgba.g = (uint8)(color >> 8);
			m_pPolys[nPoly].verts[v].rgba.r = (uint8)(color >> 16);
			m_pPolys[nPoly].verts[v].rgba.a = (uint8)(color >> 24);
		}

		if (m_pShadowPolys)
		{
			for (v = 0; v < 4; v++)
			{
				m_pShadowPolys[nPoly].verts[v].rgba.b = 0;
				m_pShadowPolys[nPoly].verts[v].rgba.g = 0;
				m_pShadowPolys[nPoly].verts[v].rgba.r = 0;
				m_pShadowPolys[nPoly].verts[v].rgba.a = (uint8)(color >> 24);
			}
		}

		nPoly++;
		pRaw++;
	}
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x569830: "r g b" (decimal, any separators) after a colour tag; the
// alpha is the colour before it
uint32 CUIFormattedPolyString_Impl::ParseColor(const wchar_t* pStr, uint32 color)
{
	uint32 a = color & 0xff000000;
	uint32 r = 0;
	uint32 g = 0;
	uint32 b = 0;

	while (*pStr < '0' || *pStr > '9') pStr++;
	while (*pStr >= '0' && *pStr <= '9') r = r * 10 + (uint8)*pStr++ - '0';

	while (*pStr < '0' || *pStr > '9') pStr++;
	while (*pStr >= '0' && *pStr <= '9') g = g * 10 + (uint8)*pStr++ - '0';

	while (*pStr < '0' || *pStr > '9') pStr++;
	while (*pStr >= '0' && *pStr <= '9') b = b * 10 + (uint8)*pStr++ - '0';

	return a | ((uint8)r << 16) | ((uint8)g << 8) | (uint8)b;
}
