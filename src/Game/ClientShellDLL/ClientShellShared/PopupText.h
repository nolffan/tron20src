// PopupText.h: interface for the CPopupText class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _POPUPTEXT_INCLUDED_
#define _POPUPTEXT_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGuiMgr.h"

class CPopupText
{
public:
	CPopupText();
	virtual ~CPopupText();

	void	Init(bool bUsePopupState=true);
	void	Term();


	// RETAIL: Tron's (retail 0x1007f630, emitted out of line): a popup
	// with a string by id and an image, the text empty
	void	Show(uint32 nTextId, uint8 nPopupId, const char* szImage) { DisplayPopup( nPopupId, L"", nTextId, szImage ); }
	// PORT: NOLF2's forms, for NOLF2's callers (CInterfaceMgr::ShowPopup)
	void	Show(uint32 nTextId, uint8 nPopupId) { DisplayPopup( nPopupId, L"", nTextId, LTNULL ); }
	void	Show(char* pText, uint8 nPopupId) { DisplayPopup( nPopupId, LTGUIWideString(pText), 0, LTNULL ); }
	void	Update();
	void	Close();

#ifndef __PSX2
	LTBOOL OnKeyDown(int key, int rep);
#else
	LTBOOL OnInterfaceCommand(int command);
#endif	

	void	Draw();

	LTBOOL  IsVisible() { return m_bVisible;}

private:

	// RETAIL: Tron's (retail 0x1014edf0): the text is wide, used when there
	// is no string id; the image is shown in a second frame
	void	DisplayPopup( uint8 nPopupId, std::wstring sText, uint32 nTextId, const char* szImage );
	LTBOOL	m_bVisible;
	bool	m_bUsePopupState;

	// RETAIL: Tron's (retail 0x1014eb00, 0xad0 bytes): the frames' own
	// textures, a second frame and when to draw it (CPopupText::Draw,
	// 0x1014edb0). The names are ours
	HTEXTURE			m_hTronUnknownFrame2Tex;	// +0xc
	HTEXTURE			m_hFrameTex;				// +0x10

	CLTGUIFrame			m_Frame;
	CLTGUITextCtrl		m_Text;
	CLTGUIFrame			m_TronUnknownFrame2;		// +0x5a4

	bool				m_bTronUnknownDrawFrame2;	// +0xacc
	bool				m_bWeaponsEnabled;			// +0xacd
/*
	void	ClearSurfaces();
	void	ShowText(int nStringId);
	void	CreateScaleFX(char *szFXName);

	LTRect			m_rcRect;
	LTIntPt			m_pos;
	uint32			m_dwWidth;
	uint32			m_dwHeight;

	int				m_nLineHeight;
#ifndef __PSX2 //*@
	int				m_nKey;

	HSURFACE		m_hForeSurf;
	CLTGUIFont		*m_pForeFont;
#else
	*/
};

#endif // _POPUPTEXT_INCLUDED_