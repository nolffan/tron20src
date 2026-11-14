//-------------------------------------------------------------------------
//
// MODULE  : RatingBarCtrl.cpp
//
// PURPOSE : The subroutine screen's performance rating bars and the bar of
//			 the upgrade (build) points they are bought with
//
// RETAIL: written from retail CShell.dll 0x1004fa70..0x100510f8 (see the
// header). PORT: retail's PlaySoundLocal takes one more argument (1.0f)
// than ours, so the functions playing sounds stay near.
//
//-------------------------------------------------------------------------

#include "stdafx.h"
#include "RatingBarCtrl.h"
#include "ScreenSubroutines.h"
#include "RatingMgr.h"
#include "InterfaceMgr.h"
#include "InterfaceResMgr.h"
#include "ClientSoundMgr.h"
#include "ClientUtilities.h"
#include "cuipolystring.h"

// RETAIL: 0x10217d44, 0x10217d48: the points bar pulses from opaque to
// 0.2 and back every 1.5 seconds (names ours)
float g_fPointsPulseTime = 1.5f;
float g_fPointsPulseMin = 0.2f;

namespace
{
	// RETAIL: the string IDs retail uses (names ours)
	const int IDS_SUBS_RATING_NAME_FIRST	= 0xF0A;	// + the rating
	const int IDS_SUBS_RATING_HELP_FIRST	= 0xF0F;	// + the rating
	const int IDS_SUBS_RATING_DECREASE		= 0xF14;
	const int IDS_SUBS_RATING_INCREASE		= 0xF15;
	const int IDS_SUBS_BUILDPOINTS_HELP		= 0xF16;

	void PlayMouseOverSound()
	{
		g_pLTClient->CPrint("interface\\subroutines\\snd\\mouseover.wav");
		g_pClientSoundMgr->PlayInterfaceSound("interface\\subroutines\\snd\\mouseover.wav");
	}
}


//////////////////////////////////////////////////////////////////////
// CBuildPointsCtrl
//////////////////////////////////////////////////////////////////////

CBuildPointsCtrl::CBuildPointsCtrl()
{
	m_bInit = false;
	m_bVisible = false;
	m_bShown = false;
	m_bHot = false;
	m_nPoints = 0;
	m_nLevelPoints = 7;
	m_nTarget = 0;
	m_fCurrent = 0;
	m_fSpeed = 40.0f;
	m_fAlpha = 1.0f;
	m_nAlphaDir = -1;
}

CBuildPointsCtrl::~CBuildPointsCtrl()
{
	Term();
}

void CBuildPointsCtrl::Init(int nPoints)
{
	m_nPoints = nPoints;
	m_nLevelPoints = g_pRatingMgr->GetLevelPoints();
	m_nTarget = (int)(100.0f / (float)m_nLevelPoints * nPoints);
	m_fCurrent = 0.0f;

	m_Bar.Create("buildpoints_bar");
	m_Bar.Pause(true);
	SetBarValue(m_fCurrent);

	m_fLastTime = g_pLTClient->GetTime();
	m_bInit = true;

	ScreenDimsChanged();

	m_fAlpha = 1.0f;
	m_nAlphaDir = -1;
}

void CBuildPointsCtrl::Term()
{
	if (m_bInit)
	{
		m_Bar.Term();
		m_bShown = false;
		m_bInit = false;
	}
}

void CBuildPointsCtrl::ScreenDimsChanged()
{
	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetXRatio();

	m_Center.x = (int)(fXRatio * 320.0f);
	m_Center.y = (int)(fYRatio * 240.0f);
	m_nTop = (int)(fYRatio * 182.0f);
	m_nBottom = (int)(fYRatio * 300.0f);
	m_fInnerRadius = fXRatio * 107.0f;
	m_fOuterRadius = fXRatio * 114.0f;
}

std::wstring CBuildPointsCtrl::GetHelpText()
{
	std::wstring sText;
	char * pBuf = debug_newa(char, 1024);
	FormatString(IDS_SUBS_BUILDPOINTS_HELP, pBuf, 1024);

	sText.erase();
	uint32 nLen = strlen(pBuf);
	for (uint32 i = 0; i < nLen; i++)
		sText.push_back((uint8)pBuf[i]);

	debug_deletea(pBuf);
	return sText;
}

