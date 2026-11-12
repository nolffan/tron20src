// ----------------------------------------------------------------------- //
//
// MODULE  : TronTurret.cpp
//
// PURPOSE : A turret that turns towards what it senses and fires a weapon
//
// RETAIL: Tron 2.0's, written from retail Object.lto (TronTurret.h). The
// names of functions not in an interface are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronTurret.h"
#include "PlayerObj.h"
#include "ServerUtilities.h"
#include "ParsedMsg.h"
#include "ObjectRelationMgr.h"
#include "RelationButeMgr.h"
#include "AIStimulusMgr.h"
#include "AIUtils.h"
#include "AISenseRecorderAbstract.h"
#include "VolumeBrushTypes.h"
#include "SurfaceFunctions.h"
#include "SurfaceMgr.h"
#include "ServerSoundMgr.h"
#include "Weapon.h"
#include "WeaponFireInfo.h"
#include "WeaponMgr.h"
#include "PropTypeMgr.h"
#include "Scanner.h"

BEGIN_CLASS(TronTurret)
	ADD_STRINGPROP_FLAG(PropType, "Turret", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)
	ADD_STRINGPROP(Target, "player")
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP(ViewFOV, 180.0f)
	ADD_REALPROP(FireFOV, 5.0f)
	ADD_REALPROP(TurnRate, 10.0f)
	ADD_LONGINTPROP(RateOfFire, 5)
	ADD_STRINGPROP_FLAG(Alignment, "TronTurret", PF_STATICLIST)
	ADD_REALPROP_FLAG(SpotRange, 10000.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(IgnoreDist, 500.0f, PF_RADIUS)
	ADD_OBJECTPROP(Parent, "")
	ADD_STRINGPROP(ParentSocket, "")
	ADD_STRINGPROP(FireSocket, "FireSocket1")
	ADD_STRINGPROP(SensingSocket, "FireSocket1")
	ADD_BOOLPROP(ON, LTTRUE)
	ADD_STRINGPROP(Weapon, "TankGun")
END_CLASS_DEFAULT_FLAGS_PLUGIN(TronTurret, GameBase, NULL, NULL, 0, TronTurretPlugin)

// RETAIL: the turret's sounds (0x102deeb4..0x102deec0); our names

static char* s_szLoopSound	= "chars\\snd\\tanks\\loop_turret.wav";
static char* s_szStartSound	= "chars\\snd\\tanks\\start_turret.wav";
static char* s_szStopSound	= "chars\\snd\\tanks\\stop_turret.wav";
static char* s_szFireSound	= "chars\\snd\\tanks\\fire.wav";

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( TronTurret )
CMDMGR_END_REGISTER_CLASS( TronTurret, GameBase )

extern CRelationButeMgr s_RelationButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurretPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill in the prop types and the alignments
//
// ----------------------------------------------------------------------- //

LTRESULT TronTurretPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	// RETAIL: 0x100696b0

	if ( !s_RelationButeMgr.IsInitialized() )
	{
		char szFile[256];
		sprintf(szFile, "%s\\Attributes\\RelationData.txt", szRezPath);
		s_RelationButeMgr.SetInRezFile(LTFALSE);
	    s_RelationButeMgr.Init(szFile);
	}

	if (_strcmpi("PropType", szPropName) == 0)
	{
		if (m_PropTypeMgrPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		{
			qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );

			return LT_OK;
		}
	}

	if ( !_strcmpi("Alignment", szPropName) )
	{
		CRelationButeMgr::_mapNameToID* pNames = s_RelationButeMgr.GetMapObjectRelationMgrNameToID();
		CRelationButeMgr::_mapNameToID::iterator it;

		for( it = pNames->begin(); it != pNames->end(); ++it )
		{
			strcpy( aszStrings[(*pcStrings)++], it->first.c_str() );
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::DefaultFilterFn / BodyFilterFn
//
//	PURPOSE:	Filters for what the turret can see through
//
// ----------------------------------------------------------------------- //

bool TronTurret::DefaultFilterFn(HOBJECT hObj, void *pUserData)
{
	// RETAIL: 0x10069890

    if (!hObj || !g_pLTServer) return false;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hObj));
	if ( pSurf && pSurf->bCanSeeThrough )
	{
        return false;
	}

    HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return false;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

    static HCLASS hProjectile = g_pLTServer->GetClass("CProjectile");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hProjectile))
	{
        return false;
	}

	return LiquidFilterFn(hObj, pUserData);
}

