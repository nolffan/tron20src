//-------------------------------------------------------------------------
//
// MODULE  : ArcCtrl.cpp
//
// PURPOSE : GUI element for interacting with a segmented arc of buttons
//
// CREATED : 12/31/01 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#include "stdafx.h"
#include "ArcCtrl.h"
#include "InterfaceResMgr.h"
#include "InterfaceMgr.h"

// RETAIL: this TU is 0x10001000..0x10002e50 in Tron's CShell.dll, written
// from the retail code: Build 69's sprites, scale FX and camera globals are
// gone; the sectors attach menu piece models to their parent piece. Near
// (2026-11-16): CArcSector::Occupy (a switch's case 0 test), GetHotPos,
// IsOnMe and CArcCtrl::RemoveHighlighting (register and block order).

#define DEBUG_DELETEA(x) if (x)	{debug_deletea(x);x = LTNULL;}

namespace
{
	const float EPSILON = 0.001f;
	const float RAD_TO_DEG = 180.0f / MATH_PI;
	const float DEG_TO_RAD = MATH_PI / 180.f;
	// RETAIL: Tron's (0x10236c20, set at 0x10001040; IsOnMe and 0x10001f40 read
	// it); the name is ours.
	const float ARC_GAP_ANGLE = DEG_TO_RAD * 10.0f;


	// RETAIL: 0x1020ff48, indexed by SegmentType
	char * segNames[] = {
		"none",
		"lib_hi",
		"sys_hi",
		"sys_conflict",
		"sys_badblock",
		"sys_basecode",
		"sys_basecode_left",
		"sys_basecode_middle",
		"sys_basecode_right",
		"sys_alpha",
		"sys_beta",
		"sys_gold",
	};

	// RETAIL: 0x1020ff78, 0x1020ff84, by SubVersion
	char * ringNames[] = {
		"optring_alpha",
		"optring_beta",
		"optring_gold",
	};

	char * corruptionNames[] = {
		"corruption_alpha",
		"corruption_beta",
		"corruption_gold",
	};