void CBuildPointsCtrl::OnMouseMove(int x, int y)
{
	m_bHot = false;
	if (!m_bInit)
		return;

	m_bLeft = false;
	if (x < m_Center.x)
		m_bLeft = true;

	if (y < m_nTop || y > m_nBottom)
		return;

	float fX = (float)(x - m_Center.x);
	float fY = (float)(y - m_Center.y);
	float fDistSqr = fY * fY + fX * fX;

	if (fDistSqr < m_fInnerRadius * m_fInnerRadius)
		return;
	if (fDistSqr > m_fOuterRadius * m_fOuterRadius)
		return;

	m_bHot = true;
}

LTIntPt CBuildPointsCtrl::GetHelpPos()
{
	int nY = m_Center.y;
	if (m_bLeft)
		return LTIntPt(m_Center.x + (int)m_fInnerRadius, nY);

	return LTIntPt(m_Center.x - (int)m_fInnerRadius, nY);
}

void CBuildPointsCtrl::Increment()
{
	m_nPoints++;
	m_nTarget = (int)(100.0f / (float)m_nLevelPoints * m_nPoints);
	if (m_nTarget > 100)
		m_nTarget = 100;
}

void CBuildPointsCtrl::Decrement()
{
	m_nPoints--;
	m_nTarget = (int)(100.0f / (float)m_nLevelPoints * m_nPoints);
}

void CBuildPointsCtrl::Apply()
{
	g_pRatingMgr->SetPoints(m_nPoints);
}

void CBuildPointsCtrl::Update()
{
	float fTime = g_pLTClient->GetTime();
	float fDelta = fTime - m_fLastTime;
	m_fLastTime = fTime;

	if (!m_bVisible)
	{
		if (m_bShown)
		{
			m_Bar.Show(false);
			m_bShown = false;
		}
		return;
	}

	if (!m_bShown)
	{
		m_bShown = true;
		m_Bar.Show(true);
	}

	if (m_bShown && m_Bar.GetObject())
	{
		float r, g, b, a;
		g_pLTClient->GetObjectColor(m_Bar.GetObject(), &r, &g, &b, &a);

		int nDir = m_nAlphaDir;
		m_fAlpha += g_pLTClient->GetFrameTime() / g_fPointsPulseTime * nDir;
		if (m_fAlpha > 1.0f || m_fAlpha < g_fPointsPulseMin)
		{
			m_fAlpha = Clamp(m_fAlpha, g_fPointsPulseMin, 1.0f);
			m_nAlphaDir = -nDir;
		}

		g_pLTClient->SetObjectColor(m_Bar.GetObject(), r, g, b, m_fAlpha);
	}

	int nTarget = m_nTarget;
	if ((int)m_fCurrent != nTarget)
	{
		float fStep = fDelta * m_fSpeed;
		float fTarget = (float)nTarget;
		if (m_fCurrent < fTarget)
		{
			float fNew = fStep + m_fCurrent;
			if (fNew > fTarget)
			{
				SetBarValue(m_fCurrent = fTarget);
				return;
			}
			m_fCurrent = fNew;
		}
		else
		{
			float fNew = m_fCurrent - fStep;
			if (fNew < fTarget)
			{
				SetBarValue(m_fCurrent = fTarget);
				return;
			}
			m_fCurrent = fNew;
		}
		SetBarValue(m_fCurrent);
	}
}

void CBuildPointsCtrl::SetBarValue(float fValue)
{
	int nValue = (int)fValue;
	if (nValue < 0)
		nValue = 0;
	else if (nValue > 100)
		nValue = 100;

	m_Bar.SetAnimTime(nValue);
}


//////////////////////////////////////////////////////////////////////
// CRatingBarCtrl
//////////////////////////////////////////////////////////////////////

