// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCyclePlayerConfig.cpp
//
// PURPOSE : Tron's single player light cycle screen: the player's cycle
//			 (its shade and its style), its model and its statistics
//
// RETAIL: written from retail CShell.dll 0x1006bf80..0x1006de20 (the
// function and member names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenLightCyclePlayerConfig.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ProfileMgr.h"
#include "LightCycleGameDataMgr.h"
#include "TRON/LightCycleButeMgr.h"
#include "GameClientShell.h"

// RETAIL: 0x1026ea38, 0x1026ea28: the model's scale for the regular and
// the super cycle (initialized by 0x1006bfb0, 0x1006c000)
static LTVector s_vRegularScale = LTVector(0.035f, 0.035f, 0.035f);
static LTVector s_vSuperScale = LTVector(0.025f, 0.025f, 0.025f);

namespace
{
	// RETAIL: Tron's command IDs (the names are ours)
	const uint32 kCmdLCUpdateCycle	= 0x43D;
	const uint32 kCmdLCShadeLeft	= 0x43E;
	const uint32 kCmdLCShadeRight	= 0x43F;
	const uint32 kCmdLCStyleLeft	= 0x440;
	const uint32 kCmdLCStyleRight	= 0x441;

	// RETAIL: 0x10219b08, 0x10219b0c: the bars show a statistic from
	// s_nStatMin to s_nStatMin + s_nStatRange (variables)
	int		s_nStatMin = 20;
	int		s_nStatRange = 100;

	// RETAIL: 0x10219b10: the model's turn rate (a variable)
	float	s_fRotationRate = 1.0f;

	// RETAIL: 0x1026ea50: the console's DynamicLight while the screen shows
	int		s_nDynamicLight;

	// RETAIL: 0x1026ea54: the header width of the bars (never set: 0)
	int		s_nStatHeaderWidth;
}

// RETAIL: 0x1006c040
CScreenLightCyclePlayerConfig::CScreenLightCyclePlayerConfig() :
	m_nShade(0),
	m_pShadeText(LTNULL),
	m_pShadeLeft(LTNULL),
	m_pShadeRight(LTNULL),
	m_nStyle(0),
	m_pStyleText(LTNULL),
	m_pStyleLeft(LTNULL),
	m_pStyleRight(LTNULL),
	m_pLabel(LTNULL),
	m_pStatsLabel(LTNULL),
	m_xTronUnknownA50(0),
	m_fRotation(90.0f),
	m_fLastTime(0.0f),
	m_pTurboSpeedText(LTNULL),
	m_pTurboSpeed(LTNULL),
	m_pMaxSpeedText(LTNULL),
	m_pMaxSpeed(LTNULL),
	m_pMinSpeedText(LTNULL),
	m_pMinSpeed(LTNULL),
	m_pTurboAccelText(LTNULL),
	m_pTurboAccel(LTNULL),
	m_pTurnLossText(LTNULL),
	m_pTurnLoss(LTNULL),
	m_fMinTurboSpeed(1.0f),
	m_fMaxTurboSpeed(1.0f),
	m_fMinMaxSpeed(1.0f),
	m_fMaxMaxSpeed(1.0f),
	m_fMinMinSpeed(1.0f),
	m_fMaxMinSpeed(1.0f),
	m_fMinTurboAccel(1.0f),
	m_fMaxTurboAccel(1.0f),
	m_fMinTurnLoss(1.0f),
	m_fMaxTurnLoss(1.0f)
{
}

// RETAIL: 0x1006c170
CScreenLightCyclePlayerConfig::~CScreenLightCyclePlayerConfig()
{
}

