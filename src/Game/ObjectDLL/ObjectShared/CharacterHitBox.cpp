// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.cpp
//
// PURPOSE : Character hit box object class implementation
//
// CREATED : 01/05/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterHitBox.h"
#include "Character.h"
#include "Body.h"
#include "Projectile.h"
#include "ServerUtilities.h"
#include "CVarTrack.h"
#include "MsgIds.h"
#include "ObjectMsgs.h"
#include "Attachments.h"
#include "GameServerShell.h"
#include "PowerArmor.h"
#include <list>
#include "DebugLineSystem.h"	// RETAIL: Tron's (its colours, Object.lto 0x10137c60)

LINKFROM_MODULE( CharacterHitBox );

static CVarTrack g_vtShowNodeRadii;
static CVarTrack g_vtShowPlayerNodeRadii;
static CVarTrack g_vtNodeRadiusUseOverride;
static CVarTrack g_vtHeadNodeRadius;
static CVarTrack g_vtTorsoNodeRadius;
static CVarTrack g_vtArmNodeRadius;
static CVarTrack g_vtLegNodeRadius;
static CVarTrack g_HitDebugTrack;
static CVarTrack g_vtDistHitBoxMethod;	// RETAIL: Tron's (Object.lto 0x10138e21)

#define HB_DEFAULT_NODE_RADIUS	12.5f
#define HB_DIMS_ENLARGE_PERCENT 1.2f

#define	HB_DIMS_MIN_XZ	24.0f
#define HB_DIMS_MIN_Y	15.0f

#pragma force_active on
BEGIN_CLASS(CCharacterHitBox)
END_CLASS_DEFAULT_FLAGS(CCharacterHitBox, GameBase, NULL, NULL, CF_HIDDEN)
#pragma force_active off

// RETAIL: the damage types the power armor does not take (Object.lto .data
// 0x102fa664; CCharacterHitBox::AbsorbDamage). PowerArmor.cpp has its own
// copy. The name is ours
static DamageType s_aeArmorProofDamageTypes[] =
{
	DT_DISC,
	DT_SEQUENCER,
	DT_CLUSTER,
	DT_ENEMYDISC,
	DT_CLUSTERAUTOBURST,
};

// RETAIL: Tron's users of a hit box add the other users to test with them
// (IHitBoxUser's slot 9, CThorne: its ball)
typedef std::list<IHitBoxUser*> HitBoxUserList;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RayIntersectSphere()
//
//	PURPOSE:	RETAIL: Tron's (Object.lto 0x101380b0; the name is ours):
//				does the ray from vOrigin along vDir hit the sphere?
//				*pfDist is the distance along the ray.
//
// ----------------------------------------------------------------------- //