bool TronTurret::BodyFilterFn(HOBJECT hObj, void *pUserData)
{
	// RETAIL: 0x100699b0

    if (!hObj || !g_pLTServer) return false;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hObj));
	if ( pSurf && pSurf->bCanSeeThrough )
	{
        return false;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

    static HCLASS hProjectile = g_pLTServer->GetClass("CProjectile");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hProjectile))
	{
        return false;
	}

	return LiquidFilterFn(hObj, pUserData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::TronTurret
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

TronTurret::TronTurret() : GameBase(OT_NORMAL)
	, m_eState(0)
	, m_eLastState(0)
	, m_hLoopSound(LTNULL)
	, m_hStartSound(LTNULL)
	, m_hStopSound(LTNULL)
	, m_fLastUpdateTime(0.0f)
	, m_fNextFireTime(0.0f)
	, m_bHasParent(false)
	, m_bHasParentSocket(false)
	, m_fYaw(0.0f)
	, m_fLastYaw(0.0f)
	, m_pWeapon(LTNULL)
	, m_fCosViewFOV(1.57f)
	, m_fSpotRange(0.0f)
	, m_fSpotRangeSqr(0.0f)
	, m_fTurnRate(1.0f)
	, m_nRateOfFire(1)
	, m_bOn(true)
	, m_fCosFireFOV(0.02f)
	, m_fIgnoreDistSqr(0.0f)
	, m_hSensingSocket(INVALID_MODEL_SOCKET)
	, m_bHasSensingSocket(false)
	, m_bHasFireSocket(false)
	, m_hFireSocket(INVALID_MODEL_SOCKET)
	, m_bHasTarget(false)
	, m_hStimulus(LTNULL)
	, m_eStimulusID(kStimID_Unset)
	, m_eStimulusSenseType(kSense_None)
	, m_fStimulusTime(0.0f)
	, m_fLastResetTime(0.0f)
	, m_bSensing(false)
	, m_bAlert(true)
	, m_dwSenses(kSense_SeeEnemy)
	, m_fSenseUpdateRate(0.01f)
	, m_fNextSenseUpdate(0.0f)
	, m_bDoneProcessingStimuli(LTFALSE)
	, m_cIntersectSegmentCount(0)
{
	// RETAIL: 0x10069aa0

	m_pObjectRelationMgr = debug_new(CObjectRelationMgr);
	m_pObjectRelationMgr->ClearRelationSystem();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::GetRelationSet / GetRelationData
//
//	PURPOSE:	IObjectRelationMgrUser's accessors
//
// ----------------------------------------------------------------------- //

const RelationSet& TronTurret::GetRelationSet() const
{
	// RETAIL: 0x10069e20
	return m_pObjectRelationMgr->CObjectRelationMgr::GetRelationUser()->GetRelations();
}

const RelationData& TronTurret::GetRelationData() const
{
	// RETAIL: 0x10069e40
	return m_pObjectRelationMgr->GetData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::~TronTurret
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

TronTurret::~TronTurret()
{
	// RETAIL: 0x1006a020

	if ( m_pObjectRelationMgr )
	{
		debug_delete( m_pObjectRelationMgr );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 TronTurret::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	// RETAIL: 0x1006a1a0

	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		return GameBase::EngineMessageFn(MID_PRECREATE, pData, fData);

		case MID_INITIALUPDATE:
		{
			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
				InitWeapon();
			}

			SetNextUpdate(0.01f);
		}
		return 0;

		case MID_ALLOBJECTSCREATED:
		{
			HandleAllObjectsCreated();
		}
		return 1;

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

		case MID_ACTIVATING:
		{
			if (m_bOn)
			{
				EnableSensing(true);
			}
		}
		break;

		case MID_DEACTIVATING:
		{
			if (m_bOn)
			{
				EnableSensing(false);
			}
		}
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::ReadProp
//
//	PURPOSE:	Read in the properties
//
// ----------------------------------------------------------------------- //

LTBOOL TronTurret::ReadProp(ObjectCreateStruct *pStruct)
{
	// RETAIL: 0x1006a330. Near: retail inlines only the first clear() and
	// calls the other six (VC6's inline budget); ours inlines all seven.

	GenericProp genProp;
	PROPTYPE* pPropType;

	m_Strings.sParent.clear();
	m_Strings.sParentSocket.clear();
	m_Strings.sAlignment.clear();
	m_Strings.sSensingSocket.clear();
	m_Strings.sFireSocket.clear();
	m_Strings.sTarget.clear();
	m_Strings.sWeapon.clear();

	if (g_pLTServer->GetPropGeneric("PropType", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			pPropType = g_pPropTypeMgr->GetPropType(genProp.m_String);
		}
	}

	// RETAIL: with no ViewFOV, the default (already a cosine) is taken
	// for an angle (RETAIL_BUGS.md)
	if (g_pLTServer->GetPropGeneric("ViewFOV", &genProp) == LT_OK)
	{
		m_fCosViewFOV = (LTFLOAT)cos(genProp.m_Float * (MATH_PI / 360.0f));
	}
	else
	{
		m_fCosViewFOV = (LTFLOAT)cos(m_fCosViewFOV * (MATH_PI / 360.0f));
	}

	if (g_pLTServer->GetPropGeneric("TurnRate", &genProp) == LT_OK)
	{
		m_fTurnRate = RADIANS_PER_DEGREE * genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("SpotRange", &genProp) == LT_OK)
	{
		m_fSpotRange = genProp.m_Float;
		m_fSpotRangeSqr = genProp.m_Float * genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("RateOfFire", &genProp) == LT_OK)
	{
		m_nRateOfFire = (uint8)genProp.m_Long;
	}

	if (g_pLTServer->GetPropGeneric("ON", &genProp) == LT_OK)
	{
		m_bOn = genProp.m_Bool;
	}

	if (g_pLTServer->GetPropGeneric("FireFOV", &genProp) == LT_OK)
	{
		m_fCosFireFOV = (LTFLOAT)cos(RADIANS_PER_DEGREE * genProp.m_Float);
	}

	if (g_pLTServer->GetPropGeneric("IgnoreDist", &genProp) == LT_OK)
	{
		m_fIgnoreDistSqr = genProp.m_Float * genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("Parent", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_Strings.sParent = genProp.m_String;
		}
	}

	if (g_pLTServer->GetPropGeneric("ParentSocket", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_Strings.sParentSocket = genProp.m_String;
		}
	}

	if (g_pLTServer->GetPropGeneric("FireSocket", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_Strings.sFireSocket = genProp.m_String;
		}
	}

	if (g_pLTServer->GetPropGeneric("SensingSocket", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_Strings.sSensingSocket = genProp.m_String;
		}
	}

	if (g_pLTServer->GetPropGeneric("Target", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_Strings.sTarget = genProp.m_String;
		}
	}

	if (g_pLTServer->GetPropGeneric("Alignment", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_Strings.sAlignment = genProp.m_String;
		}
	}

	if (g_pLTServer->GetPropGeneric("Weapon", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_Strings.sWeapon = genProp.m_String;
		}
		else
		{
			m_Strings.sWeapon = "TankGun";
		}
	}
	else
	{
		m_Strings.sWeapon = "TankGun";
	}

	// RETAIL: pPropType is not set when there is no PropType (RETAIL_BUGS.md)

	if (pPropType && !pPropType->sFilename.empty())
	{
		pStruct->m_ObjectType = OT_MODEL;
		pPropType->SetupModel(pStruct);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TronTurret::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	// RETAIL: 0x1006a9c0

	m_pObjectRelationMgr->Save(pMsg);

	SAVE_BYTE(m_eState);
	SAVE_BYTE(m_eLastState);
	SAVE_FLOAT(m_fLastUpdateTime);
	SAVE_FLOAT(m_fNextFireTime);
	SAVE_HOBJECT(m_hParent);
	SAVE_DWORD(m_hParentSocket);
	SAVE_bool(m_bHasParent);
	SAVE_bool(m_bHasParentSocket);
	SAVE_VECTOR(m_tInitial.m_Pos);
	SAVE_ROTATION(m_tInitial.m_Rot);
	SAVE_FLOAT(m_fYaw);
	SAVE_FLOAT(m_fLastYaw);
	SAVE_FLOAT(m_fCosViewFOV);
	SAVE_FLOAT(m_fSpotRange);
	SAVE_FLOAT(m_fTurnRate);
	SAVE_BYTE(m_nRateOfFire);
	SAVE_bool(m_bOn);
	SAVE_FLOAT(m_fCosFireFOV);
	SAVE_FLOAT(m_fIgnoreDistSqr);
	SAVE_DWORD(m_hSensingSocket);
	SAVE_bool(m_bHasSensingSocket);
	SAVE_bool(m_bHasFireSocket);
	SAVE_DWORD(m_hFireSocket);
	SAVE_bool(m_bHasTarget);
	SAVE_HOBJECT(m_hTarget);

	SAVE_HOBJECT( m_hStimulus );
	SAVE_DWORD(	m_eStimulusID );
	SAVE_DWORD(	m_eStimulusSenseType );
	SAVE_TIME( m_fStimulusTime );
	SAVE_TIME( m_fLastResetTime );
	SAVE_bool( m_bSensing );
	SAVE_bool( m_bAlert );

	// Save IAISensing members.

	SAVE_DWORD( m_dwSenses );
	SAVE_FLOAT( m_fSenseUpdateRate );
	SAVE_TIME( m_fNextSenseUpdate );
	SAVE_RANGE( m_rngSightGrid );

	SAVE_DWORD( m_bDoneProcessingStimuli );

	SAVE_DWORD( m_mapProcessedStimuli.size() );
	AI_PROCESSED_STIMULI_MAP::iterator psit;
	for( psit = m_mapProcessedStimuli.begin(); psit != m_mapProcessedStimuli.end(); ++psit )
	{
		SAVE_DWORD( psit->first );
	}

	SAVE_DWORD( m_cIntersectSegmentCount );

	m_Weapons.Save(pMsg, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TronTurret::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	// RETAIL: 0x1006ada0. Near: register allocation of the zero and the
	// processed-stimuli loop differ.

	m_pObjectRelationMgr->Load(pMsg);

	LOAD_BYTE(m_eState);
	LOAD_BYTE(m_eLastState);
	LOAD_FLOAT(m_fLastUpdateTime);
	LOAD_FLOAT(m_fNextFireTime);
	LOAD_HOBJECT(m_hParent);
	LOAD_DWORD(m_hParentSocket);
	LOAD_bool(m_bHasParent);
	LOAD_bool(m_bHasParentSocket);
	LOAD_VECTOR(m_tInitial.m_Pos);
	LOAD_ROTATION(m_tInitial.m_Rot);
	LOAD_FLOAT(m_fYaw);
	LOAD_FLOAT(m_fLastYaw);
	LOAD_FLOAT(m_fCosViewFOV);
	LOAD_FLOAT(m_fSpotRange);
	m_fSpotRangeSqr = m_fSpotRange * m_fSpotRange;
	LOAD_FLOAT(m_fTurnRate);
	LOAD_BYTE(m_nRateOfFire);
	LOAD_bool(m_bOn);
	LOAD_FLOAT(m_fCosFireFOV);
	LOAD_FLOAT(m_fIgnoreDistSqr);
	LOAD_DWORD(m_hSensingSocket);
	LOAD_bool(m_bHasSensingSocket);
	LOAD_bool(m_bHasFireSocket);
	LOAD_DWORD(m_hFireSocket);
	LOAD_bool(m_bHasTarget);
	LOAD_HOBJECT(m_hTarget);

	LOAD_HOBJECT( m_hStimulus );
	LOAD_DWORD_CAST( m_eStimulusID, EnumAIStimulusID );
	LOAD_DWORD_CAST( m_eStimulusSenseType, EnumAISenseType );
	LOAD_TIME( m_fStimulusTime );
	LOAD_TIME( m_fLastResetTime );
	LOAD_bool( m_bSensing );
	LOAD_bool( m_bAlert );

	// Load IAISensing members.

	LOAD_DWORD( m_dwSenses );
	LOAD_FLOAT( m_fSenseUpdateRate );
	LOAD_TIME( m_fNextSenseUpdate );
	LOAD_RANGE_CAST( m_rngSightGrid, int );
	m_fNextSenseUpdate = 0.0f;

	LOAD_DWORD( m_bDoneProcessingStimuli );

	EnumAIStimulusID eStimID;
	uint32 cStimulus;
	LOAD_DWORD( cStimulus );
	for( uint32 iStimulus=0; iStimulus < cStimulus; ++iStimulus )
	{
		LOAD_DWORD_CAST( eStimID, EnumAIStimulusID );
		m_mapProcessedStimuli.insert( AI_PROCESSED_STIMULI_MAP::value_type( eStimID, 0 ) );
	}

	LOAD_DWORD( m_cIntersectSegmentCount );

	m_Weapons.Load(pMsg, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool TronTurret::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	// RETAIL: 0x1006b2d0. Near: register allocation only (retail keeps
	// _stricmp in ebx, ours a zero).

	static CParsedMsg::CToken s_cTok_On("On");
	static CParsedMsg::CToken s_cTok_Off("Off");
	static CParsedMsg::CToken s_cTok_Target("target");

	if (cMsg.GetArg(0) == s_cTok_On)
	{
		m_bOn = true;
		EnableSensing(true);
		SetNextUpdate(0.01f);
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
		m_bOn = false;
		m_eState = 0;
		EnableSensing(false);
		SetNextUpdate(0.01f);
	}

	if (cMsg.GetArg(0) == s_cTok_Target)
	{
		if (cMsg.GetArgCount() < 2)
		{
			g_pLTServer->CPrint("TRON TURRET ERROR : MSG TARGET, NOT ENOUGH ARGS");
			return false;
		}

		m_Strings.sTarget = cMsg.GetArg(1).c_str();

		if (_stricmp(m_Strings.sTarget.c_str(), "player"))
		{
			ObjArray<HOBJECT, 1> objArray;
			if (g_pLTServer->FindNamedObjects(m_Strings.sTarget.c_str(), objArray) == LT_OK && objArray.NumObjects())
			{
				m_hTarget = objArray.GetObject(0);
				m_bHasTarget = true;
			}
		}
		else
		{
			m_bHasTarget = false;
		}

		m_Strings.sTarget.clear();
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::HandleAllObjectsCreated
//
//	PURPOSE:	Find the parent, the sockets, the target and the alignment
//
// ----------------------------------------------------------------------- //

void TronTurret::HandleAllObjectsCreated()
{
	// RETAIL: 0x1006b560. Near: retail calls the last clear() out of line
	// (inline budget).

	if (m_Strings.sParent.length())
	{
		if (FindNamedObject(m_Strings.sParent.c_str(), m_hParent) == LT_OK)
		{
			m_bHasParent = true;
			if (g_pModelLT->GetSocket(m_hParent, m_Strings.sParentSocket.c_str(), m_hParentSocket) == LT_OK)
			{
				m_bHasParentSocket = true;
			}
		}
		else
		{
			g_pLTServer->CPrint("ERROR in TronTurret, could not find a parent %s", m_Strings.sParent.c_str());
		}
	}

	if (m_Strings.sSensingSocket.length())
	{
		if (g_pModelLT->GetSocket(m_hObject, m_Strings.sSensingSocket.c_str(), m_hSensingSocket) == LT_OK)
		{
			m_bHasSensingSocket = true;
		}
	}

	if (m_Strings.sFireSocket.length())
	{
		if (g_pModelLT->GetSocket(m_hObject, m_Strings.sFireSocket.c_str(), m_hFireSocket) == LT_OK)
		{
			m_bHasFireSocket = true;
		}
	}

	if (m_Strings.sTarget.length() && _stricmp(m_Strings.sTarget.c_str(), "player"))
	{
		ObjArray<HOBJECT, 1> objArray;
		if (g_pLTServer->FindNamedObjects(m_Strings.sTarget.c_str(), objArray) == LT_OK && objArray.NumObjects())
		{
			m_hTarget = objArray.GetObject(0);
			m_bHasTarget = true;
		}
	}

	if (m_Strings.sAlignment.length())
	{
		m_pObjectRelationMgr->Init(this, m_Strings.sAlignment.c_str());
	}

	m_Strings.sParent.clear();
	m_Strings.sParentSocket.clear();
	m_Strings.sAlignment.clear();
	m_Strings.sSensingSocket.clear();
	m_Strings.sFireSocket.clear();
	m_Strings.sTarget.clear();
	m_Strings.sWeapon.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::InitWeapon
//
//	PURPOSE:	Give the turret its weapon
//
// ----------------------------------------------------------------------- //

bool TronTurret::InitWeapon()
{
	// RETAIL: 0x1006b7c0

	const WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_Strings.sWeapon.c_str());

	m_Weapons.Init(m_hObject);
	m_Weapons.ObtainWeapon(pWeaponData->nId);
	m_Weapons.ChangeWeapon(pWeaponData->nId);

	m_pWeapon = m_Weapons.GetCurWeapon();
	m_Weapons.SetAmmo(m_pWeapon->GetAmmoId());

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::*ProcessingStimuli()
//
//	PURPOSE:	Process stimuli.
//
// ----------------------------------------------------------------------- //

LTBOOL TronTurret::GetDoneProcessingStimuli() const
{
	// RETAIL: 0x1006b840
	return m_bDoneProcessingStimuli;
}

void TronTurret::SetDoneProcessingStimuli(LTBOOL bDone)
{
	// RETAIL: 0x1006b850
	m_bDoneProcessingStimuli = bDone;
}

void TronTurret::ClearProcessedStimuli()
{
	// RETAIL: 0x1006b860
	m_mapProcessedStimuli.clear();
}

LTBOOL TronTurret::ProcessStimulus(CAIStimulusRecord* pRecord)
{
	// RETAIL: 0x1006b8c0

	if( m_mapProcessedStimuli.find( pRecord->m_eStimulusID ) == m_mapProcessedStimuli.end() )
	{
		m_mapProcessedStimuli.insert( AI_PROCESSED_STIMULI_MAP::value_type( pRecord->m_eStimulusID, 0 ) );
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::EnableSensing
//
//	PURPOSE:	Turn on/off sensing.
//
// ----------------------------------------------------------------------- //

void TronTurret::EnableSensing(bool bEnable)
{
	// RETAIL: 0x1006ba20

	if( bEnable != m_bSensing )
	{
		if( bEnable )
		{
			g_pAIStimulusMgr->AddSensingObject( this );
			SetNextSenseUpdate( GetSenseUpdateRate() + g_pLTServer->GetTime() );
			m_bSensing = true;
		}
		else {
			g_pAIStimulusMgr->RemoveSensingObject( this );
			m_bSensing = false;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::HandleSenseRecord()
//
//	PURPOSE:	Check if an object of interest has come into view.
//
// ----------------------------------------------------------------------- //

LTBOOL TronTurret::HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle)
{
	// RETAIL: 0x1006bab0

	if( !pStimulusRecord )
	{
		return LTFALSE;
	}

	if( pStimulusRecord->m_fTimeStamp <= m_fLastResetTime )
	{
		return LTFALSE;
	}

	LTBOOL bSeeObject = LTFALSE;
	if( IsBody( pStimulusRecord->m_hStimulusSource ) )
	{
		bSeeObject = CanSeeObject( BodyFilterFn, pStimulusRecord->m_hStimulusSource );
	}
	else {
		bSeeObject = CanSeeObject( DefaultFilterFn, pStimulusRecord->m_hStimulusSource );
	}

	if( bSeeObject )
	{
		// Only the target, when there is one.

		if( m_bHasTarget && pStimulusRecord->m_hStimulusSource != m_hTarget )
		{
			m_hStimulus = LTNULL;
			return LTFALSE;
		}

		m_hStimulus = pStimulusRecord->m_hStimulusSource;
		m_eStimulusID = pStimulusRecord->m_eStimulusID;
		m_eStimulusSenseType = pStimulusRecord->m_pAIBM_Stimulus->eSenseType;
		m_fStimulusTime = g_pLTServer->GetTime();
		return LTTRUE;
	}

	m_hStimulus = LTNULL;
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::GetSenseDistance()
//
//	PURPOSE:	Get the distance the turret can see.
//
// ----------------------------------------------------------------------- //

LTFLOAT TronTurret::GetSenseDistance(EnumAISenseType eSenseType)
{
	// RETAIL: 0x1006bbc0
	return m_fSpotRange;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::GetSenseRelationSet()
//
//	PURPOSE:	Get the relation set for the turret.
//
// ----------------------------------------------------------------------- //

const RelationSet& TronTurret::GetSenseRelationSet() const
{
	// RETAIL: 0x1006bbd0. Tron's IAISensing slot 19 returns the relation
	// manager itself (as do retail CScanner's and CAI's): a static
	// CObjectRelationMgr when there is none.
	if( m_pObjectRelationMgr )
	{
		return *(const RelationSet*)m_pObjectRelationMgr;
	}

	static CObjectRelationMgr s_ObjectRelationMgr;
	return *(const RelationSet*)&s_ObjectRelationMgr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::CanSeeObject()
//
//	PURPOSE:	Is this object visible to us?
//
// ----------------------------------------------------------------------- //

LTBOOL TronTurret::CanSeeObject(ObjectFilterFn ofn, HOBJECT hObject)
{
	// RETAIL: 0x1006bc20. Near: retail calls Length() and the vector
	// constructors out of line (inline budget).

    if (!hObject) return LTFALSE;

    LTVector vPos;
	g_pLTServer->GetObjectPos(hObject, &vPos);

	LTVector vSensingPos = GetSensingPosition();

    LTVector vDir;
	vDir = vPos - vSensingPos;

	if (VEC_MAGSQR(vDir) >= m_fSpotRangeSqr)
	{
        return LTFALSE;
	}

	vDir.Normalize();

    LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

    LTVector vUp, vRight, vForward;
	vUp = rRot.Up();
	vRight = rRot.Right();
	vForward = rRot.Forward();

    LTFLOAT fDp = vDir.Dot(vForward);

	if (fDp < m_fCosViewFOV)
	{
        return LTFALSE;
	}

	// See if we can see the position in question

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vSensingPos);
	VEC_COPY(IQuery.m_To, vPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = ofn;
	IQuery.m_PolyFilterFn = CScanner::SeeThroughPolyFilterFn;

    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		if (IInfo.m_hObject == hObject)
		{
            return LTTRUE;
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::InitialUpdate
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

bool TronTurret::InitialUpdate()
{
	// RETAIL: 0x1006bf20

	m_fYaw = 0.0f;
	m_fLastYaw = 0.0f;

	g_pLTServer->GetObjectPos(m_hObject, &m_tSensing.m_Pos);

	m_fNextFireTime = g_pLTServer->GetTime() + 60.0f / (LTFLOAT)m_nRateOfFire;

	LTVector vPos;
	LTRotation rRot;

	if (m_bHasParent)
	{
		// RETAIL: the parent's position and rotation are read into locals
		// and not used.
		g_pLTServer->GetObjectPos(m_hParent, &vPos);
		g_pLTServer->GetObjectRotation(m_hParent, &rRot);
		return true;
	}

	g_pLTServer->GetObjectPos(m_hObject, &m_tInitial.m_Pos);
	g_pLTServer->GetObjectRotation(m_hObject, &m_tInitial.m_Rot);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::GetTurnAmount
//
//	PURPOSE:	How far to turn this update to face vDir
//
// ----------------------------------------------------------------------- //

LTFLOAT TronTurret::GetTurnAmount(const LTRotation &rRot, const LTVector &vDir)
{
	// RETAIL: 0x1006c010. Near: size-exact, retail's frame is 0x18 bytes
	// bigger (stack offsets only). vUp is ours (unused; it orders the dot
	// product's terms as retail's).

	LTFLOAT fMaxTurn = (g_pLTServer->GetTime() - m_fLastUpdateTime) * m_fTurnRate;

	LTVector vUp = rRot.Up();
	LTVector vForward = rRot.Forward();
	if ((LTFLOAT)acos(vForward.Dot(vDir)) >= fMaxTurn)
	{
		LTVector vRight = rRot.Right();
		if ((LTFLOAT)acos(vRight.Dot(vDir)) <= MATH_HALFPI)
		{
			return fMaxTurn;
		}

		return -fMaxTurn;
	}

	return 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::Update
//
//	PURPOSE:	Turn towards the target and fire
//
// ----------------------------------------------------------------------- //

bool TronTurret::Update()
{
	// RETAIL: 0x1006c150. Near: retail calls the vector, rotation and
	// transform operators out of line (inline budget); Tron's sound calls
	// take one more argument.

	LTRotation rRot;

	if (m_bHasTarget && !m_hTarget)
	{
		m_bHasTarget = false;
	}

	if (m_bOn)
	{
		UpdateTarget();
	}

	if (m_bHasFireSocket)
	{
		g_pModelLT->GetSocketTransform(m_hObject, m_hFireSocket, m_tFire, LTTRUE);
	}
	else
	{
		m_tFire = m_tSensing;
	}

	LTVector vPos;

	if (m_bHasParent)
	{
		if (m_bHasParentSocket)
		{
			LTransform tf;
			if (g_pModelLT->GetSocketTransform(m_hParent, m_hParentSocket, tf, LTTRUE) == LT_OK)
			{
				vPos = tf.m_Pos;
				rRot = tf.m_Rot;
			}
		}
		else
		{
			g_pLTServer->GetObjectPos(m_hParent, &vPos);
			g_pLTServer->GetObjectRotation(m_hParent, &rRot);

			LTVector vDims;
			g_pPhysicsLT->GetObjectDims(m_hParent, &vDims);
			vPos.y += vDims.y;
		}
	}
	else
	{
		vPos = m_tInitial.m_Pos;
		rRot = m_tInitial.m_Rot;
	}

	if (m_bHasSensingSocket)
	{
		g_pModelLT->GetSocketTransform(m_hObject, m_hSensingSocket, m_tSensing, LTTRUE);
	}
	else
	{
		m_tSensing.m_Pos = vPos + rRot.Up() * 228.0f;
		m_tSensing.m_Rot = rRot;
	}

	if (m_bOn && (m_eState == 1 || m_eState == 2))
	{
		LTVector vTargetPos;
		g_pLTServer->GetObjectPos(m_hStimulus, &vTargetPos);

		LTVector vDir = vTargetPos - m_tFire.m_Pos;
		LTRotation rTurret = rRot;
		vDir.Normalize();

		rTurret = rTurret * LTRotation(0.0f, (LTFLOAT)sin(m_fYaw * 0.5f), 0.0f, (LTFLOAT)cos(m_fYaw * 0.5f));

		if (rTurret.Forward().Dot(vDir) >= m_fCosFireFOV)
		{
			m_eState = 2;

			if (g_pLTServer->GetTime() > m_fNextFireTime)
			{
				if (Fire() == W_FIRED)
				{
					m_fNextFireTime = g_pLTServer->GetTime() + 60.0f / (LTFLOAT)m_nRateOfFire;
				}
			}
		}
		else
		{
			m_fYaw += GetTurnAmount(rTurret, vDir);
		}
	}
	else
	{
		LTRotation rTurret = rRot;
		m_eState = 3;

		LTRotation rYaw(0.0f, (LTFLOAT)sin(m_fYaw * 0.5f), 0.0f, (LTFLOAT)cos(m_fYaw * 0.5f));
		LTRotation rTemp;
		quat_Mul(rTemp.m_Quat, rRot.m_Quat, rYaw.m_Quat);
		rTurret = rTemp;

		m_fYaw += GetTurnAmount(rTurret, rRot.Forward());
		if (fabs(m_fYaw) < 0.01)
		{
			m_eState = 0;
		}
	}

	LTRotation rYaw(0.0f, (LTFLOAT)sin(m_fYaw * 0.5f), 0.0f, (LTFLOAT)cos(m_fYaw * 0.5f));
	LTRotation rTemp;
	quat_Mul(rTemp.m_Quat, rRot.m_Quat, rYaw.m_Quat);
	rRot = rTemp;

	g_pLTServer->SetObjectPos(m_hObject, &vPos);
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);

	// RETAIL: Tron's sound calls take one more argument (1.0f) than ours.

	if (m_eState == 1 && m_eLastState != 1)
	{
		if (m_hLoopSound)
		{
			g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
			m_hLoopSound = LTNULL;
		}

		if (m_hStartSound)
		{
			bool bDone;
			if (g_pLTServer->SoundMgr()->IsSoundDone(m_hStartSound, bDone) != LT_OK || bDone)
			{
				m_hStartSound = LTNULL;
			}
		}

		if (!m_hStartSound)
		{
			m_hStartSound = g_pServerSoundMgr->PlaySoundFromPos(vPos, s_szStartSound, 3000.0f, SOUNDPRIORITY_MISC_LOW,
				PLAYSOUND_GETHANDLE | PLAYSOUND_TIME, 100, 1.0f, -1.0f, 0);
		}
	}
	else if (m_eState != 1 && m_eState != m_eLastState)
	{
		if (m_hStartSound)
		{
			g_pLTServer->SoundMgr()->KillSound(m_hStartSound);
			m_hStartSound = LTNULL;
		}

		if (m_hLoopSound)
		{
			g_pLTServer->SoundMgr()->KillSound(m_hLoopSound);
			m_hLoopSound = LTNULL;
		}

		if (m_hStopSound)
		{
			bool bDone;
			if (g_pLTServer->SoundMgr()->IsSoundDone(m_hStopSound, bDone) != LT_OK || bDone)
			{
				m_hStopSound = LTNULL;
			}
		}

		if (!m_hStopSound)
		{
			m_hStopSound = g_pServerSoundMgr->PlaySoundFromPos(vPos, s_szStopSound, 3000.0f, SOUNDPRIORITY_MISC_LOW,
				PLAYSOUND_GETHANDLE | PLAYSOUND_TIME, 100, 1.0f, -1.0f, 0);
		}
	}
	else if (!m_hLoopSound && (m_eState == 1 || m_eState == 3))
	{
		if (m_hStartSound)
		{
			bool bDone;
			if (g_pLTServer->SoundMgr()->IsSoundDone(m_hStartSound, bDone) != LT_OK || bDone)
			{
				m_hStartSound = LTNULL;
			}
		}

		if (!m_hStartSound && !m_hLoopSound)
		{
			m_hLoopSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, s_szLoopSound, 3000.0f, SOUNDPRIORITY_MISC_LOW,
				PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP, 100, 1.0f, -1.0f, 0);
		}
	}

	SetNextUpdate(0.01f);

	m_fLastUpdateTime = g_pLTServer->GetTime();
	m_eLastState = m_eState;
	m_fLastYaw = m_fYaw;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::GetTarget
//
//	PURPOSE:	What the turret is after
//
// ----------------------------------------------------------------------- //

HOBJECT TronTurret::GetTarget()
{
	// RETAIL: 0x1006cb50
	return m_hStimulus;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::Fire
//
//	PURPOSE:	Fire the weapon at the target
//
// ----------------------------------------------------------------------- //

WeaponState TronTurret::Fire()
{
	// RETAIL: 0x1006cb60. Near: Tron's WeaponFireInfo is laid out
	// differently from ours (retail clears a 22-byte array in it and sets
	// a member to &GetRelationData()), and its sound call takes one more
	// argument.

	if (!m_pWeapon) return W_IDLE;

	// RETAIL: WEAPON's +0x17c (our m_nFireDelay) is compared as a float
	if (g_pLTServer->GetTime() - m_pWeapon->GetLastFireTime() <= *(const LTFLOAT*)&m_pWeapon->GetWeaponData()->m_nFireDelay)
	{
		return W_IDLE;
	}

	WeaponFireInfo weaponFireInfo;
	weaponFireInfo.hFiredFrom = m_hObject;
	weaponFireInfo.hTestObj = GetTarget();

	LTVector vTargetPos;
	g_pLTServer->GetObjectPos(m_hStimulus, &vTargetPos);

	weaponFireInfo.vPath = vTargetPos - m_tFire.m_Pos;
	weaponFireInfo.vFirePos = m_tFire.m_Pos;
	weaponFireInfo.vFlashPos = m_tFire.m_Pos;

	WeaponState eState = m_pWeapon->UpdateWeapon(weaponFireInfo, LTTRUE);
	if (eState == W_FIRED)
	{
		PlayClientFX("Tank_Blast", LTNULL, &m_tFire.m_Pos, &m_tFire.m_Rot, 0);

		// RETAIL: Tron's sound calls take one more argument (10.0f) than ours.
		m_hFireSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, s_szFireSound, 10000.0f, SOUNDPRIORITY_MISC_MEDIUM,
			PLAYSOUND_GETHANDLE | PLAYSOUND_TIME, 100, 1.0f, -1.0f, 0);
	}

	return eState;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronTurret::UpdateTarget
//
//	PURPOSE:	Keep (or lose) the target in sight
//
// ----------------------------------------------------------------------- //

void TronTurret::UpdateTarget()
{
	// RETAIL: 0x1006cd60

	if (m_bHasTarget)
	{
		if (CanSeeObject(DefaultFilterFn, m_hTarget))
		{
			m_hStimulus = m_hTarget;
		}
		else
		{
			m_hStimulus = LTNULL;
		}
	}

	if (!m_hStimulus)
	{
		m_eState = 0;
	}

	if (m_hStimulus)
	{
		m_eState = 1;
	}
}