// RETAIL: 0x1006c180
LTBOOL CScreenLightCyclePlayerConfig::Build()
{
	LTBOOL bRet = CBaseScreenLightCycle::Build();

	m_pLabel = AddTextItem(0x25CD,LTNULL,LTNULL,g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LabelPos"));
	m_pLabel->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_pLabel->Enable(LTFALSE);

	// RETAIL: read and not used
	LTIntPt cmdPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"CommandPos");
	int nColumnWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_BASE_LIGHTCYCLE,"ColumnWidth");

	HTEXTURE hLeft = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowlt.dtx");
	HTEXTURE hLeftH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowlt_h.dtx");
	HTEXTURE hRight = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowrt.dtx");
	HTEXTURE hRightH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowrt_h.dtx");

	int nBarWidth = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"BarWidth");

	// the shade
	m_pShadeText = AddTextItem(0x25CA,LTNULL,0x25CB,g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"ShadeTextPos"));
	m_pShadeText->SetColors(argbWhite,argbWhite,argbWhite);
	m_pShadeText->Enable(LTFALSE);

	m_pShadeLeft = debug_new(CLTGUIButton);
	if (m_pShadeLeft)
	{
		LTIntPt arrowPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"ShadeLeftArrowPos");
		m_pShadeLeft->Create(kCmdLCShadeLeft,LTNULL,hLeft,hLeftH);
		m_pShadeLeft->SetBasePos(arrowPos);
		AddControl(m_pShadeLeft);
		m_pShadeLeft->SetCommandHandler(this);
	}

	m_pShadeRight = debug_new(CLTGUIButton);
	if (m_pShadeRight)
	{
		LTIntPt arrowPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"ShadeRightArrowPos");
		m_pShadeRight->Create(kCmdLCShadeRight,LTNULL,hRight,hRightH);
		m_pShadeRight->SetBasePos(arrowPos);
		AddControl(m_pShadeRight);
		m_pShadeRight->SetCommandHandler(this);
	}

	// the style
	m_pStyleText = AddTextItem(0x25C8,LTNULL,0x25C9,g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"TypeTextPos"));
	m_pStyleText->SetColors(argbWhite,argbWhite,argbWhite);
	m_pStyleText->Enable(LTFALSE);

	m_pStyleLeft = debug_new(CLTGUIButton);
	if (m_pStyleLeft)
	{
		LTIntPt arrowPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"TypeLeftArrowPos");
		m_pStyleLeft->Create(kCmdLCStyleLeft,LTNULL,hLeft,hLeftH);
		m_pStyleLeft->SetBasePos(arrowPos);
		AddControl(m_pStyleLeft);
		m_pStyleLeft->SetCommandHandler(this);
	}

	m_pStyleRight = debug_new(CLTGUIButton);
	if (m_pStyleRight)
	{
		LTIntPt arrowPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"TypeRightArrowPos");
		m_pStyleRight->Create(kCmdLCStyleRight,LTNULL,hRight,hRightH);
		m_pStyleRight->SetBasePos(arrowPos);
		AddControl(m_pStyleRight);
		m_pStyleRight->SetCommandHandler(this);
	}

	// the statistics
	m_pStatsLabel = AddTextItem(0x25CD,LTNULL,LTNULL,g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"StatsPos"));
	m_pStatsLabel->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_pStatsLabel->Enable(LTFALSE);

	m_pTurboSpeedText = AddTextItem(0x25CE,LTNULL,0x25CF);
	m_pTurboSpeedText->SetColors(argbWhite,argbWhite,argbWhite);
	m_pTurboSpeed = AddSlider(0,0x25CF,s_nStatHeaderWidth,nBarWidth,-1,LTNULL);
	m_pTurboSpeed->SetSliderRange(0,s_nStatMin + s_nStatRange);
	m_pTurboSpeed->SetSliderIncrement(1);
	m_pTurboSpeed->SetDisplayOnly(true);

	m_pMaxSpeedText = AddTextItem(0x25D0,LTNULL,0x25D1);
	m_pMaxSpeedText->SetColors(argbWhite,argbWhite,argbWhite);
	m_pMaxSpeed = AddSlider(0,0x25D1,s_nStatHeaderWidth,nBarWidth,-1,LTNULL);
	m_pMaxSpeed->SetSliderRange(0,s_nStatMin + s_nStatRange);
	m_pMaxSpeed->SetSliderIncrement(1);
	m_pMaxSpeed->SetDisplayOnly(true);

	m_pMinSpeedText = AddTextItem(0x25D2,LTNULL,0x25D3);
	m_pMinSpeedText->SetColors(argbWhite,argbWhite,argbWhite);
	m_pMinSpeed = AddSlider(0,0x25D3,s_nStatHeaderWidth,nBarWidth,-1,LTNULL);
	m_pMinSpeed->SetSliderRange(0,s_nStatRange + s_nStatMin);
	m_pMinSpeed->SetSliderIncrement(1);
	m_pMinSpeed->SetDisplayOnly(true);

	m_pTurboAccelText = AddTextItem(0x25D6,LTNULL,0x25D7);
	m_pTurboAccelText->SetColors(argbWhite,argbWhite,argbWhite);
	m_pTurboAccel = AddSlider(0,0x25D7,s_nStatHeaderWidth,nBarWidth,-1,LTNULL);
	m_pTurboAccel->SetSliderRange(0,s_nStatRange + s_nStatMin);
	m_pTurboAccel->SetSliderIncrement(1);
	m_pTurboAccel->SetDisplayOnly(true);

	m_pTurnLossText = AddTextItem(0x25D8,LTNULL,0x25D9);
	m_pTurnLossText->SetColors(argbWhite,argbWhite,argbWhite);
	m_pTurnLoss = AddSlider(0,0x25D9,s_nStatHeaderWidth,nBarWidth,-1,LTNULL);
	m_pTurnLoss->SetSliderRange(0,s_nStatMin + s_nStatRange);
	m_pTurnLoss->SetSliderIncrement(1);
	m_pTurnLoss->SetDisplayOnly(true);

	// the background behind the model
	LTIntPt bgPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LightCycleBackgroundPos");
	LTRect rcBg = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"LightCycleBackgroundRect");
	g_pDrawPrim->SetXYWH(&m_BackgroundPoly,(float)bgPos.x * g_pInterfaceResMgr->GetXRatio(),(float)bgPos.y * g_pInterfaceResMgr->GetXRatio(),
		(float)(rcBg.right - rcBg.left) * g_pInterfaceResMgr->GetXRatio(),(float)(rcBg.bottom - rcBg.top) * g_pInterfaceResMgr->GetXRatio());
	g_pDrawPrim->SetUVWH(&m_BackgroundPoly,0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_BackgroundPoly,argbWhite);

	char szTexture[64];
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,"LightCycleBackgroundTextureName",szTexture,sizeof(szTexture));
	m_hBackground = g_pInterfaceResMgr->GetTexture(szTexture);

	UpdateStatRanges();

	return bRet;
}

