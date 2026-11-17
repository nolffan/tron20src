// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSubroutines.cpp
//
// PURPOSE : Screen for managing player's inventory of "subroutines"
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// RETAIL: Tron 2.0 1.030's, written from retail CShell.dll
// (0x1007d090..0x10081e25), in retail's order. The names of the functions
// that have no counterpart in Build 69 are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenSubroutines.h"
#include "ScreenMgr.h"
#include "LayoutMgr.h"
#include "ScreenCommands.h"
#include "WinUtil.h"
#include "GameClientShell.h"
#include "SubroutineMgr.h"
#include "TRONPlayerStats.h"
#include "TransitionFXMgr.h"
#include "CursorMgr.h"
#include "ProfileMgr.h"
#include "MissionButeMgr.h"
#include "ClientUtilities.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
	// RETAIL: the screen's commands (0x1007ef90)
	const uint32 CMD_DONE			= 0x434;
	const uint32 CMD_CLEAR			= 0x435;
	const uint32 CMD_TOOLTIPS		= 0x436;

	const float MEMORY_INNER_RADIUS = 118;
	const float MEMORY_OUTER_RADIUS = 166;

	// RETAIL: the screen's sounds, 0x1007d090..0x1007d3d2 (names ours).
	void PlaySubroutineStartSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("subroutinestart", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
	}

	void PlayCompileSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("compile", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
	}

	void PlayPickupSound(PlayerSubroutine * pSub)
	{
		if (pSub->eState == SUBSTATE_CORRUPT)
		{
			g_pClientSoundMgr->PlaySoundLocal("pickup_corrupt", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
			return;
		}
		if (pSub->eState == SUBSTATE_FOREIGN)
		{
			g_pClientSoundMgr->PlaySoundLocal("pickup_foreign", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
			return;
		}

		SubFunction eFunction = pSub->pTronSubroutine->eFunction;
		if (eFunction == FUNCTION_BADBLOCK)
		{
			g_pClientSoundMgr->PlaySoundLocal("pickup_badblock", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
			return;
		}
		if (eFunction != FUNCTION_BASECODE)
		{
			g_pClientSoundMgr->PlaySoundLocal("pickup_subroutine", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
		}
	}

	void PlayMouseOverSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("mouseover", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayMouseOverSound(PlayerSubroutine * pSub)
	{
		if (pSub->eState == SUBSTATE_CORRUPT)
		{
			g_pClientSoundMgr->PlaySoundLocal("mouseover_corrupt", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
			return;
		}
		if (pSub->eState == SUBSTATE_FOREIGN)
		{
			g_pClientSoundMgr->PlaySoundLocal("mouseover_foreign", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
			return;
		}

		SubFunction eFunction = pSub->pTronSubroutine->eFunction;
		if (eFunction == FUNCTION_BADBLOCK)
		{
			g_pClientSoundMgr->PlaySoundLocal("mouseover_badblock", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
			return;
		}
		if (eFunction != FUNCTION_BASECODE)
		{
			PlayMouseOverSound();
		}
	}

	void PlayClearMemSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("clearmem", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayToggleToolTipsSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("toggletooltips", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayDropIntoSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("dropinto", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayDropIntoProceduralSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("dropintoprocedural", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayDropIntoLibrarySound()
	{
		g_pClientSoundMgr->PlaySoundLocal("dropintolibrary", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayCantDropSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("cantdrop", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayActiveMemoryYesSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("mouseover_activememory_yes", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayActiveMemoryNoSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("mouseover_activememory_no", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSubroutines::CScreenSubroutines()
{
	m_pSysName = LTNULL;
	m_pVersion = LTNULL;
	m_bShowToolTips = LTTRUE;
	m_nToolTipFontSize = 14;
	m_pBuildNoteCount = LTNULL;
	m_nBuildNoteCountStringId = 0;
	m_nPopupTextId = 0;
	m_nPopupId = 0;
	m_szPopupText[0] = 0;
	m_dwInterfaceSoundMask = 0;
	m_hAnimSound = LTNULL;
	m_pLastCtrl = LTNULL;
	m_bRatingLeftHot = false;
	m_bRatingRightHot = false;
	m_bLibraryCCWHot = false;
	m_bLibraryCWHot = false;
}

CScreenSubroutines::~CScreenSubroutines()
{
}

// RETAIL: 0x1007d800
void CScreenSubroutines::Term()
{
	if (m_hAnimSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hAnimSound);
		m_hAnimSound = LTNULL;
	}

	m_PopupText.Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::Build()
//
//	PURPOSE:	Build any data structures one time
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenSubroutines::Build()
{
	LTIntPt clearPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_SUBROUTINES, "ClearPos");
	LTIntPt sysNamePos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_SUBROUTINES, "SysNamePos");
	LTIntPt titlePos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_SUBROUTINES, "ToolTipTitlePos");
	LTIntPt togglePos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_SUBROUTINES, "ToolTipTogglePos");
	m_nToolTipFontSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SUBROUTINES, "ToolTipFontSize");

	LTVector vColor = g_pLayoutMgr->GetVector("ScreenSubroutines", "ToolTipColor");
	m_ToolTip.SetTextColor(SET_ARGB(0xFF, (uint8)vColor.x, (uint8)vColor.y, (uint8)vColor.z));

	vColor = g_pLayoutMgr->GetVector("ScreenSubroutines", "ToolTipFrameColor");
	uint8 nRed = (uint8)vColor.x;
	uint8 nGreen = (uint8)vColor.y;
	uint8 nBlue = (uint8)vColor.z;
	uint8 nAlpha = LTCLAMP((uint8)(g_pLayoutMgr->GetFloat("ScreenSubroutines", "ToolTipAlpha", 0.0f) * 255.0f), 0, 255);
	m_ToolTip.SetBackColor(SET_ARGB(nAlpha, nRed, nGreen, nBlue));

	m_pClear = AddTextItem(0xcb2, CMD_CLEAR, 0, clearPos);

	if (g_pProfileMgr)
		m_bShowToolTips = g_pProfileMgr->GetCurrentProfile()->m_bAlwaysShowToolTip;

	m_pToolTipTitle = AddTextItem(0xc9e, 0, 0, titlePos);
	m_pToolTipTitle->Enable(LTFALSE);

	m_pToolTipToggle = AddToggle(L"", 0, 0, &m_bShowToolTips, togglePos);
	m_pToolTipToggle->NotifyOnChange(CMD_TOOLTIPS, this);
	m_pToolTipToggle->SetOffString(LoadTempWString(0xca0));
	m_pToolTipToggle->SetOnString(LoadTempWString(0xc9f));

	m_pSysName = AddTextItem(std::wstring(), 0, 0, sysNamePos);
	m_pSysName->SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

	m_fToolTipWidth = g_pLayoutMgr->GetScreenCustomFloat(SCREEN_ID_SUBROUTINES, "ToolTipWidth");

	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);

	m_SystemMemoryCtrl.Init(LTIntPt(320, 240), MEMORY_INNER_RADIUS, MEMORY_OUTER_RADIUS, 24);
	m_SystemMemoryCtrl.ShowOutlines(LTFALSE);
	m_SystemMemoryCtrl.SetArcType(ARC_TYPE_SYSTEM_MEMORY);

	// RETAIL: each rating's buttons
	for (int i = 0; i < NUM_RATINGS; i++)
	{
		char szName[32];
		sprintf(szName, "ArrowLeftRect%d", i);
		LTRect rcLeft = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_SUBROUTINES, szName);
		sprintf(szName, "ArrowRightRect%d", i);
		LTRect rcRight = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_SUBROUTINES, szName);
		m_RatingBars[i].SetRects(rcLeft, rcRight);
		m_RatingBars[i].SetBuildPoints(&m_BuildPoints);
	}

	m_ModelMgr.Activate();
	m_pScreenModel = LTNULL;
	m_hOptimizer = LTNULL;
	m_bOptimizerHot = false;
	m_fLButtonDownTime = 0.0f;
	m_pDuplicateSecond = LTNULL;
	m_pDuplicateFirst = LTNULL;

	int nVersionSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SUBROUTINES, "VersionSize");
	m_VersionPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_SUBROUTINES, "VersionPos");
	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nVersionSize);
	m_pVersion = AddTextItem(L"", 0, 0, m_VersionPos);
	m_pVersion->SetFont(pFont, nVersionSize);

	int nCountSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SUBROUTINES, "BuildNoteCountSize");
	LTVector vCountColor = g_pLayoutMgr->GetScreenCustomVector(SCREEN_ID_SUBROUTINES, "BuildNoteCountColor");
	m_BuildNoteCountPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_SUBROUTINES, "BuildNoteCountPos");
	m_nBuildNoteCountStringId = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SUBROUTINES, "BuildNoteCountStringId");
	m_nBuildNoteIconSpacer = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SUBROUTINES, "BuildNoteIconSpacer");
	m_fBuildNoteIconScalar = g_pLayoutMgr->GetScreenCustomFloat(SCREEN_ID_SUBROUTINES, "BuildNoteIconScalar");
	pFont = g_pInterfaceResMgr->GetFontForHeight(nCountSize);
	m_pBuildNoteCount = AddTextItem(L"", 0, 0, m_BuildNoteCountPos);
	m_pBuildNoteCount->SetFont(pFont, nCountSize);
	uint32 nCountColor = SET_ARGB(0xFF, (uint8)vCountColor.x, (uint8)vCountColor.y, (uint8)vCountColor.z);
	m_pBuildNoteCount->SetColors(nCountColor, nCountColor, nCountColor);

	char szTexture[1024];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_SUBROUTINES, "BuildNoteIconTexture", szTexture, sizeof(szTexture));
	m_hBuildNoteIcon = g_pInterfaceResMgr->GetTexture(szTexture);
	g_pDrawPrim->SetRGBA(&m_BuildNoteIconPoly, 0xFFFFFFFF);
	g_pDrawPrim->SetUVWH(&m_BuildNoteIconPoly, 0.0f, 0.0f, 1.0f, 1.0f);

	m_PopupText.Init(false);

	return LTTRUE;
}

// RETAIL: 0x1007e0a0
void CScreenSubroutines::SetSystemName(int nStringID)
{
	if (!m_pSysName) return;

	if (nStringID)
		m_pSysName->SetString(LoadTempWString(nStringID));
	else
		m_pSysName->SetString(L"");
}

// RETAIL: 0x1007e1c0
void CScreenSubroutines::SetPopupText(uint32 nTextId, uint8 nPopupId, char * pText)
{
	m_nPopupTextId = nTextId;
	m_nPopupId = nPopupId;
	if (pText)
	{
		strncpy(m_szPopupText, pText, 127);
		m_szPopupText[127] = 0;
	}
	else
		m_szPopupText[0] = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::OnFocus()
//
//	PURPOSE:	This is called when the screen gets or loses focus
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if (g_pInterfaceResMgr->GetXRatio() != m_fLastScale)
		ScreenDimsChanged();

	if (bFocus)
	{
		m_bDoneIntro = false;
		m_pLastCtrl = LTNULL;
		m_bRatingLeftHot = false;
		m_bRatingRightHot = false;
		m_bLibraryCCWHot = false;
		m_bLibraryCWHot = false;

		PlaySubroutineStartSound();

		// the screen glow is off while the screen is up
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("screenglowenable");
		m_nOldScreenGlow = 0;
		if (hVar)
			m_nOldScreenGlow = (int)g_pLTClient->GetVarValueFloat(hVar);
		g_pLTClient->RunConsoleString("screenglowenable 0");

		m_eCursorMode = CursorPickup;
		m_SystemMemoryCtrl.SetNumHighlightSegments(-1);
		m_SystemMemoryCtrl.ShowSprites(true);

		m_bRatingLeftHot = false;
		m_pHotSub = LTNULL;
		m_pHotProc = LTNULL;
		m_pCursorSub = LTNULL;
		m_bBuildPointsHot = false;
		m_bRatingRightHot = false;
		m_bLibraryCCWHot = false;
		m_bLibraryCWHot = false;

		m_Libraries[0].Init(0, 45.0f, 135.0f);
		m_Libraries[1].Init(1, 165.0f, 255.0f);
		m_Libraries[2].Init(2, 285.0f, 15.0f);

		CreateMenuPieces();

		m_pScreenMgr->GetTransitionFXMgr()->ClearScreenHistory();
		m_pScreenMgr->GetTransitionFXMgr()->SetLooping(false);

		g_pCursorMgr->UseSprite("Interface\\cursor.dtx");
		g_pCursorMgr->UseGlowSprite("Interface\\cursor_glow.dtx");
		g_pCursorMgr->SetCenter(32, 32);

		m_bShowToolTips = pProfile->m_bAlwaysShowToolTip;

		float fRatio = g_pInterfaceResMgr->GetXRatio();
		m_ToolTip.m_nFontSize = (int)((float)m_nToolTipFontSize * fRatio);
		m_ToolTip.m_nOffset = (int)(fRatio * 40.0f);
		m_ToolTip.m_nWidth = (int)(fRatio * m_fToolTipWidth);
		m_ToolTip.m_fDelay = 0.0f;
		m_ToolTip.m_fFadeTime = 0.0f;

		m_DuplicateDlg.Init();

		// the jet version
		uint16 nVersion;
		((CTronPlayerStats*)g_pPlayerStats)->GetJetVersion(nVersion);
		int nMajor = nVersion / 100;
		nVersion -= nMajor * 100;
		int nMinor = nVersion / 10;
		nVersion -= nMinor * 10;

		char szTmp[80];
		FormatString(0xeec, szTmp, sizeof(szTmp), nMajor, nMinor, nVersion);
		{
			// TRON: widened into a string of its own (RETAIL: 0x1007e4be)
			std::wstring sText;
			LTGUIWiden(sText, szTmp);
			m_pVersion->SetString(sText);
			m_bTextFade = false;
			m_pVersion->Show(LTFALSE);
			m_fTextFade = 0.0f;
			m_pSysName->Show(LTFALSE);
			m_pClear->Show(LTFALSE);
			m_pToolTipTitle->Show(LTFALSE);
			m_pToolTipToggle->Show(LTFALSE);

			float fXRatio = g_pInterfaceResMgr->GetXRatio();
			int nWidth = (int)((float)m_pVersion->GetBaseWidth() / fXRatio);
			m_pVersion->GetBaseHeight();
			m_pVersion->SetBasePos(LTIntPt(m_VersionPos.x - nWidth / 2, m_VersionPos.y));
		}

		// the build notes found
		int nFound = 0;
		CIntelItemList* pList = g_pPlayerStats->GetIntelList();
		int nCount = pList->GetCount();
		for (int i = 0; i < nCount; i++)
		{
			INTEL_ITEM* pItem = g_pPlayerStats->GetIntelList()->Get(i);
			if (pItem && pItem->nType == 1)
				nFound++;
		}

		// PORT: retail adds up the build notes of each level of each
		// mission (Tron's LEVEL +0x1004, 0x1007e706), which our LEVEL
		// does not have yet
		int nTotal = 0;

		if (m_pBuildNoteCount)
		{
			FormatString(m_nBuildNoteCountStringId, szTmp, sizeof(szTmp), nFound, nTotal);
			// TRON: widened into a string of its own (RETAIL: 0x1007e7b3)
			std::wstring sText;
			LTGUIWiden(sText, szTmp);
			m_pBuildNoteCount->SetString(sText);
			m_bBuildNoteFade = false;
			m_pBuildNoteCount->Show(LTFALSE);
			m_fTextFade = 0.0f;	// RETAIL: 0x1007e8ad: not m_fBuildNoteFade (docs/RETAIL_BUGS.md)

			int nTextWidth = (int)((float)m_pBuildNoteCount->GetBaseWidth() / g_pInterfaceResMgr->GetXRatio());
			int nTextHeight = (int)((float)m_pBuildNoteCount->GetBaseHeight() / g_pInterfaceResMgr->GetYRatio());

			uint32 nTexW, nTexH;
			g_pTexInterface->GetTextureDims(m_hBuildNoteIcon, nTexW, nTexH);
			int nIconW = (int)((float)nTexW * m_fBuildNoteIconScalar);
			int nIconH = (int)((float)nTexH * m_fBuildNoteIconScalar);

			float fTextH = (float)nTextHeight;
			float fIconH = (float)nIconH;
			float fMaxH = (fIconH < fTextH) ? fTextH : fIconH;

			int nLeft = (int)((float)m_BuildNoteCountPos.x - (float)(m_nBuildNoteIconSpacer + nIconW + nTextWidth) * 0.5f);
			float fTop = (float)m_BuildNoteCountPos.y;

			g_pDrawPrim->SetXYWH(&m_BuildNoteIconPoly,
				(float)nLeft * g_pInterfaceResMgr->GetXRatio(),
				((fMaxH - fIconH) * 0.5f + fTop) * g_pInterfaceResMgr->GetYRatio(),
				(float)nIconW * g_pInterfaceResMgr->GetXRatio(),
				fIconH * g_pInterfaceResMgr->GetYRatio());

			m_pBuildNoteCount->SetBasePos(LTIntPt(
				(int)((float)(m_nBuildNoteIconSpacer + nIconW) + (float)nLeft),
				(int)((fMaxH - fTextH) * 0.5f + fTop)));
		}

		SetSelection(kNoSelection);
		UpdateData(LTTRUE);
		g_pCursorMgr->ShowCursorLines(false);	// RETAIL: the interface manager's +0x40
	}
	else
	{
		char szCmd[32];
		sprintf(szCmd, "screenglowenable %d", m_nOldScreenGlow);
		g_pLTClient->RunConsoleString(szCmd);

		pProfile->m_bAlwaysShowToolTip = (m_bShowToolTips != LTFALSE);

		if (m_hOptimizer)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_OBJECT_MESSAGE);
			cMsg.WriteObject(m_hOptimizer);
			cMsg.Writeuint32(MID_OPTIMIZER_RESULT);
			cMsg.Writebool(m_Optimizer.IsDone() || m_Optimizer.IsBusy());
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			m_hOptimizer = LTNULL;
		}

		for (int i = 0; i < NUM_LIBRARIES; i++)
			m_Libraries[i].Term();

		TermControls();

		m_pScreenMgr->GetTransitionFXMgr()->SetLooping(true);

		ClearScreen();

		g_pCursorMgr->UseSprite((CScreenSprite*)LTNULL);
		g_pCursorMgr->SetCenter(16, 16);

		m_ToolTip.Hide();
		if (m_PopupText.IsVisible())
			m_PopupText.Close();
		m_DuplicateDlg.Term();

		if (m_hAnimSound)
		{
			g_pLTClient->SoundMgr()->KillSound(m_hAnimSound);
			m_hAnimSound = LTNULL;
		}
	}

	CBaseScreen::OnFocus(bFocus);
}

