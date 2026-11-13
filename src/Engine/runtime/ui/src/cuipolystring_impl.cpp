//-------------------------------------------------------------------
//
//   MODULE    : CUIPOLYSTRING_IMPL.CPP
//
//   PURPOSE   : implements the CUIPolyString_Impl Class
//
//   CREATED   : 1/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------
//
// TRON: written from retail Lithtech.exe (0x562ba0..0x5645b0, and the
// inline accessors 0x565af0..0x565c40); see cuipolystring_impl.h.


#ifndef __CUIPOLYSTRING_IMPL_H__
#include "cuipolystring_impl.h"
#endif

#ifndef __CUIRENDERSTATE_H__
#include "cuirenderstate.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#include <math.h>
#include <string.h>


// use the internal drawprim interface (RETAIL: the holder's registration
// 0x562b60)
static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);


//  ---------------------------------------------------------------------------
// RETAIL: 0x562ba0
CUIPolyString_Impl::CUIPolyString_Impl()
{
	m_pColors[0] = m_pColors[1] = m_pColors[2] = m_pColors[3] = 0xffffffff;

	m_pPolys					= NULL;
	m_NumAllocatedPolys			= 0;
	m_pShadowPolys				= NULL;
	m_NumAllocatedShadowPolys	= 0;

	m_ShadowOffsetX		= 1;
	m_ShadowOffsetY		= 1;
	m_bDropShadow		= false;
	m_bClip				= false;
	m_bClipped			= false;
	m_bKeepMaps			= true;
	m_bGetExtents		= true;
	m_ScaledHeight		= 0;
	m_CharScreenHeight	= 0;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x562dd0
CUIPolyString_Impl::~CUIPolyString_Impl()
{
	if (m_pPolys) {
		delete [] m_pPolys;
		m_pPolys = NULL;
	}

	if (m_pShadowPolys) {
		delete [] m_pShadowPolys;
		m_pShadowPolys = NULL;
	}

	ClearPolyMap(m_PolyMap, true);
	ClearPolyMap(m_ShadowPolyMap, true);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x562f40
CUI_RESULTTYPE CUIPolyString_Impl::SetFont(CUIFont* pFont)
{
	m_pFont = pFont;

	uint32 nHeight = pFont->GetDefCharScreenHeight();
	float fHeight = (float)nHeight;
	m_CharScreenHeight = nHeight;

	float fScale = (float)GetFontHeight() / fHeight;
	m_ScaledHeight = (int32)(fHeight * fScale);

	ApplyFont();
	Format();
	Layout();

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x562fd0
CUI_RESULTTYPE CUIPolyString_Impl::SetCharScreenSize(uint32 height)
{
	m_CharScreenHeight = height;

	float fHeight = (float)m_pFont->GetDefCharScreenHeight();
	float fScale = fHeight / (float)GetFontHeight();
	m_ScaledHeight = (int32)((float)m_CharScreenHeight * fScale);

	Format();
	Layout();

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563060
CUI_RESULTTYPE CUIPolyString_Impl::SetText(const std::wstring& sText)
{
	m_Text = sText;

	AllocPolys();
	ApplyFont();
	ApplyColors(0, -1);
	Format();
	Layout();

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563120: widens each character (as unsigned)
CUI_RESULTTYPE CUIPolyString_Impl::SetText(const char* pBuffer)
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
// RETAIL: 0x5632a0
CUI_RESULTTYPE CUIPolyString_Impl::SetPosition(int32 x, int32 y)
{
	m_Rect.x = (float)x;
	m_Rect.y = (float)y;

	Format();
	Layout();

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5632d0
CUI_RESULTTYPE CUIPolyString_Impl::SetColors(uint32 argb0,
											 uint32 argb1,
											 uint32 argb2,
											 uint32 argb3)
{
	m_pColors[0] = argb0;
	m_pColors[1] = argb1;
	m_pColors[2] = argb2;
	m_pColors[3] = argb3;

	ApplyColors(0, -1);

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563300
CUI_RESULTTYPE CUIPolyString_Impl::SetClipRect(CUIRECT* pRect)
{
	if (!pRect)
		return CUIPS_ERROR;

	m_ClipRect = *pRect;

	Format();
	Layout();

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563350
CUI_RESULTTYPE CUIPolyString_Impl::GetClipRect(CUIRECT* pRect)
{
	if (!pRect)
		return CUIPS_ERROR;

	*pRect = m_ClipRect;

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563380
CUI_RESULTTYPE CUIPolyString_Impl::SetDropShadow(bool bDropShadow)
{
	m_bDropShadow = bDropShadow;

	if (bDropShadow)
	{
		uint32 nLen = m_Text.size();

		if (m_pShadowPolys && nLen > m_NumAllocatedShadowPolys)
		{
			delete [] m_pShadowPolys;
			m_pShadowPolys = NULL;
		}

		if (!m_pShadowPolys && m_bDropShadow)
		{
			int32 nPolys = ((int32)nLen < 1) ? 1 : nLen;
			m_NumAllocatedShadowPolys = nPolys;
			LT_MEM_TRACK_ALLOC(m_pShadowPolys = new LT_POLYGT4[nPolys],LT_MEM_TYPE_UI);
		}

		ApplyFont();
		ApplyColors(0, -1);
		Layout();
	}
	else if (m_pShadowPolys)
	{
		delete [] m_pShadowPolys;
		m_pShadowPolys = NULL;
		m_NumAllocatedShadowPolys = 0;
	}

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5634a0
CUI_RESULTTYPE CUIPolyString_Impl::EnableClipping(bool bClip)
{
	m_bClip = bClip;
	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5634b0: (start, count); a string on one font page draws its
// polygons in one call, any other page by page from the maps
CUI_RESULTTYPE CUIPolyString_Impl::Render(int32 start, int32 end)
{
	if (!m_pPolys)
		return CUIPS_ERROR;

	if (!m_pFont)
		return CUIPS_ERROR;

	CUIRenderState::SetRenderState(NULL);

	if (m_bOnePage)
	{
		CUIRenderState::SetRenderState(m_pFont->GetTexture(m_Page));

		if (!start && end == -1)
		{
			if (m_pShadowPolys)
			{
				pDrawPrimInternal->DrawPrim(m_pShadowPolys, m_Text.size());
				CUIRenderState::SetRenderState(m_pFont->GetTexture(m_Page));
			}

			pDrawPrimInternal->DrawPrim(m_pPolys, m_Text.size());
			return CUIPS_OK;
		}

		if (m_pShadowPolys)
		{
			pDrawPrimInternal->DrawPrim(&m_pShadowPolys[start], end);
			CUIRenderState::SetRenderState(m_pFont->GetTexture(m_Page));
		}

		pDrawPrimInternal->DrawPrim(&m_pPolys[start], end);
		return CUIPS_OK;
	}

	CUIPolyMap::iterator iter;

	for (iter = m_ShadowPolyMap.begin(); iter != m_ShadowPolyMap.end(); iter++)
	{
		CUIPolyVector* pPolys = (*iter).second;
		if (pPolys->begin() != pPolys->end())
		{
			CUIRenderState::SetRenderState(m_pFont->GetTexture((*iter).first));
			pDrawPrimInternal->DrawPrim(pPolys->begin(), pPolys->size());
		}
	}

	for (iter = m_PolyMap.begin(); iter != m_PolyMap.end(); ++iter)
	{
		CUIPolyVector* pPolys = (*iter).second;
		if (pPolys->begin() != pPolys->end())
		{
			CUIRenderState::SetRenderState(m_pFont->GetTexture((*iter).first));
			pDrawPrimInternal->DrawPrim(pPolys->begin(), pPolys->size());
		}
	}

	return CUIPS_OK;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5636b0
CUI_RESULTTYPE CUIPolyString_Impl::RenderClipped(CUIRECT* pClipRect, int32 start, int32 end)
{
	if (m_bClip && !m_bClipped)
	{
		m_ClipRect = *pClipRect;
		ClipPolys(m_pPolys);
		ClipPolys(m_pShadowPolys);
	}

	return Render(start, end);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563710 (slot 32): one polygon per character
void CUIPolyString_Impl::AllocPolys()
{
	if (m_pPolys && m_Text.size() > m_NumAllocatedPolys)
	{
		delete [] m_pPolys;
		m_pPolys = NULL;

		if (m_pShadowPolys)
		{
			delete [] m_pShadowPolys;
			m_pShadowPolys = NULL;
		}
	}

	if (!m_pPolys)
	{
		int32 nPolys = m_Text.size();
		if (nPolys < 1) nPolys = 1;

		m_NumAllocatedPolys = nPolys;
		LT_MEM_TRACK_ALLOC(m_pPolys = new LT_POLYGT4[nPolys],LT_MEM_TYPE_UI);
	}

	if (!m_pShadowPolys && m_bDropShadow)
	{
		LT_MEM_TRACK_ALLOC(m_pShadowPolys = new LT_POLYGT4[m_NumAllocatedPolys],LT_MEM_TYPE_UI);
	}
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563840 (slot 40)
void CUIPolyString_Impl::AddPolyToMap(CUIPolyMap& map, uint8 page, LT_POLYGT4* pPoly)
{
	CUIPolyVector* pPolys;

	CUIPolyMap::iterator iter = map.find(page);
	if (iter == map.end())
	{
		LT_MEM_TRACK_ALLOC(pPolys = new CUIPolyVector,LT_MEM_TYPE_UI);
		pPolys->reserve(8);
		map[page] = pPolys;
	}
	else
	{
		pPolys = (*iter).second;
	}

	pPolys->push_back(pPoly);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5639b0 (slot 41): empties each page's vector; frees the vectors
// (and empties the map) unless they are kept
void CUIPolyString_Impl::ClearPolyMap(CUIPolyMap& map, bool bDestroy)
{
	CUIPolyMap::iterator iter = map.begin();
	while (iter != map.end())
	{
		CUIPolyVector* pPolys = (*iter).second;
		iter++;

		pPolys->clear();

		if (bDestroy || !m_bKeepMaps)
		{
			delete pPolys;
		}
	}

	if (bDestroy || !m_bKeepMaps)
	{
		map.clear();
	}
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563ab0 (slot 34): places each character's polygon
void CUIPolyString_Impl::Layout()
{
	if (m_Text.empty() || !m_pFont)
		return;

	int32 nLen			= m_Text.size();
	int32 nFontHeight	= m_pFont->GetDefCharScreenHeight();
	int32 x				= (int32)m_Rect.x;
	int32 y				= (int32)m_Rect.y;
	int32 nHeight		= m_ScaledHeight;

	for (int32 i = 0; i < nLen; i++)
	{
		uint16 c = m_Text[i];
		int32 nWidth;

		switch (c)
		{
			case '\n':
				nWidth = 0;
				x = (int32)m_Rect.x;
				y += m_pFont->GetLineSpacing() + m_CharScreenHeight;
				pDrawPrimInternal->SetXYWH(&m_pPolys[i], (float)x, (float)y, 0, 0);
				if (m_pShadowPolys)
					pDrawPrimInternal->SetXYWH(&m_pShadowPolys[i], (float)x, (float)y, 0, 0);
				break;

			case '\r':
				nWidth = 0;
				pDrawPrimInternal->SetXYWH(&m_pPolys[i], (float)x, (float)y, 0, 0);
				if (m_pShadowPolys)
					pDrawPrimInternal->SetXYWH(&m_pShadowPolys[i], (float)x, (float)y, 0, 0);
				break;

			case ' ':
				nWidth = (int32)ceil((float)m_pFont->GetSpaceWidth() * (float)nHeight / nFontHeight);
				pDrawPrimInternal->SetXYWH(&m_pPolys[i], (float)x, (float)y, (float)nWidth, (float)nHeight);
				if (m_pShadowPolys)
					pDrawPrimInternal->SetXYWH(&m_pShadowPolys[i], (float)x, (float)y, 0, 0);
				break;

			default:
			{
				CUIWideFontChar* pChar = CUIGetFontChar(m_pFont, c);

				nWidth = (int32)ceil((float)pChar->m_Width * (float)nHeight / nFontHeight);
				pDrawPrimInternal->SetXYWH(&m_pPolys[i], (float)x, (float)y, (float)nWidth, (float)nHeight);
				if (m_pShadowPolys)
					pDrawPrimInternal->SetXYWH(&m_pShadowPolys[i],
											   (float)(x + m_ShadowOffsetX),
											   (float)(y + m_ShadowOffsetY),
											   (float)nWidth, (float)nHeight);
				break;
			}
		}

		x += m_pFont->GetCharSpacing() + nWidth;
	}

	GetExtents();
	ClearClipped();
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563e20 (slot 36): (start, count); white space is transparent
void CUIPolyString_Impl::ApplyColors(uint32 start, int32 count)
{
	if (m_Text.empty() || !m_pFont)
		return;

	if (count < 0)
		count = m_Text.size();

	uint32 end = start + count;
	for (uint32 i = start; i < end; i++)
	{
		switch (m_Text[i])
		{
			case '\n':
			case '\r':
			case ' ':
				pDrawPrimInternal->SetRGBA4(&m_pPolys[i], 0, 0, 0, 0);
				if (m_pShadowPolys)
					pDrawPrimInternal->SetRGBA4(&m_pShadowPolys[i], 0, 0, 0, 0);
				break;

			default:
				pDrawPrimInternal->SetRGBA4(&m_pPolys[i], m_pColors[0], m_pColors[1], m_pColors[2], m_pColors[3]);
				// RETAIL: the shadow's first vertex takes the text's colour
				// (docs/RETAIL_BUGS.md)
				if (m_pShadowPolys)
					pDrawPrimInternal->SetRGBA4(&m_pShadowPolys[i], m_pColors[0], 0xff000000, 0xff000000, 0xff000000);
				break;
		}
	}

	if (m_bClip)
		m_bClipped = false;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x563f40 (slot 33): each character's place on its font page, and
// the polygons by page
void CUIPolyString_Impl::ApplyFont()
{
	if (m_Text.empty() || !m_pFont)
		return;

	int32 nLen	= m_Text.size();
	int32 nPage	= -1;

	ClearPolyMap(m_PolyMap, false);
	ClearPolyMap(m_ShadowPolyMap, false);

	m_bOnePage = true;

	for (int32 i = 0; i < nLen; i++)
	{
		uint16 c = m_Text[i];

		if (c == '\n' || c == '\r' || c == ' ')
		{
			pDrawPrimInternal->SetUVWH(&m_pPolys[i], 0, 0, 0, 0);
			if (m_pShadowPolys)
				pDrawPrimInternal->SetUVWH(&m_pShadowPolys[i], 0, 0, 0, 0);
			continue;
		}

		CUIWideFontChar* pChar = CUIGetFontChar(m_pFont, c);
		float fWidth = (float)pChar->m_Width;

		if (nPage < 0)
		{
			m_Page = pChar->m_Page;
			nPage = pChar->m_Page;
		}
		else if (nPage != pChar->m_Page)
		{
			m_bOnePage = false;
		}

		pDrawPrimInternal->SetUVWH(&m_pPolys[i], m_pFont->GetTexture(pChar->m_Page),
								   (float)pChar->m_U, (float)pChar->m_V,
								   fWidth, (float)m_pFont->GetDefCharScreenHeight());
		AddPolyToMap(m_PolyMap, pChar->m_Page, &m_pPolys[i]);

		if (m_pShadowPolys)
		{
			pDrawPrimInternal->SetUVWH(&m_pShadowPolys[i], m_pFont->GetTexture(pChar->m_Page),
									   (float)pChar->m_U, (float)pChar->m_V,
									   fWidth, (float)m_pFont->GetDefCharScreenHeight());
			AddPolyToMap(m_ShadowPolyMap, pChar->m_Page, &m_pShadowPolys[i]);
		}
	}
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x564250 (slot 35): hides each polygon not wholly inside the clip
// rectangle (NOLF2's cut polygons at its edges)
void CUIPolyString_Impl::ClipPolys(LT_POLYGT4* pPolys)
{
	if (!m_bClip || m_Text.empty() || !m_pFont || !pPolys)
		return;

	float left		= m_ClipRect.x;
	float right		= m_ClipRect.x + m_ClipRect.width;
	float top		= m_ClipRect.y;
	float bottom	= m_ClipRect.y + m_ClipRect.height;

	int32 nLen = m_Text.size();
	for (int32 i = 0; i < nLen; i++, pPolys++)
	{
		float minx = pPolys->verts[0].x;
		float maxx = pPolys->verts[0].x;
		float miny = pPolys->verts[0].y;
		float maxy = pPolys->verts[0].y;

		for (int32 v = 1; v < 4; v++)
		{
			if (pPolys->verts[v].x > maxx) maxx = pPolys->verts[v].x;
			if (pPolys->verts[v].x < minx) minx = pPolys->verts[v].x;
			if (pPolys->verts[v].y > maxy) maxy = pPolys->verts[v].y;
			if (pPolys->verts[v].y < miny) miny = pPolys->verts[v].y;
		}

		// wholly outside
		if (minx > right || maxx < left || miny > bottom || maxy < top)
		{
			pDrawPrimInternal->SetRGBA4(pPolys, 0, 0, 0, 0);
		}
		// partly outside
		else if (minx < left || maxx > right || miny < top || maxy > bottom)
		{
			pDrawPrimInternal->SetRGBA4(pPolys, 0, 0, 0, 0);
		}
	}

	m_bClipped = true;
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5643f0 (slot 38): the rectangle the polygons cover
void CUIPolyString_Impl::GetExtents()
{
	if (!m_bGetExtents)
		return;

	int32 minx = 0;
	int32 maxx = 0;
	int32 miny = 0;
	int32 maxy = 0;

	LT_POLYGT4* pPoly = m_pPolys;
	if (pPoly)
	{
		minx = maxx = (int32)pPoly->verts[0].x;
		miny = maxy = (int32)pPoly->verts[0].y;

		for (uint32 i = 0; i < m_Text.size(); i++, pPoly++)
		{
			int32 v;

			for (v = 0; v < 4; v++)
				if (maxx <= (int32)pPoly->verts[v].x) maxx = (int32)pPoly->verts[v].x;
			for (v = 0; v < 4; v++)
				if (minx >= (int32)pPoly->verts[v].x) minx = (int32)pPoly->verts[v].x;
			for (v = 0; v < 4; v++)
				if (maxy <= (int32)pPoly->verts[v].y) maxy = (int32)pPoly->verts[v].y;
			for (v = 0; v < 4; v++)
				if (miny >= (int32)pPoly->verts[v].y) miny = (int32)pPoly->verts[v].y;
		}
	}

	m_Rect.x		= (float)minx;
	m_Rect.y		= (float)miny;
	m_Rect.width	= (float)(maxx - minx);
	m_Rect.height	= (float)(maxy - miny);
}


//  ---------------------------------------------------------------------------
// RETAIL: 0x5645b0 (slot 42)
uint32 CUIPolyString_Impl::GetFontHeight()
{
	return m_pFont->GetDefCharScreenHeight();
}
