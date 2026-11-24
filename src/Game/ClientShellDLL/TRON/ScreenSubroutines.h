// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSubroutines.h
//
// PURPOSE : Screen for managing player's inventory of "subroutines"
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// RETAIL: Tron 2.0 1.030's, written from retail CShell.dll
// (0x1007d090..0x10081e25). The class is 0x4fc8 bytes (constructor
// 0x1007d3e0, destructor 0x1007d620). The names of the members and of the
// functions that have no counterpart in Build 69 are ours.
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_SUBROUTINES_H_
#define _SCREEN_SUBROUTINES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "ArcCtrl.h"
#include "LibraryCtrl.h"
#include "ProceduralCtrl.h"
#include "RatingBarCtrl.h"
#include "Ratings.h"
#include "SubroutineMgr.h"
#include "ToolTip.h"
#include "MenuPieceModel.h"
#include "DuplicateSubroutineDlg.h"
#include "PopupText.h"

// RETAIL: every TU that includes this header has these initializers
const int NUM_SECTORS = 24;
const int PRIMS_PER_SECTOR = 3;

const float SLICE_SIZE = MATH_PI * 2.0f / (float)NUM_SECTORS;
const float PRIM_SIZE = SLICE_SIZE / PRIMS_PER_SECTOR;

typedef enum
{
	CursorPickup = 0,
	CursorSubroutineDrop,
	CursorAdditiveDrop
} SubCursorMode;


class CScreenSubroutines : public CBaseScreen
{
public:
	CScreenSubroutines();							// RETAIL: 0x1007d3e0
	virtual ~CScreenSubroutines();					// RETAIL: 0x1007d620

	virtual void	Term();							// RETAIL: 0x1007d800

	// Build the screen
	virtual LTBOOL	Build();						// RETAIL: 0x1007d840

	// RETAIL: the system's name (0x1007e0a0)
	void		SetSystemName(int nStringID);

	// RETAIL: a popup to show when the screen draws next (0x1007e1c0)
	void		SetPopupText(uint32 nTextId, uint8 nPopupId, char * pText);

	// This is called when the screen gets or loses focus
	virtual void	OnFocus(LTBOOL bFocus);			// RETAIL: 0x1007e210
	virtual void	Escape();						// RETAIL: 0x1007f050

	virtual LTBOOL	HandleKeyDown(int key, int rep);	// RETAIL: 0x1007f000

	virtual LTBOOL	OnMouseMove(int x, int y);		// RETAIL: 0x1007f780
	virtual LTBOOL	OnLButtonDown(int x, int y);	// RETAIL: 0x10081220
	virtual LTBOOL	OnLButtonUp(int x, int y);		// RETAIL: 0x10081380
	virtual LTBOOL	OnLButtonDblClick(int x, int y);	// RETAIL: 0x100812d0
	virtual LTBOOL	OnRButtonDown(int x, int y);	// RETAIL: 0x10081420
	virtual LTBOOL	OnWheelUp();					// RETAIL: 0x1007f6a0
	virtual LTBOOL	OnWheelDown();					// RETAIL: 0x1007f710

	virtual void	ScreenDimsChanged();			// RETAIL: 0x10081c20

	// Used by the SubroutineMgr
	void		ClearScreen();						// RETAIL: 0x1007ffd0
	void		AddSubroutine(PlayerSubroutine * pSubroutine);	// RETAIL: 0x10080140
	void		AddProcedural(Procedural * pProcedural);	// RETAIL: 0x10080260

	void		Compile();							// RETAIL: 0x10080350

	// RETAIL: the 1.030 screen shows the performance ratings, which
	// CRatingMgr::PopulateRatings (0x100516d0) fills
	void		SetRating(int nRating, int nValue);	// RETAIL: 0x10080070
	void		SetRatingPoints(int nPoints);		// RETAIL: 0x100800c0
	void		SetOptimizer(HOBJECT hOptimizer);	// RETAIL: 0x100800e0

