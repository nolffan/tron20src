// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.h
//
// PURPOSE : Character hit box object class definition
//
// CREATED : 01/05/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_HIT_BOX_H__
#define __CHARACTER_HIT_BOX_H__

#include "GameBase.h"
#include "ModelButeMgr.h"
#include "TemplateList.h"
#include <float.h>

class CProjectile;
class CAttachments;
struct IHitBoxUser;
struct DamageStruct;

// RETAIL: Tron's best hit of a hit box test (0x10 bytes; initialized
// inline, Object.lto 0x101397f9 and AIGoalSpecialDamage 0x10191e3e).
// Names ours
struct HitBoxHit
{
	HitBoxHit()
		: m_hObject( LTNULL ),
		m_eModelNode( eModelNodeInvalid ),
		m_fPriority( FLT_MIN ),
		m_fDist( FLT_MAX )
	{
	}

	HOBJECT		m_hObject;		// the hit box user that was hit
	ModelNode	m_eModelNode;
	LTFLOAT		m_fPriority;
	LTFLOAT		m_fDist;		// along the ray
};

// RETAIL: Tron's node test of one hit box user (0x4c bytes; built in
// CCharacterHitBox::FindHitNodeSphere, Object.lto 0x10139240). The test
// function is called for each node that has a transform. Names ours
struct HitBoxNodeTest;
typedef bool (*HitBoxNodeTestFn)( HitBoxNodeTest* pTest, HitBoxHit* pHit );

struct HitBoxNodeTest
{
	HitBoxNodeTest( HitBoxNodeTestFn pfnTest, IHitBoxUser* pHitBoxUser, const LTVector& vDir, const LTVector& vFrom )
		: m_pfnTest( pfnTest ),
		m_pHitBoxUser( pHitBoxUser ),
		m_vDir( vDir ),
		m_vFrom( vFrom ),
		m_eModelNode( eModelNodeInvalid )
	{
	}

	HitBoxNodeTestFn	m_pfnTest;
	IHitBoxUser*		m_pHitBoxUser;
	LTVector			m_vDir;
	LTVector			m_vFrom;
	ModelNode			m_eModelNode;	// the node being tested
	LTransform			m_tfNode;		// its transform
};

LINKTO_MODULE( CharacterHitBox );

struct NodeRadiusStruct
{
	NodeRadiusStruct()
	{
        hModel = LTNULL;
		eNode = eModelNodeInvalid;
	}

	~NodeRadiusStruct()
	{
		if (hModel)
		{
            g_pLTServer->RemoveObject(hModel);
		}
	}

	LTObjRef hModel;
	ModelNode eNode;
};

class CCharacterHitBox : public GameBase
{
	public :

		CCharacterHitBox();
		virtual ~CCharacterHitBox();

		LTBOOL   Init(HOBJECT hModel, IHitBoxUser* pUser);
        void  SetOffset(LTVector vOffset) { m_vOffset = vOffset; }
		const LTVector& GetOffset() const { return m_vOffset; }

		void  Update();

        LTBOOL HandleImpact(CProjectile* pProj, IntersectInfo & iInfo,
            LTVector vDir, LTVector & vFrom);
		// RETAIL: Tron's (0x10138fe0; CProjectile::GetHitBoxImpact): the
		// test starts at vFrom, not at the hit point. The name is ours
        LTBOOL HandleImpactFromPos(CProjectile* pProj, IntersectInfo & iInfo,
            LTVector vDir, LTVector & vFrom);

        LTBOOL DidProjectileImpact(CProjectile* pProjectile);

		// RETAIL: Tron's (0x10139240; AIGoalSpecialDamage calls it): test
		// the ray against the node spheres of the user and of the users it
		// adds (CThorne's ball). The name is ours
		void	FindHitNodeSphere( const LTVector& vDir, const LTVector& vFrom, HitBoxHit* pHit );

		// RETAIL: Tron's (0x101395d0): the power armor attached to the user
		// takes damage before the user. The name is ours
		void	AbsorbDamage( DamageStruct* pDamage );

		HOBJECT	GetModelObject() const { return m_hModel; }

		LTBOOL CanActivate() const { return m_bCanActivate; }
		void SetCanActivate(LTBOOL b) { m_bCanActivate = b; }
		// RETAIL: Tron's (0x10138f80). The name is ours
		void SetTronUnknown7D(bool b);
		bool GetTronUnknown7D() const { return m_bTronUnknown7D; }	// RETAIL: read by CCharacter::UpdateClientHitBox (0x10091ade)

		bool CanBeSearched() const { return m_bCanBeSearched; }
		void SetCanBeSearched(bool b);

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		void	SetAnimControllingDims( bool bControls, HMODELANIM hAni ) { m_bAnimControlsDims = bControls; m_hControllingAnim = hAni; }
		bool	IsAnimControllingDims() { return m_bAnimControlsDims; }

		void	SetAnimControllingOffset( bool bControls, HMODELANIM hAni ) { m_bAnimControlsOffset = bControls; m_hControllingAnim = hAni; }	
		bool	IsAnimControllingOffset() { return m_bAnimControlsOffset; }

		void	SetDimsToModel();
		// RETAIL: no GetDefaultModelDims (NOLF2 1.x's): Tron's Body::Update
		// and CAI::SetUnconscious take the hit box's own dims
		void	EnlargeAndSetDims( LTVector &vDims );

	protected :

        virtual	LTVector GetBoundingBoxColor();

		// RETAIL: Tron's (0x10139020): HandleImpact and HandleImpactFromPos
		LTBOOL	HandleImpact(CProjectile* pProj, IntersectInfo & iInfo,
			LTVector vDir, LTVector & vFrom, LTBOOL bFromPos);

		// RETAIL: Tron's (0x10139460): test the ray against the model's
		// OBBs. The name is ours
		void	FindHitNodeOBB( const LTVector& vDir, const LTVector& vFrom, HitBoxHit* pHit );

		// RETAIL: Tron's (0x10139540): call the test for each node of the
		// test's user. The name is ours
		void	TestNodes( HitBoxNodeTest* pTest, HitBoxHit* pHit );

		// RETAIL: Tron's (0x101397d0)
        LTBOOL  HandleVectorImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector vDir,
            LTVector vFrom, ModelNode& eModelNode, LTBOOL bFromPos);

        LTFLOAT			GetNodeRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		IHitBoxUser* m_pHitBoxUser;
		LTObjRef	m_hModel;
        LTVector	m_vOffset;
		
		bool		m_bAnimControlsDims;
		bool		m_bAnimControlsOffset;

		HMODELANIM	m_hControllingAnim;

	private :

		// Debugging helper...
		CTList<NodeRadiusStruct*>	m_NodeRadiusList;

		LTBOOL	m_bCanActivate;
		bool	m_bCanBeSearched;
		// RETAIL: Tron's (+0x7d; saved after m_bCanBeSearched). Set by
		// CAI::UpdateUserFlagCanActivate and Body::Init. Not yet identified
		bool	m_bTronUnknown7D;

		// RETAIL: Tron's copies of the model's OBBs and the skeleton node
		// of each (+0x80..+0x88; InitOBBs, retail 0x10138a40). Names ours
		uint32		m_nNumOBBs;
		ModelOBB*	m_pOBBs;
		ModelNode*	m_pOBBNodes;

		void InitOBBs();
		
		void CreateNodeRadiusModels();
		void RemoveNodeRadiusModels();
		void UpdateNodeRadiusModels();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);
};


#endif  // __CHARACTER_HIT_BOX_H__