// RETAIL: 0x1007ed10
void CScreenSubroutines::TermControls()
{
	ModelPieceArray::iterator iter;
	for (iter = m_ModelPieceArray.begin(); iter != m_ModelPieceArray.end(); iter++)
	{
		(*iter)->Term();
		debug_delete(*iter);
	}
	m_ModelPieceArray.clear();

	m_pScreenModel = LTNULL;
	m_Optimizer.Term();

	for (int i = 0; i < NUM_LIBRARIES; i++)
	{
		m_Libraries[i].Term();
		m_ProcCtrl[i].Term();
	}

	m_BuildPoints.Term();
	for (int j = 0; j < NUM_RATINGS; j++)
		m_RatingBars[j].Term();
}

// RETAIL: 0x1007edf0, the screen's models ("Piece%d")
void CScreenSubroutines::CreateMenuPieces()
{
	char szPiece[128] = "";
	char szName[32];

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	int n = 0;
	bool bSkipIntro = false;
	if (IsMultiplayerGame() || pProfile->m_bSkipSubIntro)
		bSkipIntro = true;

	bool bFound;
	do
	{
		szPiece[0] = 0;
		bFound = false;
		sprintf(szName, "Piece%d", n);
		if (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID, szName))
		{
			bFound = true;
			g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID, szName, szPiece, sizeof(szPiece));

			CMenuPieceModel * pPiece = debug_new(CMenuPieceModel);
			pPiece->Create(szPiece);
			pPiece->Show(true);
			m_ModelPieceArray.push_back(pPiece);

			if (!_stricmp(szPiece, "sub_hub"))
			{
				pPiece->PlayAnim("show", false, false);
				m_pScreenModel = pPiece;
				m_SystemMemoryCtrl.AssociateModel("sub_hub");
			}

			if (bSkipIntro)
				pPiece->PlayAnim("shown", false, false);
		}
		n++;
	}
	while (bFound);

	m_Libraries[0].AssociateModel("browser_top");
	m_Libraries[1].AssociateModel("browser_left");
	m_Libraries[2].AssociateModel("browser_right");
}

