// ----------------------------------------------------------------------- //
//
// MODULE  : TargetMgr.cpp
//
// PURPOSE : Implementation of class to handle tracking whjat the player is aimed at.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TargetMgr.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "SurfaceFunctions.h"
#include "GameClientShell.h"
#include "ClientServerShared.h"
#include "ClientButeMgr.h"
#include "BodyFX.h"
#include "CharacterFX.h"
#include "PickupItemFX.h"
#include "GadgetTargetFX.h"
#include "VarTrack.h"
#include "ClientResShared.h"
#include "ActivateObjectFX.h"
#include "Searcher.h"
#include "VehicleMgr.h"
#include "DoomsdayPieceFX.h"

VarTrack	g_vtActivationDistance;
VarTrack	g_vtTargetDistance;

extern VarTrack g_vtCamZoom1MaxDist;

static bool DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData);
static bool ActivateFilterFn(HOBJECT hTest, void *pUserData);

CActivationData	g_adFallbackActivationObject;

static inline bool IsUserFlagSet(HOBJECT hObj, uint32 dwTestFlags)
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwFlags);
	return !!(dwFlags & dwTestFlags);
}

CTargetMgr::CTargetMgr()
{
	m_hTarget = NULL;
	m_hLockedTarget = NULL;
	m_fTargetRange = 10000.0f;
	// RETAIL: 0x1016d5f0 sets no target team
	m_szString[0] = 0;
	m_szDebugString[0] = 0;
	m_nString = 0;

	m_bSearchTarget = false;
	m_bMoveTarget = false;
	m_bCanActivate = false;
}


CTargetMgr::~CTargetMgr()
{
}

