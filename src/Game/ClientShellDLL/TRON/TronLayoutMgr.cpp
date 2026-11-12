// ----------------------------------------------------------------------- //
//
// MODULE  : TronLayoutMgr.cpp
//
// PURPOSE : Attribute file manager for interface layout info
//			 Tron-specific functionality
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronLayoutMgr.h"
#include "LCHUDHelper.h"

#define LO_MENUPIECE_TAG			"MenuPiece"

#define	LO_MENUPIECE_NAME			"Name"
#define	LO_MENUPIECE_MOD			"Model"
#define	LO_MENUPIECE_SKIN			"Skin"
#define	LO_MENUPIECE_STYLE			"RenderStyle"
#define	LO_MENUPIECE_POS			"Pos"
#define LO_MENUPIECE_LOOPANIM		"LoopAnim"
#define LO_MENUPIECE_INTROANIM		"IntroAnim"
#define LO_MENUPIECE_PARENTPIECE	"ParentPiece"
#define LO_MENUPIECE_PARENTSOCKET	"ParentSocket"
#define LO_MENUPIECE_LAYER			"Layer"

#define LO_HUD_TAG					"HUDLayout"
#define LO_PROGRESSBOX_TAG			"ProgressBox"
#define LO_PROCEDURALHUD_TAG		"ProceduralHUD"
#define LO_MISC_TAG					"Miscellaneous"

// RETAIL: 0x1026eebc
static char s_aTagName[30];

CLayoutMgr* g_pLayoutMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	INT_MENUPIECE::Init
//
//	RETAIL: 0x100a00a0.  Tron's pieces have an intro animation, a parent
//	piece and socket and a layer instead of build 69's "Function".
//
// ----------------------------------------------------------------------- //

LTBOOL INT_MENUPIECE::Init(CButeMgr & buteMgr, char* aTagName)
{
	buteMgr.GetString(aTagName, LO_MENUPIECE_NAME, "", szName, sizeof(szName));
	if (strlen(szName) == 0) return LTFALSE;

	buteMgr.GetString(aTagName, LO_MENUPIECE_MOD, "", szModel, sizeof(szModel));
	if (strlen(szModel) == 0) return LTFALSE;

	buteMgr.GetString(aTagName, LO_MENUPIECE_LOOPANIM, "", szLoopAnim, sizeof(szLoopAnim));
	buteMgr.GetString(aTagName, LO_MENUPIECE_INTROANIM, "", szIntroAnim, sizeof(szIntroAnim));

	blrSkins.Read(&buteMgr, aTagName, LO_MENUPIECE_SKIN, 128);
	blrRenderStyles.Read(&buteMgr, aTagName, LO_MENUPIECE_STYLE, 128);

	buteMgr.GetString(aTagName, LO_MENUPIECE_PARENTPIECE, "", szParentPiece, sizeof(szParentPiece));
	buteMgr.GetString(aTagName, LO_MENUPIECE_PARENTSOCKET, "", szParentSocket, sizeof(szParentSocket));

    CAVector zero(0.0, 0.0, 0.0);
    vPos = buteMgr.GetVector(aTagName, LO_MENUPIECE_POS, zero);

	nLayer = buteMgr.GetInt(aTagName, LO_MENUPIECE_LAYER, 0);

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// RETAIL: 0x100a0250
CTronLayoutMgr::CTronLayoutMgr() : CLayoutMgr()
{
	g_pLayoutMgr = this;
	m_pLCHUDHelper = debug_new(LCHUDHelper);
}

// RETAIL: 0x100a0300
CTronLayoutMgr::~CTronLayoutMgr()
{
	if (m_pLCHUDHelper)
	{
		debug_delete(m_pLCHUDHelper);
		m_pLCHUDHelper = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronLayoutMgr::Init()
//
//	PURPOSE:	Init mgr
//
//	RETAIL: 0x100a03d0.  No "Subroutine" models in retail.
//
// ----------------------------------------------------------------------- //

LTBOOL CTronLayoutMgr::Init(const char* szAttributeFile)
{
	if (!CLayoutMgr::Init(szAttributeFile))
		return LTFALSE;

	m_pLCHUDHelper->Init();

	// Menu Pieces
	sprintf(s_aTagName, "%s0", LO_MENUPIECE_TAG);
	int numPieces = 0;
	while (m_buteMgr.Exist(s_aTagName))
	{
		INT_MENUPIECE *pPiece = debug_new(INT_MENUPIECE);
		if (pPiece->Init(m_buteMgr,s_aTagName))
			m_MenuPieceArray.push_back(pPiece);
		else
		{
			debug_delete(pPiece);
		}
		numPieces++;
		sprintf(s_aTagName, "%s%d", LO_MENUPIECE_TAG, numPieces);
	}

	return LTTRUE;
}

// RETAIL: 0x100a04e0
INT_MENUPIECE* CTronLayoutMgr::GetMenuPiece(char * szPieceName)
{
	int i = 0;
	while (i < (int)m_MenuPieceArray.size())
	{
		if (stricmp(szPieceName,m_MenuPieceArray[i]->szName) == 0)
			return m_MenuPieceArray[i];
		i++;
	}
	return LTNULL;
}

// TODO: retail 0x100a0550 builds an INT_MENUPIECE for a PlayerSubroutine
// ("sub:%s", its TronSubroutine's IconModel at +0xb0 and IconSkin at +0xf0)
// -- not written yet: our TronSubroutine lacks those members.

// RETAIL: the getters below are retail 0x100a0600..0x100a21f0, in order.

int CTronLayoutMgr::GetHealthEnergyFont(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthEnergyFont"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, "HealthEnergyFont", 0);
}

int CTronLayoutMgr::GetHealthEnergyFontSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthEnergyFontSize"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, "HealthEnergyFontSize", 16);
}

LTIntPt CTronLayoutMgr::GetInfoTextPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "InfoTextPos"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "InfoTextPos");
}