CRatingBarCtrl::CRatingBarCtrl()
{
	m_bInit = false;
	m_bButtonsShown = false;
	m_bTextVisible = false;
	m_bHot = false;
	m_bOverLeft = false;
	m_bOverRight = false;
	m_fCurrent = 0;
	m_nMin = 0;
	m_nMax = 100;
	m_nValue = 0;
	m_nInitialValue = 0;
	m_fFixed = 0;
	m_nTarget = 0;
	m_fSpeed = 40.0f;
	m_fFade = 0;
	m_pBuildPoints = LTNULL;
	m_pLabel = LTNULL;
	m_rcBaseLeft.left = 0;
	m_rcBaseLeft.top = 0;
	m_rcBaseLeft.right = 0;
	m_rcBaseLeft.bottom = 0;
	m_rcBaseRight.left = 0;
	m_rcBaseRight.top = 0;
	m_rcBaseRight.right = 0;
	m_rcBaseRight.bottom = 0;
	m_rcLeft.left = 0;
	m_rcLeft.top = 0;
	m_rcLeft.right = 0;
	m_rcLeft.bottom = 0;
	m_rcRight.left = 0;
	m_rcRight.top = 0;
	m_rcRight.right = 0;
	m_rcRight.bottom = 0;
}

CRatingBarCtrl::~CRatingBarCtrl()
{
	Term();
}

void CRatingBarCtrl::Term()
{
	if (!m_bInit)
		return;

	m_FixedBar.Term();
	m_Bar.Term();
	m_LeftButton.Term();
	m_RightButton.Term();
	m_Surge.Term();
	m_bButtonsShown = false;

	if (m_pLabel)
	{
		g_pFontManager->DestroyPolyString(m_pLabel);
		m_pLabel = LTNULL;
	}

	m_bTextVisible = false;
	m_bInit = false;
}

void CRatingBarCtrl::SetRects(LTRect rcLeft, LTRect rcRight)
{
	m_rcBaseLeft.left = rcLeft.left;
	m_rcBaseLeft.right = rcLeft.right;
	m_rcBaseLeft.top = rcLeft.top;
	m_rcBaseLeft.bottom = rcLeft.bottom;
	m_rcBaseRight.left = rcRight.left;
	m_rcBaseRight.right = rcRight.right;
	m_rcBaseRight.top = rcRight.top;
	m_rcBaseRight.bottom = rcRight.bottom;

	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	m_rcLeft.left = (int)((float)rcLeft.left * fXRatio);
	m_rcLeft.right = (int)((float)rcLeft.right * fXRatio);
	m_rcLeft.top = (int)((float)rcLeft.top * fYRatio);
	m_rcLeft.bottom = (int)((float)rcLeft.bottom * fYRatio);
	m_rcRight.left = (int)((float)rcRight.left * fXRatio);
	m_rcRight.right = (int)((float)rcRight.right * fXRatio);
	m_rcRight.top = (int)((float)rcRight.top * fYRatio);
	m_rcRight.bottom = (int)((float)rcRight.bottom * fYRatio);
}

void CRatingBarCtrl::SetTextVisible(bool bVisible)
{
	m_bTextVisible = bVisible;
	if (bVisible)
		m_fFade = 1.0f;
}

void CRatingBarCtrl::Show(bool bShow)
{
	SetTextVisible(bShow);
	m_FixedBar.Show(bShow);
	m_Bar.Show(bShow);
	m_LeftButton.Show(bShow);
	m_RightButton.Show(bShow);
	m_Surge.Show(bShow);
}

LTIntPt CRatingBarCtrl::GetCenter()
{
	return LTIntPt((m_rcLeft.left + m_rcRight.right) / 2, (m_rcLeft.top + m_rcRight.bottom) / 2);
}

std::wstring CRatingBarCtrl::GetHelpText()
{
	std::wstring sText;
	char * pBuf = debug_newa(char, 1024);
	FormatString(IDS_SUBS_RATING_HELP_FIRST + m_nRating, pBuf, 1024);

	if (m_bCanDecrease)
		strcat(pBuf, LoadTempString(IDS_SUBS_RATING_DECREASE));
	else if (m_bCanIncrease)
		strcat(pBuf, LoadTempString(IDS_SUBS_RATING_INCREASE));

	sText.erase();
	uint32 nLen = strlen(pBuf);
	for (uint32 i = 0; i < nLen; i++)
		sText.push_back((uint8)pBuf[i]);

	debug_deletea(pBuf);
	return sText;
}