// RETAIL: 0x1007ef90
uint32 CScreenSubroutines::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_DONE:
		Escape();
		return 1;

	case CMD_CLEAR:
		ClearMemory();
		return 1;

	case CMD_TOOLTIPS:
		PlayToggleToolTipsSound();
		UpdateData(LTTRUE);
		return 1;
	}
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
}

// RETAIL: 0x1007f000
LTBOOL CScreenSubroutines::HandleKeyDown(int key, int rep)
{
	if (m_PopupText.IsVisible())
	{
		m_PopupText.Close();
		return LTTRUE;
	}

	if (key == VK_F1 || key == VK_ESCAPE)
	{
		Escape();
		return LTTRUE;
	}

	return CBaseScreen::HandleKeyDown(key, rep);
}

// RETAIL: 0x1007f050, leaving compiles the screen into the player's
// subroutines
void CScreenSubroutines::Escape()
{
	PlayCompileSound();
	Compile();
	CBaseScreen::Escape();
}

// RETAIL: 0x1007f070: once the hub's "show" is done, the screen shows its
// controls
void CScreenSubroutines::UpdateIntro()
{
	if (m_bDoneIntro || !m_pScreenModel)
		return;

	if (!m_pScreenModel->IsAnimDone() && !IsMultiplayerGame() &&
		!g_pProfileMgr->GetCurrentProfile()->m_bSkipSubIntro)
		return;

	m_bDoneIntro = true;

	if (!IsMultiplayerGame())
	{
		m_bTextFade = true;
		m_fTextFade = 1.0f;
		m_pVersion->Show(LTTRUE);
		m_pSysName->Show(LTTRUE);
		m_pClear->Show(LTTRUE);
		m_pToolTipTitle->Show(LTTRUE);
		m_pToolTipToggle->Show(LTTRUE);

		if (!m_hOptimizer)
		{
			m_bBuildNoteFade = true;
			m_fBuildNoteFade = 1.0f;
			m_pBuildNoteCount->Show(LTTRUE);
		}
	}

	m_pScreenModel->PlayAnim("shown", true, false);

	m_Libraries[0].Show();
	m_Libraries[1].Show();
	m_Libraries[2].Show();

	if (!m_hOptimizer && !IsMultiplayerGame())
	{
		for (int i = 0; i < NUM_RATINGS; i++)
			m_RatingBars[i].SetTextVisible(true);
	}

	if (m_pDuplicateFirst)
		m_DuplicateDlg.Show(m_pDuplicateFirst, m_pDuplicateSecond);
}

// RETAIL: 0x1007f1e0
LTBOOL CScreenSubroutines::Render(HSURFACE hDestSurf)
{
	LTBOOL bResult = CBaseScreen::Render(hDestSurf);

	if (m_nPopupTextId)
	{
		// RETAIL: the text by id, with the image named by m_szPopupText
		// (0x1007f213; CPopupText::Show at 0x1007f630)
		m_PopupText.Show(m_nPopupTextId, m_nPopupId, m_szPopupText);
		m_nPopupTextId = 0;
	}

	if (m_PopupText.IsVisible())
		m_PopupText.Draw();
	else if (m_DuplicateDlg.IsVisible())
		m_DuplicateDlg.Draw();

	m_ModelMgr.Update();
	m_SystemMemoryCtrl.Render();
	m_Optimizer.Update();
	UpdateIntro();

	if (m_DuplicateDlg.IsVisible())
		return bResult;

	if (m_PopupText.IsVisible())
		return bResult;

	if (m_bDoneIntro)
	{
		for (int i = 0; i < NUM_LIBRARIES; i++)
		{
			m_Libraries[i].Update();
			m_Libraries[i].Render();
		}
	}

	if (m_bTextFade)
	{
		if (m_fTextFade > 0.0f)
		{
			m_fTextFade -= g_pLTClient->GetFrameTime();
			if (m_fTextFade > 0.0f)
			{
				uint32 nColor = ((uint32)(uint8)((1.0f - m_fTextFade) * 255.0f) << 24) | 0x00FFFFFF;
				m_pVersion->SetColors(nColor, nColor, nColor);
				m_pSysName->SetColors(nColor, nColor, nColor);
				m_pClear->SetColors(nColor, nColor, nColor);
				m_pToolTipTitle->SetColors(nColor, nColor, nColor);
				m_pToolTipToggle->SetColors(nColor, nColor, nColor);
			}
			else
			{
				m_pVersion->SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
				m_pSysName->SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
				m_pClear->SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
				m_pToolTipTitle->SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
				m_pToolTipToggle->SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
			}
		}
		m_pVersion->Render();
	}

	if (m_bBuildNoteFade)
	{
		if (m_fBuildNoteFade > 0.0f)
		{
			m_fBuildNoteFade -= g_pLTClient->GetFrameTime();
			if (m_fBuildNoteFade > 0.0f)
			{
				uint32 nColor = ((uint32)(uint8)((1.0f - m_fBuildNoteFade) * 255.0f) << 24) | 0x00FFFFFF;
				m_pBuildNoteCount->SetColors(nColor, nColor, nColor);
			}
			else
			{
				m_pBuildNoteCount->SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
			}
		}

		if (m_hBuildNoteIcon)
		{
			g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
			g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
			g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
			g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
			g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
			g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
			g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
			g_pDrawPrim->SetTexture(m_hBuildNoteIcon);
			g_pDrawPrim->DrawPrim(&m_BuildNoteIconPoly, 1);
		}
		m_pBuildNoteCount->Render();
	}

	if (!m_hOptimizer && !IsMultiplayerGame())
	{
		m_BuildPoints.Update();
		for (int i = 0; i < NUM_RATINGS; i++)
			m_RatingBars[i].Update();
	}

	// the tool tips show while Tab is down
	bool bOldToolTips = m_bToolTips;
	m_bToolTips = (m_bShowToolTips || IsKeyDown(VK_TAB));
	if (bOldToolTips != m_bToolTips)
	{
		if (bOldToolTips)
			m_ToolTip.Hide();
		else
		{
			m_pHotSub = LTNULL;
			m_pHotProc = LTNULL;
			m_pHotRating = LTNULL;
			m_pLastCtrl = LTNULL;
			m_bBuildPointsHot = false;
			m_bRatingLeftHot = false;
			m_bRatingRightHot = false;
			m_bLibraryCCWHot = false;
			m_bLibraryCWHot = false;
			m_bOptimizerHot = false;
			OnMouseMove(m_CursorPos.x, m_CursorPos.y);
		}
	}

	if (g_pInterfaceMgr->IsMessageBoxVisible())
		m_ToolTip.Hide();

	return bResult;
}