uint32 CTronLayoutMgr::GetInfoTextColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "InfoTextAlpha"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT fAlpha = GetFloat(s_aTagName, "InfoTextAlpha");
	uint8 nA = (uint8)(255.0f * fAlpha);

	LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "InfoTextColor"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, "InfoTextColor");

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

uint32 CTronLayoutMgr::GetInfoTextBadColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "InfoTextBadAlpha"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT fAlpha = GetFloat(s_aTagName, "InfoTextBadAlpha");
	uint8 nA = (uint8)(255.0f * fAlpha);

	LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "InfoTextBadColor"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, "InfoTextBadColor");

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

uint8 CTronLayoutMgr::GetInfoTextJustify(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "InfoTextJustify"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, "InfoTextJustify", 0);
}

uint8 CTronLayoutMgr::GetInfoTextSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "InfoTextSize"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, "InfoTextSize", 0);
}

uint16 CTronLayoutMgr::GetInfoTextWidth(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "InfoTextWidth"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "InfoTextWidth", 0);
}

LTIntPt CTronLayoutMgr::GetHealthBGOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthBGOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "HealthBGOffset");
}

LTIntPt CTronLayoutMgr::GetEnergyBGOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "EnergyBGOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "EnergyBGOffset");
}

LTIntPt CTronLayoutMgr::GetHealthFGOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthFGOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "HealthFGOffset");
}

LTIntPt CTronLayoutMgr::GetEnergyFGOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "EnergyFGOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "EnergyFGOffset");
}

uint32 CTronLayoutMgr::GetHealthBarWidth(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthBarWidth"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "HealthBarWidth", 0);
}

uint32 CTronLayoutMgr::GetEnergyBarWidth(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "EnergyBarWidth"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "EnergyBarWidth", 0);
}

uint32 CTronLayoutMgr::GetHealthBlinkTime(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthBlinkTime"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "HealthBlinkTime", 0);
}

LTRect CTronLayoutMgr::GetHealthBGTex(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthBGTex"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetRect(s_aTagName, "HealthBGTex");
}

LTRect CTronLayoutMgr::GetHealthTex(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthTex"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetRect(s_aTagName, "HealthTex");
}

