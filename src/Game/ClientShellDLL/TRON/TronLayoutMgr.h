// ----------------------------------------------------------------------- //
//
// MODULE  : TronLayoutMgr.h
//
// PURPOSE : Attribute file manager for interface layout info
//			 Tron-specific items
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_TRONLAYOUTMGR_H_)
#define _TRONLAYOUTMGR_H_

#include "LayoutMgr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//class CTronLayoutMgr;
//extern CTronLayoutMgr* g_pTronLayoutMgr;

// RETAIL: Tron's code reaches its layout manager by dynamic_cast'ing
// g_pLayoutMgr each time (e.g. retail 0x10040cb0); the macro's name is ours.
#define g_pTronLayoutMgr (dynamic_cast<CTronLayoutMgr*>(g_pLayoutMgr))
 
struct INT_SUBMODEL
{
	INT_SUBMODEL()	{szName[0] = szModel[0] = 0;}

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szName[128];
	char	szModel[128];

	CButeListReader blrSkins;
	CButeListReader blrRenderStyles;
};

struct INT_MENUPIECE // it's basically INT_CHAR
{
	// RETAIL: layout from retail 0x100a2520 (new, size 0x320) and Init 0x100a00a0
	INT_MENUPIECE()	{szName[0] = szModel[0] = szIntroAnim[0] = szLoopAnim[0] = szParentPiece[0] = szParentSocket[0] = 0; vPos.Init(); }

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szName[128];
	char	szModel[128];
	char	szIntroAnim[128];
	char	szLoopAnim[128];
	char	szParentPiece[128];
	char	szParentSocket[128];
	int		nLayer;

	CButeListReader blrSkins;
	CButeListReader blrRenderStyles;

    LTVector vPos;
};

class LCHUDHelper;

class CTronLayoutMgr : public CLayoutMgr
{
	// RETAIL: LCHUDHelper reads m_buteMgr directly (retail 0x10040cb0)
	friend class LCHUDHelper;

public:
	CTronLayoutMgr();
	virtual ~CTronLayoutMgr();

    LTBOOL			Init(const char* szAttributeFile=LO_DEFAULT_FILE);

	INT_MENUPIECE* GetMenuPiece(char * szModelName);
	INT_MENUPIECE* GetSubroutinePiece(struct PlayerSubroutine * pSub);	// RETAIL: 0x100a0550 (the name is ours)

	LCHUDHelper*	GetLCHUDHelper() { return m_pLCHUDHelper; }

	// RETAIL: in retail order (0x100a0600..0x100a21f0); names are ours,
	// after the attributes they read. The uint32 ones are GetInt cast to
	// uint16 (retail masks them).
	int         GetHealthEnergyFont(int nLayout);
	int         GetHealthEnergyFontSize(int nLayout);
	LTIntPt     GetInfoTextPos(int nLayout);
	uint32      GetInfoTextColor(int nLayout);
	uint32      GetInfoTextBadColor(int nLayout);
	uint8       GetInfoTextJustify(int nLayout);
	uint8       GetInfoTextSize(int nLayout);
	uint16      GetInfoTextWidth(int nLayout);
	LTIntPt     GetHealthBGOffset(int nLayout);
	LTIntPt     GetEnergyBGOffset(int nLayout);
	LTIntPt     GetHealthFGOffset(int nLayout);
	LTIntPt     GetEnergyFGOffset(int nLayout);
	uint32      GetHealthBarWidth(int nLayout);
	uint32      GetEnergyBarWidth(int nLayout);
	uint32      GetHealthBlinkTime(int nLayout);
	LTRect      GetHealthBGTex(int nLayout);
	LTRect      GetHealthTex(int nLayout);
	LTRect      GetHealthBlinkTex(int nLayout);
	LTRect      GetEnergyBGTex(int nLayout);
	LTRect      GetEnergyTex(int nLayout);
	LTRect      GetEnergyDeficitTex(int nLayout);
	LTIntPt     GetHealthTextOffset(int nLayout);
	LTIntPt     GetEnergyTextOffset(int nLayout);
	LTIntPt     GetArmorOffset(int nLayout);
	LTIntPt     GetPingIconSize(int nLayout);
	LTIntPt     GetPingStartPoint(int nLayout);
	uint32      GetPingSpacing(int nLayout);
	uint32      GetPingFontNum(int nLayout);
	uint32      GetPingFontSize(int nLayout);
	int         GetPingBlinkCount(int nLayout);
	LTFLOAT     GetPingBlinkTime(int nLayout);
	LTIntPt     GetPsetOffset(int nLayout);
	uint32      GetPsetWidth(int nLayout);
	uint32      GetPsetHeight(int nLayout);
    LTRect      GetProgressBoxRect(char *pAttribute);
	int         GetProgressHUDOffset();
	int         GetProgressTitleFont();
	int         GetProgressTitleFontSize();
	uint32      GetProgressTitleColor();
	int         GetProgressObjectiveFont();
	int         GetProgressObjectiveFontSize();
	int         GetProgressObjectiveSpacing();
	uint32      GetProgressObjectiveColor();
	uint32      GetProgressCompletedObjectiveColor();
	uint32      GetProgressSecondaryObjectiveColor();
	int         GetProgressProceduralFont();
	int         GetProgressProceduralFontSize();
	uint32      GetProgressProceduralColor();
	int         GetProgressProceduralHeight();
	int         GetProgressProceduralSpacing();
	LTRect      GetIOCameraRect();
	LTFLOAT     GetIOCameraFOVX();
	LTFLOAT     GetIOCameraFOVY();
	LTFLOAT     GetWeaponIndicatorAnimTime(int nLayout);
	LTIntPt     GetWeaponIndicatorOffset(int nLayout);
	LTIntPt     GetWeaponIndicatorIconOffset(int nLayout);
	LTIntPt     GetChooserBoxSize(int nLayout);
	LTIntPt     GetChooserSpacing(int nLayout);
	LTIntPt     GetChooserIconOffset(int nLayout);
	LTIntPt     GetChooserMargin(int nLayout);
	int         GetChooserTitleHeight(int nLayout);
	LTFLOAT     GetChooserIntroTime(int nLayout);
	LTFLOAT     GetChooserOutroTime(int nLayout);
	LTIntPt     GetProfilerPos(int nLayout);
	int         GetProfilerTextSize(int nLayout);

private:
	// RETAIL: +0x1160
	typedef std::vector<INT_MENUPIECE *> MenuPieceArray;
	MenuPieceArray m_MenuPieceArray;

	// RETAIL: +0x116c, created by the constructor (retail 0x100a0250)
	LCHUDHelper*	m_pLCHUDHelper;
};

#endif // _TRONLAYOUTMGR_H_