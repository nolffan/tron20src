//-------------------------------------------------------------------
//
//   MODULE    : CUIFormattedPolyString_Impl.h
//
//   PURPOSE   : defines the CUIFormattedPolyString_Impl Class
//
//   CREATED   : 7/00
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------
//
// TRON: Tron 2.0's formatted polystring (no source survives; written from
// retail Lithtech.exe: constructor 0x568550, vtable 0x591a78, 0x9c bytes).
// It derives from the engine's polystring (its vtable is CUIPolyString's 43
// slots and the SDK CUIFormattedPolyString's two), breaks the text into
// lines at the wrap width (a list of each line's last character and its
// width), aligns each line, and takes "<color r g b>" tags out of the text:
// the text with its tags is kept (m_RawText), and ApplyColors reads the
// tags from it. The names of the members are ours.

#ifndef __CUIFORMATTEDPOLYSTRING_IMPL_H__
#define __CUIFORMATTEDPOLYSTRING_IMPL_H__


#ifndef __CUIFORMATTEDPOLYSTRING_H__
#include "cuiformattedpolystring.h"
#endif

#ifndef __CUIPOLYSTRING_IMPL_H__
#include "cuipolystring_impl.h"
#endif

#include <list>


// one line of a formatted polystring (8 bytes, a list node's value)
struct CUIFormatLine
{
	int32	m_LastChar;		// the index of the line's last character
	int32	m_Width;		// in pixels
};


// TRON: an engine class, not the SDK's CUIFormattedPolyString: its first 43
// virtuals are the polystring's and the next two (SetWrapWidth,
// SetAlignmentH) are where the SDK's CUIFormattedPolyString puts them, so
// the font manager hands it to the game as one (RETAIL: 0x5663d0 returns
// the object itself).
class CUIFormattedPolyString_Impl : public CUIPolyString_Impl
{
	public:

		CUIFormattedPolyString_Impl();
		virtual ~CUIFormattedPolyString_Impl();

		virtual CUI_RESULTTYPE	SetText(const char* pBuffer);
		virtual CUI_RESULTTYPE	SetText(const std::wstring& sText);

		// the SDK's CUIFormattedPolyString's (slots 43, 44)
		virtual CUI_RESULTTYPE	SetWrapWidth(uint32 wrap);
		virtual	CUI_RESULTTYPE	SetAlignmentH(CUI_ALIGNMENTTYPE halign);

		CUIFormattedPolyString*	GetInterface()	{ return (CUIFormattedPolyString*)(CUIPolyString*)this; }

	protected:

		virtual void			Layout();
		virtual void			ApplyColors(uint32 start, int32 count);
		virtual void			Format();
		virtual void			GetExtents();

		void					ApplyFormattedColors(uint32 start, int32 count);
		uint32					ParseColor(const wchar_t* pStr, uint32 color);

		// a line's offset for the alignment (RETAIL: inline, 0x568f46)
		int32					GetLineOffset(const CUIFormatLine& line)
		{
			switch (m_Alignment)
			{
				case CUI_HALIGN_RIGHT:	return m_WrapWidth - line.m_Width;
				case CUI_HALIGN_CENTER:	return (m_WrapWidth - line.m_Width) / 2;
				default:				return 0;
			}
		}

	protected:

		CUI_ALIGNMENTTYPE			m_Alignment;	// +0x84
		uint32						m_WrapWidth;	// +0x88: 0 for none
		std::list<CUIFormatLine>	m_Lines;		// +0x8c
		std::wstring				m_RawText;		// +0x90: the text with its tags
};


#endif //__CUIFORMATTEDPOLYSTRING_IMPL_H__
