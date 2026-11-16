// ----------------------------------------------------------------------- //
//
// MODULE  : ToolTip.h
//
// PURPOSE : A tool tip: text in a frame near the cursor, with an icon
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (retail CShell.dll 0x1014e230..0x1014eaf9, sizeof 0x5cc:
// CScreenSubroutines' +0x1a94, constructed at 0x1007d4ba). It has no RTTI
// and no virtuals: the class and member names are ours. Show sets
// CScreenSpriteMgr's tool tip to itself, and CScreenSpriteMgr::Update
// draws it under the cursor's layers.

#ifndef __TOOLTIP_H__
#define __TOOLTIP_H__

#include "ltguiframe.h"

class CLightLine;
class CUIFormattedPolyString;

class CToolTip
{
public:
	CToolTip();		// RETAIL: 0x1014e230
	~CToolTip();	// RETAIL: 0x1014e2c0

	void	SetBackColor(uint32 nColor);	// RETAIL: 0x1014e310
	void	SetTextColor(uint32 nColor);	// RETAIL: 0x1014e340
	void	Hide();							// RETAIL: 0x1014e360
	void	Draw();							// RETAIL: 0x1014e3e0
	void	Show(int x, int y, std::wstring sText, HTEXTURE hIcon);	// RETAIL: 0x1014e5c0

	bool	IsVisible() const { return m_bVisible; }

private:
	bool					m_bVisible;		// +0x0
	float					m_fDelay;		// +0x4 the time before it shows
	float					m_fFadeTime;	// +0x8 the time it fades in
	float					m_fStartTime;	// +0xc
	int						m_nWidth;		// +0x10
	int						m_nFontSize;	// +0x14
	int						m_nOffset;		// +0x18 from the cursor
	LTRect					m_rcBorder;		// +0x1c
	uint32					m_nTextColor;	// +0x2c
	uint32					m_nBackColor;	// +0x30
	HTEXTURE				m_hFrame;		// +0x34
	HTEXTURE				m_hIcon;		// +0x38
	LT_POLYFT4				m_IconPoly;		// +0x3c
	CUIFormattedPolyString*	m_pText;		// +0x90
	CLTGUIFrame				m_Frame;		// +0x94

	enum { kNumLines = 4 };
	CLightLine*				m_pLines[kNumLines];	// +0x5bc the frame's edges
};

#endif // __TOOLTIP_H__
