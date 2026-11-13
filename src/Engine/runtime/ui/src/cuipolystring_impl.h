//-------------------------------------------------------------------
//
//   MODULE    : CUIPolyString_Impl.h
//
//   PURPOSE   : defines the CUIPolyString_Impl Class
//
//   CREATED   : 7/00
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------
//
// TRON: Tron 2.0's polystring (no source survives; written from retail
// Lithtech.exe: constructor 0x562ba0, vtable 0x59182c, 0x84 bytes). It is
// not NOLF2's bridge and implementation pair: the engine's one class
// implements the SDK's CUIPolyString directly (the game calls its virtuals,
// progress/engine/CUIPolyString-vtable.csv). The text is a std::wstring;
// each character is one polygon (and one more for its drop shadow), and the
// polygons are kept by font page in two maps, so that Render sets each
// page's texture once. The class name (retail has no RTTI) and the names
// of the members are ours.

#ifndef __CUIPOLYSTRING_IMPL_H__
#define __CUIPOLYSTRING_IMPL_H__


#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif

#ifndef __CUIWIDEBITMAPFONT_H__
#include "cuiwidebitmapfont.h"
#endif

#include <map>
#include <vector>


// TRON: Tron's polystrings return 0 for success and 1 for a NULL argument
// (RETAIL: 0x5632a0, 0x563300): LT_OK and LT_ERROR, where NOLF2's returned
// CUIR_OK (1) and CUIR_ERROR (0).
#define CUIPS_OK		((CUI_RESULTTYPE)LT_OK)
#define CUIPS_ERROR		((CUI_RESULTTYPE)LT_ERROR)


// the polygons of one font page
typedef std::vector<LT_POLYGT4*> CUIPolyVector;

struct CUIPolyString::CUIPolyMap : public std::map<uint8, CUIPolyVector*>
{
};


// a character's entry in a wide font: the font's remap first, its default
// character for one it lacks (RETAIL: inline in every layout loop, 0x563b42)
inline CUIWideFontChar* CUIGetFontChar(CUIFont* pFont, uint16 c)
{
	if (pFont->IsRemapped() && pFont->GetRemapTables()[c >> 8])
	{
		c = pFont->GetRemapTables()[c >> 8][c & 0xff];
	}

	if (!((CUIWideFontChar***)pFont->GetCharTables())[c >> 8] ||
		!((CUIWideFontChar***)pFont->GetCharTables())[c >> 8][c & 0xff])
	{
		c = pFont->GetDefaultChar();
	}

	return ((CUIWideFontChar***)pFont->GetCharTables())[c >> 8][c & 0xff];
}


class CUIPolyString_Impl : public CUIPolyString
{
	public:

