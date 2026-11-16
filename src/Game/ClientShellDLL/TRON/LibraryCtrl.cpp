// ----------------------------------------------------------------------- //
//
// MODULE  : LibraryCtrl.cpp
//
// PURPOSE : One subroutine library of the subroutine screen
//
// RETAIL: Tron 2.0 1.030's, written from retail CShell.dll
// (0x10090fb0..0x10091ddb). The names are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LibraryCtrl.h"
#include "InterfaceMgr.h"
#include "InterfaceResMgr.h"
#include "ClientSoundMgr.h"
#include "ClientUtilities.h"
#include "cuipolystring.h"
#include "iltfontmanager.h"

extern ILTFontManager* g_pFontManager;

namespace
{
	const float LIBRARY_INNER_RADIUS = 178.0f;
	const float LIBRARY_OUTER_RADIUS = 226.0f;

	const int	LIBRARY_SEGMENTS = 6;
	const int	LIBRARY_MAX_SUBS = 100;

	// RETAIL: the libraries' titles are CRes.dll strings 3408..3410
	const int	IDS_LIBRARY_TITLE = 3408;

	const float LIBRARY_FADE_TIME = 0.5f;

	// RETAIL: folds with RatingBarCtrl.cpp's (0x1004fae0)
	void PlayMouseOverSound()
	{
		g_pLTClient->CPrint("interface\\subroutines\\snd\\mouseover.wav");
		g_pClientSoundMgr->PlayInterfaceSound("interface\\subroutines\\snd\\mouseover.wav");
	}
}

// arcs of text (ProceduralCtrl.cpp)
extern void ArcPolyString(CUIPolyString * pStr, int x, int y, float fRadius, float fAngle,
				   bool bOutside, int nJustify);

// ----------------------------------------------------------------------- //

CLibraryCtrl::CLibraryCtrl()
{
	m_bInit = false;
	m_bHot = false;
	m_bWasHot = false;
	m_pHotSub = LTNULL;
	m_nLastIndex = 5;
	m_bStayOpen = false;
	m_pTitle = LTNULL;
	m_fTitleAngle = 0.0f;
	m_nTitleAlpha = 0x66;
	m_bFlash = false;
	m_fFlashTime = 0.0f;
}

CLibraryCtrl::~CLibraryCtrl()
{
	Term();
}

// ----------------------------------------------------------------------- //

void CLibraryCtrl::Init(int nFunction, float fArcStart, float fArcEnd)
{
	m_SubArray.clear();

	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	m_nFunction = nFunction;

	m_Arc.Init(LTIntPt(320, 240), LIBRARY_INNER_RADIUS, LIBRARY_OUTER_RADIUS, LIBRARY_SEGMENTS);
	m_Arc.SetArcType(ARC_TYPE_LIBRARY);
	m_Arc.SetArc(fArcStart, fArcEnd);
	m_Arc.SetNumHighlightSegments(1);
	m_Arc.ShowOutlines(LTFALSE);
	m_Arc.AssociateModel(LTNULL);
	m_Arc.ShowSprites(false);

	m_eState = LIBRARY_STATE_CLOSED;
	m_bHot = false;
	m_bCCWHot = false;
	m_bCWHot = false;
	m_nLastIndex = 5;

	if (m_pTitle)
	{
		g_pFontManager->DestroyPolyString(m_pTitle);
		m_pTitle = LTNULL;
	}

	int nHeight = (int)(fYRatio * 36.0f);
	CUIFont * pFont = g_pInterfaceResMgr->GetFontForHeight(nHeight);
	m_pTitle = g_pFontManager->CreatePolyString(pFont, LoadTempWString(IDS_LIBRARY_TITLE + nFunction), 0.0f, 0.0f);
	m_pTitle->SetCharScreenHeight(nHeight);
	m_pTitle->SetColor(((uint32)m_nTitleAlpha << 24) | 0xFF);

	// the title goes in the middle of the arc
	if (fArcEnd < fArcStart)
		fArcEnd += 360.0f;
	fArcStart = (fArcEnd + fArcStart) * 0.5f;
	m_fTitleAngle = fArcStart;

	m_fTitleRadius = ((LIBRARY_OUTER_RADIUS - LIBRARY_INNER_RADIUS) * 0.25f + LIBRARY_INNER_RADIUS) * fXRatio;
	m_fTitleHalfRadius = m_fTitleRadius * 0.5f;
	m_Center.x = (int)(fXRatio * 320.0f);
	m_Center.y = (int)(fYRatio * 240.0f);

	ArcPolyString(m_pTitle, m_Center.x, m_Center.y, m_fTitleRadius, fArcStart, m_nFunction == FUNCTION_COMBAT, 0);

	m_bInit = true;
	m_bVisible = false;
}

