//-------------------------------------------------------------------
//
//   MODULE    : CUIPOLYSTRING.H
//
//   PURPOSE   : defines the CUIPolyString bridge Class
//
//   CREATED   : 7/00 
//
//-------------------------------------------------------------------


#ifndef __CUIPOLYSTRING_H__
#define __CUIPOLYSTRING_H__

 

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif

// TRON: Tron's text is wide.
#include <string>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
// forward declaration for the CUIBase Implementation class 
class CUIPolyString_Impl;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*!  CUIPolyString class.  This class is used to create and store
collections of polygons that have been mapped to font textures via 
the CUIFont class.

\see class CUIFont
\see class CUIFormattedPolyString

Used for: Text and UI.   */

// TRON: Tron's polystring is not NOLF2's. NOLF2's is a wrapper whose
// methods (in the engine) call a CUIPolyString_Impl; Tron's engine creates
// its own class (0x84 bytes, vtable 0x59182c in retail Lithtech.exe), and
// the game calls its 43 virtuals directly. Its text is a std::wstring (there
// is a char* SetText that widens), its positions and sizes are integers, and
// it has no GetClassName. The slots, and how each was named, are in
// progress/engine/CUIPolyString-vtable.csv; the names of what NOLF2 does not
// have are ours. The game never creates one itself (ILTFontManager does).

class CUIPolyString
{
	public:

		virtual ~CUIPolyString() {}

		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont) = 0;

		// RETAIL: 0x563060 (+0x8) assigns the string; 0x563120 (+0xc)
		// widens each char. VC6 lays overloads out in the reverse of their
		// declaration order: these are declared last slot first.
		virtual CUI_RESULTTYPE	SetText(const char* pBuffer) = 0;
		virtual CUI_RESULTTYPE	SetText(const std::wstring& sText) = 0;

		virtual CUI_RESULTTYPE	SetPosition(int32 x, int32 y) = 0;

		// TRON: sets a flag (true from the constructor) that rendering reads.
		virtual CUI_RESULTTYPE	TronUnknownVirtual5(bool bValue) = 0;

		virtual CUI_RESULTTYPE	SetColors(uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3) = 0;
		virtual CUI_RESULTTYPE	SetColor(uint32 argb) = 0;

		// TRON: the rectangle RenderClipped and the clipping flag use.
		virtual CUI_RESULTTYPE	SetClipRect(CUIRECT* pRect) = 0;

		// TRON: the character height in pixels; scales the width to it.
		virtual CUI_RESULTTYPE	SetCharScreenSize(uint32 height) = 0;

		virtual uint32			GetColor() = 0;
		virtual CUIFont*		GetFont() = 0;
		virtual const std::wstring&	GetText() = 0;
		virtual CUI_RESULTTYPE	GetRect(CUIRECT* pRect) = 0;
		virtual CUI_RESULTTYPE	GetPosition(int32* pX, int32* pY) = 0;
		virtual CUI_RESULTTYPE	GetDims(int32* pWidth, int32* pHeight) = 0;
		virtual int32			GetWidth() = 0;
		virtual int32			GetHeight() = 0;
		virtual int32			GetX() = 0;
		virtual int32			GetY() = 0;

		// TRON: the flag TronUnknownVirtual5 sets.
		virtual bool			TronUnknownVirtual20() = 0;

		virtual uint32			GetLength() = 0;
		virtual CUI_RESULTTYPE	GetClipRect(CUIRECT* pRect) = 0;
		virtual uint32			GetCharScreenHeight() = 0;
		virtual LT_POLYGT4*		GetPolys() = 0;

		// TRON: the drop shadow's polygons, and turning it on (the name is
		// ours: a second set of polygons, one per character).
		virtual LT_POLYGT4*		GetShadowPolys() = 0;
		virtual CUI_RESULTTYPE	SetDropShadow(bool bDropShadow) = 0;

		// TRON: whether Render clips to the clip rectangle.
		virtual CUI_RESULTTYPE	EnableClipping(bool bClip) = 0;

		virtual CUI_RESULTTYPE	Render(int32 start = 0, int32 end = -1) = 0;
		virtual CUI_RESULTTYPE	RenderClipped(CUIRECT* pClipRect,
											  int32 start = 0,
											  int32 end = -1) = 0;

		// TRON: returns true (0x482590), whatever it is given.
		virtual bool			TronUnknownVirtual30(void* pValue) = 0;

		// TRON: calls SetCharScreenSize (0x565c30); the game's text controls
		// call this one where NOLF2's call SetCharScreenHeight.
		virtual CUI_RESULTTYPE	SetCharScreenHeight(uint32 height) = 0;

	protected:

		// TRON: the engine's own: building the polygons, applying the font
		// and the colors (0x563710 .. 0x5645b0).
		virtual void			TronUnknownVirtual32() = 0;
		virtual void			TronUnknownVirtual33() = 0;
		virtual void			TronUnknownVirtual34() = 0;
		virtual void			TronUnknownVirtual35() = 0;
		virtual void			TronUnknownVirtual36() = 0;
		virtual void			TronUnknownVirtual37() = 0;
		virtual void			TronUnknownVirtual38() = 0;
		virtual void			TronUnknownVirtual39() = 0;
		virtual void			TronUnknownVirtual40() = 0;
		virtual void			TronUnknownVirtual41() = 0;
		virtual void			TronUnknownVirtual42() = 0;
};

#endif //__CUIPOLYSTRING_H__
