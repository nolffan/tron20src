// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceResMgr.h
//
// PURPOSE : Manager for resources associated with the interface
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_INTERFACERESMGR_H_)
#define _INTERFACERESMGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "InterfaceSurfMgr.h"
#include "iltfontmanager.h"

class CGameClientShell;
class CInterfaceResMgr;
extern CInterfaceResMgr* g_pInterfaceResMgr;

typedef std::vector<CUIFont*> FontArray;
typedef std::set<HTEXTURE> TextureSet;

const int kMaxSoundFilename = 64;

class CInterfaceResMgr
{
public:
	CInterfaceResMgr();
	virtual ~CInterfaceResMgr();

    LTBOOL              Init();
	void				Term();
	HSURFACE			GetSharedSurface(char *lpszPath)	{ return m_InterfaceSurfMgr.GetSurface(lpszPath); }
	void				FreeSharedSurface(char *lpszPath)	{ m_InterfaceSurfMgr.FreeSurface(lpszPath); }
	void				FreeSharedSurface(HSURFACE hSurf)	{ m_InterfaceSurfMgr.FreeSurface(hSurf); }

	HTEXTURE			GetTexture(const char *szTexName);

	CUIFont*			GetFont(uint32 nIndex);

	// TRON: the font (after the first) whose height is nearest nHeight
	// (RETAIL: 0x100c57a0; the name is ours)
	CUIFont*			GetFontForHeight(int nHeight);

	// RETAIL: Tron has no software cursor (no GetSurfaceCursor, no
	// "interface\\cursor0.pcx" in retail CShell.dll)

	const char			*GetSoundSelect();
	const char			*GetSoundChange();
	const char			*GetSoundPageChange();
	const char			*GetSoundUnselectable();
	const char			*GetSoundArrowUp();
	const char			*GetSoundArrowDown();
	const char			*GetSoundArrowLeft();
	const char			*GetSoundArrowRight();
	const char			*GetObjectiveAddedSound();
	const char			*GetObjectiveRemovedSound();
	const char			*GetObjectiveCompletedSound();

	void				ScreenDimsChanged();


    LTBOOL              IsEnglish()                         { return m_bEnglish; }

	void				DrawScreen();


	void				ConvertScreenRect(LTRect &rect);
	void				ConvertScreenPos(LTIntPt &pos) {ConvertScreenPos(pos.x,pos.y);}
	void				ConvertScreenPos(int &x, int &y);
    LTFLOAT             GetXRatio()                         {return m_fXRatio;}
    LTFLOAT             GetYRatio()                         {return m_fYRatio;}
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: the interface's 640x480 page is scaled the same both
	// ways (GetXRatio() == GetYRatio(): the smaller of width/640 and
	// height/480) and centred on the screen. These are the page's left and
	// top edges on the screen (0 at 4:3). Every place that puts the page on
	// the screen adds them.
	int					GetXOffset()						{return m_nXOffset;}
	int					GetYOffset()						{return m_nYOffset;}
	// A point of the page on the screen
	float				PageX(float x)						{return x * m_fXRatio + (float)m_nXOffset;}
	float				PageY(float y)						{return y * m_fYRatio + (float)m_nYOffset;}
	// The HUD keeps to the screen's edges: an item in the left (top) third
	// of the page stays at the left (top) edge, one in the right (bottom)
	// third moves with the right (bottom) edge, and one in the middle third
	// stays centred. The offset for an item at nBaseX (nBaseY) on the page:
	int					GetHUDXOffset(int nBaseX)
	{
		if (nBaseX < 640/3) return 0;
		if (nBaseX >= 2*640/3) return 2*m_nXOffset;
		return m_nXOffset;
	}
	int					GetHUDYOffset(int nBaseY)
	{
		if (nBaseY < 480/3) return 0;
		if (nBaseY >= 2*480/3) return 2*m_nYOffset;
		return m_nYOffset;
	}
	// A point of a HUD item on the screen (the item's anchor on the page
	// selects the offset)
	float				HUDX(float x, int nAnchorX)			{return x * m_fXRatio + (float)GetHUDXOffset(nAnchorX);}
	float				HUDY(float y, int nAnchorY)			{return y * m_fYRatio + (float)GetHUDYOffset(nAnchorY);}
	// The same offset in page units, for items whose base position is
	// scaled later by code that does not add an offset
	int					GetHUDBaseShiftX(int nBaseX)
	{
		if (m_fXRatio <= 0.0f) return 0;
		return (int)((float)GetHUDXOffset(nBaseX) / m_fXRatio + 0.5f);
	}
	int					GetHUDBaseShiftY(int nBaseY)
	{
		if (m_fYRatio <= 0.0f) return 0;
		return (int)((float)GetHUDYOffset(nBaseY) / m_fYRatio + 0.5f);
	}
#endif