void CLibraryCtrl::Rescale()
{
	m_Arc.Rescale();
}

void CLibraryCtrl::AssociateModel(char * szPiece)
{
	m_Model.Create(szPiece);
	m_Arc.AssociateModel(szPiece);
}

void CLibraryCtrl::Term()
{
	if (!m_bInit)
		return;

	Clear();
	m_Model.Term();

	if (m_pTitle)
	{
		g_pFontManager->DestroyPolyString(m_pTitle);
		m_pTitle = LTNULL;
	}

	m_bInit = false;
}

void CLibraryCtrl::Clear()
{
	m_SubArray.clear();
	m_Arc.Build();
}

// ----------------------------------------------------------------------- //

void CLibraryCtrl::OnMouseMove(int x, int y, PlayerSubroutine * pCursorSub)
{
	bool bWasCWHot = m_bCWHot;
	bool bWasCCWHot = m_bCCWHot;
	bool bWasHot = m_bHot;

	m_pHotSub = LTNULL;
	m_bHot = false;

	if (!pCursorSub || m_eState == LIBRARY_STATE_OPEN)
	{
		m_Arc.OnMouseMove(x, y);
		m_bCWHot = m_Arc.IsStartHot();
		m_bCCWHot = m_Arc.IsEndHot();
		m_bHot = m_bWasHot = m_Arc.IsHot();
		m_pHotSub = m_Arc.GetHotSub();
	}

	if (m_bHot)
	{
		m_fCloseTime = 2.0f;
		if (m_bHot && !bWasHot && !pCursorSub)
		{
			Open(false);
		}
	}

	if (m_eState == LIBRARY_STATE_OPEN)
	{
		if (m_bCWHot)
		{
			if (!bWasCWHot)
				PlayMouseOverSound();
		}
		else if (m_bCCWHot)
		{
			if (!bWasCCWHot)
				PlayMouseOverSound();
		}
		else if (bWasCWHot || bWasCCWHot)
		{
			m_Model.PlayAnim("shown", true, false);
		}
	}
}

