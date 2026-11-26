// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectilePrankster.cpp
//
// PURPOSE : The prankster bit - implementation
//
// RETAIL: Tron 2.0's, written from retail Object.lto
// (ProjectilePrankster.h).  The names are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectilePrankster.h"
#include "WeaponFireInfo.h"
#include "FXButeMgr.h"
#include "MsgIDs.h"
#include "SFXMsgIds.h"
#include "PlayerObj.h"
// RETAIL: DebugLineSystem.h's colours follow PlayerShared.h's initializers
// at the start of the TU (0x1003eed0..0x1003f080).
#include "DebugLineSystem.h"
#include "Character.h"
#include "AI.h"
#include "AIGoalMgr.h"
#include "Destructible.h"
#include "SurfaceFunctions.h"
#include "Camera.h"
#include "CVarTrack.h"

BEGIN_CLASS(CProjectilePrankster)
END_CLASS_DEFAULT_FLAGS(CProjectilePrankster, CTronProjectile, NULL, NULL, CF_HIDDEN)

extern bool ExplosionFilterFn(HOBJECT hObj, void *pUserData);

// RETAIL: Projectile.cpp's (0x1037c290)
extern CVarTrack g_vtProjectileNewCollision;

// RETAIL: Tron's own message and effect ids (0x10040889, 0x100410b3,
// 0x1003feba).  MsgIDs.h/SFXMsgIds.h do not have Tron's values yet; the
// names are ours.
#define PRANKSTER_MID_PROJECTILE		0x8a	// to the owner's client
#define PRANKSTER_MPROJ_DONE			0xc9	// the prankster bit is gone
#define PRANKSTER_MPROJ_DETONATE		0xca	// from the owner: stop now
#define PRANKSTER_MID_SFX_MESSAGE		0xe7
#define PRANKSTER_SFX_PROJECTILE_ID		0x3b
#define PRANKSTER_PFX_ACQUIRE			1

// RETAIL: the AI goal 0x34 an AI in stasis must finish first (0x100407a5)
#define PRANKSTER_AIGOAL_STASIS			((EnumAIGoalType)0x34)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::CProjectilePrankster
//
//	PURPOSE:	Constructor (RETAIL: 0x1003f380)
//
// ----------------------------------------------------------------------- //

CProjectilePrankster::CProjectilePrankster()
	: CTronProjectile(),
	  m_eState( kPrankster_None ),
	  m_AcquireTimer(),
	  m_Victims(),
	  m_fImplodeVelocity( 0.0f ),
	  m_fDeathRadius( 0.0f ),
	  m_fMaxDistance( 0.0f ),
	  m_fTimeVictimStasis( 0.0f ),
	  m_fTimeVictimStasisRange( 0.0f ),
	  m_fTimeToAcquireTargets( 0.0f ),
	  m_sEliminateTargetsFX(),
	  m_bSentDone( false )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::~CProjectilePrankster
//
//	PURPOSE:	Destructor (RETAIL: 0x1003f170)
//
// ----------------------------------------------------------------------- //

