// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIMgr.h
//
// PURPOSE : Shared header for the library of LTGUI controls
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_LTGUIMGR_H_)
#define _LTGUIMGR_H_


#include "lithtech.h"
#include "iltfontmanager.h"

//TO2 specific 
#include "..\..\shared\DebugNew.h"

#pragma warning( disable : 4786 )
#include <vector>

typedef std::vector<CUIPolyString*> PStringArray;
typedef std::vector<CUIFormattedPolyString*> FPStringArray;

// TRON: Tron's UI text is wide (std::wstring: ILTFontManager, CUIPolyString).
// PORT: this widening is ours, for the code that still makes its text narrow
// where retail's has it wide already; it widens each char as the engine's
// CUIPolyString::SetText(const char*) does (0x563120 in Lithtech.exe).
inline std::wstring LTGUIWideString(const char* pText)
{
	std::wstring sText;
	if (pText)
	{
		while (*pText)
			sText += (wchar_t)(uint8)*pText++;
	}
	return sText;
}

// TRON: retail widens inline, into a string of the caller's (CLTGUISlider::
// SetSliderPos, 0x100c0ac0: cleared, then each char appended); the name is
// ours.
inline void LTGUIWiden(std::wstring& sOut, const char* pText)
{
	sOut.erase(sOut.begin(), sOut.end());
	uint32 nLen = strlen(pText);
	for (uint32 i = 0; i < nLen; i++)
		sOut += (wchar_t)(uint8)pText[i];
}

// PORT: ours too: the other way, for narrow code reading wide text (each
// character's low byte).
inline std::string LTGUINarrowString(const std::wstring& sText)
{
	std::string sNarrow;
	for (std::wstring::const_iterator it = sText.begin(); it != sText.end(); ++it)
		sNarrow += (char)*it;
	return sNarrow;
}

//base classes
#include "..\ltguimgr\ltguicommandhandler.h"
#include "..\ltguimgr\ltguictrl.h"
typedef std::vector<CLTGUICtrl*> ControlArray;

//basic control classes
#include "..\ltguimgr\ltguitextitemctrl.h"
#include "..\ltguimgr\ltguibutton.h"
#include "..\ltguimgr\ltguicyclectrl.h"
#include "..\ltguimgr\ltguitoggle.h"
#include "..\ltguimgr\ltguislider.h"
#include "..\ltguimgr\ltguiframe.h"		// TRON: before the column control, which has one
#include "..\ltguimgr\ltguicolumnctrl.h"
#include "..\ltguimgr\ltguieditctrl.h"
#include "..\ltguimgr\ltguilargetext.h"

//container control classes
#include "..\ltguimgr\ltguiwindow.h"
#include "..\ltguimgr\ltguilistctrl.h"


//these are defined in the module that links
extern ILTDrawPrim*		g_pDrawPrim;
extern ILTFontManager*	g_pFontManager;
extern ILTTexInterface*	g_pTexInterface;

//utility functions for handling UV coordinates
#include "..\ltguimgr\ltquaduvutils.h"

#endif // _LTGUIMGR_H_
