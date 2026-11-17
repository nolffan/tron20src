// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerCamera.cpp
//
// PURPOSE : PlayerCamera - Implementation
//
// CREATED : 10/5/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerCamera.h"
#include "iltclient.h"
#include <stdio.h>
#include "GameClientShell.h"
#include "VarTrack.h"
#include "VehicleMgr.h"
#include "CMoveMgr.h"
#include "ProfileMgr.h"
#include "InterfaceMgr.h"
#include "SFXMgr.h"
#include "../TRON/LightCycleFX.h"

VarTrack		g_vtCameraMoveUpTime;
VarTrack		g_vtCameraMoveUpMinVel;
VarTrack		g_vtCameraMaxYDifference;
VarTrack		g_vtCameraClipDistance;
VarTrack		g_vtCameraCollisionObjScale;
// RETAIL: Tron's (CShell 0x10275328, 0x10275308, 0x102752f8; the names are
// ours, from their console variables)
VarTrack		g_vtCameraCollisionLightCycleObjScaleY;
VarTrack		g_vtCameraCollisionUseObject;
VarTrack		g_vtCameraUseFinalClipping;
VarTrack		g_vtCameraExtendSpeed;

#define CAMERA_COLLISION_MODEL			"models\\1x1_square.ltb"
#define DEFAULT_COLLISION_MODEL_SCALE	12.0f

// Camera's distance back from the player's position
#define DEFAULT_CAMERA_DIST_BACK	110
// Camera's distance up from the player's position
#define DEFAULT_CAMERA_DIST_UP		40
// Camera's distance back from the player's position but at a 45 degree angle
// (for example, if the camera is looking at the player from the southeast position)
#define DEFAULT_CAMERA_DIST_DIAG	106

// Camera's X offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_X	0
// Camera's Y offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_Y 10
// Camera's Z offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_Z 29

