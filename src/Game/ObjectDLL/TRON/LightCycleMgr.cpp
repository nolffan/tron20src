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
#include "GameServerShell.h"

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
//	ROUTINE:	CLightCycleMgr::CheckForCollision
//
//	PURPOSE:	Does the segment ptStart..ptEnd of a cyclist hit a live
//				cycle, or a trail? The first hit is put in pInfo (its
//				nTrail -1 for a cycle). Also keeps the nearest wall a
//				cycle runs beside (nTronUnknownA4) and its side
//				(bTronUnknownAC).
//
//	RETAIL: 0x100bbef0 (Tron's; the names of the parameters and locals
//	are ours)
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::CheckForCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL_POINT& ptStart, LIGHT_CYCLE_TRAIL_POINT& ptEnd,
	LightCycleCollisionInfo* pInfo, bool bCheckCycles, LIGHT_CYCLIST* pIgnore)
{
	if(!pCyclist)
		return false;

	pCyclist->nTronUnknownA4 = pCyclist->nMaxWallDist + 100;

	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		LIGHT_CYCLIST* pOther = (*iter);

		// The cycles first
		if(bCheckCycles && (pOther->fStartDerezTime == 0.0f))
		{
			if((!pCyclist->bTronUnknownD1 && (pOther != pCyclist)) || (pCyclist->bTronUnknownD1 && (pOther != pIgnore)))
			{
				float fMinX = LTMIN(ptEnd.GetX(), ptStart.GetX());
				float fMaxX = LTMAX(ptEnd.GetX(), ptStart.GetX());
				float fMinZ = LTMIN(ptEnd.GetZ(), ptStart.GetZ());
				float fMaxZ = LTMAX(ptEnd.GetZ(), ptStart.GetZ());

				if(pCyclist->vForward.x == 0.0f)
				{
					fMinX -= 16.0f;
					fMaxX += 16.0f;
					fMinZ -= pCyclist->fHalfHeight;
					fMaxZ += pCyclist->fHalfHeight;
				}
				else
				{
					fMinZ -= 16.0f;
					fMaxZ += 16.0f;
					fMinX -= pCyclist->fHalfHeight;
					fMaxX += pCyclist->fHalfHeight;
				}

				float fOtherMinX, fOtherMaxX, fOtherMinZ, fOtherMaxZ;
				if(pOther->vForward.x == 0.0f)
				{
					fOtherMinX = pOther->vPos.x - 16.0f;
					fOtherMaxX = pOther->vPos.x + 16.0f;
					fOtherMinZ = pOther->vPos.z - pOther->fHalfHeight;
					fOtherMaxZ = pOther->vPos.z + pOther->fHalfHeight;
				}
				else
				{
					fOtherMinX = pOther->vPos.x - pOther->fHalfHeight;
					fOtherMaxX = pOther->vPos.x + pOther->fHalfHeight;
					fOtherMinZ = pOther->vPos.z - 16.0f;
					fOtherMaxZ = pOther->vPos.z + 16.0f;
				}

				if((fOtherMaxX >= fMinX) && (fOtherMinX <= fMaxX) && (fOtherMaxZ >= fMinZ) && (fOtherMinZ <= fMaxZ))
				{
					pInfo->pCyclist = pOther;
					pInfo->nTrail = -1;

					if(pCyclist->bTronUnknownD1)
						return true;

					if((pCyclist->dwFlags & 1) && !(pOther->dwFlags & 1))
					{
						// Only we have it: the other one derezzes
						DerezCyclist(pOther, pCyclist, true);
					}
					else
					{
						if(!(pCyclist->dwFlags & 1) && (pOther->dwFlags & 1))
							return true;

						// Head on?
						bool bAlongX = (pCyclist->vForward.x != 0.0f);
						bool bOtherAlongX = (pOther->vForward.x != 0.0f);
						if(bAlongX == bOtherAlongX)
						{
							if(bAlongX)
							{
								if(pCyclist->vForward.x != pOther->vForward.x)
								{
									HandleHeadOnCollision(pCyclist, pOther);
									return true;
								}
							}
							else if(pCyclist->vForward.z != pOther->vForward.z)
							{
								HandleHeadOnCollision(pCyclist, pOther);
								return true;
							}
						}

						return true;
					}
				}
			}
		}

		// Then its trails
		std::vector<LIGHT_CYCLE_TRAIL*>::iterator trailIter;
		for(trailIter=pOther->collTrails.begin();trailIter!=pOther->collTrails.end();trailIter++)
		{
			LIGHT_CYCLE_TRAIL* pTrail = (*trailIter);
			int nLastPoint = -1;

			// Not the end of our own trail, we are on it
			LIGHT_CYCLIST* pSelf = pIgnore ? pIgnore : pCyclist;
			if((pOther == pSelf) && (pTrail == pSelf->pCurTrail))
			{
				if(pTrail->collTrailPoints.size() >= 4)
					nLastPoint = pTrail->collTrailPoints.size() - 4;
			}
			else
			{
				nLastPoint = pTrail->collTrailPoints.size() - 2;
			}

			int nEndX = (int)ptEnd.GetX();
			int nEndZ = (int)ptEnd.GetZ();
			int nStartX = (int)ptStart.GetX();
			int nStartZ = (int)ptStart.GetZ();

			for(int i=0;i<=nLastPoint;i++)
			{
				LIGHT_CYCLE_TRAIL_POINT* pPt1 = &pTrail->collTrailPoints[i];
				LIGHT_CYCLE_TRAIL_POINT* pPt2 = &pTrail->collTrailPoints[i+1];

				int nX1 = (int)pPt1->GetX();
				int nZ1 = (int)pPt1->GetZ();
				int nX2 = (int)pPt2->GetX();
				int nZ2 = (int)pPt2->GetZ();

				if(nX1 == nX2)
				{
					// A wall along z
					if(nStartX == nEndX)
					{
						// We go along it
						int nHalf = (int)pCyclist->fHalfLength;
						if((nEndX - nHalf <= nX1) && (nEndX + nHalf >= nX1))
						{
							int nMinZ = (nStartZ < nEndZ) ? nStartZ : nEndZ;
							int nMaxZ = (nStartZ > nEndZ) ? nStartZ : nEndZ;
							int nWallMinZ = (nZ1 < nZ2) ? nZ1 : nZ2;
							int nWallMaxZ = (nZ1 > nZ2) ? nZ1 : nZ2;
							if((nMinZ <= nWallMaxZ) && (nMaxZ >= nWallMinZ))
							{
								pInfo->pCyclist = pOther;
								pInfo->nTrail = trailIter - pOther->collTrails.begin();
								pInfo->nPoint = i;
								pInfo->vCollisionPoint.x = ptEnd.GetX();
								pInfo->vCollisionPoint.y = ptEnd.GetY();
								if(nEndZ > nStartZ)
									pInfo->vCollisionPoint.z = (float)((nStartZ > nWallMinZ) ? nStartZ : nWallMinZ);
								else
									pInfo->vCollisionPoint.z = (float)((nStartZ < nWallMaxZ) ? nStartZ : nWallMaxZ);
								return true;
							}
						}
						else
						{
							// How near is it?
							int nDist1 = abs((nEndX - nHalf) - nX1);
							int nDist2 = abs((nEndX + nHalf) - nX1);
							int nDist = (nDist1 < nDist2) ? nDist1 : nDist2;
							if(nDist < pCyclist->nTronUnknownA4)
							{
								int nMinZ = (nStartZ < nEndZ) ? nStartZ : nEndZ;
								int nMaxZ = (nStartZ > nEndZ) ? nStartZ : nEndZ;
								int nWallMinZ = (nZ1 < nZ2) ? nZ1 : nZ2;
								int nWallMaxZ = (nZ1 > nZ2) ? nZ1 : nZ2;
								if((nMinZ < nWallMaxZ) && (nMaxZ > nWallMinZ))
								{
									pCyclist->nTronUnknownA4 = nDist;
									if(nDist1 < nDist2)
										pCyclist->bTronUnknownAC = (nEndZ > nStartZ);
									else
										pCyclist->bTronUnknownAC = (nEndZ < nStartZ);
								}
							}
						}
					}
					else
					{
						// We cross it
						int nHalf = (int)pCyclist->fHalfLength;
						int nWallMinZ = (nZ1 < nZ2) ? nZ1 : nZ2;
						int nWallMaxZ = (nZ1 > nZ2) ? nZ1 : nZ2;
						if((nEndZ - nHalf <= nWallMaxZ) && (nEndZ + nHalf >= nWallMinZ))
						{
							int nMinX = (nStartX < nEndX) ? nStartX : nEndX;
							int nMaxX = (nStartX > nEndX) ? nStartX : nEndX;
							if((nMinX <= nX1) && (nMaxX >= nX1))
							{
								pInfo->pCyclist = pOther;
								pInfo->nTrail = trailIter - pOther->collTrails.begin();
								pInfo->nPoint = i;
								pInfo->vCollisionPoint.x = (float)nX1;
								pInfo->vCollisionPoint.y = ptEnd.GetY();
								pInfo->vCollisionPoint.z = ptEnd.GetZ();
								return true;
							}
						}
					}
				}
				else
				{
					// A wall along x
					if(nStartZ == nEndZ)
					{
						// We go along it
						int nHalf = (int)pCyclist->fHalfLength;
						if((nEndZ - nHalf <= nZ1) && (nEndZ + nHalf >= nZ1))
						{
							int nMinX = (nStartX < nEndX) ? nStartX : nEndX;
							int nMaxX = (nStartX > nEndX) ? nStartX : nEndX;
							int nWallMinX = (nX1 < nX2) ? nX1 : nX2;
							int nWallMaxX = (nX1 > nX2) ? nX1 : nX2;
							if((nMinX <= nWallMaxX) && (nMaxX >= nWallMinX))
							{
								pInfo->pCyclist = pOther;
								pInfo->nTrail = trailIter - pOther->collTrails.begin();
								pInfo->nPoint = i;
								if(nEndX > nStartX)
									pInfo->vCollisionPoint.x = (float)((nStartX > nWallMinX) ? nStartX : nWallMinX);
								else
									pInfo->vCollisionPoint.x = (float)((nStartX < nWallMaxX) ? nStartX : nWallMaxX);
								pInfo->vCollisionPoint.y = ptEnd.GetY();
								pInfo->vCollisionPoint.z = ptEnd.GetZ();
								return true;
							}
						}
						else
						{
							// How near is it?
							int nDist1 = abs((nEndZ - nHalf) - nZ1);
							int nDist2 = abs((nEndZ + nHalf) - nZ1);
							int nDist = (nDist1 < nDist2) ? nDist1 : nDist2;
							if(nDist < pCyclist->nTronUnknownA4)
							{
								int nMinX = (nStartX < nEndX) ? nStartX : nEndX;
								int nMaxX = (nStartX > nEndX) ? nStartX : nEndX;
								int nWallMinX = (nX1 < nX2) ? nX1 : nX2;
								int nWallMaxX = (nX1 > nX2) ? nX1 : nX2;
								if((nMinX < nWallMaxX) && (nMaxX > nWallMinX))
								{
									pCyclist->nTronUnknownA4 = nDist;
									if(nDist2 < nDist1)
										pCyclist->bTronUnknownAC = (nEndX > nStartX);
									else
										pCyclist->bTronUnknownAC = (nEndX < nStartX);
								}
							}
						}
					}
					else
					{
						// We cross it
						int nHalf = (int)pCyclist->fHalfLength;
						int nWallMinX = (nX1 < nX2) ? nX1 : nX2;
						int nWallMaxX = (nX1 > nX2) ? nX1 : nX2;
						if((nEndX - nHalf <= nWallMaxX) && (nEndX + nHalf >= nWallMinX))
						{
							int nMinZ = (nStartZ < nEndZ) ? nStartZ : nEndZ;
							int nMaxZ = (nStartZ > nEndZ) ? nStartZ : nEndZ;
							if((nMinZ <= nZ1) && (nMaxZ >= nZ1))
							{
								pInfo->pCyclist = pOther;
								pInfo->nTrail = trailIter - pOther->collTrails.begin();
								pInfo->nPoint = i;
								pInfo->vCollisionPoint.x = ptEnd.GetX();
								pInfo->vCollisionPoint.y = ptEnd.GetY();
								pInfo->vCollisionPoint.z = (float)nZ1;
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::SnapToGrid
//
//	PURPOSE:	Rounds a position's x and z to the light cycle grid
//
//	RETAIL: 0x100bca50 (written for AILightCycle, which calls it)
//
// ----------------------------------------------------------------------- //

#define LCMGR_ROUND(x)		((int)((x) + (((x) > 0.0f) ? 0.5 : -0.5)))

void CLightCycleMgr::SnapToGrid(LTVector& vPos)
{
	float fGridSize = m_LightCycleButeMgr.GetGridSize();

	int nX = LCMGR_ROUND(vPos.x);
	if (fGridSize > 1.0f)
	{
		nX = LCMGR_ROUND(nX / fGridSize);
		vPos.x = nX * fGridSize;
	}
	else
	{
		vPos.x = (float)nX;
	}

	int nZ = LCMGR_ROUND(vPos.z);
	if (fGridSize > 1.0f)
	{
		nZ = LCMGR_ROUND(nZ / fGridSize);
		vPos.z = nZ * fGridSize;
	}
	else
	{
		vPos.z = (float)nZ;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::GetDifficulty
//
//	PURPOSE:	The game's difficulty
//
//	RETAIL: 0x100be510 (written for AILightCycle, which calls it)
//
// ----------------------------------------------------------------------- //

int CLightCycleMgr::GetDifficulty()
{
	return g_pGameServerShell->GetDifficulty();
}
