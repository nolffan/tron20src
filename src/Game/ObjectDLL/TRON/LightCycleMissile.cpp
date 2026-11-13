// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMissile.cpp
//
// PURPOSE : The missile power-up of the light cycle game
//
// RETAIL: Tron 2.0's, written from retail Object.lto (LightCycleMissile.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleMissile.h"
#include "LightCycleMgr.h"
#include "PlayerObj.h"
#include "TronPlayerObj.h"
#include "ServerUtilities.h"

// RETAIL: 0x10358ca0, the cyclist the missile's segment is tested as
// (0x1002a6bc); built by 0x10029fb0. The name is ours.
static LIGHT_CYCLIST s_MissileCyclist(1.0f, 1.0f, true);

BEGIN_CLASS(LightCycleMissile)
END_CLASS_DEFAULT_FLAGS(LightCycleMissile, GameBase, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleMissile::LightCycleMissile()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

LightCycleMissile::LightCycleMissile() : GameBase(OT_NORMAL)
{
	m_hFX = LTNULL;
	m_bCreateFX = true;
	m_vStartPos.Init();
	m_vDir.Init();
	m_hOwner = LTNULL;
	m_pOwnerCyclist = NULL;
	m_fLastHitTime = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleMissile::~LightCycleMissile()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

LightCycleMissile::~LightCycleMissile()
{
	if (m_hFX)
	{
		g_pLTServer->RemoveObject(m_hFX);
		m_hFX = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleMissile::PostPropRead
//
//	PURPOSE:	Update the create struct
//
// ----------------------------------------------------------------------- //

void LightCycleMissile::PostPropRead(ObjectCreateStruct *pStruct)
{
	pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleMissile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 LightCycleMissile::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData != PRECREATE_SAVEGAME)
			{
				PostPropRead((ObjectCreateStruct*)pData);
				m_bCreateFX = true;
			}
			else
			{
				m_bCreateFX = false;
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleMissile::InitialUpdate
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL LightCycleMissile::InitialUpdate()
{
	CreateFX();
	SetNextUpdate(UPDATE_NEXT_FRAME);
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleMissile::Update
//
//	PURPOSE:	Fly: derez the cycles and trails on the way, and go when
//				something solid is hit
//
// ----------------------------------------------------------------------- //

void LightCycleMissile::Update()
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	LTVector vVel = m_vDir * g_pLightCycleButeMgr->GetMissileSpeed();
	LTVector vNewPos;
	vNewPos = vPos + vVel * g_pLTServer->GetFrameTime();

	LIGHT_CYCLE_TRAIL_POINT ptStart, ptEnd;
	g_pLightCycleMgr->CreateTrailPointFromVector(ptStart, vPos);
	g_pLightCycleMgr->CreateTrailPointFromVector(ptEnd, vNewPos);

	LIGHT_CYCLIST* pOwner = m_hOwner ? m_pOwnerCyclist : NULL;

	LightCycleCollisionInfo info;
	while (g_pLightCycleMgr->CheckForCollision(&s_MissileCyclist, &ptStart, &ptEnd.vPoint, &info, true, pOwner))
	{
		bool bNewHit;
		if (g_pLTServer->GetTime() - m_fLastHitTime < 0.5f)
		{
			bNewHit = false;
		}
		else
		{
			m_fLastHitTime = g_pLTServer->GetTime();
			bNewHit = true;
		}

		if (info.nTrail == -1)
		{
			// A cycle
			g_pLightCycleMgr->DerezCyclist(info.pCyclist, pOwner, true);
		}
		else
		{
			g_pLightCycleMgr->HandleShieldCollision(&s_MissileCyclist, &info, bNewHit, false);
		}
	}

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To = vNewPos;
	IQuery.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY;
	IQuery.m_FilterFn = FilterFn;
	IQuery.m_pUserData = LTNULL;

	if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		if (m_hFX)
		{
			g_pLTServer->RemoveObject(m_hFX);
			m_hFX = LTNULL;
		}

		g_pLTServer->RemoveObject(m_hObject);

		char szFX[512];
		sprintf(szFX, "FxName %s%s%s", LCPOWERUP_SOUND_PREFIX, g_aszLCPowerupNames[kLCPowerup_Missile], LCPOWERUP_SOUND_REMOVE);
		CreateClientFXObject(szFX, m_hObject);
		return;
	}

	g_pLTServer->MoveObject(m_hObject, &vNewPos);

	if (m_hFX)
	{
		g_pLTServer->SetObjectPos(m_hFX, &vNewPos);
	}

	SetNextUpdate(UPDATE_NEXT_FRAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleMissile::CreateFX
//
//	PURPOSE:	The missile's looping client fx
//
// ----------------------------------------------------------------------- //

void LightCycleMissile::CreateFX()
{
	if (!m_bCreateFX) return;

	char szFX[512];
	sprintf(szFX, "FxName %s%s%s;Loop 1;SmoothShutdown 0", LCPOWERUP_SOUND_PREFIX, g_aszLCPowerupNames[kLCPowerup_Missile], LCPOWERUP_SOUND_ACTIVE);
	m_hFX = CreateClientFXObject(szFX, m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleMissile::FilterFn
//
//	PURPOSE:	Fly through cycles (the players' cycle models too); stop at
//				what is solid
//
// ----------------------------------------------------------------------- //

bool LightCycleMissile::FilterFn(HOBJECT hObj, void *pUserData)
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);

	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(hObj);
	if (pCyclist)
	{
		if (pCyclist->hObject == hObj)
			return false;

		if (pCyclist->bIsPlayer)
		{
			CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->HandleToObject(hObj);
			if (hObj == pPlayer->m_hLightCycle)
				return false;
		}
	}

	return (dwFlags & FLAG_SOLID) ? true : false;
}
