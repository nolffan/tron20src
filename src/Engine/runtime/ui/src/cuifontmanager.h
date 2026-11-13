//-------------------------------------------------------------------
//
//   MODULE    : CUIFONTMANAGER.H
//
//   PURPOSE   : defines the CUIFontManager class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFONTMANAGER_H__
#define __CUIFONTMANAGER_H__


#ifndef __ILTFONTMANAGER_H__
#include "iltfontmanager.h"
#endif



// TRON: Tron 2.0's font manager (RETAIL: Lithtech.exe 0x5661e0..0x5664a5,
// vtable 0x591924, the instance 0x5ecb50): its fonts are CUIWideBitmapFonts
// read from a stream, and its polystrings the engine's wide ones.
class CUIFontManager : public ILTFontManager
{
	public:
	
		// interface database	
		declare_interface(CUIFontManager);

		CUIFontManager();
		~CUIFontManager();

		// housekeeping
		void	Init();
		void	Term();
		bool	IsInitted()		{ return m_bInitted; }
			
		// font creation functions (RETAIL: the first two are one stub,
		// 0x4b4470, returning NULL)
		CUIFont* CreateFont(ILTStream* pStream);
		CUIFont* CreateFont(struct TronUnknownFontSource1* pSource)	{ return NULL; }
		CUIFont* CreateFont(struct TronUnknownFontSource0* pSource)	{ return NULL; }

		// font destruction function
		void	 DestroyFont(CUIFont* pFont);

		// font info
		uint32	 GetNumFontsInUse();

		// RETAIL: stubs (one body, 0x5614a0)
		HTEXTURE	CharToTexture(uint8* pTTFData, uint32 dataSize, uint32 character, uint32 pointSize)		{ return NULL; }
		HTEXTURE	StringToTexture(uint8* pTTFData, uint32 dataSize, char* pCharacters, uint32 pointSize)	{ return NULL; }
		HTEXTURE	StringToTexture(uint8* pTTFData, uint32 dataSize, uint32* pCharacters, uint32 pointSize)	{ return NULL; }

		// Creating of PolyStrings

		virtual CUIPolyString*	CreatePolyString(CUIFont* pFont, 
											     const std::wstring& sText,
											     float x,
											     float y);

		virtual CUIFormattedPolyString*	CreateFormattedPolyString(CUIFont* pFont, 
											     const std::wstring& sText,
											     float x,
											     float y,
												 CUI_ALIGNMENTTYPE alignment);

		// PolyString destruction	
		
		virtual void DestroyPolyString(CUIPolyString* pPolystr);


	private:

		bool	m_bInitted;			// +0x4
		uint32	m_NumValidFonts;	// +0x8

};


#endif //__CUIFONTMANAGER_H__