		CUIPolyString_Impl();
		virtual ~CUIPolyString_Impl();

		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont);
		virtual CUI_RESULTTYPE	SetText(const char* pBuffer);
		virtual CUI_RESULTTYPE	SetText(const std::wstring& sText);
		virtual CUI_RESULTTYPE	SetPosition(int32 x, int32 y);
		virtual CUI_RESULTTYPE	TronUnknownVirtual5(bool bValue)	{ m_bKeepMaps = bValue; return CUIPS_OK; }
		virtual CUI_RESULTTYPE	SetColors(uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);
		virtual CUI_RESULTTYPE	SetColor(uint32 argb)	{ return SetColors(argb, argb, argb, argb); }
		virtual CUI_RESULTTYPE	SetClipRect(CUIRECT* pRect);
		virtual CUI_RESULTTYPE	SetCharScreenSize(uint32 height);

		virtual uint32			GetColor()		{ return m_pColors[0]; }
		virtual CUIFont*		GetFont()		{ return m_pFont; }
		virtual const std::wstring&	GetText()	{ return m_Text; }
		virtual CUI_RESULTTYPE	GetRect(CUIRECT* pRect)	{ *pRect = m_Rect; return CUIPS_OK; }
		virtual CUI_RESULTTYPE	GetPosition(int32* pX, int32* pY)
		{
			*pX = (int32)m_Rect.x;
			*pY = (int32)m_Rect.y;
			return CUIPS_OK;
		}
		virtual CUI_RESULTTYPE	GetDims(int32* pWidth, int32* pHeight)
		{
			*pWidth  = (int32)m_Rect.width;
			*pHeight = (int32)m_Rect.height;
			return CUIPS_OK;
		}
		virtual int32			GetWidth()		{ return (int32)m_Rect.width; }
		virtual int32			GetHeight()		{ return (int32)m_Rect.height; }
		virtual int32			GetX()			{ return (int32)m_Rect.x; }
		virtual int32			GetY()			{ return (int32)m_Rect.y; }
		virtual bool			TronUnknownVirtual20()	{ return m_bKeepMaps; }
		virtual uint32			GetLength()		{ return m_Text.size(); }
		virtual CUI_RESULTTYPE	GetClipRect(CUIRECT* pRect);
		virtual uint32			GetCharScreenHeight()	{ return m_CharScreenHeight; }
		virtual LT_POLYGT4*		GetPolys()		{ return m_pPolys; }
		virtual LT_POLYGT4*		GetShadowPolys()	{ return m_pShadowPolys; }
		virtual CUI_RESULTTYPE	SetDropShadow(bool bDropShadow);
		virtual CUI_RESULTTYPE	EnableClipping(bool bClip);
		virtual CUI_RESULTTYPE	Render(int32 start = 0, int32 end = -1);
		virtual CUI_RESULTTYPE	RenderClipped(CUIRECT* pClipRect,
											  int32 start = 0,
											  int32 end = -1);
		virtual bool			TronUnknownVirtual30(CUI_ALIGNMENTTYPE align)	{ return true; }
		virtual CUI_RESULTTYPE	SetCharScreenHeight(uint32 height)	{ return SetCharScreenSize(height); }

	protected:

		virtual void			AllocPolys();
		virtual void			ApplyFont();
		virtual void			Layout();
		virtual void			ClipPolys(LT_POLYGT4* pPolys);
		virtual void			ApplyColors(uint32 start, int32 count);
		virtual void			Format()		{}
		virtual void			GetExtents();
		virtual void			ClearClipped()	{ m_bClipped = false; }
		virtual void			AddPolyToMap(CUIPolyMap& map, uint8 page, LT_POLYGT4* pPoly);
		virtual void			ClearPolyMap(CUIPolyMap& map, bool bDestroy);
		virtual uint32			GetFontHeight();

	protected:

		LT_POLYGT4*		m_pPolys;					// +0x4: one per character
		LT_POLYGT4*		m_pShadowPolys;				// +0x8: the drop shadow's
		uint32			m_NumAllocatedPolys;		// +0xc
		uint32			m_NumAllocatedShadowPolys;	// +0x10 (SetDropShadow's)
		std::wstring	m_Text;						// +0x14
		CUIFont*		m_pFont;					// +0x20
		bool			m_bOnePage;					// +0x24: every character on one page,
		uint8			m_Page;						// +0x25: this one (ApplyFont)
		bool			m_bKeepMaps;				// +0x26: ClearPolyMap keeps the vectors
		CUIRECT			m_Rect;						// +0x28: the extents
		CUIRECT			m_ClipRect;					// +0x38
		bool			m_bClip;					// +0x48: RenderClipped clips
		bool			m_bClipped;					// +0x49: the polygons are clipped
		uint32			m_pColors[4];				// +0x4c
		int8			m_ShadowOffsetX;			// +0x5c
		int8			m_ShadowOffsetY;			// +0x5d
		bool			m_bDropShadow;				// +0x5e
		CUIPolyMap		m_PolyMap;					// +0x60
		CUIPolyMap		m_ShadowPolyMap;			// +0x6c
		bool			m_bGetExtents;				// +0x78: GetExtents measures
		int32			m_ScaledHeight;				// +0x7c: a character's height on screen
		uint32			m_CharScreenHeight;			// +0x80
};


#endif //__CUIPOLYSTRING_IMPL_H__
