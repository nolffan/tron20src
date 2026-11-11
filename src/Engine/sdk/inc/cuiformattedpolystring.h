//-------------------------------------------------------------------
//
//   MODULE    : CUIFORMATTEDPOLYSTRING.H
//
//   PURPOSE   : defines the CUIFormattedPolyString bridge Class
//
//   CREATED   : 7/00 
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFORMATTEDPOLYSTRING_H__
#define __CUIFORMATTEDPOLYSTRING_H__


#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif


/*!  MAX_POLYSTRING_LINES determines how many lines can be displayed by a CUIFormattedPolyString.
decreasing this value will save memory (less pre-allocated formatting data).  If you try to create
a polystring whose lines exceed this value, you'll see an assertion failure--(in DEBUG mode) and your
string will be capped.
*/
#define		MAX_POLYSTRING_LINES	64

/*!  MAX_POLYSTRING_WORDS determines how many words can be displayed by a CUIFormattedPolyString.
decreasing this value will save memory (less pre-allocated formatting data).  If you try to create
a polystring whose words exceed this value, you'll see an assertion failure (in DEBUG mode) and your
string will be capped.
*/
#define		MAX_POLYSTRING_WORDS	128


/*!  CUIFormattedPolyString class.  This class is used to create and store
collections of polygons that have been mapped to font textures via 
the CUIFont class.  In addition to the functionality of the regular CUIPolyString,
the CUIFormattedPolyString adds support for newlines, word-wrap, and justification.

\see class CUIFont

Used for: Text and UI.   
*/
// TRON: Tron's formatted polystring (0x9c bytes, vtable 0x591a78 in retail
// Lithtech.exe) adds two virtuals to its CUIPolyString's: the wrap width
// (0x568750, member +0x88) and then the alignment (0x568780, +0x84; the one
// CreateFormattedPolyString sets) -- NOLF2 declares them the other way round.
// The game never creates one itself (ILTFontManager does).
class CUIFormattedPolyString : public CUIPolyString
{
	public:

		virtual CUI_RESULTTYPE	SetWrapWidth(uint32 wrap) = 0;
		virtual	CUI_RESULTTYPE	SetAlignmentH(CUI_ALIGNMENTTYPE halign) = 0;
};


#endif //__CUIFORMATTEDPOLYSTRING_H__
