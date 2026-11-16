//-------------------------------------------------------------------------
//
// MODULE  : ArcCtrl.h
//
// PURPOSE : GUI element for interacting with a segmented arc of buttons
//
// CREATED : 12/31/01 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __ARC_CTRL_H
#define __ARC_CTRL_H

#include "iltdrawprim.h"
#include "ScreenSpriteMgr.h"
#include "BaseScaleFX.h"
#include "TronLayoutMgr.h"
#include "SubroutineMgr.h"
#include "MenuPieceModel.h"

// RETAIL: Tron's segment types index the model names at 0x1020ff48
// ("none" .. "sys_gold"); Build 69's corruption types are gone.
typedef enum 
{
	SEG_TYPE_NONE = 0,

	SEG_LIBRARY_HIGHLIGHT,
	SEG_SYSTEM_HIGHLIGHT,
	SEG_SYSTEM_CONFLICT,
	SEG_SYSTEM_BADBLOCK,
	SEG_SYSTEM_BASECODE,
	SEG_SYSTEM_BASECODE_LEFT,
	SEG_SYSTEM_BASECODE_MIDDLE,
	SEG_SYSTEM_BASECODE_RIGHT,
	SEG_SYSTEM_ALPHA,
	SEG_SYSTEM_BETA,
	SEG_SYSTEM_GOLD,

	SEG_TYPE_LAST,
} SegmentType;

typedef enum
{
	ARC_TYPE_UNDEFINED = 0,
	ARC_TYPE_SYSTEM_MEMORY,
	ARC_TYPE_LIBRARY,
} ArcType;

// RETAIL: Tron's CArcSector (0x474 bytes: ctor 0x10001120, array new
// 0x10002d30) shows a subroutine with menu piece models attached to the
// sockets of its parent piece; Build 69's sprites and scale FX are gone.
class CArcSector
{
public:
	CArcSector();												// RETAIL: 0x10001120
	~CArcSector();												// RETAIL: 0x10001210

	void Build(int iNum, LTIntPt pt, float r1, float r2, float theta1, float theta2, ArcType eArcType);	// RETAIL: 0x100012a0
	void Resize(LTIntPt pt, float r1, float r2);				// RETAIL: 0x10001340 (the name is ours)
	void Term();												// RETAIL: 0x10001370

	PlayerSubroutine * GetSubObject()		{return m_pSubPtr;}

	void GetRange(int &iFirst, int &iLast) {iFirst = m_iFirst; iLast = m_iLast;}

	void ShowSprites(bool bShow);								// RETAIL: 0x10001380
	void ShowIcon(bool bShow);									// RETAIL: 0x100013d0
	void SetParentPiece(char * szParent);						// RETAIL: 0x100013f0 (the name is ours)

	void Occupy(int iFirst, int iLast, PlayerSubroutine * pSubPtr);	// RETAIL: 0x10001400
	void ClearFX();												// RETAIL: 0x10001800

	bool IsOccupied() {return m_bOccupied;}

	LTIntPt GetIconPos() {return m_IconPos;}

	void AddHighlighting(SegmentType eSegType, int iSize);		// RETAIL: 0x10001850
	void RemoveHighlighting();									// RETAIL: 0x10001980

private:
	bool			m_bHighlighted;			// +0x00 (the name is ours)
	ArcType			m_eArcType;				// +0x04
	int				m_iSectorNumber;		// +0x08
	int				m_iAnimNumber;			// +0x0c the socket of the sector's center
	bool			m_bOccupied;			// +0x10 used for the 'drop' cursor
	uint32			m_iColor;				// +0x14
	int				m_iFirst;				// +0x18
	int				m_iLast;				// +0x1c
	bool			m_bShowSprites;			// +0x20
	bool			m_bShowIcon;			// +0x21
	int				m_nTronUnknown24;		// +0x24 RETAIL: not touched in this TU

	PlayerSubroutine * m_pSubPtr;			// +0x28

	// The models (the names are ours)
	CMenuPieceModel	m_Highlight;			// +0x2c "lib_hi", "sys_hi", ...
	CMenuPieceModel	m_Ring;					// +0x100 "optring_*"
	CMenuPieceModel	m_Icon;					// +0x1d4 the subroutine's own model
	CMenuPieceModel	m_Condition;			// +0x2a8 "corruption_*"
	CMenuPieceModel	m_Segment;				// +0x37c the system memory segment

	char *			m_szParent;				// +0x450 the piece to attach to

	// Visual appearance parameters
	LTIntPt			m_Position;				// +0x454
	LTIntPt			m_IconPos;				// +0x45c (the name is ours)
	float			m_fInnerRadius;			// +0x464
	float			m_fOuterRadius;			// +0x468
	float			m_fTheta1;				// +0x46c
	float			m_fTheta2;				// +0x470
};

