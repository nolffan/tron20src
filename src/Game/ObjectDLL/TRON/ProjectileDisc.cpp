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

	// RETAIL: Tron's has one more state before the return (0x10039c5e sets
	// it; UpdateDisc's jump table, 0x1003904c, updates it as a deflection).
	// The name is ours
	DISC_STATE_TRONUNKNOWN3,

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
	, m_bTronUnknown9E4( false )
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

	m_bTronUnknown9E4 = info.bTronUnknown164;

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
	if ( !_stricmp( "Sequencer", m_pWeaponData->szName ) && m_bTronUnknownF1 )
	{
		m_fTurnRate *= m_fTronUnknown108;
		m_fIncidentAngleToControlLine *= m_fTronUnknown108;
		m_fIncidentAngleToControlLineDecay *= m_fTronUnknown108;
	}
	else if ( !_stricmp( "Cluster", m_pWeaponData->szName ) && m_bTronUnknownF3 )
	{
		m_fTurnRate *= m_fTronUnknown18C;
		m_fIncidentAngleToControlLine *= m_fTronUnknown18C;
		m_fIncidentAngleToControlLineDecay *= m_fTronUnknown18C;
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

			case MPROJ_RETURNING:
			{
				// RETAIL: the owner stops guiding the disc
				if ( !IsDiscDeflecting() )
				{
					m_eTrackingMode = DISC_TRACKING_MODE_STEADY;
					m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT;
				}
			}
			break;

			case MPROJ_DISC_TRONUNKNOWN14:
			{
				DestroyInFlight();
				SendReturnedMessage();
			}
			break;

			case MPROJ_DISC_TRONUNKNOWN150:
			{
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
	SAVE_TIME( m_fDeflectStartTime );
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
	SAVE_bool( m_bTronUnknown9E4 );
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
	LOAD_bool( m_bTronUnknown9E4 );
	LOAD_FLOAT( m_fMagneticRadius );
	LOAD_FLOAT( m_fMagneticMiddleInfluence );
	LOAD_FLOAT( m_fMagneticEdgeInfluence );
	LOAD_FLOAT( m_fMagneticAngleMax );
	LOAD_BYTE( m_nDiscAmmoId );
	LOAD_bool( m_bTronUnknown9F9 );
	pMsg->ReadType( &m_vTronUnknown9FC );
	LOAD_HOBJECT( m_hLastTouched );
	LOAD_FLOAT( m_fTronUnknownA0C );
	LOAD_FLOAT( m_fMultiplayerLeashLength );
	pMsg->ReadType( &m_vTronUnknownA14 );
	LOAD_DWORD( m_dwFlagsAtStart );
	LOAD_TIME( m_fLastUpdateTime );
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
		case DISC_STATE_TRONUNKNOWN3:
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
	if ( m_pTronUnknownRelationData && ( m_eDiscCatchableStimID == kStimID_Unset ) )
	{
		// PORT: Tron's overload also takes the projectile's relation data
		// (0x10100e30, not yet written)
		m_eDiscCatchableStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_CatchableProjectileVisible,
			m_hFiredFrom, m_hObject, CAIStimulusRecord::kDynamicPos_TrackTarget, LTVector( 0.0f, 0.0f, 0.0f ), 1.0f );
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

	// RETAIL: CProjectile's 0xdc and 0xe0
	*reinterpret_cast< LTFLOAT* >( &m_xTronUnknownDC ) = MATH_PI;
	m_xTronUnknownE0 = 0x7fffffff;

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
//				RETAIL: 0x1003a3f0.  The second argument is not used.
//				Returns true when it changes the direction.
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateMagneticHoming( LTVector *pvDir, LTVector const *pvTronUnknown )
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

	if ( m_pTronUnknownRelationData )
	{
		// PORT: Tron's overload also takes the projectile's relation data
		// (0x10100e30, not yet written)
		m_eDiscBlockableStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_ProjectileVisible,
			m_hFiredFrom, m_hObject, CAIStimulusRecord::kDynamicPos_TrackTarget, LTVector( 0.0f, 0.0f, 0.0f ), 1.0f );
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
	         ( DISC_STATE_TRONUNKNOWN3 == m_eDiscState ) );
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
//				PORT: Build 69's; Tron's (0x10038a00) also deflects off
//				characters and handles multiplayer touches
//
// ----------------------------------------------------------------------- //

void CDisc::HandleTouch( HOBJECT hObj )
{
	switch ( m_eDiscState )
	{
		case DISC_STATE_FLYING:
		{
			// handle normally
			CTronProjectile::HandleTouch( hObj );
		};
		break;

		case DISC_STATE_DEFLECTING:
		case DISC_STATE_TRONUNKNOWN3:
		{
		};
		break;

		case DISC_STATE_RETURNING:
		{
			// the disc is back with its owner
			if ( hObj == m_hFiredFrom )
			{
				SendReturnedMessage();
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

// RETAIL: the impact type of a deadly block (ImpactType.h has no name for
// it).  The name is ours
#define DISC_IMPACT_TYPE_DEADLY_BLOCKED		3

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
		// PORT: Tron sends the disc back at its owner here (0x10039c30,
		// state 3; not yet written).  The normal deflection stands in.
		HandleDeflect();
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
	// pointer check
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pPhysicsLT );

	// this will become more refined as time progresses
	// for now, only compute this stuff if the point is new
	if ( !m_bNewTarget || ( DISC_TRACKING_MODE_STEADY == m_eTrackingMode ) )
	{
		// just update "normally"
		return true;
	}

	switch ( m_eTrackingStyle )
	{
		case DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET:
		{
			//
			// Simple basic easy disc tracking method
			//

			// get the current velocity, it will probably change
			LTVector vVel;
			g_pPhysicsLT->GetVelocity( m_hObject, &vVel );

			// Figure out a new velocity
			LTVector vTarget;
			LTVector vSelfPos;
			LTVector vSelfToTarget;

			// trying to hit a target, figure out where the target is
			vTarget = m_vPointTarget;

			// get the disc's location
			g_pLTServer->GetObjectPos( m_hObject, &vSelfPos );

			// find a vector from the disc and its target
			vSelfToTarget = vTarget - vSelfPos;

			// get the ammo data
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
			ASSERT( 0 != pAmmo );

			// get the projectile
			PROJECTILEFX *pProjectileFX = pAmmo->pProjectileFX;
			ASSERT( 0 != pProjectileFX );

			// get the disc class data
			DISCCLASSDATA *pDiscClassData = 
				dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
			ASSERT( 0 != pDiscClassData );

			// bail if we have bad data
			if ( !pAmmo || !pProjectileFX || !pDiscClassData )
			{
				return false;
			}

			// get the direction
			vSelfToTarget.Normalize();

			// scale the direction to the desired velocity
			vVel = vSelfToTarget;
			vVel *= m_fVelocity;

			// set the velocity
			g_pPhysicsLT->SetVelocity( m_hObject, &vVel );

			// orient the disc along the path of travel
			LTRESULT ltResult;
			LTRotation rotDiscRotation;
			ltResult = g_pLTServer->GetObjectRotation( m_hObject, &rotDiscRotation );
			LTVector vUp = rotDiscRotation.Up();
			LTRotation rotNewDiscRotation( vSelfToTarget, vUp );
			g_pLTServer->SetObjectRotation( m_hObject, &rotNewDiscRotation );

			// keep track that we updated the target
			m_bNewTarget = false;
		}
		break;

		case DISC_TRACKING_STYLE_CONTROL_LINE:
		{
			//
			// Disc tracking, pass2
			//

			LTRESULT ltResult;

			//
			// find a normalized vector pointing from the disc to the
			// closest point on a line extending from the control point
			// in the direction of the control direction
			//

			// get the disc position
			LTVector vDiscPos;
			ltResult = g_pLTServer->GetObjectPos( m_hObject, &vDiscPos );
			ASSERT( LT_OK == ltResult );

			// get the disc's current direction
			LTVector vDiscDirection;
			ltResult = g_pPhysicsLT->GetVelocity( m_hObject, &vDiscDirection );
			ASSERT( LT_OK == ltResult );
			vDiscDirection.Normalize();

			// using the direction, get the point on that line that is
			// closest to the current location of the disc
			LTVector vVectorToControlLine;
			LTVector vVectorToControlLineNormalized;
			GetVectorToLine( m_vControlPosition,
							 m_vControlDirection,
							 vDiscPos,
							 &vVectorToControlLine );
			vVectorToControlLineNormalized = vVectorToControlLine;
			vVectorToControlLineNormalized.Normalize();

			//
			// determine the incident angle
			//

			// get the elapsed time
			LTFLOAT fLifeElapsed = g_pLTServer->GetTime() - m_fStartTime;

			// find the incident angle, which is the initial angle
			// minus the decay times the time elapsed
			LTFLOAT fIncidentAngle;
			fIncidentAngle = m_fIncidentAngleToControlLine -
				m_fIncidentAngleToControlLineDecay * fLifeElapsed;

			// make sure the angle is not less than 0
			if ( DISC_EPSILON > fIncidentAngle )
			{
				fIncidentAngle = 0.0f;
			}

			//
			// determine the desired direction of the disc
			//

			// convert the angle to a percentage (of 90 degrees)
			// and linearly interpolate it
			LTVector vDesiredDirection;
			VEC_LERP(
					vDesiredDirection,
					m_vControlDirection,
					vVectorToControlLineNormalized,
					( ( fIncidentAngle ) / MATH_HALFPI )
				);

			// renormalize the result to eliminate the skew
			vDesiredDirection.Normalize();

			// RETAIL: Tron turns the desired direction toward a near
			// character (0x1003c2a3)
			UpdateMagneticHoming( &vDesiredDirection, &vDiscPos );

			//
			// determine the angle between the disc's current direction
			// and the desired direction
			//

			// change the dot product equation around to get the angle
			// NOTE: these should both be unit vectors
			LTFLOAT vDirectionAngle;
			ASSERT( DISC_EPSILON > ( 1.0f - vDesiredDirection.Mag() ) );
			ASSERT( DISC_EPSILON > ( 1.0f - vDiscDirection.Mag() ) );
			vDirectionAngle = ltacosf( vDesiredDirection.Dot( vDiscDirection ) );

			//
			// determine how much time has passed since the last update
			//
			LTFLOAT fTimeSinceLastUpdate;
			fTimeSinceLastUpdate = g_pLTServer->GetTime() - m_fLastUpdateTime;

			//
			// interpolate the vector's current direction toward the
			// desired direction
			//

			// determine how much to interpolate
			LTFLOAT fInterpolation = m_fTurnRate * fTimeSinceLastUpdate;

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
					( m_fTurnRate * fTimeSinceLastUpdate )
				);

			// scale the new direction to the disc's travelling velocity
			vNewDirection.Normalize();
			LTVector vNewVelocity = vNewDirection;
			vNewVelocity *= m_fVelocity;

			// set the new velocity
			g_pPhysicsLT->SetVelocity( m_hObject, &vNewVelocity );

			//
			// orient the disc 
			//
			// not currently implemented

			// keep track of the update time
			m_fLastUpdateTime = g_pLTServer->GetTime();
		}
		break;

		default:
		{
			ASSERT( !"Invalid tracking style" );
		}
		break;
	};

	/*
	//DANO: hardcoded lifetime
	if ( g_pLTServer->GetTime() >= ( m_fStartTime + 10.0f ) )
	{
		Detonate(LTNULL);
	}
	*/

	// we handled it
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
	// [kml] 4/2/02 Commenting this out as per DanO's request
	// get the current time
	/*int nCurrentServerTime =
		static_cast< int >( g_pLTServer->GetTime() * 1000.0f );

	// get the length of the deflection
	int nDeflectDuration = 1000;

	if ( nCurrentServerTime > ( m_nDeflectServerTimeStart + nDeflectDuration ) )
	{
		EnterReturnMode();
		return true;
	}*/

	return true;
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
	// pointer check
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pPhysicsLT );
	ASSERT( 0 != g_pModelLT );

	// Get the position of the owner
	LTRESULT ltResult;
	LTVector vOwnerOrigin;
	ltResult = g_pLTServer->GetObjectPos( m_hFiredFrom, &vOwnerOrigin );
	ASSERT( LT_OK == ltResult );

	// Get the default position to return to
	LTVector vDefaultPosition = vOwnerOrigin + vOffset;

	// If we have a node to return to, then try going there
	if ( m_hReturnToSocket != INVALID_MODEL_SOCKET)
	{
		// Get the position of the Socket
		LTransform transform;
		LTRESULT SocketTransform = g_pModelLT->GetSocketTransform(m_hFiredFrom, m_hReturnToSocket, transform, LTTRUE);
		ASSERT( SocketTransform == LT_OK );
		LTVector vSocketPos = transform.m_Pos;

		// Get the name of the currently playing animation
		ANIMTRACKERID nTracker;
		g_pModelLT->GetMainTracker(m_hFiredFrom, nTracker);
		HMODELANIM hAnim;
		g_pModelLT->GetCurAnim(m_hFiredFrom, nTracker, hAnim );

		LTVector vDims;
		g_pCommonLT->GetModelAnimUserDims(m_hFiredFrom, &vDims, hAnim );
		ASSERT( ltResult == LT_OK );

		// See if the socket position is inside the touch box.
		// If it is not, then we have a problem as the return may fail, so if
		// the position is outside the box, then attempt to return to the
		// default position instead
		if (vSocketPos.x > vOwnerOrigin.x+vDims.x || 
			vSocketPos.x < vOwnerOrigin.x-vDims.x ||
			vSocketPos.y > vOwnerOrigin.y+vDims.y ||
			vSocketPos.y < vOwnerOrigin.y-vDims.y ||
			vSocketPos.z > vOwnerOrigin.z+vDims.z ||
			vSocketPos.z < vOwnerOrigin.z-vDims.z )
		{
			// Get the name of the model
			const int MODEL_LEN = 128;
			char szFileName[MODEL_LEN];
			char szSkinName[MODEL_LEN];
			g_pModelLT->GetFilenames(m_hFiredFrom, szFileName, MODEL_LEN, szSkinName, MODEL_LEN);

			// Get the name of the currently playing animation
			ANIMTRACKERID nTracker;
			g_pModelLT->GetMainTracker(m_hFiredFrom, nTracker);
			HMODELANIM hAnim;
			g_pModelLT->GetCurAnim(m_hFiredFrom, nTracker, hAnim );

			// Print out an error so that it can be corrected.
			char szError[1024];
			sprintf( szError, "Return to node outside dims: Model: %s Anim: %d", szFileName, hAnim );
			g_pLTServer->CPrint( szError );
		
			// Set the position we are going to be returning to back to a 
			// known safe location -- the owners position if this happens
			vSocketPos = vDefaultPosition;
		}
	
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
	// pointer check
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pPhysicsLT );

	// state check
	ASSERT( DISC_STATE_RETURNING == m_eDiscState );

	// for checking return values
	LTRESULT ltResult;

	// get the current velocity, it will probably change
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

	// scale the direction to the desired velocity
	vVel = vSelfToTarget;
	vVel *= pDiscClassData->fReturnVelocity;

	// set the velocity
	ltResult = g_pPhysicsLT->SetVelocity( m_hObject, &vVel );
	ASSERT( LT_OK == ltResult );

	// orient the disc along the path of travel
	LTRotation rotDiscRotation;
	ltResult = g_pLTServer->GetObjectRotation( m_hObject, &rotDiscRotation );
	LTVector vUp = rotDiscRotation.Up();
	LTRotation rotNewDiscRotation( vSelfToTarget, vUp );
	g_pLTServer->SetObjectRotation( m_hObject, &rotNewDiscRotation );

/*
	//DANO: hardcoded lifetime
	if ( g_pLTServer->GetTime() >= ( m_fStartTime + 10.0f ) )
	{
		Detonate( LTNULL );
	}
*/

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
