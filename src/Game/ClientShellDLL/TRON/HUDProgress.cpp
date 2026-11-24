// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProgress.cpp
//
// PURPOSE : HUDItem to display objectives and working procedurals
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientRes.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"
#include "SubroutineMgr.h"
#include "ClientSoundMgr.h"

// TODO mark objectives that are complete or just remove them?

namespace
{
	char * szUVNames[] = 
	{
		"UVFrameTopLeft",
		"UVFrameTop",
		"UVFrameTopRight",
		"UVFrameLeft",
		"UVFrameBottomLeft",
		"UVFrameBottom",
		"UVBoxTopLeft",
		"UVBoxTop",
		"UVBoxTopRight",
		"UVBoxLeft",
//		"UVBoxLeftInset",
		"UVBoxCenter",
		"UVBoxRight",
		"UVBoxBottomLeft",
		"UVBoxBottom",
		"UVBoxBottomRight",
	};
};

//******************************************************************************************
//**
//** HUD Progress
//**
//******************************************************************************************

// RETAIL: 0x10032de0
CHUDProgress::CHUDProgress()
{
	// Make this something new
	m_UpdateFlags = kHUDObjectives;
	m_fScale = 0.0f;
	m_bShow = false;
	m_nNumObjectives = 0;
	m_nNumProcs = 0;
	m_pTitleStr = LTNULL;
	int i;
	for (i = 0; i < HUDPROGRESS_MAX_OBJECTIVES; i++)
	{
		m_pObjectiveStr[i] = m_pCompletedObjectiveStr[i] = m_pSecondaryObjectiveStr[i] = LTNULL;
	}
	for (i = 0; i < HUDPROGRESS_MAX_PROCS; i++)
	{
		m_pProcStr[i] = m_pSubroutineStr[i] = LTNULL;
		m_hProcTex[i] = m_hSubTex[i] = LTNULL;
	}
}

// RETAIL: 0x100330f0
LTBOOL CHUDProgress::Init()
{
	m_BoxFrame.Init("chooserbox");
	m_OuterFrame.Init("chooserframe");

	m_hProgressTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\hud.dtx");
	m_hBarTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\progressbar_alpha.dtx");

	ReadLayout();
	UpdateLayout();

	// Build the "Progress" string
	// RETAIL: the narrow string is loaded, and not used
	char * szStr = LoadTempString(IDS_PROGRESS);
	CUIFont * pFont = g_pInterfaceResMgr->GetFontForHeight(m_iTitleFontSize);
	m_pTitleStr = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont,LoadTempWString(IDS_PROGRESS),0.0f, 0.0f);
	m_pTitleStr->SetCharScreenHeight(m_iTitleFontSize);
	m_pTitleStr->SetColor(m_iTitleFontColor);

	// Build the "Optional" string
	pFont = g_pInterfaceResMgr->GetFontForHeight(m_iObjectiveFontSize);
	m_pOptionalStr = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont,LoadTempWString(IDS_OPTIONAL_OBJECTIVES),0.0f, 0.0f);
	m_pOptionalStr->SetCharScreenHeight(m_iObjectiveFontSize);
	m_pOptionalStr->SetColor(m_iTitleFontColor);

	return LTTRUE;
}

// RETAIL: 0x10033320
void CHUDProgress::Term()
{
	// String cleanup
	if (m_pTitleStr)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pTitleStr);
		m_pTitleStr = LTNULL;
	}
}

