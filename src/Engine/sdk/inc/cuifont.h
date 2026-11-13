//-------------------------------------------------------------------
//
//   MODULE    : CUIFONT.H
//
//   PURPOSE   : defines the inteface for the CUIFont Class
//
//   CREATED   : 7/00 
//
//-------------------------------------------------------------------


#ifndef __CUIFONT_H__
#define __CUIFONT_H__

 
#ifndef __CUI_H__
#include "cui.h"
#endif


/*! 
FX bitmasks that can be applied to CUIFonts.
*/
typedef enum {
/*! 
Turns \b Italics on or off.
*/
	CUI_FONT_ITALIC		= 1,
/*! 
Turns \b Bold on or off.
*/
	CUI_FONT_BOLD		= 2,

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	// currently unsupported
	CUI_FONT_UNDERLINE	= 4,
	CUI_FONT_STRIKEOUT	= 8,
	CUI_FONT_DROPSHADOW = 16
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

}	CUI_FONTFXTYPE;


#ifndef DOXYGEN_SHOULD_SKIP_THIS
// forward decl needed to avoid circular include
class CUIPolyString;
class ILTStream;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

// TRON: Tron's text is wide.
#include <string>


/*! 
The maximum number of charcters allowed in a font.
\b Note:  For efficiency reasons, the CUI font system is largely based on a uint8, 
and simply changing this \e #define will not have the effect of making fonts capable
of holding more than 255 elements.
*/
#define CUI_MAX_BITMAPFONT_CHARS	255


/*!
The CUIFont class is used to store font resources.  Fonts can be monospace or
proportional, and can be created from HTEXTURES or TrueType resources.

\see interface ILTFontManager
\see class CUIPolyString

Used for: Text and UI.   */

// TRON: Tron's fonts are CUIWideBitmapFonts (0x1124 bytes, vtable 0x591a30 in
// retail Lithtech.exe), which read their character tables from a font file
// and their pages from "\interface\fonts\%s_%03i.dtx". Their 18 virtuals
// are not NOLF2's; the slots, and how each was named, are in
// progress/engine/CUIFont-vtable.csv. Most are not yet identified, and the
// game calls only GetDefCharScreenHeight (whose name is ours).
class CUIFont
{
	public:

		virtual ~CUIFont() {};

		// RETAIL: 0x568440 returns "CUIWideBitmapFont".
		virtual const char* GetClassName() = 0;

		// TRON: two setters with one body (0x568450: the byte at +0x1b).
		virtual CUI_RESULTTYPE	TronUnknownVirtual2(int8 nValue) = 0;
		virtual CUI_RESULTTYPE	TronUnknownVirtual3(int8 nValue) = 0;

		// TRON: the font's height in pixels (from its file).
		virtual uint32			GetDefCharScreenHeight() = 0;

		virtual int8			TronUnknownVirtual5() = 0;
		virtual int8			TronUnknownVirtual6() = 0;
		virtual uint8			TronUnknownVirtual7() = 0;

		// TRON: the texture of one of the font's pages.
		virtual HTEXTURE		GetTexture(uint8 nPage) = 0;

		virtual uint16			TronUnknownVirtual9() = 0;
		virtual bool			TronUnknownVirtual10() = 0;
		virtual uint16**		TronUnknownVirtual11() = 0;
		virtual bool			TronUnknownVirtual12() = 0;
		virtual void*			TronUnknownVirtual13() = 0;
		virtual uint32			TronUnknownVirtual14(const std::wstring& sText) = 0;
		virtual bool			TronUnknownVirtual15(uint16 nChar) = 0;	// RETAIL: 0x567f80, whether the font has the character

	protected:

		// TRON: reads the font from a stream (0x567fe0: ILTFontManager's
		// CreateFont), and frees its tables (0x5681d0).
		virtual bool			Load(ILTStream* pStream) = 0;
		virtual void			Term() = 0;
};



#endif //__CUIFONT_H__
