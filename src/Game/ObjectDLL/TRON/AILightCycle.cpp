// ----------------------------------------------------------------------- //
//
// MODULE  : AILightCycle.cpp
//
// PURPOSE : An AI controlled light cycle of the light cycle game
//
// RETAIL: Tron 2.0's, written from retail Object.lto (AILightCycle.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AILightCycle.h"
#include "ServerLightCycleMgr.h"
#include "LightCyclePowerup.h"
#include "PlayerObj.h"
#include "CommandMgr.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "ServerUtilities.h"
#include <algorithm>

// RETAIL: the Behavior property's values (0x102d6770, .data; ReadProp
// compares with them). The names are ours.
static char* s_aszBehaviors[] =
{
	"aggressive",
	"wander",
	"avoid",
};

// RETAIL: a copy the plugin lists (0x10337ba4, filled by 0x10001250)
static const char* s_aszBehaviorList[] =
{
	s_aszBehaviors[0],
	s_aszBehaviors[1],
	s_aszBehaviors[2],
};

// RETAIL: vtable 0x102932d0 (4 bytes): fills Behavior. Its function is
// defined in the class (emitted with the vtable, 0x100015a0).
class CAILCPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength)
		{
			if (_stricmp("Behavior", szPropName) == 0)
			{
				strncpy(aszStrings[*pcStrings], s_aszBehaviorList[0], cMaxStringLength);
				strncpy(aszStrings[++(*pcStrings)], s_aszBehaviorList[1], cMaxStringLength);
				strncpy(aszStrings[++(*pcStrings)], s_aszBehaviorList[2], cMaxStringLength);
				++(*pcStrings);
				return LT_OK;
			}

			return LT_OK;
		}
};