void CLibraryCtrl::Update()
{
	float fNow = g_pLTClient->GetTime();
	float fDelta = fNow - m_fLastTime;
	m_fLastTime = fNow;

	float fScale = 1.0f;

	if (m_eState == LIBRARY_STATE_OPENING || m_eState == LIBRARY_STATE_CLOSING)
	{
		if (m_fFadeTime > 0.0f)
			m_fFadeTime -= fDelta;

		float fRatio = m_fFadeTime / LIBRARY_FADE_TIME;
		if (fRatio < 0.0f)
			fRatio = 0.0f;
		else if (fRatio > 1.0f)
			fRatio = 1.0f;

		if (m_eState == LIBRARY_STATE_CLOSING)
			fScale = 1.0f - fRatio;
		else if (m_eState == LIBRARY_STATE_OPENING)
			fScale = fRatio;

		m_pTitle->SetColor(((uint32)(uint8)(m_nTitleAlpha * fScale) << 24) | 0xFF);
	}
	else if (m_eState == LIBRARY_STATE_CLOSED)
	{
		m_pTitle->SetColor(((uint32)m_nTitleAlpha << 24) | 0xFF);
	}

	if (m_eState == LIBRARY_STATE_CLOSED)
	{
		if (m_bFlash)
		{
			m_fFlashTime -= g_pLTClient->GetFrameTime();
			while (m_fFlashTime < 0.0f)
				m_fFlashTime += 1.0f;

			float fFlash;
			if (m_fFlashTime > 0.5f)
				fFlash = m_fFlashTime;
			else
				fFlash = 1.0f - m_fFlashTime;

			uint8 nColor = (uint8)(fFlash * 255.0f);
			m_pTitle->SetColor(0xFF0000FF | (((nColor << 8) | nColor) << 8));
		}
		return;
	}

	if (m_eState == LIBRARY_STATE_OPEN)
	{
		if (m_bHot)
			return;

		if (!m_bStayOpen)
		{
			m_fCloseTime -= fDelta;
			if (m_fCloseTime <= 0.0f)
				Close();
		}
	}

	if (m_eState == LIBRARY_STATE_OPENING || m_eState == LIBRARY_STATE_CLOSING)
	{
		if (m_Model.IsAnimDone())
		{
			if (m_eState == LIBRARY_STATE_OPENING)
			{
				m_eState = LIBRARY_STATE_OPEN;
				m_Model.PlayAnim("shown", true, false);
			}
			else if (m_eState == LIBRARY_STATE_CLOSING)
			{
				m_eState = LIBRARY_STATE_CLOSED;
				m_Arc.ShowSprites(false);
				m_Model.PlayAnim("hid", true, false);
			}
		}
	}
}

void CLibraryCtrl::Render()
{
	m_Arc.Render();

	if (m_eState != LIBRARY_STATE_OPEN && m_pTitle)
	{
		m_pTitle->Render(0, -1);
	}
}

// ----------------------------------------------------------------------- //

void CLibraryCtrl::Open(bool bStayOpen)
{
	m_bStayOpen = bStayOpen;

	if (m_eState == LIBRARY_STATE_OPENING || m_eState == LIBRARY_STATE_OPEN)
		return;

	if (m_eState == LIBRARY_STATE_CLOSING)
	{
		m_Model.PlayAnim("shown", true, false);
		m_eState = LIBRARY_STATE_OPEN;
		return;
	}

	m_Arc.ShowSprites(true);
	m_bFlash = false;
	m_pTitle->SetColor(((uint32)m_nTitleAlpha << 24) | 0xFF);

	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
	{
		int iSeg = (*iter)->nTempSlot - (m_nFunction + 1) * 100;
		if (iSeg < LIBRARY_SEGMENTS)
		{
			m_Arc.ShowIcon(iSeg, (*iter)->pProcedural == LTNULL);
		}
	}

	m_Model.PlayAnim("show", false, false);
	m_fFadeTime = LIBRARY_FADE_TIME;
	m_eState = LIBRARY_STATE_OPENING;
	g_pClientSoundMgr->PlaySoundLocal("interface\\subroutines\\snd\\slideout.wav");
}

void CLibraryCtrl::Close()
{
	m_bStayOpen = false;

	if (m_eState == LIBRARY_STATE_CLOSING || m_eState == LIBRARY_STATE_CLOSED)
		return;

	if (m_eState == LIBRARY_STATE_OPENING)
	{
		m_Model.PlayAnim("hid", true, false);
		m_eState = LIBRARY_STATE_CLOSED;
	}
	else if (m_eState == LIBRARY_STATE_OPEN)
	{
		m_Model.PlayAnim("hide", false, false);
		m_fFadeTime = LIBRARY_FADE_TIME;
		m_eState = LIBRARY_STATE_CLOSING;
	}
}

void CLibraryCtrl::Show()
{
	m_Model.Show(true);
	m_Arc.ShowSprites(true);
	m_bVisible = true;
}

// ----------------------------------------------------------------------- //

