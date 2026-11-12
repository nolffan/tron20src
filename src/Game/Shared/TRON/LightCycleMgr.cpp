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
	pCyclist->m_nTronUnknownE0 = 0;
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
	pCyclist->m_fTronUnknown78		= 0.0f;

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
	if(AddLightCycleTrailPoint(pCyclist, pTrail, vPos))
	{
		return pTrail;
	}

	return NULL;
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
	// RETAIL: 0x100fedd0 (1055 bytes) is not written yet: the trail is only
	// started (see BeginLightCycleTrail).
	if(!pCyclist)
		return false;

	if(!pCyclist->pCurTrail)
	{
		return (BeginLightCycleTrail(pCyclist, pCyclist->vLastPos) != NULL);
	}

	if(pvForward)
	{
		pCyclist->vForward = *pvForward;
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
//	ROUTINE:	CLightCycleMgr::HandleCollision
//
//	PURPOSE:	Handles a collision. RETAIL: 0x100ffbe0
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::HandleCollision(LIGHT_CYCLIST *pCyclist, LightCycleCollisionInfo &info)
{
	TronUnknownVirtual5(pCyclist, info.nTronUnknown0, true);
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
bool CLightCycleMgr::ActivatePowerup(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown)
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
			TronUnknownVirtual10(pCyclist, nTronUnknown);
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

// ----------------------------------------------------------------------- //
// RETAIL: the virtuals below are not written yet (their retail bodies are
// given); they do nothing.
// ----------------------------------------------------------------------- //

// RETAIL: 0x10100960 (1156 bytes)
bool CLightCycleMgr::TronUnknownVirtual8(uint32, uint32, uint32, uint32)
{
	return false;
}

// RETAIL: 0x10100df0 (441 bytes)
bool CLightCycleMgr::TronUnknownVirtual9(uint32, uint32, uint32, uint32)
{
	return false;
}

// RETAIL: 0x10101280 (666 bytes)
bool CLightCycleMgr::TronUnknownVirtual10(LIGHT_CYCLIST* pCyclist, uint32)
{
	return false;
}

// RETAIL: 0x10101520 (1008 bytes)
bool CLightCycleMgr::TronUnknownVirtual11(LIGHT_CYCLIST* pCyclist)
{
	return false;
}

// RETAIL: 0x100fffd0 (1275 bytes)
bool CLightCycleMgr::TronUnknownVirtual12(LIGHT_CYCLIST* pCyclist, uint32)
{
	return false;
}