// RETAIL: 0x10033360: shown while the progress command is on; the new
// objectives' insets flash for five seconds after the box opens
void CHUDProgress::Render()
{
	if (IsMultiplayerGame())
		return;

	if (!IsEnabled())
		return;

	bool bWasShown = m_bShow;
	uint32 nFlashColor = argbWhite;

	m_bShow = g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_PROGRESS);
	if (!m_bShow)
	{
		if (bWasShown)
		{
			// the box closed: the objectives are seen
			g_pTronPlayerStats->GetTronUnknownList9C()->Clear();
			for (int i = 0; i < HUDPROGRESS_MAX_OBJECTIVES; i++)
			{
				m_bNewObjective[i] = m_bNewSecondaryObjective[i] = m_bNewCompletedObjective[i] = false;
			}
		}
		return;
	}

	if (!bWasShown)
	{
		// the box opened
		m_fFlashTime = 5.0f;
		m_fLastFlashTime = g_pLTClient->GetTime();
		g_pTronPlayerStats->TronUnknownClearObjectivesPing();
		g_pClientSoundMgr->PlaySoundLocal("mission_obj", SOUNDPRIORITY_MISC_MEDIUM);
	}
	else if (m_fFlashTime > 0.0f)
	{
		float fTime = g_pLTClient->GetTime();
		m_fFlashTime -= fTime - m_fLastFlashTime;
		m_fLastFlashTime = fTime;

		float fFlash = m_fFlashTime * 2.0f;
		float fPulse = (fFlash - (float)floor(fFlash)) * 0.5f;
		uint8 nAlpha = (uint8)((fPulse + 0.5f) * 255.0f);
		nFlashColor = SET_ARGB(0xFF, nAlpha, nAlpha, nAlpha);
	}

	SetRenderState();

	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	g_pDrawPrim->SetTexture(m_hProgressTex);
	m_BoxFrame.Render();
	m_OuterFrame.Render();

	// Place insets by each objective
	if (m_nNumObjectives || m_nNumSecondaryObjectives || m_nNumCompletedObjectives)
	{
		g_pDrawPrim->SetTexture(m_hProgressTex);

		int i;
		if (m_nNumObjectives)
		{
			for (i = 0; i < m_nNumObjectives; i++)
			{
				if (m_fFlashTime > 0.0f && m_bNewObjective[i])
					g_pDrawPrim->SetRGBA(&m_ObjectiveInset[i], nFlashColor);
				else
					g_pDrawPrim->SetRGBA(&m_ObjectiveInset[i], argbWhite);
			}
			g_pDrawPrim->SetTexture(m_hProgressTex);
			g_pDrawPrim->DrawPrim(m_ObjectiveInset, m_nNumObjectives);
		}

		if (m_nNumSecondaryObjectives)
		{
			for (i = 0; i < m_nNumSecondaryObjectives; i++)
			{
				if (m_fFlashTime > 0.0f && m_bNewSecondaryObjective[i])
					g_pDrawPrim->SetRGBA(&m_SecondaryInset[i], nFlashColor);
				else
					g_pDrawPrim->SetRGBA(&m_SecondaryInset[i], argbWhite);
			}
			g_pDrawPrim->SetTexture(m_hProgressTex);
			g_pDrawPrim->DrawPrim(m_SecondaryInset, m_nNumSecondaryObjectives);

			// While placing insets, also place a separator for the secondary objectives
			g_pDrawPrim->SetTexture(m_hProgressTex);
			g_pDrawPrim->DrawPrim(&m_SeparatorPrim, 1);
		}

		if (m_nNumCompletedObjectives)
		{
			g_pDrawPrim->SetTexture(m_hProgressTex);
			g_pDrawPrim->DrawPrim(m_CompletedInset, m_nNumCompletedObjectives);
		}
	}

	if (m_pTitleStr)
	{
		m_pTitleStr->Render();
	}

	if (m_nNumObjectives)
	{
		for (int i = 0; i < m_nNumObjectives; i++)
		{
			m_pObjectiveStr[i]->Render();
		}
	}
	if (m_nNumCompletedObjectives)
	{
		for (int i = 0; i < m_nNumCompletedObjectives; i++)
		{
			m_pCompletedObjectiveStr[i]->Render();
		}
	}
	if (m_nNumSecondaryObjectives)
	{
		m_pOptionalStr->Render();
		for (int i = 0; i < m_nNumSecondaryObjectives; i++)
		{
			m_pSecondaryObjectiveStr[i]->Render();
		}
	}
}

