// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileDisc.cpp
//
// PURPOSE : Projectile Disc class - implementation
//
// CREATED : 12/12/2001
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
//
// RETAIL: Tron's TU runs from 0x10036a70 (the class's static data) to
// 0x1003d8c0.  The functions below are written from the retail code where
// a RETAIL: note gives the address; the others are still Build 69's (PORT).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileDisc.h"
#include "MsgIDs.h"
#include "SFXMsgIds.h"
#include "WeaponFireInfo.h"
#include "TronPlayerObj.h"
#include "PlayerObj.h"
#include "AIStimulusMgr.h"
#include "CharacterMgr.h"
#include "ImpactType.h"
#include "FxDefs.h"
#include "FXButeMgr.h"
#include "CVarTrack.h"
#include "PowerArmor.h"
#include "CharacterHitBox.h"
#include "IHitBoxUser.h"
#include "SharedScoring.h"
#include "CharacterAlignment.h"
#include "AIUtils.h"
#include "ModelButeMgr.h"
#include "SubroutineDataMgr.h"

//
// LithTech Macros
//
BEGIN_CLASS(CDisc)
END_CLASS_DEFAULT_FLAGS(CDisc, CTronProjectile, NULL, NULL, CF_HIDDEN)

//
// Defines
//
#define CDISC_UPDATE_DELTA            0.001f


//
// Globals
//
extern CAIStimulusMgr* g_pAIStimulusMgr;

// RETAIL: the console variables (0x10359998 down to 0x103598b8).  The
// fourth is not used (0x10359968); its name is ours
static CVarTrack	g_vtDiscMagneticHoming;
static CVarTrack	g_vtDiscMagneticShowTarget;
static CVarTrack	g_vtDiscAutoAimRadius;
static CVarTrack	g_vtDiscTronUnknown;
static CVarTrack	g_vtDiscAutoAimTurnMult;
static CVarTrack	g_vtDiscAutoAimAngleMult;
static CVarTrack	g_vtDiscAutoAimAngleDecayMult;
static CVarTrack	g_vtDiscAutoAimTargetHeadOffset;
static CVarTrack	g_vtDiscAutoAimTargetFeetOffset;
static CVarTrack	g_vtDiscVelocity;
static CVarTrack	g_vtDiscVelocityReturn;
static CVarTrack	g_vtDiscDeflectVelocity;
static CVarTrack	g_vtDiscDeflectTime;
static CVarTrack	g_vtDiscHomingControlYOffsetPercent;
static CVarTrack	g_vtDiscHomingTargetYOffsetPercent;

static bool			s_bDiscVarsInitted;	// RETAIL: 0x103599f4

//
// Enumerations
//

enum DISC_STATE
{
	// this state is invalid while the projectile disc exists
	DISC_STATE_NONE,

	// the disc is flying toward a target
	DISC_STATE_FLYING,

	// the disc has been deflected, it isn't obeying properly
	DISC_STATE_DEFLECTING,

	// RETAIL: Tron's has one more state before the return: the deadly
	// deflection sends the disc back from the blocker (HandleDeadlyDeflect,
	// 0x10039c30; UpdateDisc's jump table, 0x1003904c, updates it as a
	// deflection).  The name is ours
	DISC_STATE_DEADLY_DEFLECTING,

	// the disc is returning to the owner
	DISC_STATE_RETURNING,

	// always last
	DISC_STATE_COUNT
};

// Disc tracking modes refer to how it adjusts its flight path
// when it flys toward its target.  Once in the "return" state,
// it always operates the same.
enum DISC_TRACKING_MODE
{
	// invalid state
	DISC_TRACKING_MODE_NONE,

	// disc flys straight with no special tracking
	DISC_TRACKING_MODE_STEADY,

	// disc will change its flight path to hit a point
	DISC_TRACKING_MODE_POINT_GUIDED,

	// disc will use a control line emitted from the owner
	DISC_TRACKING_MODE_CONTROL_LINE,

	// disc will change its flight path to hit an object
	DISC_TRACKING_MODE_HOMING,

	// always last
	DISC_TRACKING_MODE_COUNT
};


// This is the method is uses while flying through the air,
// for example, the disc can travel straight at the target
// no matter what, or it can curve in interesting ways.
enum DISC_TRACKING_STYLE
{
	// invalid tracking style
	DISC_TRACKING_STYLE_NONE,

	// disc flys straight, nothing exerts control over it
	DISC_TRACKING_STYLE_STRAIGHT,

	// disc flys straight towards its target at maximum velocity
	DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET,

	// disc flys toward a control line provided by the owner
	DISC_TRACKING_STYLE_CONTROL_LINE,
};


// RETAIL: Tron's projectile messages that the disc reads after
// MPROJ_RETURNING (ObjectMessageFn 0x10037d90).  The names are ours
#define MPROJ_DISC_TRONUNKNOWN14		14
#define MPROJ_DISC_TRONUNKNOWN150		150


// RETAIL: the impact type of a deadly block (ImpactType.h has no name for
// it).  The name is ours
#define DISC_IMPACT_TYPE_DEADLY_BLOCKED		3