CProjectilePrankster::~CProjectilePrankster()
{
	SendDone();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::OnLinkBroken
//
//	PURPOSE:	A victim is gone (RETAIL: 0x1003f490)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::OnLinkBroken(LTObjRefNotifier *pRef, HOBJECT hObj)
{
	m_Victims.erase(LTObjRefNotifier(hObj));

	CTronProjectile::OnLinkBroken(pRef, hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::Setup
//
//	PURPOSE:	Set up from the class data (RETAIL: 0x1003f5c0)
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectilePrankster::Setup(CWeapon const* pWeapon, WeaponFireInfo const& info)
{
	LTBOOL bRet = CTronProjectile::Setup(pWeapon, info);

	if (m_aModifiers[7].bEnabled)
	{
		m_fLifeTime = m_ModifierData.m7.fLifeTime;
	}

	// RETAIL: the class data is looked up by the name at CProjectile 0x108
	// when the projectile has ammo (0x1003f611); it is not checked afterwards.
	PRANKSTERCLASSDATA *pClassData = !m_pAmmoData ? LTNULL : dynamic_cast<PRANKSTERCLASSDATA*>(g_pFXButeMgr->GetProjectileClassData(m_ModifierData.m7.szClassData));

	m_fImplodeVelocity			= GetRandom(-pClassData->fVelocityRange, pClassData->fVelocityRange) + pClassData->fBaseVelocity;
	m_fDeathRadius				= pClassData->fDeathRadius;
	m_fMaxDistance				= pClassData->fMaxDistance;
	m_fTimeVictimStasis			= pClassData->fTimeVictimStasis;
	m_fTimeVictimStasisRange	= pClassData->fTimeVictimStasisRange;
	m_fTimeToAcquireTargets		= pClassData->fTimeToAcquireTargets;
	m_sEliminateTargetsFX		= pClassData->szEliminateTargetsFX;

	RequestStartSendingMessages();
	SetState(kPrankster_Flying);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::Detonate
//
//	PURPOSE:	Stop where we hit and start acquiring targets (RETAIL:
//				0x1003f700; CProjectile's, but the prankster stops instead
//				of being removed, and does its damage before its impact)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::Detonate(HOBJECT hObj, CollisionInfo* pCollInfo)
{
	if (m_bDetonated) return;

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
		if (g_vtProjectileNewCollision.GetFloat() != 0.0f)
		{
			// the nearest touch the new collision code kept
			ProjectilePendingTouch* pTouch = GetNearestPendingTouch(LTNULL);

			vPos = pTouch->vLastPos;
			eType = GetSurfaceType(pTouch->iInfo);
			vNormal = -pTouch->iInfo.m_Plane.m_Normal;
			m_fInstDamage = pTouch->fInstDamage;

			LTVector vZero(0.0f, 0.0f, 0.0f);
			g_pPhysicsLT->SetVelocity(m_hObject, &vZero);
		}
		else if (IsMainWorld(hObj) || GetObjectType(hObj) == OT_WORLDMODEL)
		{
			// get the collision info for this impact
			CollisionInfo info;
			g_pLTServer->GetLastCollision(&info);

			// stop
			LTVector vStop = -info.m_vStopVel;
			g_pPhysicsLT->SetVelocity(m_hObject, &vStop);

			// check if we have a valid polygon
			if (info.m_hPoly != INVALID_HPOLY)
			{
				// get the surface type
				eType = GetSurfaceType(info.m_hPoly);
			}

			// get the normal of the plane we impacted with
			LTPlane plane = info.m_Plane;
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
				LTFLOAT fDot1 = VEC_DOT(vNormal, vP0) - info.m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT(vNormal, vP1) - info.m_Plane.m_Dist;

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

	m_vTronUnknownD0 = vPos;

	// Handle impact damage...
	if (hObj)
	{
		HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : LTObjRef(m_hObject);

		ImpactDamageObject(hDamager, hObj);
	}

	// do all the impact work
	AddImpact(hObj, m_vFlashPos, vPos, vNormal, eType);

	SetState(kPrankster_Acquiring);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::EngineMessageFn
//
//	PURPOSE:	Handle engine messages (RETAIL: 0x1003fdc0)
//
// ----------------------------------------------------------------------- //

uint32 CProjectilePrankster::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_INITIALUPDATE:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FULLPOSITIONRES, FLAG_FULLPOSITIONRES | FLAG_RAYHIT);
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
	}

	return CTronProjectile::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::ObjectMessageFn
//
//	PURPOSE:	The owner stops the flight (RETAIL: 0x1003fea0)
//
// ----------------------------------------------------------------------- //

uint32 CProjectilePrankster::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->Readuint32();
	if (pMsg->Readuint8() == PRANKSTER_MPROJ_DETONATE)
	{
		Detonate(LTNULL, LTNULL);
	}

	return CTronProjectile::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::TronUnknownVirtual30
//
//	PURPOSE:	Touch only while flying (RETAIL: 0x1003fee0)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::TronUnknownVirtual30(HOBJECT hObj)
{
	if (m_bSetup && !m_bObjectRemoved && !m_bProcessInvImpact && hObj && !m_bDetonated)
	{
		CTronProjectile::TronUnknownVirtual30(hObj);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::ImpactDamageObject
//
//	PURPOSE:	Damage an object only once (RETAIL: 0x1003ff10)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj)
{
	if (!IsDamagedObject(hObj))
	{
		CTronProjectile::ImpactDamageObject(hDamager, hObj);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::AddSpecialFX
//
//	PURPOSE:	Add the client-side special fx (RETAIL: 0x1003ff40)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::AddSpecialFX()
{
	if (!g_pWeaponMgr) return;

	uint8 nShooterId = -1;
	if (IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hFiredFrom));
		if (pPlayer)
		{
			nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(PRANKSTER_SFX_PROJECTILE_ID);
	cMsg.Writeuint8(m_pWeaponData->nId);
	cMsg.Writeuint8(m_pAmmoData->nId);
	cMsg.Writeuint8(nShooterId);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::Save
//
//	PURPOSE:	Save the object (RETAIL: 0x100400d0)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	pMsg->Writeuint8((uint8)m_eState);
	m_AcquireTimer.Save(pMsg);

	pMsg->Writeuint8((uint8)m_Victims.size());
	for (VictimMap::iterator it = m_Victims.begin(); it != m_Victims.end(); it++)
	{
		SAVE_HOBJECT(it->first);
		it->second.Save(pMsg);
	}

	SAVE_FLOAT(m_fImplodeVelocity);
	SAVE_FLOAT(m_fDeathRadius);
	SAVE_FLOAT(m_fMaxDistance);
	SAVE_FLOAT(m_fTimeVictimStasis);
	SAVE_FLOAT(m_fTimeVictimStasisRange);
	SAVE_FLOAT(m_fTimeToAcquireTargets);
	SAVE_STDSTRING(m_sEliminateTargetsFX);
	SAVE_bool(m_bSentDone);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::Load
//
//	PURPOSE:	Load the object (RETAIL: 0x10040280)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	m_eState = pMsg->Readuint8();
	m_AcquireTimer.Load(pMsg);

	int nVictims = pMsg->Readuint8();
	m_Victims.clear();
	for (int i = nVictims; i; i--)
	{
		LTObjRefNotifier hVictim;
		hVictim = pMsg->ReadObject();

		CPranksterVictimTimer cTimer;
		cTimer.Load(pMsg);

		m_Victims.insert(std::make_pair(hVictim, cTimer));
	}

	LOAD_FLOAT(m_fImplodeVelocity);
	LOAD_FLOAT(m_fDeathRadius);
	LOAD_FLOAT(m_fMaxDistance);
	LOAD_FLOAT(m_fTimeVictimStasis);
	LOAD_FLOAT(m_fTimeVictimStasisRange);
	LOAD_FLOAT(m_fTimeToAcquireTargets);

	char szFX[128];
	pMsg->ReadString(szFX, ARRAY_LEN(szFX));
	m_sEliminateTargetsFX = szFX;

	LOAD_bool(m_bSentDone);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::RemoveObject
//
//	PURPOSE:	Remove the object (RETAIL: 0x100405c0)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::RemoveObject()
{
	SendDone();
	CTronProjectile::RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::Update
//
//	PURPOSE:	MID_UPDATE (RETAIL: 0x100405e0)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::Update()
{
	switch (m_eState)
	{
		case kPrankster_Flying:		UpdateFlying();		break;
		case kPrankster_Acquiring:	UpdateAcquiring();	break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::UpdateFlying
//
//	PURPOSE:	Fly where the owner looks (RETAIL: 0x10040600)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::UpdateFlying()
{
	CCharacter *pOwner = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hFiredFrom));
	LTRotation rRot = pOwner->GetRotationWithPitch();
	LTVector vForward = rRot.Forward();

	LTVector vVel = vForward * (LTFLOAT)m_pAmmoData->pProjectileFX->nVelocity;
	g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::UpdateAcquiring
//
//	PURPOSE:	Catch characters until the time is up, then wait for the
//				AIs in stasis (RETAIL: 0x100406e0)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::UpdateAcquiring()
{
	if (!m_AcquireTimer.Stopped())
	{
		AcquireTargets(m_fMaxDistance);
		UpdateVictims();
		return;
	}

	VictimMap::iterator itEnd = m_Victims.end();
	for (VictimMap::iterator it = m_Victims.begin(); it != itEnd; ++it)
	{
		CAI *pAI = dynamic_cast<CAI*>(g_pLTServer->HandleToObject(it->first));
		if (pAI && pAI->GetGoalMgr()->FindGoalByType(PRANKSTER_AIGOAL_STASIS))
		{
			UpdateVictims();
			return;
		}
	}

	SetState(kPrankster_Done);
	UpdateVictims();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::SendDone
//
//	PURPOSE:	Tell the owner's client the prankster bit is gone, once
//				(RETAIL: 0x10040810)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::SendDone()
{
	if (m_bSentDone) return;
	if (!g_pLTServer) return;

	m_bSentDone = true;

	CAutoMessage cMsg;
	cMsg.Writeuint8(PRANKSTER_MID_PROJECTILE);
	cMsg.Writeuint8(PRANKSTER_MPROJ_DONE);

	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hFiredFrom));
	if (pPlayer)
	{
		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
			g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::AcquireTargets
//
//	PURPOSE:	Catch the characters near us that we can see
//				(RETAIL: 0x10040950)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::AcquireTargets(LTFLOAT fRadius)
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// RETAIL: the list is never given back (RelinquishList): see
	// docs/RETAIL_BUGS.md
	ObjectList *pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, fRadius);
#ifdef TRON20SRC_MODERN
	// GAME-76: give the list back
	if (!pList) return;
	if (!pList->m_pFirstLink)
	{
		g_pLTServer->RelinquishList(pList);
		return;
	}
#else
	if (!pList || !pList->m_pFirstLink) return;
#endif

	for (ObjectLink *pLink = pList->m_pFirstLink; pLink; pLink = pLink->m_pNext)
	{
		if (!IsCharacter(pLink->m_hObject) || IsPlayer(pLink->m_hObject)) continue;

		LTObjRefNotifier hVictim(pLink->m_hObject);
		hVictim.SetReceiver(*this);

		if (hVictim)
		{
			bool bNew = (m_Victims.find(LTObjRefNotifier(pLink->m_hObject)) == m_Victims.end());
			if (bNew && CanSee(m_hObject, pLink->m_hObject) && IsNotSpeaker(pLink->m_hObject))
			{
				AddVictim(hVictim);
			}
		}
	}

#ifdef TRON20SRC_MODERN
	g_pLTServer->RelinquishList(pList);
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::AddVictim
//
//	PURPOSE:	Put a character in stasis (RETAIL: 0x10040b00)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::AddVictim(LTObjRefNotifier const& hVictim)
{
	CPranksterVictimTimer cTimer;
	cTimer.Start(GetRandom(-m_fTimeVictimStasisRange, m_fTimeVictimStasisRange) + m_fTimeVictimStasis);

	m_Victims.insert(std::make_pair(hVictim, cTimer));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::UpdateVictims
//
//	PURPOSE:	Hold the victims in stasis; implode the ones whose time is
//				up (RETAIL: 0x10040c50)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::UpdateVictims()
{
	if (m_Victims.size() <= 0) return;

	DamageStruct damage;
	damage.hDamager = m_hFiredFrom;
	damage.eType = DT_PRANKSTERBIT;

	for (VictimMap::iterator it = m_Victims.begin(); it != m_Victims.end(); it++)
	{
		if (!it->first) continue;

		damage.DoDamage(dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hFiredFrom)), it->first);

		if (!it->second.Stopped()) continue;

		LTVector vPos, vVictimPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->GetObjectPos(it->first, &vVictimPos);

		LTVector vDir(vVictimPos.x - vPos.x, vVictimPos.y - vPos.y, vVictimPos.z - vPos.z);
		vDir = vDir / vDir.Length();

		CCharacter *pVictim = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(it->first));
		pVictim->GetDestructible()->SetTronImplode(m_fDeathRadius, m_fImplodeVelocity, vDir);

		DamageStruct implode;
		implode.hDamager = m_hFiredFrom;
		implode.vDir = vDir;
		implode.eType = DT_PRANKSTERBITIMPLODE;
		implode.fDamage = 10000.0f;
		implode.DoDamage(this, it->first);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::SetState
//
//	PURPOSE:	Change the state (RETAIL: 0x10041050)
//
// ----------------------------------------------------------------------- //

void CProjectilePrankster::SetState(int eState)
{
	if (eState < kPrankster_None || eState >= kPrankster_NumStates) return;

	switch (eState)
	{
		case kPrankster_Acquiring:
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(PRANKSTER_MID_SFX_MESSAGE);
			cMsg.Writeuint8(PRANKSTER_SFX_PROJECTILE_ID);
			cMsg.WriteObject(m_hObject);
			cMsg.Writeuint8(PRANKSTER_PFX_ACQUIRE);
			cMsg.WriteString("");
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

			m_AcquireTimer.Start(m_fTimeToAcquireTargets);

			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);

			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			LTRotation rRot;
			g_pLTServer->GetObjectRotation(m_hObject, &rRot);

			PlayClientFX((char*)m_sEliminateTargetsFX.c_str(), LTNULL, &vPos, &rRot, 0);
		}
		break;

		case kPrankster_Done:
		{
			RemoveObject();
		}
		break;
	}

	m_eState = eState;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::CanSee
//
//	PURPOSE:	Nothing solid between the two objects, either way
//				(RETAIL: 0x10041210)
//
// ----------------------------------------------------------------------- //

bool CProjectilePrankster::CanSee(HOBJECT hFrom, HOBJECT hTo)
{
	LTVector vFrom, vTo;
	g_pLTServer->GetObjectPos(hFrom, &vFrom);
	g_pLTServer->GetObjectPos(hTo, &vTo);

	IntersectInfo iInfo;
	IntersectQuery qInfo;

	qInfo.m_From = vFrom;
	qInfo.m_To = vTo;
	qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_FilterFn = ExplosionFilterFn;

	bool bHit1 = g_pLTServer->IntersectSegment(&qInfo, &iInfo);

	qInfo.m_From = vTo;
	qInfo.m_To = vFrom;

	bool bHit2 = g_pLTServer->IntersectSegment(&qInfo, &iInfo);

	if (!bHit1 && !bHit2) return true;
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectilePrankster::IsNotSpeaker
//
//	PURPOSE:	Speakers are not victims (RETAIL: 0x100413a0)
//
// ----------------------------------------------------------------------- //

bool CProjectilePrankster::IsNotSpeaker(HOBJECT hObj)
{
	static HCLASS hSpeaker = g_pLTServer->GetClass("Speaker");

	HCLASS hClass = g_pLTServer->GetObjectClass(hObj);
	return g_pLTServer->IsKindOf(hClass, hSpeaker) ? false : true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPranksterVictimTimer::Save/Load
//
//	PURPOSE:	Save/load a victim's timer (RETAIL: 0x10041780, 0x10041870)
//
// ----------------------------------------------------------------------- //

inline void CPranksterVictimTimer::Save(ILTMessage_Write *pMsg)
{
	CTimer::Save(pMsg);
}

inline void CPranksterVictimTimer::Load(ILTMessage_Read *pMsg)
{
	CTimer::Load(pMsg);
}