LTRect CTronLayoutMgr::GetHealthBlinkTex(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthBlinkTex"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetRect(s_aTagName, "HealthBlinkTex");
}

LTRect CTronLayoutMgr::GetEnergyBGTex(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "EnergyBGTex"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetRect(s_aTagName, "EnergyBGTex");
}

LTRect CTronLayoutMgr::GetEnergyTex(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "EnergyTex"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetRect(s_aTagName, "EnergyTex");
}

LTRect CTronLayoutMgr::GetEnergyDeficitTex(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "EnergyDeficitTex"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetRect(s_aTagName, "EnergyDeficitTex");
}

LTIntPt CTronLayoutMgr::GetHealthTextOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "HealthTextOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "HealthTextOffset");
}

LTIntPt CTronLayoutMgr::GetEnergyTextOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "EnergyTextOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "EnergyTextOffset");
}

LTIntPt CTronLayoutMgr::GetArmorOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ArmorOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "ArmorOffset");
}

LTIntPt CTronLayoutMgr::GetPingIconSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PingIconSize"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "PingIconSize");
}

LTIntPt CTronLayoutMgr::GetPingStartPoint(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PingStartPoint"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "PingStartPoint");
}

uint32 CTronLayoutMgr::GetPingSpacing(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PingSpacing"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "PingSpacing", 0);
}

uint32 CTronLayoutMgr::GetPingFontNum(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PingFontNum"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "PingFontNum", 0);
}

uint32 CTronLayoutMgr::GetPingFontSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PingFontSize"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "PingFontSize", 0);
}

int CTronLayoutMgr::GetPingBlinkCount(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PingBlinkCount"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, "PingBlinkCount", 0);
}

LTFLOAT CTronLayoutMgr::GetPingBlinkTime(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PingBlinkTime"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, "PingBlinkTime", 0.0f);
}

LTIntPt CTronLayoutMgr::GetPsetOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PsetOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "PsetOffset");
}

uint32 CTronLayoutMgr::GetPsetWidth(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PsetWidth"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "PsetWidth", 0);
}

uint32 CTronLayoutMgr::GetPsetHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "PsetHeight"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, "PsetHeight", 0);
}

LTRect CTronLayoutMgr::GetProgressBoxRect(char *pAttribute)
{
	if (!m_buteMgr.Exist(LO_PROGRESSBOX_TAG, pAttribute))
		sprintf(s_aTagName, "%s0", pAttribute);

	return GetRect(LO_PROGRESSBOX_TAG, pAttribute);
}

int CTronLayoutMgr::GetProgressHUDOffset()
{
	return GetInt(LO_PROGRESSBOX_TAG, "HUDOffset", 64);
}

int CTronLayoutMgr::GetProgressTitleFont()
{
	return GetInt(LO_PROGRESSBOX_TAG, "TitleFont", 0);
}

int CTronLayoutMgr::GetProgressTitleFontSize()
{
	return GetInt(LO_PROGRESSBOX_TAG, "TitleFontSize", 16);
}

uint32 CTronLayoutMgr::GetProgressTitleColor()
{
	LTFLOAT fAlpha = GetFloat(LO_PROGRESSBOX_TAG, "TitleAlpha");
	uint8 nA = (uint8)(255.0f * fAlpha);

	LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, "TitleColor");

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

int CTronLayoutMgr::GetProgressObjectiveFont()
{
	return GetInt(LO_PROGRESSBOX_TAG, "ObjectiveFont", 0);
}

int CTronLayoutMgr::GetProgressObjectiveFontSize()
{
	return GetInt(LO_PROGRESSBOX_TAG, "ObjectiveFontSize", 16);
}

int CTronLayoutMgr::GetProgressObjectiveSpacing()
{
	return GetInt(LO_PROGRESSBOX_TAG, "ObjectiveSpacing", 8);
}

