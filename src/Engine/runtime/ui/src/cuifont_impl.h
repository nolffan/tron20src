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
class CUIFont_Impl : public CUIFont
{
	public:

		CUIFont_Impl();
		virtual ~CUIFont_Impl();

		// slot 12 (RETAIL: 0x566120): whether the font file said the font
		// is valid
		virtual bool	TronUnknownVirtual12()	{ return m_Valid; }

	protected:

		// the polystring NOLF2's fonts drew strings with: deleted with the
		// last font (Tron's fonts no longer make one; the destructor keeps
		// the clean-up)
		static CUIPolyString*	sm_pDrawStr;

		uint32		m_xTronUnknown4;	// RETAIL: +0x4, zeroed by the constructor
		bool		m_bLoaded;			// RETAIL: +0x8, set by Load, cleared by Term
		bool		m_Valid;			// RETAIL: +0x9, read from the font file
};

#endif //__CUIFONT_IMPL_H__