void CRatingBarCtrl::ScreenDimsChanged()
{
	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	m_rcLeft.left = (int)((float)m_rcBaseLeft.left * fXRatio);
	m_rcLeft.right = (int)((float)m_rcBaseLeft.right * fXRatio);
	m_rcLeft.top = (int)((float)m_rcBaseLeft.top * fYRatio);
	m_rcLeft.bottom = (int)((float)m_rcBaseLeft.bottom * fYRatio);
	m_rcRight.left = (int)((float)m_rcBaseRight.left * fXRatio);
	m_rcRight.right = (int)((float)m_rcBaseRight.right * fXRatio);
	m_rcRight.top = (int)((float)m_rcBaseRight.top * fYRatio);
	m_rcRight.bottom = (int)((float)m_rcBaseRight.bottom * fYRatio);

	if (m_pLabel)
	{
		int nHeight = (int)(fYRatio * 18.0f);
		m_pLabel->SetFont(g_pInterfaceResMgr->GetFontForHeight(nHeight));
		m_pLabel->SetCharScreenHeight(nHeight);
		// RETAIL: the label's y is offset by the rating's index
		m_pLabel->SetPosition(m_rcLeft.right + 2, m_rcLeft.bottom - m_nRating - 4);
	}
}

void CRatingBarCtrl::Init(int nRating, int nValue)
{
	char szBase[16];
	char szName[64];

	m_nRating = nRating;
	sprintf(szBase, "rating%d", nRating + 1);

	sprintf(szName, "%s_bar", szBase);
	m_Bar.Create(szName);
	m_Bar.Show(true);
	m_Bar.Pause(true);

	sprintf(szName, "%s_fixedbar", szBase);
	m_FixedBar.Create(szName);
	m_FixedBar.Show(true);
	m_FixedBar.Pause(true);

	sprintf(szName, "%s_button_left", szBase);
	m_LeftButton.Create(szName);
	m_LeftButton.Update();

	sprintf(szName, "%s_button_right", szBase);
	m_RightButton.Create(szName);
	m_RightButton.Update();

	m_Surge.Create("ratings_bar_surge");
	sprintf(szName, "skt_ratbar_%d", nRating + 1);
	m_Surge.Attach("sub_hub", szName);
	m_Surge.Show(false);

	m_nMin = g_pRatingMgr->GetMinRating((PerformanceRating)nRating);
	m_nMax = g_pRatingMgr->GetMaxRating((PerformanceRating)nRating);
	m_nValue = nValue;
	m_nInitialValue = nValue;
	m_Surge.Show(nValue == m_nMax);

	m_nTarget = (int)(100.0f / (float)(m_nMax - m_nMin) * nValue);
	m_fCurrent = 0;

	m_Bar.PlayAnim("fill", false, true);
	m_FixedBar.PlayAnim("loop", false, true);

	m_fFixed = m_fCurrent;
	SetBarValue(m_fFixed, true);
	SetBarValue(m_fCurrent, false);

	m_fLastTime = g_pLTClient->GetTime();

	if (m_pLabel)
	{
		g_pFontManager->DestroyPolyString(m_pLabel);
	}

	int nHeight = (int)(g_pInterfaceResMgr->GetXRatio() * 16.0f);
	CUIFont * pFont = g_pInterfaceResMgr->GetFontForHeight(nHeight);
	m_pLabel = g_pFontManager->CreatePolyString(pFont, LoadTempWString(IDS_SUBS_RATING_NAME_FIRST + nRating), 0, 0);
	m_pLabel->SetColor(0xFFFFFFFF);
	m_pLabel->SetCharScreenHeight(nHeight);
	// RETAIL: the label's y is offset by the rating's index
	m_pLabel->SetPosition(m_rcLeft.right + 2, m_rcLeft.bottom - m_nRating - 4);

	m_bHot = false;
	m_bInit = true;
}

