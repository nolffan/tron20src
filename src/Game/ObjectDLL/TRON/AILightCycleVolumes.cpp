// ----------------------------------------------------------------------- //
//
// MODULE  : AILightCycleVolumes.cpp
//
// PURPOSE : The walls of the light cycle AI: the outlines of the enabled
//			 AI volumes
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1000a1f0..0x1000abc0, a TU without classes after
// AILightCycle's). The module's and the functions' names are ours
// (declared in AILightCycle.h). Retail's AIVolume has its corners 8 bytes
// further on than ours (+0x50, +0x5c, +0x68, +0x74) and IsVolumeEnabled at
// +0xd0.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AILightCycle.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include <list>

typedef std::list<AIVolume*> AIVolumeList;

static AIVolumeList	s_lstVolumes;				// RETAIL: 0x10337cc8
static bool			s_bVolumesInitted = false;	// RETAIL: 0x10337cf4

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitLightCycleVolumes
//
//	PURPOSE:	Collect the AI volumes
//
// ----------------------------------------------------------------------- //

void InitLightCycleVolumes()
{
	if (s_bVolumesInitted == true)
	{
		ClearLightCycleVolumes();
	}

	uint32 nNumVolumes = g_pAIVolumeMgr->GetNumVolumes();
	for (uint32 i = 0; i < nNumVolumes; i++)
	{
		AIVolume* pVolume = g_pAIVolumeMgr->GetVolume(i);
		if (pVolume)
		{
			s_lstVolumes.push_back(pVolume);
		}
	}

	s_bVolumesInitted = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClearLightCycleVolumes
//
//	PURPOSE:	Forget the AI volumes
//
// ----------------------------------------------------------------------- //

void ClearLightCycleVolumes()
{
	s_lstVolumes.clear();
	s_bVolumesInitted = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsPointInLightCycleVolume
//
//	PURPOSE:	Is vPos (x, z) in an enabled volume?
//
// ----------------------------------------------------------------------- //

bool IsPointInLightCycleVolume(const LTVector & vPos)
{
	for (AIVolumeList::iterator it = s_lstVolumes.begin(); it != s_lstVolumes.end(); ++it)
	{
		AIVolume* pVolume = *it;
		if (pVolume->IsVolumeEnabled() &&
			vPos.x <= pVolume->GetFrontTopRight().x &&
			vPos.x >= pVolume->GetFrontTopLeft().x &&
			vPos.z <= pVolume->GetFrontTopLeft().z &&
			vPos.z >= pVolume->GetBackTopLeft().z)
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IntersectLightCycleVolumes
//
//	PURPOSE:	The nearest crossing of vStart..vEnd with an enabled
//				volume's outline
//
// ----------------------------------------------------------------------- //

bool IntersectLightCycleVolumes(const LTVector & vStart, const LTVector & vEnd,
	float* pfDist, LTVector* pvHit, LTVector* pvHit2)
{
	*pfDist = (float)HUGE_VAL;

	for (AIVolumeList::iterator it = s_lstVolumes.begin(); it != s_lstVolumes.end(); ++it)
	{
		AIVolume* pVolume = *it;
		if (!pVolume->IsVolumeEnabled()) continue;

		LTVector vHit, vHit2;
		float fDist;

		LTVector vA = pVolume->GetFrontTopLeft();
		LTVector vB = pVolume->GetFrontTopRight();
		if (IntersectSegments(vStart, vEnd, vA, vB, &vHit, &vHit2))
		{
			fDist = LTVector(vHit.x - vStart.x, vHit.y - vStart.y, vHit.z - vStart.z).Mag();
			if (fDist < *pfDist)
			{
				*pfDist = fDist;
				*pvHit = vHit;
				*pvHit2 = vHit2;
			}
		}

		vA = pVolume->GetFrontTopRight();
		vB = pVolume->GetBackTopRight();
		if (IntersectSegments(vStart, vEnd, vA, vB, &vHit, &vHit2))
		{
			fDist = LTVector(vHit.x - vStart.x, vHit.y - vStart.y, vHit.z - vStart.z).Mag();
			if (fDist < *pfDist)
			{
				*pfDist = fDist;
				*pvHit = vHit;
				*pvHit2 = vHit2;
			}
		}

		vA = pVolume->GetBackTopRight();
		vB = pVolume->GetBackTopLeft();
		if (IntersectSegments(vStart, vEnd, vA, vB, &vHit, &vHit2))
		{
			fDist = LTVector(vHit.x - vStart.x, vHit.y - vStart.y, vHit.z - vStart.z).Mag();
			if (fDist < *pfDist)
			{
				*pfDist = fDist;
				*pvHit = vHit;
				*pvHit2 = vHit2;
			}
		}

		vA = pVolume->GetBackTopLeft();
		vB = pVolume->GetFrontTopLeft();
		if (IntersectSegments(vStart, vEnd, vA, vB, &vHit, &vHit2))
		{
			fDist = LTVector(vHit.x - vStart.x, vHit.y - vStart.y, vHit.z - vStart.z).Length();
			if (fDist < *pfDist)
			{
				*pfDist = fDist;
				*pvHit = vHit;
				*pvHit2 = vHit2;
			}
		}
	}

	return (*pfDist != HUGE_VAL);
}
