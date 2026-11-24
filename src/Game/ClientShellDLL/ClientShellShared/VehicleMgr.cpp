// ----------------------------------------------------------------------- //
//
// MODULE  : VehicleMgr.cpp
//
// PURPOSE : Client side vehicle mgr - Implementation
//
// CREATED : 6/12/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VehicleMgr.h"
#include "VarTrack.h"
#include "MsgIDs.h"
#include "CharacterFX.h"
#include "CMoveMgr.h"
#include "PlayerMgr.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientSoundMgr.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "LTEulerAngles.h"
#include "JumpVolumeFX.h"
#include "PlayerLureFX.h"
#include "FXButeMgr.h"
#include "SurfaceFunctions.h"
#include "PlayerViewAttachmentMgr.h"
#include "MissionMgr.h"
#include "PlayerCamera.h"

#define ROUNDFLOAT(f) ((int)((f) + ((f) > 0 ? 0.5 : -0.5)))

#define SNOWMOBILE_SLIDE_TO_STOP_TIME	5.0f

#define VEHICLE_GEAR_0					0
#define VEHICLE_GEAR_1					1
#define VEHICLE_GEAR_2					2

#define DEFAULT_WORLD_GRAVITY			-2000.0f

extern CMoveMgr* g_pMoveMgr;
extern VarTrack g_vtMaxVehicleHeadCant;

// [KLS 6/29/02] - Added more snowmobile debugging info...
#define		SNOW_TRACK_GENERAL		1.0f
#define		SNOW_TRACK_ANIMATION	2.0f
#define		SNOW_TRACK_VERBOSE		3.0f

VarTrack	g_vtSnowmobileInfoTrack;
VarTrack	g_vtSnowmobileVel;
VarTrack	g_vtLightCycleVel;
VarTrack	g_vtLightCycleGridSize;

VarTrack	g_vtSnowmobileAccel;
VarTrack	g_vtSnowmobileAccelTime;
VarTrack	g_vtSnowmobileMaxDecel;
VarTrack	g_vtSnowmobileDecel;
VarTrack	g_vtSnowmobileDecelTime;
VarTrack	g_vtSnowmobileMaxBrake;
VarTrack	g_vtSnowmobileBrakeTime;

VarTrack	g_vtSnowmobileOffsetX;
VarTrack	g_vtSnowmobileOffsetY;
VarTrack	g_vtSnowmobileOffsetZ;

VarTrack	g_vtBicycleInterBumpTimeRange;
VarTrack	g_vtBicycleIntraBumpTimeRange;
VarTrack	g_vtBicycleBumpHeightRange;
VarTrack	g_vtBicycleBobRate;
VarTrack	g_vtBicycleBobSwayAmplitude;
VarTrack	g_vtBicycleBobRollAmplitude;
VarTrack	g_vtBicycleModelOffset;


VarTrack	g_vtSnowmobileStoppedPercent;
VarTrack	g_vtSnowmobileStoppedTurnPercent;
VarTrack	g_vtSnowmobileMaxSpeedPercent;
VarTrack	g_vtSnowmobileAccelGearChange;

VarTrack	g_vtSnowmobileMinTurnAccel;

VarTrack	g_vtVehicleImpactDamage;
VarTrack	g_vtVehicleImpactDamageMin;
VarTrack	g_vtVehicleImpactDamageMax;
VarTrack	g_vtVehicleImpactDamageVelPercent;
VarTrack	g_vtVehicleImpactPushVelPercent;
VarTrack	g_vtVehicleImpactMinAngle;
VarTrack	g_vtVehicleImpactMaxAdjustAngle;
VarTrack	g_vtVehicleImpactPushAmount;
VarTrack	g_vtVehicleImpactPushRadius;
VarTrack	g_vtVehicleImpactPushDuration;
VarTrack	g_vtVehicleImpactFXOffsetForward;
VarTrack	g_vtVehicleImpactFXOffsetUp;
VarTrack	g_vtVehicleImpactThreshold;

VarTrack	g_vtVehicleImpactAIDamageVelPercent;
VarTrack	g_vtVehicleImpactAIMinDamage;
VarTrack	g_vtVehicleImpactAIMaxDamage;
VarTrack	g_vtVehicleImpactAIDoNormalCollision;

VarTrack	g_vtVehicleFallDamageMinHeight;
VarTrack	g_vtVehicleFallDamageMaxHeight;

VarTrack	g_vtVehicleTurnRateScale;
VarTrack	g_vtVehicleTurnRate;
VarTrack	g_vtVehicleInitialTurnRate;
// RETAIL: Tron's (retail 0x10271808, 0x102717f8)
VarTrack	g_vtVehicleTurnScaleStart;
VarTrack	g_vtVehicleTurnScaleEnd;

VarTrack	g_vtVehicleMouseMinYawBuffer;
VarTrack	g_vtVehicleMouseMaxYawBuffer;

VarTrack	g_vtVehicleMaxHeadOffsetX;
VarTrack	g_vtVehicleMaxHeadOffsetY;
VarTrack	g_vtVehicleMaxHeadOffsetZ;

VarTrack	g_vtVehicleMaxHeadYaw;

VarTrack	g_vtVehicleCollisionInfo;

VarTrack	g_vtJumpVolumeWaveType;
VarTrack	g_vtJumpVolumeMaxDirectionPercent;	// Give 100% if at or above this max
VarTrack	g_vtJumpVolumeMinDirectionPercent;	// Give 0% if at or below this min
VarTrack	g_vtJumpVolumeInfo;

VarTrack	g_vtVehicleContourPlayerViewModel;
VarTrack	g_vtVehicleContourPoints;
VarTrack	g_vtVehicleContourExtraDimsX;
VarTrack	g_vtVehicleContourExtraDimsZ;
VarTrack	g_vtVehicleCamContourExtraDimsX;
VarTrack	g_vtVehicleCamContourExtraDimsZ;
VarTrack	g_vtVehicleContourMaxRotation;
VarTrack	g_vtVehicleCamContourMaxRotation;
VarTrack	g_vtVehicleContourRate;
// RETAIL: Tron has no "VehicleContour" (retail 0x1011b560 ends with the rate)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WrapAngleToCircle
//
//	PURPOSE:	Put an angle in the 0 to 2pi range.  RETAIL: 0x1011a190
//				(Tron's; the name is ours. Retail calls it from
//				AlignAngleToAxis and 0x101219b0)
//
// ----------------------------------------------------------------------- //