// RETAIL: 0x1006ca70
void CScreenLightCyclePlayerConfig::ScreenDimsChanged()
{
	LTIntPt bgPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LightCycleBackgroundPos");
	LTRect rcBg = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"LightCycleBackgroundRect");
	g_pDrawPrim->SetXYWH(&m_BackgroundPoly,g_pInterfaceResMgr->GetXRatio() * bgPos.x,g_pInterfaceResMgr->GetXRatio() * bgPos.y,
		(float)(rcBg.right - rcBg.left) * g_pInterfaceResMgr->GetXRatio(),(float)(rcBg.bottom - rcBg.top) * g_pInterfaceResMgr->GetXRatio());
	g_pDrawPrim->SetUVWH(&m_BackgroundPoly,0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_BackgroundPoly,argbWhite);

	CBaseScreenLightCycle::ScreenDimsChanged();
}

// RETAIL: 0x1006cb70
uint32 CScreenLightCyclePlayerConfig::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}

	switch (dwCommand)
	{
	// the previous shade a single player game may use
	case kCmdLCShadeLeft:
		{
			uint8 nOld = m_nShade;
			bool bFound = false;
			do
			{
				if (!m_nShade)
					m_nShade = g_pLightCycleButeMgr->GetNumShades();
				m_nShade--;
				if (!g_pLightCycleButeMgr->GetShade(m_nShade)->bMultiOnly)
					bFound = true;
			} while (nOld != m_nShade && !bFound);
		}
		break;

	// the next shade a single player game may use
	case kCmdLCShadeRight:
		{
			uint8 nOld = m_nShade;
			bool bFound = false;
			do
			{
				m_nShade++;
				if (m_nShade == g_pLightCycleButeMgr->GetNumShades())
					m_nShade = 0;
				if (!g_pLightCycleButeMgr->GetShade(m_nShade)->bMultiOnly)
					bFound = true;
			} while (nOld != m_nShade && !bFound);
		}
		// fall through
	case kCmdLCUpdateCycle:
		UpdateData(LTTRUE);
		UpdateCycle();
		return 1;

	// RETAIL: the two cases test apart (no range test), one body
	case kCmdLCStyleLeft:
		m_nStyle = (m_nStyle != 1);
		UpdateData(LTTRUE);
		UpdateCycle();
		return 1;

	case kCmdLCStyleRight:
		m_nStyle = (m_nStyle != 1);
		UpdateData(LTTRUE);
		UpdateCycle();
		return 1;

	case CMD_BACK:
		m_pScreenMgr->EscapeCurrentScreen();
		return 1;

	default:
		return CBaseScreenLightCycle::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	UpdateData(LTTRUE);
	UpdateCycle();
	return 1;
}

