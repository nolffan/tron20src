// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMgr.cpp
//
// PURPOSE : The light cycle manager (the base of the client's and the
//           server's), see LightCycleMgr.h
//
// RETAIL: written from retail CShell.dll 0x100fe3b0..0x10101a40; the
// functions are in retail's order. Object.lto has the same TU at
// 0x100bb200.. (the Build 69 ancestor: ObjectDLL/TRON/LightCycleMgr.cpp).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleMgr.h"
#include "CommonUtilities.h"

#ifdef _CLIENTBUILD
#include "GameClientShell.h"
#define LCM_LT	g_pLTClient
#else
#define LCM_LT	g_pLTServer
#endif

CLightCycleMgr* g_pLightCycleMgr = NULL;	// RETAIL: CShell 0x10236d98

// RETAIL: the rounding of BeginLightCycleTrail (inline; the name is ours)
#define LCM_ROUND(fVal)	((int)((fVal) + ((fVal) > 0.0f ? 0.5 : -0.5)))

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LCMGroundFilterFn
//
//	PURPOSE:	The ground intersect filter of AddCyclist: skips the cycle.
//				RETAIL: 0x100fe3b0 (the name is ours)
//
// ----------------------------------------------------------------------- //
static bool LCMGroundFilterFn(HOBJECT hTest, void *pUserData)
{
	return ((HOBJECT)pUserData != hTest);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LIGHT_CYCLIST::~LIGHT_CYCLIST
//
//	PURPOSE:	Destructor. RETAIL: 0x100fe3c0
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST::~LIGHT_CYCLIST()
{
	std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter;
	for(iter=collTrails.begin();iter!=collTrails.end();iter++)
	{
		debug_delete(*iter);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LIGHT_CYCLIST::ClearTrailSpike
//
//	PURPOSE:	Removes the trail spike's trails. RETAIL: 0x100fe4e0
//
// ----------------------------------------------------------------------- //
void LIGHT_CYCLIST::ClearTrailSpike()
{
	if(((uint8)(nFlags >> 2) & 1) && (pSpikeTrail || pSpikeTrail2))
	{
		bTrailSpike = false;

		std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter = collTrails.begin();
		while(iter != collTrails.end())
		{
			if(((*iter) == pSpikeTrail) || ((*iter) == pSpikeTrail2))
			{
				iter = collTrails.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		if(pSpikeTrail)
		{
			debug_delete(pSpikeTrail);
			pSpikeTrail = NULL;
		}

		if(pSpikeTrail2)
		{
			debug_delete(pSpikeTrail2);
			pSpikeTrail2 = NULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CLightCycleMgr
//
//	PURPOSE:	Constructor. RETAIL: 0x100fe5c0
//
// ----------------------------------------------------------------------- //
CLightCycleMgr::CLightCycleMgr()
{
	m_nTronUnknown1338 = 0;
	m_fTronUnknown20 = 500.0f;
	m_fGameSpeedScalar = 1.0f;
	m_fTrailLength = 30000.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::Init
//
//	PURPOSE:	Initialization. RETAIL: 0x100fe640
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::Init()
{
	m_bUpdating = true;

	if(!m_ButeMgr.Init())
		return false;

	RemoveAllCyclists();
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::Term
//
//	PURPOSE:	Cleanup. RETAIL: 0x100fe670
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
//	PURPOSE:	Creates a new cyclist (or resets the object's) and sets it
//				up from its cycle color. RETAIL: 0x100fe680
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST* CLightCycleMgr::AddCyclist(HOBJECT hObj, const char* szCycleColor, bool bUpdate,
	LTVector* pvDims, uint32 nTronUnknown4, uint32 nTronUnknown5)
{
	LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
	if(pCyclist)
	{
		// Reset him
		std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter;
		for(iter=pCyclist->collTrails.begin();iter!=pCyclist->collTrails.end();iter++)
		{
			debug_delete(*iter);
		}
		pCyclist->collTrails.erase(pCyclist->collTrails.begin(), pCyclist->collTrails.end());

		pCyclist->pCurTrail = NULL;
		pCyclist->pSpikeTrail = NULL;
		pCyclist->pSpikeTrail2 = NULL;
		pCyclist->nPowerup = LC_NO_POWERUP;
		pCyclist->nFlags = 0;
		pCyclist->bDerezRequested = false;
	}
	else
	{
		// Allocate
		pCyclist = debug_new(LIGHT_CYCLIST);

		// Add to our list
		m_collCyclists.push_back(pCyclist);
	}

	pCyclist->bUpdating = bUpdate;
	pCyclist->hObject = hObj;
	pCyclist->fTrailCheckTime = 0.0f;
	pCyclist->m_vTronUnknownE4.Init();
	pCyclist->fStartDerezTime = 0.0f;
	UpdateCyclistForward(pCyclist);
	pCyclist->m_bTronUnknown130 = false;

	LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetCycleColor(szCycleColor);
	LIGHTCYCLETYPE* pType = g_pLightCycleButeMgr->GetCycleType(pColor->nCycleType);

	int nMod = pColor->nCycleMod;
	if((GetGameType() == eGameTypeMultiplayerLightcycles) || (GetGameType() == eGameTypeSingle))
	{
		nMod = 5;
	}
	LIGHTCYCLEMOD* pMod = g_pLightCycleButeMgr->GetCycleMod(nMod);

	float fDifficulty = g_pLightCycleButeMgr->GetLCDifficultyModifier(GetDifficulty());

	pCyclist->fMaxTrailLength		= m_fTrailLength;
	pCyclist->nMaxWallDist			= pType->nMaxWallDist;
	pCyclist->fPlayerNoticeDist		= pType->fPlayerNoticeDist;
	pCyclist->fRightSensorDist		= pType->fRightSensorDist;
	pCyclist->fLeftSensorDist		= pType->fLeftSensorDist;
	pCyclist->fFrontSensorDist		= pType->fFrontSensorDist;
	pCyclist->fBaseSpeed			= pMod->fBaseSpeed * pType->fBaseSpeed * m_fGameSpeedScalar;
	pCyclist->fSpeed				= pMod->fBaseSpeed * pType->fBaseSpeed * m_fGameSpeedScalar;
	pCyclist->fMinSpeed				= pType->fMinSpeed * pMod->fMinSpeed * m_fGameSpeedScalar;
	pCyclist->fMaxManualSpeedAdd	= pMod->fMaxManualSpeedAdd * pType->fMaxManualSpeedAdd;
	pCyclist->fMaxExternalSpeedAdd	= pMod->fMaxExternalSpeedAdd * pType->fMaxExternalSpeedAdd;
	pCyclist->fMaxTurboSpeed		= pType->fMaxTurboSpeed * pMod->fMaxTurboSpeed * m_fGameSpeedScalar;
	pCyclist->fTurboAccelRate		= pType->fTurboAccelRate * pMod->fTurboAccelRate;
	pCyclist->fBrakeDecelRate		= pType->fBrakeDecelRate * pMod->fBrakeDecelRate;
	pCyclist->fNormalAccelRate		= pType->fNormalAccelRate * pMod->fNormalAccelRate;
	pCyclist->fNormalDecelRate		= pType->fNormalDecelRate * pMod->fNormalDecelRate;
	pCyclist->fMaxWallAccelRate		= pType->fMaxWallAccelRate * pMod->fMaxWallAccelRate;
	pCyclist->fTurnSpeedLossPercent	= pType->fTurnSpeedLossPercent * pMod->fTurnSpeedLossPercent;
	pCyclist->fMaxSpeed			= 0.0f;

	// The AI's speed depends on the difficulty
	if(!pvDims)
	{
		pCyclist->fBaseSpeed += fDifficulty * pCyclist->fMaxManualSpeedAdd;
	}

	pCyclist->nCycleType	= pColor->nCycleType;
	pCyclist->nWallDist		= pCyclist->nMaxWallDist + 100;
	pCyclist->nRed			= pColor->nRed;
	pCyclist->nGreen		= pColor->nGreen;
	pCyclist->nBlue			= pColor->nBlue;
	pCyclist->nCycleDialog	= pColor->nCycleDialog;
	strcpy(pCyclist->szCycleColor, szCycleColor);

	LTVector vDims;
	if(pvDims)
	{
		pCyclist->bFixedDims = true;
		vDims = *pvDims;
		pCyclist->fWidth = 4.0f;
		pCyclist->fHeight = vDims.y;
		pCyclist->fLength = 4.0f;
	}
	else
	{
		g_pPhysicsLT->GetObjectDims(hObj, &vDims);
		pCyclist->fWidth = vDims.x;
		pCyclist->fHeight = vDims.y;
		pCyclist->fLength = vDims.z;
	}

	UpdateCyclistDims(pCyclist, vDims, true);

	// Put him on the ground
	LTVector vPos;
	LCM_LT->GetObjectPos(pCyclist->hObject, &vPos);

	IntersectQuery iq;
	IntersectInfo ii;
	iq.m_From = vPos;
	iq.m_To = LTVector(vPos.x, vPos.y - 10000.0f, vPos.z);
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY | CHECK_FROM_POINT_INSIDE_OBJECTS;
	iq.m_FilterFn = LCMGroundFilterFn;
	iq.m_pUserData = (HOBJECT)pCyclist->hObject;

	if(LCM_LT->IntersectSegment(&iq, &ii))
	{
		float fHeight = vPos.y - ii.m_Point.y;
		if(fHeight > vDims.y)
		{
			vPos.y -= fHeight - (vDims.y + 0.1f);
			LCM_LT->SetObjectPos(pCyclist->hObject, &vPos, LTTRUE);
		}
	}

	return pCyclist;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::DerezCyclist
//
//	PURPOSE:	Flags a cyclist for a derez. RETAIL: 0x100fec30
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::DerezCyclist(HOBJECT hObj)
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		if((*iter)->hObject == hObj)
		{
			(*iter)->bDerezRequested = true;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::BeginLightCycleTrail
//
//	PURPOSE:	Starts a new light cycle trail at the grid-snapped position.
//				RETAIL: 0x100fec60
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLE_TRAIL* CLightCycleMgr::BeginLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos)
{
	if(!pCyclist)
		return NULL;

	// Create a new trail
	LIGHT_CYCLE_TRAIL* pTrail = debug_new(LIGHT_CYCLE_TRAIL);

	// Add this trail to the light cycle's list
	pCyclist->collTrails.push_back(pTrail);

	// Set our current trail
	pCyclist->pCurTrail = pTrail;

	// Snap to the grid
	int nX = LCM_ROUND(vPos.x);
	int nZ = LCM_ROUND(vPos.z);

	float fGridSize = m_ButeMgr.GetGridSize();
	if(fGridSize > 1.0f)
	{
		nX = LCM_ROUND(nX / fGridSize);
		vPos.x = nX * fGridSize;
		nZ = LCM_ROUND(nZ / fGridSize);
		vPos.z = nZ * fGridSize;
	}
	else
	{
		vPos.x = (float)nX;
		vPos.z = (float)nZ;
	}

	// Add the point
	return (AddLightCycleTrailPoint(pCyclist, pTrail, vPos) > 0) ? pTrail : NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::UpdateLightCycleTrail
//
//	PURPOSE:	Updates a light cyclist's trail. RETAIL: 0x100fedd0
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::UpdateLightCycleTrail(LIGHT_CYCLIST* pCyclist, bool bCheckCollision, LTVector* pvForward, bool bTronUnknown)
{
	if(!pCyclist)
		return false;

	LIGHT_CYCLE_TRAIL_POINT newPoint;
	CreateTrailPointFromVector(newPoint, pCyclist->vLastPos);

	LIGHT_CYCLE_TRAIL* pTrail = pCyclist->pCurTrail;
	if(!pTrail)
	{
		// Simple case: We're starting a new trail
		return (BeginLightCycleTrail(pCyclist, pCyclist->vLastPos) != NULL);
	}

	// Save the old forward vector
	LTVector vOldForward = pCyclist->vForward;
	if(pvForward)
	{
		pCyclist->vForward = *pvForward;

		LTVector vDims;
		UpdateCyclistDims(pCyclist, vDims, false);
	}

	int nPoints = pTrail->collTrailPoints.size();
	if(nPoints > 0)
	{
		// Get the last point
		LIGHT_CYCLE_TRAIL_POINT* pOldPoint = &(pTrail->collTrailPoints[nPoints-1]);
		LIGHT_CYCLE_TRAIL_POINT turnPoint = *pOldPoint;

		float fDist = (float)(fabs(pOldPoint->GetZ() - newPoint.GetZ()) + fabs(pOldPoint->GetX() - newPoint.GetX()));
		if(fDist < 1.0f)
			return true;

		if(bTronUnknown || !pCyclist->bFixedDims)
		{
			pTrail->fLength += fDist;
		}

		if(nPoints > 1)
		{
			// Check for a turn
			if(pvForward && (pvForward->x != vOldForward.x) && (pvForward->z != vOldForward.z))
			{
				// The turn point keeps the old travel's axis of the
				// new position
				if(vOldForward.x > 0.5f)
				{
					turnPoint.SetX(pCyclist->vLastPos.x);
				}
				else if(vOldForward.x < -0.5f)
				{
					turnPoint.SetX(pCyclist->vLastPos.x);
				}
				else if(vOldForward.z > 0.5f)
				{
					turnPoint.SetZ(pCyclist->vLastPos.z);
				}
				else
				{
					turnPoint.SetZ(pCyclist->vLastPos.z);
				}

				LightCycleCollisionInfo info;
				if(CheckForCollision(pCyclist, pOldPoint, &turnPoint.vPoint, &info, bCheckCollision, NULL) && bCheckCollision)
				{
					if((info.nTrail != -1) && pCyclist->bShield)
					{
						HandleShieldCollision(pCyclist, &info, true, false);
						DeactivateShield(pCyclist, true);

						pTrail = pCyclist->pCurTrail;
						pOldPoint = &(pTrail->collTrailPoints[pTrail->collTrailPoints.size()-1]);
					}
					else
					{
						HandleCollision(pCyclist, info);
						return true;
					}
				}

				// Change the last point to the turn point and start the
				// new leg there
				*pOldPoint = turnPoint;
				pTrail->collTrailPoints.push_back(turnPoint);
				pOldPoint = &(pTrail->collTrailPoints[pTrail->collTrailPoints.size()-1]);
			}

			// Move the last point to our location
			LightCycleCollisionInfo info;
			if(CheckForCollision(pCyclist, pOldPoint, &newPoint.vPoint, &info, bCheckCollision, NULL) && bCheckCollision)
			{
				if((info.nTrail != -1) && pCyclist->bShield)
				{
					HandleShieldCollision(pCyclist, &info, true, false);
					DeactivateShield(pCyclist, true);

					pTrail = pCyclist->pCurTrail;
					pOldPoint = &(pTrail->collTrailPoints[pTrail->collTrailPoints.size()-1]);
				}
				else
				{
					HandleCollision(pCyclist, info);
					return true;
				}
			}

			*pOldPoint = newPoint;
		}
		else
		{
			// We only have one point on our list (the starting point)
			// so add a new one
			LightCycleCollisionInfo info;
			if(CheckForCollision(pCyclist, pOldPoint, &newPoint.vPoint, &info, bCheckCollision, NULL) && bCheckCollision)
			{
				if((info.nTrail != -1) && pCyclist->bShield)
				{
					HandleShieldCollision(pCyclist, &info, true, false);
					DeactivateShield(pCyclist, true);
					pTrail = pCyclist->pCurTrail;
				}
				else
				{
					HandleCollision(pCyclist, info);
					return true;
				}
			}

			pTrail->collTrailPoints.push_back(newPoint);
		}

		if(pCyclist->bFixedDims && !bTronUnknown)
		{
			if(LCM_LT->GetTime() == pCyclist->fTrailCheckTime)
			{
				TrimTrailLength(pCyclist, pTrail, true);
			}
		}
		else
		{
			TrimTrailLength(pCyclist, pTrail, false);
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::AddLightCycleTrailPoint
//
//	PURPOSE:	Adds a point onto a light cycle trail. RETAIL: 0x100ff1f0
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
//	ROUTINE:	CLightCycleMgr::FindCyclist
//
//	PURPOSE:	Looks for a cyclist in the list by object. RETAIL: 0x100ff280
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
//	ROUTINE:	CLightCycleMgr::GetNextCyclist
//
//	PURPOSE:	The next living cyclist after the object's. RETAIL: 0x100ff2b0
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST* CLightCycleMgr::GetNextCyclist(HOBJECT hObj, bool bWrap)
{
	bool bFound = (hObj == NULL);

	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		if(bFound)
		{
			if((*iter)->fStartDerezTime == 0.0f)
				return (*iter);
		}
		else if((*iter)->hObject == hObj)
		{
			bFound = true;
		}
	}

	if(bWrap && m_collCyclists.size() && bFound)
	{
		LIGHT_CYCLIST* pCyclist = *m_collCyclists.begin();
		if((pCyclist->fStartDerezTime == 0.0f) && (hObj != pCyclist->hObject))
			return pCyclist;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CreateTrailPointFromVector
//
//	PURPOSE:	Creates a trail point from a vector and will do
//				grid-snapping or alignment if we ever need it.
//				RETAIL: 0x100ff340
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::CreateTrailPointFromVector(LIGHT_CYCLE_TRAIL_POINT &pt, LTVector &vPos)
{
	// For now, it's just a straight copy
	pt.vPoint = vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::TrimTrailLength
//
//	PURPOSE:	Cuts the trail's tail to the cyclist's max trail length, a
//				grid multiple at a time. RETAIL: 0x100ffc00
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::TrimTrailLength(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL* pTrail, bool bToTrailEnd)
{
	if(!(pCyclist->fMaxTrailLength > 0.0f))
		return;

	if(!(pTrail->fLength > pCyclist->fMaxTrailLength))
		return;

	float fExcess = pTrail->fLength - pCyclist->fMaxTrailLength;
	float fGridSize = m_ButeMgr.GetGridSize();
	if(fExcess < fGridSize)
		return;

	if(fGridSize > 1.0f)
	{
		fExcess = (float)LCM_ROUND(fExcess / fGridSize) * fGridSize;
	}
	else
	{
		fExcess = (float)LCM_ROUND(fExcess);
	}

	pTrail->m_fTronUnknown14 -= fExcess;

	int nRemove = 0;
	bool bDone = false;
	int nLast = pTrail->collTrailPoints.size() - 1;

	for(int i = 0; !bDone && (i < nLast); i++)
	{
		LIGHT_CYCLE_TRAIL_POINT* pPoints = pTrail->collTrailPoints.begin();
		LIGHT_CYCLE_TRAIL_POINT* pCur = &pPoints[i];
		LIGHT_CYCLE_TRAIL_POINT* pNext;
		LIGHT_CYCLE_TRAIL_POINT endPoint;

		if(bToTrailEnd && (i + 1 == nLast))
		{
			CreateTrailPointFromVector(endPoint, pCyclist->m_vTronUnknownE4);
			pNext = &endPoint;
		}
		else
		{
			pNext = &pPoints[i + 1];
		}

		float fDX = (float)fabs(pCur->vPoint.x - pNext->vPoint.x);
		float fDZ = (float)fabs(pCur->vPoint.z - pNext->vPoint.z);
		float fSegment = (fDX > fDZ) ? fDX : fDZ;

		if(fSegment < fExcess)
		{
			// The whole segment goes
			fExcess -= fSegment;
			nRemove++;
			pTrail->fLength -= fSegment;
		}
		else if(fDX > fDZ)
		{
			if(pNext->vPoint.x < pCur->vPoint.x)
			{
				pCur->vPoint.x -= fExcess;
			}
			else
			{
				pCur->vPoint.x += fExcess;
			}

			bDone = true;
			pTrail->fLength -= fExcess;
		}
		else
		{
			if(pNext->vPoint.z < pCur->vPoint.z)
			{
				pCur->vPoint.z -= fExcess;
			}
			else
			{
				pCur->vPoint.z += fExcess;
			}

			bDone = true;
			pTrail->fLength -= fExcess;
		}

		if(pTrail->m_fTronUnknown14 < 0.0f)
		{
			pTrail->m_fTronUnknown14 += m_fTronUnknown20 + 8.0f;
		}
	}

	if(nRemove > 0)
	{
		pTrail->collTrailPoints.erase(pTrail->collTrailPoints.begin(), pTrail->collTrailPoints.begin() + nRemove);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::SnapToGrid
//
//	PURPOSE:	Snaps the position to the bute grid. RETAIL: 0x100ffec0
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::SnapToGrid(LTVector & vPos)
{
	float fGridSize = m_ButeMgr.GetGridSize();

	int nX = LCM_ROUND(vPos.x);
	if(fGridSize > 1.0f)
	{
		nX = LCM_ROUND(nX / fGridSize);
		vPos.x = nX * fGridSize;
	}
	else
	{
		vPos.x = (float)nX;
	}

	int nZ = LCM_ROUND(vPos.z);
	if(fGridSize > 1.0f)
	{
		nZ = LCM_ROUND(nZ / fGridSize);
		vPos.z = nZ * fGridSize;
	}
	else
	{
		vPos.z = (float)nZ;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CheckForCollision
//
//	PURPOSE:	Checks the move from the trail's last point to vPos against
//				the other cycles and all the trails (on the grid's integer
//				coordinates); also finds the nearest parallel wall.
//				RETAIL: 0x100ff360
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::CheckForCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL_POINT* pLast, LTVector* pvPos,
	LightCycleCollisionInfo* pInfo, bool bCheckCollision, LIGHT_CYCLIST* pOwner)
{
	if(!pCyclist)
		return false;

	pCyclist->nWallDist = pCyclist->nMaxWallDist + 100;

	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		LIGHT_CYCLIST* pOther = (*iter);

		// The cycles
		if(bCheckCollision && (pOther->fStartDerezTime == 0.0f) &&
			((!pCyclist->m_bTronUnknownD1 && (pCyclist != pOther)) || (pCyclist->m_bTronUnknownD1 && (pOwner != pOther))))
		{
			float fMinX = (pvPos->x < pLast->vPoint.x) ? pvPos->x : pLast->vPoint.x;
			float fMaxX = (pvPos->x > pLast->vPoint.x) ? pvPos->x : pLast->vPoint.x;
			float fMinZ = (pvPos->z < pLast->vPoint.z) ? pvPos->z : pLast->vPoint.z;
			float fMaxZ = (pvPos->z > pLast->vPoint.z) ? pvPos->z : pLast->vPoint.z;

			if(pCyclist->vForward.x == 0.0f)
			{
				fMinX -= 16.0f;
				fMaxX += 16.0f;
				fMinZ -= pCyclist->fLength;
				fMaxZ += pCyclist->fLength;
			}
			else
			{
				fMinZ -= 16.0f;
				fMaxZ += 16.0f;
				fMinX -= pCyclist->fLength;
				fMaxX += pCyclist->fLength;
			}

			float fOtherMinX, fOtherMaxX, fOtherMinZ, fOtherMaxZ;
			if(pOther->vForward.x == 0.0f)
			{
				fOtherMinX = pOther->vLastPos.x - 16.0f;
				fOtherMaxX = pOther->vLastPos.x + 16.0f;
				fOtherMinZ = pOther->vLastPos.z - pOther->fLength;
				fOtherMaxZ = pOther->vLastPos.z + pOther->fLength;
			}
			else
			{
				fOtherMinX = pOther->vLastPos.x - pOther->fLength;
				fOtherMaxX = pOther->vLastPos.x + pOther->fLength;
				fOtherMinZ = pOther->vLastPos.z - 16.0f;
				fOtherMaxZ = pOther->vLastPos.z + 16.0f;
			}

			if(!(fOtherMaxX < fMinX) && !(fOtherMinX > fMaxX) && !(fOtherMaxZ < fMinZ) && !(fOtherMinZ > fMaxZ))
			{
				pInfo->pCyclist = pOther;
				pInfo->nTrail = -1;

				if(pCyclist->m_bTronUnknownD1)
					return true;

				if(pCyclist->bShield && !pOther->bShield)
				{
					// The shield wins
					CyclistCrash(pOther, pCyclist, true);
				}
				else
				{
					if(!pCyclist->bShield && pOther->bShield)
						return true;

					bool bAlongX = (pCyclist->vForward.x != 0.0f);
					bool bOtherAlongX = (pOther->vForward.x != 0.0f);
					if(bAlongX != bOtherAlongX)
						return true;

					if(bAlongX)
					{
						if(pCyclist->vForward.x == pOther->vForward.x)
							return true;
					}
					else
					{
						if(pCyclist->vForward.z == pOther->vForward.z)
							return true;
					}

					HeadOnCollision(pCyclist, pOther);
					return true;
				}
			}
		}

		// The trails
		std::vector<LIGHT_CYCLE_TRAIL*>::iterator iterTrail;
		for(iterTrail=pOther->collTrails.begin();iterTrail!=pOther->collTrails.end();iterTrail++)
		{
			LIGHT_CYCLE_TRAIL* pTrail = (*iterTrail);

			// Skip the segments next to the owner's own cycle
			int nLastSegment = -1;
			LIGHT_CYCLIST* pTrailOwner = pOwner ? pOwner : pCyclist;
			if((pOther == pTrailOwner) && (pTrail == pTrailOwner->pCurTrail))
			{
				uint32 nPoints = pTrail->collTrailPoints.size();
				if(nPoints >= 4)
					nLastSegment = nPoints - 4;
			}
			else
			{
				nLastSegment = pTrail->collTrailPoints.size() - 2;
			}

			int nToX = (int)pvPos->x;
			int nToZ = (int)pvPos->z;
			int nFromX = (int)pLast->vPoint.x;
			int nFromZ = (int)pLast->vPoint.z;

			for(int i = 0; i <= nLastSegment; i++)
			{
				LIGHT_CYCLE_TRAIL_POINT* pPoints = pTrail->collTrailPoints.begin();
				int nSX = (int)pPoints[i].vPoint.x;
				int nSZ = (int)pPoints[i].vPoint.z;
				int nEX = (int)pPoints[i+1].vPoint.x;
				int nEZ = (int)pPoints[i+1].vPoint.z;

				if(nSX == nEX)
				{
					// The segment runs along z
					if(nFromX == nToX)
					{
						// ...and so do we
						int nWidth = (int)pCyclist->fWidth;
						int nLo = nToX - nWidth;
						int nHi = nToX + nWidth;
						if((nLo <= nSX) && (nHi >= nSX))
						{
							int nMin = (nFromZ < nToZ) ? nFromZ : nToZ;
							int nMax = (nFromZ > nToZ) ? nFromZ : nToZ;
							int nSMin = (nSZ < nEZ) ? nSZ : nEZ;
							int nSMax = (nSZ > nEZ) ? nSZ : nEZ;
							if((nMin <= nSMax) && (nMax >= nSMin))
							{
								pInfo->pCyclist = pOther;
								pInfo->nTrail = iterTrail - pOther->collTrails.begin();
								pInfo->nSegment = i;
								pInfo->vCollisionPoint.x = pvPos->x;
								pInfo->vCollisionPoint.y = pvPos->y;
								if(nToZ > nFromZ)
									pInfo->vCollisionPoint.z = (float)((nFromZ > nSMin) ? nFromZ : nSMin);
								else
									pInfo->vCollisionPoint.z = (float)((nFromZ < nSMax) ? nFromZ : nSMax);
								return true;
							}
						}
						else
						{
							// A parallel wall: how near?
							int nLoDist = abs(nLo - nSX);
							int nHiDist = abs(nHi - nSX);
							int nDist = (nLoDist < nHiDist) ? nLoDist : nHiDist;
							if(nDist < pCyclist->nWallDist)
							{
								int nMin = (nFromZ < nToZ) ? nFromZ : nToZ;
								int nMax = (nFromZ > nToZ) ? nFromZ : nToZ;
								int nSMin = (nSZ < nEZ) ? nSZ : nEZ;
								int nSMax = (nSZ > nEZ) ? nSZ : nEZ;
								if((nMin < nSMax) && (nMax > nSMin))
								{
									pCyclist->nWallDist = nDist;
									if(nLoDist < nHiDist)
										pCyclist->m_bTronUnknownAC = (nToZ > nFromZ);
									else
										pCyclist->m_bTronUnknownAC = (nToZ < nFromZ);
								}
							}
						}
					}
					else
					{
						// We cross it along x
						int nWidth = (int)pCyclist->fWidth;
						int nLo = nToZ - nWidth;
						int nHi = nToZ + nWidth;
						int nSMin = (nSZ < nEZ) ? nSZ : nEZ;
						int nSMax = (nSZ > nEZ) ? nSZ : nEZ;
						if((nLo <= nSMax) && (nHi >= nSMin))
						{
							int nMin = (nFromX < nToX) ? nFromX : nToX;
							int nMax = (nFromX > nToX) ? nFromX : nToX;
							if((nMin <= nSX) && (nMax >= nSX))
							{
								pInfo->pCyclist = pOther;
								pInfo->nTrail = iterTrail - pOther->collTrails.begin();
								pInfo->nSegment = i;
								pInfo->vCollisionPoint.x = (float)nSX;
								pInfo->vCollisionPoint.y = pvPos->y;
								pInfo->vCollisionPoint.z = pvPos->z;
								return true;
							}
						}
					}
				}
				else
				{
					// The segment runs along x
					if(nFromZ == nToZ)
					{
						// ...and so do we
						int nWidth = (int)pCyclist->fWidth;
						int nLo = nToZ - nWidth;
						int nHi = nToZ + nWidth;
						if((nLo <= nSZ) && (nHi >= nSZ))
						{
							int nMin = (nFromX < nToX) ? nFromX : nToX;
							int nMax = (nFromX > nToX) ? nFromX : nToX;
							int nSMin = (nSX < nEX) ? nSX : nEX;
							int nSMax = (nSX > nEX) ? nSX : nEX;
							if((nMin <= nSMax) && (nMax >= nSMin))
							{
								pInfo->pCyclist = pOther;
								pInfo->nTrail = iterTrail - pOther->collTrails.begin();
								pInfo->nSegment = i;
								if(nToX > nFromX)
									pInfo->vCollisionPoint.x = (float)((nFromX > nSMin) ? nFromX : nSMin);
								else
									pInfo->vCollisionPoint.x = (float)((nFromX < nSMax) ? nFromX : nSMax);
								pInfo->vCollisionPoint.y = pvPos->y;
								pInfo->vCollisionPoint.z = pvPos->z;
								return true;
							}
						}
						else
						{
							// A parallel wall: how near?
							int nLoDist = abs(nLo - nSZ);
							int nHiDist = abs(nHi - nSZ);
							int nDist = (nLoDist < nHiDist) ? nLoDist : nHiDist;
							if(nDist < pCyclist->nWallDist)
							{
								int nMin = (nFromX < nToX) ? nFromX : nToX;
								int nMax = (nFromX > nToX) ? nFromX : nToX;
								int nSMin = (nSX < nEX) ? nSX : nEX;
								int nSMax = (nSX > nEX) ? nSX : nEX;
								if((nMin < nSMax) && (nMax > nSMin))
								{
									pCyclist->nWallDist = nDist;
									if(nLoDist < nHiDist)
										pCyclist->m_bTronUnknownAC = (nToX > nFromX);
									else
										pCyclist->m_bTronUnknownAC = (nToX < nFromX);
								}
							}
						}
					}
					else
					{
						// We cross it along z
						int nWidth = (int)pCyclist->fWidth;
						int nLo = nToX - nWidth;
						int nHi = nToX + nWidth;
						int nSMin = (nSX < nEX) ? nSX : nEX;
						int nSMax = (nSX > nEX) ? nSX : nEX;
						if((nLo <= nSMax) && (nHi >= nSMin))
						{
							int nMin = (nFromZ < nToZ) ? nFromZ : nToZ;
							int nMax = (nFromZ > nToZ) ? nFromZ : nToZ;
							if((nMin <= nSZ) && (nMax >= nSZ))
							{
								pInfo->pCyclist = pOther;
								pInfo->nTrail = iterTrail - pOther->collTrails.begin();
								pInfo->nSegment = i;
								pInfo->vCollisionPoint.x = pvPos->x;
								pInfo->vCollisionPoint.y = pvPos->y;
								pInfo->vCollisionPoint.z = (float)nSZ;
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
//	ROUTINE:	CLightCycleMgr::HandleCollision
//
//	PURPOSE:	Handles a collision. RETAIL: 0x100ffbe0
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::HandleCollision(LIGHT_CYCLIST *pCyclist, LightCycleCollisionInfo &info)
{
	CyclistCrash(pCyclist, info.pCyclist, true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::UpdateCyclistMovement
//
//	PURPOSE:	Turns and accelerates the object's cycle from the LCMF_
//				flags and sets its velocity. RETAIL: 0x100fffd0
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::UpdateCyclistMovement(HOBJECT hObj, uint32 nFlags)
{
	LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
	if(!pCyclist)
		return false;

	bool bTurned = false;
	LTVector vForward = pCyclist->vForward;
	float fSpeed = pCyclist->fSpeed;
	LTVector vZero(0.0f, 0.0f, 0.0f);
	LTVector vVel(0.0f, 0.0f, 0.0f);
	float fAccel = 0.0f;

	if(nFlags & (LCMF_TURNLEFT | LCMF_TURNRIGHT))
	{
		if(nFlags & LCMF_TURNLEFT)
		{
			if(vForward.x > 0.5f)
			{
				vForward.z = 1.0f;
				vForward.x = 0.0f;
			}
			else if(vForward.x < -0.5f)
			{
				vForward.z = -1.0f;
				vForward.x = 0.0f;
			}
			else
			{
				vForward.x = (vForward.z > 0.5f) ? -1.0f : 1.0f;
				vForward.z = 0.0f;
			}
		}
		else
		{
			if(vForward.x > 0.5f)
			{
				vForward.x = 0.0f;
				vForward.z = -1.0f;
			}
			else if(vForward.x < -0.5f)
			{
				vForward.x = 0.0f;
				vForward.z = 1.0f;
			}
			else
			{
				vForward.x = (vForward.z > 0.5f) ? 1.0f : -1.0f;
				vForward.z = 0.0f;
			}
		}

		// Turning costs speed
		if(!((uint8)(pCyclist->nFlags >> 5) & 1) && !(nFlags & (LCMF_SPEEDZONE | LCMF_SLOWZONE)) &&
			(fSpeed > pCyclist->fMinSpeed))
		{
			fSpeed -= (fSpeed - pCyclist->fMinSpeed) * pCyclist->fTurnSpeedLossPercent;
		}

		bTurned = true;
	}

	float fWallAccel = GetWallAccelRate(pCyclist);

	if(nFlags & LCMF_ACCELERATE)
	{
		fAccel = pCyclist->fTurboAccelRate;
	}
	else if(nFlags & LCMF_BRAKE)
	{
		fAccel = -pCyclist->fBrakeDecelRate;
	}

	float fRate;
	if(nFlags & LCMF_SPEEDZONE)
	{
		fRate = m_ButeMgr.GetSpeedZoneAccelRate();
	}
	else if(nFlags & LCMF_SLOWZONE)
	{
		fRate = -m_ButeMgr.GetSlowZoneDecelRate();
	}
	else if((uint8)(pCyclist->nFlags >> 5) & 1)
	{
		fRate = m_ButeMgr.GetTurboCurseAccelRate();
	}
	else if((uint8)(pCyclist->nFlags >> 1) & 1)
	{
		fRate = pCyclist->fTurboAccelRate;
	}
	else
	{
		fRate = fWallAccel + fAccel;
	}

	float fNewSpeed;
	float fMaxSpeed;
	if(fRate == 0.0f)
	{
		// Back to the base speed
		fNewSpeed = fSpeed;
		if(fSpeed > pCyclist->fBaseSpeed)
		{
			fRate -= pCyclist->fNormalDecelRate;
			fNewSpeed = LCM_LT->GetFrameTime() * fRate + fSpeed;
			if(fNewSpeed < pCyclist->fBaseSpeed)
				fNewSpeed = pCyclist->fBaseSpeed;
		}
		else if(fSpeed < pCyclist->fBaseSpeed)
		{
			fRate += pCyclist->fNormalAccelRate;
			fNewSpeed = LCM_LT->GetFrameTime() * fRate + fSpeed;
			if(fNewSpeed > pCyclist->fBaseSpeed)
				fNewSpeed = pCyclist->fBaseSpeed;
		}

		fMaxSpeed = pCyclist->fMaxSpeed;
	}
	else
	{
		float fMinSpeed = pCyclist->fMinSpeed;
		if(nFlags & LCMF_SPEEDZONE)
		{
			fMaxSpeed = m_ButeMgr.GetMaxSpeedZoneSpeed() * m_fGameSpeedScalar;
		}
		else if(nFlags & LCMF_SLOWZONE)
		{
			fMinSpeed = m_ButeMgr.GetMinSlowZoneSpeed() * m_fGameSpeedScalar;
			fMaxSpeed = m_ButeMgr.GetMaxTurboCurseSpeed();
		}
		else if((uint8)(pCyclist->nFlags >> 5) & 1)
		{
			fMaxSpeed = m_ButeMgr.GetMaxTurboCurseSpeed();
		}
		else if((uint8)(pCyclist->nFlags >> 1) & 1)
		{
			fMaxSpeed = pCyclist->fMaxTurboSpeed;
		}
		else
		{
			fMaxSpeed = pCyclist->fBaseSpeed;
			if(fAccel > 0.0f)
				fMaxSpeed += pCyclist->fMaxManualSpeedAdd;
			if(fWallAccel > 0.0f)
				fMaxSpeed += pCyclist->fMaxExternalSpeedAdd;
		}

		// Never as fast as a missile
		if(!(m_ButeMgr.GetMissileSpeed() > fMaxSpeed))
		{
			fMaxSpeed = m_ButeMgr.GetMissileSpeed() - 50.0f;
		}

		if(fSpeed > fMaxSpeed)
		{
			fRate = -((nFlags & LCMF_BRAKE) ? pCyclist->fBrakeDecelRate : pCyclist->fNormalDecelRate);
			fNewSpeed = LCM_LT->GetFrameTime() * fRate + fSpeed;
		}
		else
		{
			fNewSpeed = LCM_LT->GetFrameTime() * fRate + fSpeed;
			if(fNewSpeed > fMaxSpeed)
				fNewSpeed = fMaxSpeed;
		}

		if(fNewSpeed < fMinSpeed)
			fNewSpeed = fMinSpeed;
	}

	pCyclist->fSpeed = fNewSpeed;
	vVel = vForward * fNewSpeed;

	float fOldRate = pCyclist->fAccelRate;
	pCyclist->fAccelRate = fRate;

	g_pPhysicsLT->SetAcceleration(hObj, &vZero);
	g_pPhysicsLT->SetVelocity(hObj, &vVel);

	if((fOldRate != fRate) || (fMaxSpeed != pCyclist->fMaxSpeed) || bTurned)
	{
		CyclistMovementChanged(pCyclist, bTurned, fAccel, fWallAccel, nFlags);
	}

	pCyclist->fMaxSpeed = fMaxSpeed;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::GetWallAccelRate
//
//	PURPOSE:	The extra acceleration of riding close to a wall.
//				RETAIL: 0x101004d0
//
// ----------------------------------------------------------------------- //
float CLightCycleMgr::GetWallAccelRate(LIGHT_CYCLIST* pCyclist)
{
	float fRate = 0.0f;

	int nDist = pCyclist->nWallDist;
	int nMaxDist = pCyclist->nMaxWallDist;
	if(nDist < nMaxDist)
	{
		fRate = (1.0f - ((float)nDist - 1.0f) / ((float)nMaxDist - 1.0f)) * pCyclist->fMaxWallAccelRate;
	}

	return fRate;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::UpdateCyclistDims
//
//	PURPOSE:	Sets the cycle's dims along its forward. RETAIL: 0x10100520
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::UpdateCyclistDims(LIGHT_CYCLIST* pCyclist, LTVector & vDims, bool bTronUnknown)
{
	vDims.y = pCyclist->fHeight;
	if(pCyclist->vForward.x == 0.0f)
	{
		vDims.z = pCyclist->fLength;
		vDims.x = pCyclist->fWidth;
	}
	else
	{
		vDims.x = pCyclist->fLength;
		vDims.z = pCyclist->fWidth;
	}

	g_pPhysicsLT->SetObjectDims(pCyclist->hObject, &vDims, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::RemoveAllCyclists
//
//	PURPOSE:	Frees all the cyclists. RETAIL: 0x10100580
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::RemoveAllCyclists()
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		debug_delete(*iter);
		(*iter) = NULL;
	}

	m_collCyclists.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::UpdateDerez
//
//	PURPOSE:	Frees the cyclists flagged for a derez and the trails whose
//				derez is over. RETAIL: 0x101005e0
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::UpdateDerez()
{
	std::vector<LIGHT_CYCLIST*>::iterator iter = m_collCyclists.begin();
	while(iter != m_collCyclists.end())
	{
		LIGHT_CYCLIST* pCyclist = (*iter);
		if(pCyclist->bDerezRequested)
		{
			debug_delete(pCyclist);
			m_collCyclists.erase(iter);
			continue;
		}

		std::vector<LIGHT_CYCLE_TRAIL*>::iterator iterTrail = pCyclist->collTrails.begin();
		while(iterTrail != pCyclist->collTrails.end())
		{
			// A dead cyclist's trails go at once
			float fDelay = (pCyclist->fStartDerezTime > 0.0f) ? 0.0f : m_ButeMgr.GetTrailDerezDelay();

			LIGHT_CYCLE_TRAIL* pTrail = (*iterTrail);
			if(pTrail->fDerezStartTime > 0.0f)
			{
				float fDerezTime = m_ButeMgr.GetTrailDerezTime();
				if((LCM_LT->GetTime() - pTrail->fDerezStartTime - fDelay) > fDerezTime)
				{
					if(pTrail == pCyclist->pSpikeTrail)
						pCyclist->pSpikeTrail = NULL;

					if(pTrail == pCyclist->pSpikeTrail2)
						pCyclist->pSpikeTrail2 = NULL;

					debug_delete(pTrail);
					pCyclist->collTrails.erase(iterTrail);
					continue;
				}
			}

			iterTrail++;
		}

		iter++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::SetCyclistPowerup
//
//	PURPOSE:	Gives a cyclist a power-up. RETAIL: 0x10100770
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::SetCyclistPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup)
{
	if(pCyclist && ((nPowerup < kLCPowerup_ForceUse + 1) || (nPowerup == LC_NO_POWERUP)))
	{
		pCyclist->nPowerup = nPowerup;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::ActivatePowerup
//
//	PURPOSE:	Uses a cyclist's power-up. RETAIL: 0x101007a0
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::ActivatePowerup(LIGHT_CYCLIST* pCyclist, LTVector* pvPos)
{
	if(!pCyclist)
		return false;

	switch(pCyclist->nPowerup)
	{
		case kLCPowerup_Shield:
		{
			pCyclist->bShield = true;
		}
		break;

		case kLCPowerup_Turbo:
		{
			pCyclist->bTurboCursed = false;
			pCyclist->bTurbo = true;
			pCyclist->fPowerupStartTime = LCM_LT->TronUnknownVirtual44();
			// RETAIL: bute +0x1144 (our GetTurboCurseTime: the names of
			// +0x1144/+0x1148 may be swapped)
			pCyclist->fPowerupDuration = m_ButeMgr.GetTurboCurseTime();
		}
		break;

		case kLCPowerup_TrailSpike:
		{
			pCyclist->ClearTrailSpike();
			pCyclist->bTrailSpike = true;
			BeginTrailSpike(pCyclist, pvPos);
		}
		break;

		case kLCPowerup_Missile:
		break;

		case kLCPowerup_WallExtender:
		{
			pCyclist->fMaxTrailLength += m_ButeMgr.GetWallExtenderAdd();
		}
		break;

		case kLCPowerup_TurboCurse:
		{
			if((uint8)(pCyclist->nFlags >> 1) & 1)
			{
				pCyclist->bTurbo = false;
			}

			LIGHT_CYCLIST* pFirst = GetNextCyclist(NULL, true);
			LIGHT_CYCLIST* pOther = pFirst;
			while(pOther)
			{
				pOther->bTurboCursed = true;
				pOther->fPowerupStartTime = LCM_LT->TronUnknownVirtual44();
				pOther->fPowerupDuration = m_ButeMgr.GetTurboTime();

				pOther = GetNextCyclist(pOther->hObject, true);
				if(pOther == pFirst)
					break;
			}

			pCyclist->bTurboCurse = true;
		}
		break;

		case kLCPowerup_Grab:
		case kLCPowerup_TrailReset:
		case kLCPowerup_ForceUse:
		break;

		case kLCPowerup_Random:
			return false;

		case kNumLCPowerupTypes:
		break;

		default:
			return false;
	}

	pCyclist->nPowerup = LC_NO_POWERUP;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::DeactivateShield
//
//	PURPOSE:	Drops a cyclist's shield. RETAIL: 0x10100950
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::DeactivateShield(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown)
{
	pCyclist->bShield = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::HandleShieldCollision
//
//	PURPOSE:	A shielded cycle hit a trail: the trail is cut in two at the
//				hit segment (the new trail gets the points after it).
//				RETAIL: 0x10100960
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::HandleShieldCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo* pInfo, bool bNotify, bool bCurrentTrail)
{
	LIGHT_CYCLIST* pOther = pInfo->pCyclist;
	if(pInfo->nTrail == -1)
		return true;

	if(pOther)
	{
		LIGHT_CYCLE_TRAIL* pTrail = NULL;
		if(bCurrentTrail)
		{
			pTrail = pOther->pCurTrail;
		}
		else if((int)pOther->collTrails.size() > pInfo->nTrail)
		{
			pTrail = pOther->collTrails[pInfo->nTrail];
		}

		if(pTrail && ((int)pTrail->collTrailPoints.size() > pInfo->nSegment))
		{
			LIGHT_CYCLE_TRAIL* pNewTrail = debug_new(LIGHT_CYCLE_TRAIL);

			if(pTrail->fDerezStartTime == 0.0f)
			{
				pTrail->fDerezStartTime = LCM_LT->GetTime();
			}
			else
			{
				pNewTrail->fDerezStartTime = pTrail->fDerezStartTime;
			}

			if(pTrail->m_bTronUnknownC)
			{
				pNewTrail->m_bTronUnknownC = true;
				pTrail->fDerezStartTime = LCM_LT->GetTime();
				pNewTrail->fDerezStartTime = LCM_LT->GetTime();
			}

			if(pTrail == pOther->pCurTrail)
			{
				pOther->pCurTrail = pNewTrail;
			}
			else if(pNewTrail->fDerezStartTime == 0.0f)
			{
				pNewTrail->fDerezStartTime = LCM_LT->GetTime();
			}

			int nPoints = pTrail->collTrailPoints.size();
			if(pInfo->nSegment + 1 >= nPoints)
			{
				pInfo->nSegment = nPoints - 2;
			}

			// The new trail: from the hit segment on
			LIGHT_CYCLE_TRAIL_POINT* pSegment = &pTrail->collTrailPoints[pInfo->nSegment];
			pNewTrail->collTrailPoints.insert(pNewTrail->collTrailPoints.begin(), pSegment, pTrail->collTrailPoints.end());

			bool bKeep = true;
			if(!CutTrailPoint(pNewTrail->collTrailPoints, pNewTrail->collTrailPoints.begin(), pNewTrail->collTrailPoints.begin() + 1, pInfo))
			{
				if(pNewTrail->collTrailPoints.size() > 2)
				{
					pNewTrail->collTrailPoints.erase(pNewTrail->collTrailPoints.begin());
				}
				else if(pOther->pCurTrail == pNewTrail)
				{
					*pNewTrail->collTrailPoints.begin() = *(pNewTrail->collTrailPoints.begin() + 1);
				}
				else
				{
					pNewTrail->collTrailPoints.erase(pNewTrail->collTrailPoints.begin(), pNewTrail->collTrailPoints.end());
					debug_delete(pNewTrail);
					bKeep = false;
				}
			}

			if(bKeep)
			{
				pOther->collTrails.push_back(pNewTrail);

				if((pOther->pCurTrail == pNewTrail) && (pNewTrail->collTrailPoints.size() > 1))
				{
					pNewTrail->fLength = 0.0f;
					for(uint32 i = 0; i < pNewTrail->collTrailPoints.size() - 1; i++)
					{
						LIGHT_CYCLE_TRAIL_POINT* pPoints = pNewTrail->collTrailPoints.begin();
						float fLength = (float)fabs(pPoints[i].vPoint.x - pPoints[i+1].vPoint.x);
						if(!(fLength > 0.0001f))
						{
							fLength = (float)fabs(pPoints[i].vPoint.z - pPoints[i+1].vPoint.z);
						}
						pNewTrail->fLength += fLength;
					}
				}
			}

			// The old trail: up to the hit segment
			if((int)pTrail->collTrailPoints.size() > pInfo->nSegment + 2)
			{
				pTrail->collTrailPoints.erase(pSegment + 2, pTrail->collTrailPoints.end());
			}

			nPoints = pTrail->collTrailPoints.size();
			if(!CutTrailPoint(pTrail->collTrailPoints, &pTrail->collTrailPoints[nPoints-1], &pTrail->collTrailPoints[nPoints-2], pInfo))
			{
				if(pTrail->collTrailPoints.size() > 2)
				{
					// RETAIL: erases end() (docs/RETAIL_BUGS.md): with STLport
					// that drops the last point
					pTrail->collTrailPoints.erase(pTrail->collTrailPoints.end());
				}
				else
				{
					pTrail->collTrailPoints.erase(pTrail->collTrailPoints.begin(), pTrail->collTrailPoints.end());

					if(pOther->pCurTrail != pTrail)
					{
						std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter = pOther->collTrails.begin();
						while(iter != pOther->collTrails.end())
						{
							if((*iter) == pTrail)
							{
								pOther->collTrails.erase(iter);
							}
							else
							{
								iter++;
							}
						}

						if(pOther->pSpikeTrail == pTrail)
						{
							pOther->pSpikeTrail = NULL;
						}
						else if(pOther->pSpikeTrail2 == pTrail)
						{
							pOther->pSpikeTrail2 = NULL;
						}

						debug_delete(pTrail);
					}
				}
			}
		}
	}

	if(bNotify)
	{
		TrailCut(pInfo);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::TrimTrail
//
//	PURPOSE:	Drops the current trail's points before nPoint.
//				RETAIL: 0x10100df0
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::TrimTrail(LIGHT_CYCLIST* pCyclist, int nTrail, int nPoint, uint32 nTronUnknown)
{
	if(!pCyclist)
		return false;

	if((int)pCyclist->collTrails.size() <= nTrail)
		return false;

	LIGHT_CYCLE_TRAIL* pTrail = pCyclist->collTrails[nTrail];
	if(!pTrail || (pTrail != pCyclist->pCurTrail))
		return false;

	int nPoints = pTrail->collTrailPoints.size();
	if(nPoints > nPoint + 2)
	{
		pTrail->collTrailPoints.erase(pTrail->collTrailPoints.begin(), pTrail->collTrailPoints.begin() + nPoint + 1);
		return true;
	}

	if(nPoints == 2)
	{
		pTrail->collTrailPoints.erase(pTrail->collTrailPoints.begin());

		// Snap the new first point to the grid
		LTVector & vPos = pTrail->collTrailPoints.begin()->vPoint;

		int nX = LCM_ROUND(vPos.x);
		int nZ = LCM_ROUND(vPos.z);

		float fGridSize = m_ButeMgr.GetGridSize();
		if(fGridSize > 1.0f)
		{
			nX = LCM_ROUND(nX / fGridSize);
			vPos.x = nX * fGridSize;
			nZ = LCM_ROUND(nZ / fGridSize);
			vPos.z = nZ * fGridSize;
		}
		else
		{
			vPos.x = (float)nX;
			vPos.z = (float)nZ;
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CutTrailPoint
//
//	PURPOSE:	Pulls the trail point back from the hit point by 64; if it
//				would pass its neighbour, drops it. RETAIL: 0x10100fb0
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::CutTrailPoint(std::vector<LIGHT_CYCLE_TRAIL_POINT> & collPoints, LIGHT_CYCLE_TRAIL_POINT* pPoint,
	LIGHT_CYCLE_TRAIL_POINT* pNeighbour, LightCycleCollisionInfo* pInfo)
{
	uint32 nPoints = collPoints.size();
	float fDX = (float)fabs(pPoint->vPoint.x - pNeighbour->vPoint.x);
	float fDZ = (float)fabs(pPoint->vPoint.z - pNeighbour->vPoint.z);

	uint32 nIndex = 0;
	while((nIndex < nPoints) && (pPoint != &collPoints[nIndex]))
	{
		nIndex++;
	}

	if(fDX > fDZ)
	{
		if(pNeighbour->vPoint.x < pPoint->vPoint.x)
		{
			if(pPoint->vPoint.x == pInfo->vCollisionPoint.x)
			{
				float fX = pInfo->vCollisionPoint.x - 64.0f;
				if(!(fX > pNeighbour->vPoint.x))
					return false;
				pPoint->vPoint.x = fX;
				return true;
			}

			float fX = pInfo->vCollisionPoint.x - 64.0f;
			if(!(fX < pNeighbour->vPoint.x))
			{
				pPoint->vPoint.x = fX;
				return true;
			}
		}
		else
		{
			if(pPoint->vPoint.x == pInfo->vCollisionPoint.x)
			{
				float fX = pInfo->vCollisionPoint.x + 64.0f;
				if(!(fX < pNeighbour->vPoint.x))
					return false;
				pPoint->vPoint.x = fX;
				return true;
			}

			float fX = pInfo->vCollisionPoint.x + 64.0f;
			if(!(fX > pNeighbour->vPoint.x))
			{
				pPoint->vPoint.x = fX;
				return true;
			}
		}
	}
	else
	{
		if(pNeighbour->vPoint.z < pPoint->vPoint.z)
		{
			if(pPoint->vPoint.z == pInfo->vCollisionPoint.z)
			{
				float fZ = pInfo->vCollisionPoint.z - 64.0f;
				if(!(fZ > pNeighbour->vPoint.z))
					return false;
				pPoint->vPoint.z = fZ;
				return true;
			}

			float fZ = pInfo->vCollisionPoint.z - 64.0f;
			if(!(fZ < pNeighbour->vPoint.z))
			{
				pPoint->vPoint.z = fZ;
				return true;
			}
		}
		else
		{
			if(pPoint->vPoint.z == pInfo->vCollisionPoint.z)
			{
				float fZ = pInfo->vCollisionPoint.z + 64.0f;
				if(!(fZ < pNeighbour->vPoint.z))
					return false;
				pPoint->vPoint.z = fZ;
				return true;
			}

			float fZ = pInfo->vCollisionPoint.z + 64.0f;
			if(!(fZ > pNeighbour->vPoint.z))
			{
				pPoint->vPoint.z = fZ;
				return true;
			}
		}
	}

	// It would pass its neighbour: drop it
	if(nPoints <= 2)
		return false;

	collPoints.erase(&collPoints[nIndex]);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::GetLivingCyclist
//
//	PURPOSE:	The object's cyclist, unless it is derezzing.
//				RETAIL: 0x10101250
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST* CLightCycleMgr::GetLivingCyclist(HOBJECT hObj)
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(hObj);
	if(pCyclist && (pCyclist->fStartDerezTime == 0.0f))
		return pCyclist;

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::BeginTrailSpike
//
//	PURPOSE:	Starts the trail spike's two trails at vPos, across the
//				cycle's travel. RETAIL: 0x10101280
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::BeginTrailSpike(LIGHT_CYCLIST* pCyclist, LTVector* pvPos)
{
	pCyclist->vSpikePos = *pvPos;

	if(pCyclist->vForward.x > 0.5f)
	{
		pCyclist->vSpikeDir.z = 1.0f;
		pCyclist->vSpikeDir.x = 0.0f;
		pCyclist->vSpikeDir2.x = 0.0f;
		pCyclist->vSpikeDir2.z = -1.0f;
	}
	else if(pCyclist->vForward.x < -0.5f)
	{
		pCyclist->vSpikeDir.z = -1.0f;
		pCyclist->vSpikeDir.x = 0.0f;
		pCyclist->vSpikeDir2.x = 0.0f;
		pCyclist->vSpikeDir2.z = 1.0f;
	}
	else if(pCyclist->vForward.z > 0.5f)
	{
		pCyclist->vSpikeDir.x = -1.0f;
		pCyclist->vSpikeDir.z = 0.0f;
		pCyclist->vSpikeDir2.x = 1.0f;
		pCyclist->vSpikeDir2.z = 0.0f;
	}
	else
	{
		pCyclist->vSpikeDir.x = 1.0f;
		pCyclist->vSpikeDir.z = 0.0f;
		pCyclist->vSpikeDir2.x = -1.0f;
		pCyclist->vSpikeDir2.z = 0.0f;
	}

	LIGHT_CYCLE_TRAIL* pTrail = debug_new(LIGHT_CYCLE_TRAIL);
	pTrail->m_bTronUnknownC = true;
	pCyclist->collTrails.push_back(pTrail);
	pCyclist->pSpikeTrail = pTrail;

	pTrail = debug_new(LIGHT_CYCLE_TRAIL);
	pTrail->m_bTronUnknownC = true;
	pCyclist->collTrails.push_back(pTrail);
	pCyclist->pSpikeTrail2 = pTrail;

	LTVector vStart = (pCyclist->vSpikeDir * 48.0f) + pCyclist->vSpikePos;
	LTVector vStart2 = (pCyclist->vSpikeDir2 * 48.0f) + pCyclist->vSpikePos;

	// Each trail starts as two points, its end is moved by UpdateTrailSpike
	AddLightCycleTrailPoint(pCyclist, pCyclist->pSpikeTrail, vStart);
	AddLightCycleTrailPoint(pCyclist, pCyclist->pSpikeTrail2, vStart2);
	AddLightCycleTrailPoint(pCyclist, pCyclist->pSpikeTrail, vStart);
	AddLightCycleTrailPoint(pCyclist, pCyclist->pSpikeTrail2, vStart2);

	pCyclist->fSpikeFullTime = 0.0f;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::UpdateTrailSpike
//
//	PURPOSE:	Grows the trail spike's ends to the bute distance; after the
//				bute delay its starts follow, then it is removed.
//				RETAIL: 0x10101520
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::UpdateTrailSpike(LIGHT_CYCLIST* pCyclist)
{
	bool bRemove = false;

	if(pCyclist->pSpikeTrail || pCyclist->pSpikeTrail2)
	{
		LTVector* pPoint = NULL;
		LTVector* pPoint2 = NULL;

		if(pCyclist->fSpikeFullTime > 0.0f)
		{
			// Grown: after the delay its starts close in
			float fDelay = m_ButeMgr.GetTrailSpikeDelay();
			if((fDelay + pCyclist->fSpikeFullTime) > LCM_LT->TronUnknownVirtual44())
				return;

			if(pCyclist->pSpikeTrail)
				pPoint = &pCyclist->pSpikeTrail->collTrailPoints.begin()->vPoint;

			if(pCyclist->pSpikeTrail2)
				pPoint2 = &pCyclist->pSpikeTrail2->collTrailPoints.begin()->vPoint;
		}
		else
		{
			if(pCyclist->pSpikeTrail)
				pPoint = &(pCyclist->pSpikeTrail->collTrailPoints.begin() + 1)->vPoint;

			if(pCyclist->pSpikeTrail2)
				pPoint2 = &(pCyclist->pSpikeTrail2->collTrailPoints.begin() + 1)->vPoint;
		}

		float fMove = LCM_LT->TronUnknownVirtual45() * m_ButeMgr.GetTrailSpikeSpeed();

		LTVector* pDir = NULL;
		LTVector* pNew = NULL;
		LTVector vNew, vNew2;

		if(pPoint)
		{
			vNew = *pPoint + (pCyclist->vSpikeDir * fMove);
			pDir = &pCyclist->vSpikeDir;
			pNew = &vNew;
		}

		if(pPoint2)
		{
			vNew2 = *pPoint2 + (pCyclist->vSpikeDir2 * fMove);
			pDir = &pCyclist->vSpikeDir2;
			pNew = &vNew2;
		}

		if(pDir && pNew)
		{
			float fDist = m_ButeMgr.GetTrailSpikeDist();
			bool bFull = false;

			if(pDir->x > 0.5f)
			{
				bFull = ((pNew->x - pCyclist->vSpikePos.x) > fDist);
			}
			else if(pDir->x < -0.5f)
			{
				bFull = ((pCyclist->vSpikePos.x - pNew->x) > fDist);
			}
			else if(pDir->z > 0.5f)
			{
				bFull = ((pNew->z - pCyclist->vSpikePos.z) > fDist);
			}
			else
			{
				bFull = ((pCyclist->vSpikePos.z - pNew->z) > fDist);
			}

			if(bFull)
			{
				if(pCyclist->fSpikeFullTime > 0.0f)
				{
					// The starts reached the ends: done
					bRemove = true;
				}
				else
				{
					// Clamp the ends to the distance
					if((pDir->x > 0.5f) || (pDir->x < -0.5f))
					{
						if(pPoint)
							vNew.x = (fDist * pCyclist->vSpikeDir.x) + pCyclist->vSpikePos.x;
						if(pPoint2)
							vNew2.x = (fDist * pCyclist->vSpikeDir2.x) + pCyclist->vSpikePos.x;
					}
					else
					{
						if(pPoint)
							vNew.z = (fDist * pCyclist->vSpikeDir.z) + pCyclist->vSpikePos.z;
						if(pPoint2)
							vNew2.z = (fDist * pCyclist->vSpikeDir2.z) + pCyclist->vSpikePos.z;
					}

					pCyclist->fSpikeFullTime = LCM_LT->TronUnknownVirtual44();
				}
			}

			if(pPoint && (pCyclist->pSpikeTrail->fDerezStartTime == 0.0f))
			{
				pPoint->x = vNew.x;
				pPoint->z = vNew.z;
			}

			if(pPoint2 && (pCyclist->pSpikeTrail2->fDerezStartTime == 0.0f))
			{
				pPoint2->x = vNew2.x;
				pPoint2->z = vNew2.z;
			}

			if(!bRemove)
				return;
		}
	}

	pCyclist->ClearTrailSpike();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::Update
//
//	PURPOSE:	Starts the derez of the living cyclists' old trails.
//				RETAIL: 0x10101910
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::Update()
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		LIGHT_CYCLIST* pCyclist = (*iter);
		if(pCyclist->fStartDerezTime == 0.0f)
		{
			std::vector<LIGHT_CYCLE_TRAIL*>::iterator iterTrail;
			for(iterTrail=pCyclist->collTrails.begin();iterTrail!=pCyclist->collTrails.end();iterTrail++)
			{
				if(((*iterTrail) != pCyclist->pCurTrail) && ((*iterTrail)->fDerezStartTime == 0.0f))
				{
					(*iterTrail)->fDerezStartTime = LCM_LT->GetTime();
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::GetDifficulty
//
//	PURPOSE:	The game's difficulty. RETAIL: 0x10101980
//
// ----------------------------------------------------------------------- //
int CLightCycleMgr::GetDifficulty()
{
#ifdef _CLIENTBUILD
	return g_pGameClientShell->GetDifficulty();
#else
	return g_pGameServerShell->GetDifficulty();
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::GetGameSpeedIndex
//
//	PURPOSE:	The GameSpeedScalars index of our scalar. RETAIL: 0x10101990
//
// ----------------------------------------------------------------------- //
int CLightCycleMgr::GetGameSpeedIndex()
{
	for(int i = 0; i <= 3; i++)
	{
		float fScalar = g_pLightCycleButeMgr->GetGameSpeedScalar(i);
		if(((fScalar + 0.1f) > m_fGameSpeedScalar) && ((fScalar - 0.1f) < m_fGameSpeedScalar))
			return i;
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::GetTrailLengthIndex
//
//	PURPOSE:	The TrailLengthValues index of our length. RETAIL: 0x101019e0
//
// ----------------------------------------------------------------------- //
int CLightCycleMgr::GetTrailLengthIndex()
{
	for(int i = 0; i <= 3; i++)
	{
		float fLength = g_pLightCycleButeMgr->GetTrailLengthValue(i);
		if(((fLength + 100.0f) > m_fTrailLength) && ((fLength - 100.0f) < m_fTrailLength))
			return i;
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::GetTronUnknown1338
//
//	PURPOSE:	RETAIL: 0x10101a30
//
// ----------------------------------------------------------------------- //
int CLightCycleMgr::GetTronUnknown1338()
{
	return m_nTronUnknown1338;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::UpdateCyclistForward
//
//	PURPOSE:	Turns the cycle's forward to the axis nearest its rotation's.
//				RETAIL: 0x10101a40
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::UpdateCyclistForward(LIGHT_CYCLIST* pCyclist)
{
	LTRotation rRot;
	LCM_LT->GetObjectRotation(pCyclist->hObject, &rRot);

	pCyclist->vForward = rRot.Forward();
	pCyclist->vForward.y = 0.0f;

	if(fabs(pCyclist->vForward.x) > fabs(pCyclist->vForward.z))
	{
		if(pCyclist->vForward.x > 0.0f)
		{
			pCyclist->vForward.x = 1.0f;
			pCyclist->vForward.z = 0.0f;
		}
		else
		{
			pCyclist->vForward.x = -1.0f;
			pCyclist->vForward.z = 0.0f;
		}
	}
	else
	{
		if(pCyclist->vForward.z > 0.0f)
		{
			pCyclist->vForward.z = 1.0f;
			pCyclist->vForward.x = 0.0f;
		}
		else
		{
			pCyclist->vForward.z = -1.0f;
			pCyclist->vForward.x = 0.0f;
		}
	}
}

