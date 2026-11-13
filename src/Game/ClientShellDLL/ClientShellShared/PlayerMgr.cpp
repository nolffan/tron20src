// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.cpp
//
// PURPOSE : Implementation of class used to manage the client player
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "PlayerMgr.h"
#include "AttachButeMgr.h"
#include "BodyFX.h"
#include "CMoveMgr.h"
#include "CameraFX.h"
#include "CameraOffsetMgr.h"
#include "ClientUtilities.h"
#include "ClientWeaponMgr.h"
#include "FlashLight.h"
#include "GadgetDisabler.h"
#include "GameClientShell.h"	
#include "HeadBobMgr.h"
#include "LeanMgr.h"
#include "MsgIDs.h"
#include "PlayerCamera.h"
#include "PlayerShared.h"
#include "Searcher.h"
#include "VarTrack.h"
#include "VehicleMgr.h"
#include "VisionModeMgr.h"
#include "VolumeBrushFX.h"
#include "TargetMgr.h"
#include "MissionMgr.h"
#include "ClientMultiplayerMgr.h"
#include "ClientButeMgr.h"
#include "DynamicOccluderVolumeFX.h"
#include "TriggerFX.h"
#include "PlayerLureFX.h"
#include "CharacterFX.h"
#include "PolyGridFX.h"
#include "PlayerViewAttachmentMgr.h"
#include "LTEulerAngles.h"
#include "DoomsDayPieceFX.h"

CPlayerMgr* g_pPlayerMgr = NULL;

#define MAX_SHAKE_AMOUNT		10.0f

#define FOVX_ZOOMED				20.0f
#define FOVX_ZOOMED1			7.0f
#define FOVX_ZOOMED2			2.0f
#define ZOOM_TIME				0.5f
#define LOD_ZOOMADJUST			-5.0f
#define DEFAULT_LOD_OFFSET				0.0f

#define DEFAULT_CSENDRATE				15.0f
#define DEFAULT_CSENDRATE_MIN			2.0f	// RETAIL: Tron's, as Contract J.A.C.K.'s

namespace
{
	bool		g_bCinChangedNumModelShadows = false;
	int			g_nCinSaveNumModelShadows = 0;
	const		g_kMaxNumberOfCinShadows = 10;
	uint8		s_nLastCamType = CT_FULLSCREEN;
	float       s_fDeadTimer    = 0.0f;
	float       s_fDeathDelay   = 0.0f;
	LTVector	g_vSVModelColor; // model color modifier for spy vision
	int			g_nSaveSpyVisionShadows = 0;
	// RETAIL: Tron's: the player's death FX, waited for before the mission
	// is failed or the player respawns (retail 0x10271ee4; the name is
	// ours)
	CLIENTFX_LINK	s_DeathFX;
}

LTVector g_vPlayerCameraOffset = g_kvPlayerCameraOffset;

extern bool	g_bScreenShotMode;


void SVModelHook(ModelHookData *pData, void *pUser);


VarTrack			g_CV_CSendRate;		// The SendRate console variable.
VarTrack			g_vtPlayerRotate;	// The PlayerRotate console variable
VarTrack			g_vtCamZoom1MaxDist;
VarTrack			g_vtCamZoom2MaxDist;
VarTrack			g_vtUseCamRecoil;
VarTrack			g_vtCamRecoilRecover;
VarTrack			g_vtBaseCamRecoilPitch;
VarTrack			g_vtMaxCamRecoilPitch;
VarTrack			g_vtBaseCamRecoilYaw;
VarTrack			g_vtMaxCamRecoilYaw;
VarTrack			g_vtFireJitterDecayTime;
VarTrack			g_vtFireJitterMaxPitchDelta;
VarTrack			g_vtFOVXNormal;
VarTrack			g_vtFOVYNormal;
VarTrack			g_vtFOVXCycle;	// RETAIL: Tron's (the names are ours)
VarTrack			g_vtFOVYCycle;
VarTrack			g_vtFOVYMaxUW;
VarTrack			g_vtFOVYMinUW;
VarTrack			g_vtUWFOVRate;
VarTrack			g_vtRespawnWaitTime;
VarTrack			g_vtMultiplayerRespawnWaitTime;
VarTrack			g_vtCamRotInterpTime;
VarTrack			g_vtScreenFadeInTime;
VarTrack			g_vtScreenFadeOutTime;
VarTrack			g_vtChaseCamPitchAdjust;
VarTrack			g_vtChaseCamOffset;
VarTrack			g_vtChaseCamDistUp;
VarTrack			g_vtChaseCamDistBack;
VarTrack			g_vtNormalTurnRate;
VarTrack			g_vtFastTurnRate;
VarTrack			g_vtLookUpRate;
VarTrack			g_vtCameraSwayXFreq;
VarTrack			g_vtCameraSwayYFreq;
VarTrack			g_vtCameraSwayXSpeed;
VarTrack			g_vtCameraSwayYSpeed;
VarTrack			g_vtCameraSwayDuckMult;
VarTrack			g_vtModelGlowTime;
VarTrack			g_vtModelGlowMin;
VarTrack			g_vtModelGlowMax;
VarTrack			g_vtActivateOverride;
VarTrack			g_vtCamDamage;
VarTrack			g_vtCamDamagePitch;
VarTrack			g_vtCamDamageRoll;
VarTrack			g_vtCamDamageTime1;
VarTrack			g_vtCamDamageTime2;
VarTrack			g_vtCamDamageMinPitchVal;
VarTrack			g_vtCamDamageMaxPitchVal;
VarTrack			g_vtCamDamageMinRollVal;
VarTrack			g_vtCamDamageMaxRollVal;
VarTrack			g_vtCamDamagePitchMin;
VarTrack			g_vtCamDamageRollMin;
VarTrack			g_vtCamDamageFXOffsetX;
VarTrack			g_vtCamDamageFXOffsetY;
VarTrack			g_vtCamDamageFXOffsetZ;
VarTrack			g_vtAlwaysHUD;
VarTrack			g_vtDamageFadeRate;
VarTrack			g_vtAdaptiveMouse;
VarTrack			g_vtAdaptiveMouseMaxOffset;
VarTrack			g_vtShowSoundFilterInfo;
VarTrack			g_vtCSendRateMin;	// RETAIL: Tron's, as Contract J.A.C.K.'s

extern VarTrack		g_vtUseSoundFilters;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsSameMessage()
//
//	PURPOSE:	RETAIL: Tron's, as Contract J.A.C.K.'s (retail 0x10123340;
//				the name is cjacksrc's): do two messages hold the same bits?
//				(Their read positions are kept.)
//
// ----------------------------------------------------------------------- //

static bool IsSameMessage( ILTMessage_Read* pMsgA, ILTMessage_Read* pMsgB )
{
	if( !pMsgA || !pMsgB )
		return ( pMsgA == pMsgB );

	if( pMsgA->Size( ) != pMsgB->Size( ))
		return false;

	uint32 nPosA = pMsgA->Tell( );
	uint32 nPosB = pMsgB->Tell( );
	pMsgA->SeekTo( 0 );
	pMsgB->SeekTo( 0 );

	bool bSame = true;
	while( bSame && !pMsgA->EOM( ))
	{
		bSame &= ( pMsgA->ReadBits( 32 ) == pMsgB->ReadBits( 32 ));
	}

	pMsgA->SeekTo( nPosA );
	pMsgB->SeekTo( nPosB );

	return bSame;
}

// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CPlayerMgr::CPlayerMgr()
	:  m_bServerAccurateRotation( false )
	, m_bSendCameraOffsetToServer( false )
{
	// RETAIL: Tron's allocates with plain new, inline (retail 0x10123400)
	m_pHeadBobMgr = new CHeadBobMgr;            ASSERT( 0 != m_pHeadBobMgr );
	m_pCameraOffsetMgr = new CCameraOffsetMgr;  ASSERT( 0 != m_pCameraOffsetMgr );
	m_pFlashLight = new CFlashLightPlayer;      ASSERT( 0 != m_pFlashLight );
	m_pGadgetDisabler = new CGadgetDisabler;    ASSERT( 0 != m_pGadgetDisabler );
	// RETAIL: Tron has no searcher (retail 0x10123400 allocates none; no
	// search sounds or strings)
	m_pMoveMgr = new CMoveMgr;                  ASSERT( 0 != m_pMoveMgr );
	m_pVisionModeMgr = new CVisionModeMgr;      ASSERT( 0 != m_pVisionModeMgr );
	m_pAttachButeMgr = new CAttachButeMgr;      ASSERT( 0 != m_pAttachButeMgr );
	m_pLeanMgr = new CLeanMgr;                  ASSERT( 0 != m_pLeanMgr );
	m_pClientWeaponMgr = new CClientWeaponMgr;  ASSERT( 0 != m_pClientWeaponMgr );
	m_pPlayerCamera = new CPlayerCamera;        ASSERT( 0 != m_pPlayerCamera );
	m_pPVAttachmentMgr = new CPlayerViewAttachmentMgr; ASSERT( 0 != m_pPVAttachmentMgr );

//	InitTargetMgr();

	m_hCamera                   = NULL;
	m_bStrafing                 = LTFALSE;
	m_fYawBackup                = 0.0f;
	m_fPitchBackup              = 0.0f;
	m_bRestoreOrientation       = LTFALSE;
	m_bAllowPlayerMovement      = LTTRUE;
	m_bLastAllowPlayerMovement  = LTTRUE;
	m_bWasUsingExternalCamera   = LTFALSE;
	m_bUsingExternalCamera      = LTFALSE;
	m_bCamIsListener            = LTFALSE;
	m_bCamera  = LTFALSE;
	m_bStartedPlaying			= LTFALSE;
	// RETAIL: Tron's (PlayerMgr.h)
	m_bTronUnknownA0			= LTFALSE;
	m_bTronUnknown1D7			= true;
	m_bTronUnknown1D9			= false;
	m_bTronUnknown1D8			= false;
	m_fTronUnknown1DC			= 5000.0f;
	m_vSVLightScale.Init(1, 1, 1);

	m_vShakeAmount.Init();
	m_bSpectatorMode        = LTFALSE;
	m_bInvisibleMode		= LTFALSE;

	m_fEarliestRespawnTime	= 0.0f;

	m_rRotation.Init();

	m_fPitch			= 0.0f;
	m_fYaw				= 0.0f;
	m_fRoll				= 0.0f;

	m_fPlayerPitch		= 0.0f;
	m_fPlayerYaw		= 0.0f;
	m_fPlayerRoll		= 0.0f;

	m_fModelAttachPitch	= 0.0f;
	m_fModelAttachYaw	= 0.0f;
	m_fModelAttachRoll	= 0.0f;

	m_fFireJitterPitch	= 0.0f;
	m_fFireJitterYaw	= 0.0f;

	m_dwPlayerFlags			= 0;
	m_ePlayerState			= PS_UNKNOWN;

	m_nZoomView				= 0;
	m_bZooming              = LTFALSE;
	m_bZoomingIn            = LTFALSE;
	m_fSaveLODScale			= DEFAULT_LOD_OFFSET;

	m_eCurContainerCode		= CC_NO_CONTAINER;
	m_nSoundFilterId		= 0;
	m_nGlobalSoundFilterId	= 0;
	// RETAIL: no safety net in Tron (m_bInSafetyNet is never set)

	m_fContainerStartTime	= -1.0f;
	m_fFovXFXDir			= 1.0f;

	m_vCurModelGlow.Init(127.0f, 127.0f, 127.0f);
	m_vMaxModelGlow.Init(255.0f, 255.0f, 255.0f);
	m_vMinModelGlow.Init(50.0f, 50.0f, 50.f);
	m_fModelGlowCycleTime	= 0.0f;
	m_bModelGlowCycleUp     = LTTRUE;

	m_bCameraPosInited      = LTFALSE;

	m_bPlayerUpdated		= LTFALSE;

	m_bCameraAttachedToHead	= LTFALSE;

	m_nPlayerInfoChangeFlags	= 0;
	m_fPlayerInfoLastSendTime	= 0.0f;
	m_fPlayerInfoLastFullSendTime = 0.0f;
	m_nPlayerInfoLastSent[1]	= 0;
	m_nPlayerInfoLastSent[0]	= 0;

	m_bUseWorldFog          = LTTRUE;
	m_hContainerSound       = NULL;

	m_bStartedDuckingDown   = LTFALSE;
	m_bStartedDuckingUp     = LTFALSE;
	m_fCamDuck				= 0.0f;
	m_fDuckDownV			= -75.0f;
	m_fDuckUpV				= 75.0f;
	m_fMaxDuckDistance		= -20.0f;
	m_fStartDuckTime		= 0.0f;

	m_nCarryingObject		= CFX_CARRY_NONE;
	m_bCanDropCarriedObject	= false;

 	m_fDistanceIndicatorPercent = -1.0f;
	// RETAIL: Tron's (PlayerMgr.h)
	m_szTronUnknown1B5[0]	= '\0';
	m_hDistanceIndicatorIcon	= LTNULL;

	// RETAIL: Tron's (PlayerMgr.h)
	m_bTronUnknown1D5		= false;
	m_bTronUnknown1D6		= false;
	m_nTronUnknown1E0		= 0;
	m_bTronUnknown1E4		= false;
	m_bTronUnknownB8		= false;
}

CPlayerMgr::~CPlayerMgr()
{
	// RETAIL: Tron's deletes with plain delete, inline (retail 0x101238a0)
	if ( m_pHeadBobMgr )
	{
		delete m_pHeadBobMgr;
		m_pHeadBobMgr = 0;
	}

	if ( m_pCameraOffsetMgr )
	{
		delete m_pCameraOffsetMgr;
		m_pCameraOffsetMgr = 0;
	}

	if ( m_pFlashLight )
	{
		delete m_pFlashLight;
		m_pFlashLight = 0;
	}

	if ( m_pGadgetDisabler )
	{
		delete m_pGadgetDisabler;
		m_pGadgetDisabler = 0;
	}

	if ( m_pMoveMgr )
	{
		delete m_pMoveMgr;
		m_pMoveMgr = 0;
	}

	if ( m_pVisionModeMgr )
	{
		delete m_pVisionModeMgr;
		m_pVisionModeMgr = 0;
	}

	if ( m_pAttachButeMgr )
	{
		delete m_pAttachButeMgr;
		m_pAttachButeMgr = 0;
	}

	if ( m_pLeanMgr )
	{
		delete m_pLeanMgr;
		m_pLeanMgr = 0;
	}

	if ( m_pClientWeaponMgr )
	{
		delete m_pClientWeaponMgr;
		m_pClientWeaponMgr = 0;
	}

	if ( m_pPlayerCamera )
	{
		delete m_pPlayerCamera;
		m_pPlayerCamera = 0;
	}

	if ( m_pPVAttachmentMgr )
	{
		delete m_pPVAttachmentMgr;
		m_pPVAttachmentMgr = 0;
	}

	// RETAIL: Tron's (retail 0x101238a0)
	if (s_DeathFX.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&s_DeathFX);
	}

	g_pPlayerMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
