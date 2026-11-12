// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileEnergyClaw.cpp
//
// PURPOSE : The energy claw - implementation
//
// RETAIL: Tron 2.0's, written from retail Object.lto
// (ProjectileEnergyClaw.h).  The small functions are exact; the state
// updates (UpdateSeeking..UpdateReturning, HandleReturned, CreateClawFX)
// follow retail's logic but are not matched yet (PORT: near).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileEnergyClaw.h"
#include "WeaponFireInfo.h"
#include "FXButeMgr.h"
#include "MsgIDs.h"
#include "PlayerObj.h"
#include "Destructible.h"

BEGIN_CLASS(CProjectileEnergyClaw)
END_CLASS_DEFAULT_FLAGS(CProjectileEnergyClaw, CTronProjectile, NULL, NULL, CF_HIDDEN)

// RETAIL: the class tested for (0x102dbd34 points at "CAI")
static const char *s_szClawTargetClass = "CAI";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::~CProjectileEnergyClaw
//
//	PURPOSE:	Destructor (RETAIL: 0x1003da90)
//
// ----------------------------------------------------------------------- //

CProjectileEnergyClaw::~CProjectileEnergyClaw()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::CProjectileEnergyClaw
//
//	PURPOSE:	Constructor (RETAIL: 0x1003db30)
//
// ----------------------------------------------------------------------- //

CProjectileEnergyClaw::CProjectileEnergyClaw()
	: CTronProjectile(),
	  m_eClawState( kClaw_None ),
	  m_hTarget( LTNULL ),
	  m_hClawFX( LTNULL ),
	  m_fTronUnknown93C( 0.0f )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::Setup
//
//	PURPOSE:	Set up; start seeking (RETAIL: 0x1003dbd0)
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileEnergyClaw::Setup(CWeapon const* pWeapon, WeaponFireInfo const& info)
{
	LTBOOL bRet = CTronProjectile::Setup(pWeapon, info);

	RequestStartSendingMessages();
	SetClawState(kClaw_Seeking);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::EngineMessageFn
//
//	PURPOSE:	Handle engine messages (RETAIL: 0x1003dc00)
//
// ----------------------------------------------------------------------- //

uint32 CProjectileEnergyClaw::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
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

		case MID_UPDATE:
		{
			Update();
		}
		break;
	}

	return CTronProjectile::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::ObjectMessageFn
//
//	PURPOSE:	The owner asks the claw back (RETAIL: 0x1003dc90)
//
// ----------------------------------------------------------------------- //

uint32 CProjectileEnergyClaw::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	pMsg->Readuint32();
	if (pMsg->Readuint8() == MPROJ_RETURN)
	{
		SetClawState(kClaw_Returning);
		HasClawFX();
	}

	return CTronProjectile::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::HandleTouch