namespace
{
	// MATH_EPSILON is too tight
	float const DISC_EPSILON = 0.000001f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitDiscConsoleVariables()
//
//	PURPOSE:	Set up the disc's console variables once
//				RETAIL: 0x10036d70.  The name is ours
//
// ----------------------------------------------------------------------- //

static void InitDiscConsoleVariables( ILTServer *pServer )
{
	if ( s_bDiscVarsInitted ) return;

	if ( !g_vtDiscMagneticHoming.IsInitted() )
		g_vtDiscMagneticHoming.Init( pServer, "DiscMagneticHoming", LTNULL, 0.0f );
	if ( !g_vtDiscMagneticShowTarget.IsInitted() )
		g_vtDiscMagneticShowTarget.Init( pServer, "DiscMagneticShowTarget", LTNULL, 0.0f );
	if ( !g_vtDiscAutoAimRadius.IsInitted() )
		g_vtDiscAutoAimRadius.Init( pServer, "DiscAutoAimRadius", LTNULL, 125.0f );
	if ( !g_vtDiscAutoAimTurnMult.IsInitted() )
		g_vtDiscAutoAimTurnMult.Init( pServer, "DiscAutoAimTurnMult", LTNULL, 10.0f );
	if ( !g_vtDiscAutoAimAngleMult.IsInitted() )
		g_vtDiscAutoAimAngleMult.Init( pServer, "DiscAutoAimAngleMult", LTNULL, 10.0f );
	if ( !g_vtDiscAutoAimAngleDecayMult.IsInitted() )
		g_vtDiscAutoAimAngleDecayMult.Init( pServer, "DiscAutoAimAngleDecayMult", LTNULL, 1.0f );
	if ( !g_vtDiscAutoAimTargetHeadOffset.IsInitted() )
		g_vtDiscAutoAimTargetHeadOffset.Init( pServer, "DiscAutoAimTargetHeadOffset", LTNULL, 0.0f );
	if ( !g_vtDiscAutoAimTargetFeetOffset.IsInitted() )
		g_vtDiscAutoAimTargetFeetOffset.Init( pServer, "DiscAutoAimTargetFeetOffset", LTNULL, 0.0f );
	if ( !g_vtDiscVelocity.IsInitted() )
		g_vtDiscVelocity.Init( pServer, "DiscVelocity", LTNULL, 1.0f );
	if ( !g_vtDiscVelocityReturn.IsInitted() )
		g_vtDiscVelocityReturn.Init( pServer, "DiscVelocityReturn", LTNULL, 1.0f );
	if ( !g_vtDiscDeflectVelocity.IsInitted() )
		g_vtDiscDeflectVelocity.Init( pServer, "DiscDeflectVelocity", LTNULL, 1.0f );
	if ( !g_vtDiscDeflectTime.IsInitted() )
		g_vtDiscDeflectTime.Init( pServer, "DiscDeflectTime", LTNULL, 1.0f );
	if ( !g_vtDiscHomingControlYOffsetPercent.IsInitted() )
		g_vtDiscHomingControlYOffsetPercent.Init( pServer, "DiscHomingControlYOffsetPercent", LTNULL, 0.8f );
	if ( !g_vtDiscHomingTargetYOffsetPercent.IsInitted() )
		g_vtDiscHomingTargetYOffsetPercent.Init( pServer, "DiscHomingTargetYOffsetPercent", LTNULL, 0.6f );

	s_bDiscVarsInitted = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::ClearDebugInfo()
//
//	PURPOSE:	RETAIL: 0x10037180 calls it (0x1010c0c0, folded): the body
//				is empty in the retail build.  The name is ours
//
// ----------------------------------------------------------------------- //

void CDisc::ClearDebugInfo()
{
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CDisc::RemoveObject()
//              
//	PURPOSE:	RETAIL: 0x10037180
//              
//----------------------------------------------------------------------------

void CDisc::RemoveObject()
{
	ClearDebugInfo();

	CTronProjectile::RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::CDisc()
//
//	PURPOSE:	RETAIL: 0x100371a0
//
// ----------------------------------------------------------------------- //

CDisc::CDisc() :
	  m_eDiscState( DISC_STATE_NONE )
	, m_eTrackingMode( DISC_TRACKING_MODE_NONE )
	, m_eTrackingStyle( DISC_TRACKING_STYLE_NONE )
	, m_bNewTarget( false )
	, m_vPointTarget( 0.0f, 0.0f, 0.0f )
	, m_hObjectTarget( LTNULL )
	, m_vControlDirection( 0.0f, 0.0f, 0.0f )
	, m_vControlPosition( 0.0f, 0.0f, 0.0f )
	, m_hReturnToSocket( INVALID_MODEL_SOCKET )
	, m_eDiscCatchableStimID( kStimID_Unset )
	, m_eDiscBlockableStimID( kStimID_Unset )
	, m_fReturnVelocity( 0.0f )
	, m_fReturnHeightOffset( 0.0f )
	, m_bReturnToOwner( LTFALSE )
	, m_fTurnRate( 0.0f )
	, m_fIncidentAngleToControlLine( 0.0f )
	, m_fIncidentAngleToControlLineDecay( 0.0f )
	, m_fTargetAngleTrackThreshold( 0.0f )
	, m_fLostTargetFlightTime( 0.0f )
	, m_bSentReturnedMsg( false )
	, m_bBlockable( false )
	, m_fDeflectStartTime( 0.0f )
	, m_fTronUnknown990( 0.0f )
	, m_fDeflectEndTime( 0.0f )
	, m_bTronUnknown998( false )
	, m_vDeflectPos( 0.0f, 0.0f, 0.0f )
	, m_vTronUnknown9A8( 0.0f, 0.0f, 0.0f )
	, m_fDeflectTotalTime( 0.0f )
	, m_fDeflectVelocity( 0.0f )
	, m_fDeflectDistance( 0.0f )
	, m_fDeflectInitialRadius( 0.0f )
	, m_fDeflectFinalRadius( 0.0f )
	, m_rDeflectRot()
	, m_bDeflectAdjust( false )
	, m_nDeflectUpdateNum( 0 )
	, m_bAutoAim( false )
	, m_fMagneticRadius( 0.0f )
	, m_fMagneticMiddleInfluence( 0.0f )
	, m_fMagneticEdgeInfluence( 0.0f )
	, m_fMagneticAngleMax( 0.0f )
	, m_nDiscAmmoId( 0 )
	, m_bTronUnknown9F9( false )
	, m_vTronUnknown9FC( 0.0f, 0.0f, 0.0f )
	, m_hLastTouched( LTNULL )
	, m_fTronUnknownA0C( 0.0f )
	, m_fMultiplayerLeashLength( 0.0f )
	, m_vTronUnknownA14( 0.0f, 0.0f, 0.0f )
	, m_dwFlagsAtStart( 0 )
	, m_fLastUpdateTime( 0.0f )
{
	InitDiscConsoleVariables( g_pLTServer );

	ClearDebugInfo();
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::~CDisc()
//
//	PURPOSE:	RETAIL: 0x100373d0.  Tell the owner if it does not know
//				that the disc is back; remove the stimulii.
//
// ----------------------------------------------------------------------- //

CDisc::~CDisc()
{
	if ( !m_bSentReturnedMsg && m_hFiredFrom )
	{
		SendReturnedMessage();
	}

	if ( g_pAIStimulusMgr )
	{
		if ( m_eDiscCatchableStimID != kStimID_Unset )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eDiscCatchableStimID );
			m_eDiscCatchableStimID = kStimID_Unset;
		}
		if ( m_eDiscBlockableStimID != kStimID_Unset )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eDiscBlockableStimID );
			m_eDiscBlockableStimID = kStimID_Unset;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::TronUnknownVirtual28()
//
//	PURPOSE:	RETAIL: 0x100374a0.  A player's disc in a disc game takes
//				its velocity from the "DiscEnergy" ammo
//
// ----------------------------------------------------------------------- //

void CDisc::TronUnknownVirtual28( WeaponFireInfo const &info )
{
	CTronProjectile::TronUnknownVirtual28( info );

	if ( IsPlayer( info.hFiredFrom ) && IsDiscGameType() )
	{
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( "DiscEnergy" );
		if ( pAmmo )
		{
			PROJECTILEFX const *pProjectileFX = pAmmo->pProjectileFX;
			m_fVelocity = static_cast< LTFLOAT >( pProjectileFX ? pProjectileFX->nVelocity : 0 );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::Setup()
//
//	PURPOSE:	Specialize the projectile disc setup (called externally)...
//				RETAIL: 0x10037520
//
// ----------------------------------------------------------------------- //

LTBOOL CDisc::Setup( CWeapon const *pWeapon, WeaponFireInfo const &info )
{
	// setup the base class
	LTBOOL bResult = CTronProjectile::Setup( pWeapon, info );

	m_nDiscAmmoId = static_cast< uint8 >( m_nAmmoId );

	// get the ammo data
	AMMO const *pAmmo;
	if ( IsPlayer( info.hFiredFrom ) && IsDiscGameType() )
	{
		pAmmo = g_pWeaponMgr->GetAmmo( "DiscEnergy" );
		if ( pAmmo )
		{
			m_nDiscAmmoId = static_cast< uint8 >( pAmmo->nId );
		}
	}
	else
	{
		pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
	}

	if ( !pAmmo )
	{
		return LTFALSE;
	}

	// get the projectile
	PROJECTILEFX *pProjectileFX = pAmmo->pProjectileFX;
	if ( !pProjectileFX )
	{
		return LTFALSE;
	}

	// get the projectile class data
	DISCCLASSDATA *pDiscClassData =
		dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
	if ( !pDiscClassData )
	{
		return LTFALSE;
	}

	m_bAutoAim = info.bDiscAutoAim;

	// Save the socket the disc came from and will be returning to.
	m_hReturnToSocket = info.hSocket;

	// keep track of the disc's return velocity
	m_fReturnVelocity = g_vtDiscVelocityReturn.GetFloat() * pDiscClassData->fReturnVelocity;

	// keep track of the disc's return offset
	m_fReturnHeightOffset = pDiscClassData->fReturnHeightOffset;

	m_bReturnToOwner = pDiscClassData->bReturnToOwner;
	if ( !m_bReturnToOwner )
	{
		// RETAIL: a disc that does not return has nothing to tell
		m_bSentReturnedMsg = true;
	}

	// if there is an angle offset specified, set the velocity
	// to start at that initial angle
	if ( DISC_EPSILON < info.fDiscReleaseAngle )
	{
		// get the fire direction
		LTRotation rFireRot( info.vPath, LTVector( 0.0f, 1.0f, 0.0f ) );
		rFireRot.Rotate( LTVector( 0.0f, 1.0f, 0.0f ), info.fDiscReleaseAngle );
		
		// scale to the velocity
		LTVector vOffsetVelocity;
		vOffsetVelocity = rFireRot.Forward();
		vOffsetVelocity *= static_cast< LTFLOAT >( pProjectileFX->nVelocity );

		// set the new offset velocity
		g_pPhysicsLT->SetVelocity( m_hObject, &vOffsetVelocity );

		m_vDir = vOffsetVelocity;
	}

	switch ( info.nDiscTrackingType )
	{
		case MPROJ_DISC_TRACKING_STEADY:
		{
			//
			// disc flys straight with no special tracking
			//

			// start assuming the disc is flying towards a target
			m_eDiscState = DISC_STATE_FLYING;

			// set the tracking state
			m_eTrackingMode = DISC_TRACKING_MODE_STEADY;

			// style doesn't apply
			m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT;
		}
		break;

		case MPROJ_DISC_TRACKING_POINT_GUIDED:
		{
			//
			// disc will change its flight path to hit a point
			//

			// start assuming the disc is flying towards a target
			m_eDiscState = DISC_STATE_FLYING;

			// set the tracking state
			m_eTrackingMode = DISC_TRACKING_MODE_POINT_GUIDED;

			// style is straight for point
			m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET;

			// get the initial target
			m_vPointTarget = info.vPointTarget;

			// remember to use it during our next update
			m_bNewTarget = true;
		}
		break;

		case MPROJ_DISC_TRACKING_CONTROL_LINE:
		{
			//
			// disc will try to get to a control line specified by owner
			//

			// start assuming the disc is flying towards a target
			m_eDiscState = DISC_STATE_FLYING;

			// set the tracking state
			m_eTrackingMode = DISC_TRACKING_MODE_CONTROL_LINE;

			// set the style of disc tracking
			m_eTrackingStyle = DISC_TRACKING_STYLE_CONTROL_LINE;

			// get the initial control direction
			m_vControlDirection = info.vControlDirection;

			// get the initial target
			m_vControlPosition = info.vControlPosition;

			// normalize the control direction
			m_vControlDirection.Normalize();

			// remember to use it during our next update
			m_bNewTarget = true;

			m_fTurnRate = pDiscClassData->fTurnRate;
			m_fIncidentAngleToControlLine = pDiscClassData->fIncidentAngleToControlLine;
			m_fIncidentAngleToControlLineDecay = pDiscClassData->fIncidentAngleToControlLineDecay;
		}
		break;

		case MPROJ_DISC_TRACKING_HOMING:
		{
			//
			// disc will change its flight path to hit an object
			//

			// start assuming the disc is flying towards a target
			m_eDiscState = DISC_STATE_FLYING;

			// set the tracking state
			m_eTrackingMode = DISC_TRACKING_MODE_HOMING;

			// style is straight for the target
			m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET;

			// get the initial target
			m_hObjectTarget = info.hObjectTarget;

			// remember to use it during our next update
			m_bNewTarget = true;

			m_fTurnRate = pDiscClassData->fTurnRate;
			m_fIncidentAngleToControlLine = pDiscClassData->fIncidentAngleToControlLine;
			m_fIncidentAngleToControlLineDecay = pDiscClassData->fIncidentAngleToControlLineDecay;
			m_fTargetAngleTrackThreshold = static_cast< float >( cos( pDiscClassData->fTargetAngleTrackThreshold ) );
			m_fLostTargetFlightTime = pDiscClassData->fLostTargetFlightTime;
		}
		break;
	};

	// RETAIL: the weapon's modifiers scale the turn rates
	if ( !_stricmp( "Sequencer", m_pWeaponData->szName ) && m_aModifiers[ 0 ].bEnabled )
	{
		m_fTurnRate *= m_ModifierData.m0.fTronUnknown[ 0 ];
		m_fIncidentAngleToControlLine *= m_ModifierData.m0.fTronUnknown[ 0 ];
		m_fIncidentAngleToControlLineDecay *= m_ModifierData.m0.fTronUnknown[ 0 ];
	}
	else if ( !_stricmp( "Cluster", m_pWeaponData->szName ) && m_aModifiers[ 1 ].bEnabled )
	{
		m_fTurnRate *= m_ModifierData.m1.fTronUnknown[ 33 ];
		m_fIncidentAngleToControlLine *= m_ModifierData.m1.fTronUnknown[ 33 ];
		m_fIncidentAngleToControlLineDecay *= m_ModifierData.m1.fTronUnknown[ 33 ];
	}

	// the deflection
	m_fDeflectTotalTime = IsDiscGameType() ? pDiscClassData->fMPDeflectTotalTime : pDiscClassData->fDeflectTotalTime;
	m_fDeflectTotalTime *= g_vtDiscDeflectTime.GetFloat();
	m_fDeflectVelocity = g_vtDiscDeflectVelocity.GetFloat() * pDiscClassData->fDeflectVelocity;
	m_fDeflectDistance = pDiscClassData->fDeflectDistance;
	m_fDeflectInitialRadius = pDiscClassData->fDeflectInitialRadius;
	m_fDeflectFinalRadius = pDiscClassData->fDeflectFinalRadius;

	float fRadiusChange = m_fDeflectFinalRadius - m_fDeflectInitialRadius;
	if ( fRadiusChange < 0.00001f )
	{
		m_fDeflectAngularSpeed = FLT_MAX;
	}
	else
	{
		m_fDeflectAngularSpeed = m_fDeflectVelocity * m_fDeflectTotalTime / fRadiusChange;
	}

	m_fMagneticRadius = pDiscClassData->fMagneticRadius;
	m_fMagneticMiddleInfluence = pDiscClassData->fMagneticMiddleInfluence;
	m_fMagneticEdgeInfluence = pDiscClassData->fMagneticEdgeInfluence;
	m_fMagneticAngleMax = pDiscClassData->fMagneticAngleMax;

	m_bDeflectAdjust = false;
	m_nDeflectUpdateNum = 0;

	m_fMultiplayerLeashLength = pDiscClassData->fMultiplayerLeashLength;

	// we want messages, tell who shot us to send them along
	RequestStartSendingMessages();

	// Turn on the discs blockablity
	StartBlockable();

	return bResult;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::EngineMessageFn()
//
//	PURPOSE:	Override the default projecile engine message behaviour...
//				RETAIL: 0x10037c40
//
// ----------------------------------------------------------------------- //

uint32 CDisc::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch( messageID )
	{
		case MID_UPDATE:
		{
			// this is a new update
			m_nUpdateNum++;

			uint32 dwRet = CTronProjectile::EngineMessageFn( messageID, pData, fData );

			UpdateDisc();

			m_fLastUpdateTime = g_pLTServer->GetTime();

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			m_fLastUpdateTime = g_pLTServer->GetTime();

			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FULLPOSITIONRES, FLAG_FULLPOSITIONRES | FLAG_RAYHIT );
			g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, m_dwFlagsAtStart );

			m_fStartTime = g_pLTServer->GetTime();
			m_fLifeTime = 1000.0f;
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save( ( ILTMessage_Write* )pData, ( uint32 )fData );
		}
		break;

		case MID_LOADOBJECT:
		{
			Load( ( ILTMessage_Read* )pData, ( uint32 )fData );
		}
		break;
	}

	return CTronProjectile::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::ObjectMessageFn()
//
//	PURPOSE:	Handle object messages
//				RETAIL: 0x10037d90
//
// ----------------------------------------------------------------------- //

uint32 CDisc::ObjectMessageFn( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	pMsg->SeekTo( 0 );
	uint32 messageID = pMsg->Readuint32();

	if ( MID_PROJECTILE == messageID )
	{
		uint8 nProjectileMessage = pMsg->Readuint8();
		switch ( nProjectileMessage )
		{
			case MPROJ_UPDATE_POINT_TARGET:
			{
				m_vPointTarget = pMsg->ReadLTVector();
				m_bNewTarget = true;
			}
			break;

			case MPROJ_UPDATE_OBJECT_TARGET:
			{
				m_hObjectTarget = pMsg->ReadObject();
				m_bNewTarget = true;
			}
			break;

			case MPROJ_UPDATE_CONTROL_LINE:
			{
				m_vControlDirection = pMsg->ReadLTVector();
				m_vControlPosition = pMsg->ReadLTVector();
				m_vControlDirection.Normalize();
				m_bNewTarget = true;
			}
			break;

			case MPROJ_RETURN:
			{
				if ( !IsDiscDeflecting() )
				{
					EnterReturnMode( LTNULL );
				}
			}
			break;

			case MPROJ_DISC_TRONUNKNOWN14:
			{
				// RETAIL: (0x10037f19) the owner stops guiding the disc
				if ( !IsDiscDeflecting() )
				{
					m_eTrackingMode = DISC_TRACKING_MODE_STEADY;
					m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT;
				}
			}
			break;

			case MPROJ_DISC_TRONUNKNOWN150:
			{
				// RETAIL: (0x10037f37)
				DestroyInFlight();
				SendReturnedMessage();
			}
			break;
		}
	}

	return CTronProjectile::ObjectMessageFn( hSender, pMsg );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::Save()
//
//	PURPOSE:	RETAIL: 0x100380f0
//
// ----------------------------------------------------------------------- //

void CDisc::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if ( !pMsg ) return;

	SAVE_INT( m_eDiscState );
	SAVE_INT( m_eTrackingMode );
	SAVE_INT( m_eTrackingStyle );
	SAVE_bool( m_bNewTarget );
	SAVE_VECTOR( m_vPointTarget );
	SAVE_HOBJECT( m_hObjectTarget );
	SAVE_VECTOR( m_vControlDirection );
	SAVE_VECTOR( m_vControlPosition );
	SAVE_DWORD( m_hReturnToSocket );
	SAVE_DWORD( m_eDiscCatchableStimID );
	SAVE_DWORD( m_eDiscBlockableStimID );
	SAVE_FLOAT( m_fReturnVelocity );
	SAVE_FLOAT( m_fReturnHeightOffset );
	SAVE_BOOL( m_bReturnToOwner );
	SAVE_FLOAT( m_fTurnRate );
	SAVE_FLOAT( m_fIncidentAngleToControlLine );
	SAVE_FLOAT( m_fIncidentAngleToControlLineDecay );
	SAVE_FLOAT( m_fTargetAngleTrackThreshold );
	SAVE_FLOAT( m_fLostTargetFlightTime );
	SAVE_bool( m_bSentReturnedMsg );
	SAVE_bool( m_bBlockable );
	SAVE_FLOAT( m_fDeflectStartTime - g_pLTServer->GetTime() );
	SAVE_TIME( m_fDeflectEndTime );
	SAVE_FLOAT( m_fTronUnknown990 );
	SAVE_bool( m_bTronUnknown998 );
	SAVE_VECTOR( m_vDeflectPos );
	SAVE_VECTOR( m_vTronUnknown9A8 );
	SAVE_FLOAT( m_fDeflectTotalTime );
	SAVE_FLOAT( m_fDeflectVelocity );
	SAVE_FLOAT( m_fDeflectDistance );
	SAVE_FLOAT( m_fDeflectInitialRadius );
	SAVE_FLOAT( m_fDeflectFinalRadius );
	SAVE_FLOAT( m_fDeflectAngularSpeed );
	SAVE_ROTATION( m_rDeflectRot );
	SAVE_bool( m_bDeflectAdjust );
	SAVE_INT( m_nDeflectUpdateNum );
	SAVE_bool( m_bAutoAim );
	SAVE_FLOAT( m_fMagneticRadius );
	SAVE_FLOAT( m_fMagneticMiddleInfluence );
	SAVE_FLOAT( m_fMagneticEdgeInfluence );
	SAVE_FLOAT( m_fMagneticAngleMax );
	SAVE_BYTE( m_nDiscAmmoId );
	SAVE_bool( m_bTronUnknown9F9 );
	pMsg->WriteType( m_vTronUnknown9FC );
	SAVE_HOBJECT( m_hLastTouched );
	SAVE_FLOAT( m_fTronUnknownA0C );
	SAVE_FLOAT( m_fMultiplayerLeashLength );
	pMsg->WriteType( m_vTronUnknownA14 );
	SAVE_DWORD( m_dwFlagsAtStart );
	SAVE_TIME( m_fLastUpdateTime );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::Load()
//
//	PURPOSE:	RETAIL: 0x10038520
//
// ----------------------------------------------------------------------- //

void CDisc::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if ( !pMsg ) return;

	LOAD_INT( m_eDiscState );
	LOAD_INT( m_eTrackingMode );
	LOAD_INT( m_eTrackingStyle );
	LOAD_bool( m_bNewTarget );
	LOAD_VECTOR( m_vPointTarget );
	LOAD_HOBJECT( m_hObjectTarget );
	LOAD_VECTOR( m_vControlDirection );
	LOAD_VECTOR( m_vControlPosition );
	LOAD_DWORD_CAST( m_hReturnToSocket, HMODELSOCKET );
	LOAD_DWORD_CAST( m_eDiscCatchableStimID, EnumAIStimulusID );
	LOAD_DWORD_CAST( m_eDiscBlockableStimID, EnumAIStimulusID );
	LOAD_FLOAT( m_fReturnVelocity );
	LOAD_FLOAT( m_fReturnHeightOffset );
	LOAD_BOOL( m_bReturnToOwner );
	LOAD_FLOAT( m_fTurnRate );
	LOAD_FLOAT( m_fIncidentAngleToControlLine );
	LOAD_FLOAT( m_fIncidentAngleToControlLineDecay );
	LOAD_FLOAT( m_fTargetAngleTrackThreshold );
	LOAD_FLOAT( m_fLostTargetFlightTime );
	LOAD_bool( m_bSentReturnedMsg );
	LOAD_bool( m_bBlockable );
	LOAD_TIME( m_fDeflectStartTime );
	LOAD_TIME( m_fDeflectEndTime );
	LOAD_FLOAT( m_fTronUnknown990 );
	LOAD_bool( m_bTronUnknown998 );
	LOAD_VECTOR( m_vDeflectPos );
	LOAD_VECTOR( m_vTronUnknown9A8 );
	LOAD_FLOAT( m_fDeflectTotalTime );
	LOAD_FLOAT( m_fDeflectVelocity );
	LOAD_FLOAT( m_fDeflectDistance );
	LOAD_FLOAT( m_fDeflectInitialRadius );
	LOAD_FLOAT( m_fDeflectFinalRadius );
	LOAD_FLOAT( m_fDeflectAngularSpeed );
	LOAD_ROTATION( m_rDeflectRot );
	LOAD_bool( m_bDeflectAdjust );
	LOAD_INT( m_nDeflectUpdateNum );
	LOAD_bool( m_bAutoAim );
	LOAD_FLOAT( m_fMagneticRadius );
	LOAD_FLOAT( m_fMagneticMiddleInfluence );
	LOAD_FLOAT( m_fMagneticEdgeInfluence );
	LOAD_FLOAT( m_fMagneticAngleMax );
	LOAD_BYTE( m_nDiscAmmoId );
	LOAD_bool( m_bTronUnknown9F9 );
	LOAD_VECTOR( m_vTronUnknown9FC );
	LOAD_HOBJECT( m_hLastTouched );
	LOAD_FLOAT( m_fTronUnknownA0C );
	LOAD_FLOAT( m_fMultiplayerLeashLength );
	LOAD_VECTOR( m_vTronUnknownA14 );
	LOAD_DWORD( m_dwFlagsAtStart );
	LOAD_TIME( m_fLastUpdateTime );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::TronUnknownVirtual31()
//
//	PURPOSE:	RETAIL: 0x10038f60
//
// ----------------------------------------------------------------------- //

bool CDisc::TronUnknownVirtual31( HOBJECT hAttacker, HOBJECT hVictim )
{
	if ( DISC_STATE_DEADLY_DEFLECTING == m_eDiscState )
	{
		return true;
	}

	return CTronProjectile::TronUnknownVirtual31( hAttacker, hVictim );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::TronUnknownVirtual32()
//
//	PURPOSE:	RETAIL: 0x1003ae60.  With no touch kept, a disc whose
//				touch was not taken goes back
//
// ----------------------------------------------------------------------- //

void CDisc::TronUnknownVirtual32()
{
	if ( !GetNearestPendingTouch( LTNULL ) )
	{
		if ( m_bTronUnknown1C2 )
		{
			return;
		}

		if ( m_bTronUnknown1C1 )
		{
			EnterReturnMode( LTNULL );
			return;
		}
	}

	CTronProjectile::TronUnknownVirtual32();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleDeadlyDeflectTouch()
//
//	PURPOSE:	RETAIL: 0x1003aea0: a deadly deflected disc smites the
//				characters, their hit boxes and the finders it touches
//				(once each); other solid objects end the deadly
//				deflection.  Returns true when it smites.  The name is
//				ours
//
// ----------------------------------------------------------------------- //

bool CDisc::HandleDeadlyDeflectTouch( ProjectilePendingTouch *pTouch )
{
	HOBJECT hObj = pTouch->hObject;
	if ( IsCharacter( hObj ) )
	{
		CCharacter *pCharacter = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( hObj ) );
		hObj = pCharacter->GetHitBoxObject();
	}

	HOBJECT hLastHitBox = LTNULL;

	static HCLASS s_hFinderClass = g_pLTServer->GetClass( "Finder" );

	HCLASS hClass = g_pLTServer->GetObjectClass( hObj );
	if ( IsCharacter( m_hLastTouched ) )
	{
		CCharacter *pCharacter = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( m_hLastTouched ) );
		hLastHitBox = pCharacter->GetHitBoxObject();
	}

	if ( ( hObj == m_hLastTouched ) || ( hObj == hLastHitBox ) )
	{
		return false;
	}

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( hObj, OFT_Flags, dwFlags );

	bool bHitBox = ( IsCharacterHitBox( hObj ) > 0 );
	if ( !( dwFlags & FLAG_SOLID ) )
	{
		if ( !bHitBox )
		{
			return false;
		}
	}
	else if ( !bHitBox && !IsCharacter( hObj ) && !g_pLTServer->IsKindOf( hClass, s_hFinderClass ) )
	{
		m_eDiscState = DISC_STATE_DEFLECTING;
		return false;
	}

	SmiteObject( hObj );
	m_hLastTouched = LTNULL;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::TronUnknownVirtual33()
//
//	PURPOSE:	RETAIL: slot 33 (0x1003b010): the new collision code's
//				impact of the disc.  The owner catches a returning disc,
//				the disc ricochets off the world and is blocked (2) or
//				deadly blocked (3) by a defending character
//
// ----------------------------------------------------------------------- //

// RETAIL: the radius of the stimuli of a blocked disc (0x102946cc).  The
// name is ours
static LTFLOAT s_fDiscBlockedStimulusRadius = 2000.0f;

bool CDisc::TronUnknownVirtual33( ProjectilePendingTouch *pTouch )
{
	bool bResult = CTronProjectile::TronUnknownVirtual33( pTouch );

	// Check to see if our creator is still here before doing this -- we
	// don't want to do anything if they're not
	if ( !m_hFiredFrom )
	{
		return bResult;
	}

	SurfaceType eSurfaceType = GetSurfaceType( pTouch->collInfo );
	if ( ST_INVISIBLE == eSurfaceType )
	{
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags( pTouch->hObject, OFT_Flags, dwFlags );
		if ( dwFlags & FLAG_SOLID )
		{
			return bResult;
		}
	}

	bool bTouchTaken = false;

	if ( DISC_STATE_RETURNING == m_eDiscState )
	{
		if ( IsMultiplayerGame() )
		{
			if ( IsAI( pTouch->hObject ) || IsMainWorld( pTouch->hObject ) )
			{
				// go through the world and the AIs on the way back
				g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags,
					FLAG_GOTHRUWORLD | FLAG_FULLPOSITIONRES,
					FLAG_REMOVEIFOUTSIDE | FLAG_GOTHRUWORLD | FLAG_STAIRSTEP | FLAG_GRAVITY | FLAG_SOLID | FLAG_FULLPOSITIONRES );
			}
			else if ( pTouch->hObject == m_hFiredFrom )
			{
				// the disc is back with its owner
				SendReturnedMessage();
				return true;
			}
		}
		else
		{
			if ( pTouch->hObject == m_hFiredFrom )
			{
				// the disc is back with its owner
				SendReturnedMessage();
				return true;
			}

			if ( IsCharacter( pTouch->hObject ) )
			{
				CCharacter *pCharacter = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( pTouch->hObject ) );
				if ( pCharacter->IsDefending() )
				{
					HandleDeflect();

					// tell the character that it blocked the disc
					CAutoMessage cMsg;
					cMsg.Writeuint32( MID_PROJECTILE );
					cMsg.Writeuint8( MPROJ_BLOCKED );
					cMsg.Writeuint8( IMPACT_TYPE_BLOCKED );
					g_pLTServer->SendToObject( cMsg.Read(), m_hObject, pTouch->hObject, MESSAGE_GUARANTEED );
				}
			}
		}
	}
	else if ( DISC_STATE_DEADLY_DEFLECTING == m_eDiscState )
	{
		bTouchTaken = HandleDeadlyDeflectTouch( pTouch );
	}

	LTVector vPos = pTouch->iInfo.m_Point;

	int nImpactType = IMPACT_TYPE_IMPACT;

	LTVector vNormal( 0.0f, 1.0f, 0.0f );

	if ( pTouch->hObject )
	{
		if ( IsCharacter( pTouch->hObject ) )
		{
			CCharacter *pCharacter = dynamic_cast< CCharacter * >(
					g_pLTServer->HandleToObject( pTouch->hObject )
				);

			if ( pCharacter->IsDefending() && IsBlockable() )
			{
				if ( IsPlayer( pTouch->hObject ) )
				{
					CTronPlayerObj *pPlayer = dynamic_cast< CTronPlayerObj* >( pCharacter );
					if ( pPlayer->GetDefensePercentage( &m_vDir ) > 0.95f )
					{
						PlayerSubroutineSlot *pSlot = pPlayer->FindSubroutine( "Shield_prism" );
						if ( ( pSlot && pSlot->nTronUnknown40 ) || IsDiscGameType() )
						{
							nImpactType = DISC_IMPACT_TYPE_DEADLY_BLOCKED;
						}
						else
						{
							nImpactType = IMPACT_TYPE_BLOCKED;
						}
					}
				}
				else
				{
					nImpactType = IMPACT_TYPE_BLOCKED;
				}
			}
		}
		else if ( dynamic_cast< ShellArmor* >( g_pLTServer->HandleToObject( pTouch->hObject ) ) )
		{
			nImpactType = IMPACT_TYPE_BLOCKED;
		}

		if ( ( IsMainWorld( pTouch->hObject ) || ( GetObjectType( pTouch->hObject ) == OT_WORLDMODEL ) ) &&
		     ( m_fMaxRicochetAngle > 0.0f ) &&
		     ( m_nMaxRicochets > m_nTotalRicochets ) )
		{
			LTVector vDir( pTouch->vLastVel.x, pTouch->vLastVel.y, pTouch->vLastVel.z );
			vDir.Normalize();

			vPos = pTouch->iInfo.m_Point - vDir;
			eSurfaceType = GetSurfaceType( pTouch->iInfo );
			vNormal = pTouch->iInfo.m_Plane.m_Normal;

			LTFLOAT fAngleFromNormal;
			LTVector vReversedNormal = -vNormal;
			fAngleFromNormal = ltacosf( VEC_DOT( vReversedNormal, vDir ) );

			if ( ( MATH_HALFPI - fAngleFromNormal ) < m_fMaxRicochetAngle )
			{
				// ricochet the disc

				// increase the ricochet count
				m_nTotalRicochets += 1;

				// reflect the direction
				LTVector vReflectedDir;
				vReflectedDir = vDir - 2 * vNormal * ( VEC_DOT( vDir, vNormal ) );

				AMMO const *pAmmo;
				if ( IsPlayer( m_hFiredFrom ) && IsDiscGameType() )
				{
					pAmmo = g_pWeaponMgr->GetAmmo( "DiscEnergy" );
				}
				else
				{
					pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
				}

				if ( !pAmmo )
				{
					return bResult;
				}

				// normalize and scale to velocity
				LTVector vNewVel = vReflectedDir;
				vNewVel *= static_cast< LTFLOAT >( pAmmo->pProjectileFX->nVelocity );

				// keep track of the update num to prevent multiple ricochets in the same frame
				m_nLastRicochetUpdateNum = m_nUpdateNum;

				// set the new speed
				g_pPhysicsLT->SetVelocity( m_hObject, &vNewVel );

				m_vFirePos = vPos;
				m_vDir = vReflectedDir;

				// do all the impact work
				AddImpact( pTouch->hObject, m_vFlashPos, vPos, vNormal, eSurfaceType, IMPACT_TYPE_RICOCHET );

				if ( !bTouchTaken && pTouch->hObject && IsBlockable() )
				{
					ImpactDamageObject( m_hFiredFrom, pTouch->hObject );
				}

				// RETAIL: debug output that the retail build keeps (it is
				// not printed)
				LTVector vDims;
				g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

				LTFLOAT fDimsXY = ( LTFLOAT )sqrt( vDims.x * vDims.x + vDims.y * vDims.y );
				LTFLOAT fMaxDim = ( LTFLOAT )sqrt( fDimsXY * fDimsXY + vDims.z * vDims.z );

				uint32 dwFlags;
				g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );

				char szDebug[128];
				sprintf( szDebug, "flags: 0x%08x,max dim: %f,pt: %5.3f %5.3f %5.3f,vDir: %5.3f %5.3f %5.3f",
					dwFlags, fMaxDim,
					pTouch->iInfo.m_Point.x, pTouch->iInfo.m_Point.y, pTouch->iInfo.m_Point.z,
					vDir.x, vDir.y, vDir.z );

				// move back out of the world by the disc's size
				LTVector vNewPos = -vDir * fMaxDim + pTouch->iInfo.m_Point;
				g_pLTServer->TeleportObject( m_hObject, &vNewPos );

				LTVector vNewestPos;
				g_pLTServer->GetObjectPos( m_hObject, &vNewestPos );
				sprintf( szDebug, "--newest pos: %5.3f %5.3f %5.3f", vNewestPos.x, vNewestPos.y, vNewestPos.z );

				return bResult;
			}
		}
	}
	else
	{
		eSurfaceType = ST_AIR;
	}

	if ( eSurfaceType == ST_UNKNOWN )
	{
		eSurfaceType = GetSurfaceType( pTouch->hObject );
	}

	AddImpact( pTouch->hObject, m_vFlashPos, vPos, vNormal, eSurfaceType, ( IMPACT_TYPE )nImpactType, false );

	// Do the damage
	if ( pTouch->hObject && IsBlockable() )
	{
		bool bBlocked = ( ( IMPACT_TYPE_BLOCKED == nImpactType ) || ( DISC_IMPACT_TYPE_DEADLY_BLOCKED == nImpactType ) );

		// RETAIL: a blocked disc damages only players
		if ( !bTouchTaken && ( !bBlocked || IsPlayer( pTouch->hObject ) ) )
		{
			HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : LTObjRef( m_hObject );
			ImpactDamageObject( hDamager, pTouch->hObject );
		}
	}

	if ( IsDiscReturning() || IsDiscDeflecting() )
	{
		return bResult;
	}

	if ( ( IMPACT_TYPE_BLOCKED != nImpactType ) && ( DISC_IMPACT_TYPE_DEADLY_BLOCKED != nImpactType ) )
	{
		if ( m_bReturnToOwner )
		{
			EnterReturnMode( pTouch->hObject );
		}
		else if ( !pTouch->hObject )
		{
			DestroyInFlight();
		}
		else
		{
			RemoveObject();
		}

		return bResult;
	}

	// the AIs hear the block
	if ( pTouch->hObject && IsCharacter( pTouch->hObject ) )
	{
		CCharacter *pCharacter = ( CCharacter* )g_pLTServer->HandleToObject( pTouch->hObject );
		if ( pCharacter )
		{
			g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyWeaponFireSound, pCharacter->GetUserObject(),
				pCharacter->GetRelationData(), pTouch->vLastPos, s_fDiscBlockedStimulusRadius, 1.0f );
			g_pAIStimulusMgr->RegisterStimulus( kStim_AllyWeaponFireSound, pCharacter->GetUserObject(),
				pCharacter->GetRelationData(), pTouch->vLastPos, s_fDiscBlockedStimulusRadius, 1.0f );
			g_pAIStimulusMgr->RegisterStimulus( kStim_ProhibitedWeaponFireSound, pCharacter->GetUserObject(),
				pCharacter->GetRelationData(), pTouch->vLastPos, s_fDiscBlockedStimulusRadius, 1.0f );
		}
	}

	if ( DISC_IMPACT_TYPE_DEADLY_BLOCKED == nImpactType )
	{
		HandleDeadlyDeflect( pTouch->hObject );
	}
	else
	{
		HandleDeflect();
	}

	// tell the character that it blocked the disc
	CAutoMessage cMsg;
	cMsg.Writeuint32( MID_PROJECTILE );
	cMsg.Writeuint8( MPROJ_BLOCKED );
	cMsg.Writeuint8( nImpactType );
	g_pLTServer->SendToObject( cMsg.Read(), m_hObject, pTouch->hObject, MESSAGE_GUARANTEED );

	return bResult;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::AddImpact()
//
//	PURPOSE:	RETAIL: 0x10038010
//
// ----------------------------------------------------------------------- //

void CDisc::AddImpact( HOBJECT hObj, const LTVector &vFirePos,
                       const LTVector &vImpactPos, const LTVector &vNormal,
                       SurfaceType eSurfaceType, IMPACT_TYPE eImpactType,
                       bool bDeflected )
{
	if ( !m_bSetup ) return;

	if ( IsDamagedObject( hObj ) ) return;

	if ( IsCharacter( hObj ) )
	{
		IHitBoxUser *pHitBoxUser = dynamic_cast< IHitBoxUser* >( g_pLTServer->HandleToObject( hObj ) );
		if ( pHitBoxUser && IsDamagedObject( pHitBoxUser->GetHitBoxObject() ) ) return;
	}

	if ( IsCharacterHitBox( hObj ) )
	{
		CCharacterHitBox *pHitBox = dynamic_cast< CCharacterHitBox* >( g_pLTServer->HandleToObject( hObj ) );
		if ( pHitBox && IsDamagedObject( pHitBox->GetModelObject() ) ) return;
	}

	CTronProjectile::AddImpact( hObj, vFirePos, vImpactPos, vNormal, eSurfaceType, eImpactType, bDeflected );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::UpdateDisc()
//
//	PURPOSE:	Update the disc (called every frame)
//				RETAIL: 0x10038f80
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateDisc()
{
	// Check to see if our creator is still here before doing this
	if ( !m_hFiredFrom )
	{
		HandleOwnerDeath();
		return false;
	}

	// RETAIL: a disc that is not at a finite position goes back
	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	if ( !_finite( vPos.x ) || !_finite( vPos.y ) || !_finite( vPos.z ) )
	{
		EnterReturnMode( LTNULL );
	}

	// update as fast as possible
	SetNextUpdate( CDISC_UPDATE_DELTA );

	switch ( m_eDiscState )
	{
		case DISC_STATE_FLYING:
		{
			return UpdateStateFlying();
		}
		break;

		case DISC_STATE_DEFLECTING:
		case DISC_STATE_DEADLY_DEFLECTING:
		{
			return UpdateStateDeflecting();
		}
		break;

		case DISC_STATE_RETURNING:
		{
			return UpdateStateReturning();
		}
		break;
	};

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleOwnerDeath()
//
//	PURPOSE:	RETAIL: 0x10039060
//
// ----------------------------------------------------------------------- //

void CDisc::HandleOwnerDeath()
{
	DestroyInFlight();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleTouchSky()
//
//	PURPOSE:	RETAIL: 0x10039070
//
// ----------------------------------------------------------------------- //

void CDisc::HandleTouchSky( HOBJECT hObj )
{
	if ( m_bReturnToOwner )
	{
		EnterReturnMode( LTNULL );
	}
	else
	{
		DestroyInFlight();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::EnterReturnMode()
//
//	PURPOSE:	Change the state to return mode
//				RETAIL: 0x100399b0
//
// ----------------------------------------------------------------------- //

void CDisc::EnterReturnMode( HOBJECT hObj )
{
	if ( IsDiscReturning() )
	{
		return;
	}

	if ( !m_bReturnToOwner )
	{
		DestroyInFlight();
		return;
	}

	// The disc can be caught when it returns
	if ( m_pRelationData && ( m_eDiscCatchableStimID == kStimID_Unset ) )
	{
		m_eDiscCatchableStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_CatchableProjectileVisible,
			m_hFiredFrom, m_hObject, *m_pRelationData, CAIStimulusRecord::kDynamicPos_TrackTarget, LTVector( 0.0f, 0.0f, 0.0f ), 1.0f );
	}

	if ( !hObj )
	{
		DISCCLASSDATA *pDiscClassData =
			dynamic_cast< DISCCLASSDATA* >( m_pAmmoData->pProjectileFX->pClassData );

		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );

		if ( pDiscClassData->szReturnFXName[ 0 ] )
		{
			PlayClientFX( pDiscClassData->szReturnFXName, LTNULL, &vPos, LTNULL, 0 );
		}
	}

	if ( IsDiscDeflecting() )
	{
		PROJECTILEFX const *pProjectileFX = m_pAmmoData->pProjectileFX;
		if ( pProjectileFX )
		{
			char const *szFX = pProjectileFX->szFXName;
			if ( szFX )
			{
				// RETAIL: stop the deflect FX on the clients
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_SFX_MESSAGE );
				cMsg.Writeuint8( SFX_PROJECTILE_ID );
				cMsg.WriteObject( m_hObject );
				cMsg.Writeuint8( 1 );
				cMsg.WriteString( szFX );
				g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
			}
		}
	}

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags,
		FLAG_GOTHRUWORLD | FLAG_FULLPOSITIONRES,
		FLAG_REMOVEIFOUTSIDE | FLAG_GOTHRUWORLD | FLAG_STAIRSTEP | FLAG_GRAVITY | FLAG_SOLID | FLAG_FULLPOSITIONRES );

	// done sending the messages
	RequestStopSendingMessages();

	m_eDiscState = DISC_STATE_RETURNING;

	g_pLTServer->GetObjectPos( m_hObject, &m_vTronUnknownA14 );

	if ( IsCharacter( m_hFiredFrom ) && !IsPlayer( m_hFiredFrom ) )
	{
		StopBlockable();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleDeadlyDeflect()
//
//	PURPOSE:	Send the disc back from the player that made a deadly
//				block, along the player's view, with the Shield_prism
//				subroutine's damage and velocity
//				RETAIL: 0x10039c30 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CDisc::HandleDeadlyDeflect( HOBJECT hBlocker )
{
	if ( IsDiscDeflecting() )
	{
		return;
	}

	m_eDiscState = DISC_STATE_DEADLY_DEFLECTING;
	m_fTronUnknown990 = m_fDeflectTotalTime;
	m_fDeflectStartTime = g_pLTServer->GetTime();
	m_bTronUnknown998 = true;
	m_nDeflectUpdateNum = m_nUpdateNum;

	// RETAIL: the deflected disc may ricochet at any angle, any number of
	// times
	m_fMaxRicochetAngle = MATH_PI;
	m_nMaxRicochets = 0x7fffffff;

	// the disc may now hit the player that threw it
	m_hLastTouched = hBlocker;
	m_bCanTouchFiredFromObj = true;

	DISCCLASSDATA *pDiscClassData;
	if ( !m_pAmmoData )
	{
		pDiscClassData = LTNULL;
	}
	else
	{
		pDiscClassData = dynamic_cast< DISCCLASSDATA* >( m_pAmmoData->pProjectileFX->pClassData );
	}

	char *szFX = pDiscClassData->szDeadlyDeflectAttachedFXName;
	if ( szFX[ 0 ] )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_PROJECTILE_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( 1 );
		cMsg.WriteString( szFX );
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	}

	CTronPlayerObj *pPlayer = ( CTronPlayerObj* )g_pLTServer->HandleToObject( hBlocker );

	// the shield prism's data (a disc game uses version 2, state 0)
	SubroutineDataKey key( 0, 0, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0 );
	if ( !IsDiscGameType() )
	{
		PlayerSubroutineSlot *pSlot = pPlayer->FindSubroutine( "Shield_prism" );
		key = SubroutineDataKey( pSlot->nTronUnknown42, pSlot->nTronUnknown41, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0 );
	}
	else
	{
		key = SubroutineDataKey( 2, 0, g_pGameServerShell ? g_pGameServerShell->GetDifficulty() : 0 );
	}

	const ShieldPrismData &data = g_pSubroutineDataMgr->GetShieldPrismData( key );
	m_fTronUnknownA0C = GetRandom( data.fDamageMin, data.fDamageMax );

	// send the disc along the player's view
	LTRotation rRot = pPlayer->GetRotationWithPitch();

	LTVector vVel = rRot.Forward() * data.fVelocity;
	g_pPhysicsLT->SetVelocity( m_hObject, &vVel );

	m_vDir = rRot.Forward();

	// start it at the player's camera
	LTVector vPos;
	g_pLTServer->GetObjectPos( hBlocker, &vPos );
	LTVector vNewPos = vPos + pPlayer->GetClientCameraOffset();
	g_pLTServer->TeleportObject( m_hObject, &vNewPos );
	m_vFirePos = vNewPos;

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, m_dwFlagsAtStart, FLAGMASK_ALL );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleDeflect()
//
//	PURPOSE:	Start the deflection
//				RETAIL: 0x1003a0a0
//
// ----------------------------------------------------------------------- //

void CDisc::HandleDeflect()
{
	if ( IsDiscDeflecting() )
	{
		return;
	}

	StopBlockable();

	DISCCLASSDATA *pDiscClassData;
	if ( !m_pAmmoData )
	{
		pDiscClassData = LTNULL;
	}
	else
	{
		pDiscClassData = dynamic_cast< DISCCLASSDATA* >( m_pAmmoData->pProjectileFX->pClassData );
	}

	m_eDiscState = DISC_STATE_DEFLECTING;
	m_fTronUnknown990 = m_fDeflectTotalTime;
	m_fDeflectStartTime = g_pLTServer->GetTime();
	m_bTronUnknown998 = true;

	g_pLTServer->GetObjectRotation( m_hObject, &m_rDeflectRot );
	LTVector vPos = m_rDeflectRot.Forward();
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	m_vDeflectPos = vPos;
	m_vDeflectPos.y += m_fDeflectInitialRadius;

	g_pPhysicsLT->GetVelocity( m_hObject, &m_vTronUnknown9A8 );
	LTVector vVel = -m_vTronUnknown9A8;
	g_pPhysicsLT->SetVelocity( m_hObject, &vVel );

	char *szFX = pDiscClassData->szDeflectAttachedFXName;

	m_eDiscState = DISC_STATE_DEFLECTING;
	m_nDeflectUpdateNum = m_nUpdateNum;

	// RETAIL: the deflected disc may ricochet at any angle, any number of
	// times
	m_fMaxRicochetAngle = MATH_PI;
	m_nMaxRicochets = 0x7fffffff;

	if ( szFX[ 0 ] )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_PROJECTILE_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( 1 );
		cMsg.WriteString( szFX );
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	}

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, m_dwFlagsAtStart, FLAGMASK_ALL );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::UpdateMagneticHoming()
//
//	PURPOSE:	Turn the direction toward the best character in the
//				magnetic radius (console variable DiscMagneticHoming)
//				RETAIL: 0x1003a3f0.  The second argument (UpdateStateFlying
//				gives a copy of m_fVelocity) is not used.
//				Returns true when it changes the direction.
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateMagneticHoming( LTVector *pvDir, LTFLOAT const *pfTronUnknown )
{
	if ( 0.0f == g_vtDiscMagneticHoming.GetFloat() )
	{
		return false;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	ObjectList *pList = g_pLTServer->FindObjectsTouchingSphere( &vPos, m_fMagneticRadius );

	HOBJECT hTarget = LTNULL;

	if ( pList )
	{
		// RETAIL: the character with the smallest value is the target
		float fBestValue = 1000000.0f;

		for ( ObjectLink *pLink = pList->m_pFirstLink; pLink; pLink = pLink->m_pNext )
		{
			if ( pLink->m_hObject == m_hFiredFrom ) continue;
			if ( !IsCharacter( pLink->m_hObject ) ) continue;

			LTVector vObjPos;
			g_pLTServer->GetObjectPos( pLink->m_hObject, &vObjPos );

			LTVector vToObj = vObjPos - vPos;

			LTVector vToObjFlat = vToObj;
			vToObjFlat.y = 0.0f;
			vToObjFlat.Normalize();

			LTVector vVel;
			g_pPhysicsLT->GetVelocity( m_hObject, &vVel );

			LTVector vVelFlat = vVel;
			vVelFlat.y = 0.0f;
			vVelFlat.Normalize();

			float fDot = vVelFlat.Dot( vToObjFlat );
			float fAngle = ( float )acos( fDot );

			if ( m_fMagneticAngleMax < fAngle ) continue;
			if ( fDot < 0.0f ) continue;

			float fValue = ( m_fMagneticRadius - vToObj.Mag() ) / m_fMagneticRadius +
			               ( m_fMagneticAngleMax - fAngle ) / m_fMagneticAngleMax;
			if ( fValue < fBestValue )
			{
				hTarget = pLink->m_hObject;
				fBestValue = fValue;
			}
		}

		g_pLTServer->RelinquishList( pList );

		if ( hTarget )
		{
			ClearDebugInfo();
			ShowDebugTarget( hTarget );

			LTVector vTargetPos;
			g_pLTServer->GetObjectPos( hTarget, &vTargetPos );

			LTVector vToTarget = vTargetPos - vPos;
			float fDist = vToTarget.Mag();
			vToTarget *= 1.0f / fDist;

			float fInfluence = ( 1.0f - fDist / m_fMagneticRadius ) *
			                   ( m_fMagneticMiddleInfluence - m_fMagneticEdgeInfluence ) +
			                   m_fMagneticEdgeInfluence;

			LTVector vNewDir = ( vToTarget - *pvDir ) * fInfluence + *pvDir;
			vNewDir.Normalize();
			*pvDir = vNewDir;

			return true;
		}
	}

	ClearDebugInfo();

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimFilterFn()
//
//	PURPOSE:	RETAIL: 0x1003ac60: the auto aim's line of sight test hits
//				solid objects that are not in the list (a copy of
//				ProjectileTypes' MoveToFloorFilterFn).  The name is ours
//
// ----------------------------------------------------------------------- //

static bool AutoAimFilterFn( HOBJECT hTest, void *pUserData )
{
	// First filter out objects in the list...

	if ( ObjListFilterFn( hTest, pUserData ) )
	{
		// then the line of sight is blocked by solid objects only

		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags( hTest, OFT_Flags, dwFlags );
		if ( dwFlags & FLAG_SOLID )
		{
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsAutoAimTarget()
//
//	PURPOSE:	RETAIL: 0x1003aca0: a player's disc aims at the players of
//				other teams in a disc game, at the characters that its
//				owner hates and at finders.  The name is ours
//
// ----------------------------------------------------------------------- //

static bool IsAutoAimTarget( HOBJECT hFiredFrom, HOBJECT hObj )
{
	if ( !hObj || ( hObj == hFiredFrom ) )
	{
		return false;
	}

	if ( !IsPlayer( hFiredFrom ) )
	{
		return false;
	}

	if ( IsDiscGameType() && IsPlayer( hObj ) )
	{
		CPlayerObj *pFiredFrom = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hFiredFrom ) );
		CPlayerObj *pPlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObj ) );
		if ( !pFiredFrom || !pPlayer )
		{
			return false;
		}

		uint32 nFiredFromID = g_pLTServer->GetClientID( pFiredFrom->GetClient() );
		uint32 nPlayerID = g_pLTServer->GetClientID( pPlayer->GetClient() );

		int nFiredFromTeam = g_pTronTeamMgr->GetTeamOfClient( nFiredFromID );
		int nPlayerTeam = g_pTronTeamMgr->GetTeamOfClient( nPlayerID );

		// players with no team are everyone's targets
		if ( ( TRON_MAX_TEAMS == nFiredFromTeam ) || ( TRON_MAX_TEAMS == nPlayerTeam ) )
		{
			return true;
		}

		return ( nFiredFromTeam != nPlayerTeam );
	}

	if ( IsCharacter( hObj ) )
	{
		CCharacter *pFiredFrom = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( hFiredFrom ) );
		CCharacter *pCharacter = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( hObj ) );
		if ( !pFiredFrom || !pCharacter )
		{
			return false;
		}

		return ( HATE == GetAlignment( pFiredFrom->GetRelationSet(), pCharacter->GetRelationData() ) );
	}

	return IsKindOf( hObj, "Finder" ) ? true : false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::AcquireAutoAimTarget()
//
//	PURPOSE:	RETAIL: 0x1003a780: when the disc is out of the auto aim
//				radius of its owner, home in on the nearest target in that
//				radius that the disc can see.  The name is ours
//
// ----------------------------------------------------------------------- //

bool CDisc::AcquireAutoAimTarget()
{
	if ( !m_bAutoAim || m_hObjectTarget )
	{
		return false;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTFLOAT fRadius = g_vtDiscAutoAimRadius.GetFloat();

	LTVector vOwnerPos;
	g_pLTServer->GetObjectPos( m_hFiredFrom, &vOwnerPos );
	if ( ( vOwnerPos - vPos ).Mag() <= fRadius )
	{
		return false;
	}

	ObjectList *pList = g_pLTServer->FindObjectsTouchingSphere( &vPos, fRadius );
	HOBJECT hTarget = LTNULL;
	if ( pList )
	{
		LTFLOAT fNearest = fRadius;
		for ( ObjectLink *pLink = pList->m_pFirstLink; pLink; pLink = pLink->m_pNext )
		{
			if ( !IsAutoAimTarget( m_hFiredFrom, pLink->m_hObject ) )
			{
				continue;
			}

			LTVector vTargetPos;
			g_pLTServer->GetObjectPos( pLink->m_hObject, &vTargetPos );

			LTFLOAT fDist = ( vTargetPos - vPos ).Mag();
			if ( fDist >= fNearest )
			{
				continue;
			}

			// can the disc see it?
			IntersectInfo iInfo;
			IntersectQuery qInfo;

			CCharacter *pCharacter = IsCharacter( pLink->m_hObject ) ?
				dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( pLink->m_hObject ) ) : LTNULL;
			HOBJECT hHitBox = pCharacter ? pCharacter->GetHitBoxObject() : LTNULL;

			HOBJECT ahIgnore[] = { m_hObject, m_hFiredFrom, pLink->m_hObject, hHitBox, LTNULL };

			qInfo.m_From = vPos;
			qInfo.m_To = vTargetPos;
			qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_FilterFn = AutoAimFilterFn;
			qInfo.m_pUserData = ahIgnore;

			if ( !g_pLTServer->IntersectSegment( &qInfo, &iInfo ) )
			{
				fNearest = fDist;
				hTarget = pLink->m_hObject;
			}
		}

		g_pLTServer->RelinquishList( pList );
	}

	if ( hTarget )
	{
		ClearDebugInfo();
		ShowDebugTarget( hTarget );

		// home in on it
		m_eDiscState = DISC_STATE_FLYING;
		m_eTrackingMode = DISC_TRACKING_MODE_HOMING;
		m_hObjectTarget = hTarget;
		m_bNewTarget = true;
		m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET;

		AMMO const *pAmmo;
		if ( IsPlayer( m_hFiredFrom ) && IsDiscGameType() )
		{
			pAmmo = g_pWeaponMgr->GetAmmo( "DiscEnergy" );
		}
		else
		{
			pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
		}

		if ( !pAmmo || !pAmmo->pProjectileFX )
		{
			return false;
		}

		DISCCLASSDATA *pDiscClassData = dynamic_cast< DISCCLASSDATA* >( pAmmo->pProjectileFX->pClassData );
		if ( !pDiscClassData )
		{
			return false;
		}

		m_fTurnRate = g_vtDiscAutoAimTurnMult.GetFloat() * pDiscClassData->fTurnRate;
		m_fIncidentAngleToControlLine = g_vtDiscAutoAimAngleMult.GetFloat() * pDiscClassData->fIncidentAngleToControlLine;
		m_fIncidentAngleToControlLineDecay = g_vtDiscAutoAimAngleDecayMult.GetFloat() * pDiscClassData->fIncidentAngleToControlLineDecay;
		m_fTargetAngleTrackThreshold = ( LTFLOAT )cos( pDiscClassData->fTargetAngleTrackThreshold );
		m_fLostTargetFlightTime = pDiscClassData->fLostTargetFlightTime;

		return true;
	}

	ClearDebugInfo();
	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::ShowDebugTarget()
//
//	PURPOSE:	RETAIL: 0x1026abb0 (folded): the body is empty in the
//				retail build.  The name is ours
//
// ----------------------------------------------------------------------- //

void CDisc::ShowDebugTarget( HOBJECT hTarget )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::StartBlockable()
//
//	PURPOSE:	RETAIL: 0x1003a350
//
// ----------------------------------------------------------------------- //

void CDisc::StartBlockable()
{
	m_bBlockable = true;

	if ( m_pRelationData )
	{
		m_eDiscBlockableStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_ProjectileVisible,
			m_hFiredFrom, m_hObject, *m_pRelationData, CAIStimulusRecord::kDynamicPos_TrackTarget, LTVector( 0.0f, 0.0f, 0.0f ), 1.0f );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::StopBlockable()
//
//	PURPOSE:	RETAIL: 0x1003a3c0
//
// ----------------------------------------------------------------------- //

void CDisc::StopBlockable()
{
	m_bBlockable = false;

	if ( m_eDiscBlockableStimID != kStimID_Unset )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eDiscBlockableStimID );
		m_eDiscBlockableStimID = kStimID_Unset;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::IsDiscDeflecting()
//
//	PURPOSE:	RETAIL: 0x1003cf30
//
// ----------------------------------------------------------------------- //

bool CDisc::IsDiscDeflecting()
{
	return ( ( DISC_STATE_DEFLECTING == m_eDiscState ) ||
	         ( DISC_STATE_DEADLY_DEFLECTING == m_eDiscState ) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::SendReturnedMessage()
//
//	PURPOSE:	Tell the owner (and its client) that the disc is back
//				RETAIL: 0x1003cf50
//
// ----------------------------------------------------------------------- //

void CDisc::SendReturnedMessage()
{
	// done sending the messages
	RequestStopSendingMessages();

	CAutoMessage cMsg;
	cMsg.Writeuint32( MID_PROJECTILE );
	cMsg.Writeuint8( MPROJ_RETURNED );
	cMsg.Writeuint8( m_nWeaponId );
	cMsg.Writeuint8( m_nDiscAmmoId );
	cMsg.Writeuint8( 1 );
	g_pLTServer->SendToObject( cMsg.Read(), m_hObject, m_hFiredFrom, MESSAGE_GUARANTEED );

	m_bSentReturnedMsg = true;

	if ( IsPlayer( m_hFiredFrom ) )
	{
		CPlayerObj *pPlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_hFiredFrom ) );
		if ( pPlayer )
		{
			HCLIENT hClient = pPlayer->GetClient();
			if ( hClient )
			{
				CAutoMessage cClientMsg;
				cClientMsg.Writeuint8( MID_PROJECTILE );
				cClientMsg.Writeuint8( MPROJ_RETURNED );
				cClientMsg.Writeuint8( m_nWeaponId );
				cClientMsg.Writeuint8( m_nDiscAmmoId );
				cClientMsg.Writeuint8( 1 );
				g_pLTServer->SendToClient( cClientMsg.Read(), hClient, MESSAGE_GUARANTEED );
			}
		}
	}

	RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::SmiteObject()
//
//	PURPOSE:	RETAIL: 0x1003d1b0: a deadly deflected disc hits an
//				object.  A character that allows the disk spike takes
//				the power block damage of the one who deflected the disc;
//				others take the disc's damage.  The name is ours
//
// ----------------------------------------------------------------------- //

void CDisc::SmiteObject( HOBJECT hObj )
{
	CCharacter *pCharacter = LTNULL;
	if ( IsCharacterHitBox( hObj ) )
	{
		CCharacterHitBox *pHitBox = ( CCharacterHitBox* )g_pLTServer->HandleToObject( hObj );
		HOBJECT hModel = pHitBox->GetModelObject();
		if ( IsCharacter( hModel ) )
		{
			pCharacter = ( CCharacter* )g_pLTServer->HandleToObject( hModel );
		}
	}
	else if ( IsCharacter( hObj ) )
	{
		pCharacter = ( CCharacter* )g_pLTServer->HandleToObject( hObj );
	}

	if ( pCharacter )
	{
		if ( !pCharacter->AllowDiskSpike() )
		{
			ImpactDamageObject( m_hFiredFrom, hObj );
		}
		else
		{
			pCharacter->SetModelNodeLastHit( g_pModelButeMgr->GetSkeletonDefaultHitNode( pCharacter->GetModelSkeleton() ) );

			DamageStruct damage;
			damage.eType = DT_POWERBLOCK;
			damage.fDamage = m_fTronUnknownA0C;
			damage.hDamager = m_hLastTouched;
			damage.DoDamage( this, hObj );
		}
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	// RETAIL BUG: with no ammo data the class data is null (0x1003d3a0;
	// docs/RETAIL_BUGS.md)
	DISCCLASSDATA *pDiscClassData = LTNULL;
	if ( m_pAmmoData )
	{
		pDiscClassData = dynamic_cast< DISCCLASSDATA* >( m_pAmmoData->pProjectileFX->pClassData );
	}

	PlayClientFX( pDiscClassData->szDeadlyDeflectSmiteFXName, LTNULL, &vPos, &rRot, 0 );

	m_eDiscState = DISC_STATE_DEFLECTING;
	m_bCanTouchFiredFromObj = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::DestroyInFlight()
//
//	PURPOSE:	RETAIL: 0x1003d460
//
// ----------------------------------------------------------------------- //

void CDisc::DestroyInFlight()
{
	// done sending the messages
	RequestStopSendingMessages();

	StopBlockable();

	m_bSentReturnedMsg = true;

	CreateDestroyedInFlightFX();

	RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::CreateDestroyedInFlightFX()
//
//	PURPOSE:	RETAIL: 0x1003d490
//
// ----------------------------------------------------------------------- //

void CDisc::CreateDestroyedInFlightFX()
{
	if ( !m_pAmmoData || !m_pAmmoData->pProjectileFX )
	{
		return;
	}

	DISCCLASSDATA *pDiscClassData =
		dynamic_cast< DISCCLASSDATA* >( m_pAmmoData->pProjectileFX->pClassData );

	char szProps[1024];
	sprintf( szProps, "FxName %s;Loop 0;SmoothShutdown 0 ", pDiscClassData->szDestroyedInFlightFXName );

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT( theStruct );

	g_pLTServer->GetObjectPos( m_hObject, &theStruct.m_Pos );
	g_pLTServer->GetObjectRotation( m_hObject, &theStruct.m_Rotation );

	HCLASS hClass = g_pLTServer->GetClass( "SpecialFX" );
	if ( hClass )
	{
		dynamic_cast< BaseClass* >( g_pLTServer->CreateObjectProps( hClass, &theStruct, szProps ) );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::GetDeflectOffset()
//
//	PURPOSE:	RETAIL: 0x1003d410.  The spiral of the deflection: the
//				radius grows from the initial one to the final one while
//				the disc turns and moves back
//
// ----------------------------------------------------------------------- //

void CDisc::GetDeflectOffset( LTVector *pvOffset,
                              float fInitialRadius, float fFinalRadius,
                              float fDistance, float fAngularSpeed,
                              float fTime, float fTotalTime )
{
	float fT = fTime / fTotalTime;
	float fRadius = fInitialRadius + ( fFinalRadius - fInitialRadius ) * fT;
	float fAngle = fT * fAngularSpeed - MATH_HALFPI;

	pvOffset->x = ( float )cos( fAngle ) * fRadius;
	pvOffset->y = ( float )sin( fAngle ) * fRadius;
	pvOffset->z = -( fT * fDistance );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//				RETAIL: 0x10038a00.  Build 69's has the flying and the
//				returning states only.  Tron's deadly deflected disc
//				smites what it touches (as HandleDeadlyDeflectTouch), a
//				deflected disc flies on along its spiral, and a
//				returning disc is deflected by a defending character
//
// ----------------------------------------------------------------------- //

void CDisc::HandleTouch( HOBJECT hObj )
{
	switch ( m_eDiscState )
	{
		case DISC_STATE_FLYING:
		{
			// handle normally
			CProjectile::HandleTouch( hObj );
		};
		return;

		case DISC_STATE_DEADLY_DEFLECTING:
		{
			// only one touch in the update that deflected the disc
			if ( m_nDeflectUpdateNum == m_nUpdateNum )
			{
				break;
			}

			HOBJECT hLastHitBox = LTNULL;

			static HCLASS s_hFinderClass = g_pLTServer->GetClass( "Finder" );

			HCLASS hClass = g_pLTServer->GetObjectClass( hObj );
			if ( IsCharacter( m_hLastTouched ) )
			{
				CCharacter *pCharacter = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( m_hLastTouched ) );
				hLastHitBox = pCharacter->GetHitBoxObject();
			}

			if ( ( hObj == m_hLastTouched ) || ( hObj == hLastHitBox ) )
			{
				break;
			}

			uint32 dwFlags;
			g_pCommonLT->GetObjectFlags( hObj, OFT_Flags, dwFlags );

			bool bHitBox = ( IsCharacterHitBox( hObj ) > 0 );
			if ( !( dwFlags & FLAG_SOLID ) )
			{
				if ( !bHitBox )
				{
					break;
				}
			}
			else if ( !bHitBox && !IsCharacter( hObj ) && !g_pLTServer->IsKindOf( hClass, s_hFinderClass ) )
			{
				// other solid objects end the deadly deflection
				m_eDiscState = DISC_STATE_DEFLECTING;
				goto Deflecting;
			}

			SmiteObject( hObj );
			m_hLastTouched = LTNULL;
		};
		return;

		case DISC_STATE_DEFLECTING:
		Deflecting:
		{
			// the first touch some updates after the deflection sends the
			// disc on along its spiral
			if ( m_bDeflectAdjust && ( m_nDeflectUpdateNum < m_nUpdateNum - 5 ) )
			{
				m_bDeflectAdjust = false;

				LTVector vOffset;
				GetDeflectOffset( &vOffset,
				                  m_fDeflectInitialRadius,
				                  m_fDeflectFinalRadius,
				                  m_fDeflectDistance,
				                  m_fDeflectAngularSpeed,
				                  GetDeflectElapsedTime(),
				                  m_fDeflectTotalTime );

				LTVector vNextOffset;
				GetDeflectOffset( &vNextOffset,
				                  m_fDeflectInitialRadius,
				                  m_fDeflectFinalRadius,
				                  m_fDeflectDistance,
				                  m_fDeflectAngularSpeed,
				                  GetDeflectElapsedTime() + 0.1f,
				                  m_fDeflectTotalTime );

				LTVector vDir = vNextOffset - vOffset;
				LTVector vVel = ( vDir / vDir.Mag() ) * m_fDeflectVelocity;
				g_pPhysicsLT->SetVelocity( m_hObject, &vVel );
			}

			CProjectile::HandleTouch( hObj );
		};
		return;

		case DISC_STATE_RETURNING:
		{
			if ( IsMultiplayerGame() )
			{
				if ( IsWorldModel( hObj ) || IsMainWorld( hObj ) )
				{
					// go through the world on the way back
					g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags,
						FLAG_GOTHRUWORLD | FLAG_FULLPOSITIONRES,
						FLAG_REMOVEIFOUTSIDE | FLAG_GOTHRUWORLD | FLAG_STAIRSTEP | FLAG_GRAVITY | FLAG_SOLID | FLAG_FULLPOSITIONRES );
					return;
				}

				if ( hObj == m_hFiredFrom )
				{
					// the disc is back with its owner
					SendReturnedMessage();
					return;
				}

				if ( IsBlockable() )
				{
					CProjectile::HandleTouch( hObj );
				}

				return;
			}

			if ( hObj == m_hFiredFrom )
			{
				// the disc is back with its owner
				SendReturnedMessage();
				return;
			}

			if ( IsCharacter( hObj ) )
			{
				CCharacter *pCharacter = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( hObj ) );
				if ( pCharacter->IsDefending() )
				{
					HandleDeflect();

					// tell the character that it blocked the disc
					CAutoMessage cMsg;
					cMsg.Writeuint32( MID_PROJECTILE );
					cMsg.Writeuint8( MPROJ_BLOCKED );
					cMsg.Writeuint8( IMPACT_TYPE_BLOCKED );
					g_pLTServer->SendToObject( cMsg.Read(), m_hObject, hObj, MESSAGE_GUARANTEED );
				}
			}
		};
		break;
	};
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleImpact()
//
//	PURPOSE:	Allow sub-classes to handle impacts...default is to
//				go away.
//				RETAIL: 0x10039090.  Tron's disc is blocked (2) by a
//				defending character or a shell armor and deadly blocked
//				(3) by a player with an active Shield_prism.
//
// ----------------------------------------------------------------------- //


void CDisc::HandleImpact( HOBJECT hObj )
{
	// Check to see if our creator is still here before doing this -- we
	// don't want to do anything if they're not
	if( !m_hFiredFrom )
	{
		return;
	}

	SurfaceType eSurfaceType = ST_UNKNOWN;

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTVector vNormal( 0.0f, 1.0f, 0.0f );

	int nImpactType = IMPACT_TYPE_IMPACT;

	if ( hObj )
	{
		if ( IsCharacter( hObj ) )
		{
			CCharacter *pCharacter = dynamic_cast< CCharacter * >(
					g_pLTServer->HandleToObject( hObj )
				);

			if ( pCharacter->IsDefending() && IsBlockable() )
			{
				if ( IsPlayer( hObj ) )
				{
					CTronPlayerObj *pPlayer = dynamic_cast< CTronPlayerObj* >( pCharacter );
					if ( pPlayer->GetDefensePercentage( &m_vDir ) > 0.95f )
					{
						PlayerSubroutineSlot *pSlot = pPlayer->FindSubroutine( "Shield_prism" );
						if ( pSlot && pSlot->nTronUnknown40 )
						{
							nImpactType = DISC_IMPACT_TYPE_DEADLY_BLOCKED;
						}
						else
						{
							nImpactType = IMPACT_TYPE_BLOCKED;
						}
					}
				}
				else
				{
					nImpactType = IMPACT_TYPE_BLOCKED;
				}
			}
		}
		else if ( dynamic_cast< ShellArmor* >( g_pLTServer->HandleToObject( hObj ) ) )
		{
			nImpactType = IMPACT_TYPE_BLOCKED;
		}

		if ( IsMainWorld( hObj ) || ( GetObjectType( hObj ) == OT_WORLDMODEL ) )
		{
			CollisionInfo info;
			g_pLTServer->GetLastCollision( &info );

			if ( info.m_hPoly != INVALID_HPOLY )
			{
				eSurfaceType = GetSurfaceType( info.m_hPoly );
			}

			LTPlane plane = info.m_Plane;
			vNormal = plane.m_Normal;

			// Calculate where we really hit the plane...
			LTVector vVel, vP0, vP1, vDir;
			g_pPhysicsLT->GetVelocity( m_hObject, &vVel );
			vDir = vVel;
			vDir /= vVel.Mag();

			vVel *= g_pLTServer->GetFrameTime();

			vP0 = vPos - vVel;  // a little "behind" of the impact point
			vP1 = vPos + vVel;  // a littel "forward" of the impact point

			IntersectInfo iInfo;
			IntersectQuery qInfo;

			qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;

			qInfo.m_From	  = vP0;
			qInfo.m_To		  = vPos;
			qInfo.m_FilterFn  = SpecificObjectFilterFn;
			qInfo.m_pUserData = m_hObject;

			if ( g_pLTServer->IntersectSegment( &qInfo, &iInfo ) )
			{
				vPos    = iInfo.m_Point - vDir;
				eSurfaceType   = GetSurfaceType( iInfo );
				vNormal = iInfo.m_Plane.m_Normal;
			}
			else
			{
				LTFLOAT fDot1 = VEC_DOT( vNormal, vP0 ) - info.m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT( vNormal, vP1 ) - info.m_Plane.m_Dist;

				if ( ( ( fDot1 < 0.0f ) && ( fDot2 < 0.0f ) ) ||
				     ( ( fDot1 > 0.0f ) && ( fDot2 > 0.0f ) ) )
				{
					vPos = vP1;
				}
				else
				{
					LTFLOAT fPercent = -fDot1 / ( fDot2 - fDot1 );
					VEC_LERP( vPos, vP0, vP1, fPercent );
				}
			}

			LTRotation rRot( vNormal, LTVector( 0.0f, 1.0f, 0.0f ) );
			g_pLTServer->SetObjectRotation( m_hObject, &rRot );
		}
	}
	else
	{
		eSurfaceType = ST_AIR;
	}

	if ( eSurfaceType == ST_UNKNOWN )
	{
		eSurfaceType = GetSurfaceType( hObj );
	}

	AddImpact( hObj, m_vFlashPos, vPos, vNormal, eSurfaceType, ( IMPACT_TYPE )nImpactType, false );

	// Do the damage
	if ( hObj && IsBlockable() )
	{
		// RETAIL: a blocked disc damages only players
		if ( ( ( IMPACT_TYPE_BLOCKED != nImpactType ) && ( DISC_IMPACT_TYPE_DEADLY_BLOCKED != nImpactType ) ) ||
		     IsPlayer( hObj ) )
		{
			HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : LTObjRef( m_hObject );
			ImpactDamageObject( hDamager, hObj );
		}
	}

	if ( IsDiscReturning() || IsDiscDeflecting() )
	{
		return;
	}

	if ( IMPACT_TYPE_BLOCKED == nImpactType )
	{
		HandleDeflect();
	}
	else if ( DISC_IMPACT_TYPE_DEADLY_BLOCKED == nImpactType )
	{
		HandleDeadlyDeflect( hObj );
	}
	else
	{
		if ( m_bReturnToOwner )
		{
			EnterReturnMode( hObj );
		}
		else if ( !hObj )
		{
			DestroyInFlight();
		}
		else
		{
			RemoveObject();
		}

		return;
	}

	// tell the character that it blocked the disc
	CAutoMessage cMsg;
	cMsg.Writeuint32( MID_PROJECTILE );
	cMsg.Writeuint8( MPROJ_BLOCKED );
	cMsg.Writeuint8( nImpactType );
	g_pLTServer->SendToObject( cMsg.Read(), m_hObject, hObj, MESSAGE_GUARANTEED );
}



// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::UpdateStateFlying()
//
//	PURPOSE:	Update the disc's movement when the disc is flying towards
//				a target
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateStateFlying()
{
	// RETAIL: 0x1003bba0.  Tron's disc flies on a leash in a multiplayer
	// game: when it goes too far from its owner, it returns (or is
	// destroyed)
	if ( IsMultiplayerGame() )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );

		LTVector vOwnerPos;
		g_pLTServer->GetObjectPos( m_hFiredFrom, &vOwnerPos );

		LTVector vOwnerToDisc = vPos - vOwnerPos;
		if ( vOwnerToDisc.Mag() > m_fMultiplayerLeashLength )
		{
			if ( m_bReturnToOwner )
			{
				LTVector vNewPos = vOwnerToDisc.Unit() * m_fMultiplayerLeashLength + vOwnerPos;
				g_pLTServer->SetObjectPos( m_hObject, &vNewPos );
				EnterReturnMode( LTNULL );
			}
			else
			{
				DestroyInFlight();
			}
		}
	}

	// the disc's flight time is over
	if ( g_pLTServer->GetTime() >= m_fStartTime + m_fLifeTime )
	{
		if ( m_bReturnToOwner )
		{
			EnterReturnMode( LTNULL );
		}
		else
		{
			DestroyInFlight();
		}
	}

	// a player's steady disc looks for a target
	if ( m_bAutoAim && IsPlayer( m_hFiredFrom ) &&
	     ( DISC_TRACKING_MODE_STEADY == m_eTrackingMode ) )
	{
		AcquireAutoAimTarget();
	}

	if ( m_bNewTarget && ( DISC_TRACKING_MODE_STEADY != m_eTrackingMode ) )
	{
		LTVector vControlPosition;
		LTVector vControlDirection;

		if ( DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET == m_eTrackingStyle )
		{
			// RETAIL: Tron's homing disc steers along a line from its owner
			// to the target
			if ( !m_hObjectTarget )
			{
				// the target is gone, fly straight for a time
				m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT;
				m_eTrackingMode = DISC_TRACKING_MODE_STEADY;
				m_fLifeTime = g_pLTServer->GetTime() - m_fStartTime + m_fLostTargetFlightTime;
				return true;
			}

			vControlPosition = GetControlPosition( m_hFiredFrom );

			LTVector vPos;
			g_pLTServer->GetObjectPos( m_hObject, &vPos );

			LTVector vToTarget = GetTargetPosition();

			vControlDirection = vToTarget - vControlPosition;
			vControlDirection.Normalize();

			vToTarget = vToTarget - vPos;

			// a player's disc near its target flies straight at it
			if ( m_bAutoAim && IsPlayer( m_hFiredFrom ) )
			{
				if ( vToTarget.Mag() < g_vtDiscAutoAimRadius.GetFloat() * 0.5f )
				{
					m_bTronUnknown9F9 = true;
					m_vTronUnknown9FC = vToTarget;
				}
			}

			vToTarget.Norm();

			LTVector vVel;
			g_pPhysicsLT->GetVelocity( m_hObject, &vVel );
			vVel.Norm();

			// the target is too far from the path of the disc
			if ( vVel.Dot( vToTarget ) < m_fTargetAngleTrackThreshold )
			{
				m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT;
				m_eTrackingMode = DISC_TRACKING_MODE_STEADY;

				if ( !IsPlayer( m_hFiredFrom ) )
				{
					m_fLifeTime = g_pLTServer->GetTime() - m_fStartTime + m_fLostTargetFlightTime;
				}

				return true;
			}
		}
		else if ( DISC_TRACKING_STYLE_CONTROL_LINE == m_eTrackingStyle )
		{
			vControlPosition = m_vControlPosition;
			vControlDirection = m_vControlDirection;
		}

		//
		// find a normalized vector pointing from the disc to the
		// closest point on a line extending from the control point
		// in the direction of the control direction
		//

		// get the disc position
		LTVector vDiscPos;
		g_pLTServer->GetObjectPos( m_hObject, &vDiscPos );

		// get the disc's current direction
		LTVector vDiscDirection;
		g_pPhysicsLT->GetVelocity( m_hObject, &vDiscDirection );
		vDiscDirection.Normalize();

		// using the direction, get the point on that line that is
		// closest to the current location of the disc
		LTVector vVectorToControlLine;
		GetVectorToLine( vControlPosition,
		                 vControlDirection,
		                 vDiscPos,
		                 &vVectorToControlLine );
		LTVector vVectorToControlLineNormalized = vVectorToControlLine;
		VEC_NORM( vVectorToControlLineNormalized );

		// find the incident angle, which is the initial angle
		// minus the decay times the time elapsed
		LTFLOAT fIncidentAngle = m_fIncidentAngleToControlLine -
			( g_pLTServer->GetTime() - m_fStartTime ) * m_fIncidentAngleToControlLineDecay;

		// make sure the angle is not less than 0
		if ( DISC_EPSILON > fIncidentAngle )
		{
			fIncidentAngle = 0.0f;
		}

		// convert the angle to a percentage (of 90 degrees)
		// and linearly interpolate it
		LTVector vDesiredDirection;
		VEC_LERP(
				vDesiredDirection,
				vControlDirection,
				vVectorToControlLineNormalized,
				( fIncidentAngle * ( 1.0f / MATH_HALFPI ) )
			);

		// renormalize the result to eliminate the skew
		VEC_NORM( vDesiredDirection );

		// RETAIL: Tron turns the desired direction toward a near
		// character (0x1003c2a3)
		LTFLOAT fVelocity = m_fVelocity;
		UpdateMagneticHoming( &vDesiredDirection, &fVelocity );

		// the angle between the disc's current direction and the desired
		// direction
		LTFLOAT fDirectionAngle = vDiscDirection.Dot( vDesiredDirection );
		if ( 0.0f < fDirectionAngle )
		{
			fDirectionAngle = ( LTFLOAT )acos( fDirectionAngle );
		}
		else
		{
			fDirectionAngle = MATH_PI - ( LTFLOAT )acos( fDirectionAngle );
		}

		// determine how much to interpolate
		LTFLOAT fInterpolation = ( g_pLTServer->GetTime() - m_fLastUpdateTime ) *
			( m_fTurnRate / fDirectionAngle );

		// don't interpolate more that we're supposed to
		if ( 1.0f < fInterpolation )
		{
			fInterpolation = 1.0f;
		}

		// interpolate the current and desired directions to
		// determine the new direction to go
		LTVector vNewDirection;
		VEC_LERP(
				vNewDirection,
				vDiscDirection,
				vDesiredDirection,
				fInterpolation
			);

		// RETAIL: a player's disc near its target flies straight at it
		if ( m_bTronUnknown9F9 )
		{
			vNewDirection = m_vTronUnknown9FC;
		}

		// scale the new direction to the disc's travelling velocity
		VEC_NORM( vNewDirection );
		LTVector vNewVelocity = vNewDirection;
		vNewVelocity *= g_vtDiscVelocity.GetFloat() * m_fVelocity;

		// set the new velocity
		g_pPhysicsLT->SetVelocity( m_hObject, &vNewVelocity );

		m_vFirePos = vDiscPos;
		m_vDir = vNewDirection;

		// we handled it
		return true;
	}

	// just update "normally", at the speed of the console variable
	if ( g_vtDiscVelocity.GetFloat() != 1.0f )
	{
		LTVector vVel;
		g_pPhysicsLT->GetVelocity( m_hObject, &vVel );
		vVel.Normalize();

		LTVector vNewVel = vVel;
		vNewVel *= g_vtDiscVelocity.GetFloat() * m_fVelocity;
		g_pPhysicsLT->SetVelocity( m_hObject, &vNewVel );
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::UpdateStateDeflecting()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateStateDeflecting()
{
	// RETAIL: 0x1003c790.  The deflection ends after its time: the disc
	// returns (or is destroyed when it does not return to its owner)
	if ( GetDeflectElapsedTime() >= m_fTronUnknown990 )
	{
		if ( m_bReturnToOwner )
		{
			EnterReturnMode( LTNULL );
		}
		else
		{
			DestroyInFlight();
		}

		return true;
	}

	if ( m_bDeflectAdjust )
	{
		// move the disc along the deflection's spiral
		LTVector vOffset( 0.0f, 0.0f, 0.0f );
		GetDeflectOffset( &vOffset,
		                  m_fDeflectInitialRadius,
		                  m_fDeflectFinalRadius,
		                  m_fDeflectDistance,
		                  m_fDeflectAngularSpeed,
		                  GetDeflectElapsedTime(),
		                  m_fDeflectTotalTime );

		LTMatrix mRot;
		m_rDeflectRot.ConvertToMatrix( mRot );

		// RETAIL: 0x1003c8bb calls MatVMul_H out of line; ours inlines it
		LTVector vPos = mRot * vOffset + m_vDeflectPos;
		g_pLTServer->MoveObject( m_hObject, &vPos );
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::GetControlPosition()
//
//	PURPOSE:	RETAIL: 0x1003c570: the object's position, moved up by a
//				part of its height (console variable
//				DiscHomingControlYOffsetPercent).  The name is ours
//
// ----------------------------------------------------------------------- //

LTVector CDisc::GetControlPosition( HOBJECT hObj )
{
	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( hObj, &vDims );

	LTVector vPos;
	g_pLTServer->GetObjectPos( hObj, &vPos );

	vPos.y += g_vtDiscHomingControlYOffsetPercent.GetFloat() * vDims.y;

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::GetTargetPosition()
//
//	PURPOSE:	RETAIL: 0x1003c5f0: the point of the target that the disc
//				flies at.  A player's disc keeps its height inside the
//				target's dims (except for finders); an AI's disc aims at
//				a part of the target's height.  The name is ours
//
// ----------------------------------------------------------------------- //

LTVector CDisc::GetTargetPosition()
{
	if ( !m_hObjectTarget )
	{
		return LTVector( 0.0f, 0.0f, 0.0f );
	}

	LTVector vTargetPos;
	g_pLTServer->GetObjectPos( m_hObjectTarget, &vTargetPos );

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( m_hObjectTarget, &vDims );

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	if ( IsPlayer( m_hFiredFrom ) )
	{
		if ( !IsKindOf( m_hObjectTarget, "Finder" ) )
		{
			LTFLOAT fTop = vTargetPos.y + vDims.y;
			LTFLOAT fBottom = vTargetPos.y - vDims.y;

			if ( vPos.y < fBottom )
			{
				vTargetPos.y = g_vtDiscAutoAimTargetFeetOffset.GetFloat() + fBottom;
			}
			else if ( vPos.y > fTop )
			{
				vTargetPos.y = g_vtDiscAutoAimTargetHeadOffset.GetFloat() + fTop;
			}
			else
			{
				vTargetPos.y = vPos.y;
			}
		}
	}
	else
	{
		vTargetPos.y += g_vtDiscHomingTargetYOffsetPercent.GetFloat() * vDims.y;
	}

	return vTargetPos;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CDisc::GetReturnToPosition()
//              
//	PURPOSE:	Returns the location the Disc should try to return to. 
//				Attempts to use the specified node if it exists and is
//				accessable.
//              
//----------------------------------------------------------------------------
LTVector CDisc::GetReturnToPosition(const LTVector& vOffset)
{
	// RETAIL: 0x1003c910.  Tron's has neither the asserts nor the test of
	// the socket against the owner's dims (it still reads the dims)

	// Get the position of the owner
	LTVector vOwnerOrigin;
	g_pLTServer->GetObjectPos( m_hFiredFrom, &vOwnerOrigin );

	// Get the default position to return to
	LTVector vDefaultPosition = vOwnerOrigin + vOffset;

	// If we have a node to return to, then try going there
	if ( m_hReturnToSocket != INVALID_MODEL_SOCKET)
	{
		// Get the position of the Socket
		LTransform transform;
		g_pModelLT->GetSocketTransform(m_hFiredFrom, m_hReturnToSocket, transform, LTTRUE);
		LTVector vSocketPos = transform.m_Pos;

		// Get the name of the currently playing animation
		ANIMTRACKERID nTracker;
		g_pModelLT->GetMainTracker(m_hFiredFrom, nTracker);
		HMODELANIM hAnim;
		g_pModelLT->GetCurAnim(m_hFiredFrom, nTracker, hAnim );

		LTVector vDims;
		g_pCommonLT->GetModelAnimUserDims(m_hFiredFrom, &vDims, hAnim );

		// Okay, lets go with the socket position...
		return vSocketPos;
	}

	// If There is no node to return to, then to the deault
	return vDefaultPosition;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::UpdateStateReturning()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateStateReturning()
{
	// RETAIL: 0x1003ca40

	// pointer check
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pPhysicsLT );

	// state check
	ASSERT( DISC_STATE_RETURNING == m_eDiscState );

	// for checking return values
	LTRESULT ltResult;

	// get the current velocity
	LTVector vVel;
	ltResult = g_pPhysicsLT->GetVelocity( m_hObject, &vVel );
	ASSERT( LT_OK == ltResult );

	// get the ammo data
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
	ASSERT( 0 != pAmmo );

	// get the projectile
	PROJECTILEFX *pProjectileFX = pAmmo->pProjectileFX;
	ASSERT( 0 != pProjectileFX );

	// get the projectile class data
	DISCCLASSDATA *pDiscClassData =
		dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
	ASSERT( 0 != pDiscClassData );

	// bail if we have bad data
	if ( !pAmmo || !pProjectileFX || !pDiscClassData )
	{
		return false;
	}

	// Figure out a new velocity, make the disc
	// return straight to the player
	LTVector vReturnToPos;
	LTVector vSelfPos;
	LTVector vSelfToTarget;

	// return to the owner, figure out where the owner is
	LTVector vOffset = LTVector(0,pDiscClassData->fReturnHeightOffset,0);
	vReturnToPos = GetReturnToPosition( vOffset );
	
	// get the disc's location
	ltResult = g_pLTServer->GetObjectPos( m_hObject, &vSelfPos );
	ASSERT( LT_OK == ltResult );

	// find a vector from the disc and its target
	vSelfToTarget = vReturnToPos - vSelfPos;

	// get the direction
	vSelfToTarget.Normalize();

	// scale the direction to the return velocity
	LTVector vNewVel = vSelfToTarget * m_fReturnVelocity;

	// set the velocity
	ltResult = g_pPhysicsLT->SetVelocity( m_hObject, &vNewVel );
	ASSERT( LT_OK == ltResult );

	// orient the disc along the path of travel
	LTRotation rotDiscRotation;
	ltResult = g_pLTServer->GetObjectRotation( m_hObject, &rotDiscRotation );
	LTVector vUp = rotDiscRotation.Up();
	LTRotation rotNewDiscRotation( vSelfToTarget, vUp );
	g_pLTServer->SetObjectRotation( m_hObject, &rotNewDiscRotation );

	m_vFirePos = vSelfPos;
	m_vDir = vSelfToTarget;

	// RETAIL: the disc is back when it can not get there (in time) or
	// when it flies past its owner in this update
	LTVector vMove = vNewVel * ( g_pLTServer->GetFrameTime() * 1.5f );
	LTFLOAT fDist = ( vReturnToPos - vSelfPos ).Mag();

	if ( !_finite( fDist ) )
	{
		SendReturnedMessage();
	}

	if ( ( fDist / m_fReturnVelocity > 10.0f ) ||
	     ( ( vMove.Mag() > fDist ) && ( vVel.Dot( vNewVel ) < 0.0f ) ) )
	{
		SendReturnedMessage();
	}

	// RETAIL: a blockable disc tests the line from its last position (not
	// in a multiplayer game)
	if ( IsBlockable() && !IsMultiplayerGame() )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		DoVectorTrace( m_vTronUnknownA14, vPos );
	}

	g_pLTServer->GetObjectPos( m_hObject, &m_vTronUnknownA14 );

	// we handled it
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CDisc::IsDiscReturning()
//              
//	PURPOSE:	Returns true if the disc is returning, false if it is not.
//				This allows us to keep the enumeration in the disc itself
//				instead of spreading it out over all derived classes as well.
//              
//----------------------------------------------------------------------------
bool CDisc::IsDiscReturning()
{
	return ( m_eDiscState == DISC_STATE_RETURNING );
}