BEGIN_CLASS(AILightCycle)
	ADD_BOOLPROP(Scripted, LTFALSE)
	ADD_BOOLPROP(StartPaused, LTFALSE)
	ADD_LONGINTPROP(NameId, 0)
	ADD_STRINGPROP(CrashCmd, "")
	ADD_STRINGPROP(RemoveCmd, "")
	ADD_STRINGPROP_FLAG(Behavior, s_aszBehaviors[0], PF_STATICLIST)
	ADD_BOOLPROP(GoForward, LTFALSE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(AILightCycle, LightCycle, NULL, NULL, 0, CAILCPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetRandomUnit
//
//	PURPOSE:	A random number in 0..1. RETAIL: 0x10001690
//
// ----------------------------------------------------------------------- //

static float GetRandomUnit()
{
	return rand() * (1.0f / RAND_MAX);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetTimeToPoint
//
//	PURPOSE:	How long the cyclist takes to get to vPoint at its speed.
//				RETAIL: 0x100016b0
//
// ----------------------------------------------------------------------- //

static float GetTimeToPoint(LIGHT_CYCLIST* pCyclist, const LTVector & vPoint)
{
	if (pCyclist)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(pCyclist->hObject, &vPos);

		LTVector vDiff = vPoint - vPos;
		if (pCyclist->fSpeed != 0.0f)
		{
			return vDiff.Mag() / pCyclist->fSpeed;
		}
	}

	return FLT_MAX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FindTrailHit
//
//	PURPOSE:	The nearest point where vStart..vEnd crosses the trail.
//				RETAIL: 0x10001730 (pnPoint is not set)
//
// ----------------------------------------------------------------------- //

static bool FindTrailHit(const LTVector & vStart, const LTVector & vEnd, LIGHT_CYCLE_TRAIL* pTrail,
	float* pfDist, LTVector* pvHit, int* pnPoint)
{
	float fBest = (float)HUGE_VAL;

	if (pTrail->collTrailPoints.size() < 2) return false;

	LTVector vPrev = pTrail->collTrailPoints[0].vPoint;
	for (uint32 i = 1; i < pTrail->collTrailPoints.size(); i++)
	{
		LTVector vCur = pTrail->collTrailPoints[i].vPoint;

		LTVector vHit, vHit2;
		if (IntersectSegments(vStart, vEnd, vPrev, vCur, &vHit, &vHit2))
		{
			*pfDist = (vStart - vHit).Mag();
			if (*pfDist < fBest)
			{
				*pvHit = vHit;
				fBest = *pfDist;
			}
		}

		vPrev = vCur;
	}

	return (HUGE_VAL != fBest);
}

// RETAIL: the rotations of the four directions (0x103379d0, filled by
// 0x10001900). The name is ours.
static LTRotation s_arDirections[4] =
{
	LTRotation(0.0f, 0.0f, 0.0f),
	LTRotation(0.0f, MATH_HALFPI, 0.0f),
	LTRotation(0.0f, MATH_PI, 0.0f),
	LTRotation(0.0f, MATH_PI + MATH_HALFPI, 0.0f),
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MovePoint
//
//	PURPOSE:	Moves vPos fDist along the direction nDir (inline)
//
// ----------------------------------------------------------------------- //

static inline void MovePoint(LTVector & vPos, int nDir, float fDist)
{
	switch (nDir)
	{
		case 0: vPos.z += fDist; break;
		case 1: vPos.x += fDist; break;
		case 2: vPos.z -= fDist; break;
		case 3: vPos.x -= fDist; break;
	}
}

// RETAIL: the directions to the right and to the left of a direction
static inline int TurnRight(int nDir)
{
	nDir++;
	if (nDir > 3) nDir = 0;
	return nDir;
}

static inline int TurnLeft(int nDir)
{
	nDir--;
	if (nDir < 0) nDir = 3;
	return nDir;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::AILightCycle()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AILightCycle::AILightCycle() : LightCycle()
{
	m_bFirstUpdate = false;
	m_bScripted = false;
	m_bMoved = false;
	m_bPaused = false;
	m_fUpdateDelta = 0.01f;
	m_bWaitingForClient = false;
	m_bTronUnknown70 = false;
	m_anTurns[0] = m_anTurns[1] = m_anTurns[2] = m_anTurns[3] = 0xff;
	m_nNameId = 0;
	m_dwMoveFlags = 0;
	m_nLastPowerup = LC_NO_POWERUP;
	m_bGoForward = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::~AILightCycle()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

AILightCycle::~AILightCycle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AILightCycle::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = LightCycle::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_UPDATE:
		{
			if (!m_bPaused)
			{
				Update();
			}
		}
		break;

		default : break;
	}

	return LightCycle::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 AILightCycle::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch (messageID)
	{
		case MID_LIGHT_CYCLE_INFO:
		{
			uint8 nType = pMsg->Readuint8();
			switch (nType)
			{
				case LCI_DEREZ_CYCLIST:
				{
					g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
					SetNextUpdate(UPDATE_NEVER);

					if (m_sCrashCmd != "")
					{
						g_pCmdMgr->Process(m_sCrashCmd.c_str(), m_hObject, m_hObject);
					}
				}
				return LT_OK;

				case LCI_REMOVE_CYCLIST:
				{
					if (m_sRemoveCmd != "")
					{
						g_pCmdMgr->Process(m_sRemoveCmd.c_str(), m_hObject, m_hObject);
					}

					g_pLTServer->RemoveObject(m_hObject);
					m_pCyclist = LTNULL;
				}
				return LT_OK;

				case LCI_START_UPDATING:
				{
					m_bWaitingForClient = false;
					SetNextUpdate(m_fUpdateDelta);

					if (m_hstrRezInFXDoneCommand)
					{
						const char* szCmd = g_pLTServer->GetStringData(m_hstrRezInFXDoneCommand);
						if (g_pCmdMgr->IsValidCmd(szCmd))
						{
							g_pCmdMgr->Process(szCmd, m_hObject, (HOBJECT)LTNULL);
						}
					}
				}
				break;
			}
		}
		// RETAIL: falls through into MID_TRIGGER (0x100020f1), which reads
		// the rest of the message as a trigger message

		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)pMsg->Readuint32();

			ConParse parse;
			parse.Init((char*)szMsg);

			while (g_pCommonLT->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (_stricmp(parse.m_Args[0], "START") == 0)
					{
						SetNextUpdate(m_fUpdateDelta);
					}
					else if (_stricmp(parse.m_Args[0], "RESUME") == 0)
					{
						SetNextUpdate(m_fUpdateDelta);
						m_bPaused = false;
					}
					else if (_stricmp(parse.m_Args[0], "PAUSE") == 0)
					{
						SetNextUpdate(UPDATE_NEVER);
						m_bPaused = true;
					}
					else if (_stricmp(parse.m_Args[0], "REMOVE") == 0)
					{
						if (!g_pLightCycleMgr->RemoveCyclist(m_hObject))
						{
							g_pLTServer->RemoveObject(m_hObject);
						}
					}
				}
			}
		}
		return LT_OK;

		default : break;
	}

	return LightCycle::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL AILightCycle::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("NameId", &genProp) == LT_OK)
	{
		m_nNameId = genProp.m_Long;
	}

	if (g_pLTServer->GetPropGeneric("Scripted", &genProp) == LT_OK)
	{
		m_bScripted = !!genProp.m_Bool;
	}

	if (g_pLTServer->GetPropGeneric("StartPaused", &genProp) == LT_OK)
	{
		m_bPaused = !!genProp.m_Bool;
	}

	if (g_pLTServer->GetPropGeneric("CrashCmd", &genProp) == LT_OK)
	{
		if (g_pCmdMgr->IsValidCmd(genProp.m_String))
		{
			m_sCrashCmd = genProp.m_String;
		}
	}

	if (g_pLTServer->GetPropGeneric("RemoveCmd", &genProp) == LT_OK)
	{
		if (g_pCmdMgr->IsValidCmd(genProp.m_String))
		{
			m_sRemoveCmd = genProp.m_String;
		}
	}

	// RETAIL: whatever the property says, the default behavior ends up
	// wander (0x1000259e); see docs/RETAIL_BUGS.md
	char szBehavior[256];
	if (g_pLTServer->GetPropString("Behavior", szBehavior, ARRAY_LEN(szBehavior)) == LT_OK)
	{
		if (_stricmp(szBehavior, s_aszBehaviors[0]) == 0)
		{
			m_eDefaultBehavior = kAILCBehavior_Aggressive;
		}
		else if (_stricmp(szBehavior, s_aszBehaviors[1]) == 0)
		{
			m_eDefaultBehavior = kAILCBehavior_Wander;
		}
		else if (_stricmp(szBehavior, s_aszBehaviors[2]) == 0)
		{
			m_eDefaultBehavior = kAILCBehavior_Wander;
		}
		else
		{
			m_eDefaultBehavior = kAILCBehavior_Wander;
		}
	}
	else
	{
		m_eDefaultBehavior = kAILCBehavior_Wander;
	}
	m_eDefaultBehavior = kAILCBehavior_Wander;

	if (g_pLTServer->GetPropGeneric("GoForward", &genProp) == LT_OK)
	{
		m_bGoForward = !!genProp.m_Bool;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::InitialUpdate
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL AILightCycle::InitialUpdate()
{
	LightCycle::InitialUpdate();

	m_bFirstUpdate = true;
	SetNextUpdate(0.5f);

	{
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		LTVector vForward = rRot.Forward();
		if (fabs(vForward.x) > fabs(vForward.z))
		{
			if (vForward.x > 0.0f)
			{
				m_nDirection = 1;
			}
			else
			{
				m_nDirection = 3;
			}
		}
		else
		{
			if (vForward.z > 0.0f)
			{
				m_nDirection = 0;
			}
			else
			{
				m_nDirection = 2;
			}
		}
	}

	m_nTronUnknownC4 = 0;

	Init(LTNULL);

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	m_vStartPos = vPos;

	m_lstPath.clear();
	m_pPowerup = LTNULL;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::Start
//
//	PURPOSE:	On the grid, facing its direction; add the cyclist
//
// ----------------------------------------------------------------------- //

void AILightCycle::Start()
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLightCycleMgr->SnapToGrid(vPos);
	g_pLTServer->SetObjectPos(m_hObject, &vPos);
	g_pLTServer->SetObjectRotation(m_hObject, &s_arDirections[m_nDirection]);

	bool bTronUnknown = true;
	if (m_bScripted)
	{
		m_pfnState = &AILightCycle::UpdateScripted;
	}
	else if (m_bGoForward)
	{
		m_pfnState = &AILightCycle::UpdateGoForward;
		bTronUnknown = false;
	}
	else
	{
		m_pfnState = &AILightCycle::UpdateAI;
	}

	m_eTronUnknown8C = m_eBehavior = m_eDefaultBehavior;

	m_pCyclist = g_pLightCycleMgr->AddCyclist(m_hObject, m_szCycleColor, false, LTNULL, m_nNameId, bTronUnknown);
	m_bWaitingForClient = true;
	m_bFirstUpdate = false;
	m_fCycleLength = m_pCyclist->fLength * 2.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetDirectionTo
//
//	PURPOSE:	The direction from us to vPos
//
// ----------------------------------------------------------------------- //

int AILightCycle::GetDirectionTo(const LTVector & vPos)
{
	LTVector vMyPos;
	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

	float fX = vPos.x - vMyPos.x;
	float fZ = vPos.z - vMyPos.z;

	if (fabs(fZ) > fabs(fX))
	{
		return (fZ < 0.0f) ? 2 : 0;
	}

	return (fX > 0.0f) ? 1 : 3;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetPlayerDirection
//
//	PURPOSE:	The direction the player travels
//
// ----------------------------------------------------------------------- //

int AILightCycle::GetPlayerDirection()
{
	LIGHT_CYCLIST* pPlayer = GetPlayerCyclist();
	if (pPlayer)
	{
		LTVector vForward = pPlayer->vForward;
		if (fabs(vForward.z) > fabs(vForward.x))
		{
			if (vForward.z < 0.0f) return 2;
		}
		else
		{
			return (vForward.x > 0.0f) ? 1 : 3;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::Probe
//
//	PURPOSE:	Look fDist ahead in direction nDir (from beside the trail
//				when going straight on)
//
// ----------------------------------------------------------------------- //

int AILightCycle::Probe(int nDir, AILCProbeResult* pResult, float fDist)
{
	int nResult = AILC_HIT_NONE;
	int nPoint = 0;

	LTVector vStart, vEnd;

	if (nDir == m_nDirection)
	{
		// RETAIL: alternately from the left and the right of the cycle
		static bool s_bLeft = true;

		LTVector vOffset;
		if (s_bLeft)
		{
			vOffset = GetLeft() * 10.0f;
			s_bLeft = false;
		}
		else
		{
			s_bLeft = true;
			vOffset = GetRight() * 10.0f;
		}

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		vPos += vOffset;
		MovePoint(vPos, nDir, 1.0f);
		vStart = vPos;

		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		vPos += vOffset;
		MovePoint(vPos, nDir, fDist);
		vEnd = vPos;
	}
	else
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		MovePoint(vPos, nDir, 1.0f);
		vStart = vPos;

		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		MovePoint(vPos, nDir, fDist);
		vEnd = vPos;
	}

	float fHitDist;
	LTVector vHit;

	uint32 nNumCyclists = g_pLightCycleMgr->m_collCyclists.size();
	for (uint32 i = 0; i < nNumCyclists; i++)
	{
		LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->m_collCyclists[i];
		if (!pCyclist || !pCyclist->collTrails.size()) continue;

		uint32 nNumTrails = pCyclist->collTrails.size();
		for (uint32 j = 0; j < nNumTrails; j++)
		{
			// RETAIL: always the first trail (0x10002e92); see
			// docs/RETAIL_BUGS.md
			if (FindTrailHit(vStart, vEnd, pCyclist->collTrails[0], &fHitDist, &vHit, &nPoint))
			{
				AILCProbeResult result;
				result.info.pCyclist = pCyclist;
				result.info.nTrail = nPoint;
				result.info.nSegment = j;
				result.info.vCollisionPoint = vHit;
				result.fDist = fHitDist;

				if (result.info.pCyclist == m_pCyclist)
				{
					nResult = AILC_HIT_OWNTRAIL;
				}
				else
				{
					nResult = (result.info.pCyclist == GetPlayerCyclist()) ? AILC_HIT_PLAYERTRAIL : AILC_HIT_AITRAIL;
				}

				*pResult = result;
				return nResult;
			}
		}
	}

	LTVector vNormal;
	if (IntersectLightCycleVolumes(vStart, vEnd, &fHitDist, &vHit, &vNormal))
	{
		pResult->info.vCollisionPoint = vHit;
		pResult->info.nSegment = 0;
		pResult->info.pCyclist = LTNULL;
		pResult->info.nTrail = 0;
		pResult->fDist = fHitDist;
		return AILC_HIT_VOLUME;
	}

	if (IsPointInLightCycleVolume(vStart))
	{
		nResult = AILC_HIT_OUTOFBOUNDS;
		g_pLightCycleMgr->DerezCyclist(m_pCyclist, LTNULL, true);
	}

	return nResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::ProbeAll
//
//	PURPOSE:	Look fDist ahead in direction nDir; all the hits, nearest
//				first
//
// ----------------------------------------------------------------------- //

static bool CompareProbeResults(AILCProbeResult a, AILCProbeResult b);

int AILightCycle::ProbeAll(int nDir, AILCProbeResultList* pResults, float fDist)
{
	int nCount = 0;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	MovePoint(vPos, nDir, 1.0f);
	LTVector vStart = vPos;

	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	MovePoint(vPos, nDir, fDist);
	LTVector vEnd = vPos;

	float fHitDist;
	LTVector vHit;
	int nPoint;

	uint32 nNumCyclists = g_pLightCycleMgr->m_collCyclists.size();
	for (uint32 i = 0; i < nNumCyclists; i++)
	{
		LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->m_collCyclists[i];
		if (!pCyclist || !pCyclist->collTrails.size()) continue;

		uint32 nNumTrails = pCyclist->collTrails.size();
		for (uint32 j = 0; j < nNumTrails; j++)
		{
			// RETAIL: always the first trail too (0x100031d0)
			if (FindTrailHit(vStart, vEnd, pCyclist->collTrails[0], &fHitDist, &vHit, &nPoint))
			{
				AILCProbeResult result;
				result.info.pCyclist = pCyclist;
				result.info.nTrail = 0;
				result.info.nSegment = j;
				result.info.vCollisionPoint = vHit;
				result.fDist = fHitDist;
				pResults->push_back(result);
				nCount++;
			}
		}
	}

	LTVector vNormal;
	if (IntersectLightCycleVolumes(vStart, vEnd, &fHitDist, &vHit, &vNormal))
	{
		AILCProbeResult result;
		result.info.pCyclist = LTNULL;
		result.info.nTrail = 0;
		result.info.nSegment = 0;
		result.info.vCollisionPoint = vHit;
		result.fDist = fHitDist;
		pResults->push_back(result);
		nCount++;
	}

	std::sort(pResults->begin(), pResults->end(), CompareProbeResults);

	return nCount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetPlayerAhead
//
//	PURPOSE:	Where the player is going to be, if we can get there
//
// ----------------------------------------------------------------------- //

bool AILightCycle::GetPlayerAhead(float* pfDist, LTVector* pvPos)
{
	if (!GetPlayerCyclist())
	{
		pvPos->Init();
		return false;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	vPos = vPos + m_pCyclist->vForward * 10.0f;

	LTVector vPlayerPos = GetPlayerPos() + GetPlayerCyclist()->vForward * 10.0f;

	if (CheckPath(vPos, vPlayerPos, pfDist, pvPos))
	{
		return false;
	}

	*pfDist = (vPlayerPos - vPos).Mag();
	*pvPos = vPlayerPos;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::CheckPath
//
//	PURPOSE:	Is vStart..vEnd blocked by a trail or a volume?
//
// ----------------------------------------------------------------------- //

bool AILightCycle::CheckPath(const LTVector & vStart, const LTVector & vEnd, float* pfDist, LTVector* pvHit)
{
	*pfDist = (float)HUGE_VAL;
	*pvHit = vEnd;

	float fHitDist;
	LTVector vHit;
	int nPoint;

	uint32 nNumCyclists = g_pLightCycleMgr->m_collCyclists.size();
	for (uint32 i = 0; i < nNumCyclists; i++)
	{
		LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->m_collCyclists[i];
		if (!pCyclist || !pCyclist->collTrails.size()) continue;

		uint32 nNumTrails = pCyclist->collTrails.size();
		for (uint32 j = 0; j < nNumTrails; j++)
		{
			if (FindTrailHit(vStart, vEnd, pCyclist->collTrails[j], &fHitDist, &vHit, &nPoint))
			{
				if (fHitDist < *pfDist)
				{
					*pfDist = fHitDist;
					*pvHit = vHit;
				}
			}
		}
	}

	LTVector vNormal;
	if (IntersectLightCycleVolumes(vStart, vEnd, &fHitDist, &vHit, &vNormal))
	{
		if (fHitDist < *pfDist)
		{
			*pfDist = fHitDist;
			*pvHit = vHit;
		}
	}

	return (HUGE_VAL != *pfDist);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetTurnPoint
//
//	PURPOSE:	Where we turn toward vTarget
//
// ----------------------------------------------------------------------- //

void AILightCycle::GetTurnPoint(int nTurn, const LTVector & vTarget, LTVector & vPoint)
{
	LTVector vDir;
	switch (nTurn)
	{
		case kAILCTurn_Forward:	vDir = GetForward(); break;
		case kAILCTurn_Right:	vDir = GetRight(); break;
		case kAILCTurn_Back:	vDir = GetBack(); break;
		case kAILCTurn_Left:	vDir = GetLeft(); break;
		default:				vDir = LTVector(0.0f, 0.0f, 0.0f); break;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	vPoint = vPos;

	if (vDir.x != 0.0f) vPoint.x = vTarget.x;
	if (vDir.z != 0.0f) vPoint.z = vTarget.z;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::IsToLeft
//
//	PURPOSE:	Is vPos to our left?
//
// ----------------------------------------------------------------------- //

bool AILightCycle::IsToLeft(LTVector vPos)
{
	LTVector vMyPos;
	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
	LTVector vDiff = vPos - vMyPos;
	return (GetLeft().Dot(vDiff) > 0.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::IsInFront
//
//	PURPOSE:	Is vPos ahead of us?
//
// ----------------------------------------------------------------------- //

bool AILightCycle::IsInFront(const LTVector & vPos)
{
	LTVector vMyPos;
	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
	LTVector vDiff = vPos - vMyPos;
	return (GetForward().Dot(vDiff) > 0.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::FindPowerup
//
//	PURPOSE:	Go for the nearest power-up ahead, unless the player is
//				nearer and coming our way
//
// ----------------------------------------------------------------------- //

bool AILightCycle::FindPowerup()
{
	if (GetPowerup() != LC_NO_POWERUP || m_pPowerup) return false;

	LightCyclePowerup* pPowerup = g_pServerLightCycleMgr->FindNearestPowerup(m_pCyclist->hObject, false);
	if (!pPowerup) return false;

	if ((LIGHT_CYCLIST*)pPowerup->m_nTronUnknown5C != m_pCyclist && pPowerup->m_nTronUnknown5C) return false;

	LTVector vPowerupPos;
	g_pLTServer->GetObjectPos(pPowerup->m_hObject, &vPowerupPos);
	if (!IsInFront(vPowerupPos)) return false;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	MovePoint(vPos, m_nDirection, 1.0f);
	LTVector vStart = vPos;

	float fDist;
	LTVector vHit;
	if (CheckPath(vStart, vPowerupPos, &fDist, &vHit)) return false;

	float fPlayerDist;
	if (GetPlayerAhead(&fPlayerDist, &vPos))
	{
		if (fPlayerDist < 300.0f) return false;

		if (fPlayerDist < fDist && IsInFront(vPos))
		{
			int nPlayerDir = GetPlayerDirection();
			if (IsToLeft(vPos))
			{
				if (nPlayerDir == TurnRight(m_nDirection)) return false;
			}
			else
			{
				if (nPlayerDir == TurnLeft(m_nDirection)) return false;
			}
		}
	}

	m_pPowerup = pPowerup;
	pPowerup->m_nTronUnknown5C = (uint32)m_pCyclist;
	ReplanPowerupPath();
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::PlanPath
//
//	PURPOSE:	A path to vTarget: straight, or turning once
//
// ----------------------------------------------------------------------- //

bool AILightCycle::PlanPath(int nDir, const LTVector & vTarget)
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	MovePoint(vPos, nDir, 1.0f);
	LTVector vStart = vPos;

	LTVector vTurn;
	GetTurnPoint(kAILCTurn_Forward, vTarget, vTurn);

	float fDist;
	LTVector vHit;

	if ((vTarget - vTurn).Mag() < 23.5)
	{
		m_lstPath.push_back(vTarget);
	}
	else
	{
		if (CheckPath(vStart, vTurn, &fDist, &vHit)) goto Failed;
		m_lstPath.push_back(vTurn);

		if (CheckPath(vTurn, vTarget, &fDist, &vHit)) goto Failed;
		m_lstPath.push_back(vTarget);
	}

	SetBehavior(kAILCBehavior_Powerup);
	return true;

Failed:
	if (m_pPowerup)
	{
		m_pPowerup->m_nTronUnknown5C = 0;
	}
	m_lstPath.clear();
	m_pPowerup = LTNULL;
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::ReplanPowerupPath
//
//	PURPOSE:	A new path to our power-up
//
// ----------------------------------------------------------------------- //

bool AILightCycle::ReplanPowerupPath()
{
	if (!m_pPowerup) return false;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_pPowerup->m_hObject, &vPos);

	m_lstPath.clear();

	if (!IsInFront(vPos)) return false;

	return PlanPath(m_nDirection, vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::DropPowerup
//
//	PURPOSE:	Stop going for our power-up
//
// ----------------------------------------------------------------------- //

void AILightCycle::DropPowerup()
{
	if (m_pPowerup)
	{
		m_pPowerup->m_nTronUnknown5C = 0;
		m_pPowerup = LTNULL;
		m_lstPath.clear();
		SetBehavior(kAILCBehavior_Wander);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::IsPathClear
//
//	PURPOSE:	Is our path still open?
//
// ----------------------------------------------------------------------- //

bool AILightCycle::IsPathClear()
{
	if (m_lstPath.empty()) return false;

	std::list<LTVector>::iterator it = m_lstPath.begin();

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	MovePoint(vPos, m_nDirection, 1.0f);

	LTVector vStart = vPos;
	LTVector vEnd = *m_lstPath.begin();

	float fDist;
	LTVector vHit;
	while (it != m_lstPath.end())
	{
		if (CheckPath(vStart, vEnd, &fDist, &vHit)) return false;

		vStart = vEnd;
		it++;
		vEnd = *it;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::UpdateGoForward
//
//	PURPOSE:	The GoForward state: straight on
//
// ----------------------------------------------------------------------- //

void AILightCycle::UpdateGoForward()
{
	g_pLTServer->SetObjectRotation(m_hObject, &s_arDirections[m_nDirection]);

	g_pLightCycleMgr->UpdateCyclistMovement(m_hObject, m_dwMoveFlags | AILC_MOVE_GO);

	m_bMoved = true;
	SetNextUpdate(m_fUpdateDelta);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetDistSinceTurn
//
//	PURPOSE:	How far we've gone since the last trail point
//
// ----------------------------------------------------------------------- //

float AILightCycle::GetDistSinceTurn()
{
	if (m_pCyclist->collTrails.size() == 0) return 0.0f;

	LIGHT_CYCLE_TRAIL* pTrail = m_pCyclist->pCurTrail;
	if (!pTrail) return 0.0f;

	uint32 nNumPoints = pTrail->collTrailPoints.size();
	if (nNumPoints == 0) return 0.0f;
	if (nNumPoints < 2) return 0.0f;

	LTVector vPoint = pTrail->collTrailPoints.at(nNumPoints - 2).vPoint;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	return (vPos - vPoint).Mag();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::UsePowerup
//
//	PURPOSE:	Fire our power-up
//
// ----------------------------------------------------------------------- //

void AILightCycle::UsePowerup()
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	LTVector vActivatePos = vPos;
	g_pServerLightCycleMgr->ActivatePowerup(m_pCyclist, &vActivatePos);

	m_TronUnknown108.fDuration = 0.0f;
	m_TronUnknown108.fTime = 0.0f;
	m_TronUnknown108.bOn = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::Update
//
//	PURPOSE:	Update the AI
//
// ----------------------------------------------------------------------- //

void AILightCycle::Update()
{
	if (m_bWaitingForClient)
	{
		SetNextUpdate(UPDATE_NEVER);
		return;
	}

	m_bMoved = false;

	if (m_bFirstUpdate)
	{
		Start();
	}

	(this->*m_pfnState)();

	m_dwMoveFlags = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AILightCycle::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	pMsg->Writebool(m_bScripted);
	pMsg->Writebool(m_bPaused);
	pMsg->WriteString(m_sCrashCmd.c_str());
	pMsg->WriteString(m_sRemoveCmd.c_str());
	pMsg->Writeuint8(m_eBehavior);
	pMsg->Writeuint32(m_nNameId);

	LightCycle::Save(pMsg, dwSaveFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AILightCycle::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	m_bScripted = pMsg->Readbool();
	m_bPaused = pMsg->Readbool();

	char szString[256];
	pMsg->ReadString(szString, ARRAY_LEN(szString));
	m_sCrashCmd = szString;
	pMsg->ReadString(szString, ARRAY_LEN(szString));
	m_sRemoveCmd = szString;

	m_eBehavior = pMsg->Readuint8();
	m_nNameId = pMsg->Readuint32();

	LightCycle::Load(pMsg, dwLoadFlags);

	InitialUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::IsPlayerDead
//
//	PURPOSE:	Has the player no cycle?
//
// ----------------------------------------------------------------------- //

bool AILightCycle::IsPlayerDead()
{
	return (GetPlayerCyclist() == LTNULL) ? true : false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetPlayerCyclist
//
//	PURPOSE:	The player's cyclist
//
// ----------------------------------------------------------------------- //

LIGHT_CYCLIST* AILightCycle::GetPlayerCyclist()
{
	return g_pServerLightCycleMgr->GetPlayerCyclist();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetPlayerPos
//
//	PURPOSE:	Where the (first) player is
//
// ----------------------------------------------------------------------- //

LTVector AILightCycle::GetPlayerPos()
{
	LTVector vPos(0.0f, 0.0f, 0.0f);

	if (CPlayerObj::GetPlayerObjList().size() > 0)
	{
		CPlayerObj* pPlayer = CPlayerObj::GetPlayerObjList()[0];
		g_pLTServer->GetObjectPos(pPlayer->m_hObject, &vPos);
	}

	return vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::IsTrailTight
//
//	PURPOSE:	Is the last segment of our trail shorter than the one
//				before (and that one not too long)?
//
// ----------------------------------------------------------------------- //

bool AILightCycle::IsTrailTight()
{
	if (!m_pCyclist) return false;

	LIGHT_CYCLE_TRAIL* pTrail = m_pCyclist->pCurTrail;
	if (!pTrail) return false;

	uint32 nNumPoints = pTrail->collTrailPoints.size();
	if (nNumPoints < 5) return false;

	float fPrevLen = (pTrail->collTrailPoints[nNumPoints - 4].vPoint - pTrail->collTrailPoints[nNumPoints - 3].vPoint).Mag();
	LTVector vLast = pTrail->collTrailPoints[nNumPoints - 2].vPoint - pTrail->collTrailPoints[nNumPoints - 1].vPoint;

	if (fPrevLen > 15000.0f) return false;

	return (vLast.Mag() < fPrevLen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::SetBehavior
//
//	PURPOSE:	Change behavior; returns the old one
//
// ----------------------------------------------------------------------- //

int AILightCycle::SetBehavior(int eBehavior)
{
	int eOldBehavior = m_eBehavior;

	if (eOldBehavior == kAILCBehavior_Powerup && eBehavior != kAILCBehavior_Powerup)
	{
		if (!m_lstPath.empty())
		{
			m_lstPath.clear();
		}

		if (m_pPowerup)
		{
			m_pPowerup->m_nTronUnknown5C = 0;
		}
		m_pPowerup = LTNULL;
	}

	m_eBehavior = eBehavior;
	return eOldBehavior;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetHitType
//
//	PURPOSE:	The AILC_HIT_ flag of a collision
//
// ----------------------------------------------------------------------- //

int AILightCycle::GetHitType(LightCycleCollisionInfo* pInfo)
{
	if (!pInfo) return AILC_HIT_NONE;

	if (pInfo->pCyclist == m_pCyclist) return AILC_HIT_OWNTRAIL;
	if (pInfo->pCyclist == GetPlayerCyclist()) return AILC_HIT_PLAYERTRAIL;

	return pInfo->pCyclist ? AILC_HIT_AITRAIL : AILC_HIT_VOLUME;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::GetPowerup
//
//	PURPOSE:	The power-up we have
//
// ----------------------------------------------------------------------- //

int AILightCycle::GetPowerup()
{
	return m_pCyclist->nPowerup;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AILightCycle::HasFlag
//
//	PURPOSE:	Is a flag of our cyclist on?
//
// ----------------------------------------------------------------------- //

bool AILightCycle::HasFlag(uint32 nFlag)
{
	return (m_pCyclist->nFlags & nFlag) != 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CompareProbeResults
//
//	PURPOSE:	Sorts probe results nearest first. RETAIL: 0x100073b0
//
// ----------------------------------------------------------------------- //

static bool CompareProbeResults(AILCProbeResult a, AILCProbeResult b)
{
	return a.fDist < b.fDist;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsBetween
//
//	PURPOSE:	Is p between a and b (on x, or z if a and b share x)?
//				RETAIL: 0x100079d0
//
// ----------------------------------------------------------------------- //

static int IsBetween(LTVector p, LTVector a, LTVector b)
{
	if (a.x != b.x)
	{
		if (a.x <= p.x && p.x <= b.x) return 1;
		if (a.x >= p.x && p.x >= b.x) return 1;
		return 0;
	}

	if (a.z <= p.z && p.z <= b.z) return 1;
	if (a.z >= p.z && p.z >= b.z) return 1;
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IntersectSegments
//
//	PURPOSE:	The 2D (x, z) intersection of p1..p2 and p3..p4
//
// ----------------------------------------------------------------------- //

int IntersectSegments(LTVector p1, LTVector p2, LTVector p3, LTVector p4, LTVector* pvHit, LTVector* pvHit2)
{
	LTVector a = p2 - p1;
	LTVector b = p4 - p3;
	LTVector c = p1 - p3;

	float fDenom = a.x * b.z - a.z * b.x;

	if (fabs(fDenom) < 1.0e-8f)
	{
		// Parallel: on the same line?
		if (a.x * c.z - a.z * c.x != 0.0f) return 0;
		if (c.z * b.x - b.z * c.x != 0.0f) return 0;

		float fLenA = a.MagSqr();
		float fLenB = b.MagSqr();

		if (fLenA == 0.0f)
		{
			if (fLenB == 0.0f)
			{
				if (p3 == p1)
				{
					*pvHit = p1;
					return 1;
				}
				return 0;
			}

			if (IsBetween(p1, p3, p4))
			{
				*pvHit = p1;
				return 1;
			}
			return 0;
		}

		if (fLenB == 0.0f)
		{
			if (IsBetween(p3, p1, p2))
			{
				*pvHit = p3;
				return 1;
			}
			return 0;
		}

		// Overlapping
		float t0, t1;
		float fX = p2.x - p3.x;
		float fZ = p2.z - p3.z;
		if (b.x != 0.0f)
		{
			t0 = c.x / b.x;
			t1 = fX / b.x;
		}
		else
		{
			t0 = c.z / b.z;
			t1 = fZ / b.z;
		}

		if (t0 > t1)
		{
			float fTemp = t0;
			t0 = t1;
			t1 = fTemp;
		}

		if (t0 > 1.0f) return 0;
		if (t1 < 0.0f) return 0;

		if (t0 < 0.0f) t0 = 0.0f;
		if (t1 > 1.0f) t1 = 1.0f;

		if (t0 == t1)
		{
			*pvHit = p3 + b * t0;
			return 1;
		}

		*pvHit = p3 + b * t0;
		*pvHit2 = p3 + b * t1;
		return 2;
	}

	float s = (c.z * b.x - b.z * c.x) / fDenom;
	if (s < 0.0f || s > 1.0f) return 0;

	float t = (a.x * c.z - a.z * c.x) / fDenom;
	if (t < 0.0f || t > 1.0f) return 0;

	*pvHit = p1 + a * s;
	return 1;
}