// RETAIL: 0x1016d700. Tron's is build 51's with NOLF2's later parts: the
// hit box of a character, reviving a body, waking a character, the
// multiplayer player names and the activate objects' strings. It has no
// vehicle or dead-player test, no progress bar, no model name IDs, no gadget
// bombs and no doomsday pieces, and the distances are members (see
// IsTargetInRange).
void CTargetMgr::Update()
{
	if (m_hLockedTarget && m_hTarget == m_hLockedTarget)
	{
		// RETAIL: no gadget disabling in Tron (no CGadgetDisabler)

		//are we searching something?
		if (g_pPlayerMgr->IsSearching())
		{
			m_bSearchTarget = true;
			SetTargetStringID(kTronIDS_TargetSearching);
			
			float fDistAway = 10000.0f;
			CheckForIntersect(fDistAway);
			
			return;
		}
	}

	// If we currently have a target, see if it is a body and if so remove the
	// glow flag (it may be set again below)...
	if (m_hTarget)
	{
		CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFX(m_hTarget);
		if (pBody) 
		{
			g_pCommonLT->SetObjectFlags(m_hTarget, OFT_User, 0, USRFLG_GLOW);
		}
	}


	// RETAIL: Tron has no ClearTargetInfo or FirstUpdate: the target info is
	// cleared here, and the console variables are initialised each time
	// (0x1016d792; written as separate inline functions, the console
	// variables' Init calls stay calls). The distances (0x1016d91d,
	// 0x1016d944) are kept for the inline IsTargetInRange; their defaults
	// are constants (NOLF2 read them from the interface attributes)

	//clear our targeting info
	SetTargetStringID(NULL);
	m_hTarget = NULL;
	m_fTargetRange = 10000.0f;
	m_szString[0] = 0;
	m_szDebugString[0] = 0;
	m_nString = 0;

	if (!g_vtActivationDistance.IsInitted())
	{
		g_vtActivationDistance.Init(g_pLTClient, "ActivationDistance", LTNULL, 100.0f);
	}

	if( !g_vtTargetDistance.IsInitted() )
	{
		g_vtTargetDistance.Init(g_pLTClient, "TargetDistance", LTNULL, 1000.0f);
	}
	m_fActivationDistance = g_vtActivationDistance.GetFloat();
	m_fTargetDistance = g_vtTargetDistance.GetFloat();

	m_ActivationData.Init();

	m_bSearchTarget = false;
	m_bMoveTarget = false;
	m_bCanActivate = true;


	//see what we've looking at
	float fDistAway = 10000.0f;
	CheckForIntersect(fDistAway);
	if (!m_hTarget) 
	{
		//nothing to see here
		return;
	}

	m_fTargetRange = fDistAway;

	//if its a body's hitbox, check the body instead
	CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox(m_hTarget);
	if (pBody)
	{
		m_hTarget = pBody->GetServerObj();
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}

	//if its a Character's hitbox and it is searchable, check the Character instead
	CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox(m_hTarget);
	if (pCharacter)
	{
		m_hTarget = pCharacter->GetServerObj();
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}



	uint32 dwUserFlags = 0;
    g_pCommonLT->GetObjectFlags(m_hTarget, OFT_User, dwUserFlags);

	//special case handling for bodies
	if (pBody || pCharacter)
	{
		// RETAIL: build 51's look at the body is still here (0x1016da85), but
		// nothing uses its result
		LTRotation	rCamRot;
		LTVector	vTargetPos, vCamPos, vDir, vCamR;

		HLOCALOBJ	hCamera = g_pPlayerMgr->GetCamera();

		g_pLTClient->GetObjectRotation( hCamera, &rCamRot );
		vCamR = rCamRot.Right();

		g_pLTClient->GetObjectPos( hCamera, &vCamPos );
		g_pLTClient->GetObjectPos( m_hTarget, &vTargetPos );
		

		vDir = vTargetPos - vCamPos;

		// See if we are looking at the target...

		vDir.Normalize();
		LTFLOAT	fCamAngle = vDir.Dot( vCamR );

		bool bCanSearch = !!(dwUserFlags & USRFLG_CAN_SEARCH);
		if (pBody)
		{
			if (fDistAway <= m_fActivationDistance)
			{
				// Make target glow, so it stands out more...
				g_pCommonLT->SetObjectFlags(m_hTarget, OFT_User, USRFLG_GLOW, USRFLG_GLOW);
			}

			// RETAIL: Tron revives any body that can be revived, in the
			// activation distance (no "ReviveDistance", game type or team)
			if (pBody->CanBeRevived() && fDistAway <= m_fActivationDistance)
			{
				SetTargetStringID(kTronIDS_TargetRevive);

				LTVector vObjPos, vDims;
				g_pLTClient->GetObjectPos(pBody->GetServerObj(), &vObjPos);
				g_pPhysicsLT->GetObjectDims(pBody->GetServerObj(), &vDims);

				// Players are non-solid to each other so you can revive right on top of them.
				m_bCanActivate = true;  //!CheckForCharacters(vObjPos, vDims, pBody->GetClientId());
				m_bMoveTarget = true;
				m_ActivationData.m_nType = MID_ACTIVATE_REVIVE;
				return;
			}

			m_bMoveTarget = (pBody->CanBeCarried() && g_pPlayerMgr->CanDropCarriedObject());
		}
		else if (pCharacter)
		{
			// RETAIL: no slipped characters in Tron (0x1016dc07)
			if( (pCharacter->m_cs.eCrosshairCharacterClass != BAD) && (pCharacter->CanWake()) && pCharacter->IsUnconscious() ) 
			{
				SetTargetStringID( kTronIDS_TargetWakeUp );
				
				m_bCanActivate	= true;
				m_bMoveTarget	= true;

				m_ActivationData.m_nType = MID_ACTIVATE_WAKEUP;
				return;
			}

			m_bMoveTarget = false;
		}
		else
		{
			m_bMoveTarget = false;
		}

		if (bCanSearch && fDistAway <= m_fActivationDistance)
		{
			// we can search this body
			m_bSearchTarget = true;
			m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
			SetTargetStringID(kTronIDS_TargetSearch);
			return;
		}
		else if (pBody)
		{
			return;
		}
	}

	// is this a gadget target
	if (IsGadgetActivatable(m_hTarget) && (fDistAway <= m_fActivationDistance))
	{
		// looks like we can use a gadget on it...
		SetGadgetTarget( false );
		return;
	}

	//are we aiming at a person?
	if (dwUserFlags & USRFLG_CHARACTER)
	{
		CCharacterFX* const pFX = (CCharacterFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_CHARACTER_ID, m_hTarget);

		//display debug info if we have any 
		if( pFX && pFX->GetInfoString() && *pFX->GetInfoString() )
		{
			SAFE_STRCPY(m_szDebugString,pFX->GetInfoString());			
		}
		else
		{
			m_szDebugString[0] = NULL;
		}

		// is this a person we can talk to?
		if (dwUserFlags & USRFLG_CAN_ACTIVATE)
		{
			if (fDistAway <= m_fActivationDistance)
			{
				SetTargetStringID(kTronIDS_TargetTalk);
				return;
			}
		}

		if (IsMultiplayerGame() && pFX && pFX->m_cs.bIsPlayer )
		{
			uint32 nId = pFX->m_cs.nClientID;
			CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
			CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);

			if (pCI)
			{
				m_nString = 0;
				SAFE_STRCPY(m_szString,pCI->sName.c_str());
				// RETAIL: no target team in Tron
			}
			return;
		}
	
		// RETAIL: no model name IDs and no "innocent" string here in Tron
	}


	//is this a searchable object?
	if (dwUserFlags & USRFLG_CAN_SEARCH && (fDistAway <= m_fActivationDistance))
	{
		m_bSearchTarget = true;
		m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
		SetTargetStringID(kTronIDS_TargetSearch);
		return;

	}
	
	// See if this object is part of the activate object list with it's own string ID's...

	if( fDistAway <= m_fActivationDistance )
	{
		CActivateObjectHandler *pActivateObj = LTNULL;
		CActivateObjectHandler::ActivateObjList::const_iterator iter = CActivateObjectHandler::GetActivateObjectList().begin();
		while( iter != CActivateObjectHandler::GetActivateObjectList().end() )
		{
			pActivateObj = *iter;
			
			if( pActivateObj->GetHOBJECT() == m_hTarget )
			{
				ACTIVATETYPE *pType = g_pActivateTypeMgr->GetActivateType( pActivateObj->m_nId );
				if( pType )
				{
					// Set whether or not it's disabled and set the string based on the state...

					m_bCanActivate = !pActivateObj->m_bDisabled;
					uint32 dwStringID = pType->dwStateID[pActivateObj->m_eState];
					
					if( dwStringID != (uint32)-1 )
					{
						SetTargetStringID( dwStringID );
					}

					return;
				}
			}

			++iter;
		}
	}

	//can we pick up or activate it?
	if (dwUserFlags & USRFLG_CAN_ACTIVATE && (fDistAway <= m_fActivationDistance))
	{
		// RETAIL: no gadget targets to defuse in Tron, and Tron's pickup
		// items have no team (0x1016de54; PickupItemFX.h)
		CPickupItemFX* const pFX = (CPickupItemFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_PICKUPITEM_ID, m_hTarget);

		// If we're looking at a pickup, use the take string, otherwise it's just something to interact with.
		SetTargetStringID(pFX ? kTronIDS_TargetTake : kTronIDS_TargetUse);
		return;
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CheckForIntersect()
//
//	PURPOSE:	See if there is an activation object directly in front of
//				the camera.
//
// ----------------------------------------------------------------------- //

void CTargetMgr::CheckForIntersect(float &fDistAway)
{
	m_hTarget = LTNULL;
	m_ActivationData.Init();

	uint32 dwUsrFlags = 0;
	const float fMaxDist = 100000.0f;

	// Cast ray from the camera to see if there is an object to activate...

    LTRotation rRot;
    LTVector vPos;

	HLOCALOBJ hCamera = g_pPlayerMgr->GetCamera();
	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);

 	m_ActivationData.m_vPos = vPos;
	m_ActivationData.m_vDir = rRot.Forward();	// PORT: Tron's CActivationData has a direction

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To   = IQuery.m_From + (rRot.Forward() * fMaxDist);

	// NOTE the use of the CHECK_FROM_POINT_INSIDE_OBJECTS flag.  This flag will
	// make sure that any objects that m_From is inside are considered
	IQuery.m_Flags = CHECK_FROM_POINT_INSIDE_OBJECTS | INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;

	// RETAIL: build 51's (0x1016e19a): the filter is the object filter,
	// not NOLF2's filter of the actual intersections, and it has no user data
	IQuery.m_FilterFn	  = ActivateFilterFn;
	IQuery.m_pUserData	  = LTNULL;
	IQuery.m_PolyFilterFn = DoVectorPolyFilterFn;

	// [KLS 8/3/02] - ActivateFilterFn may save an object to use that may not be
	// the best activation choice (i.e., a fallback choice).  However, if a
	// better choice isn't found, the fallback choice should be used.  That
	// fallback choice is stored in g_adFallbackActivationObject so we clear
	// it here...
	g_adFallbackActivationObject.Init();

	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		m_ActivationData.m_vIntersect = IInfo.m_Point;

        if (IsMainWorld(IInfo.m_hObject))
		{
			if (IInfo.m_hPoly != INVALID_HPOLY)
			{
				SurfaceType eType = GetSurfaceType(IInfo.m_hPoly);
				SURFACE *pSurf = g_pSurfaceMgr->GetSurface(eType);

				// See if the surface we tried to activate has an activation
				// sound...If so, the user can activate it...

				if (pSurf && pSurf->szActivationSnd[0] && pSurf->fActivationSndRadius > 0)
				{
					m_hTarget = IInfo.m_hObject;
					m_ActivationData.m_hTarget = m_hTarget;
					m_ActivationData.m_nSurfaceType = eType;
				}
			}
		}
		else
		{
			LTVector vObjPos = m_ActivationData.m_vIntersect;
			vObjPos -= vPos;

			if (vObjPos.Mag() <= fMaxDist)
			{
				m_hTarget = IInfo.m_hObject;
				m_ActivationData.m_hTarget = m_hTarget;
			}
		}

		// Calculate how far away the object is...

		LTVector vDist = m_ActivationData.m_vIntersect - vPos;
		fDistAway = vDist.Mag();
	}
	
	// [KLS 8/3/02] - Use the fallback object if we have one and we didn't 
	// find another object more suitable object... 

	// RETAIL: Tron tests the fallback object first, and a world model is
	// no reason to use it (0x1016e383)
	if ( g_adFallbackActivationObject.m_hTarget &&
		(!m_ActivationData.m_hTarget || IsMainWorld(m_ActivationData.m_hTarget)) )
	{
		// Ok we hit the fallback object reset some of our target data
	
		LTVector vObjPos;
		g_pLTClient->GetObjectPos(g_adFallbackActivationObject.m_hTarget, &vObjPos);

		m_ActivationData.m_vIntersect = vObjPos;

		vObjPos -= vPos;

		if (vObjPos.Mag() <= fMaxDist)
		{
			m_hTarget = g_adFallbackActivationObject.m_hTarget;
			m_ActivationData.m_hTarget = m_hTarget;
		}

		// Calculate how far away the object is...

		LTVector vDist = m_ActivationData.m_vIntersect - vPos;
		fDistAway = vDist.Mag();
	}
}


