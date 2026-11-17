// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.cpp
//
// PURPOSE : Projectile classs - implementation
//
// CREATED : 10/3/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileTypes.h"
#include "ServerUtilities.h"
#include "DamageTypes.h"
#include "SurfaceFunctions.h"
#include "SoundMgr.h"
#include "iltphysics.h"
#include "CVarTrack.h"
#include "Spawner.h"
#include "DebrisFuncs.h"
#include "Character.h"
#include "PlayerObj.h"
#include "CharacterHitBox.h"
#include "Body.h"
#include "ObjectMsgs.h"
#include "ServerSoundMgr.h"
#include "GameServerShell.h"
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "AIUtils.h"
#include "FXButeMgr.h"
#include "CharacterMgr.h"
#include "AI.h"
#include "WeaponItems.h"



LINKFROM_MODULE( ProjectileTypes );


#define DEFAULT_GRENADE_DAMPEN_PERCENT	0.35f
#define DEFAULT_GRENADE_MIN_VELOCITY	10.0f

static CVarTrack g_vtGrenadeDampenPercent;
static CVarTrack g_vtGrenadeMinVelMag;

// RETAIL: Tron's ProjectileTypes has CGrenade only (no RTTI, ClassDef or
// strings of NOLF2's gadget projectiles CLipstickProx, CGrenadeImpact, CSpear,
// CCameraDisabler, CCoin, CKitty, CBearTrap, CBanana, CProjectileSpawner).

CTList<CGrenade*> g_lstGrenades;

#pragma force_active on
BEGIN_CLASS(CGrenade)
END_CLASS_DEFAULT_FLAGS(CGrenade, CProjectile, NULL, NULL, CF_HIDDEN)

#pragma force_active off

