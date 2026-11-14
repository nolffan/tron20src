// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIToggle.h
//
// PURPOSE : Text control which toggles between two values.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUITOGGLE_H_)
#define _LTGUITOGGLE_H_


#include "ltguimgr.h"

class CLTGUIToggle : public CLTGUICycleCtrl
{
public:
	CLTGUIToggle();
	virtual ~CLTGUIToggle();

	// Create the control
    // pText			- The initial text that is displayed for this control. Pass in 
	//						LTNULL if you do not want initial text. A copy of this text
	//						is made so the string may be discarded after making this call.
	// pFont			- The font to use for this string.
	// nFontSize		- The font size to use for this string.
	// nHeaderWidth	- The width to use for the header string
	// pbValue			  - Value to store the on/off status in when UpdateData is called
	// TRON: the header is a wide string, by value (RETAIL: 0x100bff70)
    LTBOOL           Create ( std::wstring sText, uint32 nHelpID, CUIFont *pFont, uint8 nFontSize,
                            uint16 nHeaderWidth, LTBOOL *pbValue=LTNULL);
	// PORT: ours, for the code that still makes its text narrow
    LTBOOL           Create ( const char *pText, uint32 nHelpID, CUIFont *pFont, uint8 nFontSize,
                            uint16 nHeaderWidth, LTBOOL *pbValue=LTNULL)
	{
		return Create(LTGUIWideString(pText), nHelpID, pFont, nFontSize, nHeaderWidth, pbValue);
	}

	// TRON: wide strings, by value (RETAIL: 0x100c01b0, 0x100c0260)
	void			SetOnString(std::wstring sOn);
	void			SetOffString(std::wstring sOff);
	// PORT: ours, for the code that still makes its text narrow
	void			SetOnString(const char *pStrOn)		{ SetOnString(LTGUIWideString(pStrOn)); }
	void			SetOffString(const char *pStrOff)	{ SetOffString(LTGUIWideString(pStrOff)); }

	// Update data
    void            UpdateData(LTBOOL bSaveAndValidate=LTTRUE);

	// Sets/gets the on/off status
    LTBOOL           IsOn()              { return m_bOn; }
    virtual void    SetOn(LTBOOL bOn);

	// Left and Right key presses
    virtual LTBOOL   OnLeft()            { SetOn(!IsOn()); return LTTRUE;}
    virtual LTBOOL   OnRight()           { SetOn(!IsOn()); return LTTRUE;}

protected:
    LTBOOL           m_bOn;
    LTBOOL           *m_pbValue;     // Value to store the on/off status in when UpdateData is called.

};

#endif // !defined(_LTGUITOGGLE_H_)