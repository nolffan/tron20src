// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUICycleCtrl.h
//
// PURPOSE : Control which can cycle through a set of strings based on 
//				user input.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUICYCLECTRL_H_)
#define _LTGUICYCLECTRL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CLTGUICycleCtrl : public CLTGUICtrl
{
public:
	CLTGUICycleCtrl();
	virtual ~CLTGUICycleCtrl();

	// Create the control
    // pText			- The initial text that is displayed for this control. Pass in 
	//						LTNULL if you do not want initial text. A copy of this text
	//						is made so the string may be discarded after making this call.
	// pFont			- The font to use for this string.
	// nFontSize		- The font size to use for this string.
	// nHeaderWidth	- The width to use for the header string
	// pnValue		- Pointer to the value which receives the selection index when UpdateData is called.
	// TRON: the header is a wide string, by value (RETAIL: 0x100c1c90)
    LTBOOL           Create ( std::wstring sText, uint32 nHelpID, CUIFont *pFont, uint8 nFontSize,
                            uint16 nHeaderWidth, uint8 *pnValue=LTNULL);
	// PORT: ours, for the code that still makes its text narrow
    LTBOOL           Create ( const char *pText, uint32 nHelpID, CUIFont *pFont, uint8 nFontSize,
                            uint16 nHeaderWidth, uint8 *pnValue=LTNULL)
	{
		return Create(LTGUIWideString(pText), nHelpID, pFont, nFontSize, nHeaderWidth, pnValue);
	}

	// Destroys the control
	virtual void	Destroy ( ); 

	// Update data
    virtual void    UpdateData(LTBOOL bSaveAndValidate);

	// Add more strings to the control.  These are cycled when OnLeft() and OnRight() are called
	static const uint8	kMaxNumStrings;
	static const uint8	kNoSelection;
	// TRON: a wide string, by value (RETAIL: 0x100c1e50)
	virtual void	AddString(std::wstring sText);
	// PORT: ours, for the code that still makes its text narrow
	void			AddString(const char *pText) { AddString(LTGUIWideString(pText)); }

	CUIFormattedPolyString*	GetString(uint8 nIndex);			// Return a string at a specific index
	void			RemoveString(uint8 nIndex);		// Remove a string at a specific index
	void			RemoveAll();					// Remove all strings

	// Return the number of strings
	uint8			GetNumStrings()						{ return m_stringArray.size(); }

	// Sets/Get the currently selected index
	uint8			GetSelIndex()						{ return m_nSelIndex; }
	virtual void	SetSelIndex(uint8 nIndex);
	//this function sets up a notification when the control's value changes
    virtual void    NotifyOnChange(uint32 nCommandID,CLTGUICommandHandler *pCommandHandler, uint32 nParam1 = 0, uint32 nParam2 = 0);

	// Render the control
	virtual void	Render ();

	// Width/Height calculations
	virtual uint16	GetWidth ( )						{ return m_nWidth; }
	virtual uint16	GetHeight ( )						{ return m_nHeight; }
	virtual void	CalculateSize();

    virtual LTBOOL   OnEnter() {return LTFALSE;}
    virtual LTBOOL   OnLeft();
    virtual LTBOOL   OnRight();
    virtual LTBOOL   OnLButtonUp(int x, int y) {return OnRight();}
    virtual LTBOOL   OnRButtonUp(int x, int y) {return OnLeft();}

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);

	// Set the font, pass in a NULL pointer to change only the font size
	//   pass in a font size of 0 to retain the current size.
    virtual LTBOOL	SetFont ( CUIFont *pFont, uint8 nFontSize);


protected:
	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling

    uint16		m_nWidth;              // The width of the control
    uint16		m_nHeight;             // The height of the control

	CUIFormattedPolyString* m_pHeaderText;
	uint16					m_nHeaderWidth;
	FPStringArray			m_stringArray;
	uint8					m_nSelIndex;
	uint8*					m_pnValue;

	CLTGUICommandHandler	*m_pCommandHandler;

};


#endif // !defined(_LTGUICYCLECTRL_H_)