//-------------------------------------------------------------------
//
//   MODULE    : CUIFONT_IMPL.H
//
//   PURPOSE   : defines the CUIFont_Impl common font base class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------

#ifndef __CUIFONT_IMPL_H__
#define __CUIFONT_IMPL_H__

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif

class CUIPolyString;

// TRON: Tron 2.0's font base, under NOLF2's name. RETAIL: Lithtech.exe
// constructor 0x565d40, destructor 0x565d80, vtable 0x5918d8 (every slot
// _purecall but the destructor and slot 12): far smaller than NOLF2's
// CUIFont_Impl -- the font data are the wide bitmap font's
// (cuiwidebitmapfont.h). The only font left is CUIWideBitmapFont; NOLF2's
// bitmap and vector fonts are gone.
//
// RETAIL: not derived from the SDK's CUIFont: the same slots, declared
// again (its destructor stores only its own vtable, 0x565d80), and handed
// to the game as a CUIFont.
class CUIFont_Impl
{
	// the font manager reads m_bLoaded (0x5662ef)
	friend class CUIFontManager;

	public:

		CUIFont_Impl();
		virtual ~CUIFont_Impl();

		// CUIFont's slots 1..11 (cuifont.h)
		virtual const char* GetClassName() = 0;
		virtual CUI_RESULTTYPE	TronUnknownVirtual2(int8 nValue) = 0;
		virtual CUI_RESULTTYPE	TronUnknownVirtual3(int8 nValue) = 0;
		virtual uint32			GetDefCharScreenHeight() = 0;
		virtual int32			GetLineSpacing() = 0;
		virtual int32			GetCharSpacing() = 0;
		virtual uint32			GetSpaceWidth() = 0;
		virtual HTEXTURE		GetTexture(uint8 nPage) = 0;
		virtual uint16			GetDefaultChar() = 0;
		virtual bool			IsRemapped() = 0;
		virtual uint16**		GetRemapTables() = 0;

		// slot 12 (RETAIL: 0x566120): whether the font file said the font
		// is valid
		virtual bool	TronUnknownVirtual12()	{ return m_Valid; }

		// CUIFont's slots 13..17
		virtual void*			GetCharTables() = 0;
		virtual uint32			GetTextWidth(const std::wstring& sText) = 0;
		virtual bool			HasChar(uint16 nChar) = 0;

	protected:

		virtual bool			Load(ILTStream* pStream) = 0;
		virtual void			Term() = 0;

		// the polystring NOLF2's fonts drew strings with: deleted with the
		// last font (Tron's fonts no longer make one; the destructor keeps
		// the clean-up)
		static CUIPolyString*	sm_pDrawStr;

		uint32		m_xTronUnknown4;	// RETAIL: +0x4, zeroed by the constructor
		bool		m_bLoaded;			// RETAIL: +0x8, set by Load, cleared by Term
		bool		m_Valid;			// RETAIL: +0x9, read from the font file
};

#endif //__CUIFONT_IMPL_H__