// RETAIL: 0x1007f6a0, the wheel turns the library the cursor is over
LTBOOL CScreenSubroutines::OnWheelUp()
{
	for (int i = 0; i < NUM_LIBRARIES; i++)
	{
		if (m_Libraries[i].IsHot())
		{
			g_pClientSoundMgr->PlayInterfaceSound("interface\\subroutines\\snd\\clickon.wav");
			m_Libraries[i].RotateCCW();
			OnMouseMove(m_CursorPos.x, m_CursorPos.y);
			return LTTRUE;
		}
	}
	return LTTRUE;
}

// RETAIL: 0x1007f710
LTBOOL CScreenSubroutines::OnWheelDown()
{
	for (int i = 0; i < NUM_LIBRARIES; i++)
	{
		if (m_Libraries[i].IsHot())
		{
			g_pClientSoundMgr->PlayInterfaceSound("interface\\subroutines\\snd\\clickon.wav");
			m_Libraries[i].RotateCW();
			OnMouseMove(m_CursorPos.x, m_CursorPos.y);
			return LTTRUE;
		}
	}
	return LTTRUE;
}

// RETAIL: 0x1007f780
LTBOOL CScreenSubroutines::OnMouseMove(int x, int y)
{
	PlayerSubroutine * pOldHotSub = m_pHotSub;
	bool bOldBuildPointsHot = m_bBuildPointsHot;
	CLTGUICtrl * pOldLastCtrl = m_pLastCtrl;
	Procedural * pOldHotProc = m_pHotProc;
	bool bOldRatingLeftHot = m_bRatingLeftHot;
	CRatingBarCtrl * pOldHotRating = m_pHotRating;
	bool bOldRatingRightHot = m_bRatingRightHot;
	bool bOldLibraryCCWHot = m_bLibraryCCWHot;
	bool bOldLibraryCWHot = m_bLibraryCWHot;
	int nHotLibrary = -1;
	LTIntPt hotPos;

	m_CursorPos.x = x;
	m_CursorPos.y = y;
	m_pHotSub = LTNULL;
	m_pHotProc = LTNULL;
	m_pHotRating = LTNULL;
	m_bBuildPointsHot = false;
	m_bRatingLeftHot = false;
	m_bRatingRightHot = false;
	m_bLibraryCCWHot = false;
	m_bLibraryCWHot = false;

	if (!m_bDoneIntro)
		return LTTRUE;

	if (m_PopupText.IsVisible())
		return LTTRUE;

	if (m_DuplicateDlg.IsVisible())
	{
		m_DuplicateDlg.OnMouseMove(x, y);
		m_ToolTip.Hide();
		return LTTRUE;
	}

	CLTGUICtrl * pOldCtrl = GetControl(m_nSelection);
	m_pLastCtrl = pOldCtrl;
	LTBOOL bResult = CBaseScreen::OnMouseMove(x, y);

	uint16 nControlUnderPoint = 0;
	if (GetControlUnderPoint(x, y, &nControlUnderPoint))
		SetSelection(nControlUnderPoint);
	else
		SetSelection(kNoSelection);

	CLTGUICtrl * pCtrl = GetControl(m_nSelection);

	if (!m_hOptimizer && !IsMultiplayerGame())
	{
		for (int i = 0; i < NUM_RATINGS; i++)
		{
			m_RatingBars[i].OnMouseMove(x, y);
			if (m_RatingBars[i].IsHot())
			{
				m_pHotRating = &m_RatingBars[i];
				hotPos = m_RatingBars[i].GetCenter();
				m_bRatingLeftHot = m_RatingBars[i].IsOverLeft();
				m_bRatingRightHot = m_RatingBars[i].IsOverRight();
			}
		}

		m_BuildPoints.OnMouseMove(x, y);
		m_bBuildPointsHot = m_BuildPoints.IsHot();
		if (m_bBuildPointsHot)
			hotPos = m_BuildPoints.GetHelpPos();
	}

	for (int i = 0; i < NUM_PROC_CTRLS; i++)
	{
		m_ProcCtrl[i].OnMouseMove(x, y, m_pCursorSub);
		if (!m_pHotProc && m_ProcCtrl[i].IsHot())
		{
			SetSelection(kNoSelection);
			m_pHotProc = m_ProcCtrl[i].GetProc();
			hotPos = m_ProcCtrl[i].GetCenter();
		}
	}

	bool bOldOptimizerHot = m_bOptimizerHot;
	if (m_hOptimizer)
	{
		m_Optimizer.OnMouseMove(x, y, m_pCursorSub);
		m_bOptimizerHot = m_Optimizer.IsHot();
	}

	m_SystemMemoryCtrl.OnMouseMove(x, y);
	if (m_SystemMemoryCtrl.IsHot())
	{
		SetSelection(kNoSelection);
		m_pHotSub = m_SystemMemoryCtrl.GetHotSub();
		if (m_pHotSub)
			hotPos = m_SystemMemoryCtrl.GetHotPos();

		if (m_pCursorSub && m_eCursorMode == CursorSubroutineDrop &&
			m_nLastSegment != m_SystemMemoryCtrl.GetActiveSegment())
		{
			if (CanDropHere())
			{
				PlayActiveMemoryYesSound();
				m_nLastSegment = m_SystemMemoryCtrl.GetActiveSegment();
			}
			else
			{
				PlayActiveMemoryNoSound();
				m_nLastSegment = m_SystemMemoryCtrl.GetActiveSegment();
			}
		}
		else
			m_nLastSegment = m_SystemMemoryCtrl.GetActiveSegment();
	}
	else
		m_nLastSegment = -1;

	for (int j = 0; j < NUM_LIBRARIES; j++)
	{
		m_Libraries[j].OnMouseMove(x, y, m_pCursorSub);
		if (m_Libraries[j].IsHot())
		{
			m_bLibraryCCWHot = m_Libraries[j].IsCCWHot();
			m_bLibraryCWHot = m_Libraries[j].IsCWHot();
			nHotLibrary = j;
			SetSelection(kNoSelection);

			LibraryState eState = m_Libraries[j].GetState();
			if (eState == LIBRARY_STATE_OPENING || eState == LIBRARY_STATE_OPEN)
			{
				m_Libraries[(j + 1) % NUM_LIBRARIES].Close();
				m_Libraries[(j + 2) % NUM_LIBRARIES].Close();
			}

			m_pHotSub = m_Libraries[j].GetHotSub();
			if (m_pHotSub)
				hotPos = m_Libraries[j].GetArc()->GetHotPos();
		}
	}

	if (m_pHotSub != pOldHotSub || m_pHotProc != pOldHotProc ||
		m_pHotRating != pOldHotRating || m_bRatingRightHot != bOldRatingRightHot ||
		m_bRatingLeftHot != bOldRatingLeftHot || m_bBuildPointsHot != bOldBuildPointsHot ||
		m_bOptimizerHot != bOldOptimizerHot || pCtrl != pOldCtrl ||
		m_pLastCtrl != pOldLastCtrl || m_bLibraryCCWHot != bOldLibraryCCWHot ||
		m_bLibraryCWHot != bOldLibraryCWHot)
	{
		if (m_pHotSub && m_pHotSub != pOldHotSub)
		{
			PlayMouseOverSound(m_pHotSub);
			if (!m_pCursorSub && m_hAnimSound)
			{
				g_pLTClient->SoundMgr()->KillSound(m_hAnimSound);
				m_hAnimSound = LTNULL;
			}
		}

		if (pCtrl != pOldCtrl)
		{
			if (pCtrl == m_pClear || pCtrl == m_pToolTipTitle ||
				pCtrl == m_pToolTipToggle || pCtrl == m_pSysName)
				PlayMouseOverSound();
		}

		m_ToolTip.Hide();

		if (m_bToolTips)
		{
			if (m_pHotSub && !m_pCursorSub)
				ShowSubInfo(hotPos.x, hotPos.y);
			else if (m_pHotProc)
				ShowProcInfo(hotPos.x, hotPos.y);
			else if (m_pHotRating)
				ShowRatingInfo(hotPos.x, hotPos.y);
			else if (m_bBuildPointsHot)
				ShowBuildPointsInfo(hotPos.x, hotPos.y);
			else if (m_bLibraryCCWHot && nHotLibrary != -1)
			{
				LTIntPt pos = m_Libraries[nHotLibrary].GetHotPos();
				m_ToolTip.Show(pos.x, pos.y, LoadTempWString(0x9fb), LTNULL);
			}
			else if (m_bLibraryCWHot && nHotLibrary != -1)
			{
				LTIntPt pos = m_Libraries[nHotLibrary].GetHotPos();
				m_ToolTip.Show(pos.x, pos.y, LoadTempWString(0x9fc), LTNULL);
			}
			else if (m_bOptimizerHot)
			{
				if (m_Optimizer.IsDone() || m_Optimizer.IsBusy())
				{
					LTIntPt pos = m_Optimizer.GetCenter();
					m_ToolTip.Show(pos.x, pos.y, LoadTempWString(0xcd7), LTNULL);
				}
				else
				{
					LTIntPt pos = m_Optimizer.GetCenter();
					m_ToolTip.Show(pos.x, pos.y, LoadTempWString(0xcb4), LTNULL);
				}
			}
			else if (!pCtrl)
				m_ToolTip.Hide();
			else if (pCtrl == pOldCtrl && m_pLastCtrl == pOldLastCtrl)
				m_ToolTip.Hide();
			else
				ShowCtrlInfo(x, y, pCtrl);
		}
	}

	UpdateAnimSound();

	return bResult;
}