uint32 CTronLayoutMgr::GetProgressObjectiveColor()
{
	LTFLOAT fAlpha = GetFloat(LO_PROGRESSBOX_TAG, "ObjectiveAlpha");
	uint8 nA = (uint8)(255.0f * fAlpha);

	LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, "ObjectiveColor");

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

uint32 CTronLayoutMgr::GetProgressCompletedObjectiveColor()
{
	LTFLOAT fAlpha = GetFloat(LO_PROGRESSBOX_TAG, "CompletedObjectiveAlpha");
	uint8 nA = (uint8)(255.0f * fAlpha);

	LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, "CompletedObjectiveColor");

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

uint32 CTronLayoutMgr::GetProgressSecondaryObjectiveColor()
{
	LTFLOAT fAlpha = GetFloat(LO_PROGRESSBOX_TAG, "SecondaryObjectiveAlpha");
	uint8 nA = (uint8)(255.0f * fAlpha);

	LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, "SecondaryObjectiveColor");

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

int CTronLayoutMgr::GetProgressProceduralFont()
{
	return GetInt(LO_PROCEDURALHUD_TAG, "FontNum", 0);
}

int CTronLayoutMgr::GetProgressProceduralFontSize()
{
	return GetInt(LO_PROCEDURALHUD_TAG, "FontSize", 16);
}

uint32 CTronLayoutMgr::GetProgressProceduralColor()
{
	LTFLOAT fAlpha = GetFloat(LO_PROCEDURALHUD_TAG, "Alpha");
	uint8 nA = (uint8)(255.0f * fAlpha);

	LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROCEDURALHUD_TAG, "Color");

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

int CTronLayoutMgr::GetProgressProceduralHeight()
{
	return GetInt(LO_PROCEDURALHUD_TAG, "Height", 64);
}

int CTronLayoutMgr::GetProgressProceduralSpacing()
{
	return GetInt(LO_PROCEDURALHUD_TAG, "Spacing", 64);
}

LTRect CTronLayoutMgr::GetIOCameraRect()
{
	return GetRect(LO_MISC_TAG, "IOCameraRect");
}

LTFLOAT CTronLayoutMgr::GetIOCameraFOVX()
{
	return GetFloat(LO_MISC_TAG, "IOCameraFOVX", 90.0f);
}

LTFLOAT CTronLayoutMgr::GetIOCameraFOVY()
{
	return GetFloat(LO_MISC_TAG, "IOCameraFOVY", 78.0f);
}

LTFLOAT CTronLayoutMgr::GetWeaponIndicatorAnimTime(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "WeaponIndicatorAnimTime"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, "WeaponIndicatorAnimTime", 0.25f);
}

LTIntPt CTronLayoutMgr::GetWeaponIndicatorOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "WeaponIndicatorOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "WeaponIndicatorOffset");
}

LTIntPt CTronLayoutMgr::GetWeaponIndicatorIconOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "WeaponIndicatorIconOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "WeaponIndicatorIconOffset");
}

LTIntPt CTronLayoutMgr::GetChooserBoxSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ChooserBoxSize"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "ChooserBoxSize");
}

LTIntPt CTronLayoutMgr::GetChooserSpacing(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ChooserSpacing"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "ChooserSpacing");
}

LTIntPt CTronLayoutMgr::GetChooserIconOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ChooserIconOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "ChooserIconOffset");
}

LTIntPt CTronLayoutMgr::GetChooserMargin(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ChooserMargin"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "ChooserMargin");
}

int CTronLayoutMgr::GetChooserTitleHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ChooserTitleHeight"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, "ChooserTitleHeight", 0);
}

LTFLOAT CTronLayoutMgr::GetChooserIntroTime(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ChooserIntroTime"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, "ChooserIntroTime", 0.0f);
}

LTFLOAT CTronLayoutMgr::GetChooserOutroTime(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ChooserOutroTime"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, "ChooserOutroTime", 0.0f);
}

LTIntPt CTronLayoutMgr::GetProfilerPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ProfilerPos"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, "ProfilerPos");
}

int CTronLayoutMgr::GetProfilerTextSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName, "ProfilerTextSize"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, "ProfilerTextSize", 0);
}