static bool DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// Make sure we hit a surface type we care about...

	SurfaceType eSurfType = GetSurfaceType(hPoly);

	// RETAIL: 0x1016e470: Tron skips its surfaces 7 and 8 (their names are
	// not known; Tron's attributes number them), not ST_INVISIBLE
	if (eSurfType == (SurfaceType)7 || eSurfType == (SurfaceType)8)
	{
        return false;
	}

    return true;
}

static bool ActivateFilterFn(HOBJECT hTest, void *pUserData)
{
	// RETAIL: Tron's (0x1016e4a0): no doomsday pieces, and no range test for
	// the fallback object (NOLF2's SetFallbackActivationObject): an unsearched
	// body's hit box is the fallback; an unconscious character is not one.

	// [KLS 6/26/02] - Make sure we filter out client related objects first...

	HOBJECT hClientHitBox = LTNULL;
	CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(g_pLTClient->GetClientObject());
	if (pCharacter) 
	{
		hClientHitBox = pCharacter->GetHitBox();
	}

	HOBJECT hFilterList[] = 
	{
		g_pLTClient->GetClientObject(),
		g_pPlayerMgr->GetMoveMgr()->GetObject(),
		hClientHitBox,
		LTNULL
	};
	if (!ObjListFilterFn(hTest, (void*) hFilterList))
	{
		return false;
	}

	// Always keep gadget targets...
	if (g_pPlayerMgr->GetTargetMgr()->IsGadgetActivatable(hTest)) 
	{
		return true;
	}

	// Always keep our locked target...
	if (g_pPlayerMgr->GetTargetMgr()->GetLockedTarget() == hTest) 
	{
		return true;
	}

	// If it's a body, ignore it (we only care about its hit box)
	CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFX(hTest);
	if (pBody) 
	{
		return false;
	}

	// If it's a hitbox associated with a body we care.
	pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox(hTest);
	if (pBody)
	{
		if (IsUserFlagSet(hTest, (USRFLG_CAN_ACTIVATE | USRFLG_CAN_SEARCH)))
		{
			return true;
		}

		// Save body for later since it may overlap a higher priority object
		g_adFallbackActivationObject.m_hTarget = hTest;
		return false;
	}

	// If it's a Character and it has a hitbox, ignore it (we only care about its hit box)
	pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hTest);
	if (pCharacter) 
	{
		if (pCharacter->GetHitBox())
		{
			return false;
		}

		return true; // "Solid" object so we're done
	}

	// If it's a hitbox associated with a Character we care about it...
	pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox(hTest);
	if (pCharacter)
	{
		return true; // "Solid" object so we're done
	}

 	// Ignore non-solid objects that can't be activated...

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hTest, OFT_Flags, dwFlags);

	if (!(dwFlags & FLAG_SOLID))
	{
		// RETAIL: an info string's object is a target too
		g_pCommonLT->GetObjectFlags(hTest, OFT_User, dwFlags);
		return !!(dwFlags & (USRFLG_CAN_ACTIVATE | USRFLG_CAN_SEARCH | USRFLG_INFO_STRING));
	}

	// Hit something solid, so we're done...
    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTargetMgr::IsGadgetActivatable()
