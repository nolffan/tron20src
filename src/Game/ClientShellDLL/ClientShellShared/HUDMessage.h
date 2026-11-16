//-------------------------------------------------------------------------
//
// MODULE  : HUDMessage.h
//
// PURPOSE : Base class for HUD text display
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __HUD_MESSAGE_H__
#define __HUD_MESSAGE_H__


#include "LTGUIMgr.h"
#include "LTPoly.h"

enum MsgJustification
{
	kMsgLeft,
	kMsgCenter,
	kMsgRight,
};

typedef struct MsgCreate_t
{
	MsgCreate_t() 
	{
		pFont = LTNULL;
		nFontSize = 0;
		nTextColor = 0;
		hImage = LTNULL;
		nImageSize = 0;
		eJustify = kMsgLeft;
		fDuration = 0.0f;
		fFadeDur  = 0.0f;
		fDropShadow = 0.0f;
		nWidth	  = 640;

	};

	MsgCreate_t &operator=(const MsgCreate_t &cOther) {
		sString = cOther.sString;
		pFont = cOther.pFont;
		nFontSize = cOther.nFontSize;
		nTextColor = cOther.nTextColor;
		hImage = cOther.hImage;
		nImageSize = cOther.nImageSize;
		eJustify = cOther.eJustify;
		fDuration = cOther.fDuration;
		fFadeDur  = cOther.fFadeDur;
		fDropShadow = cOther.fDropShadow;
		nWidth	  = cOther.nWidth;
		return *this;
	}


	std::string			sString;
	CUIFont*			pFont;
	uint8				nFontSize;
	uint32				nTextColor;
	HTEXTURE			hImage;
	uint8				nImageSize;
	MsgJustification	eJustify;
	float 				fDuration;
	float 				fFadeDur;

	// RETAIL: Tron's MsgCreate is 0x34 bytes: NOLF2's bool bDropShadow is
	// gone and a float at 0x2c takes its place before nWidth (the queue's
	// constructor 0x1015e9b0 clears it; its copy in CHUDMessageQueue::Init
	// 0x1015ebd0; CHUDMessage::Create 0x101880f0 makes a second polystring
	// when it is above 0). The name is ours.
	float				fDropShadow;
	uint16				nWidth;

} MsgCreate;



class CHUDMessage : public CLTGUICtrl
{
public:
	CHUDMessage();
	virtual ~CHUDMessage() {Destroy();}

	LTBOOL	Create(MsgCreate &mc);
	virtual void	Destroy();

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);

	// Render the control
	virtual void	Render (LTBOOL bForceVisible);
	virtual void	Render ();

	void	SetAlpha(float  fAlpha);

	virtual void Update();

	virtual uint16	GetHeight() {return m_nHeight;}
	virtual uint16	GetWidth() {return m_nWidth;}

protected:
	void	SetRenderState();
	void	InitPoly();
	void	ScalePoly();
	void	JustifyPoly();
	float	GetLifetime();

	// RETAIL: Tron's split text: a second copy of the text that comes in
	// over the delay, its letters scrambled (0x10188870), and moves apart
	// from the first as the message fades (0x10188980). The names are ours
	void	ScrambleSplitText();
	void	UpdateSplitText();

protected:
	// RETAIL: an LT_POLYGT4 (cleared once, constructor 0x10188065)
	LT_POLYGT4	m_Poly;
	HTEXTURE	m_hImage;

    uint16		m_nFixedWidth;         // The width of the control
    uint16		m_nWidth;              // The width of the control
    uint16		m_nHeight;             // The width of the control

    LTIntPt		m_imageSize;
	uint8		m_nBaseImageSize;

	CUIFormattedPolyString* m_pText;
	// RETAIL: Tron's (+0xc0): the split text (CHUDMessage::Create makes it
	// when MsgCreate's split delay is above 0, 0x101880f0)
	CUIFormattedPolyString* m_pSplitText;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling
	uint32		m_nTextColor;

	float 		m_fInitTime;
	float 		m_fDuration;
	float 		m_fFadeDur;
	// RETAIL: Tron's (+0xdc, +0xe0): the split text's delay, and when its
	// letters were last scrambled. The names are ours
	float		m_fSplitDelay;
	float		m_fScrambleTime;

	float 		m_fImageGap;

	float		m_fAlpha;

	// RETAIL: Tron's has no drop shadow flag (it always sets the text's
	// drop shadow, 0x10188206)

	MsgJustification	m_eJustify;

	// RETAIL: Tron's (+0xf0): where SetScale puts the text (0x10188d1b;
	// the constructor leaves it unset). The names are ours
	int			m_nTextX;
	int			m_nTextY;
};


#endif