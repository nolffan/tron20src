// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.cpp
//
// PURPOSE : Projectile class - implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Projectile.h"
#include "ltengineobjects.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "ServerUtilities.h"
#include "Explosion.h"
#include "Character.h"
#include "ClientWeaponSFX.h"
#include "WeaponFXTypes.h"
#include "VolumeBrush.h"
#include "VolumeBrushTypes.h"
#include "ClientServerShared.h"
#include "SurfaceFunctions.h"
#include "PlayerObj.h"
#include "Body.h"
#include "AI.h"
#include "CVarTrack.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "ObjectMsgs.h"
#include "CharacterHitBox.h"
#include "GameServerShell.h"
#include "Camera.h"
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "VersionMgr.h"
#include "PlayerShared.h"	// RETAIL: its constants are initialized here (0x1011dbf0)
#include "DebugLineSystem.h"	// RETAIL: its colors are initialized here (0x1011dc50..)
#include "IHitBoxUser.h"
#include "RelationMgr.h"

LINKFROM_MODULE( Projectile );

#pragma force_active on
BEGIN_CLASS(CProjectile)
END_CLASS_DEFAULT_FLAGS(CProjectile, GameBase, NULL, NULL, CF_HIDDEN)
#pragma force_active off

extern uint16 g_wIgnoreFX;
extern uint8  g_nRandomWeaponSeed;
extern CAIStimulusMgr* g_pAIStimulusMgr;

static bool DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData);
// RETAIL: not static: CAIGoalSpecialDamage calls it too (0x10191d33)
bool DoVectorFilterFn(HOBJECT hObj, void *pUserData);
static bool CanCharacterHitCharacter( HOBJECT hFiredFrom, HOBJECT hImpacted );

#define MAX_MODEL_NODES         9999
#define MAX_VECTOR_LOOP         20

// RETAIL: a MID_PROJECTILE message whose sub id is 150 removes the
// projectile (ObjectMessageFn 0x1011e94f).  The name is ours
#define MPROJ_TRON_REMOVE		150

// RETAIL: how long an object stays in the damaged list (0x102f87ac, used by
// 0x1012204e).  The name and the value's source are ours.
static const LTFLOAT s_fDamagedObjectTime = 0.5f;

// RETAIL: in this order (their constructors 0x1011df00..0x1011dfc0)
CVarTrack g_vtNetFriendlyFire;
CVarTrack g_vtProjectileNewCollision;		// RETAIL: Tron's (global: CProjectileBall and CProjectilePrankster read it)
static CVarTrack g_vtInvisibleMaxThickness;
static CVarTrack g_vtDeflectAngleRange;
static CVarTrack g_vtProjectileCollisionDebug;		// RETAIL: Tron's