static bool MoveToFloorFilterFn(HOBJECT hTest, void *pUserData)
{
	// First filter out objects in the list...

	if (ObjListFilterFn(hTest, pUserData))
	{
		// Okay, now make sure we're not placed on top of any non-solid
		// ray-hit true objects...

		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags( hTest, OFT_Flags, dwFlags );
		if (dwFlags & FLAG_SOLID)
		{
			return true;
		}
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::CGrenade
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGrenade::CGrenade() : CProjectile()
{
	// Uncomment these lines to use normal, non-point collide physics.
	// (point collide physics is faster and should be used whenever
	// possible).
	//m_dwFlags &= ~FLAG_POINTCOLLIDE;
	//m_dwFlags |= FLAG_SOLID | FLAG_CLIENTNONSOLID;

	m_vDims.Init(5.0f, 5.0f, 5.0f);

    m_bSpinGrenade  = LTTRUE;
    m_bUpdating     = LTTRUE;

	m_fPitch	= 0.0f;
	m_fYaw		= 0.0f;
	m_fRoll		= 0.0f;
	m_fPitchVel	= 0.0f;
	m_fYawVel	= 0.0f;
	m_fRollVel	= 0.0f;

	m_cBounces = 0;

    m_hBounceSnd = LTNULL;
	m_eContainerCode = CC_NO_CONTAINER;
	m_eLastHitSurface = ST_UNKNOWN;

	m_bRotatedToRest = LTFALSE;
	m_bAddToGrenadeList = LTTRUE;

	ResetRotationVel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::~CGrenade
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGrenade::~CGrenade()
{
	// Just to be safe...
	if( m_bAddToGrenadeList )
		g_lstGrenades.Remove(this);

	if (m_hBounceSnd)
	{
        g_pLTServer->SoundMgr()->KillSound(m_hBounceSnd);
        m_hBounceSnd = LTNULL;
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Detonate
//
//	PURPOSE:	Go boom
//
// ----------------------------------------------------------------------- //

void CGrenade::Detonate(HOBJECT hObj, CollisionInfo* pCollInfo)
{
	// We're blowing up, take us out of the grenade list
	if( m_bAddToGrenadeList )
		g_lstGrenades.Remove(this);

	CProjectile::Detonate(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::HandleImpact
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CGrenade::HandleImpact(HOBJECT hObj)
{
	if (!g_vtGrenadeDampenPercent.IsInitted())
	{
        g_vtGrenadeDampenPercent.Init(g_pLTServer, "GrenadeDampenPercent", LTNULL, DEFAULT_GRENADE_DAMPEN_PERCENT);
	}

	if (!g_vtGrenadeMinVelMag.IsInitted())
	{
        g_vtGrenadeMinVelMag.Init(g_pLTServer, "GrenadeMinVelMag", LTNULL, DEFAULT_GRENADE_MIN_VELOCITY);
	}


	// RETAIL: 0x1011c760 has no NOLF2 1.3 "detonate on characters" test
	// (and so no team test).


    LTVector vVel;
	g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

	// See if we are impacting on liquid...

    LTBOOL bEnteringLiquid = LTFALSE;
    uint16 code;
    if (g_pLTServer->GetContainerCode(hObj, &code))
	{
		if (IsLiquid((ContainerCode)code))
		{
            bEnteringLiquid = LTTRUE;
		}
	}


	CollisionInfo info;
    g_pLTServer->GetLastCollision(&info);

	// Calculate where we really hit the world...
    if (IsMainWorld(hObj))
	{
        LTVector vPos, vCurVel, vP0, vP1;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

		vP1 = vPos;
        vCurVel = vVel * g_pLTServer->GetFrameTime();
		vP0 = vP1 - vCurVel;
		vP1 += vCurVel;

        LTFLOAT fDot1 = VEC_DOT(info.m_Plane.m_Normal, vP0) - info.m_Plane.m_Dist;
        LTFLOAT fDot2 = VEC_DOT(info.m_Plane.m_Normal, vP1) - info.m_Plane.m_Dist;

		if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
		{
			vPos = vP1;
		}
		else
		{
            LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
			VEC_LERP(vPos, vP0, vP1, fPercent);
		}

		// Set our new "real" pos...

        g_pLTServer->SetObjectPos(m_hObject, &vPos);
	}

	// Do the bounce, if the object we hit isn't liquid...

	if (!bEnteringLiquid)
	{
		vVel += (info.m_vStopVel * 2.0f);
	}


	// Dampen the grenade's new velocity based on the surface type...

    LTFLOAT fDampenPercent = g_vtGrenadeDampenPercent.GetFloat();

	m_eLastHitSurface = GetSurfaceType(info);
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);
	if (pSurf)
	{
		// Play a bounce sound (based on the surface type) if one isn't
		// already playing...

		if ( ShouldPlayBounceSound(pSurf) )
		{
            uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;

			int nVolume	= IsLiquid(m_eContainerCode) ? 50 : 100;

            LTVector vPos;
            g_pLTServer->GetObjectPos(m_hObject, &vPos);

			// Only play one sound at a time...
			
			if (m_hBounceSnd)
			{
                g_pLTServer->SoundMgr()->KillSound(m_hBounceSnd);
                m_hBounceSnd = LTNULL;
			}
			else {
				// Register a stimulus for the bounce sound, if this is a new bounce sound.

				g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyDisturbanceSound, 1, m_hFiredFrom, m_hObject, vPos, pSurf->fGrenadeSndRadius );
			}

			m_hBounceSnd = g_pServerSoundMgr->PlaySoundFromPos(vPos, (char*)GetBounceSound(pSurf),
				pSurf->fGrenadeSndRadius, SOUNDPRIORITY_MISC_MEDIUM, dwFlags, nVolume);
		}

		fDampenPercent = (1.0f - pSurf->fHardness);
	}

	fDampenPercent = fDampenPercent > 1.0f ? 1.0f : (fDampenPercent < 0.0f ? 0.0f : fDampenPercent);

	vVel *= (1.0f - fDampenPercent);


	// See if we should come to a rest...

    LTVector vTest = vVel;
	vTest.y = 0.0f;

	if (vTest.Mag() < g_vtGrenadeMinVelMag.GetFloat())
	{
		// If we're on the ground (or an object), stop movement...

		CollisionInfo standingInfo;
        g_pLTServer->GetStandingOn(m_hObject, &standingInfo);

		CollisionInfo* pInfo = standingInfo.m_hObject ? &standingInfo : &info;

		if (pInfo->m_hObject)
		{
			// Don't stop on walls...

			if (pInfo->m_Plane.m_Normal.y > 0.75f)
			{
				vVel.Init();

				// Turn off gravity, solid, and touch notify....

				g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID );

				// Rotate to rest...

				RotateToRest();
			}
		}
	}


	// Reset rotation velocities due to the bounce...

	ResetRotationVel(&vVel, pSurf);


	// We need to subtact this out because the engine will add it back in,
	// kind of a kludge but necessary...

	vVel -= info.m_vStopVel;


    g_pPhysicsLT->SetVelocity(m_hObject, &vVel);

	m_cBounces++;
}

// ----------------------------------------------------------------------- //

LTBOOL CGrenade::ShouldPlayBounceSound(SURFACE* pSurface)
{
	return !!pSurface->szGrenadeImpactSnd[0];
}

// ----------------------------------------------------------------------- //

const char* CGrenade::GetBounceSound(SURFACE* pSurface)
{
	return pSurface->szGrenadeImpactSnd;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CGrenade::UpdateGrenade()
{
	// Update my container code...

    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Get our container code, but ignore everything except for liquid containers...

	uint32 nIgnoreFlags = (CC_ALL_FLAG & ~ ::GetLiquidFlags());
    m_eContainerCode = ::GetContainerCode(vPos, nIgnoreFlags);


	// Update grenade spin...

	if (m_bSpinGrenade && m_bUpdating)
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0 || m_fRollVel != 0)
		{
            LTFLOAT fDeltaTime = g_pLTServer->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw   += m_fYawVel * fDeltaTime;
			m_fRoll  += m_fRollVel * fDeltaTime;

            LTRotation rRot(m_fPitch, m_fYaw, m_fRoll);
			g_pLTServer->SetObjectRotation(m_hObject, &rRot);
		}
	}


	// See if the bounce sound is done playing...

	if (m_hBounceSnd)
	{
        bool bIsDone;
        if (g_pLTServer->SoundMgr()->IsSoundDone(m_hBounceSnd, bIsDone) != LT_OK || bIsDone)
		{
            g_pLTServer->SoundMgr()->KillSound(m_hBounceSnd);
            m_hBounceSnd = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CGrenade::RotateToRest()
{
	// Record this grenade for AI's to be wary of

	if ( !m_bRotatedToRest && m_bAddToGrenadeList )
	{
		g_lstGrenades.Add(this);
	}

	// At rest

	m_bRotatedToRest = LTTRUE;

	// Turn off any animations that are playing (e.g., spin)...

	ANIMTRACKERID nTracker;
	if( LT_OK == g_pModelLT->GetMainTracker( m_hObject, nTracker ))
	{
		g_pModelLT->SetPlaying(m_hObject, nTracker, LTFALSE);
	}

	// Spin it if necessary

	if (!m_bSpinGrenade) return;

    LTRotation rRot(0.0f, m_fYaw, 0.0f);
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);

    m_bUpdating = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::ResetRotationVel()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CGrenade::ResetRotationVel(LTVector* pvNewVel, SURFACE* pSurf)
{
	if (!m_bSpinGrenade) return;

	// (TO DO: base this on new velocity and surface hardness?)

    LTFLOAT fVal  = MATH_CIRCLE/2.0f;
    LTFLOAT fVal2 = MATH_CIRCLE;
	m_fPitchVel	 = GetRandom(-fVal, fVal);
	m_fYawVel	 = GetRandom(-fVal2, fVal2);
	m_fRollVel   = GetRandom(-fVal2, fVal2);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CGrenade::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			UpdateGrenade();
		}
		break;

		default : break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CGrenade::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	CProjectile::Save(pMsg, dwSaveFlags);

	SAVE_BOOL(m_bSpinGrenade);
    SAVE_BOOL(m_bUpdating);
    SAVE_BYTE(m_eContainerCode);
    SAVE_BYTE(m_eLastHitSurface);

    SAVE_FLOAT(m_fPitch);
    SAVE_FLOAT(m_fYaw);
    SAVE_FLOAT(m_fRoll);
    SAVE_FLOAT(m_fPitchVel);
    SAVE_FLOAT(m_fYawVel);
    SAVE_FLOAT(m_fRollVel);
	SAVE_BOOL(m_bRotatedToRest);
	SAVE_INT(m_cBounces);
	SAVE_BOOL(m_bAddToGrenadeList);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CGrenade::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	CProjectile::Load(pMsg, dwLoadFlags);

    LOAD_BOOL(m_bSpinGrenade);
    LOAD_BOOL(m_bUpdating);
    LOAD_BYTE_CAST(m_eContainerCode, ContainerCode);
    LOAD_BYTE_CAST(m_eLastHitSurface, SurfaceType);

    LOAD_FLOAT(m_fPitch);
    LOAD_FLOAT(m_fYaw);
    LOAD_FLOAT(m_fRoll);
    LOAD_FLOAT(m_fPitchVel);
    LOAD_FLOAT(m_fYawVel);
    LOAD_FLOAT(m_fRollVel);
	LOAD_BOOL(m_bRotatedToRest);
	LOAD_INT(m_cBounces);
	LOAD_BOOL(m_bAddToGrenadeList);

	if ( m_bRotatedToRest && m_bAddToGrenadeList )
	{
		g_lstGrenades.Add(this);
	}
}