// RETAIL: 0x1006cd10
void CScreenLightCyclePlayerConfig::UpdateCycle()
{
	HOBJECT hChar = m_CharSFX.GetObject();
	if (!hChar) return;

	LIGHTCYCLECOLOR *pColor = g_pLightCycleButeMgr->GetShadeCycleColor(m_nStyle,m_nShade);
	if (!pColor) return;

	if (m_nShade >= g_pLightCycleButeMgr->GetNumShades()) return;

	LIGHTCYCLESHADE *pShade = g_pLightCycleButeMgr->GetShade(m_nShade);
	if (pShade->bMultiOnly) return;

	int nShadeId = IsCycleColorLocked(pColor->szName) ? pShade->nLockedNameId : pShade->nNameId;
	m_pShadeText->SetString(LoadTempWString(nShadeId));

	char szShade[128] = "";
	char szStyle[128] = "";
	LoadString(nShadeId,szShade,sizeof(szShade));
	LoadString(m_nStyle ? 0x259B : 0x259A,szStyle,sizeof(szStyle));

	char szCycle[256] = "";
	FormatString(0x2621,szCycle,sizeof(szCycle),szStyle,szShade);

	std::wstring sCycle;
	LTGUIWiden(sCycle,szCycle);
	m_pCycleName->GetColumn(1)->SetString(sCycle);

	if (!m_nStyle)
	{
		m_pStyleText->SetString(LoadTempWString(0x259A));
		g_pLTClient->SetObjectScale(m_CharSFX.GetObject(),&s_vRegularScale);
	}
	else
	{
		m_pStyleText->SetString(LoadTempWString(0x259B));
		g_pLTClient->SetObjectScale(m_CharSFX.GetObject(),&s_vSuperScale);
	}

	if (!IsCycleColorLocked(pColor->szName))
	{
		float fRatio;
		fRatio = (g_pLightCycleButeMgr->GetMaxTurboSpeed(pColor) - m_fMinTurboSpeed) / (m_fMaxTurboSpeed - m_fMinTurboSpeed);
		m_pTurboSpeed->SetSliderPos((int)(fRatio * (float)s_nStatRange) + s_nStatMin);
		fRatio = (g_pLightCycleButeMgr->GetMaxSpeed(pColor) - m_fMinMaxSpeed) / (m_fMaxMaxSpeed - m_fMinMaxSpeed);
		m_pMaxSpeed->SetSliderPos((int)(fRatio * (float)s_nStatRange) + s_nStatMin);
		fRatio = (g_pLightCycleButeMgr->GetMinSpeed(pColor) - m_fMinMinSpeed) / (m_fMaxMinSpeed - m_fMinMinSpeed);
		m_pMinSpeed->SetSliderPos((int)(fRatio * (float)s_nStatRange) + s_nStatMin);
		fRatio = (g_pLightCycleButeMgr->GetTurboAccelRate(pColor) - m_fMinTurboAccel) / (m_fMaxTurboAccel - m_fMinTurboAccel);
		m_pTurboAccel->SetSliderPos((int)(fRatio * (float)s_nStatRange) + s_nStatMin);
		fRatio = (g_pLightCycleButeMgr->GetTurnSpeedLossPercent(pColor) - m_fMinTurnLoss) / (m_fMaxTurnLoss - m_fMinTurnLoss);
		m_pTurnLoss->SetSliderPos((int)(fRatio * (float)s_nStatRange) + s_nStatMin);

		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		SAFE_STRCPY(createStruct.m_Filename,pColor->szModel);
		pColor->blrRenderStyles.CopyList(0,createStruct.m_RenderStyleNames[0],MAX_CS_FILENAME_LEN+1);
		pColor->blrSkins.CopyList(0,createStruct.m_SkinNames[0],MAX_CS_FILENAME_LEN+1);
		g_pCommonLT->SetObjectFilenames(hChar,&createStruct);
	}
	else
	{
		// the cycle is not unlocked yet: an unknown cycle, no statistics
		g_pLTClient->SetObjectScale(m_CharSFX.GetObject(),&s_vRegularScale);
		pColor = g_pLightCycleButeMgr->GetCycleColor("UnknownCycle");

		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		SAFE_STRCPY(createStruct.m_Filename,pColor->szModel);
		pColor->blrRenderStyles.CopyList(0,createStruct.m_RenderStyleNames[0],MAX_CS_FILENAME_LEN+1);
		pColor->blrSkins.CopyList(0,createStruct.m_SkinNames[0],MAX_CS_FILENAME_LEN+1);
		g_pCommonLT->SetObjectFilenames(hChar,&createStruct);

		m_pTurboSpeed->SetSliderPos(0);
		m_pMaxSpeed->SetSliderPos(0);
		m_pMinSpeed->SetSliderPos(0);
		m_pTurboAccel->SetSliderPos(0);
		m_pTurnLoss->SetSliderPos(0);
	}
}

