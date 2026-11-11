//-------------------------------------------------------------------
//
//   MODULE    : ILTFONTMANAGER.H
//
//   PURPOSE   : defines the interface for The CUI FontManager Class
//
//   CREATED   : 7/00 
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------


//	This interface is available through the Interface Database!


#ifndef __ILTFONTMANAGER_H__
#define __ILTFONTMANAGER_H__


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif

#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif

#ifndef __CUIFORMATTEDPOLYSTRING_H__
#include "cuiformattedpolystring.h"
#endif


/*!  
The ILTFontManager interface is used to create and destroy monospace and 
proportional bitmap fonts, as well as vector (TrueType) fonts, CUIPolyStrings
and CUIFormattedPolyStrings.  

For bitmap fonts, an HTEXTURE is used to hold the font graphics, and must be
256x256 pixels or smaller.

TrueType font support is compiled into the engine by default.  If you do not
wish to use TrueType fonts, open up the file 
\b engine\engine\ui\src\freetype-2.0.1\include\freetype\config\ftoption.h
and \e #undef the macro FT_ENABLE_FREETYPE.  This will save you about 80K in
code/data size.

ILTFontmanager is available from the Interface Manager.  

\see class IBase
\see class ILTTexInterface
\see class CUIFont
\see class CUIPolyString
\see class CUIFormattedPolyString


Used for: Text and UI.
*/

// TRON: Tron's font manager (CUIFontManager, vtable 0x591924 in retail
// Lithtech.exe; its instance 0x5ecb50) is not NOLF2's: its fonts are
// CUIWideBitmapFonts loaded from a stream, its strings are wide
// (std::wstring), and it has three CreateFont overloads, each of one
// pointer, of which only the third (an ILTStream*) is implemented. The slots,
// and how each was named, are in progress/engine/ILTFontManager-vtable.csv.

class ILTStream;

class ILTFontManager : public IBase
{

	public:
			
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		// add ILTFontManager to the interface database	
		interface_version(ILTFontManager, 0);
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

	// RETAIL: 0x566200 (sets the initialized flag), 0x566220, 0x566670.
	virtual void Init() = 0;
	virtual void Term() = 0;
	virtual bool IsInitted() = 0;

	// TRON: VC6 lays a name's overloads out together in the reverse of their
	// declaration order (the game's calls: CreateFont(ILTStream*) at +0x18,
	// CUIPolyString::SetText(const std::wstring&) at +0x8), so each overload
	// set here is declared last slot first.

	// TRON: creates a CUIWideBitmapFont from a font file opened by the game
	// (RETAIL: 0x566230, which news the font and has it read the stream;
	// NULL if it did not load). The game releases the stream. Slot +0x18.
	virtual CUIFont* CreateFont(ILTStream* pStream) = 0;

	// TRON: two overloads that return NULL in Tron's engine (one body,
	// 0x4b4470, for both; slots +0x14, +0x10); what they took is not known
	// (the types are ours).
	virtual CUIFont* CreateFont(struct TronUnknownFontSource1* pSource) = 0;
	virtual CUIFont* CreateFont(struct TronUnknownFontSource0* pSource) = 0;

	virtual void DestroyFont(CUIFont* pFont) = 0;
	virtual uint32 GetNumFontsInUse() = 0;

	// TRON: stubs returning NULL in Tron's engine (one body, 0x5614a0); the
	// declarations (and so the overloads' order) are NOLF2's, which agree in
	// their argument counts.
	virtual HTEXTURE CharToTexture(uint8* pTTFData, uint32 dataSize, uint32 character, uint32 pointSize) = 0;
	virtual HTEXTURE StringToTexture(uint8* pTTFData, uint32 dataSize, char* pCharacters, uint32 pointSize) = 0;
	virtual HTEXTURE StringToTexture(uint8* pTTFData, uint32 dataSize, uint32* pCharacters, uint32 pointSize) = 0;

	// TRON: the text is a wide string (RETAIL: 0x566320 and 0x5663d0 set it
	// through CUIPolyString::SetText(const std::wstring&)); the position is
	// floats, converted to the string's integer SetPosition.
	virtual CUIPolyString* CreatePolyString(CUIFont* pFont,
											const std::wstring& sText,
											float x = 0.0,
											float y = 0.0) = 0;

	virtual CUIFormattedPolyString* CreateFormattedPolyString(CUIFont* pFont,
											const std::wstring& sText,
											float x = 0.0,
											float y = 0.0,
											CUI_ALIGNMENTTYPE alignment = CUI_HALIGN_LEFT) = 0;

	virtual void DestroyPolyString(CUIPolyString* pPolystr) = 0;

};

#endif //__ILTFONTMANAGER_H__