// RETAIL: Tron's CArcCtrl is 0x98 bytes (ctor 0x100019c0, Init 0x10001af0).
class CArcCtrl
{
public:
	CArcCtrl();													// RETAIL: 0x100019c0
	virtual ~CArcCtrl();										// RETAIL: 0x10001a10

	LTBOOL		Render();										// RETAIL: 0x10001a20

	LTBOOL		Init();											// RETAIL: 0x10001af0
	LTBOOL		Init(LTIntPt pos, float fInner, float fOuter, int nNumSegments);	// RETAIL: 0x10001c00
	void		SetArcType(ArcType eType) {m_eArcType = eType;}

	void		Term();											// RETAIL: 0x10001c70

	void		Rescale();										// RETAIL: 0x10001cf0 (the name is ours)

	// Parameters for controlling appearance and input
	void		SetArc(float fArcStart, float fArcEnd);			// RETAIL: 0x10001d70
	void		SetNumHighlightSegments(int num);				// RETAIL: 0x10001e40
	void		ShowOutlines(LTBOOL bShowOutlines);				// RETAIL: 0x10001e70

	int			GetActiveSegment() { return m_CurrentSegment; }

	PlayerSubroutine * GetSegmentObject(int iSeg);				// RETAIL: 0x10001e90
	void		AssociateModel(char * szParent);				// RETAIL: 0x10001ec0 (the name is ours)
	char *		GetParentPiece() {return m_szParent;}
	PlayerSubroutine * GetHotSub();								// RETAIL: 0x10001f10
	LTIntPt		GetHotPos();									// RETAIL: 0x10001f40 (the name is ours)

	void		SetLibraryState(int i) {m_iLibraryState = i;}
	int			GetLibraryState() {return m_iLibraryState;}

	void		ShowSprites(bool bShow);						// RETAIL: 0x10002020
	void		ShowIcon(int iSeg, bool bShow);					// RETAIL: 0x10002070

	void		OccupySegment(int segment, PlayerSubroutine * pSub);	// RETAIL: 0x100020a0
	void		ClearSegment(int segment);						// RETAIL: 0x10002190

	// User input
	LTBOOL		OnMouseMove(int mx, int my);					// RETAIL: 0x10002220

	void		RemoveHighlighting();							// RETAIL: 0x10002300
	void		AddHighlighting();								// RETAIL: 0x10002390

	LTBOOL		IsOnMe(int mx, int my);							// RETAIL: 0x10002470
	bool		IsHot() {return m_bIsHot;}

	void		Build();										// RETAIL: 0x10002760

protected:
	void		ComputeOptimalTesselation();					// RETAIL: 0x10002700
	float		VectorToAngle(float x, float y);				// RETAIL: 0x10002bb0

	ArcType		m_eArcType;				// +0x04
	int			m_iTesselation;			// +0x08

	// most recent mouse cursor coordinates
	int			m_mouseX;				// +0x0c
	int			m_mouseY;				// +0x10

	bool		m_bStartHot;			// +0x14 before the arc's start (the name is ours)
	bool		m_bEndHot;				// +0x15 past the arc's end (the name is ours)

	int			m_CurrentSegment;		// +0x18
	float		m_fSegmentTheta;		// +0x1c
	float		m_fMouseAngle;			// +0x20
	LTBOOL		m_bShowInactive;		// +0x24
	LTBOOL		m_bShowOutlines;		// +0x28

	uint32		m_HighlightColor;		// +0x2c
	uint32		m_EmptyColor;			// +0x30
	uint32		m_OutlineColor;			// +0x34
	uint32		m_ConflictColor;		// +0x38

	LTIntPt		m_Position;				// +0x3c
	float		m_fArcStart;			// +0x44
	float		m_fArcEnd;				// +0x48
	float		m_fInnerRadius;			// +0x4c
	float		m_fOuterRadius;			// +0x50
	float		m_fThreshold;			// +0x54
	int			m_nNumSegments;			// +0x58
	int			m_nNumHighlightSegments;	// +0x5c 1-3, 0 = ignore, -1 = pickup

	LTBOOL		m_bHasChangedAppearance;	// +0x60
	bool		m_bOnSegment;			// +0x64 (the name is ours)
	bool		m_bIsHot;				// +0x65

	char *		m_szParent;				// +0x68 the piece the sectors attach to

	// RETAIL: unscaled (640x480) values that Rescale scales
	float		m_fBaseInnerRadius;		// +0x6c
	float		m_fBaseOuterRadius;		// +0x70
	LTIntPt		m_BasePosition;			// +0x74

	// various arrays
	int			m_nSegmentPrims;		// +0x7c
	int			m_iLibraryState;		// +0x80 RETAIL: not touched in this TU
	int			m_nOutlinePrims;		// +0x84
	LT_POLYF4	*m_pOutlinePrims;		// +0x88
	int			m_nSeparatorPrims;		// +0x8c
	LT_POLYF4	*m_pSeparatorPrims;		// +0x90

	CArcSector	*m_SectorArray;			// +0x94
};

#endif // __ARC_CTRL_H
