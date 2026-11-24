// ----------------------------------------------------------------------- //
//
// MODULE  : Body.h
//
// PURPOSE : Body Prop - Definition
//
// CREATED : 1997 (was BodyProp)
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_PROP_H__
#define __BODY_PROP_H__

enum  EnumAIStateType;
class CAttachments;
class CBodyState;
class CSearchable;

#include "Character.h"
#include "Prop.h"
#include "ModelButeMgr.h"
#include "SFXMsgIDs.h"
#include "Animator.h"
#include "DeathScene.h"
#include "GeneralInventory.h"
#include "IHitBoxUser.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( Body );

// RETAIL: Tron's is 0x14 bytes (CCharacter::SetupBody 0x10091320, Body::Init
// 0x1015a520): it has no revive flag. Its last member is the character's
// core dump object (CCharacter +0xcf0), which Body::Init keeps at Body
// +0x109c. The name is ours.
struct BODYINITSTRUCT
{
	CCharacter*		pCharacter;
	EnumAIStateType	eBodyState;
	bool			bPermanentBody;
	float			fBodyLifetime;
	HOBJECT			hCoreDump;
};

class CAnimatorBody : public CAnimator
{
	public :

		// Ctors/Dtors/etc

		void Init(HOBJECT hObject);

		// Updates

		void Update();

		// Methods

		void Twitch();

	protected :

		AniTracker			m_eAniTrackerTwitch;	// Our twitch ani tracker
		Ani					m_eAniTwitch;			// Our twitch ani
};

class Body : public Prop, public IHitBoxUser
{
	typedef Prop super;


	public : // Public methods

		// Ctors/Dtors/etc

 		Body();
		~Body();

		void Init(const BODYINITSTRUCT& bi);

		CBodyState* AI_FACTORY_NEW_State(EnumAIStateType eStateType);
		void SetState(EnumAIStateType eBodyState, LTBOOL bLoad = LTFALSE);
		EnumAIStateType GetBodyState() {return m_eBodyState;}

		void FacePos(const LTVector& vTargetPos);
		void FaceDir(const LTVector& vTargetDir);

		ModelId			GetModelId() const { return m_eModelId; }
		ModelSkeleton	GetModelSkeleton() const { return m_eModelSkeleton; }

		void			AddWeapon(HOBJECT hWeapon, char *pszPosition);
		bool			AddPickupItem(HOBJECT hItem, bool bForcePickup = false);

		void			AddSpear(HOBJECT hSpear, const LTRotation& rRot, ModelNode eModelNode=eModelNodeInvalid);
		void			SetModelNodeLastHit(ModelNode eModelNodeLastHit)	{ m_eModelNodeLastHit = eModelNodeLastHit; }
		ModelNode		GetModelNodeLastHit()	const	{ return m_eModelNodeLastHit; }

		CAttachments*	GetAttachments() { return m_pAttachments; }

		HOBJECT			GetHitBox()			const { return m_hHitBox; }

		LTFLOAT			GetLifetime() { return m_fLifetime; }
		LTFLOAT			GetStarttime() { return m_fStartTime; }
		bool			GetFadeAfterLifetime() { return m_bFadeAfterLifetime; }

		HOBJECT			GetLastDamager() const { return m_damage.GetLastDamager(); }

		const LTVector&	GetDeathDir() const { return m_vDeathDir; }

		void			ReleasePowerups(bool bWeaponsOnly = false); 
		void			RemovePowerupObjects(); 

		bool			IsPermanentBody() const { return m_bPermanentBody; }

		// for game types where a players body may sit aound indefinitely while waiting to respawn

		virtual LTBOOL UsingHitDetection() const { return LTTRUE; }