// RETAIL: 0x10033760: Tron's shows no procedurals here (m_nNumProcs stays
// 0); the objectives the player has not seen yet flash
void CHUDProgress::Update()
{
	uint32 dwUpdateFlags = g_pHUDMgr->QueryUpdateFlags();

	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	m_nNumProcs = 0;

	if (dwUpdateFlags && kHUDObjectives)
	{
		// clear the local list of strings
		int i;
		for (i = 0; i < HUDPROGRESS_MAX_OBJECTIVES; i++)
		{
			if (m_pObjectiveStr[i])
			{
				g_pLTClient->GetFontManager()->DestroyPolyString(m_pObjectiveStr[i]);
				m_pObjectiveStr[i] = LTNULL;
			}
		}
		m_nNumObjectives = 0;
		m_nNumCompletedObjectives = 0;
		m_nNumSecondaryObjectives = 0;

		IDList* pNew = g_pTronPlayerStats->GetTronUnknownList9C();
		uint8 nIndex;

		// query TronPlayerStats for the objectives
		// add required objectives
		CUIFont * pFont = g_pInterfaceResMgr->GetFontForHeight(m_iObjectiveFontSize);
		IDList* pObj = g_pPlayerStats->GetObjectives();
		for (i = pObj->m_IDArray.size()-1; i >= 0; i--)
		{
			if (m_nNumObjectives < HUDPROGRESS_MAX_OBJECTIVES)
			{
				uint32 objID = pObj->m_IDArray[i];
				m_pObjectiveStr[i] = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, LoadTempWString(objID), 0.0f, 0.0f);
				m_pObjectiveStr[i]->SetCharScreenHeight(m_iObjectiveFontSize);
				m_pObjectiveStr[i]->SetColor(m_iObjectiveFontColor);
				m_bNewObjective[i] = pNew->Have(objID, nIndex) ? true : false;
				m_nNumObjectives++;
			}
		}
		// completed objectives
		pObj = g_pPlayerStats->GetCompletedObjectives();
		for (i = pObj->m_IDArray.size()-1; i >= 0; i--)
		{
			if (m_nNumCompletedObjectives < HUDPROGRESS_MAX_OBJECTIVES)
			{
				uint32 objID = pObj->m_IDArray[i];
				m_pCompletedObjectiveStr[i] = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, LoadTempWString(objID), 0.0f, 0.0f);
				m_pCompletedObjectiveStr[i]->SetCharScreenHeight(m_iObjectiveFontSize);
				m_pCompletedObjectiveStr[i]->SetColor(m_iCompletedObjectiveFontColor);
				m_bNewCompletedObjective[i] = pNew->Have(objID, nIndex) ? true : false;
				m_nNumCompletedObjectives++;
			}
		}
		// Secondary objectives
		pObj = g_pPlayerStats->GetOptionalObjectives();
		for (i = pObj->m_IDArray.size()-1; i >= 0; i--)
		{
			if (m_nNumSecondaryObjectives < HUDPROGRESS_MAX_OBJECTIVES)
			{
				uint32 objID = pObj->m_IDArray[i];
				m_pSecondaryObjectiveStr[i] = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, LoadTempWString(objID), 0.0f, 0.0f);
				m_pSecondaryObjectiveStr[i]->SetCharScreenHeight(m_iObjectiveFontSize);
				m_pSecondaryObjectiveStr[i]->SetColor(m_iSecondaryObjectiveFontColor);
				m_bNewSecondaryObjective[i] = pNew->Have(objID, nIndex) ? true : false;
				m_nNumSecondaryObjectives++;
			}
		}
	}
	UpdateGraphics();
}