    uint32              GetScreenWidth();
    uint32              GetScreenHeight();

	void				DrawMessage(int nMessageId, uint8 nFontSize = 0);
	// TRON: the text is a wide string, by value (RETAIL: 0x100c5150)
	void				DrawMessage(std::wstring sString, uint8 nFontSize = 0);


	//call Setup() before entering a 2-d state (screen)
	//call Clean() before returning to the game
    LTBOOL               Setup();
	void				Clean();

protected:
	// More initialization

    LTBOOL              InitFonts();
	// TRON: Tron's fonts are font files the engine reads (RETAIL: 0x100c5830)
	CUIFont*			CreateFont(char const* pszFontFile, uint8 ptSize);

protected:
    LTBOOL				m_bEnglish;             // True if the resource file has English as the specified language

	TextureSet			m_TextureSet;			// list of used textures

	FontArray			m_FontArray;

	// RETAIL: no m_hSurfCursor in Tron: its constructor (CShell.dll
	// 0x100c4a40) has the yes/no key codes at +0x20/+0x24 and m_fXRatio at +0x44

	int					m_nYesVKeyCode;			// The virtual key code for "yes" responses
	int					m_nNoVKeyCode;			// The virtual key code for "no" responses

	CInterfaceSurfMgr	m_InterfaceSurfMgr;	// Used to share title graphics

    LTFLOAT              m_fXRatio;
    LTFLOAT              m_fYRatio;
    uint32              m_dwScreenWidth;
    uint32              m_dwScreenHeight;
#ifdef TRON20SRC_MODERN
	// CL-30: the page's left and top edges (static: the class keeps
	// retail's size, which CInterfaceMgr's layout checks test)
	static int			m_nXOffset;
	static int			m_nYOffset;

#endif

	char				m_szSoundSelect[kMaxSoundFilename];
	char				m_szSoundChange[kMaxSoundFilename];
	char				m_szSoundPageChange[kMaxSoundFilename];
	char				m_szSoundUnselectable[kMaxSoundFilename];
	char				m_szSoundArrowUp[kMaxSoundFilename];
	char				m_szSoundArrowDown[kMaxSoundFilename];
	char				m_szSoundArrowLeft[kMaxSoundFilename];
	char				m_szSoundArrowRight[kMaxSoundFilename];
	char				m_szSoundObjAdd[kMaxSoundFilename];
	char				m_szSoundObjRemove[kMaxSoundFilename];
	char				m_szSoundObjComplete[kMaxSoundFilename];

	CLTGUIWindow	    m_MsgDlg;
	CLTGUITextCtrl*		m_pMsgText;

};

#define TERMSHAREDSURF(surf) if(surf) { g_pInterfaceResMgr->FreeSharedSurface(surf); surf = NULL; }

inline uint32 CInterfaceResMgr::GetScreenWidth()
{
	if (m_dwScreenWidth < 0)
		ScreenDimsChanged();
	return m_dwScreenWidth;
}
inline uint32 CInterfaceResMgr::GetScreenHeight()
{
	if (m_dwScreenHeight < 0)
		ScreenDimsChanged();
	return m_dwScreenHeight;
}


#endif // !defined(_INTERFACERESMGR_H_)