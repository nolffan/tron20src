// ----------------------------------------------------------------------- //
//
// MODULE  : Finder.cpp
//
// PURPOSE : The Finder: a scanner (a turret on a keyframer) that opens,
//           turns toward the enemies it sees, locks on to them and fires
//           at them, lingers looking around when it loses them, and
//           closes again
//
// RETAIL: Tron 2.0's, written from retail Object.lto (Finder.h). The
// functions are in retail's order.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Finder.h"
#include "ParsedMsg.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "CVarTrack.h"
#include "PropTypeMgr.h"
#include "CaselessString.h"
#include "Weapon.h"
#include "Weapons.h"
#include "WeaponMgr.h"
#include "WeaponFireInfo.h"
#include "Explosion.h"
#include "AIStimulusMgr.h"
#include "AISenseRecorderAbstract.h"
#include "RelationMgr.h"
#include "ObjectRelationMgr.h"
#include "DebugLineSystem.h"
#include <algorithm>
#include <functional>

// RETAIL: the update rate, the most the lingering turns (degrees), the
// commands and the animations: constants in retail's .rdata
// (0x1029391c..0x10293954); the names are ours
static const LTFLOAT s_fUpdateRate				= 0.001f;
static const LTFLOAT s_fMaxLingerDegreesToTurn	= 150.0f;
static const char* const s_szDrawScanVolume	= "DrawScanVolume";
static const char* const s_szScanInfinite	= "ScanInfinite";
static const char* const s_aszStateAnims[]	=
{
	"",
	"Idle",
	"Idle",
	"Scan",
	"Fire_2",
	"Seek",
	"Seek",
};
static const char* const s_szNoWeaponAnim	= "Alarm";
static const char* const s_szOpenAnim		= "Open";
static const char* const s_szCloseAnim		= "Close";
static const char* const s_szExplosionAmmo	= "FinderExplosionAmmo";

// RETAIL: the props, 0x10339578 (their initializer is 0x1001b040)
BEGIN_CLASS(Finder)
	ADD_STRINGPROP_FLAG(CommandOn, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CommandOff, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Type, "IC_Finder", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)
	ADD_VECTORPROP_VAL_FLAG(Dims, 24.0f, 53.0f, 24.0f, PF_HIDDEN | PF_DIMS | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(KeyframerName, "", PF_OBJECTLINK)
	ADD_REALPROP(TurnRate, 45.0f)
	ADD_REALPROP(SecondsToLock, 5.0f)
	ADD_REALPROP(SecondsToLinger, 5.0f)
	ADD_REALPROP(LingerMaxDegreesToTurn, 45.0f)
	ADD_STRINGPROP_FLAG(ClearAlarmCommand, "", PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(DeathCommand, "", PF_NOTIFYCHANGE)
	ADD_BOOLPROP(FakeDeath, LTTRUE)
	ADD_LONGINTPROP(DisturbanceAlarmLevel, 0)
	ADD_REALPROP(DisturbanceRadius, 0.0f)
	ADD_STRINGPROP_FLAG(DeathFX, "<none>", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Finder, CScanner, NULL, NULL, 0, FinderPlugin)

// RETAIL: the console variable that draws the scan volumes (0x10339538)
// and whether it is set up (0x103397f8); the names are ours
static CVarTrack	s_vtDrawScanVolume;
static LTBOOL		s_bDrawScanVolumeInitted;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitDrawScanVolume
//
//	PURPOSE:	Set up FinderDrawScanVolume (RETAIL: 0x1001b6c0)
//
// ----------------------------------------------------------------------- //

static void InitDrawScanVolume()
{
	if (!s_bDrawScanVolumeInitted)
	{
		if (g_pLTServer)
		{
			s_vtDrawScanVolume.Init(g_pLTServer, "FinderDrawScanVolume", LTNULL, 0.0f);
		}

		s_bDrawScanVolumeInitted = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::Finder / ~Finder
//
// ----------------------------------------------------------------------- //

Finder::Finder()
:	CScanner(),
	m_pWeapon(LTNULL),
	m_pWeapons(LTNULL),
	m_hstrWeapon(LTNULL),
	m_bHasWeapon(false),
	m_hstrKeyframerName(LTNULL),
	m_hstrClearAlarmCommand(LTNULL),
	m_hstrDeathCommand(LTNULL),
	m_bFakeDeath(false),
	m_eState(eStateNone),
	m_fTurnRate(0.0f),
	m_nLastTurnDir(0),
	m_fSecondsToLock(0.0f),
	m_fSecondsToLinger(0.0f),
	m_fLastUpdateTime(0.0f),
	m_fLingerMaxTurn(0.0f),
	m_vEnemyPos(0.0f, 0.0f, 0.0f),
	m_vLingerDir(0.0f, 0.0f, 0.0f),
	m_eLingerState(eLingerTurningLeft),
	m_fDisturbanceRadius(0.0f),
	m_nDisturbanceAlarmLevel(0),
	m_eAlarmStimulusID(kStimID_Unset),
	m_hstrDeathFX(LTNULL),
	m_vSearchDir(0.0f, 0.0f, 0.0f),
	m_bScanInfinite(false),
	m_vScanInfiniteDir(0.0f, 0.0f, 0.0f)
{
	m_dwSenses = kSense_SeeEnemy;

	InitDrawScanVolume();
}

Finder::~Finder()
{
	FREE_HSTRING(m_hstrWeapon);
	FREE_HSTRING(m_hstrKeyframerName);
	FREE_HSTRING(m_hstrClearAlarmCommand);
	FREE_HSTRING(m_hstrDeathCommand);
	FREE_HSTRING(m_hstrDeathFX);

	if (m_pWeapons)
	{
		debug_delete(m_pWeapons);
		m_pWeapons = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Finder::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = CScanner::EngineMessageFn(MID_PRECREATE, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = CScanner::EngineMessageFn(MID_INITIALUPDATE, pData, fData);

			// RETAIL: also when a saved game is loaded
			InitialUpdate();
			return dwRet;
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			AllObjectsCreated();
			return 1;
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return CScanner::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages: when it is hit (and not
//				already on to an enemy), turn toward where it was hit from
//
// ----------------------------------------------------------------------- //

uint32 Finder::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	pMsg->SeekTo(0);

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (IsDead()) return 0;

			uint32 dwPos = pMsg->Tell();
			uint32 dwRet = CScanner::ObjectMessageFn(hSender, pMsg);
			pMsg->SeekTo(dwPos);

			DamageStruct damage;
			damage.InitFromMessage(pMsg);

			if (IsDead())
			{
				HandleDeath();
			}
			else if (m_eState != eStateFocusing && m_eState != eStateLocked && !m_bScanInfinite)
			{
				m_vSearchDir = -damage.vDir;
				m_vSearchDir.y = 0.0f;
				m_vSearchDir.Normalize();
				SetState(eStateSearching);
			}

			return dwRet;
		}
		break;

		default : break;
	}

	return CScanner::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool Finder::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_DrawScanVolume(s_szDrawScanVolume);
	static CParsedMsg::CToken s_cTok_ScanInfinite(s_szScanInfinite);

	if (cMsg.GetArg(0) == s_cTok_DrawScanVolume)
	{
		LTFLOAT fDraw;
		if (cMsg.GetArgCount() == 1)
		{
			fDraw = (s_vtDrawScanVolume.GetFloat() == 0.0f) ? 1.0f : 0.0f;
		}
		else
		{
			fDraw = (atoi(cMsg.GetArg(1)) > 0) ? 1.0f : 0.0f;
		}

		s_vtDrawScanVolume.SetFloat(fDraw);
		return true;
	}
	else if (cMsg.GetArg(0) == s_cTok_ScanInfinite)
	{
		bool bScan = (cMsg.GetArgCount() == 1) ? true : (atoi(cMsg.GetArg(1)) > 0);
		if (bScan)
		{
			PauseKeyframer();

			LTRotation rRot;
			g_pLTServer->GetObjectRotation(m_hObject, &rRot);
			m_vScanInfiniteDir = rRot.Forward();
			m_bScanInfinite = true;
			SetState(eStateLingering);
		}
		else
		{
			m_LingerTimer.Start(0.0f);
			m_bScanInfinite = false;
			m_vScanInfiniteDir.Init();
		}
	}

	return CScanner::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::Save
//
// ----------------------------------------------------------------------- //

void Finder::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	// RETAIL bug: m_pWeapon is not checked; a Finder without a weapon
	// (NoWeapon, or a Type that is none of the four) crashes saving
	SAVE_DWORD(m_pWeapon->GetWeaponData()->nId);
	SAVE_HSTRING(m_hstrWeapon);
	SAVE_bool(m_bHasWeapon);
	SAVE_HOBJECT(m_hKeyframer);
	SAVE_HSTRING(m_hstrKeyframerName);
	SAVE_HSTRING(m_hstrClearAlarmCommand);
	SAVE_HSTRING(m_hstrDeathCommand);
	SAVE_bool(m_bFakeDeath);
	SAVE_BYTE(m_eState);
	SAVE_FLOAT(m_fTurnRate);
	SAVE_FLOAT(m_rStartRot.m_Quat[0]);
	SAVE_FLOAT(m_rStartRot.m_Quat[1]);
	SAVE_FLOAT(m_rStartRot.m_Quat[2]);
	SAVE_FLOAT(m_rStartRot.m_Quat[3]);
	SAVE_FLOAT(m_fSecondsToLock);
	SAVE_FLOAT(m_fSecondsToLinger);
	SAVE_TIME(m_fLastUpdateTime);
	m_LingerTimer.Save(pMsg);
	SAVE_FLOAT(m_fLingerMaxTurn);
	SAVE_VECTOR(m_vEnemyPos);
	SAVE_VECTOR(m_vLingerDir);
	SAVE_BYTE(m_eLingerState);
	SAVE_FLOAT(m_fDisturbanceRadius);
	SAVE_DWORD(m_nDisturbanceAlarmLevel);
	SAVE_DWORD(m_eAlarmStimulusID);
	SAVE_HSTRING(m_hstrDeathFX);

	SAVE_DWORD(m_lstTargets.size());
	for (FinderTargetList::iterator it = m_lstTargets.begin(); it != m_lstTargets.end(); ++it)
	{
		SAVE_HOBJECT(it->m_hObj);
		it->m_Timer.Save(pMsg);
	}

	SAVE_VECTOR(m_vSearchDir);
	SAVE_bool(m_bScanInfinite);
	SAVE_VECTOR(m_vScanInfiniteDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::Load
//
// ----------------------------------------------------------------------- //

void Finder::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	SetWeapon(pMsg->Readuint32());

	FREE_HSTRING(m_hstrWeapon);
	LOAD_HSTRING(m_hstrWeapon);
	LOAD_bool(m_bHasWeapon);
	LOAD_HOBJECT(m_hKeyframer);
	FREE_HSTRING(m_hstrKeyframerName);
	LOAD_HSTRING(m_hstrKeyframerName);
	FREE_HSTRING(m_hstrClearAlarmCommand);
	LOAD_HSTRING(m_hstrClearAlarmCommand);
	FREE_HSTRING(m_hstrDeathCommand);
	LOAD_HSTRING(m_hstrDeathCommand);
	LOAD_bool(m_bFakeDeath);
	LOAD_BYTE_CAST(m_eState, State);
	LOAD_FLOAT(m_fTurnRate);
	LOAD_FLOAT(m_rStartRot.m_Quat[0]);
	LOAD_FLOAT(m_rStartRot.m_Quat[1]);
	LOAD_FLOAT(m_rStartRot.m_Quat[2]);
	LOAD_FLOAT(m_rStartRot.m_Quat[3]);
	LOAD_FLOAT(m_fSecondsToLock);
	LOAD_FLOAT(m_fSecondsToLinger);
	LOAD_TIME(m_fLastUpdateTime);
	m_LingerTimer.Load(pMsg);
	LOAD_FLOAT(m_fLingerMaxTurn);
	LOAD_VECTOR(m_vEnemyPos);
	LOAD_VECTOR(m_vLingerDir);
	LOAD_BYTE_CAST(m_eLingerState, LingerState);
	LOAD_FLOAT(m_fDisturbanceRadius);
	LOAD_DWORD(m_nDisturbanceAlarmLevel);
	LOAD_DWORD_CAST(m_eAlarmStimulusID, EnumAIStimulusID);
	LOAD_HSTRING(m_hstrDeathFX);

	int nTargets = pMsg->Readuint32();
	for (int i = 0; i < nTargets; i++)
	{
		HOBJECT hObj = pMsg->ReadObject();
		CTimer timer;
		timer.Load(pMsg);
		m_lstTargets.push_back(FinderTarget(timer, hObj));
	}

	LOAD_VECTOR(m_vSearchDir);
	LOAD_bool(m_bScanInfinite);
	LOAD_VECTOR(m_vScanInfiniteDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

bool Finder::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return false;

	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("TurnRate", &genProp) == LT_OK)
	{
		m_fTurnRate = DegreesToRadians(genProp.m_Float);
	}

	if (g_pLTServer->GetPropGeneric("SecondsToLock", &genProp) == LT_OK)
	{
		m_fSecondsToLock = genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("SecondsToLinger", &genProp) == LT_OK)
	{
		m_fSecondsToLinger = genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("LingerMaxDegreesToTurn", &genProp) == LT_OK)
	{
		m_fLingerMaxTurn = DegreesToRadians(genProp.m_Float);

		LTFLOAT fMaxTurn = DegreesToRadians(s_fMaxLingerDegreesToTurn);
		if (m_fLingerMaxTurn > fMaxTurn)
		{
			m_fLingerMaxTurn = fMaxTurn;
		}
	}

	// The weapon comes with the Type

	if (g_pLTServer->GetPropGeneric("Type", &genProp) == LT_OK && genProp.m_String[0])
	{
		PROPTYPE* pPropType = g_pPropTypeMgr->GetPropType(genProp.m_String);
		if (pPropType && !pPropType->sFilename.empty())
		{
			if (m_hstrWeapon)
			{
				g_pLTServer->FreeString(m_hstrWeapon);
			}

			// RETAIL: Tron's PROPTYPE::sType compares without case (a
			// CaselessString, CaselessString.h); ours is an std::string
			const CaselessString& sType = (const CaselessString&)pPropType->sType;

			if (sType == "IC_Finder")
			{
				m_hstrWeapon = g_pLTServer->CreateString("FinderWeapon");
			}
			else if (sType == "IC_Finder_Internet")
			{
				m_hstrWeapon = g_pLTServer->CreateString("FinderWeaponBlue");
			}
			else if (sType == "IC_Finder_AlansPC")
			{
				m_hstrWeapon = g_pLTServer->CreateString("FinderWeaponGreen");
			}
			else if (sType == "IC_Finder_Fcon")
			{
				m_hstrWeapon = g_pLTServer->CreateString("FinderWeaponPurple");
			}
		}
	}

	if (g_pLTServer->GetPropGeneric("KeyframerName", &genProp) == LT_OK)
	{
		FREE_HSTRING(m_hstrKeyframerName);
		m_hstrKeyframerName = g_pLTServer->CreateString(genProp.m_String);
	}

	if (g_pLTServer->GetPropGeneric("ClearAlarmCommand", &genProp) == LT_OK)
	{
		FREE_HSTRING(m_hstrClearAlarmCommand);
		m_hstrClearAlarmCommand = g_pLTServer->CreateString(genProp.m_String);
	}

	if (g_pLTServer->GetPropGeneric("DeathCommand", &genProp) == LT_OK)
	{
		FREE_HSTRING(m_hstrDeathCommand);
		m_hstrDeathCommand = g_pLTServer->CreateString(genProp.m_String);
	}

	if (g_pLTServer->GetPropGeneric("FakeDeath", &genProp) == LT_OK)
	{
		m_bFakeDeath = genProp.m_Bool;
	}

	// RETAIL bug: DisturbanceAlarmLevel and DisturbanceRadius are not read:
	// the alarm stimulus always has level and radius 0

	if (g_pLTServer->GetPropGeneric("DeathFX", &genProp) == LT_OK)
	{
		FREE_HSTRING(m_hstrDeathFX);
		m_hstrDeathFX = g_pLTServer->CreateString(genProp.m_String);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::PostPropRead
//
// ----------------------------------------------------------------------- //

bool Finder::PostPropRead(ObjectCreateStruct *pStruct)
{
	return pStruct != LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::SetState
//
// ----------------------------------------------------------------------- //

void Finder::SetState(State eNewState)
{
	if (eNewState == m_eState) return;

	// Leave the state we are in

	switch (m_eState)
	{
		case eStateClosed:
		{
			// Closing turns back to this
			g_pLTServer->GetObjectRotation(m_hObject, &m_rStartRot);
		}
		break;

		case eStateLocked:
		{
			if (m_hstrClearAlarmCommand)
			{
				const char* pCmd = g_pLTServer->GetStringData(m_hstrClearAlarmCommand);
				if (g_pCmdMgr->IsValidCmd(pCmd))
				{
					g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
				}
			}

			if (m_eAlarmStimulusID)
			{
				g_pAIStimulusMgr->RemoveStimulus(m_eAlarmStimulusID);
				m_eAlarmStimulusID = kStimID_Unset;
			}
		}
		break;

		case eStateLingering:
		{
			m_LingerTimer.Stop();
			m_vLingerDir.Init();
			TronUnknownNoOp();
		}
		break;

		case eStateSearching:
		{
			m_vSearchDir.Init();
		}
		break;
	}

	m_eState = eNewState;

	// Enter the new one

	switch (m_eState)
	{
		case eStateClosing:
		{
			PlayAnimation(s_szCloseAnim);
		}
		break;

		case eStateFocusing:
		{
			PlayAnimation(s_szOpenAnim);
			m_FocusTimer.Start(GetFocusTime());
		}
		break;

		case eStateLocked:
		{
			if (m_hstrSpotCommand)
			{
				const char* pCmd = g_pLTServer->GetStringData(m_hstrSpotCommand);
				if (g_pCmdMgr->IsValidCmd(pCmd))
				{
					g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
				}
			}

			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			// RETAIL: stimulus type 0x400
			m_eAlarmStimulusID = g_pAIStimulusMgr->RegisterStimulus(kStim_EnemyDisturbanceVisible,
				m_nDisturbanceAlarmLevel, m_hObject, m_hLastDetectedEnemy, vPos, m_fDisturbanceRadius);

			TronUnknownNoOp();
		}
		break;

		case eStateLingering:
		{
			m_LingerTimer.Start(m_fSecondsToLinger);

			LTRotation rRot;
			g_pLTServer->GetObjectRotation(m_hObject, &rRot);
			m_vLingerDir = rRot.Forward();

			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			if (m_nLastTurnDir == 0)
			{
				m_eLingerState = eLingerTurningLeft;
			}
			else if (m_nLastTurnDir == 1)
			{
				m_eLingerState = eLingerTurningRight;
			}
		}
		break;

		case eStateSearching:
		{
			PauseKeyframer();
			PlayAnimation(s_szOpenAnim);
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::TronUnknownNoOp
//
// ----------------------------------------------------------------------- //

void Finder::TronUnknownNoOp()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateDetect
//
//	PURPOSE:	Checks to see if we can see anything
//
// ----------------------------------------------------------------------- //

CScanner::DetectState Finder::UpdateDetect()
{
	g_pLTServer->GetObjectPos(m_hObject, &m_vScannerPos);

	UpdateRelations();

	// If it has been too long since our last stimulation, clear stimulus.

	if (m_fStimulusTime + m_fSenseUpdateRate < g_pLTServer->GetTime())
	{
		m_hStimulus = LTNULL;
	}

	if (!m_hStimulus)
	{
		m_FocusTimer.Stop();
		SetLastDetectedEnemy(LTNULL);
		SetCanProcessDetection(LTTRUE);

		if (m_eState == eStateFocusing || m_eState == eStateLocked)
		{
			if (m_bScanInfinite)
			{
				m_vSearchDir = m_vScanInfiniteDir;
				SetState(eStateSearching);
			}
			else
			{
				SetState(eStateLingering);
			}
		}
		else if (m_eState != eStateClosing && m_eState != eStateClosed &&
			m_eState != eStateLingering && m_eState != eStateSearching)
		{
			if (m_bScanInfinite)
			{
				m_vSearchDir = m_vScanInfiniteDir;
				SetState(eStateSearching);
			}
			else
			{
				SetState(eStateClosing);
			}
		}

		return DS_CLEAR;
	}

	switch (m_eState)
	{
		case eStateClosing:
		case eStateClosed:
		case eStateLingering:
		{
			SetLastDetectedEnemy(m_hStimulus);
			PauseKeyframer();

			FinderTarget* pTarget = FindTarget(m_hStimulus);
			if (pTarget)
			{
				// RETAIL bug: a time, not a duration (see AddTarget)
				pTarget->m_Timer.Start(g_pLTServer->GetTime() + 10.0f);
				SetState(eStateLocked);
				return DS_DETECTED;
			}

			SetState(eStateFocusing);
			return DS_DETECTED;
		}
		break;

		case eStateFocusing:
		{
			if (FindTarget(m_hStimulus))
			{
				SetState(eStateLocked);
				return DS_DETECTED;
			}

			if (m_hStimulus != m_hLastDetectedEnemy)
			{
				SetLastDetectedEnemy(m_hStimulus);
				SetState(eStateFocusing);
				return DS_DETECTED;
			}

			if (m_FocusTimer.Stopped())
			{
				AddTarget(m_hStimulus, 10.0f);
				SetState(eStateLocked);
				SetLastDetectedEnemy(m_hStimulus);
			}

			return DS_DETECTED;
		}
		break;

		case eStateLocked:
		{
			FinderTarget* pTarget = FindTarget(m_hStimulus);
			if (pTarget)
			{
				pTarget->m_Timer.Start(g_pLTServer->GetTime() + 10.0f);
			}

			if (m_hStimulus == m_hLastDetectedEnemy)
			{
				return DS_DETECTED;
			}

			SetLastDetectedEnemy(m_hStimulus);
			SetState(eStateFocusing);
		}
		break;
	}

	return DS_CLEAR;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::Restart
//
//	PURPOSE:	Start over, after a fake death (RETAIL: 0x1001d600)
//
// ----------------------------------------------------------------------- //

void Finder::Restart()
{
	EngineMessageFn(MID_INITIALUPDATE, LTNULL, 0.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::InitialUpdate
//
// ----------------------------------------------------------------------- //

bool Finder::InitialUpdate()
{
	EnableSensing(LTTRUE);
	SetState(eStateClosed);
	SetNextUpdate(s_fUpdateRate);

	TronUnknownNoOp();
	TronUnknownNoOp();

	PlayScanAnimation();
	SetLastDetectedEnemy(LTNULL);
	m_hStimulus = LTNULL;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::AllObjectsCreated
//
//	PURPOSE:	Get the weapon of the Type and the keyframer
//
// ----------------------------------------------------------------------- //

bool Finder::AllObjectsCreated()
{
	if (m_hstrWeapon)
	{
		WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon(g_pLTServer->GetStringData(m_hstrWeapon));
		if (_stricmp(pWeapon->szName, "NoWeapon"))
		{
			m_bHasWeapon = true;
			SetWeapon(pWeapon->nId);
		}

		g_pLTServer->FreeString(m_hstrWeapon);
		m_hstrWeapon = LTNULL;
	}

	if (m_hstrKeyframerName)
	{
		ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects(g_pLTServer->GetStringData(m_hstrKeyframerName), objArray);

		HCLASS hKeyframer = g_pLTServer->GetClass("KeyFramer");

		for (uint32 i = 0; i < objArray.NumObjects(); i++)
		{
			LTObjRef hObj;
			hObj = objArray.GetObject(i);

			if (hObj && g_pLTServer->GetObjectClass(hObj) == hKeyframer)
			{
				m_hKeyframer = hObj;
				break;
			}
		}

		g_pLTServer->FreeString(m_hstrKeyframerName);
		m_hstrKeyframerName = LTNULL;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::Update
//
// ----------------------------------------------------------------------- //

bool Finder::Update()
{
	UpdateTargets();
	UpdateDetect();

	if (m_hLastDetectedEnemy)
	{
		g_pLTServer->GetObjectPos(m_hLastDetectedEnemy, &m_vEnemyPos);
	}
	else
	{
		m_vEnemyPos.Init();
	}

	switch (m_eState)
	{
		case eStateClosing:		UpdateClosing();	break;
		case eStateClosed:		UpdateClosed();		break;
		case eStateFocusing:	UpdateFocusing();	break;
		case eStateLocked:		UpdateLocked();		break;
		case eStateLingering:	UpdateLingering();	break;
		case eStateSearching:	UpdateSearching();	break;
	}

	UpdateAnimation();

	if (IsDead())
	{
		HandleDeath();
	}

	UpdateScanVolume();

	SetNextUpdate(s_fUpdateRate);
	m_fLastUpdateTime = g_pLTServer->GetTime();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateClosing
//
//	PURPOSE:	Turn back to where it was before it opened, then close
//
// ----------------------------------------------------------------------- //

bool Finder::UpdateClosing()
{
	if (TurnTo(m_rStartRot.Forward()) < 1e-11f)
	{
		ResumeKeyframer();
		SetState(eStateClosed);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateClosed
//
//	PURPOSE:	RETAIL: folded with all the other "return true" ones
//				(0x1000a4d0)
//
// ----------------------------------------------------------------------- //

bool Finder::UpdateClosed()
{
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateFocusing
//
//	PURPOSE:	Turn toward the enemy
//
// ----------------------------------------------------------------------- //

bool Finder::UpdateFocusing()
{
	LTVector vPos, vEnemyPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectPos(m_hLastDetectedEnemy, &vEnemyPos);

	LTVector vDir = vEnemyPos - vPos;
	vDir.Normalize();
	TurnTo(vDir);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateLocked
//
//	PURPOSE:	Keep the enemy an enemy, turn toward it, fire when facing it
//
// ----------------------------------------------------------------------- //

bool Finder::UpdateLocked()
{
	CObjectRelationMgr* pORM = CRelationMgr::GetGlobalRelationMgr()->GetObjectRelationMgr(m_hLastDetectedEnemy);
	if (pORM)
	{
		m_pObjectRelationMgr->ResetRelationTime(pORM);
	}

	LTVector vPos, vEnemyPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectPos(m_hLastDetectedEnemy, &vEnemyPos);

	LTVector vDir = vEnemyPos - vPos;
	vDir.Normalize();
	if (TurnTo(vDir) < 1e-11f)
	{
		Fire();
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateLingering
//
//	PURPOSE:	Look left and right of where the enemy was lost, until the
//				linger time is up
//
// ----------------------------------------------------------------------- //

bool Finder::UpdateLingering()
{
	if (m_LingerTimer.Stopped() && !m_bScanInfinite)
	{
		SetState(eStateClosing);
	}

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	LTVector vForward = rRot.Forward();

	if (m_eLingerState == eLingerReturningFromLeft || m_eLingerState == eLingerReturningFromRight)
	{
		LTFLOAT fDot = vForward.Dot(m_vLingerDir);
		if (fDot < -1.0f)
		{
			fDot = -1.0f;
		}
		else if (fDot > 1.0f)
		{
			fDot = 1.0f;
		}

		if (fDot > 0.9f)
		{
			if (m_eLingerState == eLingerReturningFromLeft)
			{
				m_eLingerState = eLingerTurningRight;
			}
			else if (m_eLingerState == eLingerReturningFromRight)
			{
				m_eLingerState = eLingerTurningLeft;
			}
		}
	}

	LTRotation rLinger(m_vLingerDir, LTVector(0.0f, 1.0f, 0.0f));
	if (m_eLingerState == eLingerTurningLeft)
	{
		rLinger = rLinger * LTRotation(LTVector(0.0f, 1.0f, 0.0f), m_fLingerMaxTurn);
	}
	else if (m_eLingerState == eLingerTurningRight)
	{
		rLinger = rLinger * LTRotation(LTVector(0.0f, 1.0f, 0.0f), m_fLingerMaxTurn * -1.0f);
	}

	if (TurnTo(rLinger.Forward()) < 1e-11f)
	{
		if (m_eLingerState == eLingerTurningLeft)
		{
			m_eLingerState = eLingerReturningFromLeft;
		}
		else if (m_eLingerState == eLingerTurningRight)
		{
			m_eLingerState = eLingerReturningFromRight;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateSearching
//
//	PURPOSE:	Turn toward where it was hit from, then linger there
//
// ----------------------------------------------------------------------- //

bool Finder::UpdateSearching()
{
	if (TurnTo(m_vSearchDir) < 1e-11f)
	{
		SetState(eStateLingering);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::ResumeKeyframer / PauseKeyframer / ResetKeyframer
//
//	PURPOSE:	RETAIL: the strings they create are never freed
//
// ----------------------------------------------------------------------- //

void Finder::ResumeKeyframer()
{
	if (m_hKeyframer)
	{
		HSTRING hstrMsg = g_pLTServer->CreateString("RESUME");
		SendTriggerMsgToObject(this, m_hKeyframer, hstrMsg);
	}
}

void Finder::PauseKeyframer()
{
	if (m_hKeyframer)
	{
		HSTRING hstrMsg = g_pLTServer->CreateString("PAUSE");
		SendTriggerMsgToObject(this, m_hKeyframer, hstrMsg);
	}
}

void Finder::ResetKeyframer()
{
	if (m_hKeyframer)
	{
		HSTRING hstrMsg = g_pLTServer->CreateString("ResetPath");
		SendTriggerMsgToObject(this, m_hKeyframer, hstrMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::TurnTo
//
//	PURPOSE:	Turn (about the y axis) toward vDir as far as the turn rate
//				allows since the last update; 0 once it faces it
//
// ----------------------------------------------------------------------- //

LTFLOAT Finder::TurnTo(const LTVector &vDir)
{
	LTVector vTarget = vDir;
	vTarget.y = 0.0f;
	vTarget.Normalize();

	LTFLOAT fMaxTurn = (g_pLTServer->GetTime() - m_fLastUpdateTime) * m_fTurnRate;

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	LTFLOAT fDot = rRot.Forward().Dot(vTarget);
	if (fDot < -1.0f)
	{
		fDot = -1.0f;
	}
	else if (fDot > 1.0f)
	{
		fDot = 1.0f;
	}

	if ((LTFLOAT)acos(fDot) <= fMaxTurn)
	{
		LTRotation rNew(vTarget, LTVector(0.0f, 1.0f, 0.0f));
		g_pLTServer->SetObjectRotation(m_hObject, &rNew);
		return 0.0f;
	}

	LTVector vUp = rRot.Up();
	LTVector vForward = rRot.Forward();

	LTFLOAT fSide = vTarget.Dot(rRot.Right());
	if (fSide < -1.0f)
	{
		fSide = -1.0f;
	}
	else if (fSide > 1.0f)
	{
		fSide = 1.0f;
	}

	int nDir = 0;
	if (fDot < 0.0f && fDot > 0.0f)
	{
		// RETAIL: never true
	}
	else if (fSide < 0.0f)
	{
		nDir = -1;
	}
	else
	{
		nDir = 1;
	}

	rRot.Rotate(LTVector(0.0f, 1.0f, 0.0f), (LTFLOAT)nDir * fMaxTurn);

	if (nDir == 1)
	{
		m_nLastTurnDir = 0;
	}
	else if (nDir == -1)
	{
		m_nLastTurnDir = 1;
	}

	g_pLTServer->SetObjectRotation(m_hObject, &rRot);
	return fMaxTurn;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateAnimation
//
//	PURPOSE:	Loop the animation of the state once the one playing is done
//
// ----------------------------------------------------------------------- //

bool Finder::UpdateAnimation()
{
	if (!g_pLTServer->GetModelLooping(m_hObject) &&
		!(g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE))
	{
		return false;
	}

	HMODELANIM hCurAnim = g_pLTServer->GetModelAnimation(m_hObject);

	const char* szAnim;
	if (m_eState == eStateLocked && !m_bHasWeapon)
	{
		szAnim = s_szNoWeaponAnim;
	}
	else
	{
		szAnim = s_aszStateAnims[m_eState];
	}

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, szAnim);
	if (hCurAnim != hAnim)
	{
		g_pLTServer->SetModelLooping(m_hObject, true);
		g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::PlayAnimation
//
//	PURPOSE:	Play an animation once
//
// ----------------------------------------------------------------------- //

bool Finder::PlayAnimation(const char* szAnim)
{
	HMODELANIM hCurAnim = g_pLTServer->GetModelAnimation(m_hObject);
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, szAnim);

	if (hAnim != INVALID_ANI && hAnim != hCurAnim)
	{
		g_pLTServer->SetModelLooping(m_hObject, false);
		g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::PlayScanAnimation
//
// ----------------------------------------------------------------------- //

bool Finder::PlayScanAnimation()
{
	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "Scan");
	if (hAnim != INVALID_ANI)
	{
		g_pLTServer->SetModelLooping(m_hObject, true);
		g_pLTServer->SetModelAnimation(m_hObject, hAnim);
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::CreateExplosion
//
// ----------------------------------------------------------------------- //

void Finder::CreateExplosion()
{
	HCLASS hClass = g_pLTServer->GetClass("Explosion");
	if (!hClass) return;

	ObjectCreateStruct theStruct;

	g_pLTServer->GetObjectPos(m_hObject, &theStruct.m_Pos);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	theStruct.m_Rotation = LTRotation(rRot.Forward(), rRot.Up());

	Explosion* pExplosion = (Explosion*)g_pLTServer->CreateObject(hClass, &theStruct);
	if (pExplosion)
	{
		AMMO const* pAmmo = g_pWeaponMgr->GetAmmo(s_szExplosionAmmo);
		if (pAmmo)
		{
			pExplosion->Setup(m_hObject, pAmmo->nId);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::HandleDeath
//
//	PURPOSE:	Blow up: start over (FakeDeath) or go away
//
// ----------------------------------------------------------------------- //

void Finder::HandleDeath()
{
	if (m_hstrDeathCommand)
	{
		const char* pCmd = g_pLTServer->GetStringData(m_hstrDeathCommand);
		if (g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
		}
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	const char* szFX = g_pLTServer->GetStringData(m_hstrDeathFX);
	if (szFX)
	{
		PlayClientFX((char*)szFX, LTNULL, &vPos, &rRot, 0);
	}

	CreateExplosion();

	if (m_bFakeDeath)
	{
		m_damage.Reset(m_damage.GetMaxHitPoints(), m_damage.GetMaxArmorPoints(), m_damage.GetMaxEnergy());
		ResetKeyframer();
		PauseKeyframer();
		m_damage.ClearProgressiveDamage(DT_INVALID, true);
		Restart();
	}
	else
	{
		EnableSensing(LTFALSE);

		char szName[128];
		sprintf(szName, "Finder0x%08x", this);
		LineSystem::RemoveSystem(this, szName);

		g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::Fire
//
//	PURPOSE:	Fire at the enemy (at 2/3 of its height)
//
// ----------------------------------------------------------------------- //

LTBOOL Finder::Fire()
{
	if (!m_bHasWeapon) return LTTRUE;
	if (!m_pWeapon) return LTTRUE;

	// PORT: Tron's WEAPON has the FireDelay in seconds (a float, +0x17c);
	// ours (NOLF2's) has it in milliseconds
	if (g_pLTServer->GetTime() - m_pWeapon->GetLastFireTime() <= m_pWeapon->GetWeaponData()->m_nFireDelay / 1000.0f)
	{
		return LTTRUE;
	}

	WeaponFireInfo fireInfo;
	fireInfo.hFiredFrom = m_hObject;

	LTVector vPos, vEnemyPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectPos(m_hLastDetectedEnemy, &vEnemyPos);

	LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hLastDetectedEnemy, &vDims, g_pLTServer->GetModelAnimation(m_hLastDetectedEnemy));
	vEnemyPos.y += vDims.y * 0.66f;

	fireInfo.hTestObj	= m_hLastDetectedEnemy;
	fireInfo.vPath		= vEnemyPos - vPos;
	fireInfo.vPath.Normalize();
	fireInfo.vFirePos	= vPos;
	fireInfo.vFlashPos	= vPos;

	// PORT: retail also keeps GetRelationData() (IObjectRelationMgrUser's
	// slot 2) in Tron's WeaponFireInfo (+0x50); ours has no such member

	return m_pWeapon->UpdateWeapon(fireInfo, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::SetWeapon
//
// ----------------------------------------------------------------------- //

bool Finder::SetWeapon(int nWeaponId)
{
	WEAPON const* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeaponData) return false;

	m_pWeapons = debug_new(CWeapons);
	m_pWeapons->Init(m_hObject);
	m_pWeapons->ObtainWeapon(pWeaponData->nId);
	m_pWeapons->ChangeWeapon(pWeaponData->nId);

	CWeapon* pWeapon = m_pWeapons->GetCurWeapon();
	if (pWeapon)
	{
		m_pWeapon = pWeapon;
		m_pWeapons->SetAmmo(pWeapon->GetAmmoId());
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateScanVolume
//
//	PURPOSE:	Draw the scan volume (FinderDrawScanVolume), in the color
//				of the state
//
// ----------------------------------------------------------------------- //

void Finder::UpdateScanVolume()
{
	char szName[128];
	sprintf(szName, "Finder0x%08x", this);

	DebugLineSystem& system = LineSystem::GetSystem(this, szName);
	system.Clear();

	if (s_vtDrawScanVolume.GetFloat() != 0.0f)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		LTMatrix mat;
		rRot.ConvertToMatrix(mat);

		LTFLOAT fRadius = (LTFLOAT)tan(acos(m_fFOV)) * m_fVisualRange;

		LTVector aPoints[30];
		LTFLOAT fAngle = 0.0f;
		int i;
		for (i = 0; i < 30; i++)
		{
			mat.Apply(LTVector((LTFLOAT)cos(fAngle) * fRadius, (LTFLOAT)sin(fAngle) * fRadius, m_fVisualRange), aPoints[i]);
			fAngle += MATH_CIRCLE / 30.0f;
		}

		DebugLine::Color color = Color::Purple;
		switch (m_eState)
		{
			case eStateClosing:		color = Color::Blue;	break;
			case eStateClosed:		color = Color::Green;	break;
			case eStateFocusing:	color = Color::Yellow;	break;
			case eStateLocked:		color = Color::Red;		break;
			case eStateLingering:	color = Color::White;	break;
		}

		for (i = 0; i < 30; i++)
		{
			system.AddLine(vPos, vPos + aPoints[i], color, 126);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::IsDead
//
// ----------------------------------------------------------------------- //

LTBOOL Finder::IsDead() const
{
	return m_damage.IsDead();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::UpdateTargets
//
//	PURPOSE:	Forget the targets that are gone or timed out
//
// ----------------------------------------------------------------------- //

LTBOOL Finder::UpdateTargets()
{
	FinderTargetList::iterator it = m_lstTargets.begin();
	while (it != m_lstTargets.end())
	{
		FinderTargetList::iterator itNext = it;
		++itNext;

		if (!it->m_hObj)
		{
			m_lstTargets.erase(it);
		}

		it = itNext;
	}

	// RETAIL bug: compares the time left with the game time (and the
	// timers were started with a time as their duration, AddTarget)
	LTFLOAT fTime = g_pLTServer->GetTime();

	it = m_lstTargets.begin();
	while (it != m_lstTargets.end())
	{
		FinderTargetList::iterator itNext = it;
		++itNext;

		if (it->m_Timer.GetCountdownTime() < fTime)
		{
			m_lstTargets.erase(it);
		}

		it = itNext;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::AddTarget
//
// ----------------------------------------------------------------------- //

LTBOOL Finder::AddTarget(HOBJECT hObj, LTFLOAT fTime)
{
	CTimer timer;

	// RETAIL bug: a time, not a duration: the timer's countdown is the
	// game time + 2 * fTime - now, which UpdateTargets compares with now
	timer.Start(g_pLTServer->GetTime() + fTime);

	m_lstTargets.push_back(FinderTarget(timer, hObj));

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Finder::FindTarget
//
// ----------------------------------------------------------------------- //

// The target of an object (ours)
struct FinderTargetIs : public std::binary_function<FinderTarget, HOBJECT, bool>
{
	bool operator()(const FinderTarget& target, HOBJECT hObj) const
	{
		return target.m_hObj == hObj;
	}
};

FinderTarget* Finder::FindTarget(HOBJECT hObj)
{
	FinderTargetList::iterator it = std::find_if(m_lstTargets.begin(), m_lstTargets.end(),
		std::bind2nd(FinderTargetIs(), hObj));

	if (it != m_lstTargets.end())
	{
		return &(*it);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	FinderPlugin
//
// ----------------------------------------------------------------------- //

bool FinderPlugin::sm_bInitted;
SpecialFXPlugin FinderPlugin::sm_FXPlugin;