// RETAIL: 0x10033c00
int CHUDProgress::GetPieceWidth(ePrimType eType)
{
	return (m_rcPrimDims[eType].right - m_rcPrimDims[eType].left + 1);

}

// RETAIL: 0x10033c20
int CHUDProgress::GetPieceHeight(ePrimType eType)
{
	return (m_rcPrimDims[eType].bottom - m_rcPrimDims[eType].top + 1);

}

// RETAIL: 0x10033c40: the text, the insets and the two frames, from the
// bottom of the box up
void CHUDProgress::UpdateGraphics()
{
	// compute the height of the box here.
	int iBoxTop = g_pInterfaceResMgr->GetScreenHeight() - m_iHUDOffset;
	int iBottom = iBoxTop;
	int i;

	// Compute the width of the text area
	// then use it to constrain all of the polystrings
	LTRect rcMargins = m_BoxFrame.GetMargins();
	int iTextX = m_rcProgress.left + GetPieceWidth(FRAME_LEFT) + rcMargins.left + 8;
	int iTextWidth = m_rcProgress.right - rcMargins.right - iTextX;

	float fInsetX = (float)GetPieceWidth(FRAME_LEFT) + m_rcProgress.left - 2.0f;

	// Position the text and elements of the procedural display
	if (m_nNumProcs)
	{
		// RETAIL: the frames go to the active procedurals in turn
		int iFrame = 0;
		for (i = 0; i < HUDPROGRESS_MAX_PROCS; i++)
		{
			if (m_bIsProcActive[i])
			{
				iBoxTop -= m_iProceduralHeight + 2;

				m_pProcStr[i]->SetPosition(iTextX + 48, iBoxTop + 6);
				m_pSubroutineStr[i]->SetPosition(iTextX + 48, iBoxTop + m_iProceduralHeight - 19);

				// position the procedural icon and the subroutine icon
				float fTop = (float)iBoxTop;
				g_pDrawPrim->SetXYWH(&m_ProcPrim[i], (float)iTextX - 12.0f, fTop, 64.0f, 64.0f);
				g_pDrawPrim->SetXYWH(&m_SubPrim[i], (float)(iTextX + iTextWidth) - 64.0f, fTop, 64.0f, 64.0f);

				// the progress bar fills the procedural's frame
				LTRect rcProc = m_ProcFrame[iFrame].GetRect();
				iFrame++;
				g_pDrawPrim->SetXYWH(&m_ProgressBar[i], (float)rcProc.left, (float)rcProc.top, (float)(rcProc.right - rcProc.left) * m_fProcPercent[i], (float)(rcProc.bottom - rcProc.top));

				g_pDrawPrim->SetXYWH(&m_ProcInset[i], fInsetX, fTop + 4.0f, m_fInsetWidth, m_fInsetHeight);
			}
		}
	}

	iBoxTop -= 4;

	int32 nWidth, nHeight;
	if (m_nNumSecondaryObjectives)
	{
		for (i = m_nNumSecondaryObjectives - 1; i >= 0; i--)
		{
			m_pSecondaryObjectiveStr[i]->SetWrapWidth(iTextWidth);
			m_pSecondaryObjectiveStr[i]->GetDims(&nWidth, &nHeight);
			iBoxTop -= (int)(float)nHeight + m_iObjectiveSpacing;
			m_pSecondaryObjectiveStr[i]->SetPosition(iTextX, iBoxTop);
			g_pDrawPrim->SetXYWH(&m_SecondaryInset[i], fInsetX, (float)iBoxTop, m_fInsetWidth, m_fInsetHeight);
		}

		// Place the text
		m_pOptionalStr->GetDims(&nWidth, &nHeight);
		float fHeight = (float)nHeight;
		iBoxTop -= (int)fHeight + m_iObjectiveSpacing;
		float fTextWidth = (float)iTextWidth;
		float fTextX = (float)iTextX;
		m_pOptionalStr->SetPosition((int)((fTextWidth - nWidth) * 0.5f + fTextX), iBoxTop);

		// Place the drawprim
		g_pDrawPrim->SetXYWH(&m_SeparatorPrim, fTextX, (float)(iBoxTop + (fHeight / 2) - 1), fTextWidth, 4.0f);
	}

	if (m_nNumObjectives)
	{
		for (i = m_nNumObjectives - 1; i >= 0; i--)
		{
			m_pObjectiveStr[i]->SetWrapWidth(iTextWidth);
			m_pObjectiveStr[i]->GetDims(&nWidth, &nHeight);
			iBoxTop -= (int)(float)nHeight + m_iObjectiveSpacing;
			m_pObjectiveStr[i]->SetPosition(iTextX, iBoxTop);
			g_pDrawPrim->SetXYWH(&m_ObjectiveInset[i], fInsetX, (float)iBoxTop, m_fInsetWidth, m_fInsetHeight);
		}
	}

	if (m_nNumCompletedObjectives)
	{
		for (i = m_nNumCompletedObjectives - 1; i >= 0; i--)
		{
			m_pCompletedObjectiveStr[i]->SetWrapWidth(iTextWidth);
			m_pCompletedObjectiveStr[i]->GetDims(&nWidth, &nHeight);
			iBoxTop -= (int)(float)nHeight + m_iObjectiveSpacing;
			m_pCompletedObjectiveStr[i]->SetPosition(iTextX, iBoxTop);
			g_pDrawPrim->SetXYWH(&m_CompletedInset[i], fInsetX, (float)iBoxTop, m_fInsetWidth, m_fInsetHeight);
		}
	}

	// Position the frames around the box
	int iLeft = m_rcProgress.left;
	int iTop = iBoxTop - rcMargins.top - GetPieceHeight(FRAME_TOP);

	// Take a moment here to position the title...
	if (m_pTitleStr)
	{
		// center it over the top of the frame
		int32 nTitleWidth, nTitleHeight;
		m_pTitleStr->GetDims(&nTitleWidth, &nTitleHeight);
		float fTitleHeight = (float)nTitleHeight;
		float ty = iTop + (GetPieceHeight(FRAME_TOP_LEFT) * 0.5f) - (fTitleHeight * 0.5f);
		ty -= 3;
		m_pTitleStr->SetPosition(iLeft + 10, (int)ty);
	}

	float fY = (float)GetPieceHeight(FRAME_TOP_LEFT) + iTop;
	float fX = (float)GetPieceWidth(FRAME_LEFT) + iLeft - 2.0f;
	float fBottom = (float)iBottom;
	int iFrameBottom = (int)(fBottom - GetPieceHeight(FRAME_BOTTOM)) - 4;

	LTRect rcFrameMargins = m_OuterFrame.GetMargins();
	int x = (int)fX;
	int y = (int)fY;
	m_OuterFrame.SetRect(LTRect(x, y, m_rcProgress.right - rcFrameMargins.right, iFrameBottom));

	iFrameBottom -= (m_iProceduralHeight + 2) * m_nNumProcs;
	m_BoxFrame.SetRect(LTRect(x, y, m_rcProgress.right, iFrameBottom));
}