namespace
{
	void ProjectileInitFileGlobals()
	{
		if (!g_vtInvisibleMaxThickness.IsInitted())
		{
			g_vtInvisibleMaxThickness.Init(g_pLTServer, "InvisibleMaxThickness", LTNULL, 33.0f);
		}
		if (!g_vtNetFriendlyFire.IsInitted())
		{
			g_vtNetFriendlyFire.Init(g_pLTServer, "NetFriendlyFire", LTNULL, 0.0f);
		}
		if (!g_vtDeflectAngleRange.IsInitted())
		{
			g_vtDeflectAngleRange.Init(g_pLTServer, "DeflectAngleRange", LTNULL, 30.0f );
		}
		// RETAIL: Tron's (0x1011e14d)
		if (!g_vtProjectileCollisionDebug.IsInitted())
		{
			g_vtProjectileCollisionDebug.Init(g_pLTServer, "ProjectileCollisionDebug", LTNULL, 0.0f );
		}
		if (!g_vtProjectileNewCollision.IsInitted())
		{
			g_vtProjectileNewCollision.Init(g_pLTServer, "ProjectileNewCollision", LTNULL, 1.0f );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ProjectileTouchFilterFn
//
//	PURPOSE:	RETAIL: Tron's (0x1011e240): the intersect filter of the
//				new collision code (TestTouchSegment).  The name is ours
//
// ----------------------------------------------------------------------- //

static bool ProjectileTouchFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj) return false;
	if (!pUserData) return true;

	return ((CProjectile*)pUserData)->TouchFilter(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::~CProjectile
//
//	PURPOSE:	Destructor (RETAIL: out of line, 0x1011e260; before the
//				constructor)
//
// ----------------------------------------------------------------------- //

CProjectile::~CProjectile()
{
	if (m_pRelationData)
	{
		delete m_pRelationData;
		m_pRelationData = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CProjectile
//
//	PURPOSE:	Constructor (RETAIL: 0x1011e3c0)
//
// ----------------------------------------------------------------------- //

CProjectile::CProjectile()
	: GameBase(OT_MODEL)
	, m_vFlashPos( 0.0f, 0.0f, 0.0f )
	, m_vFirePos( 0.0f, 0.0f, 0.0f )
	, m_vDir( 0.0f, 0.0f, 0.0f )
	, m_bSilenced( false )
	, m_bObjectRemoved( false )
	, m_bDetonated( false )
	, m_nWeaponId( 0 )
	, m_nAmmoId( 0 )
	, m_bSetup( false )
	, m_bCanHitSameProjectileKind( false )
	, m_bCanTouchFiredFromObj( false )
	, m_eInstDamageType( DT_BULLET )
	, m_eProgDamageType( DT_UNSPECIFIED )
	, m_fInstDamage( 0.0f )
	, m_fProgDamage( 0.0f )
	, m_bTronUnknown7C( false )
	, m_fTronUnknown80( 0.0f )
	, m_fTronUnknown84( 0.0f )
	, m_fStartTime( 0.0f )
	, m_fLifeTime( 5.0f )
	, m_fVelocity( 0.0f )
	, m_fMass( INFINITE_MASS )
	, m_fRange( 10000.0f )
	, m_hFiredFrom( LTNULL )
	, m_bNumCallsToAddImpact( 0 )
	, m_xTronUnknownB0( 0.0f )
	, m_bProcessInvImpact( false )
	, m_vInvisVel( 0.0f, 0.0f, 0.0f )
	, m_vInvisNewPos( 0.0f, 0.0f, 0.0f )
	, m_vTronUnknownD0( 0.0f, 0.0f, 0.0f )
	, m_fMaxRicochetAngle( 0.0f )
	, m_nMaxRicochets( 0 )
	, m_nTotalRicochets( 0 )
	, m_nUpdateNum( 0 )
	, m_nLastRicochetUpdateNum( -1 )
	, m_bReceivingMessages( false )
	, m_lstDamagedObjects()
	, m_lstPendingTouches()
	, m_bTronUnknown1C0( false )
	, m_bTronUnknown1C1( false )
	, m_bTronUnknown1C2( false )
	, m_vLastPos( 0.0f, 0.0f, 0.0f )
	, m_vLastVel( 0.0f, 0.0f, 0.0f )
	, m_damage()
	, m_vDims( 1.0f, 1.0f, 1.0f )
	, m_dwFlags( FLAG_FORCECLIENTUPDATE | FLAG_POINTCOLLIDE | FLAG_NOSLIDING |
	    FLAG_TOUCH_NOTIFY | FLAG_NOLIGHT | FLAG_RAYHIT )
	, m_pWeaponData( LTNULL )
	, m_pAmmoData( LTNULL )
	, m_vClientObjImpactPos( 0.0f, 0.0f, 0.0f )
	, m_hClientObjImpact( LTNULL )
	, m_pRelationData( LTNULL )
	, m_bHasRelationData( false )
	, m_bTronUnknown8FD( false )
	, m_vTronUnknown900( 0.0f, 0.0f, 0.0f )
{
    m_hObject = LTNULL;

	AddAggregate(&m_damage);
	MakeTransitionable();

	// Make sure we can't be damaged by any of the progressive/volumebrush
	// damage types...
	// RETAIL: Tron builds the flags each time (0x1011e631)
	m_nCantDamageFlags = DamageTypeToFlag(DT_BLEEDING)
	        | DamageTypeToFlag(DT_BURN)
	        | DamageTypeToFlag(DT_CHOKE)
	        | DamageTypeToFlag(DT_ELECTROCUTE)
	        | DamageTypeToFlag(DT_FREEZE)
	        | DamageTypeToFlag(DT_POISON)
	        | DamageTypeToFlag(DT_ENDLESS_FALL)
			| DamageTypeToFlag(DT_SLEEPING)
			| DamageTypeToFlag(DT_STUN)
			| DamageTypeToFlag(DT_CAMERA_DISABLER)
			| DamageTypeToFlag(DT_GLUE)
			| DamageTypeToFlag(DT_BEAR_TRAP)
			| DamageTypeToFlag(DT_LAUGHING)
			| DamageTypeToFlag(DT_ASSS)
			| DamageTypeToFlag(DT_SLIPPERY);

	m_pRelationData = debug_new( RelationData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			// keep track of the number of this update
			++m_nUpdateNum;

			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if( fData != PRECREATE_SAVEGAME )
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				if (pStruct)
				{
					pStruct->m_Flags = m_dwFlags;

					// This will be set in Setup()...
					SAFE_STRCPY(pStruct->m_Filename, "Models\\Default.ltb");

					pStruct->m_NextUpdate = UPDATE_NEXT_FRAME;
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			if( !m_bSetup ) break;

			// RETAIL: Tron's new collision code keeps the touch for the
			// next update (0x1011e803)
			if( g_vtProjectileNewCollision.GetFloat() != 0.0f )
			{
				if( m_bTronUnknown1C0 ) break;

				LTFLOAT fInstDamage = m_fInstDamage;
				TronUnknownVirtual30((HOBJECT)pData);
				m_fInstDamage = fInstDamage;
			}
			else
			{
				HandleTouch((HOBJECT)pData);
			}
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


	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	uint32 dwRet = GameBase::ObjectMessageFn(hSender, pMsg);

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		// RETAIL: Tron's (0x1011e946)
		case MID_PROJECTILE:
		{
			if (pMsg->Readuint8() == MPROJ_TRON_REMOVE)
			{
				RemoveObject();
			}
		}
		break;

		case MID_DAMAGE:
		{
			if (m_damage.IsDead())
			{
				Detonate(LTNULL);
			}
		}
		break;

		default : break;
	}

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TronUnknownVirtual28
//
//	PURPOSE:	RETAIL: slot 28 (0x1011e970): determine the velocity
//				(NOLF2's code, moved out of Setup).  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::TronUnknownVirtual28(WeaponFireInfo const& info)
{
	if (info.bOverrideVelocity)
	{
		m_fVelocity = info.fOverrideVelocity;
	}
	else if (info.bAltFire)
	{
		m_fVelocity = (LTFLOAT) (m_pAmmoData->pProjectileFX ? m_pAmmoData->pProjectileFX->nAltVelocity : 0);
	}
	else
	{
		m_fVelocity = (LTFLOAT) (m_pAmmoData->pProjectileFX ? m_pAmmoData->pProjectileFX->nVelocity : 0);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Setup
//
//	PURPOSE:	Set up a projectile with the information needed
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::Setup(CWeapon const* pWeapon, WeaponFireInfo const & info)
{
	if (!pWeapon || !info.hFiredFrom || !g_pWeaponMgr) return LTFALSE;

	ProjectileInitFileGlobals();

	// get the fire position
	m_vFirePos          = info.vFirePos;

	// get the flash position
	m_vFlashPos         = info.vFlashPos;

	// get the direction
	m_vDir              = info.vPath;

	// make sure its normalized
	m_vDir.Normalize();

	// determined who fired the projectile
	m_hFiredFrom        = info.hFiredFrom;

	// RETAIL: Tron's: the shooter's relation data, for the stimuli below
	// (0x1011eaa2)
	if (info.pRelationData)
	{
		*m_pRelationData = *info.pRelationData;
		m_bHasRelationData = true;
	}
	else
	{
		m_bHasRelationData = false;
	}

	// RETAIL: Tron's weapon modifiers (0x1011eac8)
	CopyModifiers( info );

	// get the weapon/ammo ids
	m_nWeaponId         = pWeapon->GetId();
	m_nAmmoId           = pWeapon->GetAmmoId();

	// determine if this projectile can impact
	// projectiles of the same kind
	if ( WMGR_INVALID_ID != m_nAmmoId )
	{
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
		if ( ( 0 != pAmmo ) && ( 0 != pAmmo->pProjectileFX ) )
		{
			m_bCanHitSameProjectileKind = ( 0 != pAmmo->pProjectileFX->nCanImpactSameKind );
		}
	}

	// set the lifetime of the projectile
	m_fLifeTime         = pWeapon->GetLifeTime();

	m_hFiredFrom		= info.hFiredFrom;
	m_nWeaponId			= pWeapon->GetId();
	m_nAmmoId			= pWeapon->GetAmmoId();
	m_fLifeTime			= pWeapon->GetLifeTime();

	m_pWeaponData		= pWeapon->GetWeaponData();
	if (!m_pWeaponData) return LTFALSE;

	m_pAmmoData			= pWeapon->GetAmmoData();
	if (!m_pAmmoData) return LTFALSE;

	// setup the damage info
	// RETAIL: Tron scales the instant damage by the fire info's multiplier
	// (0x1011eb74), and a scaled damage of type 0x28 becomes type 0x3d
	m_fInstDamage       = pWeapon->GetInstDamage() * info.fDamageMultiplier;
	m_fProgDamage       = pWeapon->GetProgDamage();
	if (m_pAmmoData->eInstDamageType == (DamageType)0x28 && info.fDamageMultiplier > 1.95)
	{
		m_eInstDamageType	= (DamageType)0x3d;
	}
	else
	{
		m_eInstDamageType	= m_pAmmoData->eInstDamageType;
	}
	m_eProgDamageType	= m_pAmmoData->eProgDamageType;

	// determine the velocity
	// RETAIL: Tron does it in a virtual (slot 28, 0x1011e970; Setup calls
	// it at 0x1011ebc5)
	TronUnknownVirtual28( info );

	// determine the projectile's range
	// RETAIL: the fire info can give its own (0x1011ebc8)
	if (info.bOverrideRange)
	{
		m_fRange		= info.fOverrideRange;
	}
	else
	{
		m_fRange		= (LTFLOAT) m_pWeaponData->nRange;
	}

	// get the special case stuff
	m_bSilenced         = !!(pWeapon->GetSilencer());

	// determine ammo type
	AmmoType eAmmoType  = m_pAmmoData->eType;

	// no calls to add impact yet
	m_bNumCallsToAddImpact = 0;

	// Client-side hit detection
	m_vClientObjImpactPos	= info.vClientObjImpactPos;
	m_hClientObjImpact		= info.hClientObjImpact;

	// RETAIL: Tron keeps the ricochet limits (0x1011ec5e)
	if ( m_pAmmoData && m_pAmmoData->pProjectileFX )
	{
		m_fMaxRicochetAngle	= m_pAmmoData->pProjectileFX->fMaxRicochetAngle;
		m_nMaxRicochets		= m_pAmmoData->pProjectileFX->nMaxRicochets;
	}

	// RETAIL: modifier 9's damage (0x1011ec90)
	if ( m_aModifiers[9].bEnabled )
	{
		m_bTronUnknown7C	= true;
		m_fTronUnknown80	= m_vModifier9.y;
		m_fTronUnknown84	= m_vModifier9.x;
	}
	else
	{
		m_bTronUnknown7C	= false;
		m_fTronUnknown80	= 0.0f;
		m_fTronUnknown84	= 0.0f;
	}

	// [RP] - Beyond this point nothing else can justify a bad setup so set a successful
	//		  setup here.  This way calls that rely on a successful setup will have accurate info.

	m_bSetup = true;

	// See if we start inside the test object...
	// If we are inside the test object, hit that
	// test object right away, and DON'T spawn
	// a projectile or a vector.
	if (!TestInsideObject(info.hTestObj, eAmmoType))
	{
		// the projectile is NOT inside the test object
		if (eAmmoType == PROJECTILE)
		{
			//
			DoProjectile();
		}
		else if (eAmmoType == VECTOR)
		{
			DoVector();
		}
	}

	// register a Enemy Weapon Fire Sound AND a Ally WeaponFireSound

	// Get the Distance that fire noise carries
	LTFLOAT fWeaponFireNoiseDistance = (LTFLOAT)pWeapon->GetWeaponData()->nAIFireSoundRadius;

	// If we're silenced use the radius specified by the silencer...
	if (m_bSilenced)
	{
		const MOD* pMod = pWeapon->GetSilencer();
		if (pMod)
		{
			fWeaponFireNoiseDistance = (LTFLOAT) pMod->nAISilencedFireSndRadius;
		}
	}

	// RETAIL: Tron's stimuli take the shooter's relation data (0x1011ed5a)
	if( fWeaponFireNoiseDistance > 0.f && m_bHasRelationData )
	{
		g_pAIStimulusMgr->RegisterStimulus(kStim_EnemyWeaponFireSound, m_hFiredFrom, *m_pRelationData, m_vFirePos, fWeaponFireNoiseDistance, 1.f);
		g_pAIStimulusMgr->RegisterStimulus(kStim_AllyWeaponFireSound, m_hFiredFrom, *m_pRelationData, m_vFirePos, fWeaponFireNoiseDistance, 1.f);
		g_pAIStimulusMgr->RegisterStimulus(kStim_ProhibitedWeaponFireSound, m_hFiredFrom, *m_pRelationData, m_vFirePos, fWeaponFireNoiseDistance, 1.f);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TouchFilter
//
//	PURPOSE:	RETAIL: Tron's (0x1011edf0): the new collision code's
//				intersect filter.  A character or a hit box is never
//				taken here: when the projectile hits its hit box, the touch
//				is kept.  The name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::TouchFilter(HOBJECT hObj)
{
	if (IsFiredFromHitBox(hObj)) return false;

	if (!IsCharacter(hObj) && !IsCharacterHitBox(hObj)) return true;

	IntersectInfo iInfo;
	HOBJECT hHitObj = hObj;
	if (GetHitBoxImpact(hObj, &hHitObj, &iInfo))
	{
		AddPendingTouch(hHitObj, &iInfo);
		return false;
	}

	m_bTronUnknown1C2 = true;
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TestInsideObject
//
//	PURPOSE:	Test to see if the projectile is inside the test object
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType)
{
	// This appears to be for the AIs to test the object they are
	// shooting at, apparently for the case where the AI is standing
	// on top of its target.  I'm guessing that if the projectile
	// gets 1 update before checking if it hit anything, there is a
	// chance the projectile will go through the targe.  If so, this
	// function will see that it is already "inside" the object, and
	// hit it immediately.

	// return if there is no test object
	if (!hTestObj) return LTFALSE;

	// TO DO???
	// NOTE:  This code may need to be updated to use test the dims
	// of the CharacterHitBox instead of the dims of the object...
	// TO DO???

	// See if we are inside the test object...

	LTVector vTestPos, vTestDims;
	g_pLTServer->GetObjectPos(hTestObj, &vTestPos);
	g_pPhysicsLT->GetObjectDims(hTestObj, &vTestDims);

	if (m_vFirePos.x < vTestPos.x - vTestDims.x ||
		m_vFirePos.x > vTestPos.x + vTestDims.x ||
		m_vFirePos.y < vTestPos.y - vTestDims.y ||
		m_vFirePos.y > vTestPos.y + vTestDims.y ||
		m_vFirePos.z < vTestPos.z - vTestDims.z ||
		m_vFirePos.z > vTestPos.z + vTestDims.z)
	{
		// NOT inside the object, proceed as normal
		return LTFALSE;
	}


	// We're inside the object, so we automatically hit the object...
	// RETAIL: Tron's order (0x1011ef7b): a projectile detonates, and the
	// impact comes before a vector's damage

	if (eAmmoType == PROJECTILE)
	{
		// its a projectile, just detonate
		Detonate(hTestObj);
		RemoveObject();
		return LTTRUE;
	}

	LTVector vNormal(0, 1, 0);
	AddImpact(hTestObj, m_vFlashPos, vTestPos, vNormal, GetSurfaceType(hTestObj));

	if (eAmmoType == VECTOR)
	{
		if (IsCharacter(hTestObj))
		{
			// If the test object is a character, add to the amount of
			// instant damage it will do based on the model node hit.

			CCharacter *pChar = (CCharacter*) g_pLTServer->HandleToObject(hTestObj);
			if (!pChar) return LTFALSE;

			ModelNode eModelNode = g_pModelButeMgr->GetSkeletonDefaultHitNode(pChar->GetModelSkeleton());

			pChar->SetModelNodeLastHit(eModelNode);

			//DANO: POTENTIAL PROBLEM!!
			// This variable m_fInstDamage gets changed here but
			// used by another function...in effect this variable
			// is getting passed "under the hood".  This can present
			// big problems with the Sequencer!
			AdjustDamage(pChar->ComputeDamageModifier(eModelNode));
		}

		ImpactDamageObject(m_hFiredFrom, hTestObj);
	}

	RemoveObject();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AdjustDamage()
//
//	PURPOSE:	Adjust the instant damage by a modifier
//
// ----------------------------------------------------------------------- //

void CProjectile::AdjustDamage(LTFLOAT fModifier)
{
	// Only adjust the damage if we are using an adjustable damage type...

	if (m_pAmmoData->bCanAdjustInstDamage)
	{
		m_fInstDamage *= fModifier; 
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

void CProjectile::InitialUpdate(int nInfo)
{
	// projectiles move alot, make the position updates unguaranteed
	// to help bandwidth
	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED);

	//
	if (nInfo == INITIALUPDATE_SAVEGAME)
	{
		// RETAIL: Tron sets the dims here too (0x1011f0c8)
		g_pPhysicsLT->SetObjectDims(m_hObject, &m_vDims, 0);
		return;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_MOVEABLE, USRFLG_MOVEABLE);
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_SERVERDIMS, FLAG2_SERVERDIMS);

	// RETAIL: Tron clears the solid flag (0x1011f116)
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_SOLID);

	// setup the damage object
	m_damage.Init(m_hObject);
	m_damage.SetMass(m_fMass);
	m_damage.SetHitPoints(1.0f);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetCanHeal(LTFALSE);
	m_damage.SetCanRepair(LTFALSE);
	m_damage.SetApplyDamagePhysics(LTFALSE);

	// include our default "can't damage" types
	// in the damage object
	DamageFlags nDamageFlags = m_damage.GetCantDamageFlags();
	m_damage.SetCantDamageFlags(nDamageFlags | m_nCantDamageFlags);

	// RETAIL: the time of the last update (0x1011f1b5)
	m_xTronUnknownB0 = g_pLTServer->GetTime();

	// prepare the next update
	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Currently this is NOT set to the model's dimensions.
	// Should it be?
	g_pPhysicsLT->SetObjectDims(m_hObject, &m_vDims, 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void CProjectile::Update()
{
	// RETAIL: Tron's (0x1011f1f4)
	UpdatePendingPos();

	// RETAIL: the new collision code handles the touches it kept
	// (0x1011f224)
	if (g_vtProjectileNewCollision.GetFloat() != 0.0f)
	{
		m_bTronUnknown1C0 = true;
		TronUnknownVirtual32();
		m_bTronUnknown1C1 = false;
		m_bTronUnknown1C2 = false;
		m_bTronUnknown1C0 = false;
	}
	// Process moving us through an invisible object if necessary.  This is
	// done here since the position/velocity can't be updated in the touch
	// notify (since the engine is in the process of updating it)...
	else if (m_bProcessInvImpact)
	{
		m_bProcessInvImpact = false;
		g_pPhysicsLT->SetVelocity(m_hObject, &m_vInvisVel);
		g_pLTServer->SetObjectPos(m_hObject, &m_vInvisNewPos);
	}

	// RETAIL: forget the objects damaged long enough ago (0x1011f285)
	UpdateDamagedObjects();

	m_xTronUnknownB0 = g_pLTServer->GetTime();

	// setup the next update
	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Detonate if the life time has expired
	if (g_pLTServer->GetTime() >= (m_fStartTime + m_fLifeTime))
	{
		Detonate(LTNULL);
	}

	// RETAIL: keep the position and the velocity (0x1011f2da)
	g_pLTServer->GetObjectPos(m_hObject, &m_vLastPos);
	g_pPhysicsLT->GetVelocity(m_hObject, &m_vLastVel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::IsIgnoringProjectiles
//
//	PURPOSE:	RETAIL: Tron's (0x1011f310): true when the object (or the
//				hit box of a hit box user) ignores projectiles.  The name
//				is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::IsIgnoringProjectiles(HOBJECT hObj)
{
	IHitBoxUser* pHitBoxUser = dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(hObj));
	if (pHitBoxUser)
	{
		hObj = pHitBoxUser->GetHitBoxObject();
	}

	if (!hObj) return true;

    uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUsrFlags);
	return (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) ? true : false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TronUnknownVirtual31
//
//	PURPOSE:	RETAIL: slot 31 (0x1011f380).  The name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::TronUnknownVirtual31(HOBJECT hAttacker, HOBJECT hVictim)
{
	return CanCharacterHitCharacter( hAttacker, hVictim );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::IsFiredFromHitBox
//
//	PURPOSE:	RETAIL: Tron's (0x1011f3a0): true for the hit box of the
//				object that fired us.  The name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::IsFiredFromHitBox(HOBJECT hObj)
{
	if (IsCharacterHitBox(hObj))
	{
		CCharacterHitBox *pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(hObj));
		if (pHitBox)
		{
			hObj = pHitBox->GetModelObject();
		}
	}

	if (!hObj) return false;

	return (hObj == m_hFiredFrom) ? true : false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::IsSameKindFromSameShooter
//
//	PURPOSE:	RETAIL: Tron's (0x1011f400): true for a projectile that is
//				not set up yet or that the object that fired us fired.  The
//				name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::IsSameKindFromSameShooter(HOBJECT hObj)
{
	if (IsKindOf(hObj, "CProjectile"))
	{
		CProjectile *pProjectile = dynamic_cast<CProjectile*>(g_pLTServer->HandleToObject(hObj));
		if (!pProjectile || !pProjectile->m_bSetup) return true;

		if (pProjectile->GetFiredFrom() == m_hFiredFrom) return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::IsNonSolidTouch
//
//	PURPOSE:	RETAIL: Tron's (0x1011f470): true for an object (not the
//				world) that is neither solid, a hit box nor ray hit.  The
//				name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::IsNonSolidTouch(HOBJECT hObj)
{
	LTBOOL bIsWorld = IsMainWorld(hObj);

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
	if (!bIsWorld && !(dwFlags & FLAG_SOLID) && !IsCharacterHitBox(hObj) && !(dwFlags & FLAG_RAYHIT))
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::GetHitBoxImpact
//
//	PURPOSE:	RETAIL: Tron's (0x1011f4e0): when we hit the hit box of the
//				object (or the object is a hit box), give the object the
//				hit box belongs to.  The name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::GetHitBoxImpact(HOBJECT hObj, HOBJECT *phObj, IntersectInfo *pInfo)
{
	CCharacterHitBox *pHitBox = LTNULL;

	IHitBoxUser* pHitBoxUser = dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(hObj));
	if (pHitBoxUser)
	{
		pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(pHitBoxUser->GetHitBoxObject()));
	}

	if (IsCharacterHitBox(hObj))
	{
		pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(hObj));
	}

	if (pHitBox)
	{
		LTVector vFrom = m_vFirePos;
		if (pHitBox->HandleImpactFromPos(this, *pInfo, m_vDir, vFrom))
		{
			*phObj = pHitBox->GetModelObject();
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TestTouchSegment
//
//	PURPOSE:	RETAIL: Tron's (0x1011f5f0): trace the way we moved in the
//				last frame.  The name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::TestTouchSegment(HOBJECT hObj, IntersectInfo *pInfo)
{
	LTVector vPos, vVel;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

	// determine how much we've travelled this frame
	vVel *= g_pLTServer->GetFrameTime();

	IntersectQuery qInfo;
	qInfo.m_Flags		= INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_From		= vPos - vVel;
	qInfo.m_To			= vPos;
	qInfo.m_FilterFn	= ProjectileTouchFilterFn;
	qInfo.m_pUserData	= this;

	return g_pLTServer->IntersectSegment(&qInfo, pInfo) ? true : false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::GetNearestPendingTouch
//
//	PURPOSE:	RETAIL: Tron's (0x1011f6f0): the kept touch nearest to the
//				last position.  The name is ours
//
// ----------------------------------------------------------------------- //

ProjectilePendingTouch* CProjectile::GetNearestPendingTouch(LTFLOAT *pfDistSqr)
{
	LTFLOAT fDistSqr = 0.0f;
	if (!pfDistSqr)
	{
		pfDistSqr = &fDistSqr;
	}

	ProjectilePendingTouchList::iterator iterNearest = m_lstPendingTouches.end();
	for (ProjectilePendingTouchList::iterator iter = m_lstPendingTouches.begin(); iter != m_lstPendingTouches.end(); ++iter)
	{
		if (iterNearest == m_lstPendingTouches.end() || iter->fDistSqr < *pfDistSqr)
		{
			*pfDistSqr = iter->fDistSqr;
			iterNearest = iter;
		}
	}

	if (iterNearest != m_lstPendingTouches.end())
	{
		return &(*iterNearest);
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TronUnknownVirtual32
//
//	PURPOSE:	RETAIL: slot 32 (0x1011f760): handle the kept touches,
//				nearest first, until one is taken.  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::TronUnknownVirtual32()
{
	ProjectilePendingTouch *pTouch;
	while ((pTouch = GetNearestPendingTouch(LTNULL)) != LTNULL)
	{
		m_fInstDamage = pTouch->fInstDamage;
		if (TronUnknownVirtual33(pTouch)) break;

		for (ProjectilePendingTouchList::iterator iter = m_lstPendingTouches.begin(); iter != m_lstPendingTouches.end(); ++iter)
		{
			if ((HOBJECT)iter->hObject == (HOBJECT)pTouch->hObject)
			{
				m_lstPendingTouches.erase(iter);
				break;
			}
		}
	}

	m_lstPendingTouches.clear();
	m_bProcessInvImpact = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TronUnknownVirtual33
//
//	PURPOSE:	RETAIL: slot 33 (0x1011f820): handle one kept touch; true
//				when it is taken.  The name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::TronUnknownVirtual33(ProjectilePendingTouch *pTouch)
{
	if (!pTouch) return false;

	SurfaceType eType = GetSurfaceType(pTouch->collInfo);
	if (eType == ST_SKY)
	{
		// we hit the sky
		HandleTouchSky(pTouch->hObject);
		return true;
	}
	else if (eType == ST_INVISIBLE)
	{
		if (m_bProcessInvImpact)
		{
			m_bProcessInvImpact = false;

			LTVector vNewPos = m_vLastPos + (m_vLastVel * g_pLTServer->GetFrameTime());
			g_pPhysicsLT->SetVelocity(m_hObject, &m_vLastVel);
			g_pLTServer->SetObjectPos(m_hObject, &vNewPos);
		}

		return false;
	}

	Detonate(pTouch->hObject, &pTouch->collInfo);
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddPendingTouch
//
//	PURPOSE:	RETAIL: Tron's (0x1011f940): keep a touch for the next
//				update (once for each object).  The name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::AddPendingTouch(HOBJECT hObj, IntersectInfo *pInfo)
{
	LTVector vLastPos = m_vLastPos;
	LTVector vLastVel = m_vLastVel;

	for (ProjectilePendingTouchList::iterator iter = m_lstPendingTouches.begin(); iter != m_lstPendingTouches.end(); ++iter)
	{
		if (iter->hObject == hObj) return false;
	}

	LTVector vDiff = pInfo->m_Point - vLastPos;
	LTFLOAT fDistSqr = vDiff.MagSqr();

	CollisionInfo collInfo;
	g_pLTServer->GetLastCollision(&collInfo);

	if (!m_bProcessInvImpact)
	{
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(collInfo.m_hObject, OFT_Flags, dwFlags);
		if (dwFlags & FLAG_SOLID)
		{
			m_bProcessInvImpact = true;
		}
	}

	LTFLOAT fInstDamage = m_fInstDamage;

	m_lstPendingTouches.push_back( ProjectilePendingTouch( pInfo->m_hObject, *pInfo, fInstDamage,
		fDistSqr, vLastPos, vLastVel, collInfo ));

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::LoadPendingTouches / SavePendingTouches
//
//	PURPOSE:	RETAIL: Tron's (0x1011fb50, 0x1011fca0): the kept touches
//				are saved whole.  The names are ours
//
// ----------------------------------------------------------------------- //

void CProjectile::LoadPendingTouches(ILTMessage_Read *pMsg)
{
	m_lstPendingTouches.clear();

	int nCount = pMsg->Readuint8();
	for (int i = 0; i < nCount; ++i)
	{
		ProjectilePendingTouch touch;
		pMsg->ReadType(&touch);
		m_lstPendingTouches.push_back(touch);
	}
}

void CProjectile::SavePendingTouches(ILTMessage_Write *pMsg)
{
	pMsg->Writeuint8(m_lstPendingTouches.size());

	for (ProjectilePendingTouchList::iterator iter = m_lstPendingTouches.begin(); iter != m_lstPendingTouches.end(); ++iter)
	{
		pMsg->WriteData(&(*iter), sizeof(ProjectilePendingTouch) * 8);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TronUnknownVirtual30
//
//	PURPOSE:	RETAIL: slot 30 (0x1011fd10): the new collision code's
//				touch: keep the touch for the next update when we really
//				hit the object.  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::TronUnknownVirtual30(HOBJECT hObj)
{
	if (!m_bSetup) return;
	if (m_bObjectRemoved) return;
	if (!hObj) return;

	if (!m_bCanTouchFiredFromObj && IsFiredFromHitBox(hObj)) return;

	if (IsIgnoringProjectiles(hObj)) return;

	if (!TronUnknownVirtual31(m_hFiredFrom, hObj)) return;

	if (!m_bCanHitSameProjectileKind && IsSameKindFromSameShooter(hObj)) return;

	if (IsNonSolidTouch(hObj)) return;

	IntersectInfo iInfo;
	HOBJECT hHitObj;
	if (GetHitBoxImpact(hObj, &hHitObj, &iInfo))
	{
		AddPendingTouch(hHitObj, &iInfo);
		return;
	}

	if (TestTouchSegment(hObj, &iInfo))
	{
		AddPendingTouch(hObj, &iInfo);
		return;
	}

	m_bTronUnknown1C1 = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleTouch(HOBJECT hObj)
{
	if (!m_bSetup) return;
	if (m_bObjectRemoved) return;

	// Don't process any touches until this has been cleared...
	if (m_bProcessInvImpact) return;

	 // Let it get out of our bounding box...
	if( (hObj == m_hFiredFrom) && !m_bCanTouchFiredFromObj ) return;

	// If we've hit a hit box user (a character, a body...), let its hit box
	// take control...
	// RETAIL: Tron asks the IHitBoxUser (0x1011feac)
	IHitBoxUser* pHitBoxUser = dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(hObj));
	if (pHitBoxUser)
	{
		hObj = pHitBoxUser->GetHitBoxObject();
	}

	// See if we want to impact on this object...
    uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUsrFlags);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	// get the hitbox
	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(hObj))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		// can't hit ourselves
		if( (pHitBox->GetModelObject() == m_hFiredFrom) && !m_bCanTouchFiredFromObj ) return;
	}

	
	// Handle special AI alignment cases and multiplayer friendly fire...

	if( !CanCharacterHitCharacter( m_hFiredFrom, hObj ))
		return;


	// can we hit projectiles of the same kind?
	if ( LTFALSE == m_bCanHitSameProjectileKind )
	{
		// Case where we don't hit our own type of projectiles 
		// (for multi-projectile weapons and projectiles 
		// that stick to objects)...
		if (IsKindOf(hObj, "CProjectile"))
		{
			CProjectile* pObj = (CProjectile*)g_pLTServer->HandleToObject(hObj);
			if ( !pObj->m_bSetup ) return;
			if ( pObj )
			{
				if (pObj->GetFiredFrom() == m_hFiredFrom)
				{
					return;
				}
			}
		}
	}

	LTBOOL bIsWorld = IsMainWorld(hObj);

	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
	if (!bIsWorld && !(dwFlags & FLAG_SOLID))
	{
		if (pHitBox)
		{
			// See if we really impacted on the box...
			if (pHitBox->DidProjectileImpact(this))
			{
				// This is the object that we really hit...
				hObj = pHitBox->GetModelObject();
			}
			else
			{
				return;
			}
		}
		else if (!(dwFlags & FLAG_RAYHIT))
		{
			// If we have ray hit set to true, projectiles should
			// impact on us too...
			return;
		}
	}

	// we hit the world, get details
	if (bIsWorld || (OT_WORLDMODEL == GetObjectType(hObj)))
	{
		CollisionInfo info;
		g_pLTServer->GetLastCollision(&info);

		// determine the surface type we hit
		SurfaceType eType = GetSurfaceType(info);

		if (eType == ST_SKY)
		{
			// we hit the sky
			HandleTouchSky( hObj );
			return;
		}
		else if (eType == ST_INVISIBLE)
		{
			// Ignore the impact if it's non-solid
			uint32 nObjFlags;
			g_pCommonLT->GetObjectFlags(info.m_hObject, OFT_Flags, nObjFlags);
			if ((nObjFlags & FLAG_SOLID) == 0)
			{
				return;
			}

			// we hit an invisible object
			// we have to handle this is the update function
			
			// set the flag so we know to process it later
			m_bProcessInvImpact = true;

			// prepare the info we need to process it
			g_pLTServer->GetObjectPos(m_hObject, &m_vInvisNewPos);
			g_pPhysicsLT->GetVelocity(m_hObject, &m_vInvisVel);
			m_vInvisNewPos += (m_vInvisVel * g_pLTServer->GetFrameTime());

			// Make sure this new position is inside the world
			if (LT_INSIDE == g_pCommonLT->GetPointStatus(&m_vInvisNewPos))
			{
				return;
			}
		}
		else if ( ( m_fMaxRicochetAngle > 0.0f ) &&
		          ( m_nMaxRicochets > m_nTotalRicochets ) )
		{
			// check if we have ricocheted this frame
			if ( m_nLastRicochetUpdateNum >= m_nUpdateNum ) 
			{
				// we have ricocheted, don't do it again
				return;
			}

			//
			// check for a ricochet
			//

			// TODO: this duplicates the code in HandleImpact,
			// set it up so this stuff doesn't have to be done twice.
			// (Its not the cheapest in the world).


			LTRESULT ltResult;

			// get the collision info for this impact
			CollisionInfo colInfo;
			ltResult = g_pLTServer->GetLastCollision(&colInfo);
			ASSERT( LT_OK == ltResult );

			// get the normal of the plane we impacted with
			LTPlane plane = colInfo.m_Plane;
			LTVector vNormal = plane.m_Normal;

			//
			// Calculate where we really hit the plane
			// and make sure we don't tunnel through an object
			//

			LTVector vVel, vP0, vP1, vDir;
			// get the velocity of the projectile
			ltResult = g_pPhysicsLT->GetVelocity(m_hObject, &vVel);
			ASSERT( LT_OK == ltResult );

			// get the direction of the projectile
			vDir = vVel;
			vDir.Normalize();

			// determine how much we've travelled this frame
			vVel *= g_pLTServer->GetFrameTime();

			// get the position
			LTVector vPos;
			ltResult = g_pLTServer->GetObjectPos(m_hObject, &vPos);
			ASSERT( LT_OK == ltResult );

			// get a point just a little in front and behind the impact point
			vP0 = vPos - vVel;  // a little "behind" of the impact point
			vP1 = vPos + vVel;  // a littel "forward" of the impact point

			// throw an intersect segment to determine where we really hit
			IntersectInfo iInfo;
			IntersectQuery qInfo;

			// fill out the info for this test
			qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_From	  = vP0;
			qInfo.m_To		  = vPos;
			qInfo.m_FilterFn  = SpecificObjectFilterFn;
			qInfo.m_pUserData = m_hObject;

			if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
			{
				// we did hit the plane
				
				// get the intersect information
				vPos    = iInfo.m_Point - vDir;
				eType   = GetSurfaceType(iInfo);
				vNormal = iInfo.m_Plane.m_Normal;
			}
			else
			{
				// plane was NOT hit

				// fake the impact position
				LTFLOAT fDot1 = VEC_DOT(vNormal, vP0) - colInfo.m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT(vNormal, vP1) - colInfo.m_Plane.m_Dist;

				if ( ( ( fDot1 < 0.0f ) && ( fDot2 < 0.0f ) ) ||
				     ( ( fDot1 > 0.0f ) && ( fDot2 > 0.0f ) ) )
				{
					vPos = vP1;
				}
				else
				{
					LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
					VEC_LERP(vPos, vP0, vP1, fPercent);
				}
			}

			LTFLOAT fAngleFromNormal;
			LTVector vReversedNormal = -vNormal;
			fAngleFromNormal = ltacosf( VEC_DOT( vReversedNormal, vDir ) );
			
			if ( ( MATH_HALFPI - fAngleFromNormal ) < m_fMaxRicochetAngle )
			{
				// ricochet the projectile

				// increase the ricochet count
				m_nTotalRicochets += 1;

				// reflect the direction
				LTVector vReflectedDir;
				vReflectedDir = vDir - 2 * vNormal * ( VEC_DOT( vDir, vNormal ) );

				// normalize and scale to velocity
				// RETAIL: Tron's scales by the projectile's velocity (0x10120645)
				LTVector vNewVel = vReflectedDir;

				// keep track of the update num to prevent multiple ricochets in the same frame
				m_nLastRicochetUpdateNum = m_nUpdateNum;

				vNewVel *= m_fVelocity;

				// RETAIL: Tron takes away the stopping velocity that the
				// engine will apply (0x10120681)
				vNewVel -= colInfo.m_vStopVel;

				// set the new speed
				ltResult = g_pPhysicsLT->SetVelocity(m_hObject, &vNewVel);
				ASSERT( LT_OK == ltResult );

				// RETAIL: Tron's projectile flies on from the ricochet
				// (0x101206b0)
				m_vFirePos = vPos;
				m_vDir = vReflectedDir;

				// do all the impact work
				AddImpact( hObj, m_vFlashPos, vPos, vNormal, eType, IMPACT_TYPE_RICOCHET );

				// RETAIL: the ricochet damages what it hits (0x101206f7)
				ImpactDamageObject( m_hFiredFrom, hObj );

				return;
			}
		}
	}

	// we hit something
	HandleImpact(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleTouchSky()
//
//	PURPOSE:	Handle cases where projectile touches the sky
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleTouchSky(HOBJECT hObj)
{
	// just quietly get rid of the projectile
	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleImpact()
//
//	PURPOSE:	Allow sub-classes to handle impacts...Default is to
//				go boom.
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleImpact(HOBJECT hObj)
{
	// RETAIL: 0x10120730 gives no collision.
	Detonate(hObj, LTNULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProjectile::Detonate(HOBJECT hObj, CollisionInfo* pCollInfo)
{
	if (m_bDetonated) return;

	//DANO: a better place for this?
	// Make sure we don't detonate if a cinematic is playing (i.e.,
	// make sure the user doesn't disrupt the cinematic)...
	if (Camera::IsActive())
	{
		RemoveObject();
		return;
	}

	// mark the projectile as detonated
	m_bDetonated = true;

	// eventually we will determine the surface type
	SurfaceType eType = ST_UNKNOWN;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Determine the normal of the surface we are impacting on...
	LTVector vNormal(0.0f, 1.0f, 0.0f);

	// if hObj in non-null, it is detonating on something
	if (hObj)
	{
		// projectile is detonating against something
		if (IsMainWorld(hObj) || GetObjectType(hObj) == OT_WORLDMODEL)
		{
			// get the collision info for this impact
			// RETAIL: the new collision code gives the kept touch's
			// (0x10120803)
			// (each branch has its own CollisionInfo, and the pointer is
			// used after its scope)
			if (g_vtProjectileNewCollision.GetFloat() != 0.0f)
			{
				if (!pCollInfo)
				{
					CollisionInfo info;
					g_pLTServer->GetLastCollision(&info);
					pCollInfo = &info;
				}
			}
			else
			{
				CollisionInfo info;
				g_pLTServer->GetLastCollision(&info);
				pCollInfo = &info;
			}

			// check if we have a valid polygon
			if (pCollInfo->m_hPoly != INVALID_HPOLY)
			{
				// get the surface type
				eType = GetSurfaceType(pCollInfo->m_hPoly);
			}

			// get the normal of the plane we impacted with
			LTPlane plane = pCollInfo->m_Plane;
			vNormal = plane.m_Normal;

			//
			// Calculate where we really hit the plane
			// and make sure we don't tunnel through an object
			//

			LTVector vVel, vP0, vP1, vDir;
			// get the velocity of the projectile
			g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

			// get the direction of the projectile
			vDir = vVel;
			vDir.Normalize();

			// determine how much we've travelled this frame
			vVel *= g_pLTServer->GetFrameTime();

			// get a point just a little in front and behind the impact point
			vP0 = vPos - vVel;  // a little "behind" of the impact point
			vP1 = vPos + vVel;  // a littel "forward" of the impact point

			// throw an intersect segment to determine where we really hit
			IntersectInfo iInfo;
			IntersectQuery qInfo;

			// fill out the info for this test
			qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_From	  = vP0;
			qInfo.m_To		  = vPos;
			qInfo.m_FilterFn  = SpecificObjectFilterFn;
			qInfo.m_pUserData = m_hObject;

			if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
			{
				// we did hit the plane
				
				// get the intersect information
				vPos    = iInfo.m_Point - vDir;
				eType   = GetSurfaceType(iInfo);
				vNormal = iInfo.m_Plane.m_Normal;
			}
			else
			{
				// plane was NOT hit

				// fake the impact position
				LTFLOAT fDot1 = VEC_DOT(vNormal, vP0) - pCollInfo->m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT(vNormal, vP1) - pCollInfo->m_Plane.m_Dist;

				if ( ( ( fDot1 < 0.0f ) && ( fDot2 < 0.0f ) ) ||
				     ( ( fDot1 > 0.0f ) && ( fDot2 > 0.0f ) ) )
				{
					vPos = vP1;
				}
				else
				{
					LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
					VEC_LERP(vPos, vP0, vP1, fPercent);
				}
			}

			// reset the projectile's rotation
			LTRotation rRot(vNormal, LTVector(0.0f, 1.0f, 0.0f));
			g_pLTServer->SetObjectRotation(m_hObject, &rRot);
		}
	}
	else
	{
		// Since hObj was null, this means the projectile's lifetime was up,
		// so we just blew-up in the air.
		eType = ST_AIR;
	}

	// if the surface type has not been determined, try one more time
	if (eType == ST_UNKNOWN)
	{
		// get the object's surface type
		eType = GetSurfaceType(hObj);
	}

	// RETAIL: Tron has no deflection (NOLF2's Deflect; the cvar
	// "DeflectAngleRange" is only initialized, 0x1011e0d9)

	// RETAIL: Tron keeps where it detonated (0x10120dc3)
	m_vTronUnknownD0 = vPos;

	// do all the impact work
	AddImpact(hObj, m_vFlashPos, vPos, vNormal, eType);

	// Handle impact damage...
	if (hObj)
	{
		HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : m_hObject;

		ImpactDamageObject(hDamager, hObj);
	}

	// Remove projectile from world...
	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddImpact()
//
//	PURPOSE:	Add an impact object.
//
//	USED:		both vectors and projectiles
//
// ----------------------------------------------------------------------- //

void CProjectile::AddImpact( HOBJECT hObj,
                             const LTVector &vFirePos,
                             const LTVector &vImpactPos,
                             const LTVector &vSurfaceNormal,
                             SurfaceType eType,
                             IMPACT_TYPE eImpactType,
							 bool bDeflected)
{
	if ( !m_bSetup )
	{
		return;
	}

	// validate the pointers we will use
	ASSERT( 0 != g_pSurfaceMgr );
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pWeaponMgr );
	ASSERT( 0 != g_pAIStimulusMgr );
	ASSERT( 0 != m_pWeaponData );
	ASSERT( 0 != m_pAmmoData );

	// Create the client side (impact) weapon fx...
	CLIENTWEAPONFX fxStruct;
	fxStruct.hFiredFrom     = m_hFiredFrom;
	fxStruct.vSurfaceNormal = vSurfaceNormal;
	fxStruct.vFirePos       = vFirePos;
	fxStruct.vPos           = vImpactPos + (m_vDir * -1.0f);
	fxStruct.hObj           = hObj;
	fxStruct.nWeaponId      = m_pWeaponData->nId;
	fxStruct.nAmmoId        = m_pAmmoData->nId;
	fxStruct.nSurfaceType   = eType;
	fxStruct.wIgnoreFX      = g_wIgnoreFX;
	fxStruct.eImpactType    = eImpactType;
	fxStruct.m_xTronUnknown38 = 1.0f;

	// RETAIL: Tron marks the impacts of vector ammo (0x10120fad); the flag
	// has no name
	if (m_pAmmoData->eType == VECTOR)
	{
		g_wIgnoreFX |= (1<<15);
	}

	if (bDeflected)
		fxStruct.wIgnoreFX |= (WFX_MARK | WFX_EXITMARK);

	// Always use the flash position for the first call to AddImpact...
	//DANO: why?
	if (m_bNumCallsToAddImpact == 0)
	{
		fxStruct.vFirePos = m_vFlashPos;
	}

	// If we do multiple calls to AddImpact, make sure we only do some
	// effects once :)
	//DANO: mission an "if" statement?
	g_wIgnoreFX |= WFX_SHELL | WFX_LIGHT | WFX_MUZZLE;


	// Allow exit surface fx on the next call to AddImpact...
	//DANO: missing an "if" statement?
	g_wIgnoreFX &= ~WFX_EXITSURFACE;

	// limit the amount of exit marks
	if (IsMoveable(hObj))
	{
		// Well, don't do too many exit marks...The server will add one
		// if necessary...

		g_wIgnoreFX |= WFX_EXITMARK;
	}

	// If this is a player object, get the client id...
	if (IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
			fxStruct.nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}

	// RETAIL: Tron sends the client id of a player that blocked the
	// projectile (0x1012105a)
	if ((eImpactType == IMPACT_TYPE_BLOCKED) || (eImpactType == 3))	// 3: the disc's deadly block
	{
		if (IsPlayer(hObj))
		{
			CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hObj);
			if (pPlayer)
			{
				fxStruct.m_xTronUnknownF = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
			}
		}
	}

	// do all server side fx activities
	// and send a message to the client
	CreateClientWeaponFX(fxStruct);

	// Do the area and progressive (over time) damage...
	// RETAIL: Tron's ricochets make no explosion, and the ammo can always
	// explode (0x101210af)
	if ((eImpactType != IMPACT_TYPE_RICOCHET) &&
		(m_pAmmoData->bAreaDamageAlways ||
		 (m_pAmmoData->fAreaDamage > 0.0f && eType != ST_SKY) ||
		 m_pAmmoData->fProgDamageLifetime > 0.0f))
	{
		AddExplosion(vImpactPos, vSurfaceNormal);
	}

	// Update Character fire info...
	if (m_hFiredFrom && IsCharacter(m_hFiredFrom) && CanSetLastFireInfo())
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pChar)
		{
			CharFireInfo info;
			info.hObject    = hObj;
			info.vFiredPos  = m_vFlashPos;  // Use initial flash pos
			info.vImpactPos = vImpactPos;
			info.nWeaponId  = m_pWeaponData->nId;
			info.nAmmoId    = m_pAmmoData->nId;
			info.fTime      = g_pLTServer->GetTime();
			info.bSilenced  = m_bSilenced;
			info.eSurface   = eType;

			pChar->SetLastFireInfo(&info);
		}
	}

	// if the projectile was fired from a character
	// RETAIL: Tron's need the relation data (0x10121236)
	if(m_bHasRelationData && m_pRelationData && m_hFiredFrom && IsCharacter(m_hFiredFrom))
	{
		//
		// send a bunch of AI stimuli
		//

		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_pWeaponData->nId);
		ASSERT( 0 != pWeapon );

		// Send a HearEnemyWeaponImpact stimulus.
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_pAmmoData->nId);
		if ( (!pAmmo) || (!pAmmo->pImpactFX) ) return;
		ASSERT(pAmmo && pAmmo->pImpactFX);

		// Get the Distance that the impact noise carries
		LTFLOAT fWeaponImpactNoiseDistance = (LTFLOAT) pAmmo->pImpactFX->nAISoundRadius;
	
		// Scale based on surface types
		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eType);
		ASSERT(pSurf && "Surface type not found" );
		if( !pSurf ) return;

		if ( !pAmmo->pImpactFX->bAIIgnoreSurface )
		{
			fWeaponImpactNoiseDistance *= pSurf->fImpactNoiseModifier;
		}

		// note: the enemy part of kStim_EnemyWeaponImpactSound
		// is largly irrelevant, this will just register a weapon impact sound
		// and the AI can react however it wants

		if( fWeaponImpactNoiseDistance > 0.f )
		{
			g_pAIStimulusMgr->RegisterStimulus( (EnumAIStimulusType)pAmmo->pImpactFX->nAIStimulusType, pAmmo->pImpactFX->nAIAlarmLevel, m_hFiredFrom, hObj, vImpactPos, fWeaponImpactNoiseDistance);
			g_pAIStimulusMgr->RegisterStimulus(kStim_ProhibitedWeaponImpactSound, m_hFiredFrom, hObj, vImpactPos, fWeaponImpactNoiseDistance, 1.f);
		}

		// Send a SeeEnemyWeaponImpact stimulus, if it hit a character.
		if(IsCharacter(hObj))
		{
			// [RP] 8/08/02 - I really don't like special casing the damage types like this 
			//		but since we are so close to gold this is the safest way of excluding the
			//		tracking damage from registering a visible stimulus.
			// [JO] 8/14/02 - added skipping Unspecified, so neutral AI do not freak out when a coin comes near them.
			// [JO] 9/07/02 - added skipping camera disabler.

			// RETAIL: Tron checks only the unspecified damage (0x1012136b)
			if( ( pAmmo->eInstDamageType != DT_UNSPECIFIED ) || ( pAmmo->eProgDamageType != DT_UNSPECIFIED ) )
			{
				g_pAIStimulusMgr->RegisterStimulus(kStim_EnemyWeaponImpactVisible, m_hFiredFrom, hObj, vImpactPos, 1.f, 1.f);
				g_pAIStimulusMgr->RegisterStimulus(kStim_ProhibitedWeaponImpactVisible, m_hFiredFrom, hObj, vImpactPos, 1.f, 1.f);
			}
		}
	}

	// keep track of the number of times AddImpact is called
	m_bNumCallsToAddImpact++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CProjectile::AddExplosion(const LTVector &vPos, const LTVector &vNormal)
{
	ASSERT( 0 != g_pLTServer );

	//
	// create an explosion object
	//

	// get the class
	HCLASS hClass = g_pLTServer->GetClass("Explosion");
	if (!hClass) return;

	// get an Object Create Struct
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	// set the position
	theStruct.m_Pos = vPos;

	// set the rotation to the normal
	theStruct.m_Rotation = LTRotation(vNormal, vNormal);

	// create the object
	Explosion* pExplosion = (Explosion*)g_pLTServer->CreateObject(hClass, &theStruct);

	// validate its creation
	if (pExplosion)
	{
		// RETAIL: Tron sets the explosion up with message 0x3f3 and the
		// projectile's modifiers (Explosion::ObjectMessageFn 0x100ef790)
		CAutoMessage cMsg;
		cMsg.Writeuint32(0x3f3);
		cMsg.WriteObject(m_hFiredFrom);
		cMsg.Writeuint32(m_nAmmoId);
		cMsg.Writebool(true);
		cMsg.Writeuint32(m_nWeaponId);
		SaveModifiers(cMsg);
		g_pLTServer->SendToObject(cMsg.Read(), m_hObject, pExplosion->m_hObject, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void CProjectile::AddSpecialFX()
{
	ASSERT( 0 != g_pLTServer );

	//
	// This apparently only creates the special FX related the the firing
	// of the weapon.
	//

	if (!g_pWeaponMgr) return;

	// If this is a player object, get the client id...
	uint8 nShooterId = -1; 
	if (IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
			nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}


		// Create a special fx...
		CAutoMessage cMsg;

		cMsg.Writeuint8(SFX_PROJECTILE_ID);

		cMsg.Writeuint8(m_pWeaponData->nId);

		cMsg.Writeuint8(m_pAmmoData->nId);

		cMsg.Writeuint8(nShooterId);

		LTRESULT ltResult;
		ltResult = g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
		ASSERT( LT_OK == ltResult );


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::RemoveObject()
//
//	PURPOSE:	Remove the object, and do clean up (isle 4)
//
//	USED:		both vectors and projectiles
//
// ----------------------------------------------------------------------- //

void CProjectile::RemoveObject()
{
	if (m_bObjectRemoved) return;

	// NOTE: the actual removal doesn't happen until the
	// end of the frame
	g_pLTServer->RemoveObject(m_hObject);

	// make note that its been removed so it doesn't do anything silly
	m_bObjectRemoved = true;

	// RETAIL: Tron's (0x1012193f)
	RequestStopSendingMessages();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CopyModifiers
//
//	PURPOSE:	RETAIL: Tron's (0x10121950): take the weapon modifiers
//				from the fire info.  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::CopyModifiers(WeaponFireInfo const& info)
{
	for ( int i = 0; i < TRON_NUM_WEAPON_MODIFIERS; ++i )
	{
		m_aModifiers[i] = info.aModifiers[i];
	}

	if ( m_aModifiers[0].bEnabled )
		m_ModifierData.m0 = info.ModifierData.m0;

	if ( m_aModifiers[1].bEnabled )
		m_ModifierData.m1 = info.ModifierData.m1;

	if ( m_aModifiers[2].bEnabled )
		m_ModifierData.m0 = info.ModifierData.m0;

	if ( m_aModifiers[3].bEnabled )
		m_ModifierData.m3 = info.ModifierData.m3;

	if ( m_aModifiers[4].bEnabled )
		m_ModifierData.m4 = info.ModifierData.m4;

	if ( m_aModifiers[5].bEnabled )
		m_ModifierData.m4 = info.ModifierData.m4;

	if ( m_aModifiers[6].bEnabled )
		m_ModifierData.m6 = info.ModifierData.m6;

	if ( m_aModifiers[7].bEnabled )
		m_ModifierData.m7 = info.ModifierData.m7;

	if ( m_aModifiers[8].bEnabled )
		m_Modifier8 = info.Modifier8;

	if ( m_aModifiers[9].bEnabled )
		m_vModifier9 = info.vModifier9;

	if ( m_aModifiers[10].bEnabled )
		m_Modifier10 = info.Modifier10;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::LoadModifiers / SaveModifiers
//
//	PURPOSE:	RETAIL: Tron's (0x10121b00, 0x10121c70): the weapon
//				modifiers.  The names are ours
//
// ----------------------------------------------------------------------- //

void CProjectile::LoadModifiers(ILTMessage_Read *pMsg)
{
	for ( int i = 0; i < TRON_NUM_WEAPON_MODIFIERS; ++i )
	{
		m_aModifiers[i].Load( pMsg );
	}

	if ( m_aModifiers[0].bEnabled )
		pMsg->ReadType( &m_ModifierData.m0 );

	if ( m_aModifiers[1].bEnabled )
		pMsg->ReadType( &m_ModifierData.m1 );

	if ( m_aModifiers[2].bEnabled )
		pMsg->ReadType( &m_ModifierData.m0 );

	if ( m_aModifiers[3].bEnabled )
		pMsg->ReadType( &m_ModifierData.m3 );

	if ( m_aModifiers[4].bEnabled )
		pMsg->ReadType( &m_ModifierData.m4 );

	if ( m_aModifiers[5].bEnabled )
		pMsg->ReadType( &m_ModifierData.m4 );

	if ( m_aModifiers[6].bEnabled )
		pMsg->ReadType( &m_ModifierData.m6 );

	if ( m_aModifiers[7].bEnabled )
		pMsg->ReadType( &m_ModifierData.m7 );

	if ( m_aModifiers[8].bEnabled )
		pMsg->ReadType( &m_Modifier8 );

	if ( m_aModifiers[9].bEnabled )
		pMsg->ReadType( &m_vModifier9 );

	if ( m_aModifiers[10].bEnabled )
		pMsg->ReadType( &m_Modifier10 );
}

void CProjectile::SaveModifiers(ILTMessage_Write *pMsg)
{
	for ( int i = 0; i < TRON_NUM_WEAPON_MODIFIERS; ++i )
	{
		m_aModifiers[i].Save( pMsg );
	}

	if ( m_aModifiers[0].bEnabled )
		pMsg->WriteType( m_ModifierData.m0 );

	if ( m_aModifiers[1].bEnabled )
		pMsg->WriteType( m_ModifierData.m1 );

	if ( m_aModifiers[2].bEnabled )
		pMsg->WriteType( m_ModifierData.m0 );

	if ( m_aModifiers[3].bEnabled )
		pMsg->WriteType( m_ModifierData.m3 );

	if ( m_aModifiers[4].bEnabled )
		pMsg->WriteType( m_ModifierData.m4 );

	if ( m_aModifiers[5].bEnabled )
		pMsg->WriteType( m_ModifierData.m4 );

	if ( m_aModifiers[6].bEnabled )
		pMsg->WriteType( m_ModifierData.m6 );

	if ( m_aModifiers[7].bEnabled )
		pMsg->WriteType( m_ModifierData.m7 );

	if ( m_aModifiers[8].bEnabled )
		pMsg->WriteType( m_Modifier8 );

	if ( m_aModifiers[9].bEnabled )
		pMsg->WriteType( m_vModifier9 );

	if ( m_aModifiers[10].bEnabled )
		pMsg->WriteType( m_Modifier10 );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ScaleTronDamage...
//
//	PURPOSE:	RETAIL: Tron's (Object.lto 0x10121df0, 0x10121ea0,
//				0x10121ec0): scale a damage by the modifiers the projectile
//				has. The names are ours
//
// ----------------------------------------------------------------------- //

void CProjectile::ScaleTronDamage(LTFLOAT* pfDamage)
{
	if( m_aModifiers[0].bEnabled )
		*pfDamage *= m_ModifierData.m0.fTronUnknown[2];

	if( m_aModifiers[1].bEnabled )
		*pfDamage *= m_ModifierData.m1.fTronUnknown[0x22];

	if( m_aModifiers[2].bEnabled )
		*pfDamage *= m_ModifierData.m0.fTronUnknown[2];

	if( m_aModifiers[3].bEnabled )
		*pfDamage *= m_ModifierData.m3.fTronUnknown[1];

	if( m_aModifiers[4].bEnabled )
		*pfDamage *= m_ModifierData.m4.fDamageScale;

	if( m_aModifiers[5].bEnabled )
		*pfDamage *= m_ModifierData.m4.fDamageScale;

	if( m_aModifiers[6].bEnabled )
		*pfDamage *= m_ModifierData.m6.fTronUnknown[3];

	if( m_aModifiers[7].bEnabled )
		*pfDamage *= m_ModifierData.m7.fDamageScale;
}

void CProjectile::ScaleTronDamage101(LTFLOAT* pfDamage)
{
	if( m_aModifiers[8].bEnabled )
		*pfDamage *= m_Modifier8.fDamageScale;
}

void CProjectile::ScaleTronDamage105(LTFLOAT* pfDamage)
{
	if( m_pWeaponData->bIsPlayerPrimitive && m_aModifiers[10].bEnabled )
		*pfDamage *= m_Modifier10.fDamageScale;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddDamagedObject
//
//	PURPOSE:	RETAIL: Tron's (0x10121ef0): a projectile damages an
//				object once (the hit box counts as its object).  The name
//				is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::AddDamagedObject(HOBJECT hObj)
{
	if (!m_pAmmoData || m_pAmmoData->eType != PROJECTILE) return;

	HOBJECT hTestObj = hObj;
	if (IsCharacterHitBox(hObj))
	{
		CCharacterHitBox *pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(hObj));
		if (pHitBox)
		{
			hTestObj = pHitBox->GetModelObject();
		}
	}

	for (ProjectileDamagedObjectList::iterator iter = m_lstDamagedObjects.begin(); iter != m_lstDamagedObjects.end(); ++iter)
	{
		if (iter->hObject == hTestObj) return;
	}

	LTFLOAT fTime = g_pLTServer->GetTime();
	m_lstDamagedObjects.push_back( ProjectileDamagedObject( hObj, fTime ));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::UpdateDamagedObjects
//
//	PURPOSE:	RETAIL: Tron's (0x10122020): forget the objects damaged
//				long enough ago.  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::UpdateDamagedObjects()
{
	if (!m_pAmmoData || m_pAmmoData->eType != PROJECTILE) return;

	ProjectileDamagedObjectList::iterator iter = m_lstDamagedObjects.begin();
	while (iter != m_lstDamagedObjects.end())
	{
		if (iter->hObject && (s_fDamagedObjectTime + iter->fTime) >= g_pLTServer->GetTime())
		{
			++iter;
		}
		else
		{
			iter = m_lstDamagedObjects.erase(iter);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::IsDamagedObject
//
//	PURPOSE:	RETAIL: Tron's (0x101220a0).  The name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::IsDamagedObject(HOBJECT hObj)
{
	if (!m_pAmmoData || m_pAmmoData->eType != PROJECTILE) return false;

	if (IsCharacterHitBox(hObj))
	{
		CCharacterHitBox *pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(hObj));
		if (pHitBox)
		{
			hObj = pHitBox->GetModelObject();
		}
	}

	for (ProjectileDamagedObjectList::iterator iter = m_lstDamagedObjects.begin(); iter != m_lstDamagedObjects.end(); ++iter)
	{
		if (iter->hObject == hObj) return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::LoadDamagedObjects / SaveDamagedObjects
//
//	PURPOSE:	RETAIL: Tron's (0x10122120, 0x10122240): the damaged
//				objects are saved whole.  The names are ours
//
// ----------------------------------------------------------------------- //

void CProjectile::LoadDamagedObjects(ILTMessage_Read *pMsg)
{
	m_lstDamagedObjects.clear();

	int nCount = pMsg->Readuint8();
	for (int i = 0; i < nCount; ++i)
	{
		ProjectileDamagedObject damaged( LTNULL, 0.0f );
		pMsg->ReadData(&damaged, sizeof(ProjectileDamagedObject) * 8);
		m_lstDamagedObjects.push_back(damaged);
	}
}

void CProjectile::SaveDamagedObjects(ILTMessage_Write *pMsg)
{
	pMsg->Writeuint8(m_lstDamagedObjects.size());

	for (ProjectileDamagedObjectList::iterator iter = m_lstDamagedObjects.begin(); iter != m_lstDamagedObjects.end(); ++iter)
	{
		pMsg->WriteData(&(*iter), sizeof(ProjectileDamagedObject) * 8);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::UpdatePendingPos
//
//	PURPOSE:	RETAIL: Tron's (0x101222b0): move to the position kept for
//				this update.  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::UpdatePendingPos()
{
	if (m_bTronUnknown8FD)
	{
		g_pLTServer->SetObjectPos(m_hObject, &m_vTronUnknown900);
		m_bTronUnknown8FD = false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoVectorTrace
//
//	PURPOSE:	RETAIL: Tron's (0x101222e0): NOLF2's DoVector loop, moved
//				out of DoVector.  True when the vector hit something.  The
//				name is ours
//
// ----------------------------------------------------------------------- //

bool CProjectile::DoVectorTrace(LTVector vFrom, LTVector vTo)
{
	IntersectInfo iInfo;
	IntersectQuery qInfo;

	LTVector vOriginalFrom;

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY | CHECK_FROM_POINT_INSIDE_OBJECTS;

	LTBOOL bDone         = LTFALSE;

	int nLoopCount = 0; // No infinite loops thanks.


	const int cMaxNumFilterObjects = 11;
	HOBJECT hFilterList[cMaxNumFilterObjects];

	for (int i=0; i < cMaxNumFilterObjects; i++)
	{
		hFilterList[i] = LTNULL;
	}

	// Make sure we don't hit the object that fired the weapon...

	hFilterList[0] = m_hFiredFrom;
	int nNumFilteredObjects = 1;

	qInfo.m_PolyFilterFn = DoVectorPolyFilterFn;
	qInfo.m_FilterFn	 = DoVectorFilterFn;
	qInfo.m_pUserData	 = hFilterList;

	// RETAIL: the impacts may change the damage; Tron keeps it
	LTFLOAT fInstDamage = m_fInstDamage;

	while (!bDone)
	{
		qInfo.m_From = vFrom;
		qInfo.m_To   = vTo;

		if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
		{
			if (IsCharacterHitBox(iInfo.m_hObject))
			{
				vOriginalFrom = vFrom;

				if (HandlePotentialHitBoxImpact(iInfo, vFrom))
				{
					HandleVectorImpact(iInfo, vOriginalFrom, vTo);
					m_fInstDamage = fInstDamage;
					return true;
				}
			}
			else
			{
				if (HandleVectorImpact(iInfo, vFrom, vTo))
				{
					m_fInstDamage = fInstDamage;
					return true;
				}
			}

			// Filter out the object we just hit from the next call
			// to intersect segment...

			if (iInfo.m_hObject && !IsMainWorld(iInfo.m_hObject))
			{
				if (nNumFilteredObjects < cMaxNumFilterObjects)
				{
					// Filter out the object we just hit...

					hFilterList[nNumFilteredObjects] = iInfo.m_hObject;
					nNumFilteredObjects++;
				}
				else
				{
					// Hit too many objects...
					g_pLTServer->CPrint("ERROR in CProjectile::DoVector() - Tried to filter too many objects!!!");
					bDone = LTTRUE;
				}
			}
		}
		else // Didn't hit anything...
		{
            bDone = LTTRUE;
		}


		// Melee weapons can't shoot through objects...
		if (m_pAmmoData->eInstDamageType == DT_MELEE)
		{
			bDone = LTTRUE;
		}


		// Make sure we don't loop forever...

		if (++nLoopCount > MAX_VECTOR_LOOP)
		{
            g_pLTServer->CPrint("ERROR in CProjectile::DoVector() - Infinite loop encountered!!!");
            bDone = LTTRUE;
		}
	}

	m_fInstDamage = fInstDamage;
	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoVectorPolyFilterFn()
//
//	PURPOSE:	Handle filtering out unwanted polies
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

static bool DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// Don't filter for now...
	// RETAIL: folded with the other "return true" functions (0x1000a4d0)
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoVectorFilterFn()
//
//	PURPOSE:	Filter the attacker out of IntersectSegment
//				calls (so you don't shot yourself).  Also handle
//				AIs of the same alignment not shooting eachother
//
//  SPECIAL NOTES: pUserData must be a NULL terminated HOBJECT array and
//				   the first object in the array MUST be m_hFiredFrom!!!!!
//				   RETAIL: Tron's takes the array itself (0x101225d0)
//
// ----------------------------------------------------------------------- //

bool DoVectorFilterFn(HOBJECT hObj, void *pUserData)
{
	// Filter out the specified objects...
	if (ObjListFilterFn(hObj, pUserData))
	{
		// CharacterHitBox objects are used for vector impacts, don't
		// impact on the character/body prop object itself....

		if (IsCharacter(hObj) || IsBody(hObj) || IsKindOf(hObj, "Intelligence"))
		{
            return false;
		}

		// Check special character hit box cases...

		if (IsCharacterHitBox(hObj))
		{
            CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(hObj);
			if (pCharHitBox)
			{
				// NOTE: The first object in the filter list MUST be m_hFiredFrom!!!!!

				HOBJECT hUs = ((HOBJECT*)pUserData)[0];

				HOBJECT hTestObj = pCharHitBox->GetModelObject();
                if (!hTestObj) return false;

				if (hTestObj == hUs)
				{
                    return false;
				}


				return CanCharacterHitCharacter( hUs, hTestObj );
			}
		}

        return true;
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CanCharacterHitCharacter
//
//	PURPOSE:	Checks the allignment of the characters to see if a projectile
//				from one character is allowed to impact on the other character.
//				RETAIL: Tron's takes the shooter itself and has no friendly
//				fire test (0x10122660)
//
// ----------------------------------------------------------------------- //

static bool CanCharacterHitCharacter( HOBJECT hFiredFrom, HOBJECT hImpacted )
{
	// If we get hitboxes get the character objects...
	if( IsCharacterHitBox( hFiredFrom ))
	{
		CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( hFiredFrom ));
		if( !pCharHitBox )
			return true;

		hFiredFrom = pCharHitBox->GetModelObject();
	}

	if( IsCharacterHitBox( hImpacted ))
	{
		CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( hImpacted ));
		 if( !pCharHitBox )
			 return true;

		hImpacted = pCharHitBox->GetModelObject();
	}

	// Do special AI hitting AI case...
	if (IsAI(hFiredFrom) && IsAI(hImpacted))
	{
        CAI *pAI = (CAI*) g_pLTServer->HandleToObject(hFiredFrom);
        if (!pAI)
		{
			return false;
		}

        CCharacter* pB = (CCharacter*)g_pLTServer->HandleToObject(hImpacted);
        if (!pB)
		{
			return false;
		}

		// We can't hit guys we like, unless they're NEUTRAL to us

		if ( NEUTRAL != GetRelativeAlignment( pAI->GetRelationSet(),
			pB->GetRelationData()) )
		{
			// If they are NOT neutral to us, then find out if we like or
			// dislike them.
			CharacterAlignment eAlignment = GetAlignment(pAI->GetRelationSet(),
				pB->GetRelationData() );

			// Return true if we don't like them, false if we do.
			return LIKE != eAlignment;
		}
	}

	// Player-AI, AI-Player, and Player-Player with friendly fire on...

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoProjectile
//
//	PURPOSE:	Do projectile stuff...
//
//	USED:		only for projectiles
//
// ----------------------------------------------------------------------- //

void CProjectile::DoProjectile()
{
	if (!m_pAmmoData || !m_pAmmoData->pProjectileFX) return;

	// validate the pointers we will use
	ASSERT( 0 != g_pSurfaceMgr );
	ASSERT( 0 != g_pCommonLT );
	ASSERT( 0 != g_pPhysicsLT );

	// Set up the model...

	ObjectCreateStruct createStruct;
	createStruct.Clear();

	SAFE_STRCPY(createStruct.m_Filename, m_pAmmoData->pProjectileFX->szModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], m_pAmmoData->pProjectileFX->szSkin);

    g_pCommonLT->SetObjectFilenames(m_hObject, &createStruct);
    g_pLTServer->ScaleObject(m_hObject, &(m_pAmmoData->pProjectileFX->vModelScale));

	// Set the dims based on the current animation...

    LTVector vDims;
    g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));

	// Set object dims based on scale value...

    LTVector vNewDims = vDims * m_pAmmoData->pProjectileFX->vModelScale;
   	g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, 0);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);


	// Start your engines...

    m_fStartTime = g_pLTServer->GetTime();


	// Make the flash position the same as the fire position...

	m_vFlashPos	= m_vFirePos;


	// Set our force ignore limit and mass...

	g_pLTServer->SetBlockingPriority(m_hObject, 0);
	g_pPhysicsLT->SetForceIgnoreLimit(m_hObject, 0.0f);
	g_pPhysicsLT->SetMass(m_hObject, m_fMass);


	// Make sure we are pointing in the direction we are traveling...

	LTVector	vU, vR;

	if( (1.0f == m_vDir.y) || (-1.0f == m_vDir.y) )
	{
		vR = m_vDir.Cross( LTVector( 1.0f, 0.0f, 0.0f ));
	}
	else
	{
		vR = m_vDir.Cross( LTVector( 0.0f, 1.0f, 0.0f ));
	}

	vU = vR.Cross( m_vDir );
	vU.Normalize();
	
    LTRotation rRot(m_vDir, vU );
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);


	// Make sure we have the correct flags set...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_pAmmoData->pProjectileFX->dwObjectFlags, m_pAmmoData->pProjectileFX->dwObjectFlags);


	// And away we go...

    LTVector vVel;
	vVel = m_vDir * m_fVelocity;
	g_pPhysicsLT->SetVelocity(m_hObject, &vVel);


	// Special case of 0 life time...

	if (m_fLifeTime <= 0.0f)
	{
        Detonate(LTNULL);
	}
	else
	{
		// Adds an initial special FX for this projectile?
		AddSpecialFX();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoVector
//
//	PURPOSE:	Do vector stuff
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

void CProjectile::DoVector()
{
	// Check to see if we should use the client-side intersection
	if (HandleClientVectorImpact())
	{
		return;
	}

	LTVector vTo, vFrom;
	vFrom = m_vFirePos;
	vTo	= vFrom + (m_vDir * m_fRange);

	// RETAIL: Tron's loop is its own function (0x101222e0)
	if (DoVectorTrace(vFrom, vTo))
	{
		return;
	}

	// Didn't hit anything so just impact at the end pos...

	// RETAIL: the up vector shares the stack with the range vector
    AddImpact(LTNULL, m_vFlashPos, vTo, LTVector(0.0f, 1.0f, 0.0f), ST_SKY);


	// Okay, we're all done now...bye, bye...

	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleClientVectorImpact
//
//	PURPOSE:	Handle a client-side vector hitting something
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandleClientVectorImpact()
{
	if (!m_hClientObjImpact) return LTFALSE;

	// Filter the object they told us about, because we know stuff they don't
	HOBJECT hFilterList[] = { m_hFiredFrom, LTNULL };
	if (!DoVectorFilterFn(m_hClientObjImpact, hFilterList))
	{
		return LTFALSE;
	}

	LTVector vCurObjPos;
	g_pLTServer->GetObjectPos(m_hClientObjImpact, &vCurObjPos);
	LTVector vCurObjDims;
	g_pPhysicsLT->GetObjectDims(m_hClientObjImpact, &vCurObjDims);

	// Check the impact position against an expanded version of the dims so we know we're not entirely off base...
	LTVector vCurObjMin = vCurObjPos - (vCurObjDims * 2.0f);
	LTVector vCurObjMax = vCurObjPos + (vCurObjDims * 2.0f);
	if ((vCurObjMin.x > m_vClientObjImpactPos.x) ||
		(vCurObjMin.y > m_vClientObjImpactPos.y) ||
		(vCurObjMin.z > m_vClientObjImpactPos.z) ||
		(vCurObjMax.x < m_vClientObjImpactPos.x) ||
		(vCurObjMax.y < m_vClientObjImpactPos.y) ||
		(vCurObjMax.z < m_vClientObjImpactPos.z))
	{
		return LTFALSE;
	}

	// Ok, act like we hit them
	IntersectInfo iInfo;
	iInfo.m_hObject = m_hClientObjImpact;
	iInfo.m_hPoly = INVALID_HPOLY;
	iInfo.m_Point = m_vClientObjImpactPos;

	HandleVectorImpact(iInfo, m_vFirePos, m_vClientObjImpactPos);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandleVectorImpact(IntersectInfo & iInfo, LTVector & vFrom,
                                     LTVector & vTo)
{
	// Get the surface type...

	SurfaceType eSurfType = GetSurfaceType(iInfo);


	// See if we hit an invisible surface...

	if (eSurfType == ST_INVISIBLE)
	{
		if (!CalcInvisibleImpact(iInfo, eSurfType))
		{
			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
            if (!pSurf) return LTTRUE;

			return UpdateDoVectorValues(*pSurf, 0, iInfo.m_Point, vFrom, vTo);
		}
	}

	// If the fire position is the initial fire position, use the flash
	// position when building the impact special fx...

    LTVector vFirePos = (vFrom.NearlyEquals(m_vFirePos, 0.0f) ? m_vFlashPos : vFrom);

	// RETAIL: Tron has no deflection (see Detonate)

	// See if we hit an object that should be damaged...

    LTBOOL bHitWorld = IsMainWorld(iInfo.m_hObject);

	// RETAIL: 0x10123063, Tron adds the impact before the damage.

	AddImpact(iInfo.m_hObject, vFirePos, iInfo.m_Point, iInfo.m_Plane.m_Normal, eSurfType);

	if (!bHitWorld && eSurfType != ST_LIQUID)
	{
		ImpactDamageObject(m_hFiredFrom, iInfo.m_hObject);
	}


	// See if we can shoot through the surface...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
    if (!pSurf) return LTTRUE;  // Done.

	if (pSurf->bCanShootThrough)
	{
		int nMaxThickness = pSurf->nMaxShootThroughThickness;
		if (nMaxThickness == 0)
		{
			// Special case of always being able to shoot through surface...

			// Calculate new values for next DoVector iteration...

			return UpdateDoVectorValues(*pSurf, 0, iInfo.m_Point, vFrom, vTo);
		}

		// Test if object/wall intersected is thin enough to be shot
		// through...

		// Test object case first...
		// Check if this is a worldmodel.
		bool bHitWorldModel = ( OT_WORLDMODEL == GetObjectType( iInfo.m_hObject ));
		if (!bHitWorldModel && !bHitWorld && iInfo.m_hObject)
		{
			// Test to see if we can shoot through the object...

            LTVector vDims;
			g_pPhysicsLT->GetObjectDims(iInfo.m_hObject, &vDims);

			if (vDims.x*2.0f >= nMaxThickness &&  vDims.y*2.0f >= nMaxThickness &&
				vDims.z*2.0f >= nMaxThickness)
			{
				// Can't shoot through this object...
                return LTTRUE;
			}
		}

		// Determine if we shot through the wall/object...

		HOBJECT hFilterList[] = { m_hFiredFrom, LTNULL };

		IntersectInfo iTestInfo;
		IntersectQuery qTestInfo;

        qTestInfo.m_From = iInfo.m_Point + (m_vDir * (LTFLOAT)(nMaxThickness + 1));
		qTestInfo.m_To   = iInfo.m_Point - m_vDir;

		qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY | CHECK_FROM_POINT_INSIDE_OBJECTS;	// RETAIL: 0x10123352

		qTestInfo.m_FilterFn  = DoVectorFilterFn;
		qTestInfo.m_pUserData = hFilterList;

        if (g_pLTServer->IntersectSegment(&qTestInfo, &iTestInfo))
		{
			// Calculate new values for next DoVector iteration...

            LTVector vThickness = iTestInfo.m_Point - iInfo.m_Point;
			return UpdateDoVectorValues(*pSurf, vThickness.Mag(), iTestInfo.m_Point, vFrom, vTo);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::UpdateVectorValues
//
//	PURPOSE:	Update our DoVector values
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::UpdateDoVectorValues(SURFACE & surf, LTFLOAT fThickness,
                                        LTVector vImpactPos, LTVector & vFrom, LTVector & vTo)
{
	// See if we've traveled the distance...

    LTVector vDistTraveled = vImpactPos - m_vFirePos;
    LTFLOAT fDist = m_fRange - vDistTraveled.Mag();
    if (fDist < 1.0f) return LTTRUE;

	// Just dampen based on the bute file values, don't worry about the
	// surface thinkness...

	//DANO: POTENTIAL PROBLEM!!
	// This variable m_fInstDamage gets changed here but
	// used by another function...in effect this variable
	// is getting passed "under the hood".  This can present
	// big problems with the Sequencer!
	AdjustDamage(surf.fBulletDamageDampen);

	fDist *= surf.fBulletRangeDampen;

	int nPerturb = surf.nMaxShootThroughPerturb;

	if (nPerturb)
	{
        LTRotation rRot(m_vDir, LTVector(0.0f, 1.0f, 0.0f));

        LTVector vU, vR, vF;
		vU = rRot.Up();
		vR = rRot.Right();
		vF = rRot.Forward();

        LTFLOAT fRPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;
        LTFLOAT fUPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;

		m_vDir += (vR * fRPerturb);
		m_vDir += (vU * fUPerturb);

		m_vDir.Normalize();
	}

	// Make sure we move the from position...

    if (vFrom.NearlyEquals(vImpactPos, 1.0f))
	{
		vFrom += m_vDir;
	}
	else
	{
		vFrom = vImpactPos;
	}

	vTo = vFrom + (m_vDir * fDist);

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CalcInvisibleImpact
//
//	PURPOSE:	Update the impact value so it ignores invisible surfaces
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::CalcInvisibleImpact(IntersectInfo & iInfo, SurfaceType & eSurfType)
{
	// Since we hit an invisible surface try and find a solid surface that
	// is the real surface of impact.  NOTE:  We assume that the solid
	// surface will have a normal facing basically the opposite direction...

	HOBJECT hFilterList[] = { m_hFiredFrom, LTNULL };

	IntersectInfo iTestInfo;
	IntersectQuery qTestInfo;

	qTestInfo.m_From = iInfo.m_Point + (m_vDir * g_vtInvisibleMaxThickness.GetFloat());
	qTestInfo.m_To   = iInfo.m_Point - m_vDir;

	qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY | CHECK_FROM_POINT_INSIDE_OBJECTS;

	qTestInfo.m_FilterFn  = DoVectorFilterFn;
	qTestInfo.m_pUserData = hFilterList;

    if (g_pLTServer->IntersectSegment(&qTestInfo, &iTestInfo))
	{
		eSurfType = GetSurfaceType(iTestInfo);

		// If we hit another invisible surface, we're done...

		if (eSurfType != ST_INVISIBLE)
		{
			iInfo = iTestInfo;
            return LTTRUE;
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandlePotentialHitBoxImpact
//
//	PURPOSE:	Handle a vector hitting a Character's hit box
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandlePotentialHitBoxImpact(IntersectInfo & iInfo,
                                               LTVector & vFrom)
{
    CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(iInfo.m_hObject);
    if (!pCharHitBox) return LTFALSE;

	return pCharHitBox->HandleImpact(this, iInfo, m_vDir, vFrom);
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CProjectile::RequestStartSendingMessages()
//
//	RETAIL: 0x10123a80; PORT: Build 69's CDisc version
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CProjectile::RequestStartSendingMessages()
{
	if ( false == m_bReceivingMessages )
	{
		//
		// tell owner we want messages
		//

		ASSERT( 0 != m_hObject );
		ASSERT( m_hFiredFrom );

		CAutoMessage cMsg;
		LTRESULT ltResult;

		cMsg.Writeuint32( MID_PROJECTILE );

		// write the mesasge subtype
		cMsg.Writeuint8( MPROJ_START_SENDING_MESSAGES );

		// write the number of objects that want messages
		cMsg.Writeuint8( 1 );

		// write the hobject that wants updates
		cMsg.WriteObject( m_hObject );

		// send to the owner
		ltResult = g_pLTServer->SendToObject(
				cMsg.Read(),
				m_hObject,
				m_hFiredFrom,
				MESSAGE_GUARANTEED
			);
		ASSERT( LT_OK == ltResult );

		// keep track that we want messages
		m_bReceivingMessages = true;
	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CProjectile::RequestStopSendingMessages()
//
//	RETAIL: 0x10123b90; PORT: Build 69's CDisc version
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CProjectile::RequestStopSendingMessages()
{
	// RETAIL: Tron also needs the owner (0x10123bb9)
	if ( true == m_bReceivingMessages && m_hFiredFrom )
	{
		//
		// tell owner we don't want messages
		//

		ASSERT( 0 != m_hObject );
		ASSERT( m_hFiredFrom );

		CAutoMessage cMsg;
		LTRESULT ltResult;

		cMsg.Writeuint32( MID_PROJECTILE );

		// write the mesasge subtype
		cMsg.Writeuint8( MPROJ_STOP_SENDING_MESSAGES );

		// write the number of objects that don't need messages anymore
		cMsg.Writeuint8( 1 );

		// write the hobject to remove from getting updates
		cMsg.WriteObject( m_hObject );

		// send to the owner
		ltResult = g_pLTServer->SendToObject(
				cMsg.Read(),
				m_hObject,
				m_hFiredFrom,
				MESSAGE_GUARANTEED
			);
		ASSERT( LT_OK == ltResult );

		// keep track that we don't want messages
		m_bReceivingMessages = false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ImpactDamageObject()
//
//	PURPOSE:	Handle impact damage to an object
//
// ----------------------------------------------------------------------- //

void CProjectile::ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj)
{
	// RETAIL: Tron damages an object once (0x10123cb9)
	if (IsDamagedObject(hObj)) return;

	// RETAIL: Tron's damages are virtual (slots 40..42; called in this
	// order, 0x10123cd2)
	TronUnknownVirtual41(hDamager, hObj);
	TronUnknownVirtual42(hDamager, hObj);
	TronUnknownVirtual40(hDamager, hObj);


	// Update player summary info...

	CPlayerObj* pPlayer;
	if (IsPlayer(hDamager) && IsCharacter(hObj) && IsAccuracyType(m_eInstDamageType))
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(hObj);
        pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
		if (pPlayer && pChar)
		{
			pPlayer->GetMissionStats()->dwNumHits++;


			ModelNode eModelNode = pChar->GetModelNodeLastHit();

			if (eModelNode != eModelNodeInvalid)
			{
				HitLocation eLoc = g_pModelButeMgr->GetSkeletonNodeLocation(pChar->GetModelSkeleton(),eModelNode);
				pPlayer->GetMissionStats()->dwHitLocations[eLoc]++;
			}
			else
			{
				pPlayer->GetMissionStats()->dwHitLocations[HL_UNKNOWN]++;
			}
		}
	}

	if (IsPlayer(hObj))
	{
        pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hObj);
		if (pPlayer)
		{
			pPlayer->GetMissionStats()->dwNumTimesHit++;
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TronUnknownVirtual40
//
//	PURPOSE:	RETAIL: slot 40 (0x10123dc0): the instant damage
//				(NOLF2's ImpactDamageObject did it).  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::TronUnknownVirtual40(HOBJECT hDamager, HOBJECT hObj)
{
	if (!hObj) return;

	DamageStruct damage;

	// Do Instant damage...

	if ( m_eInstDamageType != DT_UNSPECIFIED )
	{
		damage.Clear();
		damage.hDamager = hDamager;
		damage.vDir		= m_vDir;
		damage.nAmmoId	= m_nAmmoId;
		damage.eType	= m_eInstDamageType;
		damage.fDamage	= m_fInstDamage;

		ScaleTronDamage( &damage.fDamage );
		ScaleTronDamage101( &damage.fDamage );
		ScaleTronDamage105( &damage.fDamage );

		damage.DoDamage(this, hObj);

		AddDamagedObject(hObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TronUnknownVirtual41
//
//	PURPOSE:	RETAIL: slot 41 (0x10123f80): the progressive damage (if
//				the progressive damage is supposed to happen over time, it
//				will be done in the explosion object).  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::TronUnknownVirtual41(HOBJECT hDamager, HOBJECT hObj)
{
	if (!hObj) return;

	DamageStruct damage;

	if ( m_eProgDamageType != DT_UNSPECIFIED && m_pAmmoData->fProgDamageLifetime <= 0.0f)
	{
		damage.Clear();
		damage.hDamager = hDamager;
		damage.vDir		= m_vDir;
		damage.nAmmoId	= m_nAmmoId;
		damage.eType	 = m_eProgDamageType;
		damage.fDamage	 = m_fProgDamage;
		damage.fDuration = m_pAmmoData->fProgDamageDuration;

		ScaleTronDamage( &damage.fDamage );
		ScaleTronDamage101( &damage.fDamage );
		ScaleTronDamage105( &damage.fDamage );

		damage.DoDamage(this, hObj);

		AddDamagedObject(hObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TronUnknownVirtual42
//
//	PURPOSE:	RETAIL: slot 42 (0x10124170): modifier 9's damage (type
//				0x22, not scaled).  The name is ours
//
// ----------------------------------------------------------------------- //

void CProjectile::TronUnknownVirtual42(HOBJECT hDamager, HOBJECT hObj)
{
	if (!hObj) return;

	DamageStruct damage;

	if ( m_bTronUnknown7C )
	{
		damage.Clear();
		damage.hDamager = hDamager;
		damage.vDir		= m_vDir;
		damage.nAmmoId	= m_nAmmoId;
		damage.eType	 = (DamageType)0x22;
		damage.fDamage	 = m_fTronUnknown80;
		damage.fDuration = m_fTronUnknown84;

		damage.DoDamage(this, hObj);

		AddDamagedObject(hObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Save
//
//	PURPOSE:	Save the object (RETAIL: 0x10124320)
//
// ----------------------------------------------------------------------- //

void CProjectile::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_VECTOR(m_vFlashPos);
    SAVE_VECTOR(m_vFirePos);
    SAVE_VECTOR(m_vDir);
	SAVE_bool(m_bSilenced);
    SAVE_bool(m_bObjectRemoved);
    SAVE_bool(m_bDetonated);
    SAVE_BYTE(m_nWeaponId);
    SAVE_BYTE(m_nAmmoId);
	SAVE_bool(m_bSetup);
	SAVE_bool(m_bCanHitSameProjectileKind);
	SAVE_bool(m_bCanTouchFiredFromObj);
    SAVE_BYTE(m_eInstDamageType);
    SAVE_BYTE(m_eProgDamageType);
    SAVE_FLOAT(m_fInstDamage);
    SAVE_FLOAT(m_fProgDamage);
	SAVE_bool(m_bTronUnknown7C);
	SAVE_FLOAT(m_fTronUnknown80);
	SAVE_FLOAT(m_fTronUnknown84);
    SAVE_TIME(m_fStartTime);
    SAVE_FLOAT(m_fLifeTime);
    SAVE_FLOAT(m_fVelocity);
    SAVE_FLOAT(m_fMass);
    SAVE_FLOAT(m_fRange);
	SAVE_HOBJECT(m_hFiredFrom);
	SAVE_DWORD(m_bNumCallsToAddImpact);
	SAVE_FLOAT(m_xTronUnknownB0);
    SAVE_bool(m_bProcessInvImpact);
    SAVE_VECTOR(m_vInvisVel);
    SAVE_VECTOR(m_vInvisNewPos);
    SAVE_VECTOR(m_vTronUnknownD0);
	SAVE_FLOAT(m_fMaxRicochetAngle);
	SAVE_INT(m_nMaxRicochets);
	SAVE_INT(m_nTotalRicochets);
	SAVE_INT(m_nUpdateNum);
	SAVE_INT(m_nLastRicochetUpdateNum);
	SAVE_bool(m_bReceivingMessages);

	SaveModifiers(pMsg);
	SaveDamagedObjects(pMsg);
	SavePendingTouches(pMsg);

	SAVE_bool(m_bTronUnknown1C0);
	SAVE_bool(m_bTronUnknown1C1);
	SAVE_bool(m_bTronUnknown1C2);
	pMsg->WriteType(m_vLastPos);
	pMsg->WriteType(m_vLastVel);
	SAVE_bool(m_bHasRelationData);
	SAVE_bool(m_bTronUnknown8FD);
	pMsg->WriteType(m_vTronUnknown900);

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	SAVE_DWORD(dwFlags);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Load
//
//	PURPOSE:	Load the object (RETAIL: 0x101246c0)
//
// ----------------------------------------------------------------------- //

void CProjectile::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_VECTOR(m_vFlashPos);
    LOAD_VECTOR(m_vFirePos);
    LOAD_VECTOR(m_vDir);
    LOAD_bool(m_bSilenced);
    LOAD_bool(m_bObjectRemoved);
    LOAD_bool(m_bDetonated);
    LOAD_BYTE(m_nWeaponId);
    LOAD_BYTE(m_nAmmoId);

	m_pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	m_pAmmoData	  = g_pWeaponMgr->GetAmmo(m_nAmmoId);

	LOAD_bool(m_bSetup);
	LOAD_bool(m_bCanHitSameProjectileKind);
	LOAD_bool(m_bCanTouchFiredFromObj);
    LOAD_BYTE_CAST(m_eInstDamageType, DamageType);
    LOAD_BYTE_CAST(m_eProgDamageType, DamageType);
    LOAD_FLOAT(m_fInstDamage);
    LOAD_FLOAT(m_fProgDamage);
	LOAD_bool(m_bTronUnknown7C);
	LOAD_FLOAT(m_fTronUnknown80);
	LOAD_FLOAT(m_fTronUnknown84);
    LOAD_TIME(m_fStartTime);
    LOAD_FLOAT(m_fLifeTime);
    LOAD_FLOAT(m_fVelocity);
    LOAD_FLOAT(m_fMass);
    LOAD_FLOAT(m_fRange);
	LOAD_HOBJECT(m_hFiredFrom);
	LOAD_DWORD(m_bNumCallsToAddImpact);
	LOAD_FLOAT(m_xTronUnknownB0);
    LOAD_bool(m_bProcessInvImpact);
    LOAD_VECTOR(m_vInvisVel);
	LOAD_VECTOR(m_vInvisNewPos);
	LOAD_VECTOR(m_vTronUnknownD0);
	LOAD_FLOAT(m_fMaxRicochetAngle);
	LOAD_INT(m_nMaxRicochets);
	LOAD_INT(m_nTotalRicochets);
	LOAD_INT(m_nUpdateNum);
	LOAD_INT(m_nLastRicochetUpdateNum);
	LOAD_bool(m_bReceivingMessages);

	LoadModifiers(pMsg);
	LoadDamagedObjects(pMsg);
	LoadPendingTouches(pMsg);

	LOAD_bool(m_bTronUnknown1C0);
	LOAD_bool(m_bTronUnknown1C1);
	LOAD_bool(m_bTronUnknown1C2);
	LOAD_VECTOR(m_vLastPos);
	LOAD_VECTOR(m_vLastVel);
	LOAD_bool(m_bHasRelationData);
	LOAD_bool(m_bTronUnknown8FD);
	LOAD_VECTOR(m_vTronUnknown900);

	// RETAIL: Tron reads the saved object flags but does not set them
	// (0x10124b0a gets the flags, 0x10124b26 reads the saved ones and
	// drops them; see docs/RETAIL_BUGS.md)
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	LOAD_DWORD(dwFlags);
}

