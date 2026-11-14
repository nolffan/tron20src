//-------------------------------------------------------------------------
//
// MODULE  : RatingBarCtrl.h
//
// PURPOSE : The subroutine screen's performance rating bars and the bar of
//			 the upgrade (build) points they are bought with
//
// RETAIL: Tron 2.0 1.030's, written from retail CShell.dll (0x1004fa70..
// 0x100510f8). Neither class has a vtable, so no name survives: the file,
// class and member names are ours. The subroutine screen keeps one points
// bar (its +0x2100) and five rating bars (its +0x2214, "rating%d").
//
//-------------------------------------------------------------------------

#ifndef __RATING_BAR_CTRL_H
#define __RATING_BAR_CTRL_H

#include "MenuPieceModel.h"

class CUIPolyString;

// The upgrade points left to spend on the ratings: a ring around the
// screen's centre that fills by the points of a level.
class CBuildPointsCtrl
{
public:
	CBuildPointsCtrl();							// RETAIL: 0x1004fb10
	~CBuildPointsCtrl();						// RETAIL: 0x1004fb60

	void			Init(int nPoints);			// RETAIL: 0x1004fbb0
	void			Term();						// RETAIL: 0x1004fc50
	void			ScreenDimsChanged();		// RETAIL: 0x1004fc70

	std::wstring	GetHelpText();				// RETAIL: 0x1004fce0
	void			OnMouseMove(int x, int y);	// RETAIL: 0x1004fe60
	LTIntPt			GetHelpPos();				// RETAIL: 0x1004fef0: the ring's edge, away from the cursor

	void			Increment();				// RETAIL: 0x1004ff40: a point given back
	void			Decrement();				// RETAIL: 0x1004ff80: a point spent
	void			Apply();					// RETAIL: 0x1004ffb0: into the rating manager
	void			Update();					// RETAIL: 0x1004ffc0

	void			SetVisible(bool bVisible)	{m_bVisible = bVisible;}
	bool			IsVisible()		{return m_bVisible;}
	bool			IsHot()			{return m_bHot;}
	int				GetPoints()		{return m_nPoints;}

private:
	void			SetBarValue(float fValue);	// RETAIL: 0x100501b0

	bool			m_bInit;					// +0x0
	bool			m_bVisible;					// +0x1 there are points to spend
	bool			m_bShown;					// +0x2
	bool			m_bHot;						// +0x3
	bool			m_bLeft;					// +0x4 the cursor is left of the centre
	LTIntPt			m_Center;					// +0x8
	int				m_nTop;						// +0x10
	int				m_nBottom;					// +0x14
	float			m_fInnerRadius;				// +0x18
	float			m_fOuterRadius;				// +0x1c
	int				m_nPoints;					// +0x20
	int				m_nLevelPoints;				// +0x24 the points of a full ring
	int				m_nTarget;					// +0x28 percent of the ring
	float			m_fCurrent;					// +0x2c percent shown
	float			m_fSpeed;					// +0x30 percent a second
	CMenuPieceModel	m_Bar;						// +0x34
	float			m_fLastTime;				// +0x108
	float			m_fAlpha;					// +0x10c the bar pulses
	int				m_nAlphaDir;				// +0x110
};


// One performance rating's bar, with its buttons to spend upgrade points.
class CRatingBarCtrl
{
public:
	CRatingBarCtrl();							// RETAIL: 0x100501f0
	~CRatingBarCtrl();							// RETAIL: 0x10050380

	void			Term();						// RETAIL: 0x10050410
	// RETAIL: 0x10050480, the buttons' rectangles at 640x480
	void			SetRects(LTRect rcLeft, LTRect rcRight);
	void			SetTextVisible(bool bVisible);	// RETAIL: 0x10050580
	void			Show(bool bShow);			// RETAIL: 0x100505a0
	LTIntPt			GetCenter();				// RETAIL: 0x100505f0
	std::wstring	GetHelpText();				// RETAIL: 0x10050630
	void			ScreenDimsChanged();		// RETAIL: 0x10050800
	void			Init(int nRating, int nValue);	// RETAIL: 0x10050920
	void			OnMouseMove(int x, int y);	// RETAIL: 0x10050c90
	void			OnLButtonDown(int x, int y);	// RETAIL: 0x10050d90
	void			Apply();					// RETAIL: 0x10050f30: into the rating manager
	void			Update();					// RETAIL: 0x10050f50

	void			SetBuildPoints(CBuildPointsCtrl * pPoints) {m_pBuildPoints = pPoints;}
	bool			IsHot()			{return m_bHot;}
	int				GetValue()		{return m_nValue;}

private:
	void			SetBarValue(float fValue, bool bFixed);	// RETAIL: 0x100510c0

	bool			m_bInit;					// +0x0
	bool			m_bButtonsShown;			// +0x1
	bool			m_bTextVisible;				// +0x2
	int				m_nMin;						// +0x4
	int				m_nMax;						// +0x8
	int				m_nValue;					// +0xc
	int				m_nInitialValue;			// +0x10 can't go below it
	int				m_nTarget;					// +0x14 percent of the bar
	float			m_fCurrent;					// +0x18 percent shown
	float			m_fSpeed;					// +0x1c percent a second
	float			m_fFixed;					// +0x20 percent of the fixed bar
	int				m_nRating;					// +0x24 PerformanceRating
	CMenuPieceModel	m_FixedBar;					// +0x28
	CMenuPieceModel	m_Bar;						// +0xfc
	CMenuPieceModel	m_LeftButton;				// +0x1d0
	CMenuPieceModel	m_RightButton;				// +0x2a4
	CMenuPieceModel	m_Surge;					// +0x378
	CUIPolyString *	m_pLabel;					// +0x44c
	float			m_fFade;					// +0x450 the label fades in
	float			m_fLastTime;				// +0x454
	bool			m_bHot;						// +0x458
	bool			m_bCanDecrease;				// +0x459
	bool			m_bCanIncrease;				// +0x45a
	bool			m_bOverRight;				// +0x45b
	bool			m_bOverLeft;				// +0x45c
	LTRect			m_rcBaseLeft;				// +0x460
	LTRect			m_rcBaseRight;				// +0x470
	LTRect			m_rcLeft;					// +0x480
	LTRect			m_rcRight;					// +0x490
	CBuildPointsCtrl * m_pBuildPoints;			// +0x4a0
};

#endif // __RATING_BAR_CTRL_H