// RETAIL: 0x1006d630
void CScreenLightCyclePlayerConfig::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		s_nDynamicLight = GetConsoleInt("DynamicLight",0);
		WriteConsoleInt("DynamicLight",1);
		UpdateData(LTFALSE);
	}
	else
	{
		WriteConsoleInt("DynamicLight",s_nDynamicLight);
		UpdateData(LTTRUE);
	}

	CBaseScreenLightCycle::OnFocus(bFocus);

	if (bFocus)
	{
		m_fRotation = 90.0f;
		m_fLastTime = g_pLTClient->GetTime();

		HOBJECT hChar = m_CharSFX.GetObject();
		if (hChar)
		{
			LTRotation rRot;
			rRot.Init();
			rRot.Rotate(LTVector(0.0f, 1.0f, 0.0f), m_fRotation);
			g_pLTClient->SetObjectRotation(hChar, &rRot);
		}

		UpdateCycle();
	}
}

// RETAIL: 0x1006d7d0: the profile's cycle
void CScreenLightCyclePlayerConfig::LoadSettings()
{
	CBaseScreenLightCycle::LoadSettings();

	if (g_pProfileMgr)
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		m_nStyle = pProfile->m_nLightCycleStyle;
		m_nShade = pProfile->m_nLightCycleShade;
		UpdateCycle();
		UpdateData(LTFALSE);
	}
}

// RETAIL: 0x1006d810
void CScreenLightCyclePlayerConfig::SaveSettings()
{
	UpdateData(LTTRUE);

	if (g_pProfileMgr)
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		pProfile->m_nLightCycleStyle = m_nStyle;
		pProfile->m_nLightCycleShade = m_nShade;
		g_pLightCycleGameDataMgr->SaveCurrent();
		CBaseScreenLightCycle::SaveSettings();
	}
}

// RETAIL: 0x1006d8b0: the background, and the model turning in front of it
LTBOOL CScreenLightCyclePlayerConfig::Render(HSURFACE hDestSurf)
{
	LTBOOL bRet = CBaseScreenLightCycle::Render(hDestSurf);

	if (m_hBackground)
	{
		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_DECAL);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_NOBLEND);
		g_pDrawPrim->SetTexture(m_hBackground);
		g_pDrawPrim->DrawPrim(&m_BackgroundPoly,1);
	}

	HOBJECT hChar = m_CharSFX.GetObject();
	if (hChar)
	{
		LTFLOAT fTime = g_pLTClient->GetTime();
		m_fRotation += (fTime - m_fLastTime) * s_fRotationRate;
		m_fLastTime = fTime;

		LTRotation rRot;
		rRot.Init();
		rRot.Rotate(LTVector(0.0f, 1.0f, 0.0f), m_fRotation);
		g_pLTClient->SetObjectRotation(hChar, &rRot);

		// the model is drawn over the background (it is hidden otherwise)
		g_pLTClient->Start3D();
		g_pCommonLT->SetObjectFlags(hChar, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		// PORT: retail calls a Tron CInterfaceMgr function here (0x100cea40,
		// not yet written: it renders the object with the interface lights
		// through the interface camera); RenderObjects stands in
		{
			HOBJECT hObjs[1] = { hChar };
			g_pLTClient->RenderObjects(g_pInterfaceMgr->GetInterfaceCamera(), hObjs, 1, g_pLTClient->GetFrameTime());
		}
		g_pCommonLT->SetObjectFlags(hChar, OFT_Flags, 0, FLAG_VISIBLE);
		g_pLTClient->End3D();
	}

	return bRet;
}

