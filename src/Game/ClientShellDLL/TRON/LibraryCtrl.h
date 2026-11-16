// ----------------------------------------------------------------------- //
//
// MODULE  : LibraryCtrl.h
//
// PURPOSE : One subroutine library of the subroutine screen: an arc of six
//			 sectors that rotates through the library's subroutines
//
// RETAIL: Tron 2.0 1.030's, written from retail CShell.dll
// (0x10090fb0..0x10091ddb). Retail has no RTTI or strings that name the
// class: the name of the class, the file and the members are ours. The
// subroutine screen keeps three of them (0x1c0 bytes each) at +0x878.
//
// ----------------------------------------------------------------------- //

#ifndef __LIBRARY_CTRL_H
#define __LIBRARY_CTRL_H

#include "ArcCtrl.h"
#include "MenuPieceModel.h"
#include "SubroutineMgr.h"

class CUIPolyString;

typedef enum
{
	LIBRARY_STATE_NONE = 0,
	LIBRARY_STATE_CLOSED,
	LIBRARY_STATE_OPEN,
	LIBRARY_STATE_OPENING,
	LIBRARY_STATE_CLOSING
} LibraryState;

class CLibraryCtrl
{
public:
	CLibraryCtrl();											// RETAIL: 0x10090fb0
	~CLibraryCtrl();										// RETAIL: 0x10091060

	void		Init(int nFunction, float fArcStart, float fArcEnd);	// RETAIL: 0x10091130
	void		Rescale();									// RETAIL: 0x100913e0
	void		AssociateModel(char * szPiece);				// RETAIL: 0x100913f0
	void		Term();										// RETAIL: 0x10091420
	void		Clear();									// RETAIL: 0x10091460

	void		OnMouseMove(int x, int y, PlayerSubroutine * pCursorSub);	// RETAIL: 0x100914a0
	void		Update();									// RETAIL: 0x100915a0
	void		Render();									// RETAIL: 0x10091830

	void		Open(bool bStayOpen);						// RETAIL: 0x10091860
	void		Close();									// RETAIL: 0x10091980
	void		Show();										// RETAIL: 0x100919f0

	void		AddSub(PlayerSubroutine * pSub);			// RETAIL: 0x10091a10
	void		InsertSub(PlayerSubroutine * pSub);			// RETAIL: 0x10091aa0
	void		RemoveSub(PlayerSubroutine * pSub);			// RETAIL: 0x10091b90
	void		ShowSubIcon(PlayerSubroutine * pSub, bool bShow);	// RETAIL: 0x10091c30

	void		OnClick();									// RETAIL: 0x10091c80
	void		RotateCW();									// RETAIL: 0x10091cc0
	void		RotateCCW();								// RETAIL: 0x10091d50

	CArcCtrl *	GetArc()			{return &m_Arc;}
	LibraryState GetState()			{return m_eState;}
	bool		IsHot()				{return m_bHot;}
	bool		IsRotateHot()		{return m_bCWHot || m_bCCWHot;}
	PlayerSubroutine * GetHotSub()	{return m_pHotSub;}
	void		SetFlash(bool bFlash) {m_bFlash = bFlash;}

protected:
	bool		m_bVisible;				// +0x00
	bool		m_bInit;				// +0x01
	bool		m_bStayOpen;			// +0x02
	bool		m_bHot;					// +0x03
	bool		m_bWasHot;				// +0x04
	bool		m_bCWHot;				// +0x05
	bool		m_bCCWHot;				// +0x06

	CArcCtrl	m_Arc;					// +0x08
	LibraryState m_eState;				// +0xa0
	int			m_nFunction;			// +0xa4 SubFunction: the slots are (m_nFunction + 1) * 100 + n
	CMenuPieceModel m_Model;			// +0xa8
	int			m_nLastIndex;			// +0x17c
	float		m_fCloseTime;			// +0x180
	float		m_fLastTime;			// +0x184
	PlayerSubroutineArray m_SubArray;	// +0x188
	PlayerSubroutine * m_pHotSub;		// +0x194
	CUIPolyString * m_pTitle;			// +0x198
	float		m_fTitleAngle;			// +0x19c
	float		m_fTitleRadius;			// +0x1a0
	float		m_fTitleHalfRadius;		// +0x1a4
	LTIntPt		m_Center;				// +0x1a8
	uint8		m_nTitleAlpha;			// +0x1b0
	float		m_fFadeTime;			// +0x1b4
	bool		m_bFlash;				// +0x1b8
	float		m_fFlashTime;			// +0x1bc
};

#endif // __LIBRARY_CTRL_H
