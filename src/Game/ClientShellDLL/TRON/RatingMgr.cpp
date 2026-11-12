//
// RatingMgr.cpp
//
// Written 11/10/01 by A. Megowan
//
// Class to encapsulate management of the Performance Ratings
//
// RETAIL: the 1.030 manager (RatingMgr.h), written from the retail code
// (CShell.dll 0x100511a0..0x10051980).  Build 69's additives are gone.
//
#include "stdafx.h"
#include "RatingMgr.h"
#include "ScreenSubroutines.h"

CRatingMgr* g_pRatingMgr = LTNULL;	// RETAIL: 0x1026e250

#define RATINGS_TAG				"Ratings"

#define RATINGS_POINTSPERLEVEL	"PointsPerLevel"
#define RATINGS_INITIALPOINTS	"InitialPoints"
#define RATINGS_INITIALVALUE	"InitialValue"
#define RATINGS_MINVALUE		"MinValue"
#define RATINGS_MAXVALUE		"MaxValue"

// the rating the transfer rates scale with (RETAIL: 3; Build 69's Stealth)
#define RATING_TRANSFER			Rating_Stealth


CRatingMgr::CRatingMgr()
{
}

CRatingMgr::~CRatingMgr()
{
	Term();
}

// RETAIL: 0x100512d0
LTBOOL CRatingMgr::Init(const char* szAttributeFile)
{
	if (g_pRatingMgr) return LTFALSE;

	// Set up global pointer...
	g_pRatingMgr = this;

	int i;
	for (i = 0; i < NUM_RATINGS; i++)
	{
		m_nRating[i] = 0;
		m_nMinRating[i] = 0;
		m_nMaxRating[i] = 20;
	}
	m_nPoints = 0;
	m_nLevelPoints = m_nPointsPerLevel = 7;

	// RETAIL: a missing or bad file still succeeds
    if (!szAttributeFile) return LTTRUE;
    if (!Parse(szAttributeFile)) return LTTRUE;
	if (!m_buteMgr.Exist(RATINGS_TAG)) return LTTRUE;

	m_nLevelPoints = m_nPointsPerLevel = m_buteMgr.GetInt(RATINGS_TAG, RATINGS_POINTSPERLEVEL);
	m_nInitialPoints = m_buteMgr.GetInt(RATINGS_TAG, RATINGS_INITIALPOINTS);
	m_nInitialValue = m_buteMgr.GetInt(RATINGS_TAG, RATINGS_INITIALVALUE);
	m_nMinValue = m_buteMgr.GetInt(RATINGS_TAG, RATINGS_MINVALUE);
	m_nMaxValue = m_buteMgr.GetInt(RATINGS_TAG, RATINGS_MAXVALUE);

	m_nPoints = m_nInitialPoints;
	for (i = 0; i < NUM_RATINGS; i++)
	{
		m_nRating[i] = m_nInitialValue;
		m_nMinRating[i] = m_nMinValue;
		m_nMaxRating[i] = m_nMaxValue;
	}

	m_vAlphaSubTransfer		= m_buteMgr.GetVector(RATINGS_TAG, "AlphaSubTransfer");
	m_vBetaSubTransfer		= m_buteMgr.GetVector(RATINGS_TAG, "BetaSubTransfer");
	m_vGoldSubTransfer		= m_buteMgr.GetVector(RATINGS_TAG, "GoldSubTransfer");
	m_vProceduralTransfer	= m_buteMgr.GetVector(RATINGS_TAG, "ProceduralTransfer");
	m_vPrimitiveTransfer	= m_buteMgr.GetVector(RATINGS_TAG, "PrimitiveTransfer");
	m_vPsetTransfer			= m_buteMgr.GetVector(RATINGS_TAG, "PsetTransfer");
	m_vEmailTransfer		= m_buteMgr.GetVector(RATINGS_TAG, "EmailTransfer");
	m_vHelpFileTransfer		= m_buteMgr.GetVector(RATINGS_TAG, "HelpFileTransfer");
	m_vCustomTransfer		= m_buteMgr.GetVector(RATINGS_TAG, "CustomTransfer");

	m_buteMgr.Term();

	return LTTRUE;
}

// RETAIL: 0x10051640
void CRatingMgr::ResetRatings()
{
	m_nPoints = m_nInitialPoints;
	for (int i = 0; i < NUM_RATINGS; i++)
	{
		m_nRating[i] = m_nInitialValue;
		m_nMinRating[i] = m_nMinValue;
		m_nMaxRating[i] = m_nMaxValue;
	}
}

// RETAIL: 0x10051680
void CRatingMgr::Term()
{
	g_pRatingMgr = LTNULL;
}

// RETAIL: 0x10051690
void CRatingMgr::SetPerformanceRating(PerformanceRating p, uint8 nVal)
{
	m_nRating[p] = nVal;
}

// RETAIL: 0x100516b0
int CRatingMgr::GetRating(PerformanceRating p)
{
	if (m_nRating[p] > 100)
		return 100;
	return m_nRating[p];
}

// RETAIL: 0x100516d0
void CRatingMgr::PopulateRatings(CScreenSubroutines *pScreen)
{
	if (!pScreen) return;

	for (int i = 0; i < NUM_RATINGS; i++)
		pScreen->SetRating(i, m_nRating[i]);
	pScreen->SetRatingPoints(m_nPoints);
}

// RETAIL: 0x10051710
LTFLOAT CRatingMgr::GetTransferRate(RatingTransferType eType, int nBuild)
{
	LTVector vRate;
	switch (eType)
	{
		case kTransfer_Subroutine:
		{
			switch (nBuild)
			{
				case 0: vRate = m_vAlphaSubTransfer; break;
				case 2: vRate = m_vGoldSubTransfer; break;
				case 1: vRate = m_vBetaSubTransfer; break;
				default: vRate = m_vAlphaSubTransfer; break;
			}
		}
		break;
		case kTransfer_Procedural:	vRate = m_vProceduralTransfer; break;
		case kTransfer_Primitive:	vRate = m_vPrimitiveTransfer; break;
		case kTransfer_Pset:		vRate = m_vPsetTransfer; break;
		case kTransfer_Email:		vRate = m_vEmailTransfer; break;
		case kTransfer_HelpFile:	vRate = m_vHelpFileTransfer; break;
		case kTransfer_Custom:		vRate = m_vCustomTransfer; break;
	}
	return InterpolateRating<LTFLOAT>(RATING_TRANSFER, vRate.x, vRate.y, vRate.z);
}

// PORT: Build 69's additives are not in the retail manager (RatingMgr.h)
void CRatingMgr::GivePlayerAdditive(char *pszName)
{
}

// RETAIL: 0x100518d0
void CRatingMgr::Save(ILTMessage_Write *pMsg)
{
	for (int i = 0; i < NUM_RATINGS; i++)
	{
		pMsg->Writeuint8(m_nRating[i]);
		pMsg->Writeuint8(m_nMaxRating[i]);
	}
	pMsg->Writeint32(m_nPoints);
}

// RETAIL: 0x10051930
void CRatingMgr::Load(ILTMessage_Read *pMsg)
{
	for (int i = 0; i < NUM_RATINGS; i++)
	{
		m_nRating[i] = pMsg->Readuint8();
		m_nMaxRating[i] = pMsg->Readuint8();
	}
	m_nPoints = pMsg->Readint32();
}