//
//	PURPOSE:	Can the passed in object be activated with any gadget.
//
// ----------------------------------------------------------------------- //

bool CTargetMgr::IsGadgetActivatable(HOBJECT hObj)
{
	if (!hObj) return false;

	uint32 dwUserFlags = 0;
    g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUserFlags);

	bool bIsGadgetActivatable = !!(dwUserFlags & USRFLG_GADGET_CAN_DISABLE);

	return bIsGadgetActivatable;
}



void CTargetMgr::SetTargetStringID(uint16 nID)
{
	m_nString = nID;
	if (nID > 0)
		LoadString(nID,m_szString,sizeof(m_szString));
	else
		m_szString[0] = 0;
}


void CTargetMgr::SetGadgetTarget( bool bDisabling )
{
	// RETAIL: Tron's (0x1016e6d0): the target can be activated when the
	// current ammo does the gadget's damage type (NOLF2 looked for a gadget
	// in the inventory), with no team, power, carrying or progress bar
	// checks. Tron's string IDs: the action, and +23 for its progress.

	if (!m_hTarget) return;

	CGadgetTargetFX* const pFX = (CGadgetTargetFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_GADGETTARGET_ID, m_hTarget);
	m_bCanActivate = false;

	if (!pFX) return;

	const AMMO* pAmmo = g_pWeaponMgr->GetAmmo(g_pPlayerStats->GetCurrentAmmo());
	if (!pAmmo) return;

	DamageType eDT = pAmmo->eInstDamageType;

	switch( pFX->GetType() )
	{
	case eKeyPad:
	case eCardReader:
		m_bCanActivate = (eDT == DT_GADGET_CODE_DECIPHERER);
		SetTargetStringID( bDisabling ? 455 + 23 : 455 );
		break;
	case eCodedText:
		m_bCanActivate = (eDT == DT_GADGET_CODE_DECIPHERER);
		SetTargetStringID( bDisabling ? 457 + 23 : 457 );
		break;
	case eComboLock:
		m_bCanActivate = (eDT == DT_GADGET_WELDER);
		SetTargetStringID( bDisabling ? 456 + 23 : 456 );
		break;
	case ePadLock:
	case eDoorKnob:
		m_bCanActivate = (eDT == DT_GADGET_LOCK_PICK);
		SetTargetStringID( bDisabling ? 458 + 23 : 458 );
		break;
	case eTelephone:
		m_bCanActivate = (eDT == DT_GADGET_EAVESDROPBUG);
		SetTargetStringID( bDisabling ? 462 + 23 : 462 );
		break;
	case ePhotographable:
		m_bCanActivate = (eDT == DT_GADGET_CAMERA);
		SetTargetStringID( bDisabling ? 459 + 23 : 459 );
		break;
	case eBombable:
		m_bCanActivate = bDisabling || (eDT == DT_GADGET_TIME_BOMB);
		SetTargetStringID( bDisabling ? 460 + 23 : 460 );
		break;
	case eInvisibleInk:
		m_bCanActivate = (eDT == DT_GADGET_INK_REAGENT);
		SetTargetStringID( bDisabling ? 461 + 23 : 461 );
		break;
	case eINVALID:
	default:
		SetTargetStringID(NULL);
		break;
	}
}