void CRatingBarCtrl::OnMouseMove(int x, int y)
{
	bool bWasOverRight = m_bOverRight;
	bool bWasOverLeft = m_bOverLeft;

	m_bHot = false;
	m_bCanIncrease = false;
	m_bCanDecrease = false;
	m_bOverLeft = false;
	m_bOverRight = false;

	if (y < m_rcLeft.top || y > m_rcRight.bottom)
		return;
	if (x < m_rcLeft.left || x > m_rcRight.right)
		return;

	m_bHot = true;

	if (!m_pBuildPoints->IsVisible())
		return;

	if (x <= m_rcLeft.right)
	{
		if (!bWasOverLeft)
			PlayMouseOverSound();

		m_bOverLeft = true;
		if (m_nValue > m_nInitialValue)
		{
			m_bCanDecrease = true;
			return;
		}
	}

	if (m_pBuildPoints->GetPoints() > 0 && x >= m_rcRight.left && x <= m_rcRight.right)
	{
		if (!bWasOverRight)
			PlayMouseOverSound();

		m_bOverRight = true;
		if (m_nValue < m_nMax)
			m_bCanIncrease = true;
	}
}

void CRatingBarCtrl::OnLButtonDown(int x, int y)
{
	if (!m_bHot)
		return;

	if (m_bCanDecrease)
	{
		if (m_nValue == m_nMax)
			m_Surge.Show(false);

		m_nValue--;
		m_pBuildPoints->Increment();

		m_LeftButton.PlayAnim("press", false, false);
		m_LeftButton.SetNextAnim("loop");

		m_nTarget = (int)(100.0f / (float)(m_nMax - m_nMin) * m_nValue);
		OnMouseMove(x, y);

		g_pClientSoundMgr->PlaySoundLocal("perf_decrease", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
	}
	else if (m_bCanIncrease)
	{
		m_nValue++;
		if (m_nValue == m_nMax)
		{
			m_Surge.Show(true);
			g_pClientSoundMgr->PlaySoundLocal("perf_surge", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
		}
		else
		{
			g_pClientSoundMgr->PlaySoundLocal("perf_increase", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
		}

		m_pBuildPoints->Decrement();

		m_RightButton.PlayAnim("press", false, false);
		m_RightButton.SetNextAnim("loop");

		m_nTarget = (int)(100.0f / (float)(m_nMax - m_nMin) * m_nValue);
		OnMouseMove(x, y);
	}
}

void CRatingBarCtrl::Apply()
{
	g_pRatingMgr->SetPerformanceRating((PerformanceRating)m_nRating, m_nValue);
}

void CRatingBarCtrl::Update()
{
	float fTime = g_pLTClient->GetTime();
	float fDelta = fTime - m_fLastTime;
	m_fLastTime = fTime;

	if (m_fFade > 0.0f)
		m_fFade -= fDelta;

	if (m_pBuildPoints->IsVisible() && !m_bButtonsShown)
	{
		m_LeftButton.Show(true);
		m_RightButton.Show(true);
		m_bButtonsShown = true;
	}

	int nTarget = m_nTarget;
	if ((int)m_fCurrent != nTarget)
	{
		float fStep = fDelta * m_fSpeed;
		float fTarget = (float)nTarget;
		if (m_fCurrent < fTarget)
		{
			float fNew = fStep + m_fCurrent;
			if (fNew > fTarget)
				m_fCurrent = fTarget;
			else
				m_fCurrent = fNew;
		}
		else
		{
			float fNew = m_fCurrent - fStep;
			if (fNew < fTarget)
				m_fCurrent = fTarget;
			else
				m_fCurrent = fNew;
		}
	}

	SetBarValue(m_fCurrent, false);
	SetBarValue(m_fFixed, true);

	if (m_bTextVisible && m_pLabel)
	{
		uint8 nAlpha = 0xFF;
		if (m_fFade > 0.0f)
			nAlpha = (uint8)((1.0f - m_fFade) * 255.0f);

		m_pLabel->SetColor(((uint32)nAlpha << 24) | 0x00FFFFFF);
		m_pLabel->Render(0, -1);
	}
}

void CRatingBarCtrl::SetBarValue(float fValue, bool bFixed)
{
	int nValue = (int)fValue;
	if (nValue < 0)
		nValue = 0;
	else if (nValue > 100)
		nValue = 100;

	if (bFixed)
		m_FixedBar.SetAnimTime(nValue);
	else
		m_Bar.SetAnimTime(nValue);
}