	// RETAIL: inline in CSubroutineMgr::PopulateSubroutineScreen
	// (0x10094ddb..0x10094e03; the names are ours)
	void		FlashLibrary(int nLibrary)	{m_Libraries[nLibrary].StartFlash();}
	void		UpdateModels()				{m_ModelMgr.Update();}

protected:

	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);	// RETAIL: 0x1007ef90
	virtual LTBOOL	Render(HSURFACE hDestSurf);		// RETAIL: 0x1007f1e0

	void		TermControls();						// RETAIL: 0x1007ed10
	void		CreateMenuPieces();					// RETAIL: 0x1007edf0
	void		UpdateIntro();						// RETAIL: 0x1007f070

	void		ClearMemory();						// RETAIL: 0x100802c0
	void		PickupSubroutine(PlayerSubroutine * pSub);	// RETAIL: 0x100803b0
	void		PutSubroutineBack(bool bKeepIcon = true);	// RETAIL: 0x10081490
	int			FindFreeSegment(PlayerSubroutine * pSub);	// RETAIL: 0x10080fb0
	bool		CanDropHere();						// RETAIL: 0x10081d00

	// mouse button handlers
	void		OnPickupClick(int x, int y);		// RETAIL: 0x10080510
	void		OnSubroutineClick(int x, int y);	// RETAIL: 0x10080670
	void		OnPickupRClick(int x, int y);		// RETAIL: 0x10080b70
	void		OnSubroutineRClick(int x, int y);	// RETAIL: 0x10081020
	void		OnDuplicateClick(int x, int y);		// RETAIL: 0x10081100

	void		SetCursorMode(SubCursorMode mode);	// RETAIL: 0x100815d0

	void		ShowSubInfo(LTIntPt pos);			// RETAIL: 0x10081750
	void		ShowProcInfo(LTIntPt pos);			// RETAIL: 0x10081890
	void		ShowRatingInfo(LTIntPt pos);		// RETAIL: 0x10081990
	void		ShowBuildPointsInfo(LTIntPt pos);	// RETAIL: 0x10081a70
	void		ShowCtrlInfo(LTIntPt pos, CLTGUICtrl * pCtrl);	// RETAIL: 0x10081b60

	void		UpdateAnimSound();					// RETAIL: 0x10081d90

	bool			m_bDoneIntro;				// +0x790
	LTBOOL			m_bShowToolTips;			// +0x794
	bool			m_bToolTips;				// +0x798
	float			m_fToolTipWidth;			// +0x79c
	int				m_nToolTipFontSize;			// +0x7a0
	LTIntPt			m_CursorPos;				// +0x7a4
	int				m_nOldScreenGlow;			// +0x7ac

	CLTGUITextCtrl*	m_pSysName;					// +0x7b0
	CLTGUITextCtrl*	m_pClear;					// +0x7b4
	CLTGUITextCtrl*	m_pToolTipTitle;			// +0x7b8
	CLTGUIToggle*	m_pToolTipToggle;			// +0x7bc
	CLTGUICtrl*		m_pLastCtrl;				// +0x7c0

	SubCursorMode	m_eCursorMode;				// +0x7c4
	PlayerSubroutineArray m_SubArray;			// +0x7c8
	ProceduralArray m_ProcArray;				// +0x7d4

	CArcCtrl		m_SystemMemoryCtrl;			// +0x7e0

	enum { NUM_LIBRARIES = 3 };
	CLibraryCtrl	m_Libraries[NUM_LIBRARIES];	// +0x878

	enum { NUM_PROC_CTRLS = 3 };
	CProceduralCtrl m_ProcCtrl[NUM_PROC_CTRLS];	// +0xdb8

	PlayerSubroutine * m_pHotSub;				// +0x1a84
	PlayerSubroutine * m_pCursorSub;			// +0x1a88
	bool			m_bBuildPointsHot;			// +0x1a8c
	Procedural *	m_pHotProc;					// +0x1a90

	CToolTip		m_ToolTip;					// +0x1a94

	typedef std::vector<CMenuPieceModel *> ModelPieceArray;
	ModelPieceArray m_ModelPieceArray;			// +0x2060

	CLTGUITextCtrl*	m_pBuildNoteCount;			// +0x206c
	LTIntPt			m_BuildNoteCountPos;		// +0x2070
	LTIntPt			m_TronUnknown2078;			// +0x2078
	int				m_nBuildNoteCountStringId;	// +0x2080
	bool			m_bBuildNoteFade;			// +0x2084
	float			m_fBuildNoteFade;			// +0x2088
	int				m_nBuildNoteIconSpacer;		// +0x208c
	float			m_fBuildNoteIconScalar;		// +0x2090
	LT_POLYFT4		m_BuildNoteIconPoly;		// +0x2094
	HTEXTURE		m_hBuildNoteIcon;			// +0x20e8

	CLTGUITextCtrl*	m_pVersion;					// +0x20ec
	LTIntPt			m_VersionPos;				// +0x20f0
	bool			m_bTextFade;				// +0x20f8
	float			m_fTextFade;				// +0x20fc

	// RETAIL: the ratings and the upgrade points to raise them with
	CBuildPointsCtrl	m_BuildPoints;			// +0x2100
	CRatingBarCtrl		m_RatingBars[NUM_RATINGS];	// +0x2214
	bool			m_bRatingLeftHot;			// +0x3948
	bool			m_bRatingRightHot;			// +0x3949
	CRatingBarCtrl*	m_pHotRating;				// +0x394c

	COptimizerCtrl	m_Optimizer;				// +0x3950
	bool			m_bOptimizerHot;			// +0x3cc8
	HOBJECT			m_hOptimizer;				// +0x3ccc the optimizer object
	int				m_nTronUnknown3CD0;			// +0x3cd0

	CMenuPieceModelMgr	m_ModelMgr;				// +0x3cd4
	CMenuPieceModel *	m_pScreenModel;			// +0x3ce0
	float			m_fTronUnknown3CE4;			// +0x3ce4
	float			m_fLButtonDownTime;			// +0x3ce8

	CDuplicateSubroutineDlg	m_DuplicateDlg;		// +0x3cec
	PlayerSubroutine * m_pDuplicateFirst;		// +0x445c
	PlayerSubroutine * m_pDuplicateSecond;		// +0x4460

	CPopupText		m_PopupText;				// +0x4464
	uint32			m_nPopupTextId;				// +0x4f34
	uint8			m_nPopupId;					// +0x4f38
	char			m_szPopupText[128];			// +0x4f39
	int				m_nLastSegment;				// +0x4fbc
	HLTSOUND		m_hAnimSound;				// +0x4fc0
	bool			m_bLibraryCCWHot;			// +0x4fc4
	bool			m_bLibraryCWHot;			// +0x4fc5
};

#endif // _SCREEN_SUBROUTINES_H_