// RETAIL: 0x1007ffd0
void CScreenSubroutines::ClearScreen()
{
	m_SystemMemoryCtrl.Build();

	for (int i = 0; i < NUM_LIBRARIES; i++)
		m_Libraries[i].Clear();

	m_SubArray.clear();
	m_ProcArray.clear();

	m_pDuplicateSecond = LTNULL;
	m_pDuplicateFirst = LTNULL;
}

// RETAIL: 0x10080070
void CScreenSubroutines::SetRating(int nRating, int nValue)
{
	if (nRating < 0 || nRating >= NUM_RATINGS)
		return;

	m_RatingBars[nRating].Init(nRating, nValue);
	if (m_hOptimizer)
		m_RatingBars[nRating].Show(false);
}

// RETAIL: 0x100800c0
void CScreenSubroutines::SetRatingPoints(int nPoints)
{
	m_BuildPoints.SetVisible(nPoints > 0);
	m_BuildPoints.Init(nPoints);
}

// RETAIL: 0x100800e0: the optimizer object's screen: the optimizer
// replaces the ratings
void CScreenSubroutines::SetOptimizer(HOBJECT hOptimizer)
{
	m_hOptimizer = hOptimizer;
	if (!hOptimizer)
		return;

	m_Optimizer.Init();
	for (int i = 0; i < NUM_RATINGS; i++)
		m_RatingBars[i].Show(false);

	if (m_pBuildNoteCount)
	{
		m_pBuildNoteCount->Show(LTFALSE);
		m_bBuildNoteFade = false;
	}
}

// RETAIL: 0x10080140
void CScreenSubroutines::AddSubroutine(PlayerSubroutine * pSub)
{
	// two copies of one subroutine ask the player which to keep
	if (!m_pDuplicateFirst)
	{
		TronSubroutine * pTronSub = pSub->pTronSubroutine;
		if (pTronSub->eFunction != FUNCTION_BASECODE && pTronSub->eFunction != FUNCTION_BADBLOCK)
		{
			PlayerSubroutineArray::iterator iter;
			for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
			{
				if (pTronSub == (*iter)->pTronSubroutine &&
					pSub->eState != SUBSTATE_FOREIGN && (*iter)->eState != SUBSTATE_FOREIGN)
				{
					PlayerSubroutine * pFirst = *iter;
					m_pDuplicateSecond = pSub;
					m_pDuplicateFirst = pFirst;
					break;
				}
			}
		}
	}

	m_SubArray.push_back(pSub);
	pSub->nTempSlot = pSub->nSlot;

	int nFunction = -1;
	if (pSub->pTronSubroutine->eFunction >= FUNCTION_COMBAT && pSub->pTronSubroutine->eFunction <= FUNCTION_UTILITY)
		nFunction = pSub->pTronSubroutine->eFunction;

	if (pSub->nTempSlot >= 0 && pSub->nTempSlot < 100)
	{
		m_SystemMemoryCtrl.SetNumHighlightSegments(3 - pSub->eVersion);
		m_SystemMemoryCtrl.OccupySegment(pSub->nTempSlot, pSub);
		m_SystemMemoryCtrl.SetNumHighlightSegments(-1);
		return;
	}

	if (nFunction >= 0 && nFunction <= 2)
		m_Libraries[nFunction].InsertSub(pSub);
}

// RETAIL: 0x10080260
void CScreenSubroutines::AddProcedural(Procedural * pProcedural)
{
	m_ProcArray.push_back(pProcedural);
	m_ProcCtrl[pProcedural->iProcSlot].Init(pProcedural);
}

// RETAIL: 0x100802c0, the "Clear" button: every subroutine of the system
// memory goes back to its library
void CScreenSubroutines::ClearMemory()
{
	PlayClearMemSound();

	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
	{
		PlayerSubroutine * pSub = *iter;
		if (pSub->nTempSlot >= 0 && pSub->nTempSlot < 100)
		{
			switch (pSub->pTronSubroutine->eFunction)
			{
			case FUNCTION_COMBAT:
				m_SystemMemoryCtrl.ClearSegment(pSub->nTempSlot);
				m_Libraries[0].InsertSub(pSub);
				break;
			case FUNCTION_DEFENSE:
				m_SystemMemoryCtrl.ClearSegment(pSub->nTempSlot);
				m_Libraries[1].InsertSub(pSub);
				break;
			case FUNCTION_UTILITY:
				m_SystemMemoryCtrl.ClearSegment(pSub->nTempSlot);
				m_Libraries[2].InsertSub(pSub);
				break;
			}
		}
	}
}

// RETAIL: 0x10080350
void CScreenSubroutines::Compile()
{
	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
		(*iter)->nSlot = (*iter)->nTempSlot;

	// RETAIL: the ratings bought
	m_BuildPoints.Apply();
	for (int i = 0; i < NUM_RATINGS; i++)
		m_RatingBars[i].Apply();

	g_pSubroutineMgr->Compile();
}

// RETAIL: 0x100803b0
void CScreenSubroutines::PickupSubroutine(PlayerSubroutine * pSub)
{
	if (!pSub || !pSub->pTronSubroutine)
		return;

	m_Libraries[0].Close();
	m_Libraries[1].Close();
	m_Libraries[2].Close();

	PlayPickupSound(pSub);

	switch (pSub->pTronSubroutine->eFunction)
	{
	case FUNCTION_BADBLOCK:
	case FUNCTION_BASECODE:
		m_SystemMemoryCtrl.ShowIcon(pSub->nTempSlot, false);
		m_SystemMemoryCtrl.SetNumHighlightSegments(1);
		m_pCursorSub = pSub;
		SetCursorMode(CursorSubroutineDrop);
		return;

	case FUNCTION_COMBAT:
		m_Libraries[0].Open(true);
		break;

	case FUNCTION_DEFENSE:
		m_Libraries[1].Open(true);
		break;

	case FUNCTION_UTILITY:
		m_Libraries[2].Open(true);
		break;
	}

	m_pCursorSub = pSub;
	SetCursorMode(CursorSubroutineDrop);

	if (pSub->nTempSlot >= 0 && pSub->nTempSlot < 100)
		m_SystemMemoryCtrl.ClearSegment(pSub->nTempSlot);
	else if (pSub->nTempSlot >= 100 && pSub->nTempSlot < 200)
		m_Libraries[0].RemoveSub(pSub);
	else if (pSub->nTempSlot >= 200 && pSub->nTempSlot < 300)
		m_Libraries[1].RemoveSub(pSub);
	else if (pSub->nTempSlot >= 300 && pSub->nTempSlot < 400)
		m_Libraries[2].RemoveSub(pSub);

	if (pSub->eState == SUBSTATE_FOREIGN)
		m_SystemMemoryCtrl.SetNumHighlightSegments(1);
	else
		m_SystemMemoryCtrl.SetNumHighlightSegments(3 - pSub->eVersion);
}

// RETAIL: 0x10080510, a click while nothing is on the cursor
void CScreenSubroutines::OnPickupClick(int x, int y)
{
	int i;
	for (i = 0; i < NUM_RATINGS; i++)
	{
		if (m_RatingBars[i].IsHot())
		{
			m_RatingBars[i].OnLButtonDown(x, y);
			return;
		}
	}

	for (i = 0; i < NUM_LIBRARIES; i++)
	{
		if (m_Libraries[i].IsRotateHot())
		{
			m_Libraries[i].OnClick();
			return;
		}
	}

	if (m_pHotSub)
	{
		if (!m_pHotSub->pProcedural && m_pHotSub->pTronSubroutine->eFunction != FUNCTION_BASECODE)
		{
			PickupSubroutine(m_pHotSub);
			m_ToolTip.Hide();
			m_pHotSub = LTNULL;
		}
	}
	else if (m_pHotProc)
	{
		PlayerSubroutine * pSub = m_pHotProc->pSub;
		if (pSub)
		{
			for (int j = 0; j < NUM_PROC_CTRLS; j++)
			{
				if (m_ProcCtrl[j].GetProc() == m_pHotProc)
				{
					m_ProcCtrl[j].RemoveSub();
					if (IsKeyDown(VK_TAB) || m_bShowToolTips)
					{
						LTIntPt pos = m_ProcCtrl[j].GetCenter();
						ShowProcInfo(pos.x, pos.y);
					}
				}
			}
			PickupSubroutine(pSub);
		}
	}
	else
		CBaseScreen::OnLButtonDown(x, y);
}

