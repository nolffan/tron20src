//-------------------------------------------------------------------
//
//   MODULE    : CUIFONT_IMPL.CPP
//
//   PURPOSE   : implements the CUIFont Common Font Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __CUIFONT_IMPL_H__
#include "cuifont_impl.h"
#endif

#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif

#ifndef __ILTFONTMANAGER_H__
#include "iltfontmanager.h"
#endif


// interface database
static ILTFontManager *pFontManager = NULL;
define_holder(ILTFontManager, pFontManager);


// initialization of static data members
CUIPolyString* CUIFont_Impl::sm_pDrawStr = NULL;


//	--------------------------------------------------------------------------
// RETAIL: Lithtech.exe 0x565d40
CUIFont_Impl::CUIFont_Impl()
{
	m_xTronUnknown4	= 0;
	m_Valid			= false;
}


//	--------------------------------------------------------------------------
// RETAIL: Lithtech.exe 0x565d80
CUIFont_Impl::~CUIFont_Impl()
{
	// the font classes free their own resources

	if (pFontManager->GetNumFontsInUse() == 0) {
		// I am the last valid font, so clean up sm_pDrawStr
		if (sm_pDrawStr) {
			delete sm_pDrawStr;
			sm_pDrawStr = NULL;
		}
	}
}