// RETAIL: 0x1006dad0
void CScreenLightCyclePlayerConfig::UpdateStatRanges()
{
	m_fMinTurboSpeed = FLT_MAX;
	m_fMaxTurboSpeed = 0.0f;
	m_fMinMaxSpeed = FLT_MAX;
	m_fMaxMaxSpeed = 0.0f;
	m_fMinMinSpeed = FLT_MAX;
	m_fMaxMinSpeed = 0.0f;
	m_fMinTurboAccel = FLT_MAX;
	m_fMaxTurboAccel = 0.0f;
	m_fMinTurnLoss = FLT_MAX;
	m_fMaxTurnLoss = 0.0f;

	int nNumShades = g_pLightCycleButeMgr->GetNumShades();
	for (int nStyle = 0; nStyle < 2; nStyle++)
	{
		for (int nShade = 0; nShade < nNumShades; nShade++)
		{
			LIGHTCYCLECOLOR *pColor = g_pLightCycleButeMgr->GetShadeCycleColor(nStyle,nShade);
			if (!pColor) continue;

			LTFLOAT fValue = g_pLightCycleButeMgr->GetMaxTurboSpeed(pColor);
			if (fValue < m_fMinTurboSpeed) m_fMinTurboSpeed = fValue;
			if (fValue > m_fMaxTurboSpeed) m_fMaxTurboSpeed = fValue;

			fValue = g_pLightCycleButeMgr->GetMaxSpeed(pColor);
			if (fValue < m_fMinMaxSpeed) m_fMinMaxSpeed = fValue;
			if (fValue > m_fMaxMaxSpeed) m_fMaxMaxSpeed = fValue;

			fValue = g_pLightCycleButeMgr->GetMinSpeed(pColor);
			if (fValue < m_fMinMinSpeed) m_fMinMinSpeed = fValue;
			if (fValue > m_fMaxMinSpeed) m_fMaxMinSpeed = fValue;

			fValue = g_pLightCycleButeMgr->GetTurboAccelRate(pColor);
			if (fValue < m_fMinTurboAccel) m_fMinTurboAccel = fValue;
			if (fValue > m_fMaxTurboAccel) m_fMaxTurboAccel = fValue;

			fValue = g_pLightCycleButeMgr->GetTurnSpeedLossPercent(pColor);
			if (fValue < m_fMinTurnLoss) m_fMinTurnLoss = fValue;
			if (fValue > m_fMaxTurnLoss) m_fMaxTurnLoss = fValue;
		}
	}

	// a statistic all the cycles share gets a range below it
	if (m_fMaxTurboSpeed - m_fMinTurboSpeed == 0.0f)
		m_fMinTurboSpeed = std::max(0.0f, m_fMinTurboSpeed - 1.0f);
	if (m_fMaxMaxSpeed - m_fMinMaxSpeed == 0.0f)
		m_fMinMaxSpeed = std::max(0.0f, m_fMinMaxSpeed - 1.0f);
	if (m_fMaxMinSpeed - m_fMinMinSpeed == 0.0f)
		m_fMinMinSpeed = std::max(0.0f, m_fMinMinSpeed - 1.0f);
	if (m_fMaxTurboAccel - m_fMinTurboAccel == 0.0f)
		m_fMinTurboAccel = std::max(0.0f, m_fMinTurboAccel - 1.0f);
	if (m_fMaxTurnLoss - m_fMinTurnLoss == 0.0f)
		m_fMinTurnLoss = std::max(0.0f, m_fMinTurnLoss - 1.0f);
}

// RETAIL: 0x1006d860 (slot 39: folded with CScreenLightCycleGame's)
LTRect CScreenLightCyclePlayerConfig::GetBodyRect()
{
	return g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"TabBodyRect");
}
