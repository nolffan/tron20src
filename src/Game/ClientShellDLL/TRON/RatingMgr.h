//
//
// RatingMgr.h
//
//
//
// RETAIL: Tron 2.0 1.030's CRatingMgr is not Build 69's (additives, May
// 2002): it reads "Attributes\Ratings.txt" (the Ratings tag) and keeps, per
// performance rating, its value and the range it can be upgraded in, plus
// the upgrade points and the transfer (download) rates the ratings scale.
// Written from the retail code: CShell.dll 0x100511a0..0x10051980, vtable
// 0x101e7540; size 0x11b4 (CTronPlayerStats +0xdc).  Member and function
// names are ours unless an attribute name gives them.
//
#ifndef __RATINGMGR_H
#define __RATINGMGR_H

#include "Ratings.h"
#include "GameButeMgr.h"
#include "ScreenSpriteMgr.h"

#define RATINGS_DEFAULT_FILE	"Attributes\\Ratings.txt"

// Build 69's additives: no longer in the retail manager; kept for the
// additive control and the ratings screen of Build 69 that still name them.
#define ADDITIVE_MAX_NAME		32
#define ADDITIVE_MAX_FILE_PATH	64

struct ADDITIVE
{
	char				szName[ADDITIVE_MAX_NAME];		// internal string name
	uint16				nNameId;
	uint16				nDescriptionId;					// ID for description filler
	char				szSprite[ADDITIVE_MAX_FILE_PATH];
	CScreenSprite *		pSprite;
	int8				nTempSlot;
	int8				nSlot;							// -1 = nowhere, 0-2 = slot
	int8				Adjust[NUM_RATINGS];			// how much to change each rating
};

// What a transfer (download) rate is asked for (RETAIL: the switch of
// 0x10051710; the names are ours, from the attribute names).
enum RatingTransferType
{
	kTransfer_Subroutine = 0,	// by its build: AlphaSubTransfer, BetaSubTransfer, GoldSubTransfer
	kTransfer_Procedural,
	kTransfer_Primitive,
	kTransfer_Pset,
	kTransfer_Email,
	kTransfer_HelpFile,
	kTransfer_Custom,
};

class CRatingMgr;
extern CRatingMgr* g_pRatingMgr;

class CScreenSubroutines;
class ILTMessage_Write;
class ILTMessage_Read;

class CRatingMgr : public CGameButeMgr
{
public:
	CRatingMgr();
	~CRatingMgr();

    LTBOOL      Init(const char* szAttributeFile=RATINGS_DEFAULT_FILE);
	void		Term();

	void		ResetRatings();		// back to the attribute file's initial values

	void		SetPerformanceRating(PerformanceRating p, uint8 nVal);
	int			GetRating(PerformanceRating p);

	LTBOOL		IsSurgeRating(PerformanceRating p) {return (m_nRating[p] >= m_nMaxRating[p]); }
	uint8		GetMaxRating(PerformanceRating p) {return m_nMaxRating[p]; }

	void		PopulateRatings(CScreenSubroutines *pScreen);	// fill the ratings on the subroutines screen

	LTFLOAT		GetTransferRate(RatingTransferType eType, int nBuild);

	// PORT: Build 69's, still called through CTronPlayerStats by
	// CTronInterfaceMgr (MID_ADDITIVE_OBTAINED); the retail manager has no
	// additives, so it does nothing
	void		GivePlayerAdditive(char *pszName);

	void		Save(ILTMessage_Write *pMsg);
	void		Load(ILTMessage_Read *pMsg);

	int			GetPoints() { return m_nPoints; }
	void		SetPoints(int nPoints) { m_nPoints = nPoints; }

protected:

	int		m_nInitialValue;		// 0x111c "InitialValue"
	int		m_nMinValue;			// "MinValue"
	int		m_nMaxValue;			// "MaxValue"
	int		m_nInitialPoints;		// "InitialPoints"
	int		m_nPoints;				// 0x112c upgrade points left
	int		m_nPointsPerLevel;		// 0x1130 "PointsPerLevel"
	uint8	m_nRating[NUM_RATINGS];		// 0x1134
	uint8	m_nMinRating[NUM_RATINGS];	// 0x1139
	uint8	m_nMaxRating[NUM_RATINGS];	// 0x113e
	int		m_nLevelPoints;			// 0x1144 (also "PointsPerLevel")

	LTVector	m_vAlphaSubTransfer;	// 0x1148: x min, y max, z surge
	LTVector	m_vBetaSubTransfer;
	LTVector	m_vGoldSubTransfer;
	LTVector	m_vProceduralTransfer;
	LTVector	m_vPrimitiveTransfer;
	LTVector	m_vPsetTransfer;
	LTVector	m_vEmailTransfer;
	LTVector	m_vHelpFileTransfer;
	LTVector	m_vCustomTransfer;		// 0x11a8
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InterpolateRating()
//
//	PURPOSE:	Using the specificed rating, map the value to a given range.
//				If the rating is in Surge mode, return the surge value.
//				Otherwise, interpolate between the min and max values.
//
//				NOTE: this belongs in the CRatingMgr class, but MSVC6
//				doesn't support member templates so this function must be
//				placed externally.  I'll place in here to keep it close to
//				where it is supposed to go.
//
//				RETAIL: as inlined in 0x10051710, the rating is scaled
//				by its maximum, not by 100.
//
// ----------------------------------------------------------------------- //

template <typename T>
T InterpolateRating( PerformanceRating ePerformanceRating,
                     T tMinValue,
                     T tMaxValue,
                     T tSurgeValue )
{
	if ( g_pRatingMgr->IsSurgeRating( ePerformanceRating ) )
	{
		// rating is in surge mode: return the surge value
		return tSurgeValue;
	}
	else
	{
		// rating is NOT in surge mode: return the interpolated value 
		uint8 nCurrentRating = g_pRatingMgr->GetRating( ePerformanceRating );
		return tMinValue + ( tMaxValue - tMinValue ) * ((float)nCurrentRating / g_pRatingMgr->GetMaxRating( ePerformanceRating ));
	}
}



#endif // __RATINGMGR_H
