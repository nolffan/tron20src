// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProgress.h
//
// PURPOSE : HUDItem to display objectives and working procedurals
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PROGRESS_H
#define __HUD_PROGRESS_H

#include "HUDItem.h"

typedef enum {
	// outer frame
	FRAME_TOP_LEFT = 0,
	FRAME_TOP,
	FRAME_TOP_RIGHT,
	FRAME_LEFT,
	FRAME_BOTTOM_LEFT,
	FRAME_BOTTOM,

	// inner box
	BOX_TOP_LEFT,
	BOX_TOP,
	BOX_TOP_RIGHT,
	BOX_LEFT,
//	BOX_LEFT_INSET,
	BOX_CENTER,
	BOX_RIGHT,
	BOX_BOTTOM_LEFT,
	BOX_BOTTOM,
	BOX_BOTTOM_RIGHT,
	LAST_PRIM,
} ePrimType;

typedef enum {
	PROC_TOP_LEFT,
	PROC_TOP,
	PROC_TOP_RIGHT,
	PROC_LEFT,
	PROC_CENTER,
	PROC_RIGHT,
	PROC_BOTTOM_LEFT,
	PROC_BOTTOM,
	PROC_BOTTOM_RIGHT,
} eBoxPieceType;

//******************************************************************************************
//** HUD Ratings display
//******************************************************************************************
// RETAIL: Tron's (0x2c54 bytes; ctor 0x10032de0, vtable 0x101e70b8). The
// box is two textured frames ("chooserbox" inside "chooserframe"), one
// frame per running procedural (three at most), and an inset by each
// objective. New objectives flash when the box opens. Members not in NOLF2
// are named by us.
#include "TexturedFrame.h"

#define HUDPROGRESS_MAX_PROCS		3
#define HUDPROGRESS_MAX_OBJECTIVES	10

class CHUDProgress : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDProgress();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	void		UpdateGraphics();
	int			GetPieceWidth(ePrimType eType);
	int			GetPieceHeight(ePrimType eType);

	// RETAIL: 0x10034620: reads the layout's values (the name is ours)
	void		ReadLayout();

	void		SetUVWH(LT_POLYFT4 * pPrim, LTRect rect);

	float		m_fScale;							// +0x10

	bool		m_bShow;							// +0x14

	int			m_nNumObjectives;					// +0x18
	int			m_nNumCompletedObjectives;			// +0x1c
	int			m_nNumSecondaryObjectives;			// +0x20
	int			m_nNumProcs;						// +0x24

	// Drawing parameters
	LTRect		m_rcProgress;						// +0x28
	int			m_iTitleFont;						// +0x38
	int			m_iTitleFontSize;
	uint32		m_iTitleFontColor;

	int			m_iObjectiveFont;					// +0x44
	int			m_iObjectiveFontSize;
	uint32		m_iObjectiveFontColor;
	int			m_iObjectiveSpacing;				// +0x50
	uint32		m_iCompletedObjectiveFontColor;
	uint32		m_iSecondaryObjectiveFontColor;

	int			m_iProceduralFont;					// +0x5c
	int			m_iProceduralFontSize;
	uint32		m_iProceduralFontColor;
	int			m_iProceduralHeight;				// +0x68

	// Drawing elements
	HTEXTURE	m_hProgressTex;						// +0x6c
	int			m_iHUDOffset;						// +0x70

	LT_POLYFT4	m_PrimArray[LAST_PRIM];				// +0x74
	LTRect		m_rcPrimDims[LAST_PRIM];			// +0x560

	// the insets by each objective (the procedurals' use three)
	LT_POLYFT4	m_ObjectiveInset[HUDPROGRESS_MAX_OBJECTIVES];	// +0x650
	float		m_fInsetWidth;						// +0x998
	float		m_fInsetHeight;						// +0x99c
	LT_POLYFT4	m_SecondaryInset[HUDPROGRESS_MAX_OBJECTIVES];	// +0x9a0
	LT_POLYFT4	m_ProcInset[HUDPROGRESS_MAX_OBJECTIVES];		// +0xce8
	LTRect		m_InsetPrimDims;					// +0x1030
	LT_POLYFT4	m_CompletedInset[HUDPROGRESS_MAX_OBJECTIVES];	// +0x1040
	LTRect		m_InsetFullPrimDims;				// +0x1388

	bool		m_bIsProcActive[HUDPROGRESS_MAX_PROCS];		// +0x1398
	float		m_fProcPercent[HUDPROGRESS_MAX_PROCS];		// +0x139c
	CUIPolyString * m_pProcStr[HUDPROGRESS_MAX_PROCS];		// +0x13a8
	CUIPolyString * m_pSubroutineStr[HUDPROGRESS_MAX_PROCS];	// +0x13b4

	LT_POLYFT4	m_ProcPrim[HUDPROGRESS_MAX_PROCS];			// +0x13c0
	LT_POLYFT4	m_SubPrim[HUDPROGRESS_MAX_PROCS];			// +0x14bc
	HTEXTURE	m_hProcTex[HUDPROGRESS_MAX_PROCS];			// +0x15b8
	HTEXTURE	m_hSubTex[HUDPROGRESS_MAX_PROCS];			// +0x15c4

	HTEXTURE	m_hBarTex;									// +0x15d0
	LT_POLYFT4	m_ProgressBar[HUDPROGRESS_MAX_PROCS];		// +0x15d4
	LTRect		m_rcSeparator;						// +0x16d0
	LT_POLYFT4	m_SeparatorPrim;					// +0x16e0

	CUIFormattedPolyString * m_pTitleStr;			// +0x1734
	CUIFormattedPolyString * m_pOptionalStr;		// +0x1738

	CUIFormattedPolyString * m_pObjectiveStr[HUDPROGRESS_MAX_OBJECTIVES];			// +0x173c
	CUIFormattedPolyString * m_pSecondaryObjectiveStr[HUDPROGRESS_MAX_OBJECTIVES];	// +0x1764
	CUIFormattedPolyString * m_pCompletedObjectiveStr[HUDPROGRESS_MAX_OBJECTIVES];	// +0x178c

	CTexturedFrame	m_ProcFrame[HUDPROGRESS_MAX_PROCS];	// +0x17b4
	CTexturedFrame	m_BoxFrame;							// +0x23fc "chooserbox"
	CTexturedFrame	m_OuterFrame;						// +0x2814 "chooserframe"

	// the objectives still in the player stats' unseen list (they flash)
	bool		m_bNewObjective[HUDPROGRESS_MAX_OBJECTIVES];			// +0x2c2c
	bool		m_bNewSecondaryObjective[HUDPROGRESS_MAX_OBJECTIVES];	// +0x2c36
	bool		m_bNewCompletedObjective[HUDPROGRESS_MAX_OBJECTIVES];	// +0x2c40
	float		m_fLastFlashTime;					// +0x2c4c
	float		m_fFlashTime;						// +0x2c50: left to flash
};

#endif // __HUD_PROGRESS_H