// RETAIL: 0x10080670, a click with a subroutine on the cursor
void CScreenSubroutines::OnSubroutineClick(int x, int y)
{
	if (m_pHotProc)
	{
		if (!g_pSubroutineMgr->CanProcHookSub(m_pHotProc, m_pCursorSub))
			return;

		PlayerSubroutine * pOldSub = m_pHotProc->pSub;
		if (pOldSub)
		{
			PlayerSubroutine * pCursorSub = m_pCursorSub;
			OnPickupRClick(x, y);
			m_pCursorSub = pCursorSub;
		}

		for (int i = 0; i < NUM_PROC_CTRLS; i++)
		{
			if (m_ProcCtrl[i].GetProc() == m_pHotProc)
			{
				m_ProcCtrl[i].AddSub(m_pCursorSub);
				PlayDropIntoProceduralSound();
				if (IsKeyDown(VK_TAB) || m_bShowToolTips)
				{
					LTIntPt pos = m_ProcCtrl[i].GetCenter();
					ShowProcInfo(pos.x, pos.y);
				}
			}
		}

		if (m_pCursorSub->eState != SUBSTATE_UNUSABLE)
			PutSubroutineBack(false);

		if (pOldSub)
		{
			PickupSubroutine(pOldSub);
			m_ToolTip.Hide();
			m_pHotSub = LTNULL;
		}
		else
			SetCursorMode(CursorPickup);
		return;
	}

	if (m_pHotSub && m_pHotSub->pProcedural)
		return;

	if (m_SystemMemoryCtrl.IsHot())
	{
		if (m_pCursorSub->eState == SUBSTATE_UNUSABLE)
		{
			if (m_pCursorSub->nTempSlot != m_SystemMemoryCtrl.GetActiveSegment())
				return;

			PlayDropIntoSound();
			m_SystemMemoryCtrl.ShowIcon(m_pCursorSub->nTempSlot, true);
			SetCursorMode(CursorPickup);
			return;
		}

		if (m_pCursorSub->eState == SUBSTATE_FOREIGN)
		{
			PlayCantDropSound();
			return;
		}

		PlayerSubroutine * pDisplaced = LTNULL;
		int nSegments = 3 - m_pCursorSub->eVersion;
		for (int i = 0; i < nSegments; i++)
		{
			PlayerSubroutine * pSub = m_SystemMemoryCtrl.GetSegmentObject((m_SystemMemoryCtrl.GetActiveSegment() + i) % 24);
			if (pSub)
			{
				if (pSub->eState == SUBSTATE_UNUSABLE)
				{
					int nSlot = FindFreeSegment(m_pCursorSub);
					if (nSlot == -1)
					{
						PlayCantDropSound();
						return;
					}
					m_pCursorSub->nTempSlot = nSlot;
					goto Drop;
				}
				if (pSub != pDisplaced)
				{
					if (pDisplaced)
					{
						int nSlot = FindFreeSegment(m_pCursorSub);
						if (nSlot == -1)
						{
							PlayCantDropSound();
							return;
						}
						pDisplaced = LTNULL;
						m_pCursorSub->nTempSlot = nSlot;
						goto Drop;
					}
					pDisplaced = pSub;
				}
			}
		}
		m_pCursorSub->nTempSlot = m_SystemMemoryCtrl.GetActiveSegment();

Drop:
		if (pDisplaced)
		{
			m_SystemMemoryCtrl.ClearSegment(pDisplaced->nTempSlot);
			if (pDisplaced->pTronSubroutine->eFunction != m_pCursorSub->pTronSubroutine->eFunction)
			{
				m_Libraries[0].Close();
				m_Libraries[1].Close();
				m_Libraries[2].Close();
			}
		}
		else
		{
			switch (m_pCursorSub->pTronSubroutine->eFunction)
			{
			case FUNCTION_COMBAT:	m_Libraries[0].Open(false);	break;
			case FUNCTION_DEFENSE:	m_Libraries[1].Open(false);	break;
			case FUNCTION_UTILITY:	m_Libraries[2].Open(false);	break;
			}
		}

		m_SystemMemoryCtrl.OccupySegment(m_pCursorSub->nTempSlot, m_pCursorSub);
		PlayDropIntoSound();
		m_pHotSub = m_pCursorSub;
		SetCursorMode(CursorPickup);

		if (IsKeyDown(VK_TAB) || m_bShowToolTips)
		{
			LTIntPt pos = m_SystemMemoryCtrl.GetHotPos();
			ShowSubInfo(pos.x, pos.y);
		}

		if (!pDisplaced)
			return;

		m_pCursorSub = pDisplaced;
		pDisplaced->nTempSlot = -1;
		SetCursorMode(CursorSubroutineDrop);
		m_SystemMemoryCtrl.SetNumHighlightSegments(3 - m_pCursorSub->eVersion);

		switch (m_pCursorSub->pTronSubroutine->eFunction)
		{
		case FUNCTION_COMBAT:	m_Libraries[0].Open(true);	break;
		case FUNCTION_DEFENSE:	m_Libraries[1].Open(true);	break;
		case FUNCTION_UTILITY:	m_Libraries[2].Open(true);	break;
		}
		return;
	}

	if (m_hOptimizer && m_Optimizer.IsHot())
	{
		if (!m_Optimizer.CanAccept(m_pCursorSub))
			return;

		PlayDropIntoSound();
		m_Optimizer.Optimize(m_pCursorSub);
		OnSubroutineRClick(x, y);
		return;
	}

	for (int i = 0; i < NUM_LIBRARIES; i++)
	{
		if (m_Libraries[i].WasHot())
		{
			PlayerSubroutine * pSub = m_Libraries[i].GetHotSub();
			if (pSub)
			{
				m_Libraries[i].RemoveSub(pSub);
				m_Libraries[i].AddSub(m_pCursorSub);
				SetCursorMode(CursorPickup);
				m_pCursorSub = pSub;
				pSub->nTempSlot = -1;
				SetCursorMode(CursorSubroutineDrop);
				m_SystemMemoryCtrl.SetNumHighlightSegments(3 - pSub->eVersion);
				PlayDropIntoLibrarySound();
			}
			else
			{
				m_Libraries[i].AddSub(m_pCursorSub);
				SetCursorMode(CursorPickup);
				PlayDropIntoLibrarySound();
			}
			return;
		}
	}
}

