//-------------------------------------------------------------------------
//
// MODULE  : ProceduralCtrl.h
//
// PURPOSE : GUI element for interacting with the visual representation of
//			 procedurals
//
// CREATED : 4/3/02 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// RETAIL: Tron 2.0 1.030's, written from retail CShell.dll (0x1004e460..
// 0x1004f760): the subroutine screen's optimizer (its +0x3950) and its
// three procedural controls (its +0xdb8, 0x444 bytes each). Neither class
// has a vtable; build 69's CProceduralCtrl drew its own BaseScaleFX models,
// retail's is built of menu piece models (CMenuPieceModel). The optimizer's
// class name and all member names are ours.
//
//-------------------------------------------------------------------------

#ifndef __PROCEDURAL_CTRL_H
#define __PROCEDURAL_CTRL_H

#include "MenuPieceModel.h"
#include "ClientFXMgr.h"
#include "SubroutineMgr.h"

class CUIPolyString;

// RETAIL: 0x1004ead0, lays a poly string's characters out along an arc
// around (x, y), starting at fAngle (degrees); nJustify 0 centres it on
// fAngle, 1 starts it there, 2 ends it there (names ours)
void ArcPolyString(CUIPolyString * pStr, int x, int y, float fRadius, float fAngle,
				   bool bOutside, int nJustify);

// RETAIL: 0x1004ec90, one character's poly of an arc (the name is ours)
void SetArcPoly(LT_POLYGT4 * pPoly, int x, int y, float fRadius, float fHeight,
				float fStartAngle, float fEndAngle, bool bOutside);


// The optimizer: drop a subroutine into it to raise its version (alpha to
// beta to gold). It works once.
class COptimizerCtrl
{
public:
	COptimizerCtrl();							// RETAIL: 0x1004e460
	~COptimizerCtrl();							// RETAIL: 0x1004e510

	bool			Init();						// RETAIL: 0x1004e5b0
	void			ScreenDimsChanged();		// RETAIL: 0x1004e660
	void			Term();						// RETAIL: 0x1004e720

	LTBOOL			OnMouseMove(int x, int y, PlayerSubroutine * pCursorSub);	// RETAIL: 0x1004e790
	bool			CanAccept(PlayerSubroutine * pSub);	// RETAIL: 0x1004e8d0
	void			Update();					// RETAIL: 0x1004e910: done optimizing?
	void			Optimize(PlayerSubroutine * pSub);	// RETAIL: 0x1004e990

	bool			IsHot()			{return m_bHot;}
	bool			IsBusy()		{return m_bBusy;}
	bool			IsDone()		{return m_bDone;}
	PlayerSubroutine * GetSub()		{return m_pSub;}
	LTIntPt			GetCenter()		{return m_Center;}

private:
	void			AddFX();					// RETAIL: 0x1004ea70

	bool			m_bInit;					// +0x0
	bool			m_bBusy;					// +0x1 optimizing a subroutine
	bool			m_bDone;					// +0x2 used up
	bool			m_bHot;						// +0x3 the cursor is over it
	LTIntPt			m_Center;					// +0x4
	LTRect			m_HitRect;					// +0xc
	PlayerSubroutine * m_pSub;					// +0x1c
	bool			m_bCanAccept;				// +0x20 the cursor's subroutine can go in
	CMenuPieceModel	m_Optimizer;				// +0x24
	CMenuPieceModel	m_DeadOptimizer;			// +0xf8
	CMenuPieceModel	m_Sub;						// +0x1cc
	CMenuPieceModel	m_Background;				// +0x2a0
	CLIENTFX_LINK	m_FXLink;					// +0x374
};


class CProceduralCtrl
{
public:
	CProceduralCtrl();							// RETAIL: 0x1004ee30
	~CProceduralCtrl();							// RETAIL: 0x1004eee0

	bool			Init(Procedural * pProc);	// RETAIL: 0x1004ef70
	void			ScreenDimsChanged();		// RETAIL: 0x1004f260
	void			Term();						// RETAIL: 0x1004f2c0

	void			SetHighlight(bool bHighlight);	// RETAIL: 0x1004f370
	LTBOOL			OnMouseMove(int mx, int my, PlayerSubroutine * pCursorSub);	// RETAIL: 0x1004f400

	void			RemoveSub();				// RETAIL: 0x1004f530
	void			AddSub(PlayerSubroutine * pSub);	// RETAIL: 0x1004f5c0

	bool			IsHot()			{return m_bHot;}
	bool			CanAccept()		{return m_bCanAccept;}
	bool			IsWorking()		{return m_bWorking;}

	// Accessors
	Procedural *	GetProc()		{return m_pProc;}
	PlayerSubroutine * GetSub()		{return m_pProc ? m_pProc->pSub : LTNULL;}
	LTIntPt			GetCenter()		{return m_CenterPos;}

private:
	bool			m_bInit;					// +0x0
	bool			m_bWorking;					// +0x1 a subroutine is in it
	Procedural *	m_pProc;					// +0x4 which actual procedural do we represent?
	bool			m_bCanAccept;				// +0x8 the cursor's subroutine can go in
	bool			m_bHighlight;				// +0x9
	bool			m_bHot;						// +0xa flag for when a mouse cursor is over us
	int				m_iSlot;					// +0xc 0 left, 1 right, 2 bottom
	LTIntPt			m_CenterPos;				// +0x10 screen coordinates for center of procedural
	int				m_iRadius;					// +0x18 size
	HLTSOUND		m_hWorkSound;				// +0x1c

	CMenuPieceModel	m_Base;						// +0x20
	CMenuPieceModel	m_Working;					// +0xf4
	CMenuPieceModel	m_Progress;					// +0x1c8
	CMenuPieceModel	m_Sub;						// +0x29c
	CMenuPieceModel	m_Condition;				// +0x370
};


#endif // __PROCEDURAL_CTRL_H