static bool RayIntersectSphere( const LTVector& vOrigin, const LTVector& vDir, const LTVector& vCenter, LTFLOAT fRadius, LTFLOAT* pfDist )
{
	LTVector vToCenter = vCenter - vOrigin;
	LTFLOAT fRayDist = vToCenter.Dot( vDir );
	LTFLOAT fDistSqr = vToCenter.Dot( vToCenter );
	LTFLOAT fRadiusSqr = fRadius * fRadius;

	// The sphere is behind the origin, and the origin is outside it.
	if ( fRayDist < 0.0f && fDistSqr > fRadiusSqr )
	{
		*pfDist = 0.0f;
		return false;
	}

	// The ray passes outside the sphere.
	LTFLOAT fMissSqr = fDistSqr - fRayDist * fRayDist;
	if ( fMissSqr > fRadiusSqr )
	{
		*pfDist = 0.0f;
		return false;
	}

	LTFLOAT fHalfChord = (LTFLOAT)sqrt( fRadiusSqr - fMissSqr );
	if ( fDistSqr > fRadiusSqr )
	{
		*pfDist = fRayDist - fHalfChord;
	}
	else
	{
		*pfDist = fRayDist + fHalfChord;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RayIntersectOBB()
//
//	PURPOSE:	RETAIL: Tron's (Object.lto 0x10138190; the name is ours):
//				does the ray from vOrigin along vDir hit the OBB (the slab
//				test)? *pfDist is the distance along the ray.
//
// ----------------------------------------------------------------------- //

static bool RayIntersectOBB( const ModelOBB& obb, const LTVector& vOrigin, const LTVector& vDir, LTFLOAT* pfDist )
{
	LTFLOAT fMin = -1.0e12f;
	LTFLOAT fMax = 1.0e12f;

	LTVector vToCenter = obb.m_Pos - vOrigin;
	LTVector vHalfSize = obb.m_Size * 0.5f;

	LTFLOAT afHalfSize[3] = { vHalfSize.x, vHalfSize.y, vHalfSize.z };
	LTVector avBasis[3] = { obb.m_Basis[0], obb.m_Basis[1], obb.m_Basis[2] };

	for ( int i = 0; i < 3; i++ )
	{
		LTFLOAT e = avBasis[i].Dot( vToCenter );
		LTFLOAT f = avBasis[i].Dot( vDir );

		if ( fabs( f ) > 0.00015 )
		{
			LTFLOAT t1 = ( e + afHalfSize[i] ) / f;
			LTFLOAT t2 = ( e - afHalfSize[i] ) / f;

			if ( t1 > t2 )
			{
				LTFLOAT fTemp = t1;
				t1 = t2;
				t2 = fTemp;
			}

			if ( t1 > fMin )
			{
				fMin = t1;
			}

			if ( t2 < fMax )
			{
				fMax = t2;
			}

			if ( fMin > fMax )
			{
				return false;
			}

			if ( fMax < 0.0f )
			{
				return false;
			}
		}
		else if ( -e - afHalfSize[i] > 0.0f || -e + afHalfSize[i] < 0.0f )
		{
			*pfDist = 0.0f;
			return false;
		}
	}

	if ( fMin > 0.0f )
	{
		*pfDist = fMin;
		return true;
	}

	*pfDist = fMax;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNodeRadius()
//
//	PURPOSE:	Get the model node's radius, checking first to see if they
//				are being overridden.  If they are, then use the manually
//				set values.  If they are not, then use the hit box user's
//				value for this node (RETAIL: a file function in Tron,
//				Object.lto 0x101383b0).
//
// ----------------------------------------------------------------------- //

static LTFLOAT GetNodeRadius( IHitBoxUser* pHitBoxUser, ModelSkeleton eModelSkeleton, ModelNode eModelNode )
{
	// See if we're overriding the radius...

	if (g_vtNodeRadiusUseOverride.GetFloat())
	{
		switch ( g_pModelButeMgr->GetSkeletonNodeLocation(eModelSkeleton, eModelNode) )
		{
			case HL_HEAD :
				return ( g_vtHeadNodeRadius.GetFloat() );
			break;

			case HL_TORSO :
				return ( g_vtTorsoNodeRadius.GetFloat() );
			break;

			case HL_ARM :
				return ( g_vtArmNodeRadius.GetFloat() );
			break;

			case HL_LEG :
				return ( g_vtLegNodeRadius.GetFloat() );
			break;

			default :
				return ( HB_DEFAULT_NODE_RADIUS );
			break;
		}
	}
	else
	{
		return ( pHitBoxUser->GetNodeRadius( eModelSkeleton, eModelNode ) );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	GetNodeTransform()
//              
//	PURPOSE:	Returns by parameter the transform for the passed in node.
//				If the hit box user is an attachment, then use the node of
//				the attached model (RETAIL: a file function in Tron,
//				Object.lto 0x10138490).
//
//----------------------------------------------------------------------------

static LTBOOL GetNodeTransform( IHitBoxUser* pHitBoxUser, HOBJECT hModel, const char* const pszNodeName, LTransform& NodeTransform )
{
	// RETAIL: on each failure Tron reads the model's file name (for a
	// message the final build does not have)

	if ( pHitBoxUser->GetAttachment() != NULL )
	{
		LTRESULT ltResult;

		// Get the Parent through the use of the Attachment Parent/Child retreval
		HOBJECT hChild;
		HOBJECT hParent;
		ltResult = g_pCommonLT->GetAttachmentObjects( pHitBoxUser->GetAttachment(), hParent, hChild);
		if ( ltResult != LT_OK )
		{
			char szModelName[512];
			g_pLTServer->GetModelLT()->GetModelDBFilename( hModel, szModelName, sizeof( szModelName ));
			return LTFALSE;
		}

		// RETAIL: Tron takes the node of the attached model
		HMODELNODE hNode;
		ltResult = g_pModelLT->GetNode(hChild, const_cast<char*>(pszNodeName), hNode);
		if ( ltResult != LT_OK )
		{
			char szModelName[512];
			g_pLTServer->GetModelLT()->GetModelDBFilename( hChild, szModelName, sizeof( szModelName ));
			return LTFALSE;
		}

		ltResult = g_pCommonLT->GetAttachedModelNodeTransform( pHitBoxUser->GetAttachment(), hNode, NodeTransform);
		if ( ltResult != LT_OK )
		{
			char szModelName[512];
			g_pLTServer->GetModelLT()->GetModelDBFilename( hModel, szModelName, sizeof( szModelName ));
			return LTFALSE;
		}
	}
	else
	{
		LTRESULT ltResult;
		HMODELNODE hNode;
		ltResult = g_pModelLT->GetNode(hModel, const_cast<char*>(pszNodeName), hNode);
		if ( ltResult != LT_OK )
		{
			char szModelName[512];
			g_pLTServer->GetModelLT()->GetModelDBFilename( hModel, szModelName, sizeof( szModelName ));
			return LTFALSE;
		}

		ltResult = g_pModelLT->GetNodeTransform(hModel, hNode, NodeTransform, LTTRUE);
		if ( ltResult != LT_OK )
		{
			char szModelName[512];
			g_pLTServer->GetModelLT()->GetModelDBFilename( hModel, szModelName, sizeof( szModelName ));
			return LTFALSE;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TestNodeSphere()
//
//	PURPOSE:	RETAIL: Tron's (Object.lto 0x10138650; the name is ours):
//				the node test of FindHitNodeSphere. Keeps the nearest hit
//				of a node that has a hit priority.
//
// ----------------------------------------------------------------------- //

static bool TestNodeSphere( HitBoxNodeTest* pTest, HitBoxHit* pHit )
{
	LTFLOAT fPriority = g_pModelButeMgr->GetSkeletonNodeHitPriority( pTest->m_pHitBoxUser->GetModelSkeleton(), pTest->m_eModelNode );
	if ( fPriority == 0.0f )
	{
		return false;
	}

	LTFLOAT fNodeRadius = GetNodeRadius( pTest->m_pHitBoxUser, pTest->m_pHitBoxUser->GetModelSkeleton(), pTest->m_eModelNode );

	LTFLOAT fDist;
	if ( !RayIntersectSphere( pTest->m_vFrom, pTest->m_vDir, pTest->m_tfNode.m_Pos, fNodeRadius, &fDist ))
	{
		return false;
	}

	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
	{
		g_pLTServer->CPrint("Hit ''%s'' node", g_pModelButeMgr->GetSkeletonNodeName( pTest->m_pHitBoxUser->GetModelSkeleton(), pTest->m_eModelNode ));
	}

	if ( fDist > pHit->m_fDist )
	{
		return false;
	}

	pHit->m_eModelNode = pTest->m_eModelNode;
	pHit->m_fPriority = fPriority;
	pHit->m_fDist = fDist;
	pHit->m_hObject = pTest->m_pHitBoxUser->TronUnknownVirtual28();

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CCharacterHitBox()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::CCharacterHitBox() : GameBase(OT_NORMAL)
{
	// Setup the TransitionAggregate
	MakeTransitionable();

	m_pHitBoxUser = NULL;
    m_hModel = LTNULL;
	m_vOffset.Init();
	m_bCanActivate = LTTRUE;
	m_bCanBeSearched = false;
	m_bTronUnknown7D = false;	// RETAIL: Tron's (Object.lto 0x10138858)
	m_bAnimControlsDims = false;
	m_bAnimControlsOffset = false;
	m_hControllingAnim = INVALID_ANI;

    m_NodeRadiusList.Init(LTTRUE);

	// RETAIL: Tron's (Object.lto 0x10138881)
	m_nNumOBBs = 0;
	m_pOBBs = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::~CCharacterHitBox()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::~CCharacterHitBox()
{
	// RETAIL: Tron's (Object.lto 0x101388ed)
	if( m_nNumOBBs )
	{
		debug_deletea( m_pOBBs );
		debug_deletea( m_pOBBNodes );
		m_nNumOBBs = 0;
		m_pOBBs = LTNULL;
		m_pOBBNodes = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::InitOBBs()
//
//	PURPOSE:	RETAIL: Tron's (Object.lto 0x10138a40; the name is ours):
//				copy the OBBs of the model, and find the skeleton node of
//				each
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::InitOBBs()
{
	g_pModelLT->GetNumModelOBBs( m_hModel, m_nNumOBBs );
	if( m_nNumOBBs && !m_pOBBs )
	{
		m_pOBBs = debug_newa( ModelOBB, m_nNumOBBs );
		m_pOBBNodes = debug_newa( ModelNode, m_nNumOBBs );
		g_pModelLT->GetModelOBBCopy( m_hModel, m_pOBBs );

		for( uint32 iOBB = 0; iOBB < m_nNumOBBs; iOBB++ )
		{
			char szNodeName[256];
			g_pModelLT->GetNodeName( m_hModel, m_pOBBs[iOBB].m_iNode, szNodeName, sizeof( szNodeName ));
			m_pOBBNodes[iOBB] = g_pModelButeMgr->GetSkeletonNode( m_pHitBoxUser->GetModelSkeleton(), szNodeName );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Init()
//
//	PURPOSE:	Initialize object.  Takes as an argument the hObject of the 
//				object using the hitbox
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::Init(HOBJECT hModel, IHitBoxUser* pUser)
{
	AIASSERT( pUser, m_hObject, "CharacterHitBox Inited with non existant IHitBoxUser" );
	AIASSERT( hModel, m_hObject, "CharacterHitBox Inited with non existant hObject" );
	AIASSERT( m_hObject, m_hObject, "CharacterHitBox does not exist" );
	AIASSERT( pUser, m_hObject, "" );
	AIASSERT( dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(hModel)) == pUser, hModel,  "Passed in hModel and pUser should reference the same class -- HandleToObject(hModel) should return what pUser points to" );

	m_hModel = hModel;
	m_pHitBoxUser = pUser;

	// RETAIL: Tron's (Object.lto 0x10138b55)
	InitOBBs();

	// Set my flags...

	// RETAIL: Tron sets no USRFLG_CHARACTER here (Object.lto 0x10138b6f)
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_RAYHIT | FLAG_TOUCHABLE, FLAGMASK_ALL);

	if (!g_vtShowNodeRadii.IsInitted())
	{
        g_vtShowNodeRadii.Init(g_pLTServer, "HitBoxShowNodeRadii", LTNULL, 0.0f);
	}

	if (!g_vtNodeRadiusUseOverride.IsInitted())
	{
        g_vtNodeRadiusUseOverride.Init(g_pLTServer, "HitBoxNodeRadiusOverride", LTNULL, 0.0f);
	}

	if (!g_vtHeadNodeRadius.IsInitted())
	{
        g_vtHeadNodeRadius.Init(g_pLTServer, "HitBoxHeadNodeRadius", LTNULL, HB_DEFAULT_NODE_RADIUS);
	}

	if (!g_vtTorsoNodeRadius.IsInitted())
	{
        g_vtTorsoNodeRadius.Init(g_pLTServer, "HitBoxTorsoNodeRadius", LTNULL, HB_DEFAULT_NODE_RADIUS);
	}

	if (!g_vtArmNodeRadius.IsInitted())
	{
        g_vtArmNodeRadius.Init(g_pLTServer, "HitBoxArmNodeRadius", LTNULL, HB_DEFAULT_NODE_RADIUS);
	}

	if (!g_vtLegNodeRadius.IsInitted())
	{
        g_vtLegNodeRadius.Init(g_pLTServer, "HitBoxLegNodeRadius", LTNULL, HB_DEFAULT_NODE_RADIUS);
	}

	if (!g_HitDebugTrack.IsInitted())
	{
        g_HitDebugTrack.Init(g_pLTServer, "HitDebug", LTNULL, 0.0f);
	}

	if (!g_vtShowPlayerNodeRadii.IsInitted())
	{
        g_vtShowPlayerNodeRadii.Init(g_pLTServer, "HitBoxShowPlayerNodeRadii", LTNULL, 0.0f);
	}

	// RETAIL: Tron's (Object.lto 0x10138e21)
	if (!g_vtDistHitBoxMethod.IsInitted())
	{
        g_vtDistHitBoxMethod.Init(g_pLTServer, "DistHitBoxMethod", LTNULL, 0.0f);
	}

	SetNextUpdate(UPDATE_NEVER);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacterHitBox::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		// RETAIL: Tron's (Object.lto 0x10138e7c)
		case MID_ALLOBJECTSCREATED:
		{
			if( m_hModel )
			{
				InitOBBs();
			}
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacterHitBox::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    // Pass any messages to our model...

	if (m_hModel)
	{
		LPBASECLASS pBase = g_pLTServer->HandleToObject(m_hModel);
		if (pBase)
		{
			pBase->ObjectMessageFn(hSender, pMsg);
		}
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //

LTVector CCharacterHitBox::GetBoundingBoxColor()
{
    return LTVector(1.0, 1.0, 0.0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetCanBeSearched()
//
//	PURPOSE:	Set us to be searchable or not and notify the client...
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetCanBeSearched(bool b) 
{
	if( m_bCanBeSearched == b )
		return;

	m_bCanBeSearched = b;

	if( m_pHitBoxUser )
	{
		m_pHitBoxUser->UpdateClientHitBox();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetTronUnknown7D
//
//	PURPOSE:	RETAIL: Tron's (0x10138f80): the client hit box is updated
//				only when the flag is set
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetTronUnknown7D(bool b)
{
	if( m_bTronUnknown7D == b )
		return;

	m_bTronUnknown7D = b;

	if( b )
	{
		m_pHitBoxUser->UpdateClientHitBox();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleImpact()
//
//	PURPOSE:	Handle a vector impacting on us (RETAIL: 0x10138fa0)
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector vDir, LTVector & vFrom)
{
	return HandleImpact( pProj, iInfo, vDir, vFrom, LTFALSE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleImpactFromPos()
//
//	PURPOSE:	RETAIL: Tron's (0x10138fe0): as HandleImpact, but the
//				test starts at vFrom
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleImpactFromPos(CProjectile* pProj, IntersectInfo& iInfo, LTVector vDir, LTVector & vFrom)
{
	return HandleImpact( pProj, iInfo, vDir, vFrom, LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleImpact()
//
//	PURPOSE:	RETAIL: Tron's (0x10139020): a projectile does not hit
//				the hit box of the object that fired it (nor of the users
//				that this user adds). When nothing is hit, vFrom is moved
//				past the hit point.
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector vDir, LTVector & vFrom, LTBOOL bFromPos)
{
    if (!pProj || !m_hModel)
	{
		return LTFALSE;
	}

	// Check to see if this CharacterHitBox is using HitDetection.  If not,
	// then return right away.
	if (!m_pHitBoxUser->UsingHitDetection())
	{
		return LTTRUE;
	}

	if ( !pProj->TronUnknownVirtual26() )
	{
		if ( m_hModel == pProj->GetFiredFrom() || m_pHitBoxUser->TronUnknownVirtual28() == pProj->GetFiredFrom() )
		{
			return LTFALSE;
		}
	}

	HitBoxUserList lstHitBoxUsers;
	m_pHitBoxUser->TronUnknownVirtual24( (int)&lstHitBoxUsers );

	for ( HitBoxUserList::iterator it = lstHitBoxUsers.begin(); it != lstHitBoxUsers.end(); ++it )
	{
		if ( (*it)->TronUnknownVirtual28() == pProj->GetFiredFrom() )
		{
			return LTFALSE;
		}
	}

	ModelNode eModelNode = eModelNodeInvalid;
	if ( !HandleVectorImpact( pProj, iInfo, vDir, vFrom, eModelNode, bFromPos ))
	{
		//
		// Shot didn't hit box, propagate through box and return that point in
		// vFrom
		//

		vFrom = iInfo.m_Point + vDir*5.0f;

		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::FindHitNodeSphere()
//
//	PURPOSE:	RETAIL: Tron's (0x10139240)
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::FindHitNodeSphere( const LTVector& vDir, const LTVector& vFrom, HitBoxHit* pHit )
{
	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
	{
        g_pLTServer->CPrint("Checking hit nodes..................");
	}

	HitBoxNodeTest test( TestNodeSphere, m_pHitBoxUser, vDir, vFrom );
	TestNodes( &test, pHit );

	// Test the users that our user adds; stop when one of them is hit.

	HitBoxUserList lstHitBoxUsers;
	m_pHitBoxUser->TronUnknownVirtual24( (int)&lstHitBoxUsers );

	for ( HitBoxUserList::iterator it = lstHitBoxUsers.begin(); it != lstHitBoxUsers.end(); ++it )
	{
		HitBoxNodeTest userTest( TestNodeSphere, *it, vDir, vFrom );
		TestNodes( &userTest, pHit );

		if ( m_pHitBoxUser->TronUnknownVirtual28() != pHit->m_hObject )
		{
			break;
		}
	}

	// A node of another user is not ours.

	if ( m_pHitBoxUser->TronUnknownVirtual28() != pHit->m_hObject )
	{
		pHit->m_eModelNode = eModelNodeInvalid;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::FindHitNodeOBB()
//
//	PURPOSE:	RETAIL: Tron's (0x10139460)
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::FindHitNodeOBB( const LTVector& vDir, const LTVector& vFrom, HitBoxHit* pHit )
{
	g_pModelLT->UpdateModelOBB( m_hModel, m_pOBBs );

	LTFLOAT fDist;
	LTFLOAT fNearest = 1.0e12f;
	pHit->m_eModelNode = eModelNodeInvalid;

	LTVector vRay( vDir.x * 1000.0f, vDir.y * 1000.0f, vDir.z * 1000.0f );

	for ( uint32 iOBB = 0; iOBB < m_nNumOBBs; iOBB++ )
	{
#ifdef TRON20SRC_MODERN
		// GAME-84: only OBBs with a node; keep the distance along vDir (the
		// test's distance is along vRay, 1000 times vDir) and the user hit
		if ( m_pOBBNodes[iOBB] != eModelNodeInvalid &&
			RayIntersectOBB( m_pOBBs[iOBB], vFrom, vRay, &fDist ) && fDist <= fNearest )
		{
			fNearest = fDist;
			pHit->m_eModelNode = m_pOBBNodes[iOBB];
			pHit->m_fDist = fDist * 1000.0f;
			pHit->m_hObject = m_pHitBoxUser->TronUnknownVirtual28();
		}
#else
		if ( RayIntersectOBB( m_pOBBs[iOBB], vFrom, vRay, &fDist ) && fDist <= fNearest )
		{
			// RETAIL: the node is kept even when it is not a skeleton node,
			// and the hit's distance is not set (docs/RETAIL_BUGS.md)
			pHit->m_eModelNode = m_pOBBNodes[iOBB];
			if ( pHit->m_eModelNode != eModelNodeInvalid )
			{
				fNearest = fDist;
			}
		}
#endif
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::TestNodes()
//
//	PURPOSE:	RETAIL: Tron's (0x10139540)
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::TestNodes( HitBoxNodeTest* pTest, HitBoxHit* pHit )
{
	ModelSkeleton eModelSkeleton = pTest->m_pHitBoxUser->GetModelSkeleton();

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		pTest->m_eModelNode = (ModelNode)iNode;

		// Don't do transforms if we don't need to...
		if ( GetNodeRadius( eModelSkeleton, (ModelNode)iNode ) <= 0.0f )
		{
			continue;
		}

		const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, pTest->m_eModelNode);
		if ( !szNodeName )
		{
			continue;
		}

		if ( !GetNodeTransform( pTest->m_pHitBoxUser, pTest->m_pHitBoxUser->TronUnknownVirtual28(), szNodeName, pTest->m_tfNode ))
		{
			continue;
		}

		pTest->m_pfnTest( pTest, pHit );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::AbsorbDamage()
//
//	PURPOSE:	RETAIL: Tron's (0x101395d0): each power armor that is on
//				takes the damage (unless it is of a type the armor does not
//				take); the damage is less by the mean hit points the armors
//				had. An armor with no hit points left is detached.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::AbsorbDamage( DamageStruct* pDamage )
{
	static HCLASS hPowerArmor = g_pLTServer->GetClass("PowerArmor");

	uint32 nArmors = 0;
	LTFLOAT fArmorHitPoints = 0.0f;

	int cAttachmentPositions = m_pHitBoxUser->GetAttachments() ? m_pHitBoxUser->GetAttachments()->GetNumAttachmentPositions() : 0;
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_pHitBoxUser->GetAttachments()->GetAttachment((uint32)iAttachmentPosition);
		if ( pAttachmentPosition->GetAttachment() && pAttachmentPosition->GetAttachment()->GetType() == ATTACHMENT_TYPE_OBJECT )
		{
			CAttachment* pAttachment = pAttachmentPosition->GetAttachment();

			// RETAIL: the classes are given in reverse order
			if ( g_pLTServer->IsKindOf(hPowerArmor, g_pLTServer->GetObjectClass(pAttachment->GetModel())) )
			{
				PowerArmor* pPowerArmor = dynamic_cast<PowerArmor*>(g_pLTServer->HandleToObject(pAttachment->GetModel()));
				if ( pPowerArmor && pPowerArmor->m_eState == PowerArmor::kPA_On )
				{
					bool bArmorProof = false;
					for ( int iType = 0; iType < ARRAY_LEN(s_aeArmorProofDamageTypes); iType++ )
					{
						if ( pDamage->eType == s_aeArmorProofDamageTypes[iType] )
						{
							bArmorProof = true;
							break;
						}
					}

					if ( !bArmorProof )
					{
						fArmorHitPoints = pPowerArmor->m_fHitPoints + fArmorHitPoints;
						nArmors++;

						LTFLOAT fHitPoints = pPowerArmor->m_fHitPoints - pDamage->fDamage;
						if ( fHitPoints <= 0.0f )
						{
							fHitPoints = 0.0f;
						}

						if ( pPowerArmor->m_bCanDamage )
						{
							pPowerArmor->m_fHitPoints = fHitPoints;
						}

						if ( pPowerArmor->m_fHitPoints <= 0.0f )
						{
							PlayClientFX( (char*)pPowerArmor->m_sOnDestroyFX.c_str(), pPowerArmor->m_hObject );
							m_pHitBoxUser->GetAttachments()->Detach( pAttachmentPosition );
						}
					}
				}
			}
		}
	}

	if ( nArmors > 0 )
	{
		pDamage->fDamage -= fArmorHitPoints / nArmors;
		if ( pDamage->fDamage <= 0.0f )
		{
			pDamage->fDamage = 0.0f;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleVectorImpact()
//
//	PURPOSE:	Handle being hit by a vector (RETAIL: Tron's, 0x101397d0)
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleVectorImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector vDir, LTVector vFrom, ModelNode& eModelNode, LTBOOL bFromPos)
{
	static HCLASS hPowerArmor = g_pLTServer->GetClass("PowerArmor");

	HitBoxHit hit;

	if ( !bFromPos )
	{
		vFrom = iInfo.m_Point;
	}
	else
	{
		iInfo.m_Point = vFrom;
	}

	// See if we hit any nodes.

	if ( m_nNumOBBs )
	{
		FindHitNodeOBB( vDir, vFrom, &hit );
	}
	else
	{
		FindHitNodeSphere( vDir, vFrom, &hit );
	}

	eModelNode = hit.m_eModelNode;

	// Did we hit something?

	if ( eModelNode != eModelNodeInvalid )
	{
		ModelSkeleton eModelSkeleton = m_pHitBoxUser->GetModelSkeleton();

		// Set the hit object as us ... attachments may change this

		iInfo.m_hObject = m_hModel;
		iInfo.m_Point = hit.m_fDist * vDir + iInfo.m_Point;

		HMODELNODE hNode;
		g_pModelLT->GetNode( m_hModel, const_cast<char*>(g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eModelNode)), hNode );

		// Let the attachments handle the impact. A power armor takes the
		// impact only on the node it is attached to.

		int cAttachmentPositions = m_pHitBoxUser->GetAttachments() ? m_pHitBoxUser->GetAttachments()->GetNumAttachmentPositions() : 0;
		for ( int iAttachmentPosition = 0 ; iAttachmentPosition < cAttachmentPositions ; iAttachmentPosition++ )
		{
			CAttachmentPosition* pAttachmentPosition = m_pHitBoxUser->GetAttachments()->GetAttachment((uint32)iAttachmentPosition);
			if ( !pAttachmentPosition->GetAttachment() )
			{
				continue;
			}

			if ( pAttachmentPosition->GetAttachment()->GetType() == ATTACHMENT_TYPE_OBJECT )
			{
				CAttachment* pAttachment = pAttachmentPosition->GetAttachment();

				// RETAIL: the classes are given in reverse order
				if ( g_pLTServer->IsKindOf(hPowerArmor, g_pLTServer->GetObjectClass(pAttachment->GetModel())) )
				{
					HMODELSOCKET hSocket;
					g_pModelLT->GetSocket( m_hModel, pAttachmentPosition->GetName(), hSocket );

					HMODELNODE hSocketNode;
					g_pModelLT->GetSocketParentNode( m_hModel, hSocket, hSocketNode );

					if ( hSocketNode != hNode )
					{
						continue;
					}

					PowerArmor* pPowerArmor = dynamic_cast<PowerArmor*>(g_pLTServer->HandleToObject(pAttachment->GetModel()));
					if ( !pPowerArmor || !pPowerArmor->HandleProjectileImpact( pProj, pAttachmentPosition ))
					{
						continue;
					}
				}
			}

			if ( pAttachmentPosition->GetAttachment() )
			{
				pAttachmentPosition->GetAttachment()->HandleProjectileImpact( pProj, pAttachmentPosition, iInfo, vDir, vFrom, eModelSkeleton, eModelNode );
			}
		}

		m_pHitBoxUser->SetModelNodeLastHit(eModelNode);

		// Don't adjust damage on players or in multiplayer...

		if ( IsCharacter( iInfo.m_hObject ) && !IsMultiplayerGame() && !IsPlayer( iInfo.m_hObject ))
		{
			pProj->AdjustDamage( m_pHitBoxUser->ComputeDamageModifier( eModelNode ));
		}

		if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
		{
			g_pLTServer->CPrint("...........using ''%s''", g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eModelNode));
		}

		return LTTRUE;
	}

	if ( g_HitDebugTrack.GetFloat(0.0f) == 1.0f )
	{
		g_pLTServer->CPrint("....................hit nothing");
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::DidProjectileImpact()
//
//	PURPOSE:	See if the projectile actually hit us (RETAIL: Tron's,
//				0x10139b80: the hit box test from the fire position)
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::DidProjectileImpact(CProjectile* pProjectile)
{
    if (!m_hModel || !pProjectile)
	{
		return LTFALSE;
	}

	if (!m_pHitBoxUser->UsingHitDetection())
	{
		return LTTRUE;
	}

	IntersectInfo iInfo;
	ModelNode eModelNode;
	return HandleVectorImpact( pProjectile, iInfo, pProjectile->GetFireDir(), pProjectile->GetFirePos(), eModelNode, LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetNodeRadius()
//
//	PURPOSE:	RETAIL: 0x10139c60
//
// ----------------------------------------------------------------------- //

LTFLOAT CCharacterHitBox::GetNodeRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	return ::GetNodeRadius( m_pHitBoxUser, eModelSkeleton, eModelNode );
}


static LTVector GetNodeModelColor(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	LTVector vColor(1, 1, 1);

	switch ( g_pModelButeMgr->GetSkeletonNodeLocation(eModelSkeleton, eModelNode) )
	{
		case HL_HEAD :
			vColor = LTVector(1, 0, 0);
		break;

		case HL_TORSO :
			vColor = LTVector(1, 1, 0);
		break;

		case HL_ARM :
			vColor = LTVector(0, 1, 0);
		break;

		case HL_LEG :
			vColor = LTVector(0, 0, 1);
		break;
	}

	return vColor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CreateNodeRadiusModels()
//
//	PURPOSE:	Create models representing the model node radii.  Iterates
//				over all of the nodes in a model, and if a node transform 
//				successfully can be retrieved, created the model and adds
//				it to the list.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::CreateNodeRadiusModels()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );

	if (!m_hModel)
	{
		return;
	}

	ModelSkeleton eModelSkeleton = m_pHitBoxUser->GetModelSkeleton();
	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);

	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;
		const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

        LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, eCurrentNode);
		if (fNodeRadius <= 0.0f)
		{
			continue;
		}

  		if (!szNodeName)
		{
			continue;
		}

        LTransform transform;
		if ( GetNodeTransform( m_pHitBoxUser, m_hModel, szNodeName, transform ) == LTFALSE )
		{
			continue;
		}


		// Create the radius model...

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		theStruct.m_Pos = transform.m_Pos;
		SAFE_STRCPY(theStruct.m_Filename, "Models\\sphere.ltb");
		theStruct.m_Flags = FLAG_VISIBLE;
		theStruct.m_ObjectType = OT_MODEL;

        HCLASS hClass = g_pLTServer->GetClass("BaseClass");
        LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
		UBER_ASSERT( pModel, "Failed to create BaseClass" );
		if (!pModel)
		{
			return;
		}

		// Don't eat ticks please...
		::SetNextUpdate(pModel->m_hObject, UPDATE_NEVER);

        LTVector vScale;
		vScale.Init(fNodeRadius, fNodeRadius, fNodeRadius);
        g_pLTServer->ScaleObject(pModel->m_hObject, &vScale);

		NodeRadiusStruct* pNRS = debug_new(NodeRadiusStruct);

		pNRS->eNode	 = eCurrentNode;
		pNRS->hModel = pModel->m_hObject;

		// Set the model's color...
		LTVector vColor = GetNodeModelColor(eModelSkeleton, eCurrentNode);
		g_pLTServer->SetObjectColor(pModel->m_hObject, vColor.x, vColor.y, vColor.z, 0.9f);

		// Add the model to our list...

		m_NodeRadiusList.AddTail(pNRS);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::RemoveNodeRadiusModels()
//
//	PURPOSE:	Remove models representing the model node radii
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::RemoveNodeRadiusModels()
{
	if (m_NodeRadiusList.GetLength())
	{
		m_NodeRadiusList.Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::UpdateNodeRadiusModels()
//
//	PURPOSE:	Update models representing the model node radii
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::UpdateNodeRadiusModels()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );

	// Create the models if necessary...

	if (!m_NodeRadiusList.GetLength())
	{
		CreateNodeRadiusModels();
	}

	ModelSkeleton eModelSkeleton = m_pHitBoxUser->GetModelSkeleton();

	NodeRadiusStruct** pNRS = m_NodeRadiusList.GetItem(TLIT_FIRST);

	while (pNRS && *pNRS && (*pNRS)->hModel)
	{
		const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, (*pNRS)->eNode);

		LTransform resultTransform;
		if ( GetNodeTransform( m_pHitBoxUser, m_hModel, szNodeName, resultTransform ) != LTFALSE )
		{
			g_pLTServer->SetObjectPos((*pNRS)->hModel, &(resultTransform.m_Pos));

			LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, (*pNRS)->eNode);

			LTVector vScale;
			vScale.Init(fNodeRadius, fNodeRadius, fNodeRadius);
			g_pLTServer->ScaleObject((*pNRS)->hModel, &vScale);
		}
	
		pNRS = m_NodeRadiusList.GetItem(TLIT_NEXT);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Update()
//
//	PURPOSE:	Update our position
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Update()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );
	if (!m_hModel || !m_hObject)
	{
		return;
	}

    LTVector vPos, vMyPos;
	g_pLTServer->GetObjectPos(m_hModel, &vPos);
	
	// Make sure the hit box offset is relative to the model...
	
	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hModel, &rRot );

	LTMatrix mMat;
	rRot.ConvertToMatrix( mMat );

	vPos += (mMat * m_vOffset);

	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

	if (vPos.DistSqr(vMyPos) > 0.1)
	{
		g_pLTServer->SetObjectPos(m_hObject, &vPos);
	}
	
	if( (m_bAnimControlsDims || m_bAnimControlsOffset) && (m_hControllingAnim != INVALID_ANI) )
	{
		HMODELANIM hCurAnim = INVALID_ANI;
		if( LT_OK == g_pModelLT->GetCurAnim( m_hModel, MAIN_TRACKER, hCurAnim ))
		{
			if( hCurAnim != m_hControllingAnim )
			{
				// We changed animations from our controlling anim so default our dims and offset...

				// Set offset first since SetDimsToModel() will update the client
				SetOffset( LTVector(0,0,0) );
				SetDimsToModel();

				// The animation is no longer controlling us...

				m_bAnimControlsDims = m_bAnimControlsOffset = false;
				m_hControllingAnim = INVALID_ANI;
			}	
		}
	}
	
	// Make sure the hit box is at least at the minimum dims...

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

	if( vDims.x < HB_DIMS_MIN_XZ ||
		vDims.z < HB_DIMS_MIN_XZ )
	{
		vDims.x = vDims.z = HB_DIMS_MIN_XZ;
		g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

		m_pHitBoxUser->UpdateClientHitBox();
	}

	if( vDims.y < HB_DIMS_MIN_Y	)
	{
		vDims.y = HB_DIMS_MIN_Y;
		g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

		m_pHitBoxUser->UpdateClientHitBox();
	}

	// See if we should show our model node radii...

#ifndef _FINAL
	float fShowNodeRadii = (g_vtShowNodeRadii.GetFloat() ? (IsPlayer(m_hModel) ? g_vtShowPlayerNodeRadii.GetFloat() : 1.0f) : 0.0f);
	if (fShowNodeRadii)
	{
		UpdateNodeRadiusModels();
	}
	else
	{
		RemoveNodeRadiusModels();
	}
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetDimsToModel
//
//	PURPOSE:	Set the dims of the hit box to be an enlarged percentage of the models dims.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetDimsToModel()
{
	if( !m_hModel || !m_hObject )
		return;

	// RETAIL: Tron takes the model's own dims and enlarges them here
	// (0x10139f20)
	LTVector vModelDims;
	g_pPhysicsLT->GetObjectDims( m_hModel, &vModelDims );

	vModelDims *= HB_DIMS_ENLARGE_PERCENT;

	g_pPhysicsLT->SetObjectDims( m_hObject, &vModelDims, 0 );

	if( m_pHitBoxUser )
	{
		m_pHitBoxUser->UpdateClientHitBox();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::EnlargeAndSetDims
//
//	PURPOSE:	Set the dims of the hit box to be an enlarged percentage of the given dims.
//				Use this to be consistent when setting the hitbox dims.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::EnlargeAndSetDims( LTVector &vDims )
{
	if( !m_hObject )
		return;

	// Enlarge the hit box dims by a percentage of the given dims...
	 
	vDims *= HB_DIMS_ENLARGE_PERCENT;

	g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );
	
	// Let the HitBoxUser notify the clients of the change...

	if( m_pHitBoxUser )
	{
		m_pHitBoxUser->UpdateClientHitBox();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Save(ILTMessage_Write *pMsg)
{
	AIASSERT( pMsg, m_hObject,  "Save with no pMsg" );
	if (!pMsg)
	{
		return;
	}

    SAVE_HOBJECT(m_hModel);
	SAVE_VECTOR(m_vOffset);
	SAVE_BOOL(m_bCanActivate);
	SAVE_bool(m_bCanBeSearched);
	SAVE_bool(m_bTronUnknown7D);
	SAVE_bool( m_bAnimControlsDims );
	SAVE_bool( m_bAnimControlsOffset );
	SAVE_DWORD( m_hControllingAnim );
	AIASSERT( dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(m_hModel)) == m_pHitBoxUser, NULL, "" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Load(ILTMessage_Read *pMsg)
{
	AIASSERT( pMsg, m_hObject, "Load with no pMsg" );
	if (!pMsg)
	{
		return;
	}

	LOAD_HOBJECT(m_hModel);
	LOAD_VECTOR(m_vOffset);
    LOAD_BOOL(m_bCanActivate);
	LOAD_bool(m_bCanBeSearched);
	LOAD_bool(m_bTronUnknown7D);
	LOAD_bool( m_bAnimControlsDims );
	LOAD_bool( m_bAnimControlsOffset );
	LOAD_DWORD_CAST( m_hControllingAnim, HMODELANIM );

	m_pHitBoxUser = dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(m_hModel));
	AIASSERT( m_pHitBoxUser, m_hModel, "Unable to get the HitBoxUser interface from m_hModel." );
}