void CLibraryCtrl::AddSub(PlayerSubroutine * pSub)
{
	int iSeg = m_Arc.GetActiveSegment();
	if (m_pHotSub || iSeg == -1)
	{
		InsertSub(pSub);
		return;
	}

	int nSlot = iSeg + (m_nFunction + 1) * 100;
	m_SubArray.push_back(pSub);
	if (iSeg < LIBRARY_SEGMENTS)
	{
		m_Arc.OccupySegment(iSeg, pSub);
		pSub->nTempSlot = nSlot;
	}
}

void CLibraryCtrl::InsertSub(PlayerSubroutine * pSub)
{
	int nBase = (m_nFunction + 1) * 100;

	bool bUsed[LIBRARY_MAX_SUBS];
	memset(bUsed, 0, sizeof(bUsed));

	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
	{
		int i = (*iter)->nTempSlot - nBase;
		if (i >= 0 && i < LIBRARY_MAX_SUBS)
			bUsed[i] = true;
	}

	int nFree = 0;
	while (bUsed[nFree])
	{
		if (nFree >= LIBRARY_MAX_SUBS)
			return;
		nFree++;
	}
	if (nFree >= LIBRARY_MAX_SUBS)
		return;

	pSub->nTempSlot = nFree + nBase;
	if (nFree > m_nLastIndex)
		m_nLastIndex = nFree;

	m_SubArray.push_back(pSub);
	if (nFree < LIBRARY_SEGMENTS)
	{
		m_Arc.OccupySegment(nFree, pSub);
		if (!m_bVisible)
			m_Arc.ShowSprites(false);
	}
}

void CLibraryCtrl::RemoveSub(PlayerSubroutine * pSub)
{
	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
	{
		if (pSub == *iter)
		{
			m_Arc.ClearSegment(pSub->nTempSlot - (m_nFunction + 1) * 100);
			m_SubArray.erase(iter);
			if (m_pHotSub == pSub)
				m_pHotSub = LTNULL;
			return;
		}
	}
}

void CLibraryCtrl::ShowSubIcon(PlayerSubroutine * pSub, bool bShow)
{
	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
	{
		if (pSub == *iter)
		{
			int iSeg = pSub->nTempSlot - (m_nFunction + 1) * 100;
			if (iSeg < LIBRARY_SEGMENTS)
				m_Arc.ShowIcon(iSeg, bShow);
			return;
		}
	}
}

// ----------------------------------------------------------------------- //

void CLibraryCtrl::OnClick()
{
	if (m_bCWHot)
		RotateCW();
	else if (m_bCCWHot)
		RotateCCW();

	g_pClientSoundMgr->PlaySoundLocal("interface\\subroutines\\snd\\clickon.wav");
}

void CLibraryCtrl::RotateCW()
{
	m_Model.PlayAnim("press_cw", false, false);

	int nBase = (m_nFunction + 1) * 100;

	for (int i = 0; i < LIBRARY_SEGMENTS; i++)
		m_Arc.ClearSegment(i);

	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
	{
		PlayerSubroutine * pSub = *iter;
		pSub->nTempSlot--;
		if (pSub->nTempSlot < nBase)
			pSub->nTempSlot = m_nLastIndex + nBase;
		if (pSub->nTempSlot < nBase + LIBRARY_SEGMENTS)
			m_Arc.OccupySegment(pSub->nTempSlot - nBase, pSub);
	}
}

void CLibraryCtrl::RotateCCW()
{
	m_Model.PlayAnim("press_ccw", false, false);

	int nBase = (m_nFunction + 1) * 100;

	for (int i = 0; i < LIBRARY_SEGMENTS; i++)
		m_Arc.ClearSegment(i);

	PlayerSubroutineArray::iterator iter;
	for (iter = m_SubArray.begin(); iter != m_SubArray.end(); iter++)
	{
		PlayerSubroutine * pSub = *iter;
		pSub->nTempSlot++;
		if (pSub->nTempSlot > m_nLastIndex + nBase)
			pSub->nTempSlot = nBase;
		if (pSub->nTempSlot < nBase + LIBRARY_SEGMENTS)
			m_Arc.OccupySegment(pSub->nTempSlot - nBase, pSub);
	}
}
