// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileBall.cpp
//
// PURPOSE : A ball that sticks to the floor it lands on - implementation
//
// RETAIL: Tron 2.0's, written from retail Object.lto (ProjectileBall.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileBall.h"
#include "ServerUtilities.h"
#include "SurfaceFunctions.h"
#include "ContainerCodes.h"
#include "CVarTrack.h"
#include "FXButeMgr.h"
#include "MsgIDs.h"
#include "SFXMsgIds.h"
#include "AIUtils.h"

// RETAIL: Projectile.cpp's (0x1037c290)
extern CVarTrack g_vtProjectileNewCollision;

// RETAIL: Explosion.cpp's (0x10035200)
extern bool ExplosionFilterFn(HOBJECT hObj, void *pUserData);

BEGIN_CLASS(CProjectileBall)
END_CLASS_DEFAULT_FLAGS(CProjectileBall, CGrenade, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::~CProjectileBall
//
//	PURPOSE:	Destructor (RETAIL: 0x10033c40)
//
// ----------------------------------------------------------------------- //

CProjectileBall::~CProjectileBall()
{
	if (m_pArmTimer)
	{
		debug_delete(m_pArmTimer);
		m_pArmTimer = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::CProjectileBall
//
//	PURPOSE:	Constructor (RETAIL: 0x10033cb0)
//
// ----------------------------------------------------------------------- //

CProjectileBall::CProjectileBall()
	: CGrenade(),
	  m_bStuck					( false ),
	  m_bArmed					( false ),
	  m_fProximityThreshold		( 0.0f ),
	  m_fProximitySecondsToArm	( 0.0f ),
	  m_fRandomAngleAdjust		( 0.0f )
{
	m_pArmTimer = debug_new(CTimer);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::Setup
//
//	PURPOSE:	Read the ball's class data (RETAIL: 0x10033d50)
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileBall::Setup(CWeapon const* pWeapon, WeaponFireInfo const& info)
{
	if (!CGrenade::Setup(pWeapon, info)) return LTFALSE;

	PROJECTILEFX* pProjectileFX = m_pAmmoData->pProjectileFX;
	if (!pProjectileFX) return LTFALSE;

	BALLCLASSDATA* pClassData = dynamic_cast<BALLCLASSDATA*>(pProjectileFX->pClassData);
	if (!pClassData) return LTFALSE;

	m_fProximityThreshold		= pClassData->fProximityThreshold;
	m_fProximitySecondsToArm	= pClassData->fProximitySecondsToArm;
	m_fProximityDistance		= pClassData->fProximityDistance;
	m_fRandomAngleAdjust		= pClassData->fRandomAngleAdjust;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::HandleTouch
//
//	PURPOSE:	Stick to the world soon after we are fired, if the surface
//				is a floor (RETAIL: 0x10033de0)
//
// ----------------------------------------------------------------------- //

void CProjectileBall::HandleTouch(HOBJECT hObj)
{
	if (!m_bStuck && IsMainWorld(hObj) &&
		g_pLTServer->GetTime() - m_fStartTime < m_fProximityThreshold)
	{
		CollisionInfo info;
		g_pLTServer->GetLastCollision(&info);

		if (fabs(info.m_Plane.m_Normal.y) < 0.1)
		{
			StickToLastCollision();
			return;
		}
	}

	CGrenade::HandleTouch(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::Save
//
//	PURPOSE:	Save the object (RETAIL: 0x10033e90)
//
// ----------------------------------------------------------------------- //

void CProjectileBall::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	SAVE_bool(m_bStuck);
	SAVE_bool(m_bArmed);
	m_pArmTimer->Save(pMsg);
	SAVE_FLOAT(m_fProximityThreshold);
	SAVE_FLOAT(m_fProximitySecondsToArm);
	SAVE_FLOAT(m_fProximityDistance);
	SAVE_FLOAT(m_fRandomAngleAdjust);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::Load
//
//	PURPOSE:	Load the object (RETAIL: 0x10033fa0)
//
// ----------------------------------------------------------------------- //

void CProjectileBall::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	LOAD_bool(m_bStuck);
	LOAD_bool(m_bArmed);
	m_pArmTimer->Load(pMsg);
	LOAD_FLOAT(m_fProximityThreshold);
	LOAD_FLOAT(m_fProximitySecondsToArm);
	LOAD_FLOAT(m_fProximityDistance);
	LOAD_FLOAT(m_fRandomAngleAdjust);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::EngineMessageFn
//
//	PURPOSE:	Handle engine messages (RETAIL: 0x10034090)
//
// ----------------------------------------------------------------------- //

uint32 CProjectileBall::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_UPDATE:
		{
			if (Update())
			{
				g_pLTServer->SetNextUpdate(m_hObject, 0.00001f);
				return 1;
			}
		}
		break;

		default : break;
	}

	return CGrenade::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::TronUnknownVirtual33
//
//	PURPOSE:	A kept touch: stick to a floor soon after we are fired
//				(RETAIL: 0x100340f0)
//
// ----------------------------------------------------------------------- //

bool CProjectileBall::TronUnknownVirtual33(ProjectilePendingTouch *pTouch)
{
	if (!pTouch) return false;

	SurfaceType eSurface = GetSurfaceType(pTouch->collInfo);
	if (eSurface != ST_SKY && eSurface != ST_INVISIBLE && !m_bStuck &&
		g_pLTServer->GetTime() - m_fStartTime < m_fProximityThreshold &&
		!IsCharacter(pTouch->hObject) && !IsAlarm(pTouch->hObject) &&
		fabs(pTouch->iInfo.m_Plane.m_Normal.y) < 0.1)
	{
		StickTo(pTouch);
		return true;
	}

	return CGrenade::TronUnknownVirtual33(pTouch);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::Update
//
//	PURPOSE:	Update the ball (RETAIL: 0x100341a0)
//
// ----------------------------------------------------------------------- //

bool CProjectileBall::Update()
{
	if (m_bStuck)
	{
		return UpdateStuck();
	}

	return UpdateFlight();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::UpdateStuck
//
//	PURPOSE:	Arm when the timer runs out; once armed, blow up when a
//				character in sight is near (RETAIL: 0x100341c0)
//
// ----------------------------------------------------------------------- //

bool CProjectileBall::UpdateStuck()
{
	if (m_bArmed)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, m_fProximityDistance);
		if (!pList) return true;

		ObjectLink* pLink = pList->m_pFirstLink;
		while (pLink)
		{
			if (IsCharacter(pLink->m_hObject))
			{
				LTVector vObjPos;
				g_pLTServer->GetObjectPos(pLink->m_hObject, &vObjPos);

				IntersectQuery qInfo;
				IntersectInfo iInfo;

				LTVector vDiff = vObjPos - vPos;
				LTFLOAT fInvDist = 1.0f / vDiff.Mag();
				LTVector vDir = vDiff * fInvDist;

				qInfo.m_From		= vDir + vPos;
				qInfo.m_To			= vObjPos;
				qInfo.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
				qInfo.m_FilterFn	= ExplosionFilterFn;
				qInfo.m_pUserData	= LTNULL;

				bool bBlocked = g_pLTServer->IntersectSegment(&qInfo, &iInfo);

				qInfo.m_From		= vObjPos;
				qInfo.m_To			= vPos + vDiff * fInvDist;

				if (!bBlocked && !g_pLTServer->IntersectSegment(&qInfo, &iInfo))
				{
					Detonate(LTNULL);
				}
			}

			pLink = pLink->m_pNext;
		}

		g_pLTServer->RelinquishList(pList);
	}
	else if (m_pArmTimer->Stopped())
	{
		m_bArmed = true;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::UpdateFlight
//
//	PURPOSE:	Turn the ball a little to the side at random as it flies,
//				spin it, and blow it up when its life ends (RETAIL:
//				0x10034550; its spin and its end are CGrenade's and
//				CProjectile's)
//
// ----------------------------------------------------------------------- //

bool CProjectileBall::UpdateFlight()
{
	if (g_vtProjectileNewCollision.GetFloat() != 0.0f)
	{
		m_bTronUnknown1C0 = true;
		TronUnknownVirtual32();
		m_bTronUnknown1C0 = false;
	}

	// Update my container code...

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	m_eContainerCode = ::GetContainerCode(vPos);

	// Turn a little to the side...

	LTVector vVel;
	g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

	LTFLOAT fSpeed = vVel.Mag();
	if (fSpeed > 0.1f)
	{
		LTVector vDir = vVel.Unit();

		LTRotation rRot(vDir, LTVector(0.0f, 1.0f, 0.0f));

		LTFLOAT fAngle = GetRandom(-m_fRandomAngleAdjust, m_fRandomAngleAdjust);
		LTVector vSide((LTFLOAT)sin(fAngle) * fSpeed, 0.0f, 0.0f);

		LTMatrix mRot;
		rRot.ConvertToMatrix(mRot);

		vVel += mRot * vSide;
		g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
	}

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

	// Detonate if the life time has expired

	if (g_pLTServer->GetTime() >= (m_fStartTime + m_fLifeTime))
	{
		Detonate(LTNULL);
	}

	g_pLTServer->GetObjectPos(m_hObject, &m_vLastPos);
	g_pPhysicsLT->GetVelocity(m_hObject, &m_vLastVel);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::StickTo
//
//	PURPOSE:	Stop on the surface a kept touch hit, stand up on it, start
//				arming and show the proximity FX (RETAIL: 0x10034bf0)
//
// ----------------------------------------------------------------------- //

void CProjectileBall::StickTo(ProjectilePendingTouch *pTouch)
{
	if (pTouch)
	{
		LTVector vVel(0.0f, 0.0f, 0.0f);
		g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_GOTHRUWORLD, FLAG_GOTHRUWORLD | FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);

		LTVector vNormal = pTouch->iInfo.m_Plane.m_Normal;

		m_pArmTimer->Start(m_fProximitySecondsToArm);
		m_bStuck = true;

		CGrenade::RotateToRest();

		LTRotation rRot(vNormal, LTVector(0.0f, 1.0f, 0.0f));
		g_pLTServer->SetObjectRotation(m_hObject, &rRot);

		PROJECTILEFX* pProjectileFX = m_pAmmoData->pProjectileFX;
		if (!pProjectileFX) return;

		BALLCLASSDATA* pClassData = dynamic_cast<BALLCLASSDATA*>(pProjectileFX->pClassData);
		if (!pClassData) return;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_PROJECTILE_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(1);
		cMsg.WriteString(pClassData->szProximityBombEffectName);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileBall::StickToLastCollision
//
//	PURPOSE:	As StickTo, for the engine's last collision (RETAIL:
//				0x10034ed0)
//
// ----------------------------------------------------------------------- //

void CProjectileBall::StickToLastCollision()
{
	LTVector vVel(0.0f, 0.0f, 0.0f);

	CollisionInfo info;
	g_pLTServer->GetLastCollision(&info);

	vVel -= info.m_vStopVel;
	g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_GOTHRUWORLD, FLAG_GOTHRUWORLD | FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);

	LTVector vNormal = info.m_Plane.m_Normal;

	m_pArmTimer->Start(m_fProximitySecondsToArm);
	m_bStuck = true;

	CGrenade::RotateToRest();

	LTRotation rRot(vNormal, LTVector(0.0f, 1.0f, 0.0f));
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);

	PROJECTILEFX* pProjectileFX = m_pAmmoData->pProjectileFX;
	if (!pProjectileFX) return;

	BALLCLASSDATA* pClassData = dynamic_cast<BALLCLASSDATA*>(pProjectileFX->pClassData);
	if (!pClassData) return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_PROJECTILE_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(1);
	cMsg.WriteString(pClassData->szProximityBombEffectName);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}
