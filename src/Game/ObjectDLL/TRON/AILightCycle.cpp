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

inline const LTRotation & AILightCycle::GetDirectionRotation()
{
	return s_arDirections[m_nDirection];
}

// RETAIL: how much of the cycle's length the AI goes before turning again,
// by difficulty (0x102932b0). The name is ours.
static const float s_afDifficultyTurnScale[] = { 1.0f, 0.75f, 0.5f, 0.25f };

// RETAIL: how long the AI keeps a power-up before using it anyway, by
// power-up (0x102d67ac). The name is ours.
static float s_afPowerupHoldTime[] =
{
	15.0f,		// kLCPowerup_Shield
	15.0f,		// kLCPowerup_Turbo
	FLT_MAX,	// kLCPowerup_TrailSpike
	FLT_MAX,	// kLCPowerup_Missile
	15.0f,		// kLCPowerup_WallExtender
	15.0f,		// kLCPowerup_TurboCurse
	15.0f,		// kLCPowerup_Grab
	40.0f,		// kLCPowerup_TrailReset
	15.0f,		// kLCPowerup_ForceUse
	15.0f,		// kLCPowerup_Random
	15.0f,
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
static inline int DirRightOf(int nDir)
{
	nDir++;
	if (nDir > 3) nDir = 0;
	return nDir;
}

static inline int DirLeftOf(int nDir)
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

	LTVector vHit2;
	if (IntersectLightCycleVolumes(vStart, vEnd, &fHitDist, &vHit, &vHit2))
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

	LTVector vHit2;
	if (IntersectLightCycleVolumes(vStart, vEnd, &fHitDist, &vHit, &vHit2))
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

	LTVector vHit2;
	if (IntersectLightCycleVolumes(vStart, vEnd, &fHitDist, &vHit, &vHit2))
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
				if (nPlayerDir == DirRightOf(m_nDirection)) return false;
			}
			else
			{
				if (nPlayerDir == DirLeftOf(m_nDirection)) return false;
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
//	ROUTINE:	AILightCycle::UpdateAI
//
//	PURPOSE:	The AI state: look around, use the power-up, then go on
//				or turn as the behavior says
//
// ----------------------------------------------------------------------- //

void AILightCycle::UpdateAI()
{
	float fTurnScale = s_afDifficultyTurnScale[g_pServerLightCycleMgr->GetDifficulty()];
	float fSpeed = m_pCyclist->fSpeed;
	float fFrameDist = g_pLTServer->GetFrameTime() * fSpeed;

	bool bFrontBlocked = false;
	bool bPlayerAhead = false;
	bool bCanReachTarget = false;
	bool bStuck = false;

	float fHalfLength = m_fCycleLength * 0.5f;
	float fFarDist = m_pCyclist->fFrontSensorDist * 10.0f;
	float fFrontDist = fFrameDist * 2.0f;

	AILCProbeResult rRight, rLeft, rFront, rFarLeft, rFarRight;

	int nRight = Probe(DirRightOf(m_nDirection), &rRight, m_pCyclist->fRightSensorDist);
	int nLeft = Probe(DirLeftOf(m_nDirection), &rLeft, m_pCyclist->fLeftSensorDist);

	// RETAIL: Probe looks from beside the cycle, alternately left and
	// right, so a clear front is looked at twice
	int nFront = Probe(m_nDirection, &rFront, fFrontDist);
	if (!nFront)
	{
		nFront = Probe(m_nDirection, &rFront, fFrontDist);
	}

	int nFarLeft = Probe(DirLeftOf(m_nDirection), &rFarLeft, fFarDist);
	int nFarRight = Probe(DirRightOf(m_nDirection), &rFarRight, fFarDist);

	bool bPlayerDead = IsPlayerDead();
	float fDistSinceTurn = GetDistSinceTurn();

	// A new power-up: use it, at the latest, after its hold time
	if (GetPowerup() != LC_NO_POWERUP && m_nLastPowerup != GetPowerup())
	{
		m_TronUnknown108.fDuration = s_afPowerupHoldTime[GetPowerup()];
		m_TronUnknown108.fStartTime = g_pLTServer->GetTime();
		m_TronUnknown108.bOn = true;
	}

	// Not moved since the last update: stuck
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	if (m_vTronUnknown11C == vPos)
	{
		g_pLightCycleMgr->DerezCyclist(m_pCyclist, LTNULL, true);
	}

	LTVector vMoved = GetPos() - m_vTronUnknown11C;
	m_vTronUnknown11C = GetPos();
	m_nLastPowerup = GetPowerup();

	if (nFront)
	{
		bFrontBlocked = true;
	}

	float fPlayerDist;
	LTVector vPlayerPos;
	if (!bPlayerDead)
	{
		bPlayerAhead = GetPlayerAhead(&fPlayerDist, &vPlayerPos);
		if (bPlayerAhead)
		{
			m_vTronUnknown9C = vPlayerPos;
		}
		else
		{
			LTVector vAhead = GetPos() + m_pCyclist->vForward;
			if ((m_vTronUnknown9C - GetPos()).Mag() < m_pCyclist->fPlayerNoticeDist)
			{
				float fDist;
				LTVector vHit;
				bCanReachTarget = !CheckPath(vAhead, m_vTronUnknown9C, &fDist, &vHit);
			}
		}
	}

	if (nRight && nLeft)
	{
		g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_GoingBetweenWalls);
	}

	// Four turns the same way: stuck in a spiral?
	bool bAllLeft = (m_anTurns[0] == 0 && m_anTurns[1] == 0 && m_anTurns[2] == 0 && m_anTurns[3] == 0);
	if ((m_anTurns[0] == 1 && m_anTurns[1] == 1 && m_anTurns[2] == 1 && m_anTurns[3] == 1) || bAllLeft)
	{
		bStuck = IsTrailTight();
		if (bStuck)
		{
			g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_Stuck);
		}
	}

	if (m_eTronUnknown8C != kAILCBehavior_Aggressive && m_eBehavior == kAILCBehavior_Aggressive && bPlayerAhead)
	{
		g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_HuntingPlayer);
	}

	bool bNoTurn = false;
	bool bPlayerClose = true;
	if (m_pCyclist->fPlayerNoticeDist * 0.2f <= fPlayerDist)
	{
		bPlayerClose = false;
	}

	bool bPlayerHasPowerup = bPlayerDead ? false : (GetPlayerCyclist()->nPowerup != LC_NO_POWERUP);

	// Use the power-up?
	if (GetPowerup() != kLCPowerup_Shield && !HasFlag(1))
	{
		if (GetPowerup() == kLCPowerup_Missile)
		{
			AILCProbeResultList results;
			int nHits = ProbeAll(m_nDirection, &results, 10000.0f);
			if (nHits)
			{
				if (GetHitType(&results[0].info) == AILC_HIT_VOLUME)
				{
					bNoTurn = false;
				}
				else
				{
					bool bLeft;
					int nPlayerDir;
					float fTime = GetTimeToPoint(m_pCyclist, results[results.size() - 1].info.vCollisionPoint);
					if (bPlayerDead) goto FireMissile;
					if (!bPlayerAhead && bPlayerClose && nHits >= 1 && IsInFront(vPlayerPos))
					{
						bLeft = IsToLeft(vPlayerPos);
						nPlayerDir = GetPlayerDirection();
						if (nPlayerDir == (m_nDirection + 2) % 4) goto FireMissile;
						if (nPlayerDir == DirRightOf(m_nDirection) && bLeft) goto FireMissile;
						if (nPlayerDir == DirLeftOf(m_nDirection) && !bLeft)
						{
FireMissile:
							UsePowerup();
							AddTurn(0xff);
							m_TronUnknownF8.Start(fTime);
						}
					}
				}
			}
		}
		else if (GetPowerup() != kLCPowerup_Turbo && GetPowerup() != kLCPowerup_TurboCurse)
		{
			if (GetPowerup() == kLCPowerup_TrailSpike)
			{
				bool bPlayerSoon = (GetTimeToPoint(GetPlayerCyclist(), GetPos()) < 0.3f);
				bool bPlayerBehind = !IsInFront(vPlayerPos);
				bool bSameDir = (GetPlayerDirection() == m_nDirection);
				bool bBetweenWalls = (nLeft && nRight);

				if (m_eBehavior == kAILCBehavior_Aggressive)
				{
					m_eBehavior = kAILCBehavior_Wander;
				}

				if ((!bPlayerSoon && bPlayerClose && bPlayerBehind && bSameDir) || bBetweenWalls)
				{
					bNoTurn = true;
					UsePowerup();
					m_TronUnknownF8.Start(1.5f);
				}
			}
			else if (GetPowerup() == kLCPowerup_WallExtender)
			{
				UsePowerup();
			}
			else if (GetPowerup() == kLCPowerup_Grab || GetPowerup() == kLCPowerup_ForceUse)
			{
				if (bPlayerClose && (bPlayerHasPowerup & bPlayerAhead))
				{
					UsePowerup();
				}
			}
			else if (GetPowerup() == kLCPowerup_TrailReset)
			{
				if (bStuck && bPlayerClose)
				{
					UsePowerup();
				}
			}
		}
		else
		{
			float fTime = GetTimeToPoint(GetPlayerCyclist(), GetPos());
			bool bAvoiding = (m_eBehavior == kAILCBehavior_Avoid || m_eBehavior == kAILCBehavior_TronUnknown3);
			bool bPlayerSoon = (fTime < 5.0f);
			if (bAvoiding && bPlayerSoon && bPlayerClose)
			{
				UsePowerup();
			}

			if (m_eBehavior == kAILCBehavior_Aggressive && bPlayerAhead && bPlayerClose)
			{
				UsePowerup();
			}
		}
	}
	else
	{
		// The shield: on as soon as we have it; jump the wall ahead
		if (!HasFlag(1))
		{
			UsePowerup();
		}

		if (nFront && !(nFront & AILC_HIT_VOLUME))
		{
			float fTime = GetTimeToPoint(m_pCyclist, rFront.info.vCollisionPoint);
			bNoTurn = true;

			AILCProbeResultList results;
			int nHits = ProbeAll(m_nDirection, &results, 1000.0f);
			if (!nHits)
			{
				bNoTurn = false;
			}
			else
			{
				if (nHits > 1)
				{
					if ((results[1].info.vCollisionPoint - results[0].info.vCollisionPoint).Mag() < 228.0f)
					{
						bNoTurn = false;
					}

					if (GetHitType(&results[1].info) == AILC_HIT_VOLUME)
					{
						bNoTurn = false;
					}

					fTime = GetTimeToPoint(m_pCyclist, results[0].info.vCollisionPoint);
				}

				if (bNoTurn)
				{
					AddTurn(0xff);
					m_TronUnknownF8.Start(fTime);
				}
			}
		}
	}

	// Held the power-up long enough: use it
	if (m_TronUnknown108.bOn && m_TronUnknown108.IsTimedOut() && GetPowerup() != LC_NO_POWERUP)
	{
		UsePowerup();
		m_TronUnknown108.Stop();
	}

	// Not gone far enough since the last turn, or riding out a jump: on
	if (fTurnScale * fHalfLength > fDistSinceTurn || !m_TronUnknownF8.IsTimedOut())
	{
		if (!bFrontBlocked)
		{
			KeepGoing();
			SetNextUpdate(m_fUpdateDelta);
			return;
		}
	}

	if (nLeft && nFront)
	{
		// Boxed in on the left
		TurnRight();
		SetBehavior(kAILCBehavior_Wander);
	}
	else if (nRight)
	{
		if (nFront)
		{
			// Boxed in on the right
			TurnLeft();
			SetBehavior(kAILCBehavior_Wander);
		}
		else
		{
			goto FrontClear;
		}
	}
	else if (nFront)
	{
		// A wall ahead: score the turns
		int nLeftScore = 0;
		int nRightScore = 0;

		if (m_eBehavior == kAILCBehavior_Aggressive)
		{
			if (bPlayerAhead)
			{
				bFrontBlocked = IsToLeft(vPlayerPos);
				bool bInFront = IsInFront(vPlayerPos);
				int nPlayerDir = GetPlayerDirection();

				if (bInFront && m_fCycleLength * 4.0f > fPlayerDist)
				{
					g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_PlayerCloseInFront);
				}

				if (bFrontBlocked)
				{
					nLeftScore = 5;
					if (nPlayerDir != DirRightOf(m_nDirection))
					{
						nLeftScore += 5;
					}
				}
				else
				{
					nRightScore = 5;
				}

				if (nPlayerDir == DirRightOf(m_nDirection))
				{
					nRightScore += 5;
				}
			}
			else if (bCanReachTarget && !bPlayerDead)
			{
				bool bLeft = IsToLeft(m_vTronUnknown9C);
				if (IsInFront(m_vTronUnknown9C))
				{
					if (bLeft)
					{
						nLeftScore = 10;
					}
					else
					{
						nRightScore = 10;
					}
				}
			}

			g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_StartWallFollow);
		}

		// The nearer a wall on a side, the less we turn to it
		if (nFarRight)
		{
			LTVector vDiff(rFarRight.info.vCollisionPoint.x - GetPos().x,
				rFarRight.info.vCollisionPoint.y - GetPos().y,
				rFarRight.info.vCollisionPoint.z - GetPos().z);
			float fRatio = vDiff.Length() / fFarDist;
			nLeftScore += (int)((1.0f - fRatio * fRatio) * 20.0f);
		}

		if (nFarLeft)
		{
			LTVector vDiff(rFarLeft.info.vCollisionPoint.x - GetPos().x,
				rFarLeft.info.vCollisionPoint.y - GetPos().y,
				rFarLeft.info.vCollisionPoint.z - GetPos().z);
			float fRatio = vDiff.Length() / fFarDist;
			nRightScore += (int)((1.0f - fRatio * fRatio) * 20.0f);
		}

		// Turn the other way from the last time, if it's open
		if (!nFarRight && m_anTurns[0] == 0)
		{
			nRightScore += 10;
		}

		if (!nFarLeft && m_anTurns[0] == 1)
		{
			nLeftScore += 10;
		}

		if (!bNoTurn)
		{
			if (nRightScore > nLeftScore)
			{
				TurnRight();
			}
			else
			{
				TurnLeft();
			}
		}
	}
	else
	{
FrontClear:
		if ((nRight & AILC_HIT_PLAYERTRAIL) && (nLeft & AILC_HIT_PLAYERTRAIL))
		{
			// Between the player's walls
			SetBehavior(kAILCBehavior_Wander);
		}
		else if (nRight & AILC_HIT_PLAYERTRAIL)
		{
			// Follow the player's wall on our right
			m_nTronUnknown80 = 1;
			m_vStartPos = GetPos();
			SetBehavior(kAILCBehavior_TronUnknown3);

			if (bPlayerAhead)
			{
				if (IsToLeft(vPlayerPos))
				{
					m_nTronUnknown80 = -1;
					m_eBehavior = kAILCBehavior_Aggressive;
					SetBehavior(kAILCBehavior_Aggressive);
					TurnLeft();
				}

				g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_WallFollowingPlayerInSight);
			}

			if (m_bTronUnknown70)
			{
				g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_PlayerTurnsAway);
			}
		}
		else if (nLeft & AILC_HIT_PLAYERTRAIL)
		{
			// Follow the player's wall on our left
			m_nTronUnknown80 = 3;
			m_vStartPos = GetPos();
			SetBehavior(kAILCBehavior_TronUnknown3);

			if (bPlayerAhead)
			{
				if (!IsToLeft(vPlayerPos))
				{
					m_nTronUnknown80 = -1;
					SetBehavior(kAILCBehavior_Aggressive);
					TurnRight();
				}

				g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_WallFollowingPlayerInSight);
			}
		}
		else if (m_eBehavior == kAILCBehavior_Aggressive)
		{
			if (bPlayerAhead)
			{
				// Head the player off
				GetRandomUnit();

				float fPlayerSpeed = GetPlayerCyclist()->fSpeed;
				float fPlayerFrameDist = g_pLTServer->GetFrameTime() * fPlayerSpeed;
				float fLead = m_fCycleLength * 0.5f + fPlayerFrameDist;

				LTVector vTarget = vPlayerPos + GetPlayerCyclist()->vForward * fLead;
				int nDirTo = GetDirectionTo(vTarget);
				bool bInFront = IsInFront(vTarget);
				bFrontBlocked = IsToLeft(vTarget);

				g_pLTServer->GetObjectPos(m_hObject, &vPos);
				float fDist = (vPlayerPos - vPos).Mag();

				if (m_fCycleLength * 4.0f > fPlayerDist)
				{
					if (bInFront)
					{
						g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_PlayerCloseInFront);
					}
					else if (nDirTo == (m_nDirection + 2) % 4)
					{
						g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_PlayerCloseBehind);
					}
				}

				int nPlayerDir = GetPlayerDirection();
				if (bInFront &&
					(nDirTo == DirRightOf(m_nDirection) || nDirTo == DirLeftOf(m_nDirection)) &&
					(nPlayerDir == DirLeftOf(m_nDirection) || nPlayerDir == DirRightOf(m_nDirection)))
				{
					// Crossing paths: who gets to the crossing first?
					LTVector vMyPos = GetPos();
					LTVector vMyAhead = vMyPos + m_pCyclist->vForward * fDist;
					LTVector vPlayer = vPlayerPos;
					LTVector vPlayerAhead = vPlayerPos + GetPlayerCyclist()->vForward * fDist;

					LTVector vHit, vHit2;
					if (IntersectSegments(vMyPos, vMyAhead, vPlayer, vPlayerAhead, &vHit, &vHit2) == 1)
					{
						float fMyTime = (vHit - vMyPos).Mag() / m_pCyclist->fSpeed;
						LTVector vPlayerDiff = vHit - vPlayer;
						float fPlayerTime = vPlayerDiff.Mag() / GetPlayerCyclist()->fSpeed;
						if (fPlayerTime > fMyTime) goto Done;
					}
				}

				if (bNoTurn) goto Done;

				float fClose = fFrontDist + fHalfLength;
				bool bRightClose = false;
				bool bLeftClose = false;
				if (nFarRight && rFarRight.fDist < fClose)
				{
					bRightClose = true;
				}
				if (nFarLeft && rFarLeft.fDist < fClose)
				{
					bLeftClose = true;
				}

				if (nDirTo == DirRightOf(m_nDirection) && !bRightClose)
				{
					TurnRight();
				}
				else if (nDirTo == DirLeftOf(m_nDirection) && !bLeftClose)
				{
					TurnLeft();
				}
				else if (nDirTo == (m_nDirection + 2) % 4)
				{
					// Behind us: turn toward its side, if open
					if (bFrontBlocked)
					{
						if (!nLeft) TurnLeft();
					}
					else
					{
						if (!nRight) TurnRight();
					}
				}
			}
			else if (!bPlayerDead && fPlayerDist <= m_pCyclist->fPlayerNoticeDist)
			{
				if (!bCanReachTarget)
				{
					SetBehavior(kAILCBehavior_Wander);
					goto Done;
				}

				// Lost sight of the player: go where we saw it last
				float fClose = (fTurnScale + 1.0f) * fFrameDist + fHalfLength;
				bool bRightClose = false;
				bFrontBlocked = false;
				if (nFarRight && rFarRight.fDist < fClose)
				{
					bRightClose = true;
				}
				if (nFarLeft && rFarLeft.fDist < fClose)
				{
					bFrontBlocked = true;
				}

				g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_LostPlayerStillChasing);

				LTVector vTarget = m_vTronUnknown9C;
				if (IsInFront(vTarget))
				{
					LTVector vTurn;
					GetTurnPoint(kAILCTurn_Forward, vTarget, vTurn);
					if (vTurn != vTarget)
					{
						vTarget = vTurn + m_pCyclist->vForward * fFrameDist;
					}
				}

				int nDirTo = GetDirectionTo(vTarget);
				if (nDirTo == DirRightOf(m_nDirection) && !bRightClose)
				{
					TurnRight();
				}
				else if (nDirTo == DirLeftOf(m_nDirection) && !bFrontBlocked)
				{
					TurnLeft();
				}
			}
			else
			{
				g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_LostPlayer);
				SetBehavior(kAILCBehavior_Wander);
			}
		}
		else if (m_eBehavior == kAILCBehavior_Avoid)
		{
			// Back the way we went along the wall
			if (m_nTronUnknown80 == 3)
			{
				TurnLeft();
				m_nTronUnknown80 = -1;
			}
			else if (m_nTronUnknown80 == 1)
			{
				TurnRight();
				m_nTronUnknown80 = -1;
			}
		}
		else if (m_eBehavior == kAILCBehavior_Wander)
		{
			if (bPlayerAhead && fPlayerDist < m_pCyclist->fPlayerNoticeDist)
			{
				g_pServerLightCycleMgr->PlayDialog(m_pCyclist, kLCDialog_HuntingPlayer);
				SetBehavior(kAILCBehavior_Aggressive);
			}
			else if (!bPlayerDead)
			{
				// Now and then, turn toward the player
				if (GetRandomUnit() < 0.05f)
				{
					LTVector vTarget = (!bPlayerAhead && bCanReachTarget) ? m_vTronUnknown9C : vPlayerPos;
					int nDirTo = GetDirectionTo(vTarget);
					if (nDirTo == DirRightOf(m_nDirection) && !nRight)
					{
						if (m_anTurns[0] != 1)
						{
							TurnRight();
						}
					}
					else if (nDirTo == DirLeftOf(m_nDirection) && !nLeft && m_anTurns[0] != 0)
					{
						TurnLeft();
					}
				}
			}
			else
			{
				// Now and then, turn at random
				float fRandom = GetRandomUnit();
				if (fRandom < 0.001f)
				{
					if (nLeft)
					{
						TurnRight();
					}
					else
					{
						TurnLeft();
					}
				}
				else if (fRandom > 0.999f)
				{
					if (nRight)
					{
						TurnLeft();
					}
					else
					{
						TurnRight();
					}
				}
			}
		}
		else if (m_eBehavior == kAILCBehavior_TronUnknown3)
		{
			// Followed the player's wall far enough: avoid, or hunt
			LTVector vMyPos;
			g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
			LTVector vDiff(vMyPos.x - m_vStartPos.x, vMyPos.y - m_vStartPos.y, vMyPos.z - m_vStartPos.z);
			if (vDiff.Length() > m_fCycleLength * 1.5)
			{
				SetBehavior(kAILCBehavior_Avoid);

				if (bPlayerAhead)
				{
					bool bLeft = IsToLeft(vPlayerPos);
					if (m_nTronUnknown80 == 3)
					{
						if (bLeft)
						{
							m_nTronUnknown80 = -1;
							SetBehavior(kAILCBehavior_Aggressive);
						}
					}
					else if (m_nTronUnknown80 == 1)
					{
						if (!bLeft)
						{
							m_nTronUnknown80 = -1;
							SetBehavior(kAILCBehavior_Aggressive);
						}
					}
				}
			}
		}
		else if (m_eBehavior == kAILCBehavior_Powerup)
		{
			if (IsPathClear())
			{
				LTVector vNext = m_lstPath.front();
				LTVector vMyPos;
				g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
				if ((vNext - vMyPos).Mag() <= fFrameDist)
				{
					if (m_lstPath.size() >= 2)
					{
						m_lstPath.erase(m_lstPath.begin());
						if (IsToLeft(m_lstPath.front()))
						{
							TurnLeft();
						}
						else
						{
							TurnRight();
						}
					}
					else
					{
						m_lstPath.erase(m_lstPath.begin());
					}
				}
			}
			else
			{
				DropPowerup();
			}
		}
	}

Done:
	if (GetPowerup() != LC_NO_POWERUP)
	{
		DropPowerup();
	}
	else
	{
		FindPowerup();
	}

	KeepGoing();
	SetNextUpdate(m_fUpdateDelta);

	m_eTronUnknown8C = m_eBehavior;
	m_bTronUnknown70 = bPlayerAhead;
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