extern CGameClientShell*	g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CPlayerCamera()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerCamera::CPlayerCamera()
{
    m_hTarget           = NULL;
    m_pClientDE         = NULL;

    m_rRotation.Init();
    m_rLastTargetRot.Init();
	m_vPos.Init();
	m_vLastTargetPos.Init();
	m_vLastOptPos.Init();

	m_eOrientation		= SOUTH;
	m_eSaveOrientation	= SOUTH;

	m_OptX				= 0.0f;
	m_OptY				= 0.0f;
	m_OptZ				= 0.0f;
	m_CircleStartTime	= 0.0f;
	m_SaveAnglesY		= 0.0f;

    m_bSlide            = LTTRUE;

	m_CameraDistBack	= DEFAULT_CAMERA_DIST_BACK;
	m_CameraDistUp		= DEFAULT_CAMERA_DIST_UP;
	m_CameraDistDiag	= DEFAULT_CAMERA_DIST_DIAG;

	m_ePointType			= AT_POSITION;
    m_bStartCircle          = LTFALSE;
	m_CircleHeightOffset	= 0.0f;
	m_CircleRadius			= 75.0f;
	m_CircleTime			= 3.0f;
    m_bRestoreFirstPerson   = LTFALSE;

	m_GoingFirstPersonStart	= 0.0f;
	m_GoingFirstPersonTransition = 1.5f;

	m_eCameraMode			= CHASE;
	m_eSaveCameraMode		= CHASE;

	m_TargetFirstPersonOffset.Init();
	m_TargetChaseOffset.Init();
	m_TargetPointAtOffset.Init();

	m_hCollisionObject	= LTNULL;

	// RETAIL: Tron's (PlayerCamera.h)
	m_fTronUnknownF8	= 55.0f;
	m_fTronUnknownFC	= 94.0f;
	m_fTronUnknown100	= 1000.0f;
	m_fTronUnknown104	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::~CPlayerCamera()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPlayerCamera::~CPlayerCamera()
{
	if( m_hCollisionObject )
	{
		g_pLTClient->RemoveObject( m_hCollisionObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::Init()
//
//	PURPOSE:	Init the camera
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerCamera::Init(ILTClient* pClientDE)
{
    if (!pClientDE) return LTFALSE;
	m_pClientDE = pClientDE;

	g_vtCameraMoveUpTime.Init(pClientDE, "CameraMoveUpTime", NULL, 0.1f);
	g_vtCameraMoveUpMinVel.Init(pClientDE, "CameraMoveUpMinVel", NULL, 140.0f);
	g_vtCameraMaxYDifference.Init(pClientDE, "CameraMoveMaxYDiff", NULL, 30.0f);
	// RETAIL: Tron's clip distance is 12 (0x1015b450)
	g_vtCameraClipDistance.Init(pClientDE, "CameraClipDist", NULL, 12.0f);
	g_vtCameraCollisionObjScale.Init(pClientDE, "CameraCollisionObjScaleXZ", NULL, DEFAULT_COLLISION_MODEL_SCALE );
	g_vtCameraCollisionLightCycleObjScaleY.Init(pClientDE, "CameraCollisionLightCycleObjScaleY", NULL, 1.0f );
	g_vtCameraCollisionUseObject.Init(pClientDE, "CameraCollisionUseObject", NULL, 1.0f );
	g_vtCameraUseFinalClipping.Init(pClientDE, "CameraUseFinalClipping", NULL, 1.0f );
	g_vtCameraExtendSpeed.Init(pClientDE, "CameraExtendSpeed", NULL, 10.0f );

	CreateCameraCollisionObject();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CameraUpdate()
//
//	PURPOSE:	Update the position & orientation of the camera based
//				on the target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CameraUpdate(float deltaTime)
{
	if (!m_pClientDE || !m_hTarget) return;

    LTRotation rRot;
    LTVector vOpt, vPos;

	g_pLTClient->GetObjectPos(m_hTarget, &vPos);
	g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

    LTVector vF = rRot.Forward();

	if (m_hTarget)
	{
		if (m_eOrientation == HOLD)
			return;

		switch(m_eCameraMode)
		{
			case CHASE:
			break;

			case GOINGFIRSTPERSON:
			{
				vF.y = 0.0f;
				vPos += (vF * m_TargetFirstPersonOffset.x);
				vPos.y += m_TargetFirstPersonOffset.y;

				vOpt = vPos;
			}
			break;

			case CIRCLING:
			{
				CircleAroundTarget();
				return;
			}
			break;

			case FIRSTPERSON:
			{
				// RETAIL: Tron's points the camera itself (0x1015bd40).
				// PORT: retail's CameraUpdate (0x1015b5e0) does this only
				// while its +0x104 time is zero
				UpdateFirstPerson(vF, vPos, rRot, deltaTime);
				return;
			}
			break;

			case SCRIPT:
				return;
			break;

			default:
				return;
		}

		// Find the optimal position for the camera
		if (m_eCameraMode == GOINGFIRSTPERSON)
		{
			MoveCameraToPosition(vOpt, deltaTime);
			PointAtPosition(vOpt, rRot, m_vPos);
			return;
		}
		else
		{
			vOpt = FindOptimalCameraPosition();
		}

		// Move the camera to the optimal position
		// (it will slide or not depending on the m_bSlide param)
		MoveCameraToPosition(vOpt, deltaTime);

		// Either point the camera at the player or MLOOK
		if(m_eOrientation == MLOOK)
		{
            m_rRotation = rRot;
		}
		else
		{
			switch(m_ePointType)
			{
				case IN_DIRECTION:
					PointInDirection(rRot);
				break;

				case AT_POSITION:
				default:
				{
                    LTVector vTemp;
					vTemp = vPos + m_TargetPointAtOffset;
					PointAtPosition(vTemp, rRot, vOpt);
					// g_pLTClient->CPrint("Camera orientation : %0.4f, %0.4f, %0.4f", VEC_EXPAND(m_rRotation.Forward()));
				}
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateFirstPerson()
//
//	PURPOSE:	Update the position of the camera when in first person
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateFirstPerson(LTVector vF, LTVector vPos, LTRotation rRot, float fDeltaTime)
{
	vF.y = 0.0f;
	vPos += (vF * m_TargetFirstPersonOffset.x);
	vPos.y += m_TargetFirstPersonOffset.y;

	// RETAIL: Tron's (0x1015bd40) does not keep the standing height

	// Make sure we don't move the camera up too fast (going up stairs)...

	if (vPos.y > (m_vPos.y + 1.0f))
	{
        float fNewY = m_vPos.y;
		CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
		if (!pMoveMgr) return;

        LTBOOL bFreeMovement = pMoveMgr->IsFreeMovement();

		if (!bFreeMovement)
		{
            float fMaxYDiff = g_vtCameraMaxYDifference.GetFloat();
            float fYDiff = (vPos.y - fNewY);

			// Force the camera to never be more than fMaxYDiff away from
			// its "real" position...

			if (fYDiff > fMaxYDiff)
			{
				fNewY  += (fYDiff - fMaxYDiff);
				fYDiff = fMaxYDiff;
			}

            float fVel = fYDiff / g_vtCameraMoveUpTime.GetFloat();
			float fMinVel = g_vtCameraMoveUpMinVel.GetFloat();
			fNewY += ((fMinVel > fVel ? fMinVel : fVel) * fDeltaTime);

			if (fabs(fNewY - vPos.y) > 1.0f)
			{
				fNewY = fNewY < vPos.y ? fNewY : vPos.y;
				vPos.y = fNewY;
			}
		}
	}

	// RETAIL: Tron's keeps the position in +0x24 and points the camera
	m_vTronUnknown24 = vPos;

	if (TronUnknownMoveToTarget())
	{
		m_eCameraMode = FIRSTPERSON;
		m_rRotation = rRot;
		return;
	}

	PointAtPosition(m_vTronUnknown24, rRot, m_vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::Apply()
//
//	PURPOSE:	Matrix apply function
//
// ----------------------------------------------------------------------- //

LTVector CPlayerCamera::Apply(const LTVector &right, const LTVector &up, const LTVector &forward, const LTVector &copy)
{
    LTVector target;

	target.x = copy.x*right.x + copy.y*right.y + copy.z*right.z;
	target.y = copy.x*up.x + copy.y*up.y + copy.z*up.z;
	target.z = copy.x*forward.x + copy.y*forward.y + copy.z*forward.z;

	return target;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::PointAtPosition()
//
//	PURPOSE:	Point the camera at a position from a position
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::PointAtPosition(const LTVector &pos, const LTRotation &rRot, const LTVector &pointFrom)
{
	if (!m_pClientDE) return;

    LTVector vAngles;
	vAngles.Init();

	m_rRotation = rRot;

    float diffX = pos.x - m_vPos.x;
    float diffY = pos.z - m_vPos.z;
    vAngles.y = (float)atan2(diffX, diffY);

    LTVector     target, copy;

	copy = pos - pointFrom;

	target = Apply(m_rRotation.Right(), m_rRotation.Up(), m_rRotation.Forward(), copy);

	diffX = target.z;
	diffY = target.y;


    vAngles.x = (float)-atan2(diffY, diffX);

    LTRotation rOldRot;
    rOldRot = m_rRotation;

	m_rRotation = LTRotation(VEC_EXPAND(vAngles));

	// Make sure rotation is valid...

    if (m_rRotation.Up().y < 0) m_rRotation = rOldRot;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::SetCameraState()
//
//	PURPOSE:	Set the state/orientation of the camera
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::SetCameraState(CameraState eOrientation)
{
	switch(eOrientation)
	{
		case SOUTHEAST:
			m_OptX = m_CameraDistDiag;
			m_OptY = m_CameraDistUp;
			m_OptZ = -m_CameraDistDiag;
		break;

		case EAST:
			m_OptX = m_CameraDistBack;
			m_OptY = m_CameraDistUp;
			m_OptZ = 0;
		break;

		case NORTHEAST:
			m_OptX = m_CameraDistDiag;
			m_OptY = m_CameraDistUp;
			m_OptZ = m_CameraDistDiag;
		break;

		case NORTH:
			m_OptX = 0;
			m_OptY = m_CameraDistUp;
			m_OptZ = m_CameraDistBack;
		break;

		case NORTHWEST:
			m_OptX = -m_CameraDistDiag;
			m_OptY = m_CameraDistUp;
			m_OptZ = m_CameraDistDiag;
		break;

		case WEST:
			m_OptX = -m_CameraDistBack;
			m_OptY = m_CameraDistUp;
			m_OptZ = 0;
		break;

		case SOUTHWEST:
			m_OptX = -m_CameraDistDiag;
			m_OptY = m_CameraDistUp;
			m_OptZ = -m_CameraDistDiag;
		break;

		case SOUTH:
			m_OptX = 0;
			m_OptY = m_CameraDistUp;
			m_OptZ = -m_CameraDistBack;
		break;

		case MLOOK:
			m_OptX = DEFAULT_CAMERA_DIST_MLOOK_X;
			m_OptY = DEFAULT_CAMERA_DIST_MLOOK_Y;
			m_OptZ = DEFAULT_CAMERA_DIST_MLOOK_Z;
		break;

		default:
		break;
	}

	m_eOrientation = eOrientation;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::RotateCameraState()
//
//	PURPOSE:	Rotate the camera clockwise or counterclockwise around
//				the target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::RotateCameraState(LTBOOL bClockwise)
{
	switch(m_eOrientation)
	{
		case SOUTHEAST:
			if(bClockwise)
				SetCameraState(SOUTH);
			else
				SetCameraState(EAST);
		break;

		case EAST:
			if(bClockwise)
				SetCameraState(SOUTHEAST);
			else
				SetCameraState(NORTHEAST);
		break;

		case NORTHEAST:
			if(bClockwise)
				SetCameraState(EAST);
			else
				SetCameraState(NORTH);
		break;

		case NORTH:
			if(bClockwise)
				SetCameraState(NORTHEAST);
			else
				SetCameraState(NORTHWEST);
		break;

		case NORTHWEST:
			if(bClockwise)
				SetCameraState(NORTH);
			else
				SetCameraState(WEST);
		break;

		case WEST:
			if(bClockwise)
				SetCameraState(NORTHWEST);
			else
				SetCameraState(SOUTHWEST);
		break;

		case SOUTHWEST:
			if(bClockwise)
				SetCameraState(WEST);
			else
				SetCameraState(SOUTH);
		break;

		case SOUTH:
			if(bClockwise)
				SetCameraState(SOUTHWEST);
			else
				SetCameraState(SOUTHEAST);
		break;

		default:
		break;

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::MoveCameraToPosition()
//
//	PURPOSE:	Move the camera to a position over a time period
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::MoveCameraToPosition(LTVector pos, float deltaTime)
{
	if (!m_pClientDE || !m_hTarget) return;

	if (m_eCameraMode == GOINGFIRSTPERSON)
	{
		m_eCameraMode = FIRSTPERSON;

        LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

        LTVector vF = rRot.Forward();

        LTVector vPos;
		g_pLTClient->GetObjectPos(m_hTarget, &vPos);

		vF.y = 0.0f;
		vPos += (vF * m_TargetFirstPersonOffset.x);
		vPos.y += m_TargetFirstPersonOffset.y;

		m_vPos = vPos;

		PointInDirection(rRot);
		return;
	}

    LTVector dir;
	dir = (pos - m_vPos);

    float multiplier = 1.0f; // 0.5f;

    LTVector toMove;
	toMove = (dir * multiplier);

    float moveMag;

	if (m_bSlide)
	{
		moveMag = toMove.Length();

		if (moveMag > dir.Length())
		{
			moveMag = dir.Length();
		}

		if (toMove.x != 0.0f || toMove.y != 0.0f || toMove.z != 0.0f)
		{
			toMove.Normalize();
		}

		m_vPos += (toMove * moveMag);
	}
	else
	{
		m_vPos = pos;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::FindOptimalCameraPosition()
//
//	PURPOSE:	Find the optimal camera position
//
// ----------------------------------------------------------------------- //

LTVector CPlayerCamera::FindOptimalCameraPosition()
{
    LTVector pos;
	VEC_INIT(pos);

	if (!m_pClientDE || !m_hTarget) return pos;

    LTVector     dir;
    float      distToOptimal;
    LTVector     TargetPlusOffset;

    LTVector vTargetPos;
	g_pLTClient->GetObjectPos(m_hTarget, &vTargetPos);

    LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

	if (vTargetPos.NearlyEquals(m_vLastTargetPos, 1.0f) && (rRot == m_rLastTargetRot))
	{
		return m_vLastOptPos;
	}
	else
	{
		m_vLastTargetPos = vTargetPos;
        m_rLastTargetRot = rRot;
	}

    LTVector vTemp;
	vTemp = vTargetPos + m_TargetChaseOffset;
	TargetPlusOffset = vTemp;

	//	pos = TargetPlusOffset + right*m_OptX + up*m_OptY + forward*m_OptZ;

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	pos = TargetPlusOffset + 
		(rRot.Right() * m_OptX) + 
		(rRot.Up() * m_OptY) + 
		(rRot.Forward() * m_OptZ);

	vTemp = TargetPlusOffset - pos;
	distToOptimal = vTemp.Length();

	dir = pos - TargetPlusOffset;
	dir.Normalize();

	iQuery.m_From = TargetPlusOffset;
	iQuery.m_To = pos;

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - TargetPlusOffset;

		// If there was something in the way, move in front of that thing.
		if (vTemp.Length() < distToOptimal)
		{
			pos = iInfo.m_Point + iInfo.m_Plane.m_Normal;
		}
	}


	CalcNonClipPos(pos, rRot);

	m_vLastOptPos = pos;

	return pos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::PrintVector()
//
//	PURPOSE:	Print it!
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::PrintVector(LTVector v)
{
	if (!m_pClientDE) return;

	char buf[50];
	sprintf(buf, "x = %f, y = %f, z = %f", v.x, v.y, v.z);
	m_pClientDE->CPrint(buf);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::StartCircle()
//
//	PURPOSE:	Start the circle...jerk!
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::StartCircle(float HeightOffset, float Radius, float PointAtOffset, float Time)
{
	if (!m_pClientDE || !m_hTarget) return;

	if(m_eCameraMode != CIRCLING)
	{
		m_CircleHeightOffset = HeightOffset;
		m_CircleRadius		 = Radius;
		m_CircleHeightOffset = PointAtOffset;
		m_CircleTime		 = Time;
        m_bStartCircle       = LTFALSE;

        LTVector vTargetPos, up, right, forward;
        LTRotation rRot;

		g_pLTClient->GetObjectRotation(m_hTarget, &rRot);
		g_pLTClient->GetObjectPos(m_hTarget, &vTargetPos);

		SaveState();

		SaveCameraMode();
		//GetClientShell().CPrint("Going circle");
		m_eCameraMode = CIRCLING;

		m_CircleStartTime = m_pClientDE->GetTime();


		m_vPos = vTargetPos + 
			((rRot.Up() * m_CircleHeightOffset) - 
			 (rRot.Forward() * m_CircleRadius));

		// PointAtPosition(m_Target.m_Pos+CreateVector(0,m_CircleHeightOffset,0), m_Target.m_Angles, m_Pos);
		LTVector vTemp = vTargetPos + LTVector(0.0f, m_CircleHeightOffset, 0.0f);

		PointAtPosition(vTemp, rRot, m_vPos);

		// m_SaveAnglesY = m_Angles.y;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CircleAroundTarget()
//
//	PURPOSE:	Circle the target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CircleAroundTarget()
{
	if (!m_pClientDE || !m_hTarget) return;

    float diff = (m_pClientDE->GetTime() - m_CircleStartTime) / m_CircleTime;

	if(diff >= 1.0f)
	{
		RestoreState();
		RestoreCameraMode();
		//GetClientShell().CPrint("Done with circle");
		return;
	}

	//m_Angles.y = m_SaveAnglesY + (MATH_CIRCLE*diff);
	m_rRotation.Rotate(m_rRotation.Up(), MATH_CIRCLE*diff);

    LTVector vTargetPos;
    LTRotation rRot;
	g_pLTClient->GetObjectPos(m_hTarget, &vTargetPos);
	g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

	m_vPos = vTargetPos + 
		(rRot.Up() * m_CircleHeightOffset) - 
		(rRot.Forward() * m_CircleRadius);

	LTVector vTemp = vTargetPos + LTVector(0.0f, m_CircleHeightOffset, 0.0f);

	PointAtPosition(vTemp, rRot, m_vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::VCompare()
//
//	PURPOSE:	Compare two vectors
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerCamera::VCompare(LTVector a, LTVector b)
{
	if((fabs(a.x - b.x) > 5.0f) || (fabs(a.y - b.y) > 5.0f) || (fabs(a.z - b.z) > 5.0f))
        return LTFALSE;
	else
        return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::AttachToObject()
//
//	PURPOSE:	Attach camera to an object
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::AttachToObject(HLOCALOBJ hObj)
{
	m_hTarget = hObj;

	// Initialize our position to that of the object...

	if (m_hTarget && m_pClientDE)
	{
		g_pLTClient->GetObjectPos(m_hTarget, &m_vPos);

		// Assume for now first person...

        LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

        LTVector vF = rRot.Forward();
		vF.y = 0.0f;

		m_vPos += (vF * m_TargetFirstPersonOffset.x);
		m_vPos.y += m_TargetFirstPersonOffset.y;
	}
}

void CPlayerCamera::GoFirstPerson(bool bInstant)
{
	// RETAIL: Tron's (retail 0x1015cfe0): off the light cycle camera
	if (m_eCameraMode == TRONMODE5)
	{
		TronUnknownSaveToProfile();
		m_eCameraMode = GOINGFIRSTPERSON;

		if (bInstant)
		{
			m_fTronUnknown104 = 0.0f;
		}
		else
		{
			m_fTronUnknown104 = g_pLTClient->GetTime();
		}
		return;
	}

	if (bInstant)
	{
		m_fTronUnknown104 = 0.0f;
	}

	if((m_eCameraMode != GOINGFIRSTPERSON) && (m_eCameraMode != FIRSTPERSON))
	{
        m_GoingFirstPersonStart = g_pLTClient->GetTime();
		m_eCameraMode = GOINGFIRSTPERSON;
        CameraUpdate(g_pGameClientShell->GetFrameTime());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CreateCameraCollisionObject()
//
//	PURPOSE:	Create an object to use for collision detection when calculating a no-clip pos...
//
// ----------------------------------------------------------------------- //

bool CPlayerCamera::CreateCameraCollisionObject()
{
	if( !g_pLTClient )
		return false;

	ObjectCreateStruct ocs;

	SAFE_STRCPY( ocs.m_Filename, CAMERA_COLLISION_MODEL );
	ocs.m_ObjectType	= OT_MODEL;
	// RETAIL: Tron's steps up stairs (0x1015d137)
	ocs.m_Flags			= FLAG_STAIRSTEP;
	ocs.m_Flags2		= FLAG2_PLAYERCOLLIDE;
	ocs.m_Pos.Init();
	
	float fScale = g_vtCameraCollisionObjScale.GetFloat();
	ocs.m_Scale.Init( fScale, fScale, fScale );

	m_hCollisionObject = g_pLTClient->CreateObject( &ocs );
	if( !m_hCollisionObject )
		return false;

	g_pPhysicsLT->SetObjectDims( m_hCollisionObject, &LTVector( fScale, fScale, fScale ), 0 );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CalcNonClipPos()
//
//	PURPOSE:	Get a position that won't clip into the world or other objects...
//				Doesn't actullay move the camera to that position!
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CalcNonClipPos( LTVector & vPos, LTRotation & rRot, int nTronUnknown )
{
	// RETAIL: Tron's (0x1015d240) needs a target, keeps the light cycle and
	// the client object out of the way and sizes the collision object for
	// the cycle; it can also clip the result again without the object
	if( !m_hTarget )
		return;

	if( !m_hCollisionObject || (g_vtCameraCollisionUseObject.GetFloat() < 1.0f) )
	{
		CalcNoClipPos_WithoutObject( vPos, rRot, nTronUnknown );
		return;
	}

	HOBJECT hCycle = LTNULL;
	CSFXMgr* pSFXMgr = g_pGameClientShell->GetSFXMgr();
	if( pSFXMgr )
	{
		CLightCycleFX* pCycleFX = pSFXMgr->GetLightCycleFX( g_pLTClient->GetClientObject() );
		if( pCycleFX )
		{
			hCycle = pCycleFX->GetObject();
		}
	}

	bool bOnCycle = (hCycle != LTNULL);

	LTVector vCamPos;
	float fScale;
	if( bOnCycle )
	{
		g_pLTClient->GetObjectPos( m_hTarget, &vCamPos );
		fScale = g_vtCameraCollisionLightCycleObjScaleY.GetFloat();
	}
	else
	{
		g_pLTClient->GetObjectPos( g_pPlayerMgr->GetCamera(), &vCamPos );
		fScale = g_vtCameraCollisionObjScale.GetFloat();
	}

	g_pLTClient->SetObjectPos( m_hCollisionObject, &vCamPos, LTTRUE );
	g_pLTClient->SetObjectRotation( m_hCollisionObject, &rRot );

	LTVector vScale( fScale, fScale, fScale );
	g_pPhysicsLT->SetObjectDims( m_hCollisionObject, &vScale, 0 );

	HOBJECT hClient = g_pLTClient->GetClientObject();
	HOBJECT hMoveObj = g_pMoveMgr->GetObject();

	uint32 dwMoveObjFlags, dwCycleFlags, dwClientFlags;
	g_pCommonLT->GetObjectFlags( hMoveObj, OFT_Flags, dwMoveObjFlags );
	g_pCommonLT->GetObjectFlags( hCycle, OFT_Flags, dwCycleFlags );
	g_pCommonLT->GetObjectFlags( hClient, OFT_Flags, dwClientFlags );

	// Set objects the camera is inside of to be non-solid...

	g_pCommonLT->SetObjectFlags( hMoveObj, OFT_Flags, 0, FLAG_SOLID );
	g_pCommonLT->SetObjectFlags( hCycle, OFT_Flags, 0, FLAG_SOLID );
	g_pCommonLT->SetObjectFlags( hClient, OFT_Flags, 0, FLAG_SOLID );

	// Our collision object needs to be solid while moving...

	g_pCommonLT->SetObjectFlags( m_hCollisionObject, OFT_Flags,	FLAG_SOLID, FLAG_SOLID );

	// Do the collison checking and get the final pos...

	g_pPhysicsLT->MoveObject( m_hCollisionObject, &vPos, 0 );
	g_pLTClient->GetObjectPos( m_hCollisionObject, &vPos );

	// Set the objects to their previous solidity...

	g_pCommonLT->SetObjectFlags( hMoveObj, OFT_Flags, dwMoveObjFlags, FLAG_SOLID );
	g_pCommonLT->SetObjectFlags( hCycle, OFT_Flags, dwCycleFlags, FLAG_SOLID );
	g_pCommonLT->SetObjectFlags( hClient, OFT_Flags, dwClientFlags, FLAG_SOLID );
	g_pCommonLT->SetObjectFlags( m_hCollisionObject, OFT_Flags, 0, FLAG_SOLID );

	if( (g_vtCameraUseFinalClipping.GetFloat() != 0.0f) && bOnCycle )
	{
		CalcNoClipPos_WithoutObject( vPos, rRot, nTronUnknown );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CalcNoClipPos_WithoutObject()
//
//	PURPOSE:	Just use ray casts to test for collision...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CalcNoClipPos_WithoutObject( LTVector & vPos, LTRotation & rRot, int nTronUnknown )
{
	LTVector vTemp, vU, vR, vF;
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	
	uint32 dwFlags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	// Make sure we aren't clipping into walls...

    float fClipDistance = g_vtCameraClipDistance.GetFloat();


	// Check for walls to the right...

	vTemp = (vR * fClipDistance);
	vTemp += vPos;

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To   = vTemp;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        float fDist = (fClipDistance - vTemp.Length());

		vTemp = (vR * -fDist);
		vPos += vTemp;
	}
	else
	{
	 	// If we didn't adjust for a wall to the right, check walls to the left...

		vTemp = (vR * -fClipDistance);
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            float fDist = (fClipDistance - vTemp.Length());

			vTemp = (vR * fDist);
			vPos += vTemp;
		}
	}

	// Check for ceilings...

	vTemp = vU * fClipDistance;
	vTemp += vPos;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To = vTemp;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        float fDist = (fClipDistance - vTemp.Length());

		vTemp = vU * -fDist;
		vPos += vTemp;
	}
	else
	{
 		// If we didn't hit any ceilings, check for floors...

		vTemp = vU * -fClipDistance;
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            float fDist = (fClipDistance - vTemp.Length());

			vTemp = vU * fDist;
			vPos += vTemp;
		}
	}

	// Check infront of us...

	vTemp = vF * fClipDistance;
	vTemp += vPos;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To = vTemp;

	if( g_pLTClient->IntersectSegment( &iQuery, &iInfo ))
	{
		vTemp = iInfo.m_Point - vPos;
		float fDist = (fClipDistance - vTemp.Length());

		vTemp = vF * -fDist;
		vPos += vTemp;
	}
	else
	{
		vTemp = vF * -fClipDistance;
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if( g_pLTClient->IntersectSegment( &iQuery, &iInfo ))
		{
			vTemp = iInfo.m_Point - vPos;
			float fDist = (fClipDistance - vTemp.Length());

			vTemp = vU * -fDist;
			vPos += vTemp;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownStartOrbit
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015dfe0; the names are ours): an
//				orbit around the target from one radius and angles to
//				others in fTime seconds
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::TronUnknownStartOrbit(float fStartRadius, float fEndRadius,
										  float fStartPhi, float fEndPhi,
										  float fStartTheta, float fEndTheta, float fTime)
{
	m_fTronUnknownStartRadius	= fStartRadius;
	m_fTronUnknownEndRadius		= fEndRadius;
	m_fTronUnknownStartPhi		= fStartPhi;
	m_fTronUnknownEndPhi		= fEndPhi;
	m_fTronUnknownStartTheta	= fStartTheta;
	m_fTronUnknownEndTheta		= fEndTheta;
	m_fTronUnknownOrbitStart	= g_pLTClient->GetTime();
	m_fTronUnknownOrbitTime		= fTime;
	m_eCameraMode				= TRONMODE6;
	m_fTronUnknownOrbitYaw		= TronUnknownGetOrbitYaw();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownGetOrbitYaw
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e060; the name is ours)
//
// ----------------------------------------------------------------------- //

float CPlayerCamera::TronUnknownGetOrbitYaw()
{
	return (MATH_PI * 3.0f) - g_pPlayerMgr->GetYaw();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownMoveToTarget
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e070; the name is ours): move
//				the camera to +0x24 in the time left ("LCCSpeed" seconds
//				from +0x104); true when it is there
//
// ----------------------------------------------------------------------- //

bool CPlayerCamera::TronUnknownMoveToTarget()
{
	if (m_fTronUnknown104 > 0.0f)
	{
		LTVector vDir = m_vTronUnknown24 - m_vPos;
		float fTimeLeft = 1.0f;
		float fDist = vDir.Mag();

		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCCSpeed");
		if (hVar)
		{
			fTimeLeft = g_pLTClient->GetVarValueFloat(hVar);
		}

		vDir.Norm();

		fTimeLeft -= (g_pLTClient->GetTime() - m_fTronUnknown104);

		if (fTimeLeft > 0.0f && g_pGameClientShell->GetFrameTime() < fTimeLeft)
		{
			vDir = (vDir * g_pGameClientShell->GetFrameTime()) * (fDist / fTimeLeft);
			m_vPos += vDir;
			return false;
		}

		m_vPos = m_vTronUnknown24;
		m_fTronUnknown104 = 0.0f;
		return true;
	}

	m_vPos = m_vTronUnknown24;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownStartPan
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e240; the name is ours): the
//				light cycle camera's pan (CPlayerMgr::SetTronUnknown1D5)
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::TronUnknownStartPan(float fPhi, float fRadius, float fTime)
{
	float fStartRadius;

	if (m_eCameraMode != TRONMODE5)
	{
		bool bPan = g_pPlayerMgr->GetTronUnknown1D5();
		g_pPlayerMgr->SetTronUnknown1D5(false);
		g_pInterfaceMgr->TronUnknownThirdPersonCamera(false);
		g_pPlayerMgr->SetTronUnknown1D5(bPan);
		fStartRadius = 100.0f;
	}
	else
	{
		fStartRadius = m_fTronUnknown100;
	}

	float fTheta = m_fTronUnknownFC;
	TronUnknownStartOrbit(fStartRadius, fRadius, g_pPlayerMgr->GetPitch() + m_fTronUnknownF8,
		fPhi, fTheta, fTheta, fTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownStartLCCOrbit
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e2e0; the name is ours): the
//				orbit the "LCC*" console variables set (in degrees)
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::TronUnknownStartLCCOrbit()
{
	float fStartRadius	= 100.0f;
	float fEndRadius	= 500.0f;
	float fStartPhi		= 75.0f;
	float fEndPhi		= 80.0f;
	float fStartTheta	= 360.0f;
	float fEndTheta		= 90.0f;
	float fTime			= 4.0f;

	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCCStartRadius");
	if (hVar)
	{
		fStartRadius = g_pLTClient->GetVarValueFloat(hVar);
	}

	hVar = g_pLTClient->GetConsoleVar("LCCEndRadius");
	if (hVar)
	{
		fEndRadius = g_pLTClient->GetVarValueFloat(hVar);
	}

	hVar = g_pLTClient->GetConsoleVar("LCCStartPhi");
	if (hVar)
	{
		fStartPhi = g_pLTClient->GetVarValueFloat(hVar);
	}

	hVar = g_pLTClient->GetConsoleVar("LCCEndPhi");
	if (hVar)
	{
		fEndPhi = g_pLTClient->GetVarValueFloat(hVar);
	}

	hVar = g_pLTClient->GetConsoleVar("LCCStartTheta");
	if (hVar)
	{
		fStartTheta = g_pLTClient->GetVarValueFloat(hVar);
	}

	hVar = g_pLTClient->GetConsoleVar("LCCEndTheta");
	if (hVar)
	{
		fEndTheta = g_pLTClient->GetVarValueFloat(hVar);
	}

	hVar = g_pLTClient->GetConsoleVar("LCCTime");
	if (hVar)
	{
		fTime = g_pLTClient->GetVarValueFloat(hVar);
	}

	TronUnknownStartOrbit(fStartRadius, fEndRadius, DEG2RAD(fStartPhi), DEG2RAD(fEndPhi),
		DEG2RAD(fStartTheta), DEG2RAD(fEndTheta), fTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownGoLightCycle
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e4b0; the name is ours): the
//				light cycle camera (TRONMODE5) at a pitch, yaw (radians) and
//				zoom
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::TronUnknownGoLightCycle(float fPitch, float fYaw, float fZoom, bool bInstant)
{
	if (m_eCameraMode != TRONMODE5)
	{
		m_eCameraMode = TRONMODE5;
		m_fTronUnknownF8 = fPitch;
		m_fTronUnknownFC = fYaw;
		m_fTronUnknown100 = fZoom;

		if (bInstant)
		{
			m_fTronUnknown104 = 0.0f;
		}
		else
		{
			m_fTronUnknown104 = g_pLTClient->GetTime();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownGoLightCycleFromProfile
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e510; the name is ours): the
//				light cycle camera as the profile keeps it (in degrees)
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::TronUnknownGoLightCycleFromProfile(bool bInstant)
{
	if (m_eCameraMode == TRONMODE5)
		return;

	float fZoom = 800.0f;
	float fPitch = 45.0f;
	float fYaw = 94.0f;

	if (g_pProfileMgr)
	{
		CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();

		float fTotalPitch = pProfile->m_fLCCamPitch + RAD2DEG(pProfile->m_fLCPlayerPitch);
		if (fTotalPitch < 0.0f || fTotalPitch > 90.0f)
		{
			pProfile->m_fLCCamPitch = 45.0f;
			pProfile->m_fLCPlayerPitch = 0.0f;
		}

		if (pProfile->m_fLCCamYaw < 0.0f || pProfile->m_fLCCamYaw > 360.0f)
		{
			pProfile->m_fLCCamYaw = 94.0f;
			pProfile->m_fLCPlayerPitch = 0.0f;
		}

		fPitch = pProfile->m_fLCCamPitch;
		fYaw = pProfile->m_fLCCamYaw;
		fZoom = pProfile->m_fLCCamZoom;
		g_pPlayerMgr->SetPitch(pProfile->m_fLCPlayerPitch);
	}

	TronUnknownGoLightCycle(DEG2RAD(fPitch), DEG2RAD(fYaw), fZoom, bInstant);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronUnknownCameraFilterFn
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e8a0; the name is ours):
//				solid objects other than the player and the watched object
//
// ----------------------------------------------------------------------- //

static bool TronUnknownCameraFilterFn(HOBJECT hObj, void* pUserData)
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);

	if ((dwFlags & FLAG_SOLID) && hObj != g_pMoveMgr->GetObject() &&
		hObj != g_pMoveMgr->GetServerObject() && hObj != g_pPlayerMgr->GetTronUnknownBC())
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownClipToTarget
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e620; the name is ours): keep
//				the camera on the near side of what is between it and the
//				target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::TronUnknownClipToTarget()
{
	if (!m_hTarget)
		return;

	LTVector vTargetPos;
	g_pLTClient->GetObjectPos(m_hTarget, &vTargetPos);

	LTVector & vCamPos = m_vPos;
	LTVector vDir = vCamPos - vTargetPos;
	LTVector vPos = vCamPos;

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	iQuery.m_From		= vTargetPos;
	iQuery.m_To			= vCamPos;
	iQuery.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iQuery.m_FilterFn	= TronUnknownCameraFilterFn;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		LTVector vTemp = iInfo.m_Point - vTargetPos;
		if (vTemp.Mag() < vDir.Mag())
		{
			vDir = iInfo.m_Point + iInfo.m_Plane.m_Normal;
			vPos = vDir;
		}
	}

	VEC_COPY(vCamPos, vPos);
	m_vLastOptPos = vPos;
	m_vLastTargetPos = vTargetPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::TronUnknownSaveToProfile
//
//	PURPOSE:	RETAIL: Tron's (retail 0x1015e8f0; the name is ours): keep
//				the light cycle camera in the profile (in degrees)
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::TronUnknownSaveToProfile()
{
	if (!g_pProfileMgr)
		return;

	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_fLCCamYaw = RAD2DEG(m_fTronUnknownFC);
	pProfile->m_fLCCamPitch = RAD2DEG(m_fTronUnknownF8);
	pProfile->m_fLCCamZoom = m_fTronUnknown100;
	pProfile->m_fLCPlayerPitch = g_pPlayerMgr->GetPitch();
	pProfile->Save();
}