bool CTargetMgr::CheckForCharacters(LTVector vObjPos,LTVector vDims, uint8 nId)
{
	float fLeashLen = GetConsoleFloat("LeashLen",0.0f);
	//give 'em some room
	vDims.x += fLeashLen;
	vDims.y += fLeashLen;
	vDims.z += fLeashLen;
	vDims *= 2.0f;

	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CHARACTER_ID);
	if (!pList) return false;

	int nNumChars = pList->GetSize();

	LTVector vCharPos, vCharDims;
	for (int i=0; i < nNumChars; i++)
	{
		if ((*pList)[i])
		{
			CCharacterFX* pChar = (CCharacterFX*)(*pList)[i];

			if (pChar->m_cs.bIsPlayer && pChar->m_cs.nClientID == nId)
				continue;

			g_pLTClient->GetObjectPos(pChar->GetServerObj(), &vCharPos);

			if (vObjPos.x - vDims.x < vCharPos.x && vCharPos.x < vObjPos.x + vDims.x &&
				vObjPos.y - vDims.y < vCharPos.y && vCharPos.y < vObjPos.y + vDims.y &&
				vObjPos.z - vDims.z < vCharPos.z && vCharPos.z < vObjPos.z + vDims.z)
			{
				return true;
			}

		}
	}

	return false;
}


// RETAIL: Tron's IsTargetInRange is inline (TargetMgr.h)