// RETAIL: 0x10080b70, a right click while nothing is on the cursor: the
// subroutine goes to its library, or out of it into the system memory
void CScreenSubroutines::OnPickupRClick(int x, int y)
{
	if (m_pHotProc)
	{
		PlayerSubroutine * pSub = m_pHotProc->pSub;
		if (!pSub)
			return;

		for (int i = 0; i < NUM_PROC_CTRLS; i++)
		{
			if (m_ProcCtrl[i].GetProc() == m_pHotProc)
			{
				m_ProcCtrl[i].RemoveSub();
				if (IsKeyDown(VK_TAB) || m_bShowToolTips)
				{
					LTIntPt pos = m_ProcCtrl[i].GetCenter();
					ShowProcInfo(pos.x, pos.y);
				}
			}
		}

		if ((pSub->nTempSlot >= 0 && pSub->nTempSlot < 24) || pSub->eState == SUBSTATE_UNUSABLE)
		{
			m_SystemMemoryCtrl.ShowIcon(pSub->nTempSlot, true);
			return;
		}

		switch (pSub->pTronSubroutine->eFunction)
		{
		case FUNCTION_COMBAT:	m_Libraries[0].ShowSubIcon(pSub, true);	return;
		case FUNCTION_DEFENSE:	m_Libraries[1].ShowSubIcon(pSub, true);	return;
		case FUNCTION_UTILITY:	m_Libraries[2].ShowSubIcon(pSub, true);	return;
		}
		return;
	}

	if (!m_pHotSub)
		return;

	Procedural * pProc = m_pHotSub->pProcedural;
	if (pProc)
	{
		for (int i = 0; i < NUM_PROC_CTRLS; i++)
		{
			if (m_ProcCtrl[i].GetProc() == pProc)
				m_ProcCtrl[i].RemoveSub();
		}

		if ((m_pHotSub->nTempSlot >= 0 && m_pHotSub->nTempSlot < 24) ||
			m_pHotSub->pTronSubroutine->eFunction == FUNCTION_BADBLOCK)
		{
			m_SystemMemoryCtrl.ShowIcon(m_pHotSub->nTempSlot, true);
			if (IsKeyDown(VK_TAB) || m_bShowToolTips)
			{
				LTIntPt pos = m_SystemMemoryCtrl.GetHotPos();
				ShowSubInfo(pos.x, pos.y);
			}
			return;
		}

		switch (m_pHotSub->pTronSubroutine->eFunction)
		{
		case FUNCTION_BADBLOCK:
			m_SystemMemoryCtrl.ShowIcon(m_pHotSub->nTempSlot, true);
			if (IsKeyDown(VK_TAB) || m_bShowToolTips)
			{
				LTIntPt pos = m_SystemMemoryCtrl.GetHotPos();
				ShowSubInfo(pos.x, pos.y);
			}
			break;

		case FUNCTION_COMBAT:
			m_Libraries[0].ShowSubIcon(m_pHotSub, true);
			if (IsKeyDown(VK_TAB) || m_bShowToolTips)
			{
				LTIntPt pos = m_Libraries[0].GetHotPos();
				ShowSubInfo(pos.x, pos.y);
			}
			break;

		case FUNCTION_DEFENSE:
			m_Libraries[1].ShowSubIcon(m_pHotSub, true);
			if (IsKeyDown(VK_TAB) || m_bShowToolTips)
			{
				LTIntPt pos = m_Libraries[1].GetHotPos();
				ShowSubInfo(pos.x, pos.y);
			}
			break;

		case FUNCTION_UTILITY:
			m_Libraries[2].ShowSubIcon(m_pHotSub, true);
			if (IsKeyDown(VK_TAB) || m_bShowToolTips)
			{
				LTIntPt pos = m_Libraries[2].GetHotPos();
				ShowSubInfo(pos.x, pos.y);
			}
			break;
		}
	}

	PlayerSubroutine * pHotSub = m_pHotSub;
	if (pHotSub == m_SystemMemoryCtrl.GetHotSub())
	{
		if (pHotSub->pTronSubroutine->eFunction == FUNCTION_BADBLOCK)
			return;

		if (pHotSub->eState != SUBSTATE_UNUSABLE)
		{
			OnPickupClick(x, y);
			m_pCursorSub->nTempSlot = -1;
			OnSubroutineRClick(x, y);
		}
	}

	if (!m_pHotSub)
		return;

	if (m_pHotSub->eState == SUBSTATE_UNUSABLE || m_pHotSub->eState == SUBSTATE_FOREIGN)
	{
		PlayCantDropSound();
		return;
	}

	int nSlot = FindFreeSegment(m_pHotSub);
	if (nSlot == -1)
	{
		PlayCantDropSound();
		return;
	}

	int nSegments = 3 - m_pHotSub->eVersion;
	m_Libraries[0].RemoveSub(m_pHotSub);
	m_Libraries[1].RemoveSub(m_pHotSub);
	m_Libraries[2].RemoveSub(m_pHotSub);
	m_SystemMemoryCtrl.SetNumHighlightSegments(nSegments);
	m_pHotSub->nTempSlot = nSlot;
	m_SystemMemoryCtrl.OccupySegment(nSlot, m_pHotSub);
	m_SystemMemoryCtrl.SetNumHighlightSegments(-1);
	PlayDropIntoSound();

	if (IsKeyDown(VK_TAB) || m_bShowToolTips)
		m_ToolTip.Hide();

	OnMouseMove(m_CursorPos.x, m_CursorPos.y);
}

// RETAIL: 0x10080fb0, the first run of free segments the subroutine fits in
int CScreenSubroutines::FindFreeSegment(PlayerSubroutine * pSub)
{
	int nSegments = 3 - pSub->eVersion;
	int nSeg = 0;
	int nFree = 0;

	for (int nTries = 28; nTries > 0; nTries--)
	{
		if (m_SystemMemoryCtrl.GetSegmentObject(nSeg))
			nFree = 0;
		else
		{
			nFree++;
			if (nFree >= nSegments)
				return (nSeg - nSegments + 25) % 24;
		}

		nSeg++;
		if (nSeg == 24)
			nSeg = 0;
	}

	return -1;
}

// RETAIL: 0x10081020, a right click with a subroutine on the cursor puts it
// back into its library
void CScreenSubroutines::OnSubroutineRClick(int x, int y)
{
	PutSubroutineBack(true);

	int nFunction = m_pCursorSub->pTronSubroutine->eFunction;
	if (nFunction >= FUNCTION_COMBAT && nFunction <= FUNCTION_UTILITY)
	{
		CLibraryCtrl * pLibrary = &m_Libraries[nFunction];
		pLibrary->OnMouseMove(x, y, LTNULL);
		if (pLibrary->IsHot())
		{
			m_pHotSub = pLibrary->GetHotSub();
			if (m_pHotSub)
			{
				if (IsKeyDown(VK_TAB) || m_bShowToolTips)
				{
					LTIntPt pos = pLibrary->GetArc()->GetHotPos();
					ShowSubInfo(pos.x, pos.y);
					SetCursorMode(CursorPickup);
					return;
				}
			}
		}
		else
			pLibrary->Close();
	}

	SetCursorMode(CursorPickup);
}

// RETAIL: 0x10081100, a click on the duplicate subroutine dialog
void CScreenSubroutines::OnDuplicateClick(int x, int y)
{
	PlayerSubroutine * pKept = m_DuplicateDlg.GetKeptSubroutine(x, y);
	if (!pKept)
		return;

	m_DuplicateDlg.Hide();

	PlayerSubroutine * pRemoved = m_pDuplicateFirst;
	if (pRemoved == pKept)
		pRemoved = m_pDuplicateSecond;

	m_pDuplicateSecond = LTNULL;
	m_pDuplicateFirst = LTNULL;

	if (pRemoved->nTempSlot >= 0 && pRemoved->nTempSlot < 100)
		m_SystemMemoryCtrl.ClearSegment(pRemoved->nTempSlot);
	else if (pRemoved->nTempSlot >= 100 && pRemoved->nTempSlot < 200)
		m_Libraries[0].RemoveSub(pRemoved);
	else if (pRemoved->nTempSlot >= 200 && pRemoved->nTempSlot < 300)
		m_Libraries[1].RemoveSub(pRemoved);
	else if (pRemoved->nTempSlot >= 300 && pRemoved->nTempSlot < 400)
		m_Libraries[2].RemoveSub(pRemoved);

	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
	{
		if (*iter == pRemoved)
		{
			m_SubArray.erase(iter);
			break;
		}
	}

	g_pSubroutineMgr->RemovePlayerSubroutine(pRemoved);
}

// RETAIL: 0x10081220
LTBOOL CScreenSubroutines::OnLButtonDown(int x, int y)
{
	m_fLButtonDownTime = g_pLTClient->GetTime();

	if (m_PopupText.IsVisible())
	{
		m_PopupText.Close();
		return LTTRUE;
	}

	if (m_DuplicateDlg.IsVisible())
	{
		OnDuplicateClick(x, y);
		return LTTRUE;
	}

	switch (m_eCursorMode)
	{
	case CursorPickup:
		OnPickupClick(x, y);
		return LTTRUE;

	case CursorSubroutineDrop:
		OnSubroutineClick(x, y);
		return LTTRUE;
	}

	return CBaseScreen::OnLButtonDown(x, y);
}

// RETAIL: 0x100812d0
LTBOOL CScreenSubroutines::OnLButtonDblClick(int x, int y)
{
	if (m_eCursorMode == CursorPickup)
	{
		int i;
		for (i = 0; i < NUM_LIBRARIES; i++)
		{
			if (m_Libraries[i].IsRotateHot())
			{
				m_Libraries[i].OnClick();
				return LTTRUE;
			}
		}

		for (i = 0; i < NUM_RATINGS; i++)
		{
			if (m_RatingBars[i].IsHot())
			{
				m_RatingBars[i].OnLButtonDown(x, y);
				return LTTRUE;
			}
		}
	}

	return CBaseScreen::OnLButtonDblClick(x, y);
}

// RETAIL: 0x10081380, a drag ends where the button comes up
LTBOOL CScreenSubroutines::OnLButtonUp(int x, int y)
{
	if (!m_pCursorSub || g_pLTClient->GetTime() - m_fLButtonDownTime < 0.5f)
		return CBaseScreen::OnLButtonUp(x, y);

	switch (m_eCursorMode)
	{
	case CursorPickup:
		OnPickupClick(x, y);
		break;

	case CursorSubroutineDrop:
		OnSubroutineClick(x, y);
		UpdateAnimSound();
		return LTTRUE;
	}

	UpdateAnimSound();
	return LTTRUE;
}

// RETAIL: 0x10081420
LTBOOL CScreenSubroutines::OnRButtonDown(int x, int y)
{
	if (m_PopupText.IsVisible())
	{
		m_PopupText.Close();
		return LTTRUE;
	}

	switch (m_eCursorMode)
	{
	case CursorPickup:
		OnPickupRClick(x, y);
		break;

	case CursorSubroutineDrop:
		OnSubroutineRClick(x, y);
		break;
	}

	UpdateAnimSound();
	return CBaseScreen::OnRButtonDown(x, y);
}