//
//	PURPOSE:	A returning claw goes when it reaches its owner
//				(RETAIL: 0x1003dce0)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::HandleTouch(HOBJECT hObj)
{
	if (m_eClawState == kClaw_Returning)
	{
		if (m_hFiredFrom == hObj)
		{
			HandleReturned();
			RemoveObject();
		}
		return;
	}

	CTronProjectile::HandleTouch(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::HandleImpact
//
//	PURPOSE:	Latch on to an AI (RETAIL: 0x1003dd20)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::HandleImpact(HOBJECT hObj)
{
	if (IsKindOf(hObj, s_szClawTargetClass))
	{
		SetClawState(kClaw_Attached);
		m_hTarget = hObj;
		CreateClawFX(m_hTarget);
		return;
	}

	CTronProjectile::HandleImpact(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::RemoveObject
//
//	PURPOSE:	Remove the claw (RETAIL: 0x1003dd90)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::RemoveObject()
{
	RequestStopSendingMessages();
	CTronProjectile::RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::Load
//
//	PURPOSE:	Load the object (RETAIL: 0x1003ddb0)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	m_eClawState = pMsg->Readuint8();
	LOAD_HOBJECT(m_hTarget);
	LOAD_HOBJECT(m_hClawFX);
	LOAD_FLOAT(m_fTronUnknown93C);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::Save
//
//	PURPOSE:	Save the object (RETAIL: 0x1003de30)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	pMsg->Writeuint8((uint8)m_eClawState);
	SAVE_HOBJECT(m_hTarget);
	SAVE_HOBJECT(m_hClawFX);
	SAVE_FLOAT(m_fTronUnknown93C);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::Update
//
//	PURPOSE:	MID_UPDATE (RETAIL: 0x1003de90)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::Update()
{
	switch (m_eClawState)
	{
		case kClaw_Seeking:		UpdateSeeking();	break;
		case kClaw_Chasing:		UpdateChasing();	break;
		case kClaw_Attached:	UpdateAttached();	break;
		case kClaw_Returning:	UpdateReturning();	break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::UpdateSeeking
//
//	PURPOSE:	Find the nearest AI ahead that we can see
//				(RETAIL: 0x1003ded0; PORT: near)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::UpdateSeeking()
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	ObjectList *pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, 100.0f);
	if (!pList) return;

	LTObjRef hBest;
	LTFLOAT fBestDist = 0.0f;

	ObjectLink *pLink = pList->m_pFirstLink;
	while (pLink)
	{
		HOBJECT hObj = pLink->m_hObject;
		pLink = pLink->m_pNext;

		char szClass[0x1000];
		g_pLTServer->GetClassName(g_pLTServer->GetObjectClass(hObj), szClass, sizeof(szClass));

		if (!IsKindOf(hObj, s_szClawTargetClass)) continue;

		LTVector vObjPos, vVel;
		g_pLTServer->GetObjectPos(hObj, &vObjPos);
		g_pPhysicsLT->GetVelocity(m_hObject, &vVel);
		vVel.Normalize();

		LTVector vTo = vObjPos - vPos;
		LTFLOAT fDist = vTo.Mag();
		if (vVel.Dot(vTo.Unit()) < 0.0f) continue;
		if (hBest && fDist > fBestDist) continue;
		if (!CanSee(m_hObject, hObj)) continue;

		hBest = hObj;
		fBestDist = fDist;
	}

	if (hBest)
	{
		m_hTarget = hBest;
		SetClawState(kClaw_Chasing);
	}

	g_pLTServer->RelinquishList(pList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::UpdateChasing
//
//	PURPOSE:	Fly at the target (RETAIL: 0x1003e1d0; PORT: near)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::UpdateChasing()
{
	if (!CanSee(m_hObject, m_hTarget))
	{
		SetClawState(kClaw_Seeking);
		return;
	}

	LTVector vPos, vTargetPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectPos(m_hTarget, &vTargetPos);

	LTVector vDir = vTargetPos - vPos;
	vDir.Normalize();

	LTVector vVel = vDir * m_fVelocity;
	g_pPhysicsLT->SetVelocity(m_hObject, &vVel);

	m_vFirePos = vPos;
	m_vDir = vDir;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::UpdateAttached
//
//	PURPOSE:	Ride on the target and damage it (RETAIL: 0x1003e320;
//				PORT: near)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::UpdateAttached()
{
	if (!m_hTarget)
	{
		SetClawState(kClaw_Returning);
		HasClawFX();
		return;
	}

	LTVector vZero(0.0f, 0.0f, 0.0f);
	g_pPhysicsLT->SetVelocity(m_hObject, &vZero);

	LTVector vTargetPos;
	g_pLTServer->GetObjectPos(m_hTarget, &vTargetPos);
	g_pLTServer->SetObjectPos(m_hObject, &vTargetPos);

	UpdateClawFX();

	DamageStruct damage;
	damage.eType	= m_eInstDamageType;
	damage.fDamage	= m_fInstDamage;
	damage.hDamager	= m_hFiredFrom;
	damage.vDir		= m_vDir;
	damage.DoDamage(this, m_hTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::UpdateReturning
//
//	PURPOSE:	Fly back to the owner (RETAIL: 0x1003e510; PORT: near)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::UpdateReturning()
{
	LTVector vPos, vOwnerPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectPos(m_hFiredFrom, &vOwnerPos);

	LTVector vDir = vOwnerPos - vPos;
	vDir.Normalize();

	LTVector vVel = vDir * m_fVelocity;
	g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::HandleReturned
//
//	PURPOSE:	Tell the owning player the claw is back
//				(RETAIL: 0x1003e600; PORT: near)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::HandleReturned()
{
	if (!IsPlayer(m_hFiredFrom)) return;

	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hFiredFrom));
	if (!pPlayer) return;

	HCLIENT hClient = pPlayer->GetClient();
	if (!hClient) return;

	// RETAIL: the message id is written as a byte
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PROJECTILE);
	cMsg.Writeuint8(MPROJ_RETURNED);
	cMsg.Writeuint8(0);
	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::CreateClawFX
//
//	PURPOSE:	Create the looping claw effect on the target
//				(RETAIL: 0x1003e740; PORT: near; the effect's name is
//				retail's code's, here the projectile's FX)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::CreateClawFX(HOBJECT hTarget)
{
	ObjectCreateStruct ocs;
	ocs.Clear();

	g_pLTServer->GetObjectPos(hTarget, &ocs.m_Pos);
	ocs.m_Scale.Init(1.0f, 1.0f, 1.0f);
	ocs.m_Rotation.Init();

	const char *pFXName = (m_pAmmoData && m_pAmmoData->pProjectileFX) ? m_pAmmoData->pProjectileFX->szFXName : "";
	ocs.m_cProperties.AddProp("FxName", GenericProp(pFXName, LT_PT_STRING));
	ocs.m_cProperties.AddProp("Loop", GenericProp(true, LT_PT_BOOL));
	ocs.m_cProperties.AddProp("SmoothShutdown", GenericProp(true, LT_PT_BOOL));

	HCLASS hClass = g_pLTServer->GetClass("SpecialFX");
	if (!hClass) return;

	BaseClass *pFX = (BaseClass*)g_pLTServer->CreateObject(hClass, &ocs);
	if (!pFX) return;

	m_hClawFX = pFX->m_hObject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::UpdateClawFX
//
//	PURPOSE:	Keep the effect on the target (RETAIL: 0x1003eb30)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::UpdateClawFX()
{
	if (m_hClawFX)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hTarget, &vPos);
		g_pLTServer->SetObjectPos(m_hClawFX, &vPos);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::HasClawFX
//
//	PURPOSE:	RETAIL: 0x1003eb80
//
// ----------------------------------------------------------------------- //

bool CProjectileEnergyClaw::HasClawFX() const
{
	return m_hClawFX ? true : false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::CanSee
//
//	PURPOSE:	Nothing between the two objects? (RETAIL: 0x1003eb90)
//
// ----------------------------------------------------------------------- //

bool CProjectileEnergyClaw::CanSee(HOBJECT hFrom, HOBJECT hTo)
{
	if (!hFrom) return false;
	if (!hTo) return false;

	// RETAIL BUG: both ends are hFrom's position (0x1003ebdd, 0x1003ebee),
	// so the segment is empty and the claw always "sees" (docs/RETAIL_BUGS.md)
	LTVector vFrom, vTo;
	g_pLTServer->GetObjectPos(hFrom, &vFrom);
	g_pLTServer->GetObjectPos(hFrom, &vTo);

	HOBJECT hFilterList[] = { m_hObject, m_hFiredFrom, hFrom, hTo, LTNULL };

	IntersectQuery qInfo;
	IntersectInfo iInfo;

	qInfo.m_From		= vFrom;
	qInfo.m_To			= vTo;
	qInfo.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_FilterFn	= ObjListFilterFn;
	qInfo.m_pUserData	= hFilterList;

	if (g_pLTServer->IntersectSegment(&qInfo, &iInfo)) return false;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileEnergyClaw::SetClawState
//
//	PURPOSE:	Change state (RETAIL: 0x1003ed10)
//
// ----------------------------------------------------------------------- //

void CProjectileEnergyClaw::SetClawState(int eState)
{
	switch (eState)
	{
		case kClaw_Attached:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0x200100, 0x8272101);
		}
		break;

		case kClaw_Returning:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0x10001, 0x10001);
		}
		break;
	}

	m_eClawState = eState;
}
