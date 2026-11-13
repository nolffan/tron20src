//-------------------------------------------------------------------
//
//   MODULE    : CUIWIDEBITMAPFONT.H
//
//   PURPOSE   : defines the CUIWideBitmapFont font class
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------
//
// TRON: Tron 2.0's one font class (no source survives; written from
// retail Lithtech.exe: constructor 0x567d70, vtable 0x591a30, 0x1124
// bytes). A font is a file the game opens (ILTFontManager::CreateFont(
// ILTStream*)): a header read whole, then the character remap tables and
// the character entries, with the font's texture pages named after it
// (\interface\fonts\<name>_<page>.dtx). Characters are 16 bits: each table
// is indexed by the high byte, then the low one. The names of the members
// and of CUIWideFontChar are ours.

#ifndef __CUIWIDEBITMAPFONT_H__
#define __CUIWIDEBITMAPFONT_H__


#ifndef __CUIFONT_IMPL_H__
#include "cuifont_impl.h"
#endif


// one character of a wide font (8 bytes in the file)
struct CUIWideFontChar
{
	uint8		m_Page;				// RETAIL: the texture page (the polystrings' ApplyFont, 0x563f40)
	uint8		m_xTronUnknown1;
	int16		m_Width;			// RETAIL: the advance GetTextWidth adds
	int16		m_U;				// the character's place on its page, in texels
	int16		m_V;
};


class CUIWideBitmapFont : public CUIFont_Impl
{
	// the font manager loads fonts (ILTFontManager::CreateFont, 0x566230)
	friend class CUIFontManager;

	public:

		CUIWideBitmapFont();
		virtual ~CUIWideBitmapFont();

		virtual const char* GetClassName() { return "CUIWideBitmapFont"; }

		virtual CUI_RESULTTYPE	TronUnknownVirtual2(int8 nValue)	{ m_LineSpacing = nValue; return (CUI_RESULTTYPE)0; }
		virtual CUI_RESULTTYPE	TronUnknownVirtual3(int8 nValue)	{ m_LineSpacing = nValue; return (CUI_RESULTTYPE)0; }
		virtual uint32			GetDefCharScreenHeight()	{ return m_CharHeight; }
		virtual int32			GetLineSpacing()		{ return m_LineSpacing; }
		virtual int32			GetCharSpacing()		{ return m_CharSpacing; }
		virtual uint32			GetSpaceWidth()		{ return m_SpaceWidth; }
		virtual HTEXTURE		GetTexture(uint8 nPage)		{ return m_hPages[nPage]; }
		virtual uint16			GetDefaultChar()		{ return m_DefaultChar; }
		virtual bool			IsRemapped()		{ return m_bRemap == 1; }
		virtual uint16**		GetRemapTables()		{ return m_pRemap; }
		virtual void*			GetCharTables()		{ return m_pChars; }
		virtual uint32			GetTextWidth(const std::wstring& sText);
		virtual bool			HasChar(uint16 nChar);

	protected:

		virtual bool			Load(ILTStream* pStream);
		virtual void			Term();

	protected:

		// RETAIL: +0xc..+0x1124, read from the file in one piece (the
		// pointers in it are the file's, replaced as the tables are read)
		uint8				m_xTronUnknownC;	// 0: the space width is m_DefaultSpaceWidth
		uint8				m_xTronUnknownD;
		uint8				m_bRemap;			// the file has the remap tables
		uint8				m_SpaceWidth;
		uint32				m_xTronUnknown10;
		uint8				m_DefaultSpaceWidth;
		uint8				m_xTronUnknown15;
		uint16				m_CharHeight;
		uint16				m_DefaultChar;		// drawn for a character the font lacks
		int8				m_CharSpacing;
		int8				m_LineSpacing;
		uint32				m_xTronUnknown1C;
		CUIWideFontChar**	m_pChars[256];		// by high byte, then low byte
		uint16*				m_pRemap[256];		// by high byte: the character drawn instead
		uint32				m_bPage[256];		// the page's texture was loaded
		HTEXTURE			m_hPages[256];
		char				m_Name[256];		// the font's file name stem
		uint8				m_nPages;
};


#endif //__CUIWIDEBITMAPFONT_H__