// RETAIL: 0x10081490, the subroutine on the cursor goes back where it was
void CScreenSubroutines::PutSubroutineBack(bool bKeepIcon)
{
	if (!m_pCursorSub)
		return;

	if (m_pCursorSub->eState == SUBSTATE_UNUSABLE)
	{
		m_SystemMemoryCtrl.ShowIcon(m_pCursorSub->nTempSlot, true);
		return;
	}

	if (m_pCursorSub->nTempSlot >= 0 && m_pCursorSub->nTempSlot < 24)
	{
		m_SystemMemoryCtrl.SetNumHighlightSegments(3 - m_pCursorSub->eVersion);
		m_SystemMemoryCtrl.OccupySegment(m_pCursorSub->nTempSlot, m_pCursorSub);
		if (!bKeepIcon)
			m_SystemMemoryCtrl.ShowIcon(m_pCursorSub->nTempSlot, false);
		PlayDropIntoSound();
		return;
	}

	SubFunction eFunction = m_pCursorSub->pTronSubroutine->eFunction;
	if (eFunction == FUNCTION_COMBAT)
	{
		m_Libraries[0].InsertSub(m_pCursorSub);
		m_Libraries[0].ShowSubIcon(m_pCursorSub, bKeepIcon);
		m_Libraries[0].SetStayOpen(false);
	}
	else if (eFunction == FUNCTION_DEFENSE)
	{
		m_Libraries[1].InsertSub(m_pCursorSub);
		m_Libraries[1].ShowSubIcon(m_pCursorSub, bKeepIcon);
		m_Libraries[1].SetStayOpen(false);
	}
	else if (eFunction == FUNCTION_UTILITY)
	{
		m_Libraries[2].InsertSub(m_pCursorSub);
		m_Libraries[2].ShowSubIcon(m_pCursorSub, bKeepIcon);
		m_Libraries[2].SetStayOpen(false);
	}

	PlayDropIntoLibrarySound();
}

// RETAIL: 0x100815d0
void CScreenSubroutines::SetCursorMode(SubCursorMode mode)
{
	if (m_eCursorMode == mode)
		return;

	switch (m_eCursorMode)
	{
	case CursorSubroutineDrop:
		{
		g_pCursorMgr->UseSprite("Interface\\cursor.dtx");
		g_pCursorMgr->UseGlowSprite("Interface\\cursor_glow.dtx");
		g_pCursorMgr->SetCenter(32, 32);

		char szFX[128];
		szFX[0] = 0;
		g_pLayoutMgr->GetScreenSelectFX((eScreenID)m_nScreenID, szFX, sizeof(szFX));
		g_pInterfaceMgr->SetMouseFX(szFX);

		m_SystemMemoryCtrl.SetNumHighlightSegments(-1);
		OnMouseMove(m_CursorPos.x, m_CursorPos.y);

		for (int i = 0; i < NUM_PROC_CTRLS; i++)
			m_ProcCtrl[i].SetHighlight(false);
		}
		break;
	}

	switch (mode)
	{
	case CursorPickup:
		m_pCursorSub = LTNULL;
		break;

	case CursorSubroutineDrop:
		{
			g_pCursorMgr->UseSprite((CScreenSprite*)LTNULL);
			if (m_pCursorSub->eState == SUBSTATE_FOREIGN)
				g_pInterfaceMgr->SetMouseFX("SUBICON_foreign", true);
			else
				g_pInterfaceMgr->SetMouseFX(m_pCursorSub->pTronSubroutine->szCursorFX, true);

			for (int i = 0; i < NUM_PROC_CTRLS; i++)
			{
				Procedural * pProc = m_ProcCtrl[i].GetProc();
				if (pProc && pProc->eAffectState == m_pCursorSub->eState && !pProc->pSub)
					m_ProcCtrl[i].SetHighlight(true);
			}
		}
		break;
	}

	m_eCursorMode = mode;
}

// RETAIL: 0x10081750, the tool tip of the hot subroutine
void CScreenSubroutines::ShowSubInfo(int x, int y)
{
	if (!m_pHotSub || !m_pHotSub->pTronSubroutine)
		return;

	std::wstring sInfo = g_pSubroutineMgr->GetSubroutineInfo(m_pHotSub);

	char szIcon[256];
	if (m_pHotSub->eState != SUBSTATE_FOREIGN)
		sprintf(szIcon, "%s", m_pHotSub->pTronSubroutine->szSprite);
	else
		sprintf(szIcon, "%s", "Interface\\Subroutines\\Sprtex\\foreign_0.dtx");

	m_ToolTip.Show(x, y, sInfo, g_pInterfaceResMgr->GetTexture(szIcon));
}

// RETAIL: 0x10081890, the tool tip of the hot procedural
void CScreenSubroutines::ShowProcInfo(int x, int y)
{
	if (!m_pHotProc)
		return;

	HTEXTURE hIcon = g_pInterfaceResMgr->GetTexture(m_pHotProc->szToolTipIcon);
	std::wstring sInfo = g_pSubroutineMgr->GetProceduralInfo(m_pHotProc);
	m_ToolTip.Show(x, y, sInfo, hIcon);
}

// RETAIL: 0x10081990
void CScreenSubroutines::ShowRatingInfo(int x, int y)
{
	if (!m_pHotRating)
		return;

	std::wstring sInfo = m_pHotRating->GetHelpText();
	m_ToolTip.Show(x, y, sInfo, LTNULL);
}

// RETAIL: 0x10081a70
void CScreenSubroutines::ShowBuildPointsInfo(int x, int y)
{
	if (!m_bBuildPointsHot)
		return;

	std::wstring sInfo = m_BuildPoints.GetHelpText();
	m_ToolTip.Show(x, y, sInfo, LTNULL);
}

// RETAIL: 0x10081b60, the tool tips of the screen's own controls
void CScreenSubroutines::ShowCtrlInfo(int x, int y, CLTGUICtrl * pCtrl)
{
	if (!pCtrl)
		return;

	if (pCtrl == m_pClear)
		m_ToolTip.Show(x, y, LoadTempWString(0xcee), LTNULL);
	else if (pCtrl == m_pToolTipTitle || pCtrl == m_pToolTipToggle)
		m_ToolTip.Show(x, y, LoadTempWString(0xcef), LTNULL);
	else if (pCtrl == m_pVersion)
		m_ToolTip.Show(x, y, LoadTempWString(0xcf3), LTNULL);
	else if (pCtrl == m_pBuildNoteCount)
		m_ToolTip.Show(x, y, LoadTempWString(0xcf4), LTNULL);
	else if (pCtrl == m_pSysName)
		m_ToolTip.Show(x, y, LoadTempWString(0xcf0), LTNULL);
}

// RETAIL: 0x10081c20
void CScreenSubroutines::ScreenDimsChanged()
{
	float fRatio = g_pInterfaceResMgr->GetYRatio();
	m_ToolTip.m_nFontSize = (int)((float)m_nToolTipFontSize * fRatio);
	m_ToolTip.m_nOffset = (int)(fRatio * 40.0f);
	m_ToolTip.m_nWidth = (int)(m_fToolTipWidth * fRatio);

	m_SystemMemoryCtrl.Rescale();
	m_Libraries[0].Rescale();
	m_Libraries[1].Rescale();
	m_Libraries[2].Rescale();

	int i;
	for (i = 0; i < NUM_RATINGS; i++)
		m_RatingBars[i].ScreenDimsChanged();
	m_BuildPoints.ScreenDimsChanged();

	for (i = 0; i < NUM_PROC_CTRLS; i++)
	{
		m_Libraries[i].Rescale();
		m_ProcCtrl[i].ScreenDimsChanged();
	}

	m_Optimizer.ScreenDimsChanged();

	CBaseScreen::ScreenDimsChanged();
}

// RETAIL: 0x10081d00, can the subroutine on the cursor go into the hot
// segment of the system memory?
bool CScreenSubroutines::CanDropHere()
{
	PlayerSubroutine * pCursorSub = m_pCursorSub;
	if (pCursorSub->eState == SUBSTATE_UNUSABLE)
		return (m_SystemMemoryCtrl.GetActiveSegment() == pCursorSub->nTempSlot);

	if (pCursorSub->eState == SUBSTATE_FOREIGN)
		return false;

	PlayerSubroutine * pDisplaced = LTNULL;
	int nSegments = 3 - pCursorSub->eVersion;
	for (int i = 0; i < nSegments; i++)
	{
		PlayerSubroutine * pSub = m_SystemMemoryCtrl.GetSegmentObject((m_SystemMemoryCtrl.GetActiveSegment() + i) % 24);
		if (pSub)
		{
			if (pSub->eState == SUBSTATE_UNUSABLE)
				return false;
			if (pSub != pDisplaced)
			{
				if (pDisplaced)
					return false;
				pDisplaced = pSub;
			}
		}
	}

	return true;
}

// RETAIL: 0x10081d90, a loop sounds while a subroutine is hot or on the
// cursor
void CScreenSubroutines::UpdateAnimSound()
{
	bool bAnimating = ((m_pHotSub && m_pHotSub->pTronSubroutine->eFunction != FUNCTION_BASECODE) ||
		(m_pCursorSub && m_pCursorSub->pTronSubroutine->eFunction != FUNCTION_BASECODE));

	if (m_hAnimSound && !bAnimating)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hAnimSound);
		m_hAnimSound = LTNULL;
		return;
	}

	if (!m_hAnimSound && bAnimating)
		m_hAnimSound = g_pClientSoundMgr->PlaySoundLocal("animatingsubroutine", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
}