void WrapAngleToCircle(float &fAngle)
{
	if (fAngle < 0.0f)
	{
		fAngle += (1 - (int)(fAngle * (1.0f / MATH_CIRCLE))) * MATH_CIRCLE;
	}
	else if (fAngle > MATH_CIRCLE)
	{
		fAngle -= (int)(fAngle * (1.0f / MATH_CIRCLE)) * MATH_CIRCLE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TurnAngleTowards
//
//	PURPOSE:	Turn an angle towards a target angle by a step, the short
//				way round the circle.  RETAIL: 0x1011a200 (Tron's; the
//				name is ours. Retail calls it from 0x101219b0)
//
// ----------------------------------------------------------------------- //

void TurnAngleTowards(float &fAngle, float fTarget, float fStep)
{
	float fDiff = fTarget - fAngle;
	if (fDiff > MATH_EPSILON || fDiff < -MATH_EPSILON)
	{
		if (fabs(fDiff) < fStep)
		{
			fAngle = fTarget;
		}
		// RETAIL: one subtract and one add for both directions
		else if (fTarget > fAngle ? fDiff <= MATH_PI : fAngle - fTarget > MATH_PI)
		{
			fAngle += fStep;
		}
		else
		{
			fAngle -= fStep;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelNodeControlFn
//
//	PURPOSE:	Node control callback
//
// ----------------------------------------------------------------------- //

void ModelNodeControlFn(const NodeControlData& Data, void *pUserData)
{
	CVehicleMgr* pMgr = (CVehicleMgr*)pUserData;
	_ASSERT(pMgr);
	if (!pMgr) return;

	pMgr->HandleNodeControl(Data.m_hModel, Data.m_hNode, Data.m_pNodeTransform);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CVehicleMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVehicleMgr::CVehicleMgr()
{
	m_hVehicleModel			= LTNULL;
	m_hVehicleAttachSocket	= INVALID_MODEL_SOCKET;

    m_hVehicleStartSnd  = LTNULL;
    m_hVehicleAccelSnd  = LTNULL;
    m_hVehicleDecelSnd  = LTNULL;
	m_hVehicleImpactSnd = LTNULL;
	// RETAIL: 0x1011a2b0 clears the cruise and +0x2c sounds here.
	m_hVehicleCruiseSnd	= LTNULL;
	m_hTronUnknownSnd2c	= LTNULL;
	m_hIdleSnd			= LTNULL;
	m_hBrakeSnd			= LTNULL;

	m_vVehicleOffset.Init();
	m_vHeadOffset.Init();
	m_fHeadYaw = 0.0f;

    m_bVehicleTurning       = LTFALSE;
	m_nVehicleTurnDirection	= 0;

    m_bVehicleStopped    = LTTRUE;
    m_bVehicleAtMaxSpeed = LTFALSE;

	m_fHandlebarRoll = 0.0f;
	m_fContourRoll = 0.0f;
	m_vTronUnknown211c.Init();
	m_nTronUnknown2134 = 0;
	m_bTurned = false;

	m_vLastPlayerAngles.Init();
	m_vLastCameraAngles.Init();
	m_vLastVehiclePYR.Init();
	m_vVehiclePYR.Init();

	m_fLightCycleCamSpeed = 0.4f;
	m_fLightCycleCamLag = 1.0f;
	m_bTronUnknown2170 = false;

	InitWorldData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::~CVehicleMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CVehicleMgr::~CVehicleMgr()
{
	TermLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::Init
//
//	PURPOSE:	Initialize mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::Init()
{
    g_vtSnowmobileInfoTrack.Init(g_pLTClient, "SnowmobileInfo", NULL, 0.0f);
    g_vtSnowmobileVel.Init(g_pLTClient, "SnowmobileVel", NULL, 600.0f);
	g_vtLightCycleVel.Init(g_pLTClient, "LightCycleVel", NULL, 800.0f);
	g_vtLightCycleGridSize.Init(g_pLTClient, "LightCycleGridSize", NULL, 1.0f);

	g_vtSnowmobileAccel.Init(g_pLTClient, "SnowmobileAccel", NULL, 4000.0f);	// RETAIL: Tron's defaults (0x1011a7ec..)
	g_vtSnowmobileAccelTime.Init(g_pLTClient, "SnowmobileAccelTime", NULL, 3.0f);
	g_vtSnowmobileMaxDecel.Init(g_pLTClient, "SnowmobileDecelMax", LTNULL, 30.0f);
	g_vtSnowmobileDecel.Init(g_pLTClient, "SnowmobileDecel", LTNULL, 50.0f);
    g_vtSnowmobileDecelTime.Init(g_pLTClient, "SnowmobileDecelTime", LTNULL, 0.5f);
	g_vtSnowmobileMaxBrake.Init(g_pLTClient, "SnowmobileBrakeMax", LTNULL, 150.0f);
    g_vtSnowmobileBrakeTime.Init(g_pLTClient, "SnowmobileBrakeTime", LTNULL, 1.0f);

    g_vtSnowmobileMinTurnAccel.Init(g_pLTClient, "SnowmobileMinTurnAccel", LTNULL, 0.0f);

	g_vtSnowmobileOffsetX.Init(g_pLTClient, "SnowmobileOffsetX", LTNULL, 0.0f);
    g_vtSnowmobileOffsetY.Init(g_pLTClient, "SnowmobileOffsetY", LTNULL, -0.8f);
    g_vtSnowmobileOffsetZ.Init(g_pLTClient, "SnowmobileOffsetZ", LTNULL, 0.6f);

	g_vtBicycleInterBumpTimeRange.Init(g_pLTClient, "BicycleInterBumpTimeRange", "0.75 1.25", 0.0f );
	g_vtBicycleIntraBumpTimeRange.Init(g_pLTClient, "BicycleIntraBumpTimeRange", "0.05 0.15", 0.0f );
	g_vtBicycleBumpHeightRange.Init(g_pLTClient, "BicycleBumpHeightRange", "0 0", 0.0f );
	g_vtBicycleBobRate.Init(g_pLTClient, "BicycleBobRate", NULL, 2.5f );
	g_vtBicycleBobSwayAmplitude.Init(g_pLTClient, "BicycleBobSwayAmplitude", NULL, 2.0f );
	g_vtBicycleBobRollAmplitude.Init(g_pLTClient, "BicycleBobRollAmplitude", NULL, -0.25f );
	g_vtBicycleModelOffset.Init(g_pLTClient, "BicycleModelOffset", "0.0 -20.0 5.0", 0.0f );


    g_vtSnowmobileStoppedPercent.Init(g_pLTClient, "SnowmobileStoppedPercent", LTNULL, 0.05f);
    g_vtSnowmobileStoppedTurnPercent.Init(g_pLTClient, "SnowmobileStoppedTurnPercent", LTNULL, 0.2f);
    g_vtSnowmobileMaxSpeedPercent.Init(g_pLTClient, "SnowmobileMaxSpeedPercent", LTNULL, 0.95f);

    g_vtSnowmobileAccelGearChange.Init(g_pLTClient, "SnowmobileAccelGearChange", LTNULL, 1.0f);


	g_vtVehicleImpactDamage.Init(g_pLTClient, "VehicleImpactDamage", LTNULL, 0.0f);
	g_vtVehicleImpactDamageMin.Init(g_pLTClient, "VehicleImpactDamageMin", LTNULL, 5.0f);
	g_vtVehicleImpactDamageMax.Init(g_pLTClient, "VehicleImpactDamageMax", LTNULL, 100.0f);
	g_vtVehicleImpactDamageVelPercent.Init(g_pLTClient, "VehicleImpactDamageVelPerent", LTNULL, 0.9f);
	g_vtVehicleImpactPushVelPercent.Init(g_pLTClient, "VehicleImpactPushVelPerent", LTNULL, 0.0f);
	g_vtVehicleImpactMinAngle.Init(g_pLTClient, "VehicleImpactMinAngle", LTNULL, 20.0f);
	g_vtVehicleImpactMaxAdjustAngle.Init(g_pLTClient, "VehicleImpactMinAdjustAngle", LTNULL, 10.0f);
	g_vtVehicleImpactPushAmount.Init(g_pLTClient, "VehicleImpactPushAmount", LTNULL, 400.0f);
	g_vtVehicleImpactPushRadius.Init(g_pLTClient, "VehicleImpactPushRadius", LTNULL, 100.0f);
	g_vtVehicleImpactPushDuration.Init(g_pLTClient, "VehicleImpactPushDuration", LTNULL, 0.25f);

	g_vtVehicleImpactFXOffsetForward.Init(g_pLTClient, "VehicleImpactFXOffsetForward", LTNULL, 0.0f);
	g_vtVehicleImpactFXOffsetUp.Init(g_pLTClient, "VehicleImpactFXOffsetUp", LTNULL, 0.0f);

	g_vtVehicleImpactThreshold.Init(g_pLTClient, "VehicleImpactThreshold", LTNULL, 0.8f);

	g_vtVehicleImpactAIDamageVelPercent.Init(g_pLTClient, "VehicleImpactAIDamageVelPerent", LTNULL, 0.1f);
 	g_vtVehicleImpactAIMinDamage.Init(g_pLTClient, "VehicleImpactAIMinDamage", LTNULL, 1.0f);
 	g_vtVehicleImpactAIMaxDamage.Init(g_pLTClient, "VehicleImpactAIMaxDamage", LTNULL, 100.0f);
 	g_vtVehicleImpactAIDoNormalCollision.Init(g_pLTClient, "VehicleImpactAIDoNormalCollision", LTNULL, 1.0f);

	g_vtVehicleFallDamageMinHeight.Init(g_pLTClient, "VehicleFallDamageMinHeight", LTNULL, 400.0f);
 	g_vtVehicleFallDamageMaxHeight.Init(g_pLTClient, "VehicleFallDamageMaxHeight", LTNULL, 800.0f);

    g_vtVehicleTurnRateScale.Init(g_pLTClient, "VehicleTurnRateScale", NULL, 1.0f);
	g_vtVehicleTurnScaleStart.Init(g_pLTClient, "VehicleTurnScaleStart", NULL, 1.0f);
	g_vtVehicleTurnScaleEnd.Init(g_pLTClient, "VehicleTurnScaleEnd", NULL, 0.5f);

	g_vtVehicleTurnRate.Init(g_pLTClient, "VehicleTurnRate", NULL, 3.5f);
 	g_vtVehicleInitialTurnRate.Init(g_pLTClient, "VehicleInitialTurnRate", NULL, 1.0f);

	g_vtVehicleMouseMinYawBuffer.Init(g_pLTClient, "VehicleMouseMinYawBuffer", NULL, 5.0f);	// RETAIL: 5 (0x1011b354)
	g_vtVehicleMouseMaxYawBuffer.Init(g_pLTClient, "VehicleMouseMaxYawBuffer", NULL, 45.0f);	// RETAIL: 45 (0x1011b370)

	g_vtVehicleMaxHeadOffsetX.Init(g_pLTClient, "VehicleHeadMaxOffsetX", NULL, 0.0f);
	g_vtVehicleMaxHeadOffsetY.Init(g_pLTClient, "VehicleHeadMaxOffsetY", NULL, 0.0f);
	g_vtVehicleMaxHeadOffsetZ.Init(g_pLTClient, "VehicleHeadMaxOffsetZ", NULL, 0.0f);
	g_vtVehicleMaxHeadYaw.Init(g_pLTClient, "VehicleHeadMaxYaw", NULL, -1.0f);

    g_vtVehicleCollisionInfo.Init(g_pLTClient, "VehicleCollisionInfo", LTNULL, 0.0f);

	g_vtJumpVolumeWaveType.Init( g_pLTClient, "JumpVolumeWaveType", LTNULL, 3.0f );
	g_vtJumpVolumeMaxDirectionPercent.Init( g_pLTClient, "JumpVolumeMaxDirectionPercent", LTNULL, 0.97f );
	g_vtJumpVolumeMinDirectionPercent.Init( g_pLTClient, "JumpVolumeMinDirectionPercent", LTNULL, 0.35f );
	g_vtJumpVolumeInfo.Init( g_pLTClient, "JumpVolumeInfo", LTNULL, 0.0f );

	g_vtVehicleContourPlayerViewModel.Init( g_pLTClient, "VehicleContourPlayerViewModel", LTNULL, 1.0f );
	g_vtVehicleContourPoints.Init( g_pLTClient, "VehicleContourPoints", LTNULL, 2.0f );
	g_vtVehicleContourExtraDimsX.Init( g_pLTClient, "VehicleContourExtraDimsX", LTNULL, 24.0f );
	g_vtVehicleContourExtraDimsZ.Init( g_pLTClient, "VehicleContourExtraDimsZ", LTNULL, 32.0f );
	g_vtVehicleCamContourExtraDimsX.Init( g_pLTClient, "VehicleCamCountourExtraDimsX", LTNULL, 48.0f );
	g_vtVehicleCamContourExtraDimsZ.Init( g_pLTClient, "VehicleCamCountourExtraDimsZ", LTNULL, 64.0f );
	g_vtVehicleContourMaxRotation.Init( g_pLTClient, "VehicleContourMaxRotation", LTNULL, 65.0f );
	g_vtVehicleCamContourMaxRotation.Init( g_pLTClient, "VehicleCamContourMaxRotation", LTNULL, 5.0f );	// RETAIL: 5.0f (0x1011b549)
	g_vtVehicleContourRate.Init( g_pLTClient, "VehicleContourRate", LTNULL, 0.3f );	// RETAIL: 0.3f (0x1011b565)

	m_VehicleModelOffsetMgr.Init();

	// Init world specific data members...

	InitWorldData();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::InitWorldData
//
//	PURPOSE:	Initialize our data members that are specific to the
//				current world.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::InitWorldData()
{
	m_dwControlFlags		= 0;
	m_dwLastControlFlags	= 0;

	m_fVehicleBaseMoveAccel = 0.0f;

	m_bVehicleStopped       = LTTRUE;
    m_bVehicleAtMaxSpeed    = LTFALSE;
    m_bVehicleTurning       = LTFALSE;
	m_nVehicleTurnDirection	= 0;

	m_ePPhysicsModel		= PPM_NORMAL;

	m_nCurGear				= VEHICLE_GEAR_0;
	m_nLastGear				= VEHICLE_GEAR_0;

	m_hVehicleAttachSocket	= INVALID_MODEL_SOCKET;

	m_bSetLastAngles		= false;

	m_vVehiclePYR.Init();
	m_vLastVehiclePYR.Init();
	m_vLastPlayerAngles.Init();
	m_vLastCameraAngles.Init();

	m_eSurface				= ST_UNKNOWN;
	m_eLastSurface			= ST_UNKNOWN;

	m_fYawDiff				= 0.0f;
	m_fYawDelta				= 0.0f;
	m_eMouseTurnDirection	= TD_CENTER;
	m_bKeyboardTurning		= LTFALSE;

	m_vHeadOffset.Init();
	m_fHandlebarRoll		= 0.0f;
	m_fHeadYaw				= 0.0f;

	m_nPlayerLureId			= 0;
	m_bResetLure			= false;
	m_hCurJumpVolume		= NULL;

	m_bHaveJumpVolumeVel	= LTFALSE;
	m_fJumpVolumeVel		= 0.0f;

	// RETAIL: 0x1011b5a0 -- Tron's light cycle state.
	m_nTronUnknown2134		= 0;
	m_bTronUnknown2158		= false;
	m_TronUnknownFX2138.ClearLink();
	m_TronUnknownFX213c.ClearLink();
	m_TronUnknownFX2140.ClearLink();
	m_TronUnknownFX2144.ClearLink();

	if (g_pPlayerMgr)
		g_pPlayerMgr->TronUnknownVirtualA8(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnEnterWorld
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnEnterWorld()
{
	InitWorldData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnExitWorld
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnExitWorld()
{
	// Turn off vehicle...

	// RETAIL: 0x1011b710 resets the model before switching.
	m_ePPhysicsModel = PPM_NORMAL;
	SetNormalPhysicsModel();

	// Clean up any level specific shiznit...

	TermLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::TermLevel
//
//	PURPOSE:	Terminate any level specific stuff
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::TermLevel()
{
	if (m_hVehicleModel)
	{
        g_pLTClient->RemoveObject(m_hVehicleModel);
		m_hVehicleModel = LTNULL;
	}

	KillAllVehicleSounds();

	if (m_hVehicleImpactSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleImpactSnd);
        m_hVehicleImpactSnd = LTNULL;
	}

	m_hVehicleAttachSocket = INVALID_MODEL_SOCKET;

	// Remove any player-view attachments...

	if (g_pPVAttachmentMgr)
		g_pPVAttachmentMgr->RemovePVAttachments();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateControlFlags
//
//	PURPOSE:	Update the control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateControlFlags()
{
	// Clear control flags...

	m_dwLastControlFlags = m_dwControlFlags;
	m_dwControlFlags = 0;

	if (!g_pMoveMgr->GetObject() ||
		!g_pInterfaceMgr->AllowCameraMovement()) return;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			UpdateSnowmobileControlFlags();
		break;

		case PPM_LURE :
			UpdateLureControlFlags();
		break;

		case PPM_LIGHTCYCLE :
			UpdateLightCycleControlFlags();
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSnowmobileControlFlags
//
//	PURPOSE:	Update the snowmobile physics model control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSnowmobileControlFlags()
{
	// Determine what commands are currently on...

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RUN) ^ (g_pMoveMgr->RunLock() != LTFALSE))
	{
		m_dwControlFlags |= BC_CFLG_RUN;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))
	{
		m_dwControlFlags |= BC_CFLG_JUMP;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))
	{
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_REVERSE))
	{
		m_dwControlFlags |= BC_CFLG_REVERSE;
		m_dwControlFlags &= ~BC_CFLG_FORWARD;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_LEFT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_RIGHT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_FIRING;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_ALT_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_ALT_FIRING;
	}

	// Check for strafe left and strafe right.
	if ((m_dwControlFlags & BC_CFLG_RIGHT) && (m_dwControlFlags & BC_CFLG_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if ((m_dwControlFlags & BC_CFLG_LEFT) && (m_dwControlFlags & BC_CFLG_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if ( (m_dwControlFlags & BC_CFLG_FORWARD) ||
		 (m_dwControlFlags & BC_CFLG_REVERSE) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_LEFT) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) ||
		 (m_dwControlFlags & BC_CFLG_JUMP))
	{
		m_dwControlFlags |= BC_CFLG_MOVING;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateLureControlFlags
//
//	PURPOSE:	Update the Lure control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateLureControlFlags()
{
	// Determine what commands are currently on...

	// RETAIL: 0x1011b99a: Tron's alt-fire key first
	if (g_pLTClient->IsCommandOn(COMMAND_ID_DEFEND))
	{
		m_dwControlFlags |= BC_CFLG_TRON_ALT_FIRE;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_FIRING;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_ALT_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_ALT_FIRING;
	}

	// Get the playerlurefx object.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		// It may not be on the client yet.  Keep polling for it.
		return;
	}

	// See if we need to reset the lure info.
	if( m_bResetLure )
	{
		// Reset the lure.
		pPlayerLureFX->Reset( );

		// Reset the lure shake timer.
		m_LureTimeToNextBump.Stop( );

		// Make sure the weapon is set correctly.  Need to keep calling this since
		// we poll for the playerlurefx object.
		EnableWeapon( pPlayerLureFX->GetAllowWeapon( ));

		// Don't need to reset anymore.
		m_bResetLure = false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateMotion
//
//	PURPOSE:	Update our motion
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateMotion()
{
	g_pMoveMgr->UpdateOnGround();

	m_eLastSurface = m_eSurface;
	m_eSurface	   = g_pMoveMgr->GetStandingOnSurface();

	// See if we moved over a new surface...

	if (m_eLastSurface != m_eSurface)
	{
		LTFLOAT fLastMult = GetSurfaceVelMult(m_eLastSurface);
		LTFLOAT fNewMult = GetSurfaceVelMult(m_eSurface);

		if (fLastMult < fNewMult)
		{
			// Speeding up...

			CameraDelta delta;
			delta.Pitch.fVar	= DEG2RAD(-0.5f);
			delta.Pitch.fTime1	= 0.25f;
			delta.Pitch.fTime2	= 0.5;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;
			m_VehicleModelOffsetMgr.AddDelta(delta);
		}
		else if (fNewMult < fLastMult)
		{
			// Slowing down...

			CameraDelta delta;
			delta.Pitch.fVar	= DEG2RAD(0.75f);
			delta.Pitch.fTime1	= 0.25f;
			delta.Pitch.fTime2	= 0.5;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;
			m_VehicleModelOffsetMgr.AddDelta(delta);
		}
	}

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			UpdateVehicleMotion();
		break;

		case PPM_LURE :
			UpdateLureMotion( );
			break;

		// RETAIL: 0x1011be18: the player manager takes the light cycle's
		// pending control flags (slot 0x68)
		case PPM_LIGHTCYCLE:
			g_pPlayerMgr->TronUnknownVirtual68(m_nTronUnknown2134);
			m_nTronUnknown2134 = 0;
			break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateLureMotion
//
//	PURPOSE:	Update our lure motion
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateLureMotion()
{
	HOBJECT hObj = g_pMoveMgr->GetObject();
	if( !hObj )
	{
		ASSERT( !"CVehicleMgr::UpdateLureMotion:  Invalid client object." );
		return;
	}

	LTVector vZero( 0.0f, 0.0f, 0.0f );

	// Don't allow any acceleration or velocity.
	g_pPhysicsLT->SetAcceleration( g_pMoveMgr->GetObject(), &vZero );
	g_pPhysicsLT->SetVelocity( g_pMoveMgr->GetObject(), &vZero );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleGear
//
//	PURPOSE:	Update the appropriate vehicle gear
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleGear()
{
 	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			UpdateSnowmobileGear();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSound
//
//	PURPOSE:	Update our sounds
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSound()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			UpdateVehicleSounds();
		break;

		case PPM_LURE :
		case PPM_NORMAL:
		default :
		break;
	}

	// See if the impact snd is done playing...

	if (m_hVehicleImpactSnd)
	{
        if (g_pLTClient->IsDone(m_hVehicleImpactSnd))
		{
            g_pLTClient->SoundMgr()->KillSound(m_hVehicleImpactSnd);
            m_hVehicleImpactSnd = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleMotion
//
//	PURPOSE:	Update our motion when on a vehicle
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleMotion()
{
	HOBJECT hObj = g_pMoveMgr->GetObject();

    LTFLOAT fTime = g_pLTClient->GetTime();

	LTBOOL bHeadInLiquid = g_pMoveMgr->IsHeadInLiquid();
    LTBOOL bInLiquid     = (bHeadInLiquid || g_pMoveMgr->IsBodyInLiquid());
    LTBOOL bFreeMovement = (g_pMoveMgr->IsFreeMovement() ||
		g_pMoveMgr->IsBodyOnLadder() || g_pPlayerMgr->IsSpectatorMode());


	// Normally we have gravity on, but the containers might turn it off.

	g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, g_pPlayerMgr->IsSpectatorMode() ? 0 : FLAG_GRAVITY, FLAG_GRAVITY);

	
	// Test to see if we are still within a JumpVolume...
	
	UpdateInJumpVolume();


	// Can't ride vehicles underwater, in spectator mode, or when dead...

	if (bHeadInLiquid || g_pPlayerMgr->IsSpectatorMode() || g_pPlayerMgr->IsPlayerDead())
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
		cMsg.Writeuint8(CP_PHYSICSMODEL);
		cMsg.Writeuint8(PPM_NORMAL);
	    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		// The server won't notify us, so change models...

		SetPhysicsModel(PPM_NORMAL);
		return;
	}

	// Zero out the acceleration to start with...

    LTVector zeroVec;
	zeroVec.Init();
	g_pPhysicsLT->SetAcceleration(hObj, &zeroVec);


	// Set our current rotation...

    LTRotation rRot;
	g_pPlayerMgr->GetPlayerRotation(rRot);
	g_pLTClient->SetObjectRotation(hObj, &rRot);

	// Update our motion based on containers we are in...

	UpdateContainerMotion();

    LTVector myVel = g_pMoveMgr->GetVelocity();
	
    LTVector vAccel(0, 0, 0);
	g_pPhysicsLT->GetAcceleration( hObj, &vAccel );

    LTVector moveDir = myVel;
	moveDir.Normalize();

    LTVector moveVel = myVel;
	moveVel.y = 0;

    LTBOOL bForward = LTFALSE;
	if ((m_dwControlFlags & BC_CFLG_FORWARD))
	{
        bForward = LTTRUE;
	}

	// This case happens when we jump if the user stops pressing forward
	// and we're in mid air...don't slow down!
	if (!bForward && !g_pMoveMgr->CanDoFootstep())
	{
		bForward = LTTRUE;
	}

	LTBOOL bBrake = LTFALSE;
	if ((m_dwControlFlags & BC_CFLG_REVERSE))
	{
		// Only brake if we are on the ground...
        bBrake = g_pMoveMgr->CanDoFootstep();
	}

    LTBOOL bReverse = !bForward;

	// Determine the acceleration to use...

    LTFLOAT fMaxAccel  = GetMaxAccelMag(GetSurfaceVelMult(m_eSurface));
    LTFLOAT fMaxVel    = GetMaxVelMag(GetSurfaceVelMult(m_eSurface));

	// If we have a velocity from a JumpVolume use that as the max...
	
	if( m_bHaveJumpVolumeVel )
	{
		// [KLS 9/1/02] Changed how this calculations works...
		// See if it is time to use our normal velocity...(on the 
		// ground and not in a jump volume)...

		if (g_pMoveMgr->CanDoFootstep() && !m_hCurJumpVolume)
		{
			m_fJumpVolumeVel = 0.0f;
			m_bHaveJumpVolumeVel = LTFALSE;
		}
		else  // Use the jump volume vel...
		{
			fMaxVel = m_fJumpVolumeVel;
		}
	}

	if (bForward)
	{
		LTFLOAT fAccelTime = GetVehicleAccelTime();

		if (m_fAccelStart < 0.0f)
		{
			m_fAccelStart = fTime - (m_fVehicleBaseMoveAccel * fAccelTime / fMaxAccel);
		}

		m_fVehicleBaseMoveAccel = fMaxAccel * (fTime - m_fAccelStart) / fAccelTime;
	}
	else
	{
		m_fAccelStart = -1.0f;
	}

	if (m_bVehicleTurning && !bBrake)
	{
		if (m_fVehicleBaseMoveAccel < GetVehicleMinTurnAccel())
		{
			m_fVehicleBaseMoveAccel = GetVehicleMinTurnAccel();
			bReverse = LTFALSE;
		}
	}

	m_fVehicleBaseMoveAccel = m_fVehicleBaseMoveAccel > fMaxAccel ? fMaxAccel : m_fVehicleBaseMoveAccel;


	// Limit velocity...

	if (g_pMoveMgr->IsOnGround() && !g_pPlayerMgr->IsSpectatorMode())
	{
		float fCurLen = moveVel.Mag();
		if (fCurLen > fMaxVel)
		{
			myVel *= (fMaxVel/fCurLen);

			g_pPhysicsLT->SetVelocity(hObj, &myVel);
		}
	}
	else if (moveVel.Mag() > fMaxVel)
	{
		// Don't cap velocity in the y direction...

		moveVel.Normalize();
		moveVel *= fMaxVel;

		myVel.x = moveVel.x;
		myVel.z = moveVel.z;

		g_pPhysicsLT->SetVelocity(hObj, &myVel);
	}

	g_pLTClient->GetObjectRotation(hObj, &rRot);

    LTVector vForward = rRot.Forward();


	LTFLOAT fAccelMulti = g_pMoveMgr->GetMoveAccelMultiplier();
	fAccelMulti = (fAccelMulti > 1.0f ? 1.0f : fAccelMulti);

    LTFLOAT fMoveAccel = (m_fVehicleBaseMoveAccel * fAccelMulti);

	if (!bInLiquid && !bFreeMovement)  // Can only move forward in x and z directions
	{
		vForward.y = 0.0;
		vForward.Normalize();
	}


	// If we aren't dead we can drive around

	if (!g_pPlayerMgr->IsPlayerDead())
	{
		vAccel += (vForward * fMoveAccel);

		if (bReverse || bBrake)
		{
			// Don't let us go backwards, slow us down...

            LTVector vAccelDir = vAccel;
			vAccelDir.Normalize();
            LTFLOAT fAccelMag = vAccel.Mag();

			// Determine the acceleration to use...

            LTFLOAT fMaxDecel  = GetVehicleMaxDecel();
            LTFLOAT fDecelTime = GetVehicleDecelTime();

			if (bBrake)
			{
				fMaxDecel  = GetVehicleMaxBrake();
				fDecelTime = GetVehicleBrakeTime();
			}

			if (m_fDecelStart < 0.0f)
			{
				m_fDecelStart = fTime;
			}

            LTFLOAT fAccelAdjust = fMaxDecel * (fTime - m_fDecelStart) / fDecelTime;
			fAccelAdjust = fAccelAdjust > fAccelMag ? fAccelMag : fAccelAdjust;

			vAccel -= vAccelDir * fAccelAdjust;

			if (vAccel.Mag() < 10.0f)
			{
				vAccel.Init();
			}

			m_fVehicleBaseMoveAccel = vAccel.Mag();
		}
		else
		{
			m_fDecelStart = -1.0f;
		}
	}

	// Cap the acceleration...

	if (vAccel.Mag() > fMoveAccel)
	{
		vAccel.Normalize();
		vAccel *= fMoveAccel;
	}

	g_pPhysicsLT->SetAcceleration(hObj, &vAccel);
	
	// Add any container currents to my velocity...

    LTVector vel = g_pMoveMgr->GetVelocity();
	vel += g_pMoveMgr->GetTotalCurrent();

	g_pPhysicsLT->SetVelocity(hObj, &vel);


	// If we're dead, we can't move...

	if (g_pPlayerMgr->IsPlayerDead() || !g_pPlayerMgr->IsPlayerMovementAllowed())
	{
		g_pPhysicsLT->SetAcceleration(hObj, &zeroVec);
		g_pPhysicsLT->SetVelocity(hObj, &zeroVec);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleAccelTime
//
//	PURPOSE:	Get the current vehicle's accerlation time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleAccelTime()
{
	LTFLOAT fTime = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			fTime = g_vtSnowmobileAccelTime.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fTime;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleMinTurnAccel
//
//	PURPOSE:	Get the current vehicle's minimum turn acceleration
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleMinTurnAccel()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileMinTurnAccel.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleMaxDecel
//
//	PURPOSE:	Get the current vehicle's maximum deceleration
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleMaxDecel()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileMaxDecel.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleDecelTime
//
//	PURPOSE:	Get the current vehicle's deceleration time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleDecelTime()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileDecelTime.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleMaxBrake
//
//	PURPOSE:	Get the current vehicle's max break value
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleMaxBrake()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileMaxBrake.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleBrakeTime
//
//	PURPOSE:	Get the current vehicle's break time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleBrakeTime()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileBrakeTime.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateFriction
//
//	PURPOSE:	Update fricton
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateFriction()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			UpdateVehicleFriction(SNOWMOBILE_SLIDE_TO_STOP_TIME);
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleFriction
//
//	PURPOSE:	Update vehicle fricton
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleFriction(LTFLOAT fSlideToStopTime)
{
	// Dampen our velocity so we don't slide around too much...

    uint32 dwTestFlags = BC_CFLG_MOVING | BC_CFLG_JUMP;

    LTVector vZero(0, 0, 0);

	if ( !(m_dwControlFlags & dwTestFlags) && g_pMoveMgr->IsOnGround() &&
		g_pMoveMgr->GetTotalCurrent() == vZero)
	{
        LTVector vCurVel = g_pMoveMgr->GetVelocity();

        LTFLOAT fYVal = vCurVel.y;
		vCurVel.y = 0.0f;

        LTVector vVel;

		if (vCurVel.Mag() > 5.0f)
		{
            LTVector vDir = vCurVel;
			vDir.Normalize();

			if (fSlideToStopTime < 0.0f) fSlideToStopTime = 0.1f;

            LTFLOAT fAdjust = g_pGameClientShell->GetFrameTime() * (GetMaxVelMag()/fSlideToStopTime);

			vVel = (vDir * fAdjust);

			if (vVel.MagSqr() < vCurVel.MagSqr())
			{
				vVel = vCurVel - vVel;
			}
			else
			{
				vVel.Init();
			}

			vVel.y = fYVal;
		}
		else
		{
			vVel.Init();
		}

		g_pMoveMgr->SetVelocity(vVel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::IsVehiclePhysics
//
//	PURPOSE:	Are we doing vehicle physics
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::IsVehiclePhysics()
{
	return IsVehicleModel(m_ePPhysicsModel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CanDismount
//
//	PURPOSE:	Are we allowed do get off the vehicle
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::CanDismount()
{
	if( !IsVehiclePhysics() )
		return LTFALSE;

	return (m_ePPhysicsModel != PPM_LURE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::EnableWeapon
//
//	PURPOSE:	Enable/disables the weapon.
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron keeps this in a file static (0x10271b14), not a member.
static LTBOOL s_bHolsteredWeapon = LTFALSE;

void CVehicleMgr::EnableWeapon( bool bEnable )
{
	// Get our weapon.
	CClientWeaponMgr *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	IClientWeaponBase* pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
	// RETAIL: 0x1011cc80 has no heavy object check.
	if( !pClientWeapon )
		return;

	// Enable the weapon...
	if( bEnable )
	{
		// Check if we have a holstered weapon.
		if( s_bHolsteredWeapon )
		{
			g_pPlayerMgr->ToggleHolster( false );
			s_bHolsteredWeapon = LTFALSE;
		}

		// RETAIL: Tron enables the weapons after unholstering.
		pClientWeaponMgr->EnableWeapons();
	}
	// Disable the weapon.
	else
	{
		if( !s_bHolsteredWeapon )
		{
			// Can't holster melee weapon. (why i don't know BEP).
			if (!pClientWeapon->IsMeleeWeapon())
			{
				g_pPlayerMgr->ToggleHolster( false );
				s_bHolsteredWeapon = LTTRUE;
			}
		}

		pClientWeaponMgr->DisableWeapons();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::PreSetPhysicsModel
//
//	PURPOSE:	Do any clean up from old model
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::PreSetPhysicsModel(PlayerPhysicsModel eModel)
{
	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

	HOBJECT hObj = g_pMoveMgr->GetObject();

	LTBOOL bRet = LTTRUE;

	bool bTouchNotify;

	if (IsVehicleModel(eModel))
	{
		// Check if we should disable the weapon.
 		switch (eModel)
		{
			case PPM_SNOWMOBILE :
				EnableWeapon( false );
			break;

			// RETAIL: 0x1011cd6a: getting on a light cycle
			case PPM_LIGHTCYCLE :
			{
				if (m_ePPhysicsModel != PPM_LIGHTCYCLE)
				{
					EnableWeapon( false );
					g_pPlayerMgr->TronUnknownVirtual50();
					m_TronUnknownTimer2160.Stop();
				}
			}
			break;

			case PPM_LURE :
			case PPM_NORMAL :
			default :
			break;
		}

		// Needed for collision physics...

		bTouchNotify = true;
		g_pPhysicsLT->SetForceIgnoreLimit(hObj, 0.0f);
	}
	else
	{
		bTouchNotify = false;
		g_pPhysicsLT->SetForceIgnoreLimit(hObj, 1000.0f);

		// Turn off the vehicle...

		KillAllVehicleSounds();

 		switch (m_ePPhysicsModel)
		{
			// RETAIL: 0x1011cdef: getting off a light cycle stops it, then
			// gives the weapon back as the lure does (the cases' code is in
			// this order)
			case PPM_LIGHTCYCLE :
			{
				g_pPlayerMgr->TronUnknownVirtual54();

				LTVector vZero(0.0f, 0.0f, 0.0f);
				g_pPhysicsLT->SetVelocity(hObj, &vZero);
			}
			// fall through

			case PPM_LURE :
			{
				// Enable the weapon.
				EnableWeapon( true );
			}
			break;

			case PPM_SNOWMOBILE :
			{
				g_pClientSoundMgr->PlaySoundLocal("Snd\\Vehicle\\Snowmobile\\turnoff.wav", SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_CLIENT);

				PlayVehicleAni("Deselect");

				// Can't move until deselect is done...
				g_pMoveMgr->AllowMovement(LTFALSE);

				// Wait to change modes...
				bRet = LTFALSE;
			}
			break;

			default :
			case PPM_NORMAL :
			break;
		}

	}

	g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, bTouchNotify ? FLAG_TOUCH_NOTIFY : 0, FLAG_TOUCH_NOTIFY);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetPhysicsModel
//
//	PURPOSE:	Set the physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPhysicsModel(PlayerPhysicsModel eModel, LTBOOL bDoPreSet)
{
	// RETAIL: 0x1011ceab: cleared first
	m_bTronUnknown2170 = false;

	if (m_ePPhysicsModel == eModel) return;

	// Do clean up...

	if (bDoPreSet && !PreSetPhysicsModel(eModel)) return;

	m_ePPhysicsModel = eModel;

	// RETAIL: 0x1011ced3
	g_pPlayerMgr->SetTronUnknown1D5(false);

 	switch (eModel)
	{
		case PPM_SNOWMOBILE :
		{
			SetSnowmobilePhysicsModel();

			if (g_vtSnowmobileInfoTrack.GetFloat())
			{
				g_pLTClient->CPrint("Snowmobile Physics Mode: ON");
			}
		}
		break;

		case PPM_LIGHTCYCLE:
		{
			SetLightCyclePhysicsModel();
			m_nTronUnknown2134 = 0;
		}
		break;

		case PPM_LURE :
		{
			SetPlayerLurePhysicsModel( );
		}
		break;

		default :
		case PPM_NORMAL :
		{
			SetNormalPhysicsModel();

			if (g_vtSnowmobileInfoTrack.GetFloat())
			{
				g_pLTClient->CPrint("Normal Physics Mode: ON");
			}
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::PlayVehicleAni
//
//	PURPOSE:	Change the vehicle's animation...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::PlayVehicleAni(char* pAniName, LTBOOL bReset, LTBOOL bLoop)
{
	if (!m_hVehicleModel || !pAniName) return;

	LTBOOL bIsDone = LTFALSE;
	if (!bReset && IsCurVehicleAni(pAniName, bIsDone))
	{
		return;
	}

	uint32 dwAnim = g_pLTClient->GetAnimIndex(m_hVehicleModel, pAniName);

    g_pLTClient->SetModelLooping(m_hVehicleModel, bLoop != LTFALSE);
	g_pLTClient->SetModelAnimation(m_hVehicleModel, dwAnim);

	if (g_vtSnowmobileInfoTrack.GetFloat() == SNOW_TRACK_ANIMATION)
	{
       g_pLTClient->CPrint("Playing Ani(%s)", pAniName);
	}

	if (bReset)
	{
		g_pLTClient->ResetModelAnimation(m_hVehicleModel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::IsCurVehicleAni
//
//	PURPOSE:	See if this is the current vehicle animation
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::IsCurVehicleAni(char* pAniName, LTBOOL & bIsDone)
{
	if (!m_hVehicleModel || !pAniName) return LTFALSE;

	// See if the current ani is done...

	uint32 dwState = g_pLTClient->GetModelPlaybackState(m_hVehicleModel);
 	bIsDone = (dwState & MS_PLAYDONE);

	uint32 dwTestAni = g_pLTClient->GetAnimIndex(m_hVehicleModel, pAniName);
	uint32 dwAni	 = g_pLTClient->GetModelAnimation(m_hVehicleModel);

	return (dwTestAni == dwAni);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetSnowmobilePhysicsModel
//
//	PURPOSE:	Set the snowmobile physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetSnowmobilePhysicsModel()
{
	if (!m_hVehicleStartSnd)
	{
        uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		m_hVehicleStartSnd = g_pClientSoundMgr->PlaySoundLocal("Snd\\Vehicle\\Snowmobile\\startup.wav", SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
	}

	CreateVehicleModel();

	m_vVehicleOffset.x = g_vtSnowmobileOffsetX.GetFloat();
	m_vVehicleOffset.y = g_vtSnowmobileOffsetY.GetFloat();
	m_vVehicleOffset.z = g_vtSnowmobileOffsetZ.GetFloat();

	// Reset acceleration so we don't start off moving...

	m_fVehicleBaseMoveAccel = 0.0f;

 	ShowVehicleModel(LTTRUE);

	PlayVehicleAni("Select");

	// Can't move until select ani is done...

	g_pMoveMgr->AllowMovement(LTFALSE);

	m_bSetLastAngles = false;
	m_vLastPlayerAngles.Init();
	m_vLastCameraAngles.Init();
	m_vLastVehiclePYR.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetPlayerLurePhysicsModel
//
//	PURPOSE:	Set the playerlure physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPlayerLurePhysicsModel()
{
	// Get the playerlurefx for info.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		return;
	}

	// Do special setup for bicycle.
	if( pPlayerLureFX->GetBicycle( ))
	{
		CreateVehicleModel();
	 	ShowVehicleModel(LTTRUE);

		if (!m_hVehicleAccelSnd)
		{
			KillAllVehicleSounds();

			char* pSound = GetAccelSnd();

			if (pSound)
			{
				uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT | PLAYSOUND_LOOP;
				m_hVehicleAccelSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}
	}

	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetLightCyclePhysicsModel
//
//	PURPOSE:	Set the light cycle physics model: the cycle's model, its
//				intro fx and sound, and the countdown.  RETAIL: 0x1011d2a0
//				(Tron's; the name is ours)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetLightCyclePhysicsModel()
{
	CreateVehicleModel();
	ShowVehicleModel(LTTRUE);

	if (m_TronUnknownFX2138.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX2138);
		m_TronUnknownFX2138.ClearLink();
	}

	char szFX[512];
	char szSound[512];
	g_pPlayerMgr->TronUnknownVirtual70(g_pMoveMgr->GetObject(), szFX, szSound);

	CLIENTFX_CREATESTRUCT fxInit(szFX, FXFLAG_NOSMOOTHSHUTDOWN, g_pMoveMgr->GetObject());
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &fxInit.m_vPos);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX2138, fxInit, LTTRUE);

	m_bTronUnknown2158 = true;
	m_nTronUnknown215c = 0;

	g_pClientSoundMgr->PlaySoundLocal(szSound, SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_CLIENT);

	g_pInterfaceMgr->TronUnknownVirtual98(true, true);

	// RETAIL: the countdown message while LCCountdown is 1 (or not set),
	// except in a light cycle circuit; the player waits for the start
	// either way
	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCCountdown");
	if (!hVar || g_pLTClient->GetVarValueFloat(hVar) == 1.0f)
	{
		if (GetGameType() != eGameTypeSingleLightCycleCircuit)
		{
			g_pInterfaceMgr->SetTronUnknownMessage(4.0f, 0, LTNULL, true);
		}
	}

	g_pMoveMgr->AllowMovement(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetNormalPhysicsModel
//
//	PURPOSE:	Set the normal physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetNormalPhysicsModel()
{
	if (m_hVehicleModel)
	{
        g_pLTClient->RemoveObject(m_hVehicleModel);
		m_hVehicleModel = LTNULL;
	}

	// RETAIL: 0x1011d4ac: the light cycle's fx go too
	ShutdownLightCycleFX();

	KillAllVehicleSounds();

	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

	// RETAIL: 0x1011d4e3: the player can move again, and the camera goes
	// back to first person
	g_pMoveMgr->AllowMovement(LTTRUE);
	g_pInterfaceMgr->TronUnknownFirstPersonCamera(true);

	// Remove player-view attachments

	g_pPVAttachmentMgr->RemovePVAttachments();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CreateVehicleModel
//
//	PURPOSE:	Create the player-view vehicle model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CreateVehicleModel()
{
	// RETAIL: 0x1011d510
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	// NEED TO MOVE THESE INTO A BUTE FILE!!!
	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
		{

			createStruct.m_ObjectType = OT_MODEL;
			createStruct.m_Flags	  = FLAG_VISIBLE | FLAG_REALLYCLOSE;
			createStruct.m_Flags2	  = FLAG2_FORCETRANSLUCENT | FLAG2_DYNAMICDIRLIGHT;

			// NEED TO MOVE THESE INTO A BUTE FILE!!!

			SAFE_STRCPY(createStruct.m_Filename, "Guns\\Models_PV\\snowmobile.ltb");
			SAFE_STRCPY(createStruct.m_SkinNames[1], "Guns\\Skins_PV\\snowmobile.dtx");
			SAFE_STRCPY(createStruct.m_SkinNames[2], "Guns\\Skins_PV\\windshield.dtx");
			SAFE_STRCPY(createStruct.m_RenderStyleNames[0], "RS\\default.ltb");
			SAFE_STRCPY(createStruct.m_RenderStyleNames[1], "RS\\glass.ltb");

			CCharacterFX* pCharFX = g_pMoveMgr->GetCharacterFX();

			// RETAIL: 0x1011d7e0: the multiplayer hands unless the player
			// uses a player model
			if (pCharFX && (!IsMultiplayerGame() || g_pMoveMgr->IsPlayerModel()))
			{
				SAFE_STRCPY(createStruct.m_SkinNames[0], g_pModelButeMgr->GetHandsSkinFilename(pCharFX->GetModelId()));
			}
			else
			{
				SAFE_STRCPY(createStruct.m_SkinNames[0], "guns\\skins_pv\\MultiHands_pv.dtx");
			}
		}
		break;

		case PPM_LURE:
		{
			createStruct.m_ObjectType = OT_MODEL;
			createStruct.m_Flags	  = FLAG_VISIBLE;
			createStruct.m_Flags2	  = FLAG2_DYNAMICDIRLIGHT;

			SAFE_STRCPY(createStruct.m_Filename, "chars\\Models\\armtricycle.ltb");
			SAFE_STRCPY(createStruct.m_SkinNames[0], "chars\\skins\\armstrong.dtx");
			SAFE_STRCPY(createStruct.m_SkinNames[1], "chars\\skins\\armstronghead.dtx");
			SAFE_STRCPY(createStruct.m_RenderStyleNames[0], "RS\\default.ltb");
		}
		break;

		case PPM_LIGHTCYCLE:
		{
			// RETAIL: 0x1011d682: the player manager fills in the player's
			// light cycle (its arguments are swapped in our declaration)
			if (!g_pPlayerMgr->TronUnknownVirtual9C((void*)3, (int)&createStruct))
				return;
		}
		break;

		default :
			return;
		break;
	}

	// Remove any old vehicle model.
	if( m_hVehicleModel )
	{
		g_pLTClient->RemoveObject( m_hVehicleModel );
		m_hVehicleModel = NULL;
	}

	// RETAIL: 0x1011d84c
	if (!m_hVehicleModel)
	{
		createStruct.m_ObjectType = OT_MODEL;

		switch (m_ePPhysicsModel)
		{
			case PPM_SNOWMOBILE:
			{
				createStruct.m_Flags	= FLAG_VISIBLE | FLAG_REALLYCLOSE;
				createStruct.m_Flags2	= FLAG2_DYNAMICDIRLIGHT;
			}
			break;

			case PPM_LIGHTCYCLE:
			{
				createStruct.m_Flags	= FLAG_VISIBLE;
			}
			break;
		}

		m_hVehicleModel = g_pLTClient->CreateObject(&createStruct);
		if (!m_hVehicleModel) return;
	}

	g_pCommonLT->SetObjectFilenames(m_hVehicleModel, &createStruct);

	// Set up node control...
	// Do this AFTER we set the filename of the model since the node control is now per node.
	// First remove any we may have previously set otherwise the matrix will be applied multiple times.

	g_pModelLT->RemoveNodeControlFn( m_hVehicleModel, ModelNodeControlFn, this );
	g_pModelLT->AddNodeControlFn(m_hVehicleModel, ModelNodeControlFn, this);
    

	// Map all the nodes in our skeleton in the bute file to the nodes in the actual .ltb model file

	m_VehicleNode1.hNode = LTNULL;
	m_VehicleNode2.hNode = LTNULL;
	m_VehicleNode3.hNode = LTNULL;

	// Do post creation stuff.
	switch( m_ePPhysicsModel )
	{
		case PPM_SNOWMOBILE:
		{
			g_pLTClient->SetModelLooping(m_hVehicleModel, LTFALSE);


			int iNode = 0;
			HMODELNODE hCurNode = INVALID_MODEL_NODE;
			while (g_pLTClient->GetModelLT()->GetNextNode(m_hVehicleModel, hCurNode, hCurNode) == LT_OK)
			{
				char szName[64];
				*szName = 0;
				g_pLTClient->GetModelLT()->GetNodeName(m_hVehicleModel, hCurNode, szName, sizeof(szName));

				if (stricmp(szName, "Handlebars1") == 0)
				{
					m_VehicleNode1.hNode = hCurNode;
				}
				else if( stricmp( szName, "Speedometer1" ) == 0 )
				{
					m_VehicleNode2.hNode = hCurNode;
				}
			}
			
			// Create any player-view attachments 

			g_pPVAttachmentMgr->CreatePVAttachments( m_hVehicleModel );
		}
		break;

		case PPM_LURE:
		{
			g_pLTClient->SetModelLooping( m_hVehicleModel, LTFALSE );
		}
		break;

		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateModels
//
//	PURPOSE:	Update the player-view models
//
// ----------------------------------------------------------------------- //

void ExitLightCycleCallBack(LTBOOL bReturn, void *pData);

void CVehicleMgr::UpdateModels()
{
	// RETAIL: 0x1011da30: Tron's light cycle.  The model shows once its
	// intro fx is done; then the server is told the cycle can start, and
	// the cycle's fx and sounds begin (or, at the end of a ride, the exit
	// box comes up).  While riding, the model follows the player.
	if (m_ePPhysicsModel == PPM_LIGHTCYCLE)
	{
		if (m_bTronUnknown2158)
		{
			if (m_TronUnknownFX2138.IsValid() && !m_TronUnknownFX2138.GetInstance()->IsDone())
			{
				ShowVehicleModel(LTFALSE);
				return;
			}

			if (m_TronUnknownFX2138.IsValid())
			{
				g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX2138);
			}
			m_TronUnknownFX2138.ClearLink();
			m_bTronUnknown2158 = false;

			if (m_nTronUnknown215c == 0)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
				cMsg.Writeuint8(LCI_START_UPDATING);
				cMsg.WriteObject(g_pMoveMgr->GetServerObject());
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

				char szFX[512];
				if (g_pPlayerMgr->TronUnknownVirtual78(g_pMoveMgr->GetObject(), szFX))
				{
					CLIENTFX_CREATESTRUCT fxInit(szFX, FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, m_hVehicleModel);
					g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &fxInit.m_vPos);
					g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX2138, fxInit, LTTRUE);
				}

				g_pInterfaceMgr->ClearTronUnknownMessage();
				g_pMoveMgr->AllowMovement(LTTRUE);

				char szAltCruiseSnd[128];
				char szCruiseSnd[128];
				char szTurnSnd[128];
				g_pPlayerMgr->TronUnknownVirtual7C(g_pMoveMgr->GetObject(), szCruiseSnd, szTurnSnd, szAltCruiseSnd);

				if (!m_hVehicleCruiseSnd)
				{
					m_hVehicleCruiseSnd = g_pClientSoundMgr->PlaySoundLocal(szCruiseSnd, SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT);
				}
				if (!m_hTronUnknownSnd2c)
				{
					m_hTronUnknownSnd2c = g_pClientSoundMgr->PlaySoundLocal(szAltCruiseSnd, SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT);
				}

				ShowVehicleModel(LTTRUE);
			}
			else if (m_nTronUnknown215c == 1)
			{
				if (!IsMultiplayerGame())
				{
					if (GetGameType() == eGameTypeSingleLightCycleCircuit)
					{
						if (!g_pPlayerMgr->TronUnknownVirtual88())
						{
							g_pGameClientShell->CSPrint(LoadTempString(0x3fd));
							g_pPlayerMgr->StartDeathFX();
						}
						else
						{
							MBCreate mb;
							mb.eType = LTMB_YESNO;
							mb.pFn = ExitLightCycleCallBack;
							mb.pData = this;
							g_pInterfaceMgr->ShowMessageBox(0x397, &mb);
						}
					}
					else
					{
						MBCreate mb;
						mb.eType = LTMB_YESNO;
						mb.pFn = ExitLightCycleCallBack;
						mb.pData = this;
						g_pInterfaceMgr->ShowMessageBox(0x396, &mb);
					}
				}
			}
		}
		else
		{
			LTVector vPos;
			LTRotation rRot;
			g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPos);
			g_pLTClient->GetObjectRotation(g_pMoveMgr->GetObject(), &rRot);
			g_pLTClient->SetObjectPosAndRotation(m_hVehicleModel, &vPos, &rRot);

			ShowVehicleModel(!g_pPlayerMgr->GetTronUnknown1D5());

			if (m_TronUnknownFX2148.IsValid())
			{
				LTVector vFXPos = vPos + m_vTronUnknownFXOffset;
				m_TronUnknownFX2148.GetInstance()->SetPos(vFXPos, LTVector(0.0f, 0.0f, 0.0f));
			}
		}
	}
	else if (m_ePPhysicsModel != PPM_NORMAL)
	{
		UpdateVehicleModel();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitLightCycleCallBack
//
//	PURPOSE:	The light cycle exit message box's callback: ask the
//				server to end the light cycle, or, when the player can't
//				stay, fail the mission.  RETAIL: 0x1011dee0 (Tron's; the
//				name is ours. Retail's 0x1011da30 shows the box)
//
// ----------------------------------------------------------------------- //

void ExitLightCycleCallBack(LTBOOL bReturn, void *pData)
{
	if (bReturn)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_TRON_EXIT_REQUEST);
		cMsg.Writebool(true);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
	else
	{
		bool bCanStay = false;
		if (GetGameType() == eGameTypeSingle)
		{
			switch (g_pMissionMgr->GetCurrentMission())
			{
				case 4:
					if (g_pMissionMgr->GetCurrentLevel() == 2) bCanStay = true;
					break;
				case 5:
					if (g_pMissionMgr->GetCurrentLevel() == 2) bCanStay = true;
					break;
			}
		}

		if (!g_pPlayerMgr->TronUnknownVirtual80() && !bCanStay)
		{
			g_pGameClientShell->CSPrint(LoadTempString(1011));
			g_pPlayerMgr->StartDeathFX();
		}
		else
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_TRON_EXIT_REQUEST);
			cMsg.Writebool(false);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}
	}

	g_pPlayerMgr->TronUnknownVirtual98(bReturn);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleModel
//
//	PURPOSE:	Update the player-view vehicle model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleModel()
{
	if (!m_hVehicleModel || (m_ePPhysicsModel != PPM_SNOWMOBILE) ) return;

	//See if we are paused, if so we need to pause the animation...

	bool bPaused = g_pGameClientShell->IsServerPaused();
	g_pLTClient->Common()->SetObjectFlags(m_hVehicleModel, OFT_Flags, bPaused ? FLAG_PAUSED : 0, FLAG_PAUSED);

	if (bPaused) return;

	// Update our camera offset mgr...

	m_VehicleModelOffsetMgr.Update();

	uint32 dwVehicleFlags;
	g_pCommonLT->GetObjectFlags(m_hVehicleModel, OFT_Flags, dwVehicleFlags);

	// See if we're just waiting for he deselect animation to finish...

	LTBOOL bIsDone = LTFALSE;
	if (IsCurVehicleAni("Deselect", bIsDone))
	{
		if (bIsDone)
		{
			// Okay to move now...
			g_pMoveMgr->AllowMovement(LTTRUE);

			ShowVehicleModel(LTFALSE);
			SetPhysicsModel(PPM_NORMAL, LTFALSE);

			// Enable the weapon...
			EnableWeapon( true );

			// Tell the server we want off...NOTE: In multiplayer we may want
			// to call this in PreSetPhysicsModel()
			g_pPlayerMgr->DoActivate();
			return;
		}
	}

	// If we're in specatator mode, or 3rd-person don't show the player-view
	// vehicle model...

	if (g_pPlayerMgr->IsSpectatorMode() || !g_pPlayerMgr->IsFirstPerson())
	{
		ShowVehicleModel(LTFALSE);
		return;
	}

	ShowVehicleModel(LTTRUE);

    LTRotation rRot;

    LTVector vPitchYawRollDelta = m_VehicleModelOffsetMgr.GetPitchYawRollDelta();

	m_vVehiclePYR.x += vPitchYawRollDelta.x;
	m_vVehiclePYR.y	+= vPitchYawRollDelta.y + m_fHeadYaw;
	m_vVehiclePYR.z += vPitchYawRollDelta.z;

	rRot = LTRotation(m_vVehiclePYR.x, m_vVehiclePYR.y, m_vVehiclePYR.z);

	LTVector vNewPos = m_VehicleModelOffsetMgr.GetPosDelta();

	vNewPos += rRot.Right() * (m_vVehicleOffset.x + m_vHeadOffset.x);
	vNewPos += rRot.Up() * (m_vVehicleOffset.y + m_vHeadOffset.y);
	vNewPos += rRot.Forward() * (m_vVehicleOffset.z + m_vHeadOffset.z);

	g_pLTClient->SetObjectPosAndRotation(m_hVehicleModel, &vNewPos, &rRot);

	// Reset the vehicle angles...

	m_vVehiclePYR.Init();

	// Give player-view attachments an update...

	g_pPVAttachmentMgr->UpdatePVAttachments();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::ShowVehicleModel
//
//	PURPOSE:	Show/Hide the vehicle model (and attachments)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::ShowVehicleModel(LTBOOL bShow)
{
	if (!m_hVehicleModel) return;

	if (bShow)
	{
		g_pCommonLT->SetObjectFlags(m_hVehicleModel, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	}
	else
	{
		g_pCommonLT->SetObjectFlags(m_hVehicleModel, OFT_Flags, 0, FLAG_VISIBLE);
	}

	g_pPVAttachmentMgr->ShowPVAttachments( !!(bShow) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxVelMag
//
//	PURPOSE:	Get the max velocity for our current mode
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetMaxVelMag(LTFLOAT fMult) const
{
    LTFLOAT fMaxVel = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
		{
			fMaxVel = g_vtSnowmobileVel.GetFloat();
		}
		break;

		case PPM_LIGHTCYCLE:
		{
			fMaxVel = g_vtLightCycleVel.GetFloat();
		}
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	fMaxVel *= fMult;

	LTFLOAT fMoveMulti = g_pMoveMgr->GetMoveMultiplier();
	fMoveMulti = (fMoveMulti > 1.0f ? 1.0f : fMoveMulti);

	fMaxVel *= (g_pMoveMgr->IsBodyInLiquid() ? fMoveMulti/2.0f : fMoveMulti);

	return fMaxVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxAccelMag
//
//	PURPOSE:	Get the max acceleration for our current mode
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetMaxAccelMag(LTFLOAT fMult) const
{
    LTFLOAT fMaxVel = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
		{
			fMaxVel = g_vtSnowmobileAccel.GetFloat();
		}
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	fMaxVel *= fMult;

	return fMaxVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetSurfaceVelMult
//
//	PURPOSE:	Get the velocity multiplier for the current surface
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetSurfaceVelMult(SurfaceType eSurf) const
{
	LTFLOAT fVelMultiplier = 1.0f;

	SURFACE* pSurface = g_pSurfaceMgr->GetSurface(eSurf);

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
		{
			if (pSurface)
			{
				fVelMultiplier = pSurface->fSnowVelMult;
			}
		}
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	return fVelMultiplier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleSounds
//
//	PURPOSE:	Update sounds while on the vehicle
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleSounds()
{
	// Update what gear the vehicle is in...

	UpdateVehicleGear();

	// See if the start snd is done playing...

	if (m_hVehicleStartSnd)
	{
        if (g_pLTClient->IsDone(m_hVehicleStartSnd))
		{
            g_pLTClient->SoundMgr()->KillSound(m_hVehicleStartSnd);
            m_hVehicleStartSnd = LTNULL;
		}
	}


	// See if we are going forward or backward...

    LTBOOL bForward = LTFALSE;
	if ((m_dwControlFlags & BC_CFLG_FORWARD))
	{
        bForward = LTTRUE;
	}

	LTBOOL bBrake = LTFALSE;
	if ((m_dwControlFlags & BC_CFLG_REVERSE))
	{
        bBrake = g_pMoveMgr->CanDoFootstep();
	}

	LTBOOL bIsDone = LTFALSE;
	LTBOOL bPlayAni = LTTRUE;

	// RETAIL: 0x1011e802: no sounds once the light cycle has derezzed
	if (m_bTronUnknown2158)
	{
		return;
	}

	if (IsCurVehicleAni("Select", bIsDone))
	{
		if (bIsDone)
		{
			// Okay, we can move now...
			g_pMoveMgr->AllowMovement(LTTRUE);
		}
		else // Can't do anything until this is done...
		{
			return;
		}
	}
	else if (IsCurVehicleAni("Deselect", bIsDone))
	{
		return;
	}

	bIsDone = LTFALSE;

	// Play the appropriate sound, based on acceleration...

	if (!bForward)
	{
		if (m_hVehicleAccelSnd)
		{
            g_pLTClient->SoundMgr()->KillSound(m_hVehicleAccelSnd);
            m_hVehicleAccelSnd = LTNULL;
		}

		// Play braking ani if necessary...

		if (bBrake && !m_bVehicleStopped)
		{
			PlayVehicleAni("BrakeOn", LTFALSE);

			if (!m_hBrakeSnd)
			{
				char* pSound = GetBrakeSnd();

				if (pSound)
				{
					uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
					m_hBrakeSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
				}
			}

		}
		else
		{
			if (m_hBrakeSnd)
			{
                g_pLTClient->SoundMgr()->KillSound(m_hBrakeSnd);
                m_hBrakeSnd = LTNULL;
			}
		}

		if (m_bVehicleStopped)
		{
			if (m_hVehicleDecelSnd)
			{
                g_pLTClient->SoundMgr()->KillSound(m_hVehicleDecelSnd);
                m_hVehicleDecelSnd = LTNULL;
			}
		}
		else if (!m_hVehicleDecelSnd)
		{
			char* pSound = GetDecelSnd();

			if (pSound)
			{
                uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
				m_hVehicleDecelSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}
	}
	else // Forward
	{
		// Make sure we're playing the accelerate sound...

		if (!m_hVehicleAccelSnd)
		{
			KillAllVehicleSounds();

			char* pSound = GetAccelSnd();

			if (pSound)
			{
                uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT | PLAYSOUND_LOOP;
				m_hVehicleAccelSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}

		if (bForward && !m_bVehicleAtMaxSpeed)
		{
			// Play accelleration ani if necessary...

			if (bPlayAni)
			{
				PlayVehicleAni("Accelerate", LTFALSE);
			}
		}
	}


	// See if we are idle...

	if (!bForward)
	{
		if (m_bVehicleStopped )
		{
			PlayVehicleAni("Idle_0", LTFALSE, LTTRUE);

			if (!m_hIdleSnd)
			{
				KillAllVehicleSounds(LTNULL);

				char* pSound = GetIdleSnd();

				if (pSound)
				{
					uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
					m_hIdleSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
				}
			}
		}
		else if( !bBrake )
		{
			if (bPlayAni)
			{
				PlayVehicleAni("Decelerate", LTFALSE);
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetIdleSnd
//
//	PURPOSE:	Get the appropriate idle sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetIdleSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			return "Snd\\Vehicle\\Snowmobile\\idle.wav";
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetAccelSnd
//
//	PURPOSE:	Get the appropriate acceleration sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetAccelSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			return "Snd\\Vehicle\\Snowmobile\\accel.wav";
		break;

		case PPM_LURE :
			return "Snd\\Vehicle\\bicycle\\accel.wav";
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetDecelSnd
//
//	PURPOSE:	Get the appropriate deceleration sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetDecelSnd()
{
 	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			return "Snd\\Vehicle\\Snowmobile\\decel.wav";
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetBrakeSnd
//
//	PURPOSE:	Get the appropriate brake sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetBrakeSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
			return "Snd\\Vehicle\\Snowmobile\\brake.wav";
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetImpactSnd
//
//	PURPOSE:	Get the appropriate impact sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetImpactSnd(LTFLOAT fCurVelocityPercent, SURFACE* pSurface)
{
	if (!pSurface) return LTNULL;

    char* pSound = LTNULL;

	//char s_szImpactSndName[128] = "";
	//pSurface->GetRandomSnowmobileImpactSnd(szImpactSndName, ARRAY_LEN(szImpactSndName));
	//return s_szImpactSndName;

	if (pSurface->eType == ST_FLESH)
	{
		if (GetRandom(0, 1) == 1)
		{
			pSound = "Snd\\Vehicle\\vehiclehit1.wav";
		}
		else
		{
			pSound = "Snd\\Vehicle\\vehiclehit2.wav";
		}
	}
	else
	{
		switch (m_ePPhysicsModel)
		{
			case PPM_SNOWMOBILE :
			{
				if (fCurVelocityPercent > 0.1f && fCurVelocityPercent < 0.4f)
				{
					pSound = "Snd\\Vehicle\\Snowmobile\\slowimpact.wav";
				}
				else if (fCurVelocityPercent < 0.7f)
				{
					pSound = "Snd\\Vehicle\\Snowmobile\\medimpact.wav";
				}
				else
				{
					pSound = "Snd\\Vehicle\\Snowmobile\\fastimpact.wav";
				}
			}
			break;

			case PPM_LURE :
			break;

			case PPM_NORMAL:
			default :
			break;
		}
	}

	return pSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::KillAllVehicleSounds
//
//	PURPOSE:	Kill all the vehicle sounds
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::KillAllVehicleSounds(HLTSOUND hException)
{
	if (m_hVehicleStartSnd && hException != m_hVehicleStartSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleStartSnd);
        m_hVehicleStartSnd = LTNULL;
	}

	if (m_hVehicleAccelSnd && hException != m_hVehicleAccelSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleAccelSnd);
        m_hVehicleAccelSnd = LTNULL;
	}

	if (m_hVehicleDecelSnd && hException != m_hVehicleDecelSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleDecelSnd);
        m_hVehicleDecelSnd = LTNULL;
	}

	if (m_hIdleSnd && hException != m_hIdleSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hIdleSnd);
        m_hIdleSnd = LTNULL;
	}

	if (m_hBrakeSnd && hException != m_hBrakeSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hBrakeSnd);
        m_hBrakeSnd = LTNULL;
	}

	// RETAIL: 0x1011eb10 also kills the cruise and +0x2c sounds.
	if (m_hVehicleCruiseSnd && hException != m_hVehicleCruiseSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleCruiseSnd);
        m_hVehicleCruiseSnd = LTNULL;
	}

	if (m_hTronUnknownSnd2c && hException != m_hTronUnknownSnd2c)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSnd2c);
        m_hTronUnknownSnd2c = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnTouchNotify
//
//	PURPOSE:	Handle our object touching something...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnTouchNotify(CollisionInfo *pInfo, float forceMag)
{
	// RETAIL: Tron's (0x1011ec20): NOLF2's snowmobile collision response
	// (the surface, the impact checks, the characters it ran over and the
	// touch saved for HandleCollision) is gone; the light cycle tells the
	// player manager it hit something.

	if (!pInfo->m_hObject) return;

	// Cancel out the engine's velocity adjustment on touch.  We'll handle that, thank you.
	LTVector vOldVel = g_pMoveMgr->GetVelocity();
	g_pMoveMgr->SetVelocity(vOldVel - pInfo->m_vStopVel);

	// Filter out the player object
	if (pInfo->m_hObject == g_pLTClient->GetClientObject()) return;

	// Test for a JumpVolume...
	
	uint16 nCode;
	if( g_pLTClient->GetContainerCode( pInfo->m_hObject, &nCode) )
	{
		if( nCode == CC_JUMP_VOLUME )
		{
			if( HandleJumpVolumeTouch( pInfo->m_hObject ))
			{
				return;			
			}
		}
	}
	
	// All other objects must be solid for us to collide with them...
    
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_Flags, dwFlags);
	if (!(dwFlags & FLAG_SOLID)) 
	{
		return;
	}

	if (IsMainWorld(pInfo->m_hObject)) return;

	uint32 dwType;
	g_pCommonLT->GetObjectType(pInfo->m_hObject, &dwType);
	if (dwType == OT_WORLDMODEL) return;

	if (pInfo->m_hObject != g_pPlayerMgr->GetPlayerCamera()->GetCollisionObject())
	{
		g_pPlayerMgr->TronUnknownVirtual60();
	}
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::HandleCollision
//
//	PURPOSE:	Handle colliding with something
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::HandleCollision()
{
	// RETAIL: 0x1011ed97: a light cycle that hits something derezzes
	if (m_ePPhysicsModel == PPM_LIGHTCYCLE)
	{
		m_bTronUnknown2158 = false;
		ShutdownLightCycleFX();
		g_pPlayerMgr->SetTronUnknown1D5(true);

		// The player manager names the cycle's derez FX
		char szFX[512];
		g_pPlayerMgr->TronUnknownVirtual74(g_pMoveMgr->GetObject(), szFX, sizeof(szFX));

		CLIENTFX_CREATESTRUCT fxInit(szFX, FXFLAG_NOSMOOTHSHUTDOWN, g_pMoveMgr->GetObject());
		g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &fxInit.m_vPos);
		g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX2138, fxInit, LTTRUE);

		m_bTronUnknown2158 = true;
		m_nTronUnknown215c = 1;

		g_pMoveMgr->AllowMovement(LTFALSE);
		ShowVehicleModel(LTFALSE);
		return;
	}

	LTVector vNormal = m_cLastTouchInfo.m_Plane.m_Normal;

	SurfaceType eType = ::GetSurfaceType(m_cLastTouchInfo);
	SURFACE* pSurface = g_pSurfaceMgr->GetSurface(eType);

	LTBOOL bIsWorld = IsMainWorld(m_cLastTouchInfo.m_hObject);

	vNormal.y = 0.0f; // Don't care about this anymore...

	// Determine the current percentage of our max velocity we are
	// currently moving...

	LTVector vVel = g_pMoveMgr->GetVelocity();
	vVel.y = 0.0f;

	LTFLOAT fVelPercent = GetVehicleMovementPercent();

	// Get our forward vector...

	HOBJECT hObj = g_pMoveMgr->GetObject();

    LTVector vU, vR, vF;
    LTRotation rRot;
	g_pLTClient->GetObjectRotation(hObj, &rRot);
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();
	
	vF.y = 0.0f; // Don't care about this...

	// Determine the direction to change our forward to...
	// Determine the angle between our forward and the new direction...

	vNormal.Normalize();
	vF.Normalize();

	// fAngle will be a value between -1 (left) and 1 (right)...

	LTFLOAT fAngle = VEC_DOT(vNormal, vR);

	if (fAngle > 0.0f)
	{
		fAngle = (MATH_PI/2.0f) * (1.0f - fAngle);
	}
	else
	{
		fAngle = (-MATH_PI/2.0f) * (1.0f + fAngle);
	}

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
	   g_pLTClient->CPrint("Impact Angle: %.2f", RAD2DEG(fAngle));
	}

	LTFLOAT fImpactAngle = (LTFLOAT) fabs(fAngle);

	float fBiggestAngle = DEG2RAD(g_vtVehicleImpactMaxAdjustAngle.GetFloat());

	if (fImpactAngle > fBiggestAngle)
	{
		fAngle = fAngle > 0.0f ? fBiggestAngle : -fBiggestAngle;
	}

	// Add this angle to the yaw...

    LTFLOAT fYaw         = g_pPlayerMgr->GetYaw();
    LTFLOAT fPlayerYaw   = g_pPlayerMgr->GetPlayerYaw();

	// Adjust the min impact angle based on our current velocity (the
	// faster you go the more hitting stuff matters ;)

	float fMinImpactAngle = DEG2RAD(g_vtVehicleImpactMinAngle.GetFloat());

	fMinImpactAngle = fMinImpactAngle + (
		(1.0f - fVelPercent) * (DEG2RAD(90.0f) - fMinImpactAngle));

	// See if we hit hard enough to play a sound and do damage...

	if (fImpactAngle > fMinImpactAngle)
	{
		LTFLOAT fPushPercent = g_vtVehicleImpactPushVelPercent.GetFloat();
		if (fVelPercent < fPushPercent) return;

		// RETAIL: 0x1011f22c: Tron's plays the surface's impact FX at the
		// vehicle, or at the camera in first person (that position also
		// places the pusher)

		LTVector vPos;
		g_pLTClient->GetObjectPos(hObj, &vPos);

		uint32 dwFXFlags = 0;
		if (g_pPlayerMgr->IsFirstPerson())
		{
			dwFXFlags = FXFLAG_REALLYCLOSE;

			HLOCALOBJ hCamera = g_pPlayerMgr->GetCamera();
			if (!hCamera) return;

			g_pLTClient->GetObjectPos(hCamera, &vPos);

			LTRotation rRot;
			g_pPlayerMgr->GetCameraRotation(rRot);

			vPos += (rRot.Forward() * g_vtVehicleImpactFXOffsetForward.GetFloat());
			vPos += (rRot.Up() * g_vtVehicleImpactFXOffsetUp.GetFloat());
		}

		if (pSurface)
		{
			CLIENTFX_CREATESTRUCT fxCS(pSurface->szImpactFXName, dwFXFlags, vPos);
			fxCS.m_vTargetNorm = m_cLastTouchInfo.m_Plane.m_Normal;

			g_pClientFXMgr->CreateClientFX(LTNULL, fxCS, LTTRUE);
		}


		// Push us away from the impact...(keep y value the same so we
		// don't get pushed up into the air)

		LTVector vDir = vVel;
		vDir.Normalize();

		float dot = VEC_DOT(vDir, m_cLastTouchInfo.m_Plane.m_Normal);
		dot *= -2.0f;

		VEC_ADDSCALED(vDir, vDir, m_cLastTouchInfo.m_Plane.m_Normal, dot);

		float fOldY = vPos.y;
		vPos += (vDir * -1.0f);

		vPos.y = fOldY;

		float fPushRadius = g_vtVehicleImpactPushRadius.GetFloat();
		float fPushAmount = g_vtVehicleImpactPushAmount.GetFloat() * fVelPercent;
		float fStartDelay = 0.0f;
		float fDuration   = g_vtVehicleImpactPushDuration.GetFloat();
		g_pMoveMgr->AddPusher(vPos, fPushRadius, fStartDelay, fDuration, fPushAmount);

	
		if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
		{
			g_pLTClient->CPrint("Pusher Pos: %.2f, %.2f, %.2f Amount: %.2f", VEC_EXPAND(vPos), fPushAmount);
		}

		// Move handle bars down a bit...(head over)...

		CameraDelta delta;
		delta.Pitch.fVar	= DEG2RAD(3.0f);
		delta.Pitch.fTime1	= 0.1f;
		delta.Pitch.fTime2	= 0.3f;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);

		// Tilt steering wheel based on the surface we hit...

		LTFLOAT fOffset		= DEG2RAD(g_vtVehicleMouseMinYawBuffer.GetFloat());
		LTFLOAT fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());
		LTFLOAT fYawPercent = (fImpactAngle / DEG2RAD(90.0f));
		m_fYawDiff = fYawPercent * fMaxOffset;
		m_fYawDiff *= (fAngle > 0.0f ? 1.0f : -1.0f);

		if (m_fYawDiff < -fOffset)
		{
			m_eMouseTurnDirection = TD_LEFT;
		}
		else if (m_fYawDiff > fOffset)
		{
			m_eMouseTurnDirection = TD_RIGHT;
		}

		// Clear velocity...

		LTVector vNewVel(0, 0, 0);

		LTFLOAT fAdjust = (1.0f - fYawPercent);
		vNewVel = vVel * fAdjust;
		g_pMoveMgr->SetVelocity(vNewVel);
		m_fVehicleBaseMoveAccel *= fAdjust;
		m_fAccelStart = -1.0f;

		// Reset the ani/sounds...

		PlayVehicleAni("Idle_0", LTFALSE, LTTRUE);
		KillAllVehicleSounds(LTNULL);

		
		// Play an impact sound based on the surface we hit..

		if (pSurface && !m_hVehicleImpactSnd)
		{
			char* pSound = GetImpactSnd(fVelPercent, pSurface);
			if (pSound)
			{
				uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
				m_hVehicleImpactSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}

		// See if we hit hard enough to actually damage us...

		LTFLOAT fImpactVelPercent = g_vtVehicleImpactDamageVelPercent.GetFloat();
		if (fVelPercent > fImpactVelPercent)
		{
			// Okay, since we were going pretty damn fast, just determine
			// the damage based on the angle we hit...

			float fRange = (MATH_PI/2.0f) - fMinImpactAngle;
			if (fRange <= 0.0f) fRange = 0.01f;

			LTFLOAT fDamagePercent = 1.0f - (((MATH_PI/2.0f) - fImpactAngle) / fRange);

 			// Send damage message to server...

			float fVehicleImpactDamageMin = g_vtVehicleImpactDamageMin.GetFloat();
			float fVehicleImpactDamageMax = g_vtVehicleImpactDamageMax.GetFloat();

			float fDamageRange = fVehicleImpactDamageMax - fVehicleImpactDamageMin;
			float fDamage = fVehicleImpactDamageMin + fDamageRange * fDamagePercent;

			if (g_vtVehicleImpactDamage.GetFloat())
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
				cMsg.Writeuint8(CP_DAMAGE);
				cMsg.Writeuint8(DT_STUN);
				cMsg.Writefloat(0.0f);
				cMsg.WriteLTVector(vNormal);
				cMsg.Writeuint8(1);
				cMsg.Writefloat(1.0f);
				cMsg.WriteObject(g_pLTClient->GetClientObject());
			    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			}

			if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
			{
				g_pLTClient->CPrint("Impact Damage: %.2f", fDamage);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleLightPosRot
//
//	PURPOSE:	Get the vehicle light pos/rot
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::GetVehicleLightPosRot(LTVector & vPos, LTRotation & rRot)
{
	vPos.Init();
	rRot.Init();

	if (!m_hVehicleModel) return;

	// Get the vehicle light position/rotation

	HMODELSOCKET hSocket;

	if (g_pModelLT->GetSocket(m_hVehicleModel, "Light", hSocket) == LT_OK)
	{
		LTransform transform;
        if (g_pModelLT->GetSocketTransform(m_hVehicleModel, hSocket, transform, LTFALSE) == LT_OK)
		{
			vPos = transform.m_Pos;
			rRot = transform.m_Rot;

			HOBJECT hCamera = g_pPlayerMgr->GetCamera();
			if (!hCamera) return;

			LTVector vCamPos;
			g_pLTClient->GetObjectPos(hCamera, &vCamPos);
			g_pLTClient->GetObjectRotation(hCamera, &rRot);

			// Rotate down a bit...
			rRot.Rotate(rRot.Right(), DEG2RAD(10.0f));

			vPos += vCamPos;
		}
	}
	else
	{
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
		if (!hCamera) return;

		g_pLTClient->GetObjectRotation(hCamera, &rRot);
		g_pLTClient->GetObjectPos(hCamera, &vPos);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::HandleNodeControl
//
//	PURPOSE:	Handle node control
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat)
{
	if (m_VehicleNode1.hNode && hNode == m_VehicleNode1.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode1.matTransform;
	}
	else if (m_VehicleNode2.hNode && hNode == m_VehicleNode2.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode2.matTransform;
	}
	else if (m_VehicleNode3.hNode && hNode == m_VehicleNode3.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode3.matTransform;
	}
}







// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CalculateVehicleCameraDisplacment()
//
//	PURPOSE:	Allows vehicles to perterb the camera placement.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CalculateVehicleCameraDisplacment( LTVector& vDisplacement )
{
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		case PPM_LURE:
			{
				PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
				if( !pPlayerLureFX )
				{
					ASSERT( !"CVehicleMgr::MoveToLure:  Missing PlayerLureFX." );
					return;
				}

				// Only do displacement stuff for bicycle.
				if( !pPlayerLureFX->GetBicycle( ))
					return;

				// Don't update the bob/sway if in spectator or 3rd person.
				if( g_pPlayerMgr->IsSpectatorMode() || !g_pPlayerMgr->IsFirstPerson( ))
					return;

				// Check if we're going over a bump.
				if( !m_LureTimeOverBump.Stopped( ))
				{
					float fPeriod = MATH_PI / m_LureTimeOverBump.GetDuration( );
					float fDisplacement = m_fLureBumpHeight * ( float )sin( m_LureTimeOverBump.GetElapseTime( ) * fPeriod );

					vDisplacement.y += fDisplacement;
				}
				// Check if it's time to go over another bump.
				else if( m_LureTimeToNextBump.Stopped( ))
				{
					float fMin, fMax;

					sscanf( g_vtBicycleInterBumpTimeRange.GetStr( ), "%f %f", &fMin, &fMax );
					m_LureTimeToNextBump.Start( GetRandom( fMin, fMax ));

					sscanf( g_vtBicycleIntraBumpTimeRange.GetStr( ), "%f %f", &fMin, &fMax );
					m_LureTimeOverBump.Start( GetRandom( fMin, fMax ));

					sscanf( g_vtBicycleBumpHeightRange.GetStr( ), "%f %f", &fMin, &fMax );
					m_fLureBumpHeight = GetRandom( fMin, fMax );
				}

				// Adjust our parameterized bob.
				m_fLureBobParameter += g_pLTClient->GetFrameTime( ) * g_vtBicycleBobRate.GetFloat( );
				
				// Keep in range of 0 to 1.0.
				m_fLureBobParameter = ( float )fmod( m_fLureBobParameter, 1.0 );

				// Get the camera so we can add some roll.
				HOBJECT hCamera = g_pPlayerMgr->GetCamera( );
				LTRotation rCameraRot;
				g_pLTClient->GetObjectRotation( hCamera, &rCameraRot );

				// Make our sway left/right always camera relative.
				LTVector vBobSway = rCameraRot.Right( );
				vBobSway.y = 0.0f;
				vBobSway.Normalize( );

				// Get the amount to sway and add it to the displacement.
				float fBobSway = ( float )sin( m_fLureBobParameter * MATH_CIRCLE );
				fBobSway *= g_vtBicycleBobSwayAmplitude.GetFloat( );
				vBobSway *= fBobSway;
				vDisplacement += vBobSway;
				
				// Get the amount of roll and apply it to the camera's rotation.
				float fBobRoll = ( float )sin( m_fLureBobParameter * MATH_CIRCLE );
				fBobRoll *= g_vtBicycleBobRollAmplitude.GetFloat( ) * MATH_PI / 180.0f;
				LTRotation rBobRoll( 0.0f, 0.0f, fBobRoll );
				LTRotation rFinal = rCameraRot * rBobRoll;
				g_pLTClient->SetObjectRotation( hCamera, &rFinal );

				if( !m_hVehicleModel )
				{
					ASSERT( !"CVehicleMgr::CalculateVehicleCameraDisplacment:  Missing vehicle model." );
					return;
				}

				LTVector vBicycleOffset;
				sscanf( g_vtBicycleModelOffset.GetStr( ), "%f %f %f", &vBicycleOffset.x, &vBicycleOffset.y, 
					&vBicycleOffset.z );
	
				// Offset the vehicle model based on the lure's transform.
				LTVector vLurePos;
				g_pLTClient->GetObjectPos( pPlayerLureFX->GetServerObj( ), &vLurePos );
				LTRotation rLureRot;
				g_pLTClient->GetObjectRotation( pPlayerLureFX->GetServerObj( ), &rLureRot );
				LTMatrix matLureRot;
				rLureRot.ConvertToMatrix( matLureRot );
				vBicycleOffset = matLureRot * vBicycleOffset;
				LTVector vVehicleModelPos = vLurePos + vBicycleOffset;
				g_pLTClient->SetObjectPos( m_hVehicleModel, &vVehicleModelPos );

				// Add the bob roll into the vehicle model rotation.
				LTRotation rVehicleModelRot = rLureRot * rBobRoll;
				g_pLTClient->SetObjectRotation( m_hVehicleModel, &rVehicleModelRot );
			}
			break;

		case PPM_NORMAL:
		case PPM_SNOWMOBILE:
		case PPM_LIGHTCYCLE:
			return;
			break;

		default:
			ASSERT( !"CVehicleMgr::CalculateVehicleCameraDisplacment: Invalid physics model." );
			break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSnowmobileGear
//
//	PURPOSE:	Update the snowmobile's current gear...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSnowmobileGear()
{
	LTVector myVel = g_pMoveMgr->GetVelocity();

	// Print out info...

	
	if (g_vtSnowmobileInfoTrack.GetFloat() == SNOW_TRACK_GENERAL)
	{
		// How fast are we going anyway...
		
		LTVector vAccel;
		g_pPhysicsLT->GetAcceleration(g_pMoveMgr->GetObject(), &vAccel);
		
		g_pLTClient->CPrint("Vel = %.2f", myVel.Mag());
		g_pLTClient->CPrint("Accel = %.2f", vAccel.Mag());
		g_pLTClient->CPrint("HandleBarRoll = %.2f", m_fHandlebarRoll);
	}
		
	myVel.y = 0.0f;

	LTBOOL bUsingAccel = (LTBOOL) g_vtSnowmobileAccelGearChange.GetFloat();

	LTFLOAT fMaxVal  = bUsingAccel ? GetMaxAccelMag() : GetMaxVelMag();
	LTFLOAT fVal	 = bUsingAccel ? m_fVehicleBaseMoveAccel : myVel.Mag();

	LTBOOL bWasStopped = m_bVehicleStopped;

	m_bVehicleStopped = LTFALSE;
	if (fVal < fMaxVal * g_vtSnowmobileStoppedPercent.GetFloat())
	{
        m_bVehicleStopped = LTTRUE;

		myVel.Init();
		g_pMoveMgr->SetVelocity(myVel);
	}

	// Add a little movement to show we stopped...

	if (!bWasStopped && m_bVehicleStopped)
	{
		CameraDelta delta;
		delta.Pitch.fVar	= DEG2RAD(0.25f);
		delta.Pitch.fTime1	= 0.4f;
		delta.Pitch.fTime2	= 0.5;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);
	}
	else if (bWasStopped && !m_bVehicleStopped)
	{
		// Show a little movement to show we started moving...

		CameraDelta delta;

		delta.Pitch.fVar	= DEG2RAD(-0.5f);
		delta.Pitch.fTime1	= 0.25f;
		delta.Pitch.fTime2	= 0.5;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);
	}

    m_bVehicleAtMaxSpeed = LTFALSE;
	if (fVal > fMaxVal * g_vtSnowmobileMaxSpeedPercent.GetFloat())
	{
        m_bVehicleAtMaxSpeed = LTTRUE;
	}

	m_nLastGear = m_nCurGear;

	m_nCurGear = VEHICLE_GEAR_0;

	// Update the handlebars...

	UpdateSnowmobileHandleBars();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSnowmobileHandleBars
//
//	PURPOSE:	Update the snowmobile handlebars
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSnowmobileHandleBars()
{
	if (!m_hVehicleModel) return;

	// Reset all our nodes matrices

	m_VehicleNode1.matTransform.Identity();
	m_VehicleNode2.matTransform.Identity();
	m_VehicleNode3.matTransform.Identity();

	// Do the rotation on the handle bar node
	//
    LTRotation rRot;
    rRot.Init();
    LTVector vAxis(0.0f, 1.0f, 0.0f);

	// Negate the roll for some reason
	rRot.Rotate(vAxis, -m_fHandlebarRoll);

	// Create a rotation matrix and apply it to the current offset matrix

	LTMatrix m1;
	rRot.ConvertToMatrix(m1);
	m_VehicleNode1.matTransform = m_VehicleNode1.matTransform * m1;


	// Do the rotation on the speedometer node...
	
    LTFLOAT fMaxRot = MATH_DEGREES_TO_RADIANS(270.0f);

	// Calculate the speed dial rotation...

	LTVector vVel = g_pMoveMgr->GetVelocity();

	LTFLOAT fMaxVel = GetMaxVelMag();
	LTFLOAT fCurVel = vVel.Mag() > fMaxVel ? fMaxVel : vVel.Mag();

	LTFLOAT fPercent = 1.0f - ((fMaxVel - fCurVel) / fMaxVel);

	LTFLOAT fCurRot = fPercent*fMaxRot;
	
	rRot.Init();
	vAxis.Init(0.0f, 0.0f, 1.0f);
	rRot.Rotate(vAxis, -fCurRot);

	// Create a rotation matrix and apply it to the current offset matrix

	rRot.ConvertToMatrix(m1);
	m_VehicleNode2.matTransform = m_VehicleNode2.matTransform * m1;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::AdjustRoll
//
//	PURPOSE:	Adjust the camera roll if necessary
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::AdjustCameraRoll(LTFLOAT & fRoll)
{
	// Only adjust if we are on a vehicle...

	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		// Don't adjust camera when selecting/deselecting the vehicle...

		LTBOOL bIsDone = LTFALSE;
		if (IsCurVehicleAni("Deselect", bIsDone) || IsCurVehicleAni("Select", bIsDone))
		{
			return;
		}

		LTFLOAT fMaxCant = DEG2RAD(g_vtMaxVehicleHeadCant.GetFloat());

		LTFLOAT fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());
		LTFLOAT fYawDiff = (LTFLOAT)fabs(m_fYawDiff);
		LTFLOAT fPercent = fYawDiff / fMaxOffset;

		if (m_fYawDiff > 0.0f)
		{
			fPercent = -fPercent;
		}

		// Save off roll value for updating handlebars...

		m_fHandlebarRoll = fMaxCant * fPercent;


		// Update head can't only when moving...

		if (!m_bVehicleStopped)
		{
			fRoll = m_fHandlebarRoll;

			// Move head left or right depending on amount we are turning...

			m_vHeadOffset.x = g_vtVehicleMaxHeadOffsetX.GetFloat() * fPercent;
			m_vHeadOffset.y = g_vtVehicleMaxHeadOffsetY.GetFloat() * fPercent;
			m_vHeadOffset.z = g_vtVehicleMaxHeadOffsetZ.GetFloat() * fPercent;
			m_fHeadYaw = DEG2RAD(g_vtVehicleMaxHeadYaw.GetFloat() * fPercent);
		}
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveLocalSolidObject
//
//	PURPOSE:	Handle the moving of the object if we need to.
//
// ----------------------------------------------------------------------- //
bool CVehicleMgr::MoveLocalSolidObject( )
{
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		// These guys don't handle movement.
		case PPM_NORMAL:
		case PPM_SNOWMOBILE:
			return false;
			break;

		case PPM_LIGHTCYCLE:
		{
			// RETAIL: 0x10121050 -- Tron's player manager moves the light cycle.
			return g_pPlayerMgr->TronUnknownVirtual64();
			break;
		}

		case PPM_LURE:
			return MoveToLure( );
			break;

		default:
			ASSERT( !"CVehicleMgr::MoveLocalSolidObject: Invalid physics model." );
			return false;
			break;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveToLure
//
//	PURPOSE:	Handle the moving of the object if we need to.
//
// ----------------------------------------------------------------------- //
bool CVehicleMgr::MoveToLure( )
{
	// Get the playerlurefx.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing PlayerLureFX." );
		return false;
	}

	// Get the server lure object.
	HOBJECT hServerObj = pPlayerLureFX->GetServerObj( );
	if( !hServerObj )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing server-side player object." );
		return false;
	}

	// Get the client player object.
	HOBJECT hObj = g_pMoveMgr->GetObject( );
	if( !hObj )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing client-side player object." );
		return false;
	}

	// Our resulting pos and rotation.
	LTVector vPos;
	LTRotation rRot;

	// Apply offset if told to.
	if( pPlayerLureFX->GetRetainOffsets( ))
	{
		// Get the playerlure's transform.
		LTransform playerLureTransform;
		g_pLTClient->GetObjectPos( hServerObj, &playerLureTransform.m_Pos );
		g_pLTClient->GetObjectRotation( hServerObj, &playerLureTransform.m_Rot );
		playerLureTransform.m_Scale.Init( 1.0f, 1.0f, 1.0f );

		// Get the offset transform.
		LTransform offsetTransform = pPlayerLureFX->GetOffsetTransform( );

		// Find the offset position and rotation.
		LTransform playerTransform;
	    ILTTransform *pTransformLT = g_pLTClient->GetTransformLT();
		pTransformLT->Multiply( playerTransform, playerLureTransform, offsetTransform );

		// Stuff the result.
		vPos = playerTransform.m_Pos;
		rRot = playerTransform.m_Rot;
	}
	// Not retaining offsets.
	else
	{
		// Get the playerlure's position.
		g_pLTClient->GetObjectPos( hServerObj, &vPos );

		// Get the playerlure's rotation.
		g_pLTClient->GetObjectRotation( hServerObj, &rRot );
	}

	g_pPhysicsLT->MoveObject( hObj, &vPos, MOVEOBJECT_TELEPORT );
	g_pLTClient->SetObjectRotation( hObj, &rRot );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::GetVehicleMovementPercent
//
//  PURPOSE:	Calculate the percentage from our max velocity...
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetVehicleMovementPercent() const
{
	LTVector vVel = g_pMoveMgr->GetVelocity();
	vVel.y = 0.0f;

	LTFLOAT fMaxVel = GetMaxVelMag();
	LTFLOAT fCurVel = vVel.Mag() > fMaxVel ? fMaxVel : vVel.Mag();

	if (fMaxVel > 0.0f)
	{
		return 1.0f - ((fMaxVel - fCurVel) / fMaxVel);
	}

	return 0.0f;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::HandleJumpVolumeTouch
//
//  PURPOSE:	Handle hitting a JumpVolume...
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::HandleJumpVolumeTouch( HOBJECT hJumpVolume )
{
	if( !hJumpVolume ) return false;

	// We only want one JumpVolume to effect us at a time...
	
	if( m_hCurJumpVolume )
		return true;
	
	// Grab the SpecialFX associated with this object...
	
	CJumpVolumeFX *pFX = (CJumpVolumeFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_JUMPVOLUME_ID, hJumpVolume );
	if( pFX )
	{
			
		// Dont add the full velocity amount if we aren't at the max...
				
		LTRotation	rRot;
		g_pLTClient->GetObjectRotation( g_pMoveMgr->GetObject(), &rRot );
		
		LTVector	vF = rRot.Forward();
		vF.y = 0.0f;
		vF.Normalize();

		LTVector	vDir = pFX->GetVelocity();
		vDir.y = 0.0f;
		vDir.Normalize();
		
		float	fDot = rRot.Forward().Dot( vDir );

		// Don't let the JumpVolume effct us if we're heading in the opposite direction...

		if( fDot <= 0.001f )
			return true;

		// Adjust the percentage based on the min and max values...

		float	fDirMax = g_vtJumpVolumeMaxDirectionPercent.GetFloat();
		float	fDirMin	= g_vtJumpVolumeMinDirectionPercent.GetFloat();
		float	fT = 0.0f;
		
		if( fDot >= fDirMax )
		{
			// Give us full amount
			fT = 1.0f;
		}
		else if( fDot <= fDirMin )
		{
			// Don't let it affect us at all
			return true;
		}
		else
		{
			fT = (fDot - fDirMin) / (1.0f - fDirMin);
		}

		WaveType	eType = (WaveType)(int)g_vtJumpVolumeWaveType.GetFloat();
		float		fPercent = GetVehicleMovementPercent() * GetWaveFn(eType)(fT); 

		if( g_vtJumpVolumeInfo.GetFloat() > 0.0f )
		{
			g_pLTClient->CPrint( "Movement: %.4f Dot: %.4f T: %.4f Wave: %.3f", GetVehicleMovementPercent(), fDot, fT, GetWaveFn(eType)(fT) );
			g_pLTClient->CPrint( "Percent: %.4f", fPercent );
		}
		
		LTVector vJumpVolumeVel = g_pMoveMgr->GetVelocity() + (pFX->GetVelocity() * fPercent);
		g_pMoveMgr->SetVelocity( vJumpVolumeVel );

		m_fJumpVolumeVel = vJumpVolumeVel.Mag();
		
		// The vehicle is now being affected by a JumpVolume...

		m_hCurJumpVolume		= hJumpVolume;
		m_bHaveJumpVolumeVel	= LTTRUE;

		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::UpdateInJumpVolume
//
//  PURPOSE:	Detrimines if we are still within a JumpVolume...
//				This is really only used so a JumpVolume doesn't keep 
//				adding it's velocity if we intersect it on multiple frames.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateInJumpVolume()
{
	// Make sure we are in a JumpVolume...

	if( !m_hCurJumpVolume )
		return;

	HOBJECT hObj = g_pMoveMgr->GetObject();

	LTVector	vJVPos, vObjPos, vJVDims, vObjDims;
	LTVector	vJVMin, vJVMax, vObjMin, vObjMax;

	g_pLTClient->GetObjectPos( m_hCurJumpVolume, &vJVPos );
	g_pLTClient->GetObjectPos( hObj, &vObjPos );

	g_pPhysicsLT->GetObjectDims( m_hCurJumpVolume, &vJVDims );
	g_pPhysicsLT->GetObjectDims( hObj, &vObjDims );

	// Setup the extents...
	
	vJVMin	= vJVPos - vJVDims;
	vJVMax	= vJVPos + vJVDims;
	vObjMin	= vObjPos - vObjDims;
	vObjMax = vObjPos + vObjDims;

	// If the boxes arent intersecting then we are no longer in the JumpVolume...

	if( !BoxesIntersect( vJVMin, vJVMax, vObjMin, vObjMax ))
		m_hCurJumpVolume = LTNULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateLightCycleControlFlags
//
//	PURPOSE:	Update the Light Cycke control flags
//
// ----------------------------------------------------------------------- //
// RETAIL: 0x10121920. The forward and reverse keys set Tron's own flags
// (+0x2134), not the control flags.
void CVehicleMgr::UpdateLightCycleControlFlags()
{
	// RETAIL: 0x10121920. Turning only: the cycle always moves forward.
	m_bTurned = false;

	if (g_pLTClient->IsCommandOn(COMMAND_ID_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_LEFT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_RIGHT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))
	{
		m_nTronUnknown2134 |= 0x1000;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_REVERSE))
	{
		m_nTronUnknown2134 |= 0x10000;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CalculateVehicleRotation()
//
//	PURPOSE:	Calculate the new player/camera rotation.
//
//	RETAIL: Tron's (0x101219b0) is the light cycle's: the player faces the
//	cycle's direction and the camera yaw follows it, after a lag once the
//	player turns. NOLF2's banking, lure and contouring rotations are gone.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CalculateVehicleRotation(LTVector & vPlayerPYR,
										   LTVector & vPYR, LTFLOAT fYawDelta)
{
	if (fYawDelta != 0.0f && m_fLightCycleCamLag > 0.0f)
	{
		m_TronUnknownTimer2160.Start(m_fLightCycleCamLag);
	}

	// RETAIL: retail passes the vector's address in the int argument
	g_pPlayerMgr->TronUnknownVirtual6C((int)&m_vTronUnknown2128);
	vPlayerPYR = m_vTronUnknown2128;

	LTRotation rRot(vPlayerPYR.x, vPlayerPYR.y, vPlayerPYR.z);
	g_pLTClient->SetObjectRotation(g_pMoveMgr->GetObject(), &rRot);

	float fYaw = vPYR.y;
	WrapAngleToCircle(fYaw);

	bool bSnap = false;
	if (m_fLightCycleCamLag == 0.0f && g_pPlayerMgr->IsFirstPerson() &&
		(m_dwControlFlags & (BC_CFLG_RIGHT | BC_CFLG_LEFT)))
	{
		if (!m_bTronUnknown2170)
		{
			bSnap = true;
		}
	}
	else
	{
		m_bTronUnknown2170 = false;
	}

	if (g_pPlayerMgr->IsFirstPerson())
	{
		float fTarget = m_vTronUnknown2128.y;
		float fDiff = (float)fabs(fTarget - fYaw);
		if (fDiff > MATH_PI)
		{
			fDiff = MATH_CIRCLE - fDiff;
		}

		if (fDiff > MATH_HALFPI)
		{
			TurnAngleTowards(fYaw, fTarget, fDiff - MATH_HALFPI);
			if (bSnap)
			{
				vPYR.y = fYaw;
				return;
			}
		}
	}

	if (bSnap) return;

	if (m_TronUnknownTimer2160.Stopped() && m_fLightCycleCamLag >= 0.0f)
	{
		TurnAngleTowards(fYaw, m_vTronUnknown2128.y, g_pLTClient->GetFrameTime() * m_fLightCycleCamSpeed);
		if (!g_pPlayerMgr->GetTronUnknown1D8())
		{
			vPYR.y = fYaw;
		}
	}
}

LTBOOL CVehicleMgr::OnCommandOn(int command)
{
	// RETAIL: 0x10121cf0: Tron's light cycle turns.  One turn per press,
	// snapped to the grid's axes, and only while the player can move; the
	// turn is kept for the server (+0x2128, flags +0x2134)
	if (m_ePPhysicsModel != PPM_LIGHTCYCLE) return LTFALSE;

	if (m_bTurned) return LTTRUE;
	if (!g_pPlayerMgr->TronUnknownVirtual94()) return LTTRUE;

	if (command == COMMAND_ID_STRAFE_LEFT)
	{
		if (g_pMoveMgr->GetAllowMovement())
		{
			// Rotate the player 90 degrees to the left
			float fYaw = g_pPlayerMgr->GetPlayerYaw() - MATH_HALFPI;
			AlignAngleToAxis(fYaw);
			g_pPlayerMgr->SetPlayerYaw(fYaw);
			g_pPlayerMgr->GetPlayerPitchYawRoll(m_vTronUnknown2128);
			m_nTronUnknown2134 |= 0x10;
			m_bTurned = true;
			m_bTronUnknown2170 = true;
		}
	}
	else if (command == COMMAND_ID_STRAFE_RIGHT)
	{
		if (g_pMoveMgr->GetAllowMovement())
		{
			// Rotate the player 90 degrees to the right
			float fYaw = g_pPlayerMgr->GetPlayerYaw() + MATH_HALFPI;
			AlignAngleToAxis(fYaw);
			g_pPlayerMgr->SetPlayerYaw(fYaw);
			g_pPlayerMgr->GetPlayerPitchYawRoll(m_vTronUnknown2128);
			m_nTronUnknown2134 |= 0x100;
			m_bTurned = true;
			m_bTronUnknown2170 = true;
		}
	}

	return LTTRUE;
}

LTBOOL CVehicleMgr::OnCommandOff(int command)
{
	switch (m_ePPhysicsModel)
	{
		case PPM_LIGHTCYCLE:
		{
			return LTTRUE;
		}
		break;

		default:
		break;
	}

	return LTFALSE;
}

bool CVehicleMgr::MoveLightCycle()
{
	LTVector newPos, curPos;
	MoveInfo info;

	// Get physics interface
	ILTClientPhysics* pPhysics = (ILTClientPhysics*)g_pPhysicsLT;

	// Get our client-side player object
	HOBJECT hObj = g_pMoveMgr->GetObject();
	info.m_hObject = hObj;

	// Do the dirty work
	info.m_dt = g_pGameClientShell->GetFrameTime();
	pPhysics->UpdateMovement(&info);

	// Make sure we actually moved
	if (info.m_Offset.MagSqr() > 0.01f)
	{
		g_pLTClient->GetObjectPos(hObj, &curPos);
		newPos = curPos + info.m_Offset;
		
		if(m_bTurned)
		{
			SnapLightCycleToXZGrid(newPos,curPos,(uint32)(g_vtLightCycleGridSize.GetFloat()));
			m_bTurned = false;
		}

		pPhysics->MoveObject(hObj, &newPos, 0);
	}

	return true;
}

void CVehicleMgr::SnapLightCycleToXZGrid(LTVector &vNewPos, LTVector &vOldPos, uint32 nGridSize)
{
	float fGridSize = (float)nGridSize;

	float xDelta = (float)fabs(vNewPos.x - vOldPos.x);
	float zDelta = (float)fabs(vNewPos.z - vOldPos.z);
	
	if(zDelta > xDelta) // Traveling along Z axis, so snap X component
	{
		// First, convert to an int
		int x = ROUNDFLOAT(vNewPos.x);
		
		// Now snap it
		if(nGridSize > 1)
		{
			vNewPos.x = ROUNDFLOAT((float)x / fGridSize) * fGridSize;
		}
	}
	else // Traveling along X axis, so snap z component
	{
		// First, convert to an int
		int z = ROUNDFLOAT(vNewPos.z);
		
		// Now snap it
		if(nGridSize > 1)
		{
			vNewPos.z = ROUNDFLOAT((float)z / fGridSize) * fGridSize;
		}
	}
}

void CVehicleMgr::AlignAngleToAxis(float &fAngle)
{
	// First make sure the yaw is in the 0 - 2pi range
	// RETAIL: Tron's (0x10121e40) calls its helper, 0x1011a190
	WrapAngleToCircle(fAngle);

	// Now axis align it
	// RETAIL: Tron also keeps the axis as a vector (+0x211c)
	if(fAngle <= (MATH_PI/4.0f)) // PI/4
	{
		fAngle = 0;
		m_vTronUnknown211c.Init(0.0f, 0.0f, 1.0f);
	}
	else if(fAngle <= (MATH_PI*0.75f)) // 3PI/4
	{
		fAngle = MATH_HALFPI;
		m_vTronUnknown211c.Init(1.0f, 0.0f, 0.0f);
	}
	else if(fAngle <= (MATH_PI*1.25)) // 5PI/4
	{
		fAngle = MATH_PI;
		m_vTronUnknown211c.Init(0.0f, 0.0f, -1.0f);
	}
	else if(fAngle <= (MATH_PI*1.75)) // 7PI/4
	{
		fAngle = MATH_HALFPI*3.0f;
		m_vTronUnknown211c.Init(-1.0f, 0.0f, 0.0f);
	}
	else
	{
		fAngle = 0;
		m_vTronUnknown211c.Init(0.0f, 0.0f, 1.0f);
	}

	/***********************
	Just in case, this code does the same thing as above but uses a forward 
	vector instead of an angle

		vF.y = 0;
		LTVector vAxis(0.0f,0.0f,0.0f);
		float fTheta;

		// Figure out which quadrant it's in
		if(vF.x >= 0.0f && vF.z >= 0.0f)
		{
			vAxis.z = 1.0f; // 12 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.x = 1.0f; // 3 o'clock
			}
			else
			{
				vMoveVel.z = 1.0f; // 12 o'clock
			}
		}
		else if(vF.x >= 0.0f && vF.z <= 0.0f)
		{
			vAxis.x = 1.0f; // 3 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.z = -1.0f; // 6 o'clock
			}
			else
			{
				vMoveVel.x = 1.0f; // 3 o'clock
			}
		}
		else if(vF.x <= 0.0f && vF.z >= 0.0f)
		{
			vAxis.z = -1.0f; // 6 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.x = -1.0f; // 9 o'clock
			}
			else
			{
				vMoveVel.z = -1.0f; // 6 o'clock
			}
		}
		else // (vF.x <= 0.0f && vF.z <= 0.0f)
		{
			vAxis.x = -1.0f; // 9 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.z = 1.0f; // 12 o'clock
			}
			else
			{
				vMoveVel.x = -1.0f; // 9 o'clock
			}
		}
	***********************/
}

void CVehicleMgr::UpdateContainerMotion()
{
	// Get the containerinfo from movemgr update the gravity and viscosity...

	g_pMoveMgr->UpdateContainerList();

	uint32 nNumContainers = g_pMoveMgr->GetNumContainers();
	CContainerInfo *pInfo = LTNULL;

	for( uint32 i = 0; i < nNumContainers; ++i )
	{
		pInfo = g_pMoveMgr->GetContainerInfo( i );
		if( !pInfo )
			continue;

		g_pMoveMgr->UpdateContainerViscosity( pInfo );
		g_pMoveMgr->UpdateContainerGravity( pInfo );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveVehicleObject
//
//	PURPOSE:	Allows the vehicle to do some extra handeling of moving the object...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::MoveVehicleObject( LTVector &vPos )
{ 
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		// These guys don't handle movement.
		case PPM_SNOWMOBILE:
		{
			HOBJECT hObj = g_pMoveMgr->GetObject();

			// Remember where we started
			LTVector vStartPos;
			g_pLTClient->GetObjectPos( hObj, &vStartPos );

			// We haven't been touched yet...
			m_bWasTouched = false;

			// First do movement normally to get collision information...
			
			g_pPhysicsLT->MoveObject( hObj, &vPos, 0 );

			// If we didn't get touched, we're done
			if (!m_bWasTouched)
				break;

			// Check where we ended up
			LTVector vResultPos;
			g_pLTClient->GetObjectPos( hObj, &vResultPos );

			// If we made it to our destination anyway, we're done
			if( vResultPos == vPos )
				break;

			// If we didn't make it far enough toward the destination, it was a collision
			LTVector vDesiredOfs = vPos - vStartPos;
			LTVector vActualOfs = vResultPos - vStartPos;

			float fDesiredMag = vDesiredOfs.Mag();
			if ((vActualOfs.Dot(vDesiredOfs) / fDesiredMag) < (fDesiredMag * g_vtVehicleImpactThreshold.GetFloat()))
				HandleCollision();
		}
		break;

		case PPM_NORMAL:
		case PPM_LIGHTCYCLE:
		case PPM_LURE:
		{
			// Simple movement call...

			g_pPhysicsLT->MoveObject( g_pMoveMgr->GetObject(), &vPos, 0 );
		}
		break;

		default:
			ASSERT( !"CVehicleMgr::MoveLocalSolidObject: Invalid physics model." );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveVehicleObject
//
//	PURPOSE:	Allows the vehicle to do some extra handeling of moving the object...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPlayerLureId( uint32 nPlayerLureId )
{
	m_nPlayerLureId = nPlayerLureId; 
	m_bResetLure = true; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::KillLightCycleSounds
//
//	PURPOSE:	Stop the light cycle's looping sounds
//				(RETAIL: 0x10122140; name ours)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::KillLightCycleSounds()
{
	if (m_hVehicleCruiseSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleCruiseSnd);
        m_hVehicleCruiseSnd = LTNULL;
	}

	if (m_hTronUnknownSnd2c)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSnd2c);
        m_hTronUnknownSnd2c = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::ShutdownLightCycleFX
//
//	PURPOSE:	Shut down the light cycle's client fx
//				(RETAIL: 0x10122190; name ours)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::ShutdownLightCycleFX()
{
	// The +0x2138 fx is left running while +0x2158 is set with +0x215c 1.
	if (m_TronUnknownFX2138.IsValid() && (!m_bTronUnknown2158 || m_nTronUnknown215c != 1))
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX2138);
		m_TronUnknownFX2138.ClearLink();
		m_bTronUnknown2158 = false;
	}

	if (m_TronUnknownFX213c.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX213c);
		m_TronUnknownFX213c.ClearLink();
	}

	if (m_TronUnknownFX2140.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX2140);
		m_TronUnknownFX2140.ClearLink();
	}

	if (m_TronUnknownFX2144.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX2144);
		m_TronUnknownFX2144.ClearLink();
	}

	if (m_TronUnknownFX2148.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX2148);
		m_TronUnknownFX2148.ClearLink();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::ResetLightCyclePhysics
//
//	PURPOSE:	Leave and re-enter the light cycle physics model
//				(RETAIL: 0x10122260; name ours)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::ResetLightCyclePhysics()
{
	SetPhysicsModel(PPM_NORMAL);
	SetPhysicsModel(PPM_LIGHTCYCLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CreateTronUnknownFX213c/2140/2144
//
//	PURPOSE:	Start a looping client FX on the vehicle model, at the
//				player's position, unless one is already running.
//				RETAIL: 0x10122280, 0x10122350, 0x10122420 (Tron's; called
//				by CClientLightCycleMgr, 0x10014a73.. ; the names are ours)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CreateTronUnknownFX213c(const char* pszFX)
{
	if (m_TronUnknownFX213c.IsValid()) return;

	CLIENTFX_CREATESTRUCT fxInit(pszFX, FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, m_hVehicleModel);
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &fxInit.m_vPos);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX213c, fxInit, LTTRUE);
}

void CVehicleMgr::CreateTronUnknownFX2140(const char* pszFX)
{
	if (m_TronUnknownFX2140.IsValid()) return;

	CLIENTFX_CREATESTRUCT fxInit(pszFX, FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, m_hVehicleModel);
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &fxInit.m_vPos);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX2140, fxInit, LTTRUE);
}

void CVehicleMgr::CreateTronUnknownFX2144(const char* pszFX)
{
	if (m_TronUnknownFX2144.IsValid()) return;

	CLIENTFX_CREATESTRUCT fxInit(pszFX, FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, m_hVehicleModel);
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &fxInit.m_vPos);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX2144, fxInit, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CreateTronUnknownFX2148
//
//	PURPOSE:	Start a looping client FX on the vehicle model and keep
//				its offset.  RETAIL: 0x101224f0 (Tron's; called by
//				CClientLightCycleMgr, 0x10013f8f; the name is ours)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CreateTronUnknownFX2148(const char* pszFX, const LTVector &vOffset)
{
	if (m_TronUnknownFX2148.IsValid() || !m_hVehicleModel) return;

	CLIENTFX_CREATESTRUCT fxInit(pszFX, FXFLAG_LOOP, m_hVehicleModel);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX2148, fxInit, LTTRUE);

	m_vTronUnknownFXOffset = vOffset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::ShowTronUnknownFX2138
//
//	PURPOSE:	Show or hide the light cycle model's FX.  RETAIL:
//				0x101225d0 (Tron's; called by CTronPlayerMgr::Update,
//				0x100a5581; the name is ours)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::ShowTronUnknownFX2138(HOBJECT hObj, bool bShow)
{
	char szBuf[512];
	if (!g_pPlayerMgr->TronUnknownVirtual78(hObj, szBuf)) return;
	if (m_bTronUnknown2158) return;
	if (!m_TronUnknownFX2138.GetInstance()) return;

	if (bShow)
	{
		m_TronUnknownFX2138.GetInstance()->Show();
	}
	else
	{
		m_TronUnknownFX2138.GetInstance()->Hide();
	}
}

// RETAIL: Tron's CVehicleMgr has no Save/Load (CMoveMgr::Save 0x100f9d30
// doesn't call one).

