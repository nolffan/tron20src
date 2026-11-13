//-------------------------------------------------------------------
//
//   MODULE    : CUIFONTMANAGER.CPP
//
//   PURPOSE   : implements the CUIFontManager Interface
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFONTMANAGER_H__
#include "cuifontmanager.h"
#endif

#ifndef __CUIWIDEBITMAPFONT_H__
#include "cuiwidebitmapfont.h"
#endif

#ifndef __CUIPOLYSTRING_IMPL_H__
#include "cuipolystring_impl.h"
#endif

#ifndef __CUIFORMATTEDPOLYSTRING_IMPL_H__
#include "cuiformattedpolystring_impl.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif


// interface database (RETAIL: the registration 0x566180)
define_interface(CUIFontManager, ILTFontManager); 


//	--------------------------------------------------------------------------
// RETAIL: 0x5661e0
CUIFontManager::CUIFontManager()
{
	m_bInitted = false;	
}


//	--------------------------------------------------------------------------
// RETAIL: 0x5661f0
CUIFontManager::~CUIFontManager()
{
	if (m_bInitted) {
		CUI_ERR("ILTFontManager was destroyed without calling ::Term()!\n");
	}
}


//	--------------------------------------------------------------------------
// RETAIL: 0x566200
void CUIFontManager::Init()
{
	if (m_bInitted) return;

	m_NumValidFonts = 0;

	m_bInitted = true;
}


//	--------------------------------------------------------------------------
// RETAIL: 0x566220
void CUIFontManager::Term()
{
	if (!m_bInitted) return;

	if (m_NumValidFonts) {
		CUI_ERR("ILTFontManager was terminated, but not all fonts were destroyed!\n");
	}

	m_bInitted = false;
}


//	--------------------------------------------------------------------------
// RETAIL: 0x566230: a wide bitmap font from a font file the game opened
CUIFont* CUIFontManager::CreateFont(ILTStream* pStream)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	CUIWideBitmapFont* pFont;
	LT_MEM_TRACK_ALLOC(pFont = new CUIWideBitmapFont,LT_MEM_TYPE_UI);

	if (pFont) {
		pFont->Load(pStream);

		if (!pFont->m_bLoaded) {
			delete pFont;
			pFont = NULL;
		}
		else {
			m_NumValidFonts++;
		}
	}

	return pFont;
}


//	--------------------------------------------------------------------------
// RETAIL: 0x5662e0: only a loaded font is deleted
void CUIFontManager::DestroyFont(CUIFont* font)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return;
	}

	if (font && ((CUIFont_Impl*)font)->m_bLoaded) {
		m_NumValidFonts--;
		delete font;
	}
}


//	--------------------------------------------------------------------------
// RETAIL: 0x566310
uint32 CUIFontManager::GetNumFontsInUse()
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return 0;
	}

	return m_NumValidFonts; 
}


//	--------------------------------------------------------------------------
// RETAIL: 0x566320
CUIPolyString* CUIFontManager::CreatePolyString(CUIFont* font, 
										  const std::wstring& sText,
										  float x,
										  float y)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	CUIPolyString* p;
	LT_MEM_TRACK_ALLOC(p = new CUIPolyString_Impl,LT_MEM_TYPE_UI);

	p->SetFont(font);
	p->SetPosition((int32)x, (int32)y);
	p->SetText(sText);

	return p;
}


//	--------------------------------------------------------------------------
// RETAIL: 0x5663d0
CUIFormattedPolyString*	CUIFontManager::CreateFormattedPolyString(CUIFont* font, 
											     const std::wstring& sText,
											     float x,
											     float y,
												 CUI_ALIGNMENTTYPE alignment)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	CUIFormattedPolyString_Impl* p;
	LT_MEM_TRACK_ALLOC(p = new CUIFormattedPolyString_Impl,LT_MEM_TYPE_UI);

	p->SetFont(font);
	p->SetAlignmentH(alignment);
	p->SetPosition((int32)x, (int32)y);
	p->SetText(sText);

	return p->GetInterface();
}


//	--------------------------------------------------------------------------
// RETAIL: 0x566490
void CUIFontManager::DestroyPolyString(CUIPolyString* polystring)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return;
	}

	if (polystring) delete polystring;
}