LTBOOL CPlayerMgr::Init()
{
	g_pPlayerMgr = this;

	InitTargetMgr();

	g_CV_CSendRate.Init(g_pLTClient, "CSendRate", NULL, DEFAULT_CSENDRATE);
	g_vtPlayerRotate.Init(g_pLTClient, "PlayerRotate", NULL, 1.0f);
	g_vtCamZoom1MaxDist.Init(g_pLTClient, "CamZoom1MaxDist", NULL, 100.0f);	// RETAIL: Tron's defaults
	g_vtCamZoom2MaxDist.Init(g_pLTClient, "CamZoom2MaxDist", NULL, 700.0f);
	g_vtFOVXNormal.Init(g_pLTClient, "FovX", NULL, 90.0f);
	g_vtFOVYNormal.Init(g_pLTClient, "FovY", NULL, 78.0f);
	// RETAIL: Tron's (retail 0x10123a30; the names are ours)
	g_vtFOVXCycle.Init(g_pLTClient, "FovCycleX", NULL, 90.0f);
	g_vtFOVYCycle.Init(g_pLTClient, "FovCycleY", NULL, 78.0f);
	g_vtFOVYMaxUW.Init(g_pLTClient, "FOVYUWMax", NULL, 78.0f);
	g_vtFOVYMinUW.Init(g_pLTClient, "FOVYUWMin", NULL, 77.0f);
	g_vtUWFOVRate.Init(g_pLTClient, "FOVUWRate", NULL, 0.3f);
	g_vtUseCamRecoil.Init(g_pLTClient, "CamRecoil", NULL, 0.0f);
	g_vtCamRecoilRecover.Init(g_pLTClient, "CamRecoilRecover", NULL, 0.3f);
	g_vtBaseCamRecoilPitch.Init(g_pLTClient, "CamRecoilBasePitch", NULL, 5.0f);
	g_vtMaxCamRecoilPitch.Init(g_pLTClient, "CamRecoilMaxPitch", NULL, 75.0f);
	g_vtBaseCamRecoilYaw.Init(g_pLTClient, "CamRecoilBaseYaw", NULL, 3.0f);
	g_vtMaxCamRecoilYaw.Init(g_pLTClient, "CamRecoilMaxYaw", NULL, 35.0f);
	g_vtCamRotInterpTime.Init(g_pLTClient, "CamRotInterpTime", NULL, 0.15f);
	g_vtRespawnWaitTime.Init(g_pLTClient, "RespawnWaitTime", NULL, 1.0f);
	g_vtMultiplayerRespawnWaitTime.Init(g_pLTClient, "RespawnMultiWaitTime", NULL, 1.0f);	// RETAIL: Tron's default; no doomsday
	g_vtScreenFadeInTime.Init(g_pLTClient, "ScreenFadeInTime", NULL, 3.0f);
	g_vtScreenFadeOutTime.Init(g_pLTClient, "ScreenFadeOutTime", NULL, 5.0f);
	g_vtChaseCamOffset.Init(g_pLTClient, "ChaseCamOffset", NULL, 50.0f);
	g_vtChaseCamPitchAdjust.Init(g_pLTClient, "ChaseCamPitchAdjust", NULL, 0.0f);
	g_vtChaseCamDistUp.Init(g_pLTClient, "ChaseCamDistUp", NULL, 10.0f);
	g_vtChaseCamDistBack.Init(g_pLTClient, "ChaseCamDistBack", NULL, 100.0f);
	g_vtFastTurnRate.Init(g_pLTClient, "FastTurnRate", NULL, 2.3f);
	g_vtNormalTurnRate.Init(g_pLTClient, "NormalTurnRate", NULL, 1.5f);
	g_vtLookUpRate.Init(g_pLTClient, "LookUpRate", NULL, 2.5f);
	g_vtCameraSwayXFreq.Init(g_pLTClient, "CameraSwayXFreq", NULL, 13.0f);
	g_vtCameraSwayYFreq.Init(g_pLTClient, "CameraSwayYFreq", NULL, 5.0f);
	g_vtCameraSwayXSpeed.Init(g_pLTClient, "CameraSwayXSpeed", NULL, 12.0f);
	g_vtCameraSwayYSpeed.Init(g_pLTClient, "CameraSwayYSpeed", NULL, 1.5f);
	g_vtCameraSwayDuckMult.Init(g_pLTClient, "CameraSwayCrouchMultiplier", NULL, 0.5f);
	g_vtModelGlowTime.Init(g_pLTClient, "ModelGlowTime", NULL, 1.5f);
	g_vtModelGlowMin.Init(g_pLTClient, "ModelGlowMin", NULL, -25.0f);
	g_vtModelGlowMax.Init(g_pLTClient, "ModelGlowMax", NULL, 75.0f);
    g_vtActivateOverride.Init(g_pLTClient, "ActivateOverride", " ", 0.0f);
    g_vtCamDamage.Init(g_pLTClient, "CamDamage", NULL, 1.0f);
    g_vtCamDamagePitch.Init(g_pLTClient, "CamDamagePitch", NULL, 1.0f);
    g_vtCamDamageRoll.Init(g_pLTClient, "CamDamageRoll", NULL, 1.0f);
    g_vtCamDamageTime1.Init(g_pLTClient, "CamDamageTime1", NULL, 0.1f);
    g_vtCamDamageTime2.Init(g_pLTClient, "CamDamageTime2", NULL, 0.25f);
    g_vtCamDamageMinPitchVal.Init(g_pLTClient, "CamDamageMinPitchVal", NULL, 5.0f);
    g_vtCamDamageMaxPitchVal.Init(g_pLTClient, "CamDamageMaxPitchVal", NULL, 20.0f);
    g_vtCamDamageMinRollVal.Init(g_pLTClient, "CamDamageMinRollVal", NULL, 5.0f);
    g_vtCamDamageMaxRollVal.Init(g_pLTClient, "CamDamageMaxRollVal", NULL, 20.0f);
    g_vtCamDamagePitchMin.Init(g_pLTClient, "CamDamagePitchMin", NULL, 0.7f);
    g_vtCamDamageRollMin.Init(g_pLTClient, "CamDamageRollMin", NULL, 0.7f);
    g_vtCamDamageFXOffsetX.Init(g_pLTClient, "CamDamageFXOffsetX", NULL, 5.0f);
    g_vtCamDamageFXOffsetY.Init(g_pLTClient, "CamDamageFXOffsetX", NULL, 5.0f);
    g_vtCamDamageFXOffsetZ.Init(g_pLTClient, "CamDamageFXOffsetX", NULL, 5.0f);
	g_vtAlwaysHUD.Init(g_pLTClient, "AlwaysHUD", NULL, 0.0f);
	g_vtDamageFadeRate.Init(g_pLTClient, "DamageFadeRate", NULL, 0.25f);	// RETAIL: Tron's default
	g_vtAdaptiveMouse.Init(g_pLTClient, "AdaptiveMouse", NULL, 0.0f);
	g_vtAdaptiveMouseMaxOffset.Init(g_pLTClient, "AdaptiveMouseMaxOffset", NULL, 0.1f);
	g_vtShowSoundFilterInfo.Init(g_pLTClient, "SoundFilterInfo", NULL, 0.0f);
	// RETAIL: Tron's, as Contract J.A.C.K.'s (UpdatePlayerInfo)
	g_vtCSendRateMin.Init(g_pLTClient, "CSendRate_Min", NULL, DEFAULT_CSENDRATE_MIN);

	// Default these to use values specified in the weapon...(just here for
	// tweaking values...)

	g_vtFireJitterDecayTime.Init(g_pLTClient, "FireJitterDecayTime", NULL, -1.0f);
	g_vtFireJitterMaxPitchDelta.Init(g_pLTClient, "FireJitterMaxPitchDelta", NULL, -1.0f);

	g_vSVModelColor = g_pLayoutMgr->GetSpyVisionModelColor();
	m_vSVLightScale = g_pLayoutMgr->GetSpyVisionLightScale();

	// Create the camera...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_ObjectType = OT_CAMERA;

	m_hCamera = g_pLTClient->CreateObject(&theStruct);
	_ASSERT(m_hCamera);

	ResetCamera();

	m_pHeadBobMgr->Init();
	m_pCameraOffsetMgr->Init();
	//m_pFlashLight no init
	//m_pGadgetDisabler no init
	//m_pSearcher
	m_pMoveMgr->Init();
	//m_pVisionModeMgr no init
	m_pAttachButeMgr->Init();
	m_pLeanMgr->Init();
	m_pClientWeaponMgr->Init();
	m_pPVAttachmentMgr->Init();

	s_fDeathDelay = g_pLayoutMgr->GetDeathDelay();

	// Player camera (non-1st person) stuff...

	if (m_pPlayerCamera->Init(g_pLTClient))
	{
		InitPlayerCamera();
		m_pPlayerCamera->GoFirstPerson(true);	// RETAIL: Tron's (retail 0x10123a30)
	}
	else
	{
		g_pGameClientShell->CSPrint ("Could not init player camera!");
	}

	ClearDamageSectors();

	m_bSpyVision = false;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CPlayerMgr::Term()
{
	// terminate the weapon mgr
	m_pClientWeaponMgr->Term();

	m_pPVAttachmentMgr->Term();

	if (m_hCamera)
	{
        g_pLTClient->RemoveObject(m_hCamera);
        m_hCamera = NULL;
	}

	// RETAIL: Tron's (retail 0x10124af0)
	m_cLastPlayerInfoMsg = LTNULL;
	m_cLastPlayerExtraMsg = LTNULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Save
//
//	PURPOSE:	Save the interface info
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	// save the class information
	m_pMoveMgr->Save(pMsg, eSaveDataState);
	m_pClientWeaponMgr->Save(pMsg);

	pMsg->Writebool(m_bSpectatorMode != LTFALSE);
	pMsg->WriteLTRotation(m_rRotation);
	pMsg->Writebool(m_bLastSent3rdPerson != LTFALSE);
	pMsg->Writebool(m_bAllowPlayerMovement != LTFALSE);
	pMsg->Writebool(m_bLastAllowPlayerMovement != LTFALSE);
	pMsg->Writebool(m_bWasUsingExternalCamera != LTFALSE);
	pMsg->Writebool(m_bUsingExternalCamera != LTFALSE);
	pMsg->Writebool(m_bTronUnknownA0 != LTFALSE);	// RETAIL: Tron's
	pMsg->Writeuint8(m_ePlayerState);
	pMsg->Writeuint8(m_nSoundFilterId);
	pMsg->Writeuint8(m_nGlobalSoundFilterId);

	// RETAIL: Tron's (retail 0x10124b60)
	pMsg->Writebool(m_bTronUnknown1D7);
	pMsg->Writebool(m_bTronUnknown1D8);
	pMsg->Writebool(m_bTronUnknown1D9);
	pMsg->Writefloat(m_fTronUnknown1DC);

	pMsg->Writebool(m_pFlashLight->IsOn() != LTFALSE);

	pMsg->Writefloat(m_fPitch);
	pMsg->Writefloat(m_fYaw);
	pMsg->Writefloat(m_fRoll);

	pMsg->Writefloat(m_fPlayerPitch);
	pMsg->Writefloat(m_fPlayerYaw);
	pMsg->Writefloat(m_fPlayerRoll);

	pMsg->Writefloat(m_fModelAttachPitch);
	pMsg->Writefloat(m_fModelAttachYaw);
	pMsg->Writefloat(m_fModelAttachRoll);

	m_fPitchBackup = m_fPitch;
	m_fYawBackup = m_fYaw;

    pMsg->Writefloat(m_fPitchBackup);
    pMsg->Writefloat(m_fYawBackup);

    pMsg->Writefloat(m_fFireJitterPitch);
    pMsg->Writefloat(m_fFireJitterYaw);
    pMsg->Writefloat(m_fContainerStartTime);
    pMsg->Writefloat(m_fFovXFXDir);
    pMsg->Writefloat(m_fSaveLODScale);

	
	// RETAIL: Tron's writes the carry state as a bool (retail 0x10124b60)
	pMsg->Writebool( m_nCarryingObject != CFX_CARRY_NONE );
	pMsg->Writebool( m_bCanDropCarriedObject );

	// RETAIL: Tron's (retail 0x10124b60)
	pMsg->WriteString( m_szTronUnknown1B5 );

	pMsg->Writebool( m_bCameraAttachedToHead != LTFALSE );

	pMsg->Writebool( m_pMoveMgr->DuckLock() ? true : false );

	pMsg->Writebool( m_pMoveMgr->IsDucking() ? true : false );

	// RETAIL: Tron's (retail 0x10124b60)
	pMsg->Writebool( m_bTronUnknown1D5 );
	pMsg->Writebool( m_bTronUnknown1D6 );
	pMsg->Writeuint8( m_nTronUnknown1E0 );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Load
//
//	PURPOSE:	Load the interface info
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState)
{
	// load the contained member classes
	m_pMoveMgr->Load(pMsg, eLoadDataState);
	m_pClientWeaponMgr->Load(pMsg);

	m_bSpectatorMode            = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_rRotation                 = pMsg->ReadLTRotation();
	m_bLastSent3rdPerson        = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_bAllowPlayerMovement      = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_bLastAllowPlayerMovement  = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_bWasUsingExternalCamera   = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_bUsingExternalCamera      = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_bTronUnknownA0            = pMsg->Readbool() ? LTTRUE : LTFALSE;	// RETAIL: Tron's
	m_ePlayerState              = (PlayerState) pMsg->Readuint8();
	m_nSoundFilterId            = pMsg->Readuint8();
	m_nGlobalSoundFilterId      = pMsg->Readuint8();

	// RETAIL: Tron's (retail 0x10124ee0)
	m_bTronUnknown1D7			= pMsg->Readbool();
	m_bTronUnknown1D8			= pMsg->Readbool();
	m_bTronUnknown1D9			= pMsg->Readbool();
	m_fTronUnknown1DC			= pMsg->Readfloat();

	if (pMsg->Readbool())
	{
		m_pFlashLight->TurnOn();
	}

	m_fPitch                    = pMsg->Readfloat();
	m_fYaw                      = pMsg->Readfloat();
	m_fRoll                     = pMsg->Readfloat();

	m_fPlayerPitch              = pMsg->Readfloat();
	m_fPlayerYaw                = pMsg->Readfloat();
	m_fPlayerRoll               = pMsg->Readfloat();

	m_fModelAttachPitch         = pMsg->Readfloat();
	m_fModelAttachYaw           = pMsg->Readfloat();
	m_fModelAttachRoll          = pMsg->Readfloat();

	m_fPitchBackup              = pMsg->Readfloat();
	m_fYawBackup                = pMsg->Readfloat();

    m_fFireJitterPitch          = pMsg->Readfloat();
    m_fFireJitterYaw	        = pMsg->Readfloat();
    m_fContainerStartTime       = pMsg->Readfloat();
    m_fFovXFXDir                = pMsg->Readfloat();
    m_fSaveLODScale             = pMsg->Readfloat();

	// RETAIL: Tron's reads the carry state as a bool (retail 0x10124ee0)
	m_nCarryingObject			= pMsg->Readbool() ? CFX_CARRY_BODY : CFX_CARRY_NONE;
	m_bCanDropCarriedObject		= pMsg->Readbool();

	// RETAIL: Tron's (retail 0x10124ee0)
	pMsg->ReadString( m_szTronUnknown1B5, sizeof(m_szTronUnknown1B5) );

	m_bCameraAttachedToHead		= pMsg->Readbool() ? LTTRUE : LTFALSE;

	m_pMoveMgr->SetDuckLock( pMsg->Readbool() ? LTTRUE : LTFALSE );

	m_pMoveMgr->SetDucking( pMsg->Readbool() ? LTTRUE : LTFALSE );

	// RETAIL: Tron's (retail 0x10124ee0)
	m_bTronUnknown1D5			= pMsg->Readbool();
	m_bTronUnknown1D6			= pMsg->Readbool();
	m_nTronUnknown1E0			= pMsg->Readuint8();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnEnterWorld()
//
//	PURPOSE:	Handle entering world
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::OnEnterWorld()
{
	m_vShakeAmount.Init();
	m_ePlayerState            = PS_UNKNOWN;

	m_bPlayerUpdated          = LTFALSE;
	m_nZoomView               = 0;
	m_bZooming                = LTFALSE;
	m_bZoomingIn              = LTFALSE;
	m_bTronUnknown1E4		  = false;	// RETAIL: Tron's

	m_eCurContainerCode       = CC_NO_CONTAINER;

	m_bCameraAttachedToHead	= LTFALSE;

	SetExternalCamera(LTFALSE);

	m_bCameraPosInited        = LTFALSE;

	m_nPlayerInfoChangeFlags |= CLIENTUPDATE_PLAYERROT | CLIENTUPDATE_ALLOWINPUT;

	m_pMoveMgr->OnEnterWorld();

	m_nCarryingObject		= CFX_CARRY_NONE;
	m_bCanDropCarriedObject	= false;

	// RETAIL: Tron's (retail 0x10125240)
	m_bTronUnknown1D5		= false;
	m_bTronUnknown1D6		= false;

	m_pClientWeaponMgr->OnEnterWorld();

	m_fTronUnknown1DC		= 5000.0f;	// RETAIL: Tron's
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnExitWorld()
//
//	PURPOSE:	Handle leaving world
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::OnExitWorld()
{
	m_bStartedPlaying = LTFALSE;

	ClearPlayerModes();

	m_pPlayerCamera->AttachToObject(NULL);   // Detatch camera

	m_pFlashLight->TurnOff();
	m_pMoveMgr->OnExitWorld();
	
	if (m_hContainerSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hContainerSound);
		m_hContainerSound = NULL;
	}

	m_pClientWeaponMgr->OnExitWorld();

	// RETAIL: Tron's (retail 0x101252d0)
	if (s_DeathFX.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&s_DeathFX);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	switch(messageID)
	{
		case MID_SHAKE_SCREEN:				HandleMsgShakeScreen			(pMsg);	break;
		case MID_PLAYER_STATE_CHANGE:		HandleMsgPlayerStateChange		(pMsg);	break;
		case MID_CLIENT_PLAYER_UPDATE:		HandleMsgClientPlayerUpdate		(pMsg);	break;
		case MID_PLAYER_DAMAGE:				HandleMsgPlayerDamage			(pMsg);	break;
		case MID_PLAYER_ORIENTATION:		HandleMsgPlayerOrientation		(pMsg);	break;
		case MID_WEAPON_CHANGE:				HandleMsgWeaponChange			(pMsg);	break;
		case MID_CHANGE_WORLDPROPERTIES:	HandleMsgChangeWorldProperties	(pMsg);	break;
		case MID_GADGETTARGET:				HandleMsgGadgetTarget			(pMsg); break;
		// RETAIL: no MID_SEARCH in Tron (retail 0x10125360)
		case MID_ADD_PUSHER:				HandleMsgAddPusher				(pMsg); break;
		case MID_OBJECTIVES_DATA:			HandleMsgObjectivesData			(pMsg); break;
		default:							return LTFALSE;	break;
	}
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnKeyDown(int key, int rep)
//
//	PURPOSE:	Handle key down notification
//				Try to avoid using OnKeyDown and OnKeyUp as they
//				are not portable functions
//
// ----------------------------------------------------------------------- //
LTBOOL CPlayerMgr::OnKeyDown(int key, int rep)
{
	if (!IsPlayerInWorld()) return LTFALSE;
	if (IsPlayerDead())
	{
		// See if we can respawn.  For SP, this just makes us wait
		// a little bit before we can go to the mission failure screen.
		if (g_pLTClient->GetTime() > m_fEarliestRespawnTime)
		{
			// Death is a mission failure in sp.
			if (!IsMultiplayerGame())
			{
				g_pMissionMgr->HandleMissionFailed();
				return LTTRUE;
			}
		}
	}

	// Are we playing a cinematic...

	if (m_bUsingExternalCamera)
	{
		if (key == VK_SPACE)
		{
			// Send an activate message to stop the cinemaitc...

			DoActivate();
			g_pLTClient->ClearInput();
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnKeyUp(int key)
//
//	PURPOSE:	Handle key up notification
//
// ----------------------------------------------------------------------- //
LTBOOL CPlayerMgr::OnKeyUp(int key)
{
	return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::PreUpdate()
//
//	PURPOSE:	Handle client pre-updates
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::PreUpdate()
{
	// Conditions in which we don't want to clear the screen

	if ((m_ePlayerState == PS_UNKNOWN && g_pGameClientShell->IsWorldLoaded()))
	{
		return;
	}

	// See if we're using an external camera now - if so, clear the screen
	// immediately, and add to the clearscreen count
	if (m_bUsingExternalCamera && !m_bWasUsingExternalCamera)
	{
        m_bWasUsingExternalCamera = LTTRUE;
		g_pInterfaceMgr->AddToClearScreenCount();
	}
	else if (m_bWasUsingExternalCamera && !m_bUsingExternalCamera)
	{
        m_bWasUsingExternalCamera = LTFALSE;
		g_pInterfaceMgr->AddToClearScreenCount();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Update()
//
//	PURPOSE:	Handle client updates
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::Update()
{
	// RETAIL: Tron's (retail 0x101255f0): once the player's FX is done, the
	// mission is failed (or, in the single player light cycle games, back
	// to their screens)
	if (m_bTronUnknown1D6)
	{
		if (s_DeathFX.GetInstance())
		{
			if (!s_DeathFX.GetInstance()->IsDone())
				return;

			if (s_DeathFX.IsValid())
			{
				g_pClientFXMgr->ShutdownClientFX(&s_DeathFX);
			}
		}

		switch (GetGameType())
		{
			case eGameTypeSingle:
			{
				g_pMissionMgr->HandleMissionFailed();
			}
			break;

			// RETAIL: Tron's two-argument SwitchToScreen (retail 0x100cc6a0)
			// is not yet in our tree
			case eGameTypeSingleLightCycleCircuit:
			{
				g_pInterfaceMgr->ChangeState(GS_SCREEN);
				CScreenMgr* pScreenMgr = g_pInterfaceMgr->GetScreenMgr();
				pScreenMgr->ClearHistory();
				g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
				pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
				pScreenMgr->AddScreenToHistory(SCREEN_ID_SINGLE);
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_CIRCUIT);
			}
			break;

			case eGameTypeSingleLightCycleCustom:
			{
				g_pInterfaceMgr->ChangeState(GS_SCREEN);
				CScreenMgr* pScreenMgr = g_pInterfaceMgr->GetScreenMgr();
				pScreenMgr->ClearHistory();
				g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
				pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
				pScreenMgr->AddScreenToHistory(SCREEN_ID_SINGLE);
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_CUSTOMMAP);
			}
			break;
		}

		return;
	}

	// Update models (powerups) glowing...

	UpdateModelGlow();

	UpdateDamage();

	UpdatePlayerDeath();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayerDeath()
//
//	PURPOSE:	RETAIL: Tron's (retail 0x10125710; the name is ours): NOLF2's
//				dead player handling of Update, waiting for the player's FX
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlayerDeath()
{
	if (IsPlayerDead())
	{
		if (!g_pGameClientShell->IsGamePaused())
		{
			if (s_DeathFX.GetInstance())
			{
				if (!s_DeathFX.GetInstance()->IsDone())
					return;

				if (s_DeathFX.IsValid())
				{
					g_pClientFXMgr->ShutdownClientFX(&s_DeathFX);
				}
			}

			if (g_pLTClient->GetTime() > m_fEarliestRespawnTime)
			{
				if (IsMultiplayerGame())
				{
					HandleRespawn();
				}
				else
				{
					g_pMissionMgr->HandleMissionFailed();
				}
			}
		}
	}
	else
	{
		if (s_DeathFX.IsValid())
		{
			g_pClientFXMgr->ShutdownClientFX(&s_DeathFX);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::FirstUpdate
//
//	PURPOSE:	Handle first update (each level)
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::FirstUpdate()
{
	// Force the player camera to update...

	UpdatePlayerCamera();

	// RETAIL: Tron's sets the global sound filter through the sound
	// manager (retail 0x101257d0)

 	m_nGlobalSoundFilterId = 0;

	char aGlobalSoundFilter[256] = {0};
	char* pGlobalSoundFilter = NULL;
	if (g_pLTClient->GetSConValueString("GlobalSoundFilter", aGlobalSoundFilter, sizeof(aGlobalSoundFilter)) == LT_OK)
	{
		pGlobalSoundFilter = aGlobalSoundFilter;
	}

	if (pGlobalSoundFilter && pGlobalSoundFilter[0])
	{
		SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter(pGlobalSoundFilter);
		if (pFilter)
		{
			m_nGlobalSoundFilterId = pFilter->nId;
			g_pClientSoundMgr->SetSoundFilter(m_nGlobalSoundFilterId);
		}
	}

	// Force us to re-evaluate what container we're in.  We call
	// UpdateContainers() first to make sure any container changes
	// have been accounted for, then we clear the container code
	// and force an update (this is done for underwater situations like
	// dying underwater and respawning, and also for picking up intelligence
	// items underwater)...
	UpdateContainers();
	ClearCurContainerCode();
	UpdateContainers();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::FirstPlayingUpdate()
//
//	PURPOSE:	Handle when the player first starts playing
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::FirstPlayingUpdate()
{
	// Set Initial cheats...
	if (g_pCheatMgr)
	{
        uint8 nNumCheats = g_pClientButeMgr->GetNumCheatAttributes();
		char strCheat[64];

        for (uint8 i=0; i < nNumCheats; i++)
		{
			strCheat[0] = '\0';
			g_pClientButeMgr->GetCheat(i, strCheat, ARRAY_LEN(strCheat));
			if (strCheat[0])
			{
				g_pCheatMgr->Check(strCheat);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlaying()
//
//	PURPOSE:	Handle updating playing (normal) game state
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlaying()
{
	// RETAIL: Tron's (retail 0x10125920)
	if (g_pGameClientShell->IsServerPaused())
	{
		UpdatePlayer();
		return;
	}

	// First time UpdatePlaying is called...

	if (!m_bStartedPlaying)
	{
        m_bStartedPlaying = LTTRUE;
		FirstPlayingUpdate();
	}

	// Update player movement...

	m_pMoveMgr->Update();


	// Update our camera offset mgr...

	m_pCameraOffsetMgr->Update();


	// Update Player...

	UpdatePlayer();


	// Keep track of what the player is doing...

	UpdatePlayerFlags();


	// Update head-bob/head-cant camera offsets...

	m_pHeadBobMgr->Update();


	// Update duck camera offset...

	UpdateDuck();


	// Update the camera's position...

	UpdateCamera();


	// See if we are tracking distance to an object..

	UpdateDistanceIndicator();


	// Update player leaning...

	m_pLeanMgr->Update();

	//updates dependant on camera position and rotation should be handled after this point

	// Update the targetting info...
	m_pTargetMgr->Update();

	// Update the Gadget Disabler...

	m_pGadgetDisabler->Update();

	// Update the vision modes...

	m_pVisionModeMgr->Update();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateFrame()
//
//	PURPOSE:	RETAIL: Tron's (retail 0x101259c0, called by the game client
//				shell's Update; the name is ours)
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateFrame()
{
	Update();
	UpdatePlaying();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::PostUpdate()
//
//	PURPOSE:	Handle post updates - after the scene is rendered
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::PostUpdate()
{
	// Update container effects...
	// RETAIL: Tron's does not check m_bStartedPlaying (retail 0x101259e0)
	UpdateContainers();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateDuck()
//
//	PURPOSE:	Update ducking camera offset
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateDuck()
{
	// RETAIL: Tron's, build 51's with NOLF2's dead player reset and start
	// times (retail 0x101259f0)

	// Can't duck when free movement...

    if (g_pPlayerMgr->GetMoveMgr()->IsBodyInLiquid() || 
		g_pPlayerMgr->GetMoveMgr()->IsBodyOnLadder() ||
		g_pPlayerMgr->IsPlayerDead() ||
        IsFreeMovement(m_eCurContainerCode)) 
	{
		// Reset ducking parameters...
		m_fCamDuck = 0.0f;
        m_bStartedDuckingDown = LTFALSE;
		return;
	}

    float fTime = g_pLTClient->GetTime();

	float fBodyMult = 1.0f;

	if ( (m_dwPlayerFlags & BC_CFLG_DUCK) || m_bCameraDip)
	{
        m_bStartedDuckingUp = LTFALSE;

		// See if the duck just started...

		if (!m_bStartedDuckingDown)
		{
            m_bStartedDuckingDown = LTTRUE;
			m_fStartDuckTime = fTime - g_pLTClient->GetFrameTime();
		}

		m_fCamDuck = m_fDuckDownV * (fTime - m_fStartDuckTime);

		if (m_fCamDuck < m_fMaxDuckDistance)
		{
			m_bCameraDip = false;
			m_fCamDuck = m_fMaxDuckDistance;
		}

	}
	else if (m_fCamDuck < 0.0) // Raise up
	{
        m_bStartedDuckingDown = LTFALSE;

		if (!m_bStartedDuckingUp)
		{
			m_fStartDuckTime = fTime - g_pLTClient->GetFrameTime();
            m_bStartedDuckingUp = LTTRUE;
		}

		if( GetCarryingObject() )
			fBodyMult = 0.01f;

		m_fCamDuck += m_fDuckUpV * (fTime - m_fStartDuckTime) * fBodyMult;

		if (m_fCamDuck > 0.0f)
		{
			m_fCamDuck = 0.0f;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgShakeScreen()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgShakeScreen (ILTMessage_Read *pMsg)
{
	LTVector vAmount = pMsg->ReadLTVector();
	ShakeScreen(vAmount);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ShakeScreen()
//
//	PURPOSE:	Shanke, rattle, and roll
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::ShakeScreen(LTVector vShake)
{
	// Add...

	VEC_ADD(m_vShakeAmount, m_vShakeAmount, vShake);

	if (m_vShakeAmount.x > MAX_SHAKE_AMOUNT) m_vShakeAmount.x = MAX_SHAKE_AMOUNT;
	if (m_vShakeAmount.y > MAX_SHAKE_AMOUNT) m_vShakeAmount.y = MAX_SHAKE_AMOUNT;
	if (m_vShakeAmount.z > MAX_SHAKE_AMOUNT) m_vShakeAmount.z = MAX_SHAKE_AMOUNT;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgClientPlayerUpdate()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgClientPlayerUpdate (ILTMessage_Read *pMsg)
{
    uint16 changeFlags = pMsg->Readuint16();

	m_pMoveMgr->OnPhysicsUpdate(changeFlags, pMsg);

	if (changeFlags & PSTATE_INTERFACE)
	{
		uint8 nFlags	= pMsg->Readuint8();
		float fHideTime	= pMsg->Readfloat(); 

		LTBOOL bHiding		= !!(nFlags & PSTATE_INT_HIDING);
		LTBOOL bHidden		= !!(nFlags & PSTATE_INT_HIDDEN);
		LTBOOL bCantHide	= !!(nFlags & PSTATE_INT_CANTHIDE);

		g_pPlayerStats->UpdateHiding( bHiding, bHidden, bCantHide, fHideTime );

		// RETAIL: Tron's, as build 51's (retail 0x10125c70)
		SetCarryingObject( !!(nFlags & PSTATE_INT_CARRYING) );

		SetCanDropCarriedObject( !!(nFlags & PSTATE_INT_CAN_DROP) );
	}


	m_bPlayerUpdated = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgWeaponChange()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgWeaponChange (ILTMessage_Read *pMsg)
{
	ChangeWeapon(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgPlayerDamage()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //


void CPlayerMgr::HandleMsgPlayerDamage (ILTMessage_Read *pMsg)
{
	static const float kfSectorSize = MATH_PI * 2.0f / (float)kNumDamageSectors;
	if (!pMsg || g_bScreenShotMode) return;

    LTVector vDir = pMsg->ReadLTVector();
    DamageType eType = (DamageType) pMsg->Readuint8();
    LTBOOL bTookHealth = (LTBOOL) pMsg->Readuint8();

	// No need to handle a damage message if the player is dead...
	
	if( IsPlayerDead() )
		return;
	
    float fPercent = vDir.Mag();

	uint8 damageSector = kNumDamageSectors;

	// [KLS 3/5/02] - If we were damaged, determine the direction the damage came from...
	if (fPercent > 0.0f)
	{

		LTRotation rRot;
		GetPlayerRotation(rRot);

		vDir.Normalize();
		float fDF = rRot.Forward().Dot(vDir);
		float fDR = rRot.Right().Dot(vDir);

		float damageAngle = MATH_PI+(float)atan2(fDF,fDR);
		damageSector = (uint8)(damageAngle / kfSectorSize); 

		m_fDamage[damageSector] += fPercent;

		if (m_fDamage[damageSector] > 1.0f)
			m_fDamage[damageSector] = 1.0f;

	}
	

/*******************************
/****** Damage Sector Map ******
	            3
		     4     2

           5         1

		  6           0

		   7         11

			 8     10
			    9
*******************************/

	
	// Do some camera FX for taking armor and taking health...
	// Tie this into the DamageSector at some point??

	DamageFlags dmgFlag = DamageTypeToFlag( eType );

	if( bTookHealth )
	{
		// Play the taking health fx for the DamageFX associated with the damage type...

		DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
		while( pDamageFX )
		{
			// Test the damage flags against the DamageFX...

			if( dmgFlag & pDamageFX->m_nDamageFlag || pDamageFX->m_vtTestFX.GetFloat() > 0.0f )
			{
				CLIENTFX_CREATESTRUCT fxInit( pDamageFX->m_szTakingHealthFXName, FXFLAG_REALLYCLOSE, LTVector(0,0,0) ); 
				g_pClientFXMgr->CreateClientFX( LTNULL, fxInit, LTTRUE );
			}
			
			pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
		}
	}
	else
	{
		// Play the taking armor fx for the DamageFX associated with the damage type...

		DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
		while( pDamageFX )
		{
			// Test the damage flags against the DamageFX...

			if( dmgFlag & pDamageFX->m_nDamageFlag || pDamageFX->m_vtTestFX.GetFloat() > 0.0f )
			{
				CLIENTFX_CREATESTRUCT fxInit( pDamageFX->m_szTakingArmorFXName, FXFLAG_REALLYCLOSE, LTVector(0,0,0) ); 
				g_pClientFXMgr->CreateClientFX( LTNULL, fxInit, LTTRUE );
			}
			
			pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
		}
	}

	// Tilt the camera based on the direction the damage came from...

	if (kNumDamageSectors != damageSector && IsJarCameraType(eType) && 
		m_pPlayerCamera->IsFirstPerson() && g_vtCamDamage.GetFloat() > 0.0f)
	{
		CameraDelta delta;

		if (g_vtCamDamagePitch.GetFloat() > 0.0f)
		{
			float fPitchAngle = g_vtCamDamageMinPitchVal.GetFloat() + ((g_vtCamDamageMaxPitchVal.GetFloat() - g_vtCamDamageMinPitchVal.GetFloat()) * fPercent);
	
			if (damageSector > 7 && damageSector <11)
			{
				delta.Pitch.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Pitch.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Pitch.eWave1	= Wave_SlowOff;
				delta.Pitch.eWave2	= Wave_SlowOff;
				delta.Pitch.fVar	= DEG2RAD(fPitchAngle);
			}
			else if (damageSector > 1 && damageSector < 5)
			{
				delta.Pitch.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Pitch.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Pitch.eWave1	= Wave_SlowOff;
				delta.Pitch.eWave2	= Wave_SlowOff;
				delta.Pitch.fVar	= -DEG2RAD(fPitchAngle);
			}
		}

		if (g_vtCamDamageRoll.GetFloat() > 0.0f)
		{
			float fRollAngle = g_vtCamDamageMinRollVal.GetFloat() + ((g_vtCamDamageMaxRollVal.GetFloat() - g_vtCamDamageMinRollVal.GetFloat()) * fPercent);

			if (damageSector > 4 && damageSector < 8)
			{
				delta.Roll.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Roll.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Roll.eWave1	= Wave_SlowOff;
				delta.Roll.eWave2	= Wave_SlowOff;
				delta.Roll.fVar		= DEG2RAD(fRollAngle);
			}
			else if (damageSector < 2 || damageSector  == 11)
			{
				delta.Roll.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Roll.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Roll.eWave1	= Wave_SlowOff;
				delta.Roll.eWave2	= Wave_SlowOff;
				delta.Roll.fVar		= -DEG2RAD(fRollAngle);
			}
		}

		m_pCameraOffsetMgr->AddDelta(delta);
	}


	float fSlow = SlowMovementDuration(eType);
	if (fSlow > 0.0f)
	{
		m_pMoveMgr->AddDamagePenalty(fSlow);
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Handle()
//
//	PURPOSE:	This handles the message sent by the world properties
//				indicating that they have changed and that the player needs
//				to sync to the console variables on the server
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgChangeWorldProperties (ILTMessage_Read *pMsg)
{
	g_pGameClientShell->ResetDynamicWorldProperties(m_bUseWorldFog);
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Handle()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgPlayerOrientation (ILTMessage_Read *pMsg)
{
	// Set our pitch, yaw, and roll according to the players...

    uint8		nID = pMsg->Readuint8();
	LTVector	vVec = pMsg->ReadLTVector();

	switch( nID )
	{
		case MID_ORIENTATION_ALL :
		{
			m_fPitch	= vVec.x;
			m_fYaw		= vVec.y;
			m_fRoll		= vVec.z;
		}
		break;

		case MID_ORIENTATION_YAW :
		{
			m_fYaw		= vVec.y;
		}
		break;

		default : break;
	};

	
	m_fPlayerPitch	= m_fPitch;
	m_fPlayerYaw	= m_fYaw;
	m_fPlayerRoll	= m_fRoll;
	m_fYawBackup	= m_fYaw;
	m_fPitchBackup	= m_fPitch;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::HandleMsgGadgetTarget
//
//  PURPOSE:	Just pass the message on the the disabler object
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgGadgetTarget( ILTMessage_Read *pMsg )
{
	m_pGadgetDisabler->OnGadgetTargetMessage( pMsg );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::HandleMsgSearch
//
//  PURPOSE:	Just pass the message on the the searcher object
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgSearch( ILTMessage_Read *pMsg )
{
	// RETAIL: no searcher in Tron (PORT: NOLF2's message handler kept, inert)
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::HandleMsgAddPusher
//
//  PURPOSE:	Add a pusher to the MoveMgr.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgAddPusher( ILTMessage_Read *pMsg )
{
    LTVector vPos		= pMsg->ReadCompLTVector();
	float fRadius		= pMsg->Readfloat();
	float fStartDelay = pMsg->Readfloat();
	float fDuration	= pMsg->Readfloat();
	float fStrength	= pMsg->Readfloat();

	g_pPlayerMgr->GetMoveMgr()->AddPusher( vPos, fRadius, fStartDelay, fDuration, fStrength );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::HandleMsgObjectivesData
//
//  PURPOSE:	Set our objectives obtained from the server...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgObjectivesData( ILTMessage_Read *pMsg )
{
	g_pPlayerStats->OnObjectivesDataMessage( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCamera()
//
//	PURPOSE:	Update the camera position/rotation
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateCamera()
{
	//handle updating the camera, as long as we aren't paused
	if(!g_pGameClientShell->IsGamePaused() || !m_bCameraPosInited)
	{
		// Update the sway...

		if (IsZoomed())
		{
			UpdateCameraSway();
		}

		// RETAIL: Tron's (retail 0x10126500): +0xa0 holds the camera
		if (!m_bTronUnknownA0)
		{
			// Update the camera's position and rotation..

			UpdateAlternativeCamera();


			// Update the player camera...

			UpdatePlayerCamera();


			if (!m_bUsingExternalCamera)
			{
				if( IsMultiplayerGame() || g_pInterfaceMgr->AllowCameraMovement() )
				{
					UpdateCameraPosition();
				}
			}

			if (g_pInterfaceMgr->AllowCameraRotation() )
			{
				CalculateCameraRotation();
				UpdateCameraRotation();
			}
		}
		else if (!m_bCameraPosInited)
		{
			UpdateCameraPosition();
		}

		if (m_bUsingExternalCamera)
		{
			HandleZoomChange(LTTRUE);
		}

		// Update zoom if applicable...

		if (m_bZooming)
		{
			UpdateCameraZoom();
		}

		// Update any camera displacement.
		if (!m_bTronUnknownA0)
		{
			UpdateCameraDisplacement();
		}

		// Make sure the player gets updated

 		if (IsMultiplayerGame() || g_pInterfaceMgr->AllowCameraMovement())
		{
			UpdatePlayerInfo(true);
		}
	}
	// Remember to continue updating the server if we're in multiplayer
	else if (IsMultiplayerGame() && g_pGameClientShell->IsGamePaused())
	{
		UpdatePlayerInfo(false);
	}
}

 

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCameraDisplacement
//
//	PURPOSE:	Update any camera displacement.
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateCameraDisplacement()
{
	LTVector vDisplacement( 0.0f, 0.0f, 0.0f );
	LTVector vAdd;

	vAdd.Init( );
	UpdateCameraShake( vAdd );
	vDisplacement += vAdd;
	vAdd.Init( );
	UpdateVehicleCamera( vAdd );
	vDisplacement += vAdd;

	if (!g_pInterfaceMgr->AllowCameraMovement())
		return;

	// Check if no significant movement occurred.
	if( vDisplacement.LengthSquared( ) < 0.01f )
		return;

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vPos);
	vPos += vDisplacement;
	g_pLTClient->SetObjectPos(m_hCamera, &vPos);

	//TODO: move this code into the weapon class
	HLOCALOBJ hWeapon = 0;
	IClientWeaponBase *pClientWeapon = m_pClientWeaponMgr->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		hWeapon = pClientWeapon->GetHandle();
	}
	if (!hWeapon) return;

	g_pLTClient->GetObjectPos(hWeapon, &vPos);
	vDisplacement += LTVector(0.95f, 0.95f, 0.95f);
	vPos += vDisplacement;
	g_pLTClient->SetObjectPos(hWeapon, &vPos);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCameraShake
//
//	PURPOSE:	Update the camera's shake
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateCameraShake( LTVector& vDisplacement )
{
	// Decay...

	float fDecayAmount = 2.0f * g_pGameClientShell->GetFrameTime();

	m_vShakeAmount.x -= fDecayAmount;
	m_vShakeAmount.y -= fDecayAmount;
	m_vShakeAmount.z -= fDecayAmount;

	if (m_vShakeAmount.x < 0.0f) m_vShakeAmount.x = 0.0f;
	if (m_vShakeAmount.y < 0.0f) m_vShakeAmount.y = 0.0f;
	if (m_vShakeAmount.z < 0.0f) m_vShakeAmount.z = 0.0f;


	if (m_vShakeAmount.x <= 0.0f && m_vShakeAmount.y <= 0.0f && m_vShakeAmount.z <= 0.0f) return;


	// Apply...

	float faddX = GetRandom(-1.0f, 1.0f) * m_vShakeAmount.x * 3.0f;
	float faddY = GetRandom(-1.0f, 1.0f) * m_vShakeAmount.y * 3.0f;
	float faddZ = GetRandom(-1.0f, 1.0f) * m_vShakeAmount.z * 3.0f;

	LTVector vAdd;
	vDisplacement.Init(faddX, faddY, faddZ);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateVehicleCamera
//
//	PURPOSE:	Update the vehicle's camera displacement
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateVehicleCamera( LTVector& vDisplacement )
{
	// Check if we're not in vehicle mode.
	if( !m_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
		return;

	m_pMoveMgr->GetVehicleMgr()->CalculateVehicleCameraDisplacment( vDisplacement );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCameraSway
//
//	PURPOSE:	Update the camera's sway
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateCameraSway()
{
	// Apply...
    float swayAmount = g_pGameClientShell->GetFrameTime() / 1000.0f;

    float tm  = g_pLTClient->GetTime()/10.0f;

	// Adjust if ducking...
	float fMult = (m_dwPlayerFlags & BC_CFLG_DUCK) ? g_vtCameraSwayDuckMult.GetFloat() : 1.0f;

	//Adjust for skills
	// PORT: Tron's player stats give the zoom sway through their virtual at
	// +0x13c (a pointer to a float at +4, as build 51's GetAimModifiers()->
	// fZoomSway; retail 0x10126980), which ours have not yet
	fMult *= g_pPlayerStats->GetSkillModifier(SKL_AIM,AimModifiers::eZoomSway);

	// RETAIL: no damage FX adjustment in Tron

    float faddP = fMult * g_vtCameraSwayYSpeed.GetFloat() * (float)sin(tm*g_vtCameraSwayYFreq.GetFloat()) * swayAmount;
    float faddY = fMult * g_vtCameraSwayXSpeed.GetFloat() * (float)sin(tm*g_vtCameraSwayXFreq.GetFloat()) * swayAmount;

	m_fPitch += faddP;
	m_fYaw	 += faddY;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetExternalCamera()
//
//	PURPOSE:	Turn on/off external camera mode
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetExternalCamera(LTBOOL bExternal)
{
	// RETAIL: Tron's leaves the crosshair alone, and goes first person at
	// once (retail 0x10126ae0)
	if (bExternal && m_pPlayerCamera->IsFirstPerson())
	{
		g_pDamageFXMgr->Clear();
		
		m_pClientWeaponMgr->DisableWeapons();
		
		ShowPlayer(LTTRUE);
		m_pPlayerCamera->GoChaseMode();
		m_pPlayerCamera->CameraUpdate(0.0f);
	}
	else if (!bExternal && !m_pPlayerCamera->IsFirstPerson()) // Go Internal
	{
		m_pClientWeaponMgr->EnableWeapons();

		// We don't want to accidently fire off a round or two comming out of a cinematic...
		
		IClientWeaponBase *pWeapon = m_pClientWeaponMgr->GetCurrentClientWeapon();
		if( pWeapon )
		{
			pWeapon->ClearFiring();
		}

		ShowPlayer(LTFALSE);
		m_pPlayerCamera->GoFirstPerson(true);
		m_pPlayerCamera->CameraUpdate(0.0f);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateAlternativeCamera()
//
//	PURPOSE:	Update the camera using an alternative camera
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerMgr::UpdateAlternativeCamera()
{
	m_bLastAllowPlayerMovement = m_bAllowPlayerMovement;
    m_bAllowPlayerMovement     = LTTRUE;

    HOBJECT hObj = NULL;

	// See if we should use an alternative camera position...

	CSpecialFXList* pCameraList = g_pGameClientShell->GetSFXMgr()->GetCameraList();
	if (pCameraList)
	{
		int nNum = pCameraList->GetSize();

		for (int i=0; i < nNum; i++)
		{
			CCameraFX* pCamFX = (CCameraFX*)(*pCameraList)[i];
			if (!pCamFX) continue;

			hObj = pCamFX->GetServerObj();

			if (hObj)
			{
                uint32 dwUsrFlags;
                g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUsrFlags);

				if (dwUsrFlags & USRFLG_CAMERA_LIVE)
				{
                    g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderText);

                    SetExternalCamera(LTTRUE);

					m_bAllowPlayerMovement = pCamFX->AllowPlayerMovement();

                    LTVector vPos;
					g_pLTClient->GetObjectPos(hObj, &vPos);
					g_pLTClient->SetObjectPos(m_hCamera, &vPos);
                    m_bCameraPosInited = LTTRUE;

                    LTRotation rRot;
					g_pLTClient->GetObjectRotation(hObj, &rRot);
					g_pLTClient->SetObjectRotation(m_hCamera, &rRot);

					// Always set the camera as the listener, so ambient sounds are
					// heard from the camera position.
					m_bCamIsListener = LTTRUE;

					((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetListener(LTFALSE, &vPos, &rRot, (m_bCamIsListener ? true : false));

					// Initialize the cinematic camera

					s_nLastCamType = pCamFX->GetType();

					pCamFX->UpdateFOV();

					float	fFovX = pCamFX->GetFovX();
					float	fFovY = pCamFX->GetFovY();

					TurnOnAlternativeCamera(s_nLastCamType, fFovX, fFovY);

                    return LTTRUE;
				}
			}
		}
	}


	// Okay, we're no longer using an external camera...

	if (m_bUsingExternalCamera)
	{
		TurnOffAlternativeCamera(s_nLastCamType);
	}


    return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::TurnOnAlternativeCamera()
//
//	PURPOSE:	Set up using an alternative camera
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::TurnOnAlternativeCamera(uint8 nCamType, float fFovX, float fFovY)
{
	g_pInterfaceMgr->SetLetterBox((nCamType == CT_CINEMATIC));

	SetCameraFOV( DEG2RAD(fFovX), DEG2RAD(fFovY) ); 

	if (!m_bUsingExternalCamera)
	{
		//g_pLTClient->CPrint("TURNING ALTERNATIVE CAMERA: ON");

		// Make sure we clear whatever was on the screen before
		// we switch to this camera...
		HandleZoomChange( LTTRUE );

		g_pInterfaceMgr->ClosePopup();

		m_pClientWeaponMgr->DisableWeapons();

		// Make sure we're using the highest model lods
        g_pLTClient->RunConsoleString("+ModelLODOffset -10");
	}

	m_bUsingExternalCamera = LTTRUE;

	// All the number of model shadows in cinematics to be much higher

	if(!g_bCinChangedNumModelShadows)
	{
		g_nCinSaveNumModelShadows = GetConsoleInt("ModelShadow_Proj_MaxShadowsPerFrame", 0);
		WriteConsoleInt("ModelShadow_Proj_MaxShadowsPerFrame", g_kMaxNumberOfCinShadows);

		g_bCinChangedNumModelShadows = true;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::TurnOffAlternativeCamera()
//
//	PURPOSE:	Turn off the alternative camera mode
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::TurnOffAlternativeCamera(uint8 nCamType)
{
    //g_pLTClient->CPrint("TURNING ALTERNATIVE CAMERA: OFF");

	g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderFull);
    m_bUsingExternalCamera = LTFALSE;

	// Set the listener back to the client...

    ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetListener(LTTRUE, NULL, NULL, LTTRUE);

	// Force 1st person...

    SetExternalCamera(LTFALSE);

	// enable the weapons
	m_pClientWeaponMgr->EnableWeapons();

	// turn off the letter box
	g_pInterfaceMgr->SetLetterBox(LTFALSE);

	// Reset the normal FOV

	SetCameraFOV(DEG2RAD(g_vtFOVXNormal.GetFloat()), DEG2RAD(g_vtFOVYNormal.GetFloat()));

	// Make sure we're using the normal model lod...
    g_pLTClient->RunConsoleString("+ModelLODOffset 0");

	// Set number of shadows back to whatever they were set to before...

	if(g_bCinChangedNumModelShadows)
	{
		WriteConsoleInt("ModelShadow_Proj_MaxShadowsPerFrame", g_nCinSaveNumModelShadows);
		g_bCinChangedNumModelShadows = false;
	}

	// RETAIL: Tron's turns the projected model shadows off again (retail
	// 0x10126e80)
	g_pLTClient->RunConsoleString("ModelShadow_Proj_Enable 0");
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CPlayerMgr::GetCurrentClientWeapon()
//
//	PURPOSE:	Return a pointer to the player's bucurrent weapon
//
// ----------------------------------------------------------------------- //

IClientWeaponBase* CPlayerMgr::GetCurrentClientWeapon() const
{
	return m_pClientWeaponMgr->GetCurrentClientWeapon();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::InCameraGadgetRange
//
//	PURPOSE:	See if the given object is in the camera gadget's range
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerMgr::InCameraGadgetRange(HOBJECT hObj)
{
	if (!hObj || !m_hCamera) return LTFALSE;

	IClientWeaponBase *pClientWeapon = m_pClientWeaponMgr->GetCurrentClientWeapon();
	if ( pClientWeapon && !m_pClientWeaponMgr->WeaponsEnabled() )
	{
		return LTFALSE;
	}

	LTVector vCamPos, vObjPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);
	g_pLTClient->GetObjectPos(hObj, &vObjPos);

	LTVector vDist = vCamPos - vObjPos;
	float fDist = vDist.Length();

	// g_pLTClient->CPrint("fDist = %.2f, Zoom View = %d", fDist, m_nZoomView);

	if (fDist < g_vtCamZoom1MaxDist.GetFloat())
	{
		return LTTRUE;
	}
	else if (fDist < g_vtCamZoom2MaxDist.GetFloat())
	{
		if (m_nZoomView > 0) return LTTRUE;
	}
	else
	{
		if (m_nZoomView > 1) return LTTRUE;
	}

	// Not zoomed in enough...
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateWeaponModel()
//
//	PURPOSE:	Update the weapon model
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateWeaponModel()
{
	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;


	// Decay weapon recoil...

	DecayWeaponRecoil();


	// If possible, get these values from the camera, because it
	// is more up-to-date...

	LTRotation rRot;
	LTVector vPos(0, 0, 0);

	// Weapon model pos/rot is relative to camera now...
	g_pLTClient->GetObjectPos(m_hCamera, &vPos);
	g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

	if (!m_pPlayerCamera->IsFirstPerson() || m_bUsingExternalCamera)
	{
		// Use the gun's flash orientation...

		GetAttachmentSocketTransform(hPlayerObj, "Flash", vPos, rRot);
	}


	// If we aren't dead, and we aren't in the middle of changing weapons,
	// let us fire.

	FireType eFireType = FT_NORMAL_FIRE;
	bool bFire = false;

	// only fire if:
	//    fire key down (alt firing is disabled) AND
	//    player is not choosing ammo AND
	//    player is not choosing a weapon AND
	//    player is not dead AND
	//    player is not in spectator mode
	if ( ( m_dwPlayerFlags & BC_CFLG_FIRING ) &&
	     ( !g_pInterfaceMgr->IsChoosingAmmo() ) &&
	     ( !g_pInterfaceMgr->IsChoosingWeapon() ) &&
	     ( !IsPlayerDead() ) &&
	     ( !m_bSpectatorMode ) &&
		 ( g_pInterfaceMgr->GetGameState() == GS_PLAYING ) )
	{
		bFire = true;
	}

	// Update the model position and state...
	WeaponState eWeaponState = m_pClientWeaponMgr->Update( rRot, vPos, bFire, eFireType );

	// Do fire camera jitter...

	if (FiredWeapon(eWeaponState) && m_pPlayerCamera->IsFirstPerson())
	{
		StartWeaponRecoil();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::StartWeaponRecoil()
//
//	PURPOSE:	Start the weapon recoiling...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::StartWeaponRecoil()
{
	float fMaxPitch = g_vtFireJitterMaxPitchDelta.GetFloat();

	if (g_vtUseCamRecoil.GetFloat() > 0.0f)
	{
		// Move view up a bit...

		float fPCorrect = 1.0f - (m_fFireJitterPitch / DEG2RAD(g_vtMaxCamRecoilPitch.GetFloat()));
		float fPVal = GetRandom(0.5f,2.0f) * DEG2RAD(g_vtBaseCamRecoilPitch.GetFloat()) * fPCorrect;
		m_fFireJitterPitch += fPVal;
		m_fPitch -= fPVal;

		// Move view left/right a bit...

		float fYawDiff = (float)fabs(m_fFireJitterYaw);
		float fYCorrect = 1.0f - (fYawDiff / DEG2RAD(g_vtMaxCamRecoilYaw.GetFloat()));
		float fYVal = GetRandom(-2.0f,2.0f) * DEG2RAD(g_vtBaseCamRecoilYaw.GetFloat()) * fYCorrect;
		m_fFireJitterYaw += fYVal;
		m_fYaw -= fYVal;
	}
	else
	{
		// Move view up a bit...(based on the current weapon/ammo type)

		WEAPON const *pWeaponData = 0;
		AMMO const *pAmmoData	= 0;
		IClientWeaponBase *pClientWeapon = m_pClientWeaponMgr->GetCurrentClientWeapon();
		if ( pClientWeapon )
		{
			pWeaponData = pClientWeapon->GetWeapon();
			pAmmoData = pClientWeapon->GetAmmo();
		}

		if (pWeaponData && pAmmoData)
		{
			if (fMaxPitch < 0.0f)
			{
				fMaxPitch = pWeaponData->fFireRecoilPitch * pAmmoData->fFireRecoilMult;
			}
		}

		float fVal = (float)DEG2RAD(fMaxPitch) - m_fFireJitterPitch;
		m_fFireJitterPitch += fVal;
		m_fPitch -= fVal;
	}

	if (fMaxPitch > 0.0f)
	{
		// Shake the screen if it isn't shaking...

		if (m_vShakeAmount.x < 0.1f && m_vShakeAmount.y < 0.1f && m_vShakeAmount.z < 0.1f)
		{
			LTVector vShake(0.1f, 0.1f, 0.1f);
			ShakeScreen(vShake);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::DecayWeaponRecoil()
//
//	PURPOSE:	Decay the weapon's recoil...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::DecayWeaponRecoil()
{
	// Decay firing jitter if necessary...

	if (g_vtUseCamRecoil.GetFloat() > 0.0f)
	{
		if (m_fFireJitterPitch > 0.0f)
		{
			float fCorrect = 1.0f + m_fFireJitterPitch / DEG2RAD(g_vtMaxCamRecoilPitch.GetFloat());
			float fVal = (g_pGameClientShell->GetFrameTime() * g_vtCamRecoilRecover.GetFloat()) * fCorrect;

			if (m_fFireJitterPitch < fVal)
			{
				fVal = m_fFireJitterPitch;
			}

			m_fFireJitterPitch -= fVal;
			m_fPitch += fVal;
		}

		float fYawDiff = (float)fabs(m_fFireJitterYaw);
		if (fYawDiff > 0.0f)
		{
			float fCorrect = 1.0f + fYawDiff / DEG2RAD(g_vtMaxCamRecoilYaw.GetFloat());
			float fVal = (g_pGameClientShell->GetFrameTime() * g_vtCamRecoilRecover.GetFloat()) * fCorrect;

			if (fYawDiff < fVal)
			{
				fVal = fYawDiff;
			}
			if (m_fFireJitterYaw < 0.0f)
				fVal *= -1.0f;

			m_fFireJitterYaw -= fVal;
			m_fYaw += fVal;
		}
	}
	else
	{
		if (m_fFireJitterPitch > 0.0f)
		{
			float fVal = m_fFireJitterPitch;

			// get the weapon and ammo data
			WEAPON const *pWeaponData = 0;
			AMMO const *pAmmoData	= 0;
			IClientWeaponBase *pClientWeapon = m_pClientWeaponMgr->GetCurrentClientWeapon();
			if ( pClientWeapon )
			{
				pWeaponData = pClientWeapon->GetWeapon();
				pAmmoData = pClientWeapon->GetAmmo();
			}

			float fMaxPitch  = g_vtFireJitterMaxPitchDelta.GetFloat();
			float fTotalTime = g_vtFireJitterDecayTime.GetFloat();

			if (pWeaponData && pAmmoData)
			{
				if (fMaxPitch < 0.0f)
				{
					fMaxPitch = pWeaponData->fFireRecoilPitch * pAmmoData->fFireRecoilMult;
				}

				if (fTotalTime < 0.0f)
				{
					fTotalTime = pWeaponData->fFireRecoilDecay;
				}
			}

			if (fTotalTime > 0.01)
			{
				fVal = g_pGameClientShell->GetFrameTime() * (DEG2RAD(fMaxPitch) / fTotalTime);
			}

			if (m_fFireJitterPitch - fVal < 0.0f)
			{
				fVal = m_fFireJitterPitch;
			}

			m_fFireJitterPitch -= fVal;
			m_fPitch += fVal;
		}

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetSpectatorMode()
//
//	PURPOSE:	Turn spectator mode on/off
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetSpectatorMode(LTBOOL bOn)
{
	m_bSpectatorMode = bOn;
	
	// Don't show stats in spectator mode...
	// Unless "alwaysHUD" console variable is set.

	if (bOn && ( g_vtAlwaysHUD.GetFloat() > 0.0f ) )
	{
		g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderNone);
	}
	else
	{
		g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderFull);
	}

	if (m_pPlayerCamera->IsFirstPerson())
	{
 		m_pMoveMgr->SetSpectatorMode(bOn);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::ChangeWeapon(uint8 nWeaponId, 
							  uint8 nAmmoId,/*= WMGR_INVALID_ID*/
							  int dwAmmo/*= -1*/)
{
	// RETAIL: Tron's returns the weapon manager's answer
	return m_pClientWeaponMgr->ChangeWeapon( nWeaponId, nAmmoId, dwAmmo );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleWeaponChanged()
//
//	PURPOSE:	Handle updating necessary player data after a weapon
//				change occurs...(NOTE: this should only be called from the
//				CClientWeaponMgr::ChangeWeapon() after a successful 
//				weapon change)
//
// ----------------------------------------------------------------------- //

	// jrg 9/2/02 - this will get called twice most of the time when switching weapons:
	//			once when the weapon switch is started
	//			and again when it completes (if a switching animation was needed)
	//		bImmediateSwitch will be true on the second call (or the first if the switch is immediate)
	//		(I'm using the repeated call to track whether we are in mid switch)

void CPlayerMgr::HandleWeaponChanged(uint8 nWeaponId, uint8 nAmmoId)
{
	// Turn off zooming...
	HandleZoomChange( LTTRUE );

	// Tell the server to change weapons...
	LTRESULT ltResult;
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_CHANGE );

	cMsg.Writeuint8( nWeaponId );
	cMsg.Writeuint8( nAmmoId );

	ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	ASSERT( LT_OK == ltResult );

	// this isn't strictly necessary, the hide should have been
	// called when we entered the chase view
	if ( m_pPlayerCamera->IsChaseView() )
	{
		m_pClientWeaponMgr->HideWeapons();
	}

	// RETAIL: Tron's closes no choosers and has no gadget switching
	// (retail 0x101278a0)
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::ChangeWeapon(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    uint8 nCommandId = pMsg->Readuint8();
    LTBOOL bAuto      = pMsg->Readuint8();
    float fAmmoId  = pMsg->Readfloat();


    uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeapon) return;


    LTBOOL bChange = (bAuto && GetConsoleBool("AutoWeaponSwitch",LTTRUE));

	// See what ammo the weapon should start with...

	uint8 nAmmoId = pWeapon->nDefaultAmmoId;
	if (fAmmoId >= 0)
	{
		nAmmoId = (uint8)fAmmoId;
	}

	if (bChange)
	{
        // Force a change to the approprite weapon...
		if (g_pPlayerStats)
		{
			ChangeWeapon(nWeaponId, nAmmoId, g_pPlayerStats->GetAmmoCount(nAmmoId));
		}
    }

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ToggleHolster()
//
//	PURPOSE:	Toggle the holster state of the weapon.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::ToggleHolster( bool bPlayDeselect )
{
	// toggle the holster
	m_pClientWeaponMgr->ToggleHolster( bPlayDeselect );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::LastWeapon()
//
//	PURPOSE:	Swap to our previously used weapon.
//
// ----------------------------------------------------------------------- //
void CPlayerMgr::LastWeapon()
{
	// toggle the last weapon
	m_pClientWeaponMgr->LastWeapon();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::BeginSpyVision
//
//  PURPOSE:	setup spy vision mode
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::BeginSpyVision()
{
/* KLS - 6/22/02 - Spy vision has been cut

  // Turn on the glow effect (and set the necessary values)...
	g_pLTClient->RunConsoleString("ScreenGlowEnable 1");

	// Turn off shadows (if they're on)...
	g_nSaveSpyVisionShadows = GetConsoleInt("MaxModelShadows", 0);
	WriteConsoleInt("MaxModelShadows", 0);

    g_pLTClient->SetModelHook((ModelHookFn)SVModelHook, g_pGameClientShell);
	m_bSpyVision = true;

*/
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::EndSpyVision
//
//  PURPOSE:	end spy vision mode
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::EndSpyVision()
{
/* KLS - 6/22/02 - Spy vision has been cut

	// Turn off the glow effect...
	g_pLTClient->RunConsoleString("ScreenGlowEnable 0");

	// Turn on shadows (if they were on)...NOTE: this assumes the user won't change
	// the state of the shadows while in spy vision...
	WriteConsoleInt("MaxModelShadows", g_nSaveSpyVisionShadows);

    g_pLTClient->SetModelHook((ModelHookFn)DefaultModelHook, g_pGameClientShell);
	m_bSpyVision = false;

*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::BeginZoom()
//
//	PURPOSE:	prepare for zooming
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::BeginZoom()
{
	uint8 nScopeId = g_pPlayerStats->GetScope();

	m_bCamera = LTFALSE;
	uint8 nAmmoId = g_pPlayerStats->GetCurrentAmmo();
	if (nAmmoId != WMGR_INVALID_ID)
	{
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
		if (pAmmo)
		{
			m_bCamera = (pAmmo->eInstDamageType == DT_GADGET_CAMERA);
		}
	}


	g_pInterfaceMgr->BeginScope(m_bSpyVision, m_bCamera);

	m_pClientWeaponMgr->HideWeapons();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::EndZoom()
//
//	PURPOSE:	done zooming
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::EndZoom()
{
	g_pInterfaceMgr->EndScope();
	m_bCamera = LTFALSE;

	// show the weapon
	m_pClientWeaponMgr->ShowWeapons();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleZoomChange()
//
//	PURPOSE:	Handle a potential zoom change
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleZoomChange(LTBOOL bReset)
{
	// Reset to normal FOV...

	if (bReset)
	{
		// RETAIL: Tron's resets while zooming too (retail 0x10127b80)
		if (!m_nZoomView && !m_bZooming && !m_bZoomingIn) return;

		m_nZoomView  = 0;
		m_bZooming   = LTFALSE;
		m_bZoomingIn = LTFALSE;
		g_pInterfaceMgr->EndZoom();
		EndZoom();

		SetCameraFOV(DEG2RAD(g_vtFOVXNormal.GetFloat()), DEG2RAD(g_vtFOVYNormal.GetFloat()));

		char strConsole[40];
		sprintf(strConsole, "+ModelLODOffset %f", m_fSaveLODScale);
		g_pLTClient->RunConsoleString(strConsole);
	}


    uint8 nScopeId = g_pPlayerStats->GetScope();
	if (nScopeId == WMGR_INVALID_ID) return;

	MOD const *pMod = g_pWeaponMgr->GetMod(nScopeId);
	if (!pMod) return;

	// Play zoom in/out sounds...

	if (m_bZoomingIn)
	{
		if (pMod->szZoomInSound[0])
		{
			g_pClientSoundMgr->PlaySoundLocal(pMod->szZoomInSound, SOUNDPRIORITY_MISC_MEDIUM);
		}
	}
	else
	{
		if (pMod->szZoomOutSound[0])
		{
			g_pClientSoundMgr->PlaySoundLocal(pMod->szZoomOutSound, SOUNDPRIORITY_MISC_MEDIUM);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetCameraFOV
//
//	PURPOSE:	Set the camera's FOV
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::SetCameraFOV(float fFovX, float fFovY)
{
	if (!m_hCamera) return;

    g_pLTClient->SetCameraFOV(m_hCamera, fFovX, fFovY);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCameraZoom
//
//	PURPOSE:	Update the camera's field of view
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateCameraZoom()
{
	char strConsole[30];

    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

    float fovX, fovY;
    g_pLTClient->GetCameraFOV(m_hCamera, &fovX, &fovY);

    float fOldFovX = fovX;

	if (!fovX)
	{
		fovX = DEG2RAD(g_vtFOVXNormal.GetFloat());
	}

    m_bZooming = LTTRUE;


    float fFovXZoomed, fZoomDist;

	if (m_bZoomingIn)
	{
		if (m_nZoomView == 1)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED);
			fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - fFovXZoomed;
		}
		else if (m_nZoomView == 2)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED1);
			// KLS - Always zoom all the way in...
			fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - fFovXZoomed;
			// Remove above code and uncomment line below to do old 
			// step zoom behavoir
			//fZoomDist	= DEG2RAD(FOVX_ZOOMED) - fFovXZoomed;
		}
		else if (m_nZoomView == 3)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED2);
			// KLS - Always zoom all the way in...
			fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - fFovXZoomed;
			// Remove above code and uncomment line below to do old 
			// step zoom behavoir
			//fZoomDist	= DEG2RAD(FOVX_ZOOMED1) - fFovXZoomed;
		}
	}
	else
	{
		if (m_nZoomView == 0)
		{
			fFovXZoomed	= DEG2RAD(g_vtFOVXNormal.GetFloat());
			fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - DEG2RAD(FOVX_ZOOMED);
		}
		else if (m_nZoomView == 1)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED);
			fZoomDist	= fFovXZoomed - DEG2RAD(FOVX_ZOOMED1);
		}
		else if (m_nZoomView == 2)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED1);
			fZoomDist	= fFovXZoomed - DEG2RAD(FOVX_ZOOMED2);
		}
	}

    float fZoomVel = fZoomDist / ZOOM_TIME;
    float fZoomAmount = fZoomVel * g_pGameClientShell->GetFrameTime();

	// Zoom camera in or out...

	if (m_bZoomingIn)
	{
		if (fovX > fFovXZoomed)
		{
			// Zoom camera in...

			fovX -= fZoomAmount;
		}

		if (fovX <= fFovXZoomed)
		{
			fovX = fFovXZoomed;
            m_bZooming = LTFALSE;
			g_pInterfaceMgr->EndZoom();
		}
	}
	else  // Zoom camera out...
	{
		if (fovX < fFovXZoomed)
		{
			// Zoom camera out...

			fovX += fZoomAmount;
		}

		if (fovX >= fFovXZoomed)
		{
			fovX = fFovXZoomed;
            m_bZooming = LTFALSE;
			g_pInterfaceMgr->EndZoom();
			if (m_nZoomView == 0)
			{
				EndZoom();
			}
		}
	}

	if (fOldFovX != fovX)
	{
		fovY = (fovX * DEG2RAD(g_vtFOVYNormal.GetFloat())) / DEG2RAD(g_vtFOVXNormal.GetFloat());

		SetCameraFOV(fovX, fovY);

		// Update the lod adjustment for models...
		float fZoomAmount = (DEG2RAD(g_vtFOVXNormal.GetFloat()) - fovX) / (DEG2RAD(g_vtFOVXNormal.GetFloat()) - DEG2RAD(FOVX_ZOOMED2));
		float fNewLODOffset = m_fSaveLODScale + (LOD_ZOOMADJUST * fZoomAmount);

		sprintf(strConsole, "+ModelLODOffset %f", fNewLODOffset);
        g_pLTClient->RunConsoleString(strConsole);

        //g_pLTClient->CPrint("Current FOV (%f, %f)", fovX, fovY);
        //g_pLTClient->CPrint("Current Zoom LODOffset: %f", fNewLODOffset);
	}

}






// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnCommandOn()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerMgr::OnCommandOn(int command)
{
	// Make sure we're in the world...

	if (!IsPlayerInWorld()) return LTFALSE;

	// See if the vehiclemgr would like to trap this
	if(m_pMoveMgr->GetVehicleMgr()->OnCommandOn(command))
	{
		return LTTRUE;
	}

	// Check for weapon change...

	if (g_pWeaponMgr->GetFirstWeaponCommandId() <= command &&
		command <= g_pWeaponMgr->GetLastWeaponCommandId())
	{
		ChangeWeapon( g_pWeaponMgr->GetWeaponId( command ) );
		return LTTRUE;
	}

	// Take appropriate action

	switch (command)
	{
		// RETAIL: Tron's (retail 0x10128180): no gadgets, no reloading
		case COMMAND_ID_ACTIVATE :
		{
			// If we're pointed at something, or riding a vehicle we can get
			// off, activate it...
			if (GetTargetMgr()->IsTargetInRange() ||
				m_pMoveMgr->GetVehicleMgr()->CanDismount())
			{
				DoActivate();
				return LTTRUE;
			}
		}
		break;

		case COMMAND_ID_MOVE_BODY:
		{
			CActivationData data = GetTargetMgr()->GetActivationData();
			if( GetCarryingObject() )
			{
				data.m_nType = MID_ACTIVATE_MOVE;
				if( CanDropCarriedObject( ))
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_PLAYER_ACTIVATE);
					data.Write(cMsg);
					g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				}
			}
			else if( GetTargetMgr()->IsTargetInRange() && GetTargetMgr()->IsMoveTarget() )
			{
				// If the body can be revived then we can't carry it...
				CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFX(GetTargetMgr()->GetTargetObject());
				if( !pBody || !pBody->CanBeRevived() )
				{
					data.m_nType = MID_ACTIVATE_MOVE;
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_PLAYER_ACTIVATE);
					data.Write(cMsg);
					g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				}
			}
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if (IsPlayerDead())
			{
                if (g_pLTClient->GetTime() > m_fEarliestRespawnTime)
				{
					if (IsMultiplayerGame())
					{
						HandleRespawn();
					}
					else
					{
						g_pMissionMgr->HandleMissionFailed();
					}
				}
			}
		}
		break;

		case COMMAND_ID_RELOAD :
		{
			IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
			if ( pClientWeapon )
			{
				// Reload the clip and let the server know...
				pClientWeapon->ReloadClip( true, -1, false, true );
			}
		}
		break;

		case COMMAND_ID_NEXT_VISMODE :
		{
			m_pVisionModeMgr->NextMode();
		}
		break;

		case COMMAND_ID_ZOOM_IN :
		{
			// RETAIL: Tron's: no zooming on a light cycle, and one zoom
			// level, whatever the weapon
			if (g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE) return LTTRUE;

			const int nZoomLevel = 1;

			if (!m_bZooming)
			{
				int nOldZoom = m_nZoomView;

				// KLS - Always zoom all the way out or in...
				if (nOldZoom == nZoomLevel)
				{
					m_nZoomView = 0;
				}
				else
				{
					m_nZoomView = nZoomLevel;
				}

				if (m_nZoomView > nOldZoom)
				{
                    m_bZooming   = LTTRUE;
                    m_bZoomingIn = LTTRUE;

					if (nOldZoom == 0)
					{
						BeginZoom();
                    }

					g_pInterfaceMgr->BeginZoom(LTTRUE);
					HandleZoomChange();
				}
				else if (m_nZoomView < nOldZoom)
				{
					m_bZooming   = LTTRUE;
					m_bZoomingIn = LTFALSE;
					g_pInterfaceMgr->BeginZoom(LTFALSE);
					HandleZoomChange();
				}
			}
		}
		break;

		case COMMAND_ID_TURNAROUND :
		{
			m_fYaw += MATH_PI;
		}
		break;

		case COMMAND_ID_CENTERVIEW :
		{
			m_fPitch = 0.0f;
		}
		break;

		case COMMAND_ID_STRAFE:
		{
            m_bStrafing = LTTRUE;
		}
		break;

		case COMMAND_ID_RUNLOCK :
		{
			if (!m_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
			{
				m_pMoveMgr->SetRunLock(!m_pMoveMgr->RunLock());
			}
		}
		break;

		case COMMAND_ID_DUCKLOCK :
		{
			if( !GetCarryingObject() && !m_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())	// RETAIL: any carried object, in Tron
			{
				m_pMoveMgr->SetDuckLock(!m_pMoveMgr->DuckLock());
			}
		}
		break;

		case COMMAND_ID_JUMP : 
		case COMMAND_ID_DUCK : 
		{
			m_pMoveMgr->SetDuckLock(LTFALSE); 
		}
		break;

		default :
			return LTFALSE;
		break;
	}

	return LTTRUE;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off notification
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerMgr::OnCommandOff(int command)
{
	// See if the vehiclemgr would like to trap this
	if(m_pMoveMgr->GetVehicleMgr()->OnCommandOff(command))
	{
		return LTTRUE;
	}
	
	switch (command)
	{
		case COMMAND_ID_STRAFE :
		{
            m_bStrafing = LTFALSE;
		}
		break;

        default : 
			return LTFALSE;
			break;
	}

	return LTTRUE;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgPlayerStateChange()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgPlayerStateChange (ILTMessage_Read *pMsg)
{
	m_ePlayerState = (PlayerState) pMsg->Readuint8();

	// RETAIL: Tron's (retail 0x101288d0)
	switch (m_ePlayerState)
	{
		case PS_DYING:
		{
			// Clear out any damagefx...
			// Do this first since we may be unattaching the camera from the head when clearing out a 
			// damage fx.

			g_pDamageFXMgr->Clear();

			if( IsMultiplayerGame() )
			{
				AttachCameraToHead(LTTRUE);

				m_fEarliestRespawnTime = g_pLTClient->GetTime() + ((CCharacterFX::GetNumPlayersInGame() > 1) ?
																	g_vtMultiplayerRespawnWaitTime.GetFloat() :
																	g_vtRespawnWaitTime.GetFloat());

				g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderDead);
			}
			else
			{
				AttachCameraToHead(LTTRUE);

				m_fEarliestRespawnTime = g_pLTClient->GetTime() + g_vtRespawnWaitTime.GetFloat();

				g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderNone);
			}

			if( g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LURE )
			{
				// Get the playerlurefx.
				PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( g_pMoveMgr->GetVehicleMgr()->GetPlayerLureId() );
				if( pPlayerLureFX )
				{
					char const* pszDeathFX = pPlayerLureFX->GetDeathFX( );
					if( !pszDeathFX && pszDeathFX[0] != '\0' )
					{
						LTVector vPos;
						g_pLTClient->GetObjectPos( g_pMoveMgr->GetObject(), &vPos );

						CLIENTFX_CREATESTRUCT fxInit( pszDeathFX, 0, vPos );
						g_pClientFXMgr->CreateClientFX( LTNULL, fxInit, LTTRUE );
					}
				}
			}

			ClearPlayerModes();

			m_pClientWeaponMgr->OnPlayerDead();

			g_pInterfaceMgr->AddToClearScreenCount();

			// RETAIL: Tron's: the player's death FX
			char szDeathFX[128];
			szDeathFX[0] = '\0';
			if (IsMultiplayerGame( ))
			{
				g_pClientButeMgr->GetMultiDeathFX(szDeathFX, sizeof(szDeathFX));
			}
			else
			{
				g_pClientButeMgr->GetDeathFX(szDeathFX, sizeof(szDeathFX));
			}

			if (szDeathFX[0])
			{
				LTVector vPos(0.0f, 0.0f, 0.0f);
				CLIENTFX_CREATESTRUCT fxInit( szDeathFX, FXFLAG_REALLYCLOSE, vPos );
				g_pClientFXMgr->CreateClientFX( &s_DeathFX, fxInit, LTTRUE );
			}
		}
		break;

		case PS_ALIVE:
		{
			g_pDamageFXMgr->Clear();					// Remove all the damage sfx
			ClearDamageSectors();

			g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

			AttachCameraToHead(LTFALSE);

			g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderFull);

			SetExternalCamera(LTFALSE);

			m_pClientWeaponMgr->OnPlayerAlive();

			if( IsMultiplayerGame() )
			{
				AllowPlayerMovement( LTTRUE );
			}
		}
		break;

		case PS_DEAD:
		{
			TronUnknownVirtual58();
		}
		break;

		case PS_GHOST:
		{
			AttachCameraToHead(LTFALSE);
			m_pClientWeaponMgr->HideWeapons();

			g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderNone);

			g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

			m_pClientWeaponMgr->DisableWeapons();
		}
		break;

		default : break;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleRespawn
//
//	PURPOSE:	Handle player respawn
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::HandleRespawn()
{
	if (!IsPlayerDead()) return;

	// RETAIL: Tron's (retail 0x10128d70)
	if (s_DeathFX.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&s_DeathFX);
	}

	// if we're in multiplayer send the respawn command...

	if (IsMultiplayerGame())
	{
		// send a message to the server telling it that it's ok to respawn us now...
		SendEmptyServerMsg(MID_PLAYER_RESPAWN);
		return;
	}
	else  // Bring up load game menu...
	{
		// RETAIL: Tron's SwitchToScreen has a second argument, true here
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LOAD);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::PreRender()
//
//	PURPOSE:	Sets up the client for rendering
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerMgr::PreRender()
{
	if (!m_bCameraPosInited) return LTFALSE;

		// Make sure the rendered player object is right where it should be.

	UpdateServerPlayerModel();

	// Make sure we process attachments before updating the weapon model
	// and special fx...(some fx are based on attachment positions/rotations)

    g_pLTClient->ProcessAttachments(g_pLTClient->GetClientObject());

	// Make sure the weapon is updated before we render the camera...

	UpdateWeaponModel();

	// Make sure the move-mgr models are updated before we render...

	m_pMoveMgr->UpdateModels();

	// Update the flash light...

	m_pFlashLight->Update();


	return LTTRUE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayerCamera()
//
//	PURPOSE:	Update the player camera
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerMgr::UpdatePlayerCamera()
{
	// Make sure our player camera is attached...

	if (m_pPlayerCamera->GetAttachedObject() != m_pMoveMgr->GetObject())
	{
		m_pPlayerCamera->AttachToObject(m_pMoveMgr->GetObject());
	}

	if (m_pPlayerCamera->IsChaseView())
	{
		// Init the camera so they can be adjusted via the console...

		InitPlayerCamera();

		Update3rdPersonInfo();
	}

	// RETAIL: Tron's (retail 0x10128e20): the light cycle camera zooms
	// (the LCC* console variables override its speed and limits)
	if (g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE || TronUnknownVirtual3C(true))
	{
		if (g_pLTClient->IsCommandOn(COMMAND_ID_ZOOM_IN))
		{
			float fZoomSpeed = 1000.0f;
			float fMinZoom = 160.0f;

			HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCCZoomSpeed");
			if (hVar)
			{
				fZoomSpeed = g_pLTClient->GetVarValueFloat(hVar);
			}

			hVar = g_pLTClient->GetConsoleVar("LCCMinZoom");
			if (hVar)
			{
				fMinZoom = g_pLTClient->GetVarValueFloat(hVar);
			}

			float fCurZoom = m_pPlayerCamera->GetTronUnknown100();
			float fZoom = fCurZoom - g_pLTClient->GetFrameTime() * fZoomSpeed;
			if (fZoom < fMinZoom)
			{
				fZoom = fMinZoom;
			}

			m_pPlayerCamera->SetTronUnknown100(fZoom);
		}
		else if (g_pLTClient->IsCommandOn(COMMAND_ID_ZOOM_OUT))
		{
			float fZoomSpeed = 1000.0f;
			float fMaxZoom = m_fTronUnknown1DC;

			HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCCZoomSpeed");
			if (hVar)
			{
				fZoomSpeed = g_pLTClient->GetVarValueFloat(hVar);
			}

			hVar = g_pLTClient->GetConsoleVar("LCCMaxZoom");
			if (hVar)
			{
				fMaxZoom = g_pLTClient->GetVarValueFloat(hVar);
			}

			float fCurZoom = m_pPlayerCamera->GetTronUnknown100();
			float fZoom = fCurZoom + g_pLTClient->GetFrameTime() * fZoomSpeed;
			if (fZoom > fMaxZoom)
			{
				fZoom = fMaxZoom;
			}

			m_pPlayerCamera->SetTronUnknown100(fZoom);
		}
	}

	// Update our camera position based on the player camera...

	m_pPlayerCamera->CameraUpdate(g_pGameClientShell->GetFrameTime());


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::InitPlayerCamera()
//
//	PURPOSE:	Update the player camera
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::InitPlayerCamera()
{
    LTVector vOffset(0.0f, 0.0f, 0.0f);
	vOffset.y = g_vtChaseCamOffset.GetFloat();

	m_pPlayerCamera->SetDistUp(g_vtChaseCamDistUp.GetFloat());
	m_pPlayerCamera->SetDistBack(g_vtChaseCamDistBack.GetFloat());
	m_pPlayerCamera->SetPointAtOffset(vOffset);
	m_pPlayerCamera->SetChaseOffset(vOffset);
	m_pPlayerCamera->SetCameraState(CPlayerCamera::SOUTH);

	// Determine the first person offset...

	vOffset = g_vPlayerCameraOffset;

	CCharacterFX* pChar = m_pMoveMgr->GetCharacterFX();
	if (pChar)
	{
		vOffset = GetPlayerHeadOffset( );
	}

	m_pPlayerCamera->SetFirstPersonOffset(vOffset);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCameraPosition()
//
//	PURPOSE:	Update the camera position
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateCameraPosition()
{
    LTVector vPos = m_pPlayerCamera->GetPos();

	// RETAIL: Tron's (retail 0x10129160): build 51's, with NOLF2's attached
	// camera angles
	if (m_pPlayerCamera->IsFirstPerson())
	{
		// Special case of camera being attached to the player's head
		// (i.e., death)...
		if (m_bCameraAttachedToHead)
		{
			GetPlayerHeadPosRot(vPos, m_rRotation);

			//save this rotation into the yaw pitch and roll as well so that when we
			//come out of the attached mode, our view won't pop
			EulerAngles EA = Eul_FromQuat( m_rRotation, EulOrdYXZr );
			
			m_fModelAttachYaw		= EA.x;
			m_fModelAttachPitch		= EA.y;
			m_fModelAttachRoll		= EA.z;
		}
		else
		{
			//normal updating of the head position
			m_pHeadBobMgr->AdjustCameraPos(vPos);

			vPos.y	+= m_fCamDuck;
			vPos	+= m_pCameraOffsetMgr->GetPosDelta();
		}
	}
	else if (m_pPlayerCamera->IsChaseView())
	{
        m_rRotation = m_pPlayerCamera->GetRotation();
	}

	g_pLTClient->SetObjectPos(m_hCamera, &vPos);


    m_bCameraPosInited = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CalculateCameraRotation()
//
//	PURPOSE:	Calculate the new camera rotation
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::CalculateCameraRotation()
{
	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

	LTBOOL bIsVehicle = m_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics();

    float fVal = 1.0f + (float)(3 * m_nZoomView);

	// Get axis offsets...

	float offsets[3];
    g_pLTClient->GetAxisOffsets(offsets);

	if (m_bRestoreOrientation)
	{
		offsets[0] = offsets[1] = offsets[2] = 0.0f;
        m_bRestoreOrientation = LTFALSE;
	}

	if (m_bStrafing)
	{
		// Clear yaw and pitch offsets if we're using mouse strafing...
		offsets[0] = offsets[1] = 0.0f;
	}

    float fYawDelta    = offsets[0] / fVal;
    float fPitchDelta  = offsets[1] / fVal;

	if (g_vtAdaptiveMouse.GetFloat() && fYawDelta != 0.0f)
	{
		float fMaxOff = g_vtAdaptiveMouseMaxOffset.GetFloat();
		float fAbsYawDelta = (float)fabs(fYawDelta);
		bool bNegative = (fYawDelta < 0.0f);

		g_pLTClient->CPrint("Initial fYawDelta = %.6f", fYawDelta);

		if (fAbsYawDelta <= fMaxOff)
		{
			fVal = fAbsYawDelta / fMaxOff;
			fYawDelta = (WaveFn_SlowOn(fVal) * fMaxOff);
			fYawDelta = bNegative ? -fYawDelta : fYawDelta;

			g_pLTClient->CPrint("Adjusted fYawDelta = %.6f", fYawDelta);
		}
	}

	m_fYaw += fYawDelta;

	// [kml] 12/26/00 Check varying degrees of strafe and look.
	if(!(m_dwPlayerFlags & BC_CFLG_STRAFE))
	{
		if(m_dwPlayerFlags & BC_CFLG_LEFT)
		{
			m_fYaw -= g_pGameClientShell->GetFrameTime() * ((m_dwPlayerFlags & BC_CFLG_RUN) ? g_vtFastTurnRate.GetFloat() : g_vtNormalTurnRate.GetFloat());
		}
		if(m_dwPlayerFlags & BC_CFLG_RIGHT)
		{
			m_fYaw += g_pGameClientShell->GetFrameTime() * ((m_dwPlayerFlags & BC_CFLG_RUN) ? g_vtFastTurnRate.GetFloat() : g_vtNormalTurnRate.GetFloat());
		}
	}

	if (pSettings->MouseLook() || (m_dwPlayerFlags & BC_CFLG_LOOKUP) || (m_dwPlayerFlags & BC_CFLG_LOOKDOWN)
)
    {
        if (pSettings->MouseLook())
		{
			if (pSettings->MouseInvertY())
			{
				m_fPitch -= fPitchDelta;
			}
			else
			{
				m_fPitch += fPitchDelta;
			}
		}

		if(m_dwPlayerFlags & BC_CFLG_LOOKUP)
		{
			m_fPitch -= g_pGameClientShell->GetFrameTime() * g_vtLookUpRate.GetFloat();
		}

		if(m_dwPlayerFlags & BC_CFLG_LOOKDOWN)
		{
			m_fPitch += g_pGameClientShell->GetFrameTime() * g_vtLookUpRate.GetFloat();
		}
		// Don't allow much movement up/down if 3rd person...

		if (!m_pPlayerCamera->IsFirstPerson())
		{
            float fMinY = DEG2RAD(45.0f) - 0.1f;

			if (m_fPitch < -fMinY) m_fPitch = -fMinY;
			if (m_fPitch > fMinY)  m_fPitch = fMinY;
		}
	}
	else if (m_fPitch != 0.0f && GetConsoleInt("AutoCenter",0))
	{
        float fPitchDelta = (g_pGameClientShell->GetFrameTime() * g_vtLookUpRate.GetFloat());
		if (m_fPitch > 0.0f) m_fPitch -= Min(fPitchDelta, m_fPitch);
		if (m_fPitch < 0.0f) m_fPitch += Min(fPitchDelta, -m_fPitch);
	}

    float fMinY = MATH_HALFPI - 0.1f;

	if (m_fPitch < -fMinY) m_fPitch = -fMinY;
	if (m_fPitch > fMinY)  m_fPitch = fMinY;


	// Set camera and player variables...

	// Only use mouse values for yaw if the player isn't on a vehicle...

	if (bIsVehicle)
	{
		// Can't look up/down on vehicles...

		LTVector vPlayerPYR(m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
		LTVector vPYR(m_fPitch, m_fYaw, m_fRoll);

		m_pMoveMgr->GetVehicleMgr()->CalculateVehicleRotation(vPlayerPYR, vPYR, fYawDelta);

		m_fPlayerPitch	= vPlayerPYR.x;
		m_fPlayerYaw	= vPlayerPYR.y;
		m_fPlayerRoll	= vPlayerPYR.z;

		m_fPitch		= vPYR.x;
		m_fYaw			= vPYR.y;
		m_fRoll			= vPYR.z;
	}
	else  // Not vehicle...
	{
		m_fPlayerPitch	= 0.0f;
		m_fPlayerYaw	= m_fYaw;
		m_fPlayerRoll	= m_fRoll;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCameraRotation()
//
//	PURPOSE:	Set the new camera rotation
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerMgr::UpdateCameraRotation()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    if (!hPlayerObj) return LTFALSE;

	// Update camera orientation vars...

    LTVector vPitchYawRollDelta = m_pCameraOffsetMgr->GetPitchYawRollDelta();

	if (m_bUsingExternalCamera)
	{
		// Just calculate the correct player rotation...

		m_rRotation = LTRotation(m_fPitch, m_fYaw, m_fRoll);
	}
	// RETAIL: Tron's (retail 0x10129a20): by camera mode
	else if (m_pPlayerCamera->IsFirstPerson())
	{
		if (!m_bCameraAttachedToHead)
		{
			float fPitch = m_fPitch + vPitchYawRollDelta.x;
			float fYaw   = m_fYaw	+ vPitchYawRollDelta.y;
			float fRoll  = m_fRoll  + vPitchYawRollDelta.z;

			m_rRotation = LTRotation(fPitch, fYaw, fRoll);
		}

		g_pLTClient->SetObjectRotation(m_hCamera, &m_rRotation);
	}
	else if (m_pPlayerCamera->IsChaseView())
	{
		// Set the camera to use the rotation calculated by the player camera,
		// however we still need to calculate the correct rotation to be sent
		// to the player...

        float fAdjust = DEG2RAD(g_vtChaseCamPitchAdjust.GetFloat());
		m_rRotation.Rotate(m_rRotation.Right(), m_fPitch + fAdjust);
		g_pLTClient->SetObjectRotation(m_hCamera, &m_rRotation);

		// Okay, now calculate the correct player rotation...
		m_rRotation = LTRotation(m_fPitch, m_fYaw, m_fRoll);
	}
	else if (m_pPlayerCamera->GetCameraMode() == CPlayerCamera::TRONMODE5 ||
			 m_pPlayerCamera->GetCameraMode() == CPlayerCamera::TRONMODE6 ||
			 m_pPlayerCamera->GetCameraMode() == CPlayerCamera::GOINGFIRSTPERSON)
	{
		// The player camera's rotation, and the player's...
		m_rRotation = m_pPlayerCamera->GetRotation();
		g_pLTClient->SetObjectRotation(m_hCamera, &m_rRotation);

		m_rRotation = LTRotation(m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Update3rdPersonInfo
//
//	PURPOSE:	Update the 3rd person cross hair / camera info
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::Update3rdPersonInfo()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj || IsPlayerDead()) return;

    HOBJECT hFilterList[] = {hPlayerObj, m_pMoveMgr->GetObject(), NULL};


	ClientIntersectInfo info;
	ClientIntersectQuery query;
    LTVector vPlayerPos, vForward;

	g_pLTClient->GetObjectPos(hPlayerObj, &vPlayerPos);

    float fCrosshairDist = -1.0f;
    float fCameraOptZ = g_vtChaseCamDistBack.GetFloat();

	// Figure out crosshair distance...
	IClientWeaponBase* pClientWeapon = m_pClientWeaponMgr->GetCurrentClientWeapon();
	if (g_pInterfaceMgr->IsCrosshairOn() && pClientWeapon)
	{
		WEAPON const *pWeapon = pClientWeapon->GetWeapon();
		if (!pWeapon) return;

		fCrosshairDist = (float) pWeapon->nRange;

		vForward = m_rRotation.Forward();

		// Determine where the cross hair should be...

		LTVector vStart, vEnd, vPos;
		VEC_COPY(vStart, vPlayerPos);
		VEC_MULSCALAR(vEnd, vForward, fCrosshairDist);
		VEC_ADD(vEnd, vEnd, vStart);

		VEC_COPY(query.m_From, vStart);
		VEC_COPY(query.m_To, vEnd);

		query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		query.m_FilterFn = ObjListFilterFn;
		query.m_pUserData = hFilterList;

		if (g_pLTClient->IntersectSegment (&query, &info))
		{
			VEC_COPY(vPos, info.m_Point);
		}
		else
		{
			VEC_COPY(vPos, vEnd);
		}

		LTVector vTemp;
		VEC_SUB(vTemp, vPos, vStart);

		fCrosshairDist = VEC_MAG(vTemp);
	}


	// Figure out optinal camera distance...

	LTRotation rRot;
	g_pLTClient->GetObjectRotation(hPlayerObj, &rRot);
	vForward = rRot.Forward();

	// Determine how far behind the player the camera can go...

	LTVector vEnd;
	vEnd = vForward * -fCameraOptZ + vPlayerPos;

	query.m_From = vPlayerPos;
	query.m_To = vEnd;

	query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	query.m_FilterFn = ObjListFilterFn;
	query.m_pUserData = hFilterList;

	if (g_pLTClient->IntersectSegment (&query, &info))
	{
		LTVector vTemp;
		VEC_SUB(vTemp, info.m_Point, vPlayerPos);
		float fDist = VEC_MAG(vTemp);

		fCameraOptZ = fDist < fCameraOptZ ? -(fDist - 5.0f) : -fCameraOptZ;
	}
	else
	{
		fCameraOptZ = -fCameraOptZ;
	}


	m_pPlayerCamera->SetOptZ(fCameraOptZ);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayerFlags
//
//	PURPOSE:	Update our copy of the movement flags
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlayerFlags()
{
	// Update flags...

	m_dwPlayerFlags = m_pMoveMgr->GetControlFlags();

    if (g_pLTClient->IsCommandOn(COMMAND_ID_LOOKUP))
	{
		m_dwPlayerFlags |= BC_CFLG_LOOKUP;
	}

    if (g_pLTClient->IsCommandOn(COMMAND_ID_LOOKDOWN))
	{
		m_dwPlayerFlags |= BC_CFLG_LOOKDOWN;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayerInfo()
//
//	PURPOSE:	Tell the player about the new camera stuff
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlayerInfo(bool bPlaying)
{
	if (m_bAllowPlayerMovement != m_bLastAllowPlayerMovement)
	{
		g_pGameClientShell->SetInputState(!!m_bAllowPlayerMovement);
	}

	if (m_pPlayerCamera->IsChaseView() != m_bLastSent3rdPerson)
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_3RDPERSON;
		m_bLastSent3rdPerson = m_pPlayerCamera->IsChaseView();

		if (m_pPlayerCamera->IsChaseView())
		{
			m_nPlayerInfoChangeFlags |= CLIENTUPDATE_3RDPERVAL;
		}
	}

	if (m_bAllowPlayerMovement != m_bLastAllowPlayerMovement)
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_ALLOWINPUT;
	}

	// Always send CLIENTUPDATE_ALLOWINPUT changes guaranteed...

	if (m_nPlayerInfoChangeFlags & CLIENTUPDATE_ALLOWINPUT)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_UPDATE);
		cMsg.Writeuint8(CLIENTUPDATE_ALLOWINPUT);	// RETAIL: the flags are a byte
		cMsg.Writeuint8((uint8)m_bAllowPlayerMovement);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		m_nPlayerInfoChangeFlags &= ~CLIENTUPDATE_ALLOWINPUT;
	}

    float fCurTime   = g_pLTClient->GetTime();
	float fSendRate  = 1.0f / g_CV_CSendRate.GetFloat(DEFAULT_CSENDRATE);
	float fSendDelta = (fCurTime - m_fPlayerInfoLastSendTime);
	float fSendAllRate = g_vtCSendRateMin.GetFloat(DEFAULT_CSENDRATE_MIN);

	// RETAIL: Tron's, as Contract J.A.C.K.'s (retail 0x1012a3a0): what did
	// not change is sent only every 1/CSendRate_Min seconds
	bool bSendAll = true;
	if (!(fCurTime - m_fPlayerInfoLastFullSendTime > 1.0f / fSendAllRate))
	{
		bSendAll = false;
	}

	if (!m_pMoveMgr->IsInWorld())
		return;

	if (g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) && !(fSendDelta > fSendRate))
		return;

	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_PLAYER_UPDATE);

	LTRotation rPlayerRot(m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
	uint8 nRotation = CompressRotationByte(&rPlayerRot);
	uint8 nPitch = CompressAngleToByte(m_fPitch);

	if (g_vtPlayerRotate.GetFloat(1.0) > 0.0)
	{
		// RETAIL: Tron's has no full rotation: a snowmobile sends the
		// accurate one
		if( m_bServerAccurateRotation || PPM_SNOWMOBILE == m_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() )
		{
			m_nPlayerInfoChangeFlags |= CLIENTUPDATE_ACCURATEPLAYERROT;
			m_nPlayerInfoChangeFlags &= ~CLIENTUPDATE_PLAYERROT;
		}
		else
		{
			// Only a changed rotation...
			if (!bSendAll && nRotation == m_nPlayerInfoLastSent[0] && nPitch == m_nPlayerInfoLastSent[1])
			{
				m_nPlayerInfoChangeFlags &= ~CLIENTUPDATE_PLAYERROT;
			}
			else
			{
				m_nPlayerInfoChangeFlags |= CLIENTUPDATE_PLAYERROT;
			}
			
			m_nPlayerInfoChangeFlags &= ~CLIENTUPDATE_ACCURATEPLAYERROT;
			m_nPlayerInfoChangeFlags &= ~CLIENTUPDATE_FULLPLAYERROT;
		}
	}

	if ( m_bSendCameraOffsetToServer )
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_CAMERAOFFSET;
	}

	// RETAIL: the flags are a byte in Tron
    cMsg.Writeuint8((uint8)m_nPlayerInfoChangeFlags);

	if (m_nPlayerInfoChangeFlags & CLIENTUPDATE_PLAYERROT)
	{
		// Set the player's rotation (don't allow model to rotate up/down).

        cMsg.Writeuint8(nRotation);
		m_nPlayerInfoLastSent[0] = nRotation;
	}
	else if ( m_nPlayerInfoChangeFlags & CLIENTUPDATE_ACCURATEPLAYERROT )
	{
        LTRotation rPlayerRot( m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll );
        cMsg.Writeuint16( CompressRotationShort( &rPlayerRot ) );
	}

	if ( m_nPlayerInfoChangeFlags & ( CLIENTUPDATE_PLAYERROT | CLIENTUPDATE_ACCURATEPLAYERROT ) )
	{
		// and write the pitch to the message.
		cMsg.Writeuint8( nPitch );
		m_nPlayerInfoLastSent[1] = nPitch;
	}

	// write the camera offset
	if ( m_nPlayerInfoChangeFlags & CLIENTUPDATE_CAMERAOFFSET )
	{
		LTRESULT ltResult;

		// Get the fire position from the camera
		LTVector vCameraPosition;

		// get the camera
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();

		if ( g_pPlayerMgr->IsFirstPerson() &&
		     ( !g_pPlayerMgr->IsUsingExternalCamera() ) && 
		     ( 0 != hCamera ) )
		{
			// we're in 1st person and not using an external camera,

			// get the camera's position
			ltResult = g_pLTClient->GetObjectPos( hCamera, &vCameraPosition );
			ASSERT( LT_OK == ltResult );
		}
		else
		{
			HMODELNODE hPlayerHeadNode;
			LTransform transHeadNode;

			// external camera, figure out where the model's head
			// is and use that.  It won't be as accurate but it is
			// better than nothing.

			// get the head node (RETAIL: "Head" in Tron)
			ltResult = g_pModelLT->GetNode( g_pLTClient->GetClientObject(),
			                                "Head",
			                                hPlayerHeadNode );
			ASSERT( LT_OK == ltResult );

			// get the node's transform
			ltResult =
				g_pModelLT->GetNodeTransform( g_pLTClient->GetClientObject(),
				                              hPlayerHeadNode,
				                              transHeadNode,
				                              true );
			ASSERT( LT_OK == ltResult );

			// fake the camera position
			ltResult =
				g_pLTClient->GetTransformLT()->GetPos( transHeadNode,
				                                       vCameraPosition );
			ASSERT( LT_OK == ltResult );
		}

		// get the player's position
		LTVector vPlayerPosition;
		ltResult = g_pLTClient->GetObjectPos( g_pMoveMgr->GetObject(),
		                                      &vPlayerPosition );
		ASSERT( LT_OK == ltResult );

		// get the camera offset
		LTVector vOffset = vCameraPosition - vPlayerPosition;

		TVector3< short > vCompressedOffset;

		// compress the offset
		bool result = CompressOffset( &vCompressedOffset, vOffset, 100 );
		ASSERT( true == result );

		// write the offset to the message
		cMsg.Writeuint16( vCompressedOffset.x );

		cMsg.Writeuint16( vCompressedOffset.y );

		cMsg.Writeuint16( vCompressedOffset.z );
	}

	// Write the control flags

	if (bPlaying)
		cMsg.Writeuint32(g_pMoveMgr->GetControlFlags());
	else
		cMsg.Writeuint32(g_pMoveMgr->GetControlFlags() & BC_CFLG_DUCK);

	// Write position info, if it changed...

	{
		CAutoMessage cPositionMsg;
		m_pMoveMgr->WritePositionInfo(cPositionMsg);
		CLTMsgRef_Read cPositionRead = cPositionMsg.Read();

		if (!bSendAll && IsSameMessage(cPositionRead, m_cLastPlayerExtraMsg))
		{
			cMsg.Writebool(false);
		}
		else
		{
			cMsg.Writebool(true);
			((ILTMessage_Write*)cMsg)->WriteMessageRaw(cPositionRead);
			m_cLastPlayerExtraMsg = (ILTMessage_Read*)cPositionRead;
		}
	}

	// Send it, if it changed...

	CLTMsgRef_Read cRead = cMsg.Read();
	if (bSendAll || !IsSameMessage(cRead, m_cLastPlayerInfoMsg))
	{
		m_cLastPlayerInfoMsg = (ILTMessage_Read*)cRead;
		g_pLTClient->SendToServer(cRead, 0);
		m_fPlayerInfoLastFullSendTime = fCurTime;
	}

	m_fPlayerInfoLastSendTime = fCurTime;
	m_nPlayerInfoChangeFlags  = 0;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::IsFirstPerson()
//
//	PURPOSE:	See if we are in first person mode
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerMgr::IsFirstPerson()
{
	return m_pPlayerCamera->IsFirstPerson();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::IsPlayerInWorld()
//
//	PURPOSE:	See if the player is in the world
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerMgr::IsPlayerInWorld()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();

    if (!g_pGameClientShell->IsServerLoaded( ) || !g_pGameClientShell->IsWorldLoaded() || m_ePlayerState == PS_UNKNOWN || !hPlayerObj) return LTFALSE;

    return LTTRUE;
}

void CPlayerMgr::GetCameraRotation(LTRotation &rRot)
{
	rRot = LTRotation(m_fPitch, m_fYaw, m_fRoll);
}

void CPlayerMgr::GetPlayerRotation(LTRotation &rRot)
{
	rRot = LTRotation(m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	UpdateModelGlow
//
//	PURPOSE:	Update the current model glow color
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateModelGlow()
{
    float fColor      = 0.0f;
	float fMin		  = g_vtModelGlowMin.GetFloat();
	float fMax		  = g_vtModelGlowMax.GetFloat();
    float fColorRange = fMax - fMin;
	float fTimeRange  = g_vtModelGlowTime.GetFloat();

	if (m_bModelGlowCycleUp)
	{
		if (m_fModelGlowCycleTime < fTimeRange)
		{
			fColor = fMin + (fColorRange * (m_fModelGlowCycleTime / fTimeRange));
			m_vCurModelGlow.Init(fColor, fColor, fColor);
		}
		else
		{
			m_fModelGlowCycleTime = 0.0f;
			m_vCurModelGlow.Init(fMax, fMax, fMax);
			m_bModelGlowCycleUp = LTFALSE;
			return;
		}
	}
	else
	{
		if (m_fModelGlowCycleTime < fTimeRange)
		{
			fColor = fMax - (fColorRange * (m_fModelGlowCycleTime / fTimeRange));
			m_vCurModelGlow.Init(fColor, fColor, fColor);
		}
		else
		{
			m_fModelGlowCycleTime = 0.0f;
			m_vCurModelGlow.Init(fMin, fMin, fMin);
            m_bModelGlowCycleUp = LTTRUE;
			return;
		}
	}

	if (!g_pGameClientShell->IsServerPaused())
	{
		m_fModelGlowCycleTime += g_pGameClientShell->GetFrameTime();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ClearCurContainerCode
//
//	PURPOSE:	Clear our current container info.
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::ClearCurContainerCode()
{
	// RETAIL: Tron's clears no light scale (retail 0x1012b1f0)
	m_eCurContainerCode = CC_NO_CONTAINER;
	m_nSoundFilterId = 0;
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleWeaponDisable
//
//	PURPOSE:	Handle the weapon being disabled...
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::HandleWeaponDisable(LTBOOL bDisabled)
{
	if (bDisabled)
	{
		ClearPlayerModes(LTTRUE);
	}
	else
	{
		// Force us to re-evaluate what container we're in.  We call
		// UpdateContainers() first to make sure any container changes
		// have been accounted for, then we clear the container code
		// and force an update (this is done for underwater situations like
		// dying underwater and respawning, and also for picking up intelligence
		// items underwater)...

		if (IsPlayerInWorld())
		{
			UpdateContainers();
			ClearCurContainerCode();
			UpdateContainers();
		}

		// RETAIL: not while zoomed, in Tron (retail 0x1012b200)
		if (!IsZoomed())
		{
			m_pClientWeaponMgr->ShowWeapons();
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ClearPlayerModes
//
//	PURPOSE:	Clear any special modes the player is in
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::ClearPlayerModes(LTBOOL bWeaponOnly)
{
	HandleZoomChange( LTTRUE );
	EndZoom();

	if (!bWeaponOnly)
	{
		EndSpyVision();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::RestorePlayerModes
//
//	PURPOSE:	Restore any special modes the player was in 
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::RestorePlayerModes()
{
	if (m_bSpyVision)
	{
		BeginSpyVision();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::AttachCameraToHead
//
//	PURPOSE:	Attach the camera to a socket in the player's head
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::AttachCameraToHead( LTBOOL bAttach )
{
	// We don't want to reset our orientations if we are currently doing what was requested...

	if( m_bCameraAttachedToHead == bAttach )
		return;

	//set the new value
	m_bCameraAttachedToHead	= bAttach;

	//if we are enabling, then cache our values
	if(bAttach)
	{
		m_fModelAttachPitch = m_fPitch;
		m_fModelAttachYaw	= m_fYaw;
		m_fModelAttachRoll	= m_fRoll;
	}
	else
	{
		//we are disabling, make our animation orientation our actual orientation
		m_fPitch	= m_fModelAttachPitch;
		m_fYaw		= m_fModelAttachYaw;
		m_fRoll		= m_fModelAttachRoll;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::GetPlayerHeadPos
//
//	PURPOSE:	Get the player's head position
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::GetPlayerHeadPosRot(LTVector & vPos, LTRotation & rRot)
{
	// RETAIL: Tron's (retail 0x1012b2f0): the player's first body, else
	// the client object if a character FX has it; no animations applied

	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	HOBJECT hBody = NULL;

	// We actually want to use the body prop, so...

	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_BODY_ID);
	if (!pList) return;
	
	int nNumBodies = pList->GetSize();

    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);

	int i;
	for (i=0; i < nNumBodies; i++)
	{
		if ((*pList)[i] && ((CBodyFX*)(*pList)[i])->GetClientId() == dwId)
		{
			hBody = (*pList)[i]->GetServerObj();
			break;
		}
	}

	if( !hBody )
	{
		// If we are alive then we don't have a body and we should use the client object...

		pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CHARACTER_ID);
		if (!pList) return;

		int nNumChars = pList->GetSize();
		HOBJECT hClient = g_pLTClient->GetClientObject();

		for (i=0; i < nNumChars; i++)
		{
			if ((*pList)[i] && (*pList)[i]->GetServerObj() == hClient)
			{
				hBody = hClient;
				break;
			}
		}

		if (!hBody) return;
	}

	if (g_pModelLT->GetSocket(hBody, "Eyes", hSocket) == LT_OK)
	{
		LTransform transform;
        if (g_pModelLT->GetSocketTransform(hBody, hSocket, transform, LTTRUE) == LT_OK)
		{
			vPos = transform.m_Pos;
			rRot = transform.m_Rot;
			m_pPlayerCamera->CalcNonClipPos(vPos, rRot, 0);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::DoActivate
//
//	PURPOSE:	Tell the server to do Activate
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::DoActivate()
{
	char const* pActivateOverride = g_vtActivateOverride.GetStr();
	if (pActivateOverride && pActivateOverride[0] != ' ')
	{
        g_pLTClient->RunConsoleString(( char* )pActivateOverride);
		return;
	}

	CActivationData data = GetTargetMgr()->GetActivationData();

	//don't revive if we are too close
	if (data.m_nType == MID_ACTIVATE_REVIVE && !GetTargetMgr()->CanActivateTarget() )
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_ACTIVATE);
	data.Write(cMsg);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Teleport
//
//	PURPOSE:	Tell the server to teleport to the specified point
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::Teleport(const LTVector & vPos)
{
#ifndef _FINAL // Don't allow player teleporing in the final version...

    LTVector vPitchYawRoll(m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_TELEPORT);
	cMsg.WriteLTVector(vPos);
	cMsg.WriteLTVector(vPitchYawRoll);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

#endif // _FINAL
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateServerPlayerModel()
//
//	PURPOSE:	Puts the server's player model where our invisible one is
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateServerPlayerModel()
{
	HOBJECT hClientObj, hRealObj;

    if (!(hClientObj = g_pLTClient->GetClientObject())) return;

	if (!(hRealObj = m_pMoveMgr->GetObject())) return;

    LTVector myPos;
	g_pLTClient->GetObjectPos(hRealObj, &myPos);
	g_pLTClient->SetObjectPos(hClientObj, &myPos);

	if (g_vtPlayerRotate.GetFloat(1.0) > 0.0)
	{
	    LTRotation myRot(m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
		g_pLTClient->SetObjectRotation(hClientObj, &myRot);
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateContainers
//
//	PURPOSE:	Update anything associated with being in a container
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateContainers()
{
    LTVector vCamPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

    LTVector vScale(1.0f, 1.0f, 1.0f), vLightAdd(0.0f, 0.0f, 0.0f);

    char* pCurSound      = NULL;
	uint8 nSoundFilterId = 0;
    uint32 dwUserFlags   = USRFLG_VISIBLE;
    m_bUseWorldFog       = LTTRUE;

	// We'll update this below...

	m_bInSafetyNet = false;

	// [KLS 4/16/02] Find container objects that we care about...

	HLOCALOBJ hContainerObj = LTNULL;
	ContainerCode eCode = CC_NO_CONTAINER;

	HLOCALOBJ objList[MAX_OVERLAPPING_CONTAINERS];
	uint32 nContainerFlags = (CC_ALL_FLAG & ~CC_PLAYER_IGNORE_FLAGS);
    uint32 dwNum = ::GetPointContainers(vCamPos, objList, MAX_OVERLAPPING_CONTAINERS, nContainerFlags);

	for (uint32 i=0; i < dwNum; i++)
	{
		uint16 code;
		if (g_pLTClient->GetContainerCode(objList[i], &code))
		{
			ContainerCode eTempCode = (ContainerCode)code;

			// Ignore dynamic occluder volumes...
			if (CC_DYNAMIC_OCCLUDER_VOLUME != eTempCode)
			{
				// [KEF 9/03/02] Allow sound filter volumes to be overridden if needed

				// [KLS 9/07/02] Modified so we look at all the containers on the list,
				// and save off saftey nets separately...

				if (CC_SAFTEY_NET == eTempCode)
				{
					// Don't count this as a normal container as it has one specific
					// purpose ONLY (protect the player)...
					m_bInSafetyNet = true;
				}
				else if (CC_NO_CONTAINER == eCode || CC_FILTER == eCode)
				{
					hContainerObj = objList[i];
					eCode = eTempCode;
				}
			}
		}
	}


	// Update Dynamic occluders...

	UpdateDynamicOccluders(objList, dwNum);


	if (hContainerObj)
	{
		// Check for weather volume brush first (weather volume brushes
		// should never overlap normal volume brushes)

		CVolumeBrushFX* pFX = dynamic_cast<CVolumeBrushFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_WEATHER_ID, hContainerObj));
		if (!pFX)
		{
			pFX = dynamic_cast<CVolumeBrushFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_VOLUMEBRUSH_ID, hContainerObj));
		}

		pFX = UpdateVolumeBrushFX(pFX, eCode);

		// See if we have entered/left a container...

        float fTime = g_pLTClient->GetTime();
		
		if (m_eCurContainerCode != eCode)
		{
			m_fContainerStartTime = fTime;

			if (pFX)
			{
				// Set the sound filter override...

				nSoundFilterId = pFX->GetSoundFilterId();

				// See if this container has fog associated with it..

                LTBOOL bFog = pFX->IsFogEnable();

				if (bFog)
				{
                    m_bUseWorldFog = LTFALSE;

					char buf[30];
					sprintf(buf, "FogEnable %d", (int)bFog);
                    g_pLTClient->RunConsoleString(buf);

					sprintf(buf, "FogNearZ %d", (int)pFX->GetFogNearZ());
                    g_pLTClient->RunConsoleString(buf);

					sprintf(buf, "FogFarZ %d", (int)pFX->GetFogFarZ());
                    g_pLTClient->RunConsoleString(buf);

                    LTVector vFogColor = pFX->GetFogColor();

					sprintf(buf, "FogR %d", (int)vFogColor.x);
                    g_pLTClient->RunConsoleString(buf);

					sprintf(buf, "FogG %d", (int)vFogColor.y);
                    g_pLTClient->RunConsoleString(buf);

					sprintf(buf, "FogB %d", (int)vFogColor.z);
                    g_pLTClient->RunConsoleString(buf);
				}

				// Get the tint color...

				vScale = pFX->GetTintColor();
				vScale /= 255.0f;

				vLightAdd = pFX->GetLightAdd();
				vLightAdd /= 255.0f;
			}
		}

		switch (eCode)
		{
			case CC_WATER:
			case CC_CORROSIVE_FLUID:
			case CC_FREEZING_WATER:
			{
				pCurSound = "Chars\\Snd\\Player\\unwater.wav";
			}
			break;

			case CC_ENDLESS_FALL:
			{
                float fFallTime = 1.0f;

				if (fTime > m_fContainerStartTime + fFallTime)
				{
					vScale.Init(0.0f, 0.0f, 0.0f);
				}
				else
				{
                    float fScaleStart = .3f;
                    float fTimeLeft = (m_fContainerStartTime + fFallTime) - fTime;
                    float fScalePercent = fTimeLeft/fFallTime;
                    float fScale = fScaleStart * fScalePercent;

					vScale.Init(fScale, fScale, fScale);
				}
			}
			break;

			default : break;
		}

	} // if (hContainerObj)



	// See if we have entered/left a container...

	if (m_eCurContainerCode != eCode)
	{
		// Adjust world properties as necessary...
		g_pGameClientShell->ResetDynamicWorldProperties(m_bUseWorldFog);

		g_pGameClientShell->GetLightScaleMgr()->ClearLightScale(CLightScaleMgr::eLightScaleEnvironment);

		if (vScale.x != 1.0f || vScale.y != 1.0f || vScale.z != 1.0f)
		{
			g_pGameClientShell->GetLightScaleMgr()->SetLightScale(vScale, CLightScaleMgr::eLightScaleEnvironment);
		}

		// See if we are coming out of water...

		if (IsLiquid(m_eCurContainerCode) && !IsLiquid(eCode))
		{
            UpdateUnderWaterFX(LTFALSE);
 		}

		m_eCurContainerCode = eCode;

		UpdateSoundFilters(nSoundFilterId);

		if (m_hContainerSound)
		{
            g_pLTClient->SoundMgr()->KillSound(m_hContainerSound);
            m_hContainerSound = NULL;
		}

		if (pCurSound)
		{
            uint32 dwFlags = PLAYSOUND_CLIENT | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
			m_hContainerSound = g_pClientSoundMgr->PlaySoundLocal(pCurSound, SOUNDPRIORITY_PLAYER_MEDIUM, dwFlags);
		}

		g_pGameClientShell->GetScreenTintMgr()->Set(TINT_CONTAINER,&vLightAdd);
	}


	// See if we are under water (under any liquid)...

	if (IsLiquid(m_eCurContainerCode))
	{
		UpdateUnderWaterFX();
	}
	else
	{
		UpdateBreathingFX();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateVolumeBrushFX
//
//	PURPOSE:	Deterimine if we're really in a VolumeBrush that has
//				a polygrid surface (i.e., water)
//
//	NOTES:		Passed in eCode may be updated
//
// --------------------------------------------------------------------------- //

CVolumeBrushFX* CPlayerMgr::UpdateVolumeBrushFX(CVolumeBrushFX* pFX, 
												ContainerCode & eCode)
{
	if (!pFX) return LTNULL;

	// Get the PolyGridFX associated with the VolumeBrush...

	// Get a list of all the poly grids...

	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList( SFX_POLYGRID_ID );
	if( pList )
	{
		int nNumPGrids = pList->GetSize();
		HOBJECT hObj = pFX->GetServerObj();

		// Try and find a polygrid that is the surface of our container...
		
		for( int i = 0; i < nNumPGrids; ++i )
		{
			if( (*pList)[i] )
			{
				CPolyGridFX* pPGrid= (CPolyGridFX*)(*pList)[i];

				if( pPGrid->GetVolumeBrush() == hObj )
				{
					float fDisplacement;
					LTVector vIntersection;

					LTRotation rRot;
					g_pLTClient->GetObjectRotation( m_hCamera, &rRot );

					LTVector vCamPos;
					g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

					// Develop a position out and down from the camera to test from...

					LTVector vPos = vCamPos + (rRot.Forward() * 5.0f);
					vPos.y -= 2.0f;
					
					// See if we interected the polygrid...

					if( pPGrid->GetOrientedIntersectionHeight( vPos, LTVector(0,1,0), fDisplacement, vIntersection ))
					{
						vIntersection.y += fDisplacement;

						// If we are above it clear our container and fx...
						
						if( vPos.y > vIntersection.y )
						{
							eCode = CC_NO_CONTAINER;
							pFX = LTNULL;
						}
					}

					break;
				}
			}
		}
	}

	return pFX;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateSoundFilters
//
//	PURPOSE:	Update sound filters
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateSoundFilters(uint8 nSoundFilterId)
{
#ifndef USE_EAX20_HARDWARE_FILTERS
	return;
#endif // USE_EAX20_HARDWARE_FILTERS

	if (g_vtUseSoundFilters.GetFloat())
	{
		if ( m_nSoundFilterId != nSoundFilterId )
		{
			m_nSoundFilterId = nSoundFilterId;

			bool bFilterOK = true;

			ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();
			SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter( nSoundFilterId );

			// tell the sound engine about new filter, if a dynamic filter, 
			// use the global sound filter
			bool bUsingDynamic = false;
			if ( g_pSoundFilterMgr->IsDynamic( pFilter ) )
			{
				bUsingDynamic = true;
				m_nSoundFilterId = m_nGlobalSoundFilterId;
			   	pFilter = g_pSoundFilterMgr->GetFilter( m_nGlobalSoundFilterId );
			}

			if ( !g_pSoundFilterMgr->IsUnFiltered( pFilter ) )
			{
				if ( pSoundMgr->SetSoundFilter( pFilter->szFilterName ) == LT_OK )
				{
					for (int i=0; i < pFilter->nNumVars; i++)
					{
						if ( pSoundMgr->SetSoundFilterParam(pFilter->szVars[i], pFilter->fValues[i]) != LT_OK )
							bFilterOK = false;
					}
				}
				else
				{
					bFilterOK = false;
				}
			}
			else
			{
				bFilterOK = false;
			}

			pSoundMgr->EnableSoundFilter( bFilterOK );

#ifndef _FINAL
			if (g_vtShowSoundFilterInfo.GetFloat())
			{
				g_pLTClient->CPrint("Entering %s sound filter '%s' (%s)", 
					((bUsingDynamic && m_eCurContainerCode == CC_NO_CONTAINER)? "(Global)" : ""), pFilter->szName, 
					(bFilterOK ? "Enabled" : "Disabled"));

				// Display detailed filter info if necessary...
				if (g_vtShowSoundFilterInfo.GetFloat() > 1 && pFilter)
				{
					g_pLTClient->CPrint("  FilterName: '%s'", pFilter->szFilterName);
					for (int i=0; i < pFilter->nNumVars; i++)
					{
						g_pLTClient->CPrint("  '%s' = '%f'", pFilter->szVars[i], pFilter->fValues[i]);
					}
				}
			}
#endif // _FINAL
		
		}
	}

}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateUnderWaterFX
//
//	PURPOSE:	Update under water fx
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateUnderWaterFX(LTBOOL bUpdate)
{
	if (m_nZoomView) return;

    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	if (dwWidth < 0 || dwHeight < 0) return;


	// Initialize to default fov x and y...

    float fFovX = g_vtFOVXNormal.GetFloat();
    float fFovY = g_vtFOVYNormal.GetFloat();

	//see if we are supposed to modify the FOV. Note that we can't modify it while any damage
	//effects are active since they can modify the FOV as well which will cause cumulative issues
	//and ultimately lead to a fairly messed up camera state until it is restored
	// RETAIL: Tron's does not check the damage FX (retail 0x1012c0e0)
	if (bUpdate)
	{
        g_pLTClient->GetCameraFOV(m_hCamera, &fFovX, &fFovY);

		fFovX = RAD2DEG(fFovX);
		fFovY = RAD2DEG(fFovY);

        float fSpeed = g_vtUWFOVRate.GetFloat() * g_pGameClientShell->GetFrameTime();

		if (m_fFovXFXDir > 0)
		{
			fFovX -= fSpeed;
			fFovY += fSpeed;

			if (fFovY > g_vtFOVYMaxUW.GetFloat())
			{
				fFovY = g_vtFOVYMaxUW.GetFloat();
				m_fFovXFXDir = -m_fFovXFXDir;
			}
		}
		else
		{
			fFovX += fSpeed;
			fFovY -= fSpeed;

			if (fFovY < g_vtFOVYMinUW.GetFloat())
			{
				fFovY = g_vtFOVYMinUW.GetFloat();
				m_fFovXFXDir = -m_fFovXFXDir;
			}
		}
	}

	SetCameraFOV(DEG2RAD(fFovX), DEG2RAD(fFovY));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateBreathingFX
//
//	PURPOSE:	Update breathing fx
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateBreathingFX(LTBOOL bUpdate)
{
	//if (m_nZoomView) return;

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateDynamicOccluders
//
//	PURPOSE:	Update dynamic occluders 
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateDynamicOccluders(HLOCALOBJ* pContainerArray, uint32 nNumContainers)
{
	CDynamicOccluderVolumeFX* enabledOccluderFX[50];
	CDynamicOccluderVolumeFX* disabledOccluderFX[50];
	uint32 nNumEnabledOccluderFX = 0;
	uint32 nNumDisabledOccluderFX = 0;

	// Find the occluder volumes fx that we wish to enable...

	for (uint32 i=0; i < nNumContainers; i++)
	{
		// We only care about "visible" containers...

		uint32 dwUserFlags = USRFLG_VISIBLE;
		g_pCommonLT->GetObjectFlags(pContainerArray[i], OFT_User, dwUserFlags);
	
		if (dwUserFlags & USRFLG_VISIBLE)
		{
			uint16 code;
			if (g_pLTClient->GetContainerCode(pContainerArray[i], &code))
			{
				ContainerCode eTempCode = (ContainerCode)code;

				if (CC_DYNAMIC_OCCLUDER_VOLUME == eTempCode)
				{
					enabledOccluderFX[nNumEnabledOccluderFX] = (CDynamicOccluderVolumeFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_DYNAMIC_OCCLUDER_ID, pContainerArray[i]);
					nNumEnabledOccluderFX++;
				}
			}
		}
	}

	// Find the occluder volume FX that we wish to disable...

	CSpecialFXList* pDynOccluderList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_DYNAMIC_OCCLUDER_ID);
	if (pDynOccluderList)
	{
		uint32 nNum = pDynOccluderList->GetSize();

		for (uint32 i=0; i < nNum; i++)
		{
			CDynamicOccluderVolumeFX* pFX = (CDynamicOccluderVolumeFX*)(*pDynOccluderList)[i];
			if (!pFX) continue;

			// Check to see if this fx is on the enabled list...

			bool bEnabled = false;
			for (uint32 j=0; j < nNumEnabledOccluderFX; j++)
			{
				if (pFX == enabledOccluderFX[j])
				{
					bEnabled = true;
					break;
				}
			}

			// Add the fx to the disabled list...

			if (!bEnabled)
			{
				disabledOccluderFX[nNumDisabledOccluderFX] = pFX;
				nNumDisabledOccluderFX++;
			}
		}
	}


	// Disable the necessary occluder volumes...

	for (i=0; i < nNumDisabledOccluderFX; i++)
	{
		if (disabledOccluderFX[i])
		{
			(disabledOccluderFX[i])->Enable(false);
		}
	}

	// Enable the necessary occluder volumes...

	for (i=0; i < nNumEnabledOccluderFX; i++)
	{
		if (enabledOccluderFX[i])
		{
			(enabledOccluderFX[i])->Enable(true);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetMouseInput()
//
//	PURPOSE:	Allows or disallows mouse input on the client
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetMouseInput(LTBOOL bAllowInput)
{
	// RETAIL: build 51's, Tron's (retail 0x1012c470): the backup angles are
	// always restored
	if (bAllowInput)
	{
        m_bRestoreOrientation = LTTRUE;
		m_fYaw   = m_fYawBackup;
		m_fPitch = m_fPitchBackup;
	}
	else
	{
		m_fYawBackup = m_fYaw;
		m_fPitchBackup = m_fPitch;
	}
}

void CPlayerMgr::AllowPlayerMovement(LTBOOL bAllowPlayerMovement)
{
	m_bAllowPlayerMovement = bAllowPlayerMovement;

	SetMouseInput(bAllowPlayerMovement);
}


void CPlayerMgr::ResetCamera()
{
	// Make sure the FOV is set correctly...

    uint32 dwWidth = 640;
    uint32 dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

    g_pLTClient->SetCameraRect(m_hCamera, LTTRUE, 0, 0, dwWidth, dwHeight);
	SetCameraFOV(DEG2RAD(g_vtFOVXNormal.GetFloat()), DEG2RAD(g_vtFOVYNormal.GetFloat()));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ShowPlayer()
//
//	PURPOSE:	Show/Hide the player object
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::ShowPlayer(LTBOOL bShow)
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	g_pCommonLT->SetObjectFlags(hPlayerObj, OFT_Flags, (bShow ? FLAG_VISIBLE : 0), FLAG_VISIBLE);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayer()
//
//	PURPOSE:	Update the player
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlayer()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj || IsPlayerDead()) return;


	// This is pretty much a complete kludge, but I can't really think of
	// a better way to handle this...Okay, since the server can update the
	// player's flags at any time (and override anything that we set), we'll
	// make sure that the player's flags are always what we want them to be :)

    uint32 dwPlayerFlags;
	g_pCommonLT->GetObjectFlags(hPlayerObj, OFT_Flags, dwPlayerFlags);

	// RETAIL: Tron's tests the camera mode (retail 0x1012c5e0)
	CPlayerCamera::CameraMode eMode = m_pPlayerCamera->GetCameraMode();
	if (eMode == CPlayerCamera::FIRSTPERSON)
	{
		if (dwPlayerFlags & FLAG_VISIBLE)
		{
            ShowPlayer(LTFALSE);
		}
	}
	else if (eMode == CPlayerCamera::CHASE)
	{
		if (!(dwPlayerFlags & FLAG_VISIBLE))
		{
            ShowPlayer(LTTRUE);
		}
	}
	else if (eMode == CPlayerCamera::TRONMODE5)
	{
		if (dwPlayerFlags & FLAG_VISIBLE)
		{
            ShowPlayer(LTFALSE);
		}
	}


	// Hide/Show our attachments...

	HideShowAttachments(hPlayerObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HideShowAttachments()
//
//	PURPOSE:	Recursively hide/show attachments...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HideShowAttachments(HOBJECT hObj)
{
	if (!hObj) return;

	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

    g_pCommonLT->GetAttachments(hObj, attachList, 20, dwListSize, dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	for (int i=0; i < nNum; i++)
	{
        uint32 dwUsrFlags;
        g_pCommonLT->GetObjectFlags(attachList[i], OFT_User, dwUsrFlags);

        if (dwUsrFlags & USRFLG_ATTACH_HIDE1SHOW3)
		{
			// RETAIL: hidden on a light cycle too, in Tron (retail 0x1012c670)
			if (!m_pPlayerCamera->IsFirstPerson() && g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() != PPM_LIGHTCYCLE)
			{
				g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			}
			else
			{
				g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
			}
		}
		else if (dwUsrFlags & USRFLG_ATTACH_HIDE1)
		{
			if (m_pPlayerCamera->IsFirstPerson())
			{
				g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
			}
		}

		if (g_pVersionMgr->IsLowViolence() && dwUsrFlags & USRFLG_ATTACH_HIDEGORE)
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
		}

		// Hide/Show this attachment's attachments...
		HideShowAttachments(attachList[i]);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	SVModelHook
//
//	PURPOSE:	Special Rendering Code for SpyVision Powerup
//
// --------------------------------------------------------------------------- //
/*
void SVModelHook(ModelHookData *pData, void *pUser)
{

	CGameClientShell* pShell = (CGameClientShell*) pUser;
	if (!pShell) return;

    uint32 nUserFlags = 0;
    g_pCommonLT->GetObjectFlags(pData->m_hObject, OFT_User, nUserFlags);
	if (nUserFlags & USRFLG_SPY_VISION)
	{
		// Do the poor-man's glow if necessary...
		bool bCanDoGlow = true;  // NEED TO CALL API FUNCTION FOR THIS!!!!

		if (!bCanDoGlow)
		{
			pData->m_HookFlags &= ~MHF_USETEXTURE;
			pData->m_LightAdd = g_vSVModelColor;
		}

		// pData->m_ObjectColor = g_vSVModelColor;
		// pData->m_ObjectFlags |= FLAG_NOLIGHT;
	}
	else
	{
		DefaultModelHook(pData, pUser);
	}
}
*/

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::IsSearching()
//
//	PURPOSE:	Return whether or not the searcher object is searching
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::IsSearching()
{ 
	return false;	// RETAIL: no searcher in Tron
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::IsDisabling()
//
//	PURPOSE:	Return whether or not the player is currently disabling a gadget target
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::IsDisabling()
{ 
	return !!(m_pGadgetDisabler->IsDisabling());
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetCarryingObject()
//
//	PURPOSE:	Set the player to carry an object...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetCarryingObject( uint8 nCarry )
{	
	// RETAIL: Tron's, as build 51's SetCarryingBody (retail 0x1012c7d0): one
	// argument, the duck lock cleared, no HUD update
	if( m_nCarryingObject != nCarry )
	{
		m_nCarryingObject = nCarry;

		if( nCarry )
		{
			m_bCameraDip = true;
			m_pMoveMgr->SetDuckLock(LTFALSE);
			m_pClientWeaponMgr->DisableWeapons();
		}
		else
		{
			m_bCameraDip = false;
			m_pClientWeaponMgr->EnableWeapons();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetCanDropCarriedObject()
//
//	PURPOSE:	RETAIL: not inline in Tron (retail 0x1012c820)
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetCanDropCarriedObject( bool bCanDrop )
{
	m_bCanDropCarriedObject = bCanDrop;
}

//pulled target mgr initialization into a function so that it maybe overridden by 
// a game specific TargetMgr
void CPlayerMgr::InitTargetMgr()
{
	m_pTargetMgr = debug_new( CTargetMgr );				 
	ASSERT( 0 != m_pTargetMgr );
}


void CPlayerMgr::ClearDamageSectors()
{
	memset(m_fDamage,0,sizeof(m_fDamage));
}

void CPlayerMgr::UpdateDamage()
{
	if (g_pGameClientShell->IsGamePaused()) return;

	float fDelta = g_pGameClientShell->GetFrameTime() * g_vtDamageFadeRate.GetFloat();
	for (int i = 0; i < kNumDamageSectors; i++)
	{
		if (m_fDamage[i] > fDelta)
			m_fDamage[i] -= fDelta;
		else
			m_fDamage[i] = 0.0f;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateDistanceIndicator()
//
//	PURPOSE:	If we are close to a distance indicator object update the huditem...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateDistanceIndicator()
{
	float fLastDistPercent = m_fDistanceIndicatorPercent;
	m_fDistanceIndicatorPercent = -1.0f;

	// Currently the only distance indicator objects are triggers...

	CSpecialFXList* pTriggerList = g_pGameClientShell->GetSFXMgr()->GetFXList( SFX_TRIGGER_ID );
	if( pTriggerList )
	{
		uint32 nNum = pTriggerList->GetNumItems();

		for( uint32 i = 0; i < nNum; ++i )
		{
			CTriggerFX* pFX = (CTriggerFX*)(*pTriggerList)[i];
			if( !pFX || !pFX->WithinIndicatorRadius() ) continue;

			// Just grab the first distance being tracked.  The distance indicators
			// shouldn't overlap.  If they do it's an LD issue or this needs to change :)

			m_fDistanceIndicatorPercent	= pFX->GetDistancePercentage();
			m_hDistanceIndicatorIcon	= pFX->GetIcon();
			break;
		}
	}

	if( (m_fDistanceIndicatorPercent != fLastDistPercent) && m_hDistanceIndicatorIcon )
		g_pHUDMgr->QueueUpdate( kHUDDistance );
}





uint8 CPlayerMgr::GetGadgetFromDamageType(DamageType eDamageType)
{
	IClientWeaponBase *pClientWeapon = m_pClientWeaponMgr->GetCurrentClientWeapon();
	uint8 nCurWeapon = WMGR_INVALID_ID;
	if ( pClientWeapon )
	{
		// Check if we have ammo, we may have just run out...
		if (pClientWeapon->HasAmmo())
		{
			nCurWeapon = pClientWeapon->GetWeaponId();
		}
		else
		{
			// Start with our next weapon that has ammo...

			nCurWeapon = m_pClientWeaponMgr->GetNextWeaponId(nCurWeapon, 0);
		}
	}

	uint8 nTestWeapon = nCurWeapon;

	do
	{
		const WEAPON* pWpnData = g_pWeaponMgr->GetWeapon(nTestWeapon);
		if (pWpnData)
		{
			const AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(pWpnData->nDefaultAmmoId);
			if (pAmmoData->eInstDamageType == eDamageType)
			{
				return nTestWeapon;
			}
		}

		nTestWeapon = m_pClientWeaponMgr->GetNextWeaponId(nTestWeapon, 0);

	} while (nTestWeapon != nCurWeapon && nTestWeapon != WMGR_INVALID_ID);



	return WMGR_INVALID_ID;

}


bool CPlayerMgr::FireOnActivate()
{
	if (!GetTargetMgr()->IsTargetGadgetActivatable()) return false;
	if (GetGadgetDisabler()->DisableOnActivate()) return false;

	DamageType eDT = GetTargetMgr()->RequiredGadgetDamageType();

	IClientWeaponBase *pClientWeapon = m_pClientWeaponMgr->GetCurrentClientWeapon();
	uint8 nCurWeapon = WMGR_INVALID_ID;
	if ( pClientWeapon )
		nCurWeapon = pClientWeapon->GetWeaponId();

	const WEAPON* pWpnData = g_pWeaponMgr->GetWeapon(nCurWeapon);
	if (pWpnData)
	{
		const AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(pWpnData->nDefaultAmmoId);
		if (pAmmoData->eInstDamageType == eDT)
		{
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::TronUnknownVirtual58
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1012c990): the damage FX and the
//				damage sectors cleared, the dead timer reset, and the current
//				weapon's TronUnknownVirtual20 (HandleMsgPlayerStateChange's
//				PS_DEAD). Not yet named
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::TronUnknownVirtual58()
{
	g_pDamageFXMgr->Clear();
	ClearDamageSectors();
	s_fDeadTimer = 0.0f;
	m_pClientWeaponMgr->CurrentWeaponTronUnknownVirtual20();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetTronUnknown1B5
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1012c9c0; the name is ours)
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetTronUnknown1B5(const char* pszString)
{
	strcpy(m_szTronUnknown1B5, pszString);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetTronUnknown1D5
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1012c9f0; the name is ours): on,
//				the light cycle camera pans (the LCCPan* console variables
//				override its angle, radius and time); off, clears +0x1d6 too
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetTronUnknown1D5(bool bOn)
{
	m_bTronUnknown1D5 = bOn;

	if (bOn)
	{
		float fPhi = 20.0f;
		float fRadius, fTime;

		if (m_pPlayerCamera->IsFirstPerson())
		{
			fRadius = 300.0f;
			fTime = 1.0f;
		}
		else
		{
			fRadius = 2000.0f;
			fTime = 1.5f;
		}

		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCCPanPhi");
		if (hVar)
		{
			fPhi = g_pLTClient->GetVarValueFloat(hVar);
		}

		hVar = g_pLTClient->GetConsoleVar("LCCPanRadius");
		if (hVar)
		{
			fRadius = g_pLTClient->GetVarValueFloat(hVar);
		}

		hVar = g_pLTClient->GetConsoleVar("LCCPanTime");
		if (hVar)
		{
			fTime = g_pLTClient->GetVarValueFloat(hVar);
		}

		m_pPlayerCamera->TronUnknownStartPan(DEG2RAD(fPhi), fRadius, fTime);
	}
	else
	{
		m_bTronUnknown1D6 = false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetTronUnknown1DC
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1012cb10; the name is ours): 0 sets
//				the default (5000); the camera's +0x100 is kept within it
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetTronUnknown1DC(float fValue)
{
	if (fValue == 0.0f)
	{
		m_fTronUnknown1DC = 5000.0f;
	}
	else
	{
		m_fTronUnknown1DC = fValue;
	}

	if (m_pPlayerCamera->GetTronUnknown100() > m_fTronUnknown1DC)
	{
		m_pPlayerCamera->SetTronUnknown100(m_fTronUnknown1DC);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::StartDeathFX
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1012cb60; the name is ours): unless
//				it already plays, the single player death FX, without the
//				HUD; once it is done, Update fails the mission
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::StartDeathFX()
{
	if (s_DeathFX.IsValid()) return;

	m_bTronUnknown1D6 = true;

	char szDeathFX[128];
	szDeathFX[0] = '\0';
	g_pClientButeMgr->GetDeathFX(szDeathFX, sizeof(szDeathFX));

	g_pInterfaceMgr->SetHUDRenderLevel(kHUDRenderNone);

	if (szDeathFX[0])
	{
		LTVector vPos(0.0f, 0.0f, 0.0f);
		CLIENTFX_CREATESTRUCT fxInit( szDeathFX, FXFLAG_REALLYCLOSE, vPos );
		g_pClientFXMgr->CreateClientFX( &s_DeathFX, fxInit, LTTRUE );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::IsDeathFXDone
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1012cc90; the name is ours): is the
//				death FX done, with +0x1d6 set or the player dead?
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::IsDeathFXDone()
{
	bool bDone = (!s_DeathFX.GetInstance() || s_DeathFX.GetInstance()->IsDone());

	if (m_bTronUnknown1D6 && bDone)
		return true;

	if (IsPlayerDead() && bDone)
		return true;

	return false;
}