		virtual float GetNodeRadius( ModelSkeleton eModelSkeleton, ModelNode eModelNode ) { return ( g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eModelNode) ); }

		HOBJECT			GetCharacter( ) const { return m_hCharacter; }



		LTBOOL			CanCheckPulse();
		bool			CanBeSearched(); 

		void			AddToObjectList( ObjectList *pObjList, eObjListControl eControl );
		virtual HATTACHMENT GetAttachment() const { return NULL; }
		virtual LTFLOAT		ComputeDamageModifier(ModelNode eModelNode) { return 1.0f; }

		// Checker

		void SetChecker(HOBJECT hChecker);
		LTBOOL HasChecker() const { return !!m_hChecker; }
		HOBJECT GetChecker() const { return m_hChecker; }

		void	SetBodyResetTime( LTFLOAT fResetTime ) { m_fBodyResetTime = fResetTime; } 
		LTFLOAT GetBodyResetTime() { return m_fBodyResetTime; } 

		// General inventory accessor
		// RETAIL: Tron's Body has no general inventory (no list in the
		// constructor 0x10158d00; m_fEnergy follows m_bDimsUpdated)
		float GetEnergy() { return m_fEnergy; }

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Carrying

		virtual void SetCanCarry( bool bCarry );
		virtual void SetBeingCarried( bool bBeingCarried );
		virtual bool BeingCarried() { return m_bBeingCarried; }
		// RETAIL: Tron's (0x1015cd90)
		void ShowCarrier();

		// RETAIL: inline; CBodyStatePranksterImplode::Init (0x1021c5b0)
		// reads them. Names ours.
		float		GetTronUnknown10A4() const { return m_fTronUnknown10A4; }
		float		GetTronUnknown10A8() const { return m_fTronUnknown10A8; }
		LTVector	GetTronUnknown10AC() const { return m_vTronUnknown10AC; }

		// Hide the body and it's associated objects.
		void		HideBody( bool bHide );

		// RETAIL: IHitBoxUser's slot 8 (retail 0x101590e0)
		virtual HOBJECT GetHitBoxObject() const { return m_hHitBox; }
		// RETAIL: IHitBoxUser's (retail 0x1001b900)
		virtual HOBJECT TronUnknownVirtual28() { return m_hObject; }

		// Get the complete list of all bodies.
		typedef std::vector< Body* > BodyList;
		static BodyList const& GetBodyList( ) { return m_lstBodies; }


	protected : // Protected methods


		// Engine methods

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);
		virtual uint32 OnDeactivate();

		// Hitbox message
		void CreateHitBox(const BODYINITSTRUCT& bi);
		void UpdateHitBox();
		void UpdateClientHitBox();

		// Handling methods

		void HandleDamage(const DamageStruct& damage);
		void HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir,	LTVector& vFrom, ModelNode& eModelNode) {}

		virtual void HandleDestroy(HOBJECT hDamager);

		void ReadProp(ObjectCreateStruct *pData);

		uint32 Update(void *pData, LTFLOAT fData);

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void CreateDeathScene(CCharacter *pChar);

		void ReleasePowerup(HOBJECT hPowerup);

		// Keeps body density down within a radius.
		void CapNumberOfBodies( );

		void CreateSpecialFX(BODYCREATESTRUCT& bcs);

	protected :

		enum Constants
		{
			kMaxSpears = 32,
			kMaxWeapons = 2,
		};

	private:

		// Returns true when it shouldn't be passed to parent.
		bool HandleModelString(ArgList* pArgList);

	protected :

		CAttachments*		m_pAttachments;
		CharacterDeath		m_eDeathType;
		ModelId				m_eModelId;
		ModelSkeleton		m_eModelSkeleton;
		ModelNode			m_eModelNodeLastHit;
		LTBOOL				m_bFirstUpdate;
		LTFLOAT				m_fStartTime;
		LTFLOAT				m_fLifetime;
		bool				m_bFadeAfterLifetime;
		LTVector			m_vColor;
		LTVector			m_vDeathDir;
		DamageType			m_eDamageType;
		LTObjRef			m_hHitBox;
		LTObjRef			m_hTronUnknown894;	// RETAIL: Tron's (constructor 0x10158db4; not saved)

		uint32				m_cWeapons;
		LTObjRefNotifier	m_ahWeapons[kMaxWeapons];

		LTObjRef			m_hChecker;

		LTFLOAT				m_fBodyResetTime;

		EnumAIStateType		m_eBodyState;
		EnumAIStateType		m_eBodyStatePrevious;
		CBodyState*			m_pState;
		CAnimatorBody		m_Animator;

		uint32				m_cSpears;
		LTObjRefNotifier	m_ahSpears[kMaxSpears];

		CDeathScene			m_DeathScene;

		CSearchable*		m_pSearch;

		EnumAIStimulusID	m_eDeathStimID;	// Registration ID of dead body stimulus.

		bool				m_bCanBeCarried;

		bool				m_bUpdateHitBox;
		bool				m_bDimsUpdated;

		float				m_fEnergy;	
		
		LTObjRef			m_hCharacter;	// The character our body spawned from

		// Never remove this body through body density capping.
		bool				m_bPermanentBody;

		static BodyList		m_lstBodies;

		// RETAIL: Tron's tail (constructor 0x10158d00, Save 0x1015c380:
		// 0x10b8 bytes). The carrier is a plain HOBJECT at 0x109c (taken
		// to be NOLF2's m_hCarrier; Body::Init sets it to BODYINITSTRUCT's
		// core dump, 0x1015a5a9, and SetBeingCarried does not set it). The
		// two floats and the vector after the carry flags are Tron's, not
		// yet identified.
		HOBJECT				m_hCarrier;
		bool				m_bCanCarry;
		bool				m_bBeingCarried;
		float				m_fTronUnknown10A4;
		float				m_fTronUnknown10A8;
		LTVector			m_vTronUnknown10AC;
		bool				m_bTronUnknown10B8;	// set true by the constructor
};

#endif // __BODY_PROP_H__




