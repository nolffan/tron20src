/****************************************************************************
;
;	MODULE:			LightCycleMgr.cpp
;
;	PURPOSE:		Light Cycle Manager (SERVER) for TRON
;
;	HISTORY:		4/19/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "LightCycleMgr.h"
#include "MsgIDs.h"

// RETAIL: g_pLightCycleMgr is defined with g_pServerLightCycleMgr in
// ServerLightCycleMgr.cpp (their data is adjacent, 0x1035a54c).
//
// PORT: the functions marked so are build 69's or stand-ins; retail's base
// (0x100bb180..0x100be640) is not written yet.

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LIGHT_CYCLIST::FindTrail
//
//	PURPOSE:	Looks for a trail in the list by unique ID
//
//	PORT: build 69's (retail's trails have no id at build 69's place)
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLE_TRAIL* LIGHT_CYCLIST::FindTrail(uint16 wTrailID)
{
	std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter;
	for(iter=collTrails.begin();iter!=collTrails.end();iter++)
	{
		if((*iter)->nTronUnknown10 == wTrailID)
			return(*iter);
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CLightCycleMgr
//
//	PURPOSE:	Constructor
//
//	RETAIL: 0x100bb180
//
// ----------------------------------------------------------------------- //
CLightCycleMgr::CLightCycleMgr()
{
	m_nDifficulty = 0;
	m_fTronUnknown20 = 500.0f;
	m_fGameSpeedScalar = 1.0f;
	m_fTrailLength = 30000.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::Init
//
//	PURPOSE:	Initialization
//
//	RETAIL: 0x100bb200
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::Init()
{
	m_bUpdating = true;
	if(!m_LightCycleButeMgr.Init(LCBMGR_DEFAULT_FILE))
		return false;

	RemoveAllCyclists();
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::Term
//
//	PURPOSE:	Cleanup
//
//	RETAIL: 0x100bb230
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::Term()
{
	RemoveAllCyclists();
	g_pLightCycleMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::AddCyclist
//
//	PURPOSE:	Creates a new cyclist and adds it to the list
//
//	PORT: build 69's, without retail's set up from the light cycle butes
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST* CLightCycleMgr::AddCyclist(HOBJECT hObj, const char* szCycleColor, bool bUpdate,
	const LTVector* pvDims, uint32 nNameId, bool bTronUnknown)
{
	// Allocate
	LIGHT_CYCLIST* pCyclist = debug_new(LIGHT_CYCLIST);
	pCyclist->bUpdating = bUpdate;
	pCyclist->hObject = hObj;
	pCyclist->bIsPlayer = (pvDims != NULL);

	// Add to our list
	m_collCyclists.push_back(pCyclist);
	return pCyclist;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::RemoveCyclist
//
//	PURPOSE:	Marks a light cycle for removal
//
//	RETAIL: 0x100bb7f0
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::RemoveCyclist(HOBJECT hObj)
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		if((*iter)->hObject == hObj)
		{
			(*iter)->bRemove = true;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::FindCyclist
//
//	PURPOSE:	Looks for a cyclist in the list by its object
//
//	RETAIL: 0x100bbe10
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST* CLightCycleMgr::FindCyclist(HOBJECT hObj)
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		if((*iter)->hObject == hObj)
			return(*iter);
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::GetActiveCyclist
//
//	PURPOSE:	The object's cyclist if it is not derezzing
//
//	RETAIL: 0x100bdde0 (uses g_pLightCycleMgr, not this)
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST* CLightCycleMgr::GetActiveCyclist(HOBJECT hObj)
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(hObj);
	if(pCyclist && (pCyclist->fStartDerezTime == 0.0f))
		return pCyclist;

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::BeginLightCycleTrail
//
//	PURPOSE:	Starts a new light cycle trail
//
//	PORT: build 69's
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::BeginLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos, bool bTronUnknown)
{
	ASSERT(pCyclist);
	if(!pCyclist)
		return false;

	// Create a new trail
	LIGHT_CYCLE_TRAIL* pTrail = debug_new(LIGHT_CYCLE_TRAIL);

	// Add this trail to the light cycle's list
	pCyclist->collTrails.push_back(pTrail);

	// Set our current trail
	pCyclist->pCurTrail = pTrail;

	// Add the point
	if(AddLightCycleTrailPoint(pCyclist, pTrail, vPos) > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::RemoveAllCyclists
//
//	RETAIL: 0x100bd110
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::RemoveAllCyclists()
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		debug_delete(*iter);
		*iter = NULL;
	}

	m_collCyclists.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::SetPowerup
//
//	RETAIL: 0x100bd300
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::SetPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup)
{
	if(pCyclist && ((nPowerup < kLCPowerup_Random) || (nPowerup == LC_NO_POWERUP)))
	{
		pCyclist->nPowerup = nPowerup;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::TronUnknownVirtual17
//
//	RETAIL: 0x100bd4e0
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::TronUnknownVirtual17(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown)
{
	pCyclist->dwFlags &= ~1;
}

// ----------------------------------------------------------------------- //
//
//	PORT: stand-ins for retail's base functions not written yet
//	(0x100bb990, 0x100bd170, 0x100bd330, 0x100bd4f0, 0x100bd980,
//	0x100be0b0, 0x100bcb60, 0x100be4a0)
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::AddTrailPoints(LIGHT_CYCLIST* pCyclist, bool bTronUnknown1, LTVector& vPos, bool bTronUnknown2)
{
	if(!pCyclist->pCurTrail)
		return BeginLightCycleTrail(pCyclist, pCyclist->vPos, false);

	return UpdateLightCycleTrail(pCyclist, pCyclist->vPos);
}

void CLightCycleMgr::TronUnknownUpdate()
{
}

// PORT: retail's 0x1013c350 is in a TU not identified yet
int TronUnknownGetRandomPowerup()
{
	return 0;
}

bool CLightCycleMgr::ActivatePowerup(LIGHT_CYCLIST* pCyclist, const LTVector& vPos)
{
	if(pCyclist->nPowerup == LC_NO_POWERUP)
		return false;

	pCyclist->nPowerup = LC_NO_POWERUP;
	return true;
}

bool CLightCycleMgr::HandleCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo* pInfo, bool bTronUnknown1, bool bTronUnknown2)
{
	return DerezCyclist(pCyclist, pInfo->pCyclist, false);
}

bool CLightCycleMgr::TronUnknownVirtual9(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown1, uint32 nTronUnknown2, bool bTronUnknown)
{
	return true;
}

void CLightCycleMgr::TronUnknownVirtual11(LIGHT_CYCLIST* pCyclist)
{
}

bool CLightCycleMgr::TronUnknownVirtual12(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown)
{
	return false;
}

void CLightCycleMgr::TronUnknownVirtual18()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::UpdateLightCycleTrail
//
//	PURPOSE:	Updates a ligt cyclist's trail
//
// ----------------------------------------------------------------------- //
// PORT: build 69's; retail's (0x100bde10) has no trail id argument
bool CLightCycleMgr::UpdateLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos)
{
	ASSERT(pCyclist);
	if(!pCyclist)
		return false;

	LIGHT_CYCLE_TRAIL_POINT newPoint;
	LIGHT_CYCLE_TRAIL_POINT *pOldPoint;
	CreateTrailPointFromVector(newPoint,vPos);

	// See which trail we're dealing with
	LIGHT_CYCLE_TRAIL* pTrail = pCyclist->pCurTrail;

	ASSERT(pTrail);
	if(!pTrail)
	{
		// We tried to update a non-existant trail
		return false;
	}

	int nPoints = pTrail->collTrailPoints.size();

	if(nPoints > 0)
	{
		// Get the last point
		pOldPoint = &(pTrail->collTrailPoints[nPoints-1]);

		// Save the old forward vector
		LTVector vOldForward = pCyclist->vForward;

		// Compute the forward vector
		pCyclist->vForward.x = newPoint.GetX() - pOldPoint->GetX();
		pCyclist->vForward.y = newPoint.GetY() - pOldPoint->GetY();
		pCyclist->vForward.z = newPoint.GetZ() - pOldPoint->GetZ();
		pCyclist->vForward.Normalize();

		if(nPoints > 1)
		{
			// Check for a turn
			float fDot = (float)(fabs(vOldForward.Dot(pCyclist->vForward)));
			
			// Check for a turn
			if(fDot <= MATH_EPSILON)
			{
				// We have a turn, so we have to do two things:
				// 1) Change our last point into the actual turn point
				// 2) Add on our current location point

				LIGHT_CYCLE_TRAIL_POINT turnPoint;

				// Check to see which axis we were travelling on
				if(((float)fabs(vOldForward.z)) > MATH_EPSILON)
				{
					// We were travelling in the Z direction and now we've turned

					// Set the new forward vector
					if(newPoint.GetX() > pOldPoint->GetX())
					{
						pCyclist->vForward.x = 1.0f;
					}
					else
					{
						pCyclist->vForward.x = -1.0f;
					}

					pCyclist->vForward.z = 0.0f;

					// Now calculate the turn point

					/* o = old point, n = new point, t = turn point

						|	o
						|	|\
					   z|	| \
						|	t--n
						|
						+--------
							x 
					*/
					
					// Turn point is the old X and new Z
					turnPoint.SetX(pOldPoint->GetX());
					turnPoint.SetZ(newPoint.GetZ());
				}
				else
				{
					// We were travelling in the X direction and now we've turned

					// Set the new forward vector
					if(newPoint.GetZ() > pOldPoint->GetZ())
					{
						pCyclist->vForward.z = 1.0f;
					}
					else
					{
						pCyclist->vForward.z = -1.0f;
					}

					pCyclist->vForward.x = 0.0f;

					// Now calculate the turn point

					/* o = old point, n = new point, t = turn point

						|	o--t
						|	 \ |
					   z|	  \|
						|	   n
						|
						+-------
							x
					*/
					
					// Turn point is the old Z, and new X
					turnPoint.SetX(newPoint.GetX());
					turnPoint.SetZ(pOldPoint->GetZ());
				}

				turnPoint.SetY(newPoint.GetY());
				if(pOldPoint->GetY() > (newPoint.GetY() + MATH_EPSILON))
				{
					// We shouldn't be going up/down ramps!
					ASSERT(FALSE);
				}

				// Change the last point to the turn point
				pOldPoint->SetX(turnPoint.GetX());
				pOldPoint->SetY(turnPoint.GetY());
				pOldPoint->SetZ(turnPoint.GetZ());

				// Now add on our current location point
				pTrail->collTrailPoints.push_back(newPoint);

				/************************ TEMP */
						// Let's walk his list of points
						std::vector<LIGHT_CYCLE_TRAIL_POINT>::iterator iter;

						// Walk the list
						int nPoint = 0;
						g_pLTServer->CPrint("\n****\n");
						for(iter=pTrail->collTrailPoints.begin();iter!=pTrail->collTrailPoints.end();iter++)
						{
							g_pLTServer->CPrint("Point %d: <%f, %f, %f>\n",nPoint,iter->GetX(),iter->GetY(),iter->GetZ());
							nPoint++;
						}	
						g_pLTServer->CPrint("****\n");
				/************************ TEMP */
			}
			else
			{
				// We're continuing in the same direction, 
				// so we'll just change the last point to our location
				pOldPoint->SetX(newPoint.GetX());
				pOldPoint->SetY(newPoint.GetY());
				pOldPoint->SetZ(newPoint.GetZ());
			}
		}
		else
		{
			// We only have one point on our list (the starting point)
			// so add a new one
			pTrail->collTrailPoints.push_back(newPoint);
		}
	}
	else
	{
		// We shouldn't ever have no points when we update
		ASSERT(FALSE);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::AddLightCycleTrailPoint
//
//	PURPOSE:	Adds a point onto a light cycle trail
//
// ----------------------------------------------------------------------- //
uint32 CLightCycleMgr::AddLightCycleTrailPoint(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL* pTrail, LTVector& vPos)
{
	ASSERT(pCyclist);
	if(!pCyclist)
		return 0;

	ASSERT(pTrail);
	if(!pTrail)
		return 0;

	LIGHT_CYCLE_TRAIL_POINT newPoint;
	CreateTrailPointFromVector(newPoint,vPos);

	// Add the new point
	pTrail->collTrailPoints.push_back(newPoint);

	return pTrail->collTrailPoints.size();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CreateTrailPointFromVector
//
//	PURPOSE:	Creates a trail point from a vector and will do
//				grid-snapping or alignment if we ever need it
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::CreateTrailPointFromVector(LIGHT_CYCLE_TRAIL_POINT &pt, LTVector &vPos)
{
	// For now, it's just a straight copy
	pt.vPoint = vPos;
}