	// RETAIL: 0x1020ff90, by the size of the cursor
	char * highlightNames[] = {
		"none",
		"sys_gold_hl",
		"sys_beta_hl",
		"sys_alpha_hl",
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildPoly <global function>
//
//	PURPOSE:	Given a position, two angles and two radii, build a polygon.
//				The first radius is the inner one.
//
// ----------------------------------------------------------------------- //
void BuildPoly(LT_POLYF4 *pPoly, LTIntPt pos, float r1, float r2, float theta1, float theta2)
{
	if (!pPoly)
		return;

	float fCos1 = ltcosf(theta1);
	float fCos2 = ltcosf(theta2);
	float fSin1 = ltsinf(theta1);
	float fSin2 = ltsinf(theta2);

	pPoly->verts[0].x = pos.x + r1 * fCos1;
	pPoly->verts[0].y = pos.y - r1 * fSin1;
	pPoly->verts[0].z = SCREEN_NEAR_Z;

	pPoly->verts[1].x = pos.x + r1 * fCos2;
	pPoly->verts[1].y = pos.y - r1 * fSin2;
	pPoly->verts[1].z = SCREEN_NEAR_Z;

	pPoly->verts[2].x = pos.x + r2 * fCos2;
	pPoly->verts[2].y = pos.y - r2 * fSin2;
	pPoly->verts[2].z = SCREEN_NEAR_Z;

	pPoly->verts[3].x = pos.x + r2 * fCos1;
	pPoly->verts[3].y = pos.y - r2 * fSin1;
	pPoly->verts[3].z = SCREEN_NEAR_Z;
}

//////////////////////////////////////////////////////////////////////
// CArcSector Construction/Destruction
//////////////////////////////////////////////////////////////////////
CArcSector::CArcSector()
{
	m_eArcType			= ARC_TYPE_UNDEFINED;
	m_iSectorNumber		= -1;
	m_bOccupied			= false;
	m_iColor			= 0x80606060;
	m_iFirst			= -1;
	m_iLast				= -1;

	m_pSubPtr			= LTNULL;
	m_iAnimNumber		= 0;
	m_szParent			= LTNULL;

	// Visual appearance
	m_Position.x		= 0;
	m_Position.y		= 0;
	m_fInnerRadius		= 0.0f;
	m_fOuterRadius		= 0.0f;
	m_fTheta1			= 0.0f;
	m_fTheta2			= 0.0f;

	m_bShowSprites		= true;
	m_bShowIcon			= true;
	m_bHighlighted		= false;
}

CArcSector::~CArcSector()
{
	Term();
}

void CArcSector::Build(int iNum, LTIntPt pt, float r1, float r2, float fTheta1, float fTheta2, ArcType eType)
{
	Term();

	m_iSectorNumber		= iNum;
	m_Position			= pt;
	m_fInnerRadius		= r1;
	m_fOuterRadius		= r2;
	m_fTheta1			= fTheta1;
	m_fTheta2			= fTheta2;
	m_eArcType			= eType;

	// RETAIL: the system memory hub has 24 sockets, counted clockwise
	// from the top.
	int iDegrees = (int)((fTheta1 + fTheta2) * RAD_TO_DEG * -0.5f);
	m_iAnimNumber = ((29 - iDegrees / -15) % 24) + 1;
}

void CArcSector::Resize(LTIntPt pt, float r1, float r2)
{
	m_Position			= pt;
	m_fInnerRadius		= r1;
	m_fOuterRadius		= r2;
}

void CArcSector::Term()
{
	ClearFX();
}

void CArcSector::ShowSprites(bool bShow)
{
	m_bShowSprites = bShow;
	m_Icon.Show(bShow);
	m_Ring.Show(bShow);

	if (m_pSubPtr && m_pSubPtr->eState == SUBSTATE_CORRUPT)
	{
		m_Condition.Show(bShow);
	}
}

void CArcSector::ShowIcon(bool bShow)
{
	m_bShowIcon = bShow;
	m_Icon.Show(bShow);
}

void CArcSector::SetParentPiece(char * szParent)
{
	m_szParent = szParent;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcSector::Occupy
//
//	PURPOSE:	Put a subroutine in the sector.  Only the first sector of
//				a subroutine creates its models.
//
// ----------------------------------------------------------------------- //

void CArcSector::Occupy(int iFirst, int iLast, PlayerSubroutine * pSubPtr)
{
	ClearFX();

	m_iLast		= iLast;
	m_bOccupied	= true;
	m_pSubPtr	= pSubPtr;
	m_iFirst	= iFirst;

	if (m_iSectorNumber != iFirst)
		return;

	int iSize = 1;
	int iSegment = SEG_TYPE_NONE;

	if (m_eArcType == ARC_TYPE_SYSTEM_MEMORY)
	{
		if (pSubPtr->pTronSubroutine->eFunction == FUNCTION_BADBLOCK)
		{
			iSize = 1;
			iSegment = SEG_SYSTEM_BADBLOCK;
		}
		else
		{
			iSize = 3 - pSubPtr->eVersion;
			iSegment = pSubPtr->eVersion + SEG_SYSTEM_ALPHA;
		}
	}

	// The icon sits in the middle of the sectors the subroutine takes.
	float fAngle = m_fTheta1 + (m_fTheta2 - m_fTheta1) * iSize * 0.5f;
	if (fAngle > 2.0f * MATH_PI)
		fAngle -= 2.0f * MATH_PI;

	float fRadius = (m_fOuterRadius + m_fInnerRadius) * 0.5f;
	m_IconPos.x = m_Position.x - (int)(ltcosf(fAngle) * fRadius * -1.02f);
	m_IconPos.y = m_Position.y - (int)(ltsinf(fAngle) * fRadius * 0.98f);

	switch (pSubPtr->eState)
	{
		case SUBSTATE_OKAY:
			break;

		case SUBSTATE_CORRUPT:
		{
			if (m_eArcType == ARC_TYPE_SYSTEM_MEMORY)
			{
				m_Condition.Create(corruptionNames[pSubPtr->eVersion]);
				if (pSubPtr->nTronUnknown24 == 2)
					m_Condition.PlayAnim("left_right", false, false);
				else if (pSubPtr->nTronUnknown24 == 3)
					m_Condition.PlayAnim("right_left", false, false);
				else
					m_Condition.PlayAnim("center_out", false, false);
			}
			else
			{
				m_Condition.Create("corruption_lib");
				m_Condition.PlayAnim("loop", true, false);
			}
			m_Condition.SetAnimTime((int)(g_pSubroutineMgr->GetSubroutineCorruption(pSubPtr) * 100.0f));
		}
		break;

		case SUBSTATE_UNUSABLE:
		{
			if (pSubPtr->pTronSubroutine->eFunction == FUNCTION_BASECODE)
				iSegment = SEG_SYSTEM_BASECODE;
		}
		break;
	}

	if (pSubPtr->eState != SUBSTATE_FOREIGN)
	{
		INT_MENUPIECE * pPiece = ((CTronLayoutMgr *)g_pLayoutMgr)->GetSubroutinePiece(pSubPtr);
		if (iSegment == SEG_SYSTEM_BASECODE)
		{
			SAFE_STRCPY(pPiece->szModel, "Interface\\subroutines\\Models\\sys_basecode.ltb");
			pPiece->blrSkins.SetItem(0, "Interface\\subroutines\\Skins\\sys_basecode.dtx", 128);
		}
		m_Icon.Create(pPiece);
		if (pPiece)
		{
			debug_delete(pPiece);
		}

		if (pSubPtr->eState != SUBSTATE_UNUSABLE)
		{
			m_Ring.Create(ringNames[pSubPtr->eVersion]);
			m_Ring.PlayAnim("loop", true, false);
			m_Ring.Show(true);
		}
	}
	else
	{
		m_Icon.Create("foreign_sub");
		m_Ring.Create(ringNames[pSubPtr->eVersion]);
		m_Ring.PlayAnim("loop", true, false);
		m_Ring.Show(true);
	}

	m_Icon.PlayAnim("loop", true, false);
	m_Icon.Show(m_bShowSprites && m_bShowIcon);

	if (pSubPtr->pTronSubroutine->eFunction == FUNCTION_BASECODE)
		m_Icon.SetUseSocketRotation(true);
	else
		m_Icon.SetUseSocketRotation(false);

	char szSocket[64];

	if (m_eArcType == ARC_TYPE_LIBRARY)
	{
		sprintf(szSocket, "skt_lib_%d", 6 - m_iSectorNumber);
		m_Ring.Attach(m_szParent, szSocket);
		m_Icon.Attach(m_szParent, szSocket);

		if (m_pSubPtr && m_pSubPtr->eState == SUBSTATE_CORRUPT)
		{
			m_Condition.Attach(m_szParent, szSocket);
			m_Condition.Show(true);
		}
	}
	else
	{
		sprintf(szSocket, "skt_sys_%d", ((iSize - m_iAnimNumber * 2 + 47) % 48) + 1);
		m_Ring.Attach("sub_hub", szSocket);
		m_Icon.Attach("sub_hub", szSocket);

		if (m_pSubPtr && m_pSubPtr->eState == SUBSTATE_CORRUPT)
		{
			m_Condition.Attach(m_szParent, szSocket);
			m_Condition.Show(true);
		}

		if (iSegment != SEG_SYSTEM_BASECODE)
		{
			m_Segment.Create(segNames[iSegment]);
			m_Segment.Attach(m_szParent, szSocket);
			m_Segment.Show(true);
		}
	}
}

void CArcSector::ClearFX()
{
	m_Icon.Term();
	m_Ring.Term();
	m_Condition.Term();
	m_Segment.Term();

	RemoveHighlighting();

	m_bOccupied	= false;
	m_pSubPtr	= LTNULL;
	m_iFirst	= -1;
	m_iLast		= -1;
}

void CArcSector::AddHighlighting(SegmentType eSegType, int iSize)
{
	RemoveHighlighting();

	char szSocket[64];

	if (eSegType == SEG_LIBRARY_HIGHLIGHT)
	{
		m_Highlight.Create(segNames[SEG_LIBRARY_HIGHLIGHT]);
		sprintf(szSocket, "skt_lib_%d", 6 - m_iSectorNumber);
		m_Highlight.Attach(m_szParent, szSocket);
	}
	else
	{
		if (m_bOccupied && !(m_pSubPtr && (m_pSubPtr->pTronSubroutine->eFunction == FUNCTION_BADBLOCK ||
										   m_pSubPtr->pTronSubroutine->eFunction == FUNCTION_BASECODE)))
		{
			if (eSegType == SEG_SYSTEM_CONFLICT)
				m_Highlight.Create("sys_conflict");
			else
				m_Highlight.Create(highlightNames[iSize]);
		}
		else
		{
			m_Highlight.Create("sys_hi");
		}

		sprintf(szSocket, "skt_sys_%d", (iSize + (24 - m_iAnimNumber) * 2) % 48);
		m_Highlight.PlayAnim("hl", true, false);
		m_Highlight.Attach("sub_hub", szSocket);
	}

	if (m_pSubPtr)
	{
		m_Icon.PlayAnim("hl_on", false, false);
		m_Icon.SetNextAnim("hl_loop");
	}

	m_Highlight.Show(true);
	m_bHighlighted = true;
}

void CArcSector::RemoveHighlighting()
{
	if (!m_bHighlighted)
		return;

	m_Highlight.Term();
	m_bHighlighted = false;

	if (m_pSubPtr)
	{
		m_Icon.PlayAnim("hl_off", false, false);
		m_Icon.SetNextAnim("loop");
	}
}

//////////////////////////////////////////////////////////////////////
// CArcCtrl Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArcCtrl::CArcCtrl()
{
	m_pOutlinePrims		= LTNULL;
	m_pSeparatorPrims	= LTNULL;
	m_SectorArray		= LTNULL;
	m_nNumSegments		= 0;
}

CArcCtrl::~CArcCtrl()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Render
//
//	PURPOSE:	Draw the arc control's outlines on the screen; the
//				sectors' models draw themselves.
//
// ----------------------------------------------------------------------- //

LTBOOL CArcCtrl::Render()
{
	if (m_bHasChangedAppearance)
		Build();

	if (m_bShowOutlines)
	{
		if (m_nSeparatorPrims || m_nOutlinePrims)
		{
			g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
			g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
			g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
			g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
			g_pDrawPrim->SetColorOp(DRAWPRIM_NOCOLOROP);
			g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
			g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

			if (m_nSeparatorPrims)
				g_pDrawPrim->DrawPrim(m_pSeparatorPrims, m_nSeparatorPrims);
			if (m_nOutlinePrims)
				g_pDrawPrim->DrawPrim(m_pOutlinePrims, m_nOutlinePrims);
		}
	}
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Init()
//
//	PURPOSE:	These two functions set all initial values for a default controller
//
// ----------------------------------------------------------------------- //

LTBOOL CArcCtrl::Init()
{
	Term();

	// Get the screen center for m_Position
	m_Position.x = (int)((float)g_pInterfaceResMgr->GetScreenWidth() * 0.5f);
	m_Position.y = (int)((float)g_pInterfaceResMgr->GetScreenHeight() * 0.5f);

	m_fArcStart				= 0.0f;
	m_fArcEnd				= 0.0f;

	m_fInnerRadius			= 100.0f;
	m_fOuterRadius			= 200.0f;
	m_fThreshold			= 0.0f;

	m_nNumSegments			= 24;
	m_fSegmentTheta			= 2.0f * MATH_PI / (float)m_nNumSegments;
	m_nNumHighlightSegments	= 0;	// start in "ignore cursor" mode

	m_HighlightColor		= 0xFFC0C0FF;
	m_EmptyColor			= 0x80606060;
	m_OutlineColor			= 0xFF8080FF;
	m_ConflictColor			= 0xFFFF0000;

	m_bShowInactive			= LTTRUE;		// default to displaying the entire control
	m_bShowOutlines			= LTTRUE;		// default to outlining it in a thick line
	m_CurrentSegment		= -1;			// no active segment
	m_fMouseAngle			= -1.0f;
	m_iTesselation			= 1;			// Arbitrary value.  Break a segment

	m_SectorArray			= LTNULL;

	// Drawprim arrays
	m_nOutlinePrims			= 0;
	m_pOutlinePrims			= LTNULL;

	m_nSeparatorPrims		= 0;
	m_pSeparatorPrims		= LTNULL;

	m_bHasChangedAppearance = LTTRUE;

	m_bIsHot				= false;
	m_bEndHot				= false;
	m_bStartHot				= false;
	m_mouseX				= 0;
	m_mouseY				= 0;

	m_szParent				= LTNULL;

	m_eArcType				= ARC_TYPE_UNDEFINED;

	m_BasePosition.x		= 320;
	m_BasePosition.y		= 240;
	m_fBaseInnerRadius		= 100.0f;
	m_fBaseOuterRadius		= 200.0f;
	return LTTRUE;
}

LTBOOL CArcCtrl::Init(LTIntPt pos, float fInner, float fOuter, int nNumSegments)
{
	LTBOOL bResult = Init();

	float sx = g_pInterfaceResMgr->GetXRatio();

	m_BasePosition		= pos;
	m_fBaseInnerRadius	= fInner;
	m_fBaseOuterRadius	= fOuter;

	m_Position.x = (int)((float)pos.x * sx);
	m_Position.y = (int)((float)pos.y * g_pInterfaceResMgr->GetYRatio());
#ifdef TRON20SRC_MODERN
	m_Position.x += g_pInterfaceResMgr->GetXOffset();	// MODERN.md CL-30: on the centred page
	m_Position.y += g_pInterfaceResMgr->GetYOffset();
#endif

	m_nNumSegments = nNumSegments;
	m_fInnerRadius = fInner * sx;
	m_fOuterRadius = fOuter * sx;

	return bResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Term()
//
//	PURPOSE:	Clean up any garbage (dynamic memory, screen sprites)
//
// ----------------------------------------------------------------------- //


void CArcCtrl::Term()
{
	DEBUG_DELETEA(m_pOutlinePrims);
	DEBUG_DELETEA(m_pSeparatorPrims);
	DEBUG_DELETEA(m_SectorArray);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Rescale()
//
//	PURPOSE:	Scale the control's 640x480 values to the screen
//
// ----------------------------------------------------------------------- //

void CArcCtrl::Rescale()
{
	float sx = g_pInterfaceResMgr->GetXRatio();
	float sy = g_pInterfaceResMgr->GetYRatio();

	m_Position.x = (int)((float)m_BasePosition.x * sx);
	m_Position.y = (int)((float)m_BasePosition.y * sy);
#ifdef TRON20SRC_MODERN
	m_Position.x += g_pInterfaceResMgr->GetXOffset();	// MODERN.md CL-30: on the centred page
	m_Position.y += g_pInterfaceResMgr->GetYOffset();
#endif


	m_fInnerRadius = sx * m_fBaseInnerRadius;
	m_fOuterRadius = sx * m_fBaseOuterRadius;

	if (m_SectorArray)
	{
		for (int i = 0; i < m_nNumSegments; i++)
		{
			m_SectorArray[i].Resize(m_Position, m_fInnerRadius, m_fOuterRadius);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Parameters for controlling appearance and input
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::SetArc()
//
//	PURPOSE:	Allows the Arc's sweep to be something other than a circle
//				Note that the sweep should be defined counter-clockwise
//
// ----------------------------------------------------------------------- //

void CArcCtrl::SetArc(float fArcStart, float fArcEnd)
{
	m_fArcStart		= fArcStart * DEG_TO_RAD;
	m_fArcEnd		= fArcEnd * DEG_TO_RAD;

	// Crop angles to the the 0-2pi boundaries

	while (m_fArcStart > 2.0f * MATH_PI)
		m_fArcStart -= 2.0f * MATH_PI;
	while (m_fArcStart < 0.0f)
		m_fArcStart += 2.0f * MATH_PI;


	while (m_fArcEnd > 2.0f * MATH_PI)
		m_fArcEnd -= 2.0f * MATH_PI;
	while (m_fArcEnd < 0.0f)
		m_fArcEnd += 2.0f * MATH_PI;

	m_bHasChangedAppearance = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::SetNumHighlightSegments
//
//	PURPOSE:	Determine how many segments the cursor would take up if
//				Dragging and dropping.  If set to zero, then there is
//				no highlighting.  If set to -1, then we're in pickup mode
//
// ----------------------------------------------------------------------- //

void CArcCtrl::SetNumHighlightSegments(int num)
{
	if (m_bOnSegment)
	{
		RemoveHighlighting();
		m_nNumHighlightSegments = num;
		AddHighlighting();
	}
	else
	{
		m_nNumHighlightSegments = num;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::ShowOutlines
//
//	PURPOSE:	Sets flag for displaying outlines for sectors
//
// ----------------------------------------------------------------------- //

void CArcCtrl::ShowOutlines(LTBOOL bShowOutlines)
{
	m_bHasChangedAppearance = LTTRUE;
	m_bShowOutlines = bShowOutlines;
}

PlayerSubroutine * CArcCtrl::GetSegmentObject(int iSeg)
{
	if (iSeg >= 0 && iSeg < m_nNumSegments)
		return m_SectorArray[iSeg].GetSubObject();
	return LTNULL;
}

void CArcCtrl::AssociateModel(char * szParent)
{
	if (m_bHasChangedAppearance)
		Build();

	m_szParent = szParent;

	for (int i = 0; i < m_nNumSegments; i++)
	{
		m_SectorArray[i].SetParentPiece(szParent);
	}
}

PlayerSubroutine * CArcCtrl::GetHotSub()
{
	ASSERT(m_CurrentSegment < m_nNumSegments);

	if (m_CurrentSegment != -1 && m_CurrentSegment < m_nNumSegments)
		return m_SectorArray[m_CurrentSegment].GetSubObject();

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::GetHotPos
//
//	PURPOSE:	The screen position for the cursor: just past either end
//				of the arc, or the hot sector's icon
//
// ----------------------------------------------------------------------- //

LTIntPt CArcCtrl::GetHotPos()
{
	float fRadius = (m_fInnerRadius - m_fThreshold) + ((m_fOuterRadius + m_fThreshold) - (m_fInnerRadius - m_fThreshold)) * 0.5f;
	float fAngle;

	if (m_bStartHot)
	{
		fAngle = m_fArcStart - ARC_GAP_ANGLE * 0.5f;
	}
	else if (m_bEndHot)
	{
		fAngle = m_fArcEnd + ARC_GAP_ANGLE * 0.5f;
	}
	else
	{
		if (m_CurrentSegment != -1 && m_CurrentSegment < m_nNumSegments)
			return m_SectorArray[m_CurrentSegment].GetIconPos();

		return LTIntPt(0, 0);
	}

	int x = (int)(m_Position.x + ltcosf(fAngle) * fRadius);
	int y = (int)(m_Position.y - ltsinf(fAngle) * fRadius);
	return LTIntPt(x, y);
}

void CArcCtrl::ShowSprites(bool bShow)
{
	if (m_SectorArray)
	{
		for (int i = 0; i < m_nNumSegments; i++)
		{
			m_SectorArray[i].ShowSprites(bShow);
		}
	}
}

void CArcCtrl::ShowIcon(int iSeg, bool bShow)
{
	if (m_SectorArray)
	{
		m_SectorArray[iSeg].ShowIcon(bShow);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::OccupySegment
//
//	PURPOSE:	Put a subroutine in the segments starting at 'segment'
//
// ----------------------------------------------------------------------- //

void CArcCtrl::OccupySegment(int segment, PlayerSubroutine * pSub)
{
	if (m_bHasChangedAppearance)
		Build();

	if (segment == -1 || segment >= m_nNumSegments)
		return;

	RemoveHighlighting();

	int iFirst = segment;
	ASSERT(iFirst < 24);
	int iLast = segment + m_nNumHighlightSegments;
 	if (iLast >= m_nNumSegments)
		iLast -= m_nNumSegments;

	int i;
	for (i = 0; i < m_nNumHighlightSegments; i++)
	{
		int drawsegment = segment + i;
		if (drawsegment >= m_nNumSegments)
			drawsegment -= m_nNumSegments;

		m_SectorArray[drawsegment].Occupy(iFirst,
										  iLast,
										  pSub);
	}

	if (pSub->pProcedural)
	{
		m_SectorArray[segment].ShowIcon(false);
	}
	else
	{
		m_SectorArray[segment].ShowIcon(true);
	}

	AddHighlighting();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::ClearSegment
//
//	PURPOSE:	Remove the subroutine from a segment
//
// ----------------------------------------------------------------------- //

void CArcCtrl::ClearSegment(int segment)
{
	if (m_bHasChangedAppearance)
		Build();

	if (segment == -1 || segment >= m_nNumSegments)
		return;

	RemoveHighlighting();

	int iFirst, iLast;
	m_SectorArray[segment].GetRange(iFirst, iLast);
	ASSERT(iFirst < 24);

	if (iFirst != -1 && iLast != -1)
	{
		if (iLast < iFirst) iLast += m_nNumSegments;
		int iSize = iLast - iFirst;

		int i;
		for (i = 0; i < iSize; i++)
		{
			int drawsegment = segment + i;
			if (drawsegment >= m_nNumSegments)
				drawsegment -= m_nNumSegments;

			m_SectorArray[drawsegment].ClearFX();
		}
		AddHighlighting();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::OnMouseMove
//
//	PURPOSE:	Take the latest mouse coordinates and see if any changes
//				need to be made to the interface
//
// ----------------------------------------------------------------------- //

LTBOOL CArcCtrl::OnMouseMove(int mx, int my)
{
	m_mouseX = mx;
	m_mouseY = my;

	if (m_bHasChangedAppearance)
		Build();

	m_fMouseAngle = -1.0f;

	int iNewSegment = -1;

	if (IsOnMe(mx, my) && m_bOnSegment)
	{
		// calculate which sector the mouse is on
		// based on the angle and the cursor size
		float fAngleOffset;
		if (m_nNumHighlightSegments == -1)
		{
			fAngleOffset = 0.0f;
		}
		else
		{
			fAngleOffset = m_fSegmentTheta * (0.5f * (m_nNumHighlightSegments-1));
		}

		float fSectorAngle = m_fMouseAngle - fAngleOffset - m_fArcStart;
		if (fSectorAngle <= 0.0f)
			fSectorAngle += 2.0f * MATH_PI;

		iNewSegment = (int)(fSectorAngle / m_fSegmentTheta) % 24;
		if (iNewSegment == m_nNumSegments)
			iNewSegment--;

		// if we're in pickup mode, tweak "old" and "current" to align with any
		// highlighted sector(s).
		int iFirst, iLast;
		m_SectorArray[iNewSegment].GetRange(iFirst, iLast);
		if (m_nNumHighlightSegments == -1 && iFirst != -1)
		{
			iNewSegment = iFirst;
		}
	}

	if (iNewSegment != m_CurrentSegment)
	{
		RemoveHighlighting();
		m_CurrentSegment = iNewSegment;
		AddHighlighting();
	}

	return LTTRUE;
}

void CArcCtrl::RemoveHighlighting()
{
	if (m_CurrentSegment == -1)
		return;

	if (m_eArcType == ARC_TYPE_LIBRARY)
	{
		m_SectorArray[m_CurrentSegment].RemoveHighlighting();
		return;
	}

	int iNumDraw = m_nNumHighlightSegments;

	// in pickup mode, the highlighting is the one model of the hot sector
	if (iNumDraw == -1)
	{
		m_SectorArray[m_CurrentSegment].RemoveHighlighting();
	}
	else
	{
		for (int i = 0; i < iNumDraw; i++)
		{
			int drawsegment = m_CurrentSegment + i;
			if (drawsegment >= m_nNumSegments)
				drawsegment -= m_nNumSegments;

			m_SectorArray[drawsegment].RemoveHighlighting();
		}
	}
}

void CArcCtrl::AddHighlighting()
{
	if (m_CurrentSegment == -1)
		return;

	if (m_eArcType == ARC_TYPE_LIBRARY)
	{
		m_SectorArray[m_CurrentSegment].AddHighlighting(SEG_LIBRARY_HIGHLIGHT, 1);
		return;
	}

	int iNumDraw = m_nNumHighlightSegments;

	// the highlighting corresponds to exactly one occupied/unoccupied segment of 1-3 blocks.
	if (iNumDraw == -1)
	{
		int iFirst, iLast;
		m_SectorArray[m_CurrentSegment].GetRange(iFirst, iLast);

		if (iFirst != iNumDraw)
		{
			iNumDraw = iLast - iFirst;
			if (iNumDraw < 0)
				iNumDraw += m_nNumSegments;
		}
		else
		{
			iNumDraw = 1;
		}
	}

	if (m_nNumHighlightSegments == -1)
	{
		m_SectorArray[m_CurrentSegment].AddHighlighting(SEG_SYSTEM_HIGHLIGHT, iNumDraw);
		return;
	}

	for (int i = 0; i < iNumDraw; i++)
	{
		int drawsegment = m_CurrentSegment + i;
		if (drawsegment >= m_nNumSegments)
			drawsegment -= m_nNumSegments;

		if (m_SectorArray[drawsegment].IsOccupied())
			m_SectorArray[drawsegment].AddHighlighting(SEG_SYSTEM_CONFLICT, 1);
		else
			m_SectorArray[drawsegment].AddHighlighting(SEG_SYSTEM_HIGHLIGHT, 1);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::IsOnMe
//
//	PURPOSE:	Determine whether the mouse cursor is on the control:
//				on a sector, or in the gap just past either end of the arc
//
// ----------------------------------------------------------------------- //

LTBOOL CArcCtrl::IsOnMe(int mx, int my)
{
	m_bIsHot = false;
	m_bOnSegment = false;
	m_bEndHot = false;
	m_bStartHot = false;

	if (m_nNumHighlightSegments == 0)
		return LTFALSE;

	float fInner = m_fInnerRadius - m_fThreshold;
	float fOuter = m_fOuterRadius + m_fThreshold;

	// 1. Fast cull against rectangle
	if (mx > m_Position.x + fOuter || mx < m_Position.x - fOuter)
		return LTFALSE;

	if (my > m_Position.y + fOuter || my < m_Position.y - fOuter)
		return LTFALSE;

	// 2.  Fast cull against radii
	int dx = mx - m_Position.x;
	int dy = my - m_Position.y;
	float fDistSquared = (float)((dx * dx) + (dy * dy));

	if (fDistSquared > fOuter * fOuter)
		return LTFALSE;

	if (fDistSquared < fInner * fInner)
		return LTFALSE;

	m_fMouseAngle = VectorToAngle((float)dx, (float)-dy);

	// 3.  Optional cull against arc
	if (m_fArcStart == m_fArcEnd)
	{
		m_bOnSegment = m_bIsHot = true;
		return LTTRUE;
	}

	if (m_fArcEnd - m_fArcStart >= 2.0f * MATH_PI)
	{
		m_bIsHot = true;
		return LTTRUE;
	}

	if (m_fArcStart < m_fArcEnd)
	{
		if (m_fMouseAngle >= m_fArcStart - ARC_GAP_ANGLE && m_fMouseAngle <= m_fArcEnd + ARC_GAP_ANGLE)
		{
			if (m_fMouseAngle < m_fArcStart)
			{
				m_bIsHot = m_bStartHot = true;
			}
			else if (m_fMouseAngle > m_fArcEnd)
			{
				m_bIsHot = m_bEndHot = true;
			}
			else
			{
				m_bIsHot = m_bOnSegment = true;
			}
			return LTTRUE;
		}
	}
	else // check against an arc that spans the 360 degree end
	{
		if (m_fArcStart - ARC_GAP_ANGLE < m_fMouseAngle && m_fMouseAngle < 2.0f * MATH_PI)
		{
			if (m_fMouseAngle > m_fArcStart)
				m_bIsHot = m_bOnSegment = true;
			else
				m_bIsHot = m_bStartHot = true;
			return LTTRUE;
		}

		if (m_fMouseAngle < m_fArcEnd + ARC_GAP_ANGLE && m_fMouseAngle > 0.0f)
		{
			if (m_fMouseAngle < m_fArcEnd)
				m_bIsHot = m_bOnSegment = true;
			else
				m_bIsHot = m_bEndHot = true;
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::ComputeOptimalTesselation
//
//	PURPOSE:	Given the current parameters, set the most efficient
//				tesselation value for the rendering
//
// ----------------------------------------------------------------------- //

void CArcCtrl::ComputeOptimalTesselation()
{
	// Sanity check
	if (m_nNumSegments < 1)
		m_nNumSegments = 1;

	// tolerance is how close, in pixels, the approximation should be to a
	// true arc.
	float fTolerance = 1.0f;

	// Maximum tesselation is 24 segments
	const int iMaxTesselation = 24;

	// Start by testing with a tesselation of 1
	m_iTesselation = 1;

	// Here's my beautiful and elegant algorithm.

	// We want to determine how many straight line segments is the bare minimum
	// required in order to approximate the desired curve.  In this case, the
	// curve is an arc with a radius of 'm_fOuterRadius' and a sweep of 'fTheta'
	// radians.

	// The way we do this is to see where the line segment is farthest from the
	// true curve and minimize that distance.  The point where the line segment
	// is farthest is its midpoint.  So (stay with me here) if the line segment
	// is from zero degrees to fThetaPrime, where fThetaPrims = fTheta / iTess,
	// then we could take the midpoint of that line and compare its distance
	// from the origin to the radius.

	// Now here's the really clever bit.  If we took that line segment and
	// ROTATED it clockwise by HALF of fThetaPrime, then the line segment
	// would be perfectly vertical, and the midpoint would lie on the X axis,
	// making the comparison test trivial by eliminating magnitude tests.
	// To compute the x coordinate of the line segment is embarrassingly simple:
	// x = radius * cos(fThetaPrime / 2) !!!!!!

	// So to get the optimal value, we just increase iTess and compute x until
	// x is a tolerable distance from the true radius, or we hit our maximum
	// tesselation.  I rock.

	while (m_iTesselation < iMaxTesselation)
	{
		float fThetaPrime = m_fSegmentTheta / (float)m_iTesselation;
		float fLineX = m_fOuterRadius * ltcosf(fThetaPrime * 0.5);
		float fDelta = m_fOuterRadius - fLineX;

		// Exit when we have the tolerance we want!
		if (LTABS(fDelta) < fTolerance)
			return;

		m_iTesselation++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Build()
//
//	PURPOSE:	Reconstruct the drawprims and other computed variables.
//
// ----------------------------------------------------------------------- //
void CArcCtrl::Build()
{
	bool bCircle = false;

	if (m_fArcStart == m_fArcEnd)
	{
		bCircle = true;
	}
	else if (m_fArcEnd - m_fArcStart >= 2.0f * MATH_PI)
	{
		bCircle = true;
	}

	Term();

	m_SectorArray = debug_newa(CArcSector, m_nNumSegments);

	float fSweep = m_fArcEnd - m_fArcStart;
	if (fSweep <= 0.0f)
		fSweep += 2.0f * MATH_PI;

	m_fSegmentTheta = fSweep / (float)m_nNumSegments;

	ComputeOptimalTesselation();

	float fMiniSweep = m_fSegmentTheta / m_iTesselation;	// could theoretically be the same as fSegment


	// Segments
	// number of segment prims is m_iTesselation * m_nNumberSegments;
	m_nSegmentPrims = m_iTesselation * m_nNumSegments;

	// Separators
	// if circle, number of separator prims is m_nNumberSegments;
	// if arc, number of separator prims is m_nNumberSegments -1;
	m_nSeparatorPrims = m_nNumSegments - (bCircle ? 0 : 1);
	m_pSeparatorPrims = debug_newa(LT_POLYF4, m_nSeparatorPrims);

	// Outlines
	// number of outline prims is number of segment prims
	// if inner radius > 1, then number of outline prims is DOUBLE number of segment prims
	// if arc, number of outline prims += 2;
	m_nOutlinePrims = m_nSegmentPrims;
	if (m_fInnerRadius > 1.0f)
		m_nOutlinePrims += m_nSegmentPrims;
	if (!bCircle)
		m_nOutlinePrims += 2;
	m_pOutlinePrims = debug_newa(LT_POLYF4, m_nOutlinePrims);


	for (int i = 0; i < m_nNumSegments; i++)
	{
		float fTheta1 = m_fArcStart + (float)i * m_fSegmentTheta;
		float fTheta2 = fTheta1 + m_fSegmentTheta;

		m_SectorArray[i].Build(i,					// sector number
							  m_Position,			// center of arc
							  m_fInnerRadius,		// inner arc
							  m_fOuterRadius,		// outer arc
							  fTheta1,				// beginning of sweep
							  fTheta2,				// end of sweep
							  m_eArcType);
		m_SectorArray[i].SetParentPiece(m_szParent);
//							  m_iTesselation);		// number of polies to use
		
		// Check to draw separators
		if (m_bShowOutlines)
		{
			LT_POLYF4 * pSepPoly = LTNULL;
			if (bCircle)
			{
				pSepPoly = &m_pSeparatorPrims[i];
			}
			else if (i > 0)
			{
				pSepPoly = &m_pSeparatorPrims[i-1];
			}
			if (pSepPoly)
			{
				BuildPoly(pSepPoly, m_Position, m_fInnerRadius, m_fOuterRadius, fTheta1, fTheta1 + DEG_TO_RAD * 0.5f);
				g_pDrawPrim->SetRGBA(pSepPoly, m_OutlineColor);
			}

		}

		for (int j = 0; j < m_iTesselation; j++)
		{
			float fThetaA = fTheta1 + (float)j * fMiniSweep;
			float fThetaB = fThetaA + fMiniSweep;
			
			// Check for outlines here.
			if (m_bShowOutlines)
			{
				LT_POLYF4 * pPoly = &m_pOutlinePrims[i * m_iTesselation + j];
				BuildPoly(pPoly, m_Position, m_fOuterRadius, m_fOuterRadius + 2.0f, fThetaA, fThetaB);
				g_pDrawPrim->SetRGBA(pPoly, m_OutlineColor);

				if (m_fInnerRadius > 1.0f)
				{
					pPoly = &m_pOutlinePrims[i * m_iTesselation + j + m_nSegmentPrims];
					BuildPoly(pPoly, m_Position, m_fInnerRadius - 2.0f, m_fInnerRadius, fThetaA, fThetaB);
					g_pDrawPrim->SetRGBA(pPoly, m_OutlineColor);
				}
			}
		}
	}

	// If it's an arc, cap it.
	if (m_bShowOutlines && !bCircle)
	{
		BuildPoly(&m_pOutlinePrims[m_nOutlinePrims-2], m_Position, m_fInnerRadius-2.0f, m_fOuterRadius+2.0f, m_fArcStart, m_fArcStart - DEG_TO_RAD*0.5f);
		BuildPoly(&m_pOutlinePrims[m_nOutlinePrims-1], m_Position, m_fInnerRadius-2.0f, m_fOuterRadius+2.0f, m_fArcEnd, m_fArcEnd + DEG_TO_RAD*0.5f);
		g_pDrawPrim->SetRGBA(&m_pOutlinePrims[m_nOutlinePrims-2], m_OutlineColor);
		g_pDrawPrim->SetRGBA(&m_pOutlinePrims[m_nOutlinePrims-1], m_OutlineColor);
	}

	m_bHasChangedAppearance = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::VectorToAngle
//
//	PURPOSE:	Take a vector from origin (in cartesian space, not screen space)
//				Return the corresponding angle in radians
//
// ----------------------------------------------------------------------- //

float CArcCtrl::VectorToAngle( float x, float y )
{
	// easy case, vector is too small
	if ( ( EPSILON > LTABS( x ) ) &&
	     ( EPSILON > LTABS( y ) ) )
	{
		return 0.0f;
	}

	// are we on the y-axis?
	if (LTABS(x) < EPSILON)
	{
		return (y > 0.0f ? MATH_HALFPI :  1.5f * MATH_PI);
	}
	// are we on the x-axis?
	if (LTABS(y) < EPSILON)
	{
		return (x > 0.0f ? 0 : MATH_PI);
	}

	float result;
	if ( ( 0.0f <= x ) && ( 0.0f <= y ) )
	{
		// quadrant 1
		result = ltatanf( static_cast< double >( y / x ) );
	}
	else if ( ( 0.0f >= x ) && ( 0.0f <= y ) )
	{
		// quadrant 2
		result = ( MATH_PI - ltatanf( static_cast< double >( - y / x ) ) );
	}
	else if ( ( 0.0f >= x ) && ( 0.0f >= y ) )
	{
		// quadrant 3
		result = ( MATH_PI - ltatanf( static_cast< double >( - y / x ) ) );
	}
	else if ( ( 0.0f <= x ) && ( 0.0f >= y ) )
	{
		// quadrant 4
		result = ( 2 * MATH_PI + ltatanf( static_cast< double >( y / x ) ) );
	}
	else
	{
		// shouldn't be able to get here
		ASSERT( 0 );
		result = 0.0f;
	}
	return result;
}