// RETAIL: 0x100342d0: the prims' colors and texture rects, and the
// procedurals' frames
void CHUDProgress::UpdateLayout()
{
	m_fInsetWidth = (float)(m_InsetPrimDims.right - m_InsetPrimDims.left) + 1.0f;
	m_fInsetHeight = (float)(m_InsetPrimDims.bottom - m_InsetPrimDims.top) + 1.0f;

	int i;
	for (i = 0; i < HUDPROGRESS_MAX_PROCS; i++)
	{
		g_pDrawPrim->SetUVWH(&m_ProcPrim[i], 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetUVWH(&m_SubPrim[i], 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetUVWH(&m_ProgressBar[i], 0.0f, 0.0f, 1.0f, 1.0f);

		g_pDrawPrim->SetRGBA(&m_ProcPrim[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_SubPrim[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_ProgressBar[i], argbWhite);

		m_ProcFrame[i].Init("chooserbox");
	}

	for (i = 0; i < HUDPROGRESS_MAX_OBJECTIVES; i++)
	{
		g_pDrawPrim->SetRGBA(&m_ObjectiveInset[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_SecondaryInset[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_ProcInset[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_CompletedInset[i], argbWhite);

		SetUVWH(&m_ObjectiveInset[i], m_InsetPrimDims);
		SetUVWH(&m_SecondaryInset[i], m_InsetPrimDims);
		SetUVWH(&m_ProcInset[i], m_InsetPrimDims);
		SetUVWH(&m_CompletedInset[i], m_InsetFullPrimDims);
	}

	// Separator between mandatory and optional
	g_pDrawPrim->SetRGBA(&m_SeparatorPrim, argbWhite);
	SetUVWH(&m_SeparatorPrim, m_rcSeparator);

	for (i = 0; i < LAST_PRIM; i++)
	{
		g_pDrawPrim->SetRGBA(&m_PrimArray[i], argbWhite);
		SetUVWH(&m_PrimArray[i], m_rcPrimDims[i]);
	}

	// Position the boxes for procedurals
	int iTop = g_pInterfaceResMgr->GetScreenHeight() - m_iHUDOffset;
	LTRect rcProc;
	rcProc.left = m_rcProgress.left + (m_rcPrimDims[FRAME_LEFT].right - m_rcPrimDims[FRAME_LEFT].left + 1) - 2;
	rcProc.right = m_rcProgress.right;

	for (i = 0; i < HUDPROGRESS_MAX_PROCS; i++)
	{
		rcProc.top = iTop - m_iProceduralHeight;
		rcProc.bottom = iTop;
		m_ProcFrame[i].SetRect(rcProc);
		iTop -= m_iProceduralHeight + 2;
	}
}

// RETAIL: 0x10034620
void CHUDProgress::ReadLayout()
{
	m_iHUDOffset = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressHUDOffset();

	// Overall dimensions.
	// Note that these dimensions are for a base box in the bottom left corner
	// and that the box can be grown up vertically as needed.
	m_rcProgress = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("ProgressRect");

	m_InsetPrimDims = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("UVBoxLeftInset");
	m_InsetFullPrimDims = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("UVBoxLeftFullInset");
	m_rcSeparator = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("UVSeparator");

	for (int i = 0; i < LAST_PRIM; i++)
	{
		m_rcPrimDims[i] = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect(szUVNames[i]);
	}

	m_iTitleFont = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressTitleFont();
	m_iTitleFontSize = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressTitleFontSize();
	m_iTitleFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressTitleColor();

	m_iObjectiveFont = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressObjectiveFont();
	m_iObjectiveFontSize = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressObjectiveFontSize();
	m_iObjectiveFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressObjectiveColor();
	m_iObjectiveSpacing = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressObjectiveSpacing();
	m_iCompletedObjectiveFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressCompletedObjectiveColor();
	m_iSecondaryObjectiveFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressSecondaryObjectiveColor();

	m_iProceduralFont = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressProceduralFont();
	m_iProceduralFontSize = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressProceduralFontSize();
	m_iProceduralFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressProceduralColor();
	m_iProceduralHeight = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressProceduralHeight();
}

void CHUDProgress::SetUVWH(LT_POLYFT4 * pPrim, LTRect rect)
{
	rect.right -= rect.left;
	rect.bottom -= rect.top;

	// RETAIL: Tron's HUD textures are 256 wide (retail multiplies by 1/256;
	// Build 69's divided by 512)
	g_pDrawPrim->SetUVWH(pPrim, rect.left / 256.0f, rect.top / 256.0f, rect.right / 256.0f, rect.bottom / 256.0f);
}
