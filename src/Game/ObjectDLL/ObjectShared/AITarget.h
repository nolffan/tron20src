// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_TARGET_H__
#define __AI_TARGET_H__

#include "AIClassFactory.h"
#include "LTObjRef.h"
#include "RelationChangeObserver.h"

class CAI;
class CCharacter;


// RETAIL: Tron splits NOLF2's CAITarget: AbstractTarget (RTTI; ctor
// 0x101df0e0, vtable 0x1029b3b0: OnRelationChange, a pure virtual dtor and
// a pure GetAI) holds the target tracking, CAITarget (0x101e03b0, 0xd4
// bytes) adds its CAI and the push. CGenericTarget<CAIGoalAbstract> is
// another AbstractTarget in retail (below).

class AbstractTarget : public IRelationChangeObserver
{
	public : // Public methods

		AbstractTarget( );
		virtual ~AbstractTarget( ) = 0;

        LTBOOL IsVisibleCompletely() const { return IsVisibleFromEye() && IsVisibleFromWeapon(); }
        LTBOOL IsVisiblePartially() const { return IsVisibleFromEye() || IsVisibleFromWeapon(); }
        LTBOOL IsVisibleFromEye() const { return m_bVisibleFromEye; }
        LTBOOL IsVisibleFromWeapon() const { return m_bVisibleFromWeapon; }

		HOBJECT GetVisionBlocker() const { return m_hVisionBlocker; }

        LTBOOL IsAttacking() const { return m_bAttacking; }

		void UpdateVisibility();

		HOBJECT GetObject() const { return m_hObject; }
		// RETAIL: out of line (0x101e02c0), a dynamic_cast.
        CCharacter* GetCharacter() const;

        const LTVector& GetVisiblePosition() const { return m_vVisiblePosition; }
        void GetShootPosition(LTVector* pvShootPos);

		virtual int OnRelationChange(HOBJECT);

	protected : // Protected methods

		friend class CAI;

		virtual CAI* GetAI() = 0;

		void Init();

        LTBOOL IsValid() { return ( m_hObject != NULL ); }
        void SetCanUpdateVisibility(LTBOOL bCanUpdate) { m_bCanUpdateVisibility = bCanUpdate; }
        void SetVisibleFromEye(LTBOOL bVisibleFromEye) { m_bVisibleFromEye = bVisibleFromEye; }
        void SetVisibleFromWeapon(LTBOOL bVisibleFromWeapon) { m_bVisibleFromWeapon = bVisibleFromWeapon; }
		void SetObject(HOBJECT hObject);
        void SetVisiblePosition(const LTVector& vPosition) { m_vVisiblePosition = vPosition; }

        void SetAttacking(LTBOOL bAttacking) { m_bAttacking = bAttacking; }

        void UpdateShootPosition(const LTVector& vShootPosition, LTFLOAT fError, LTBOOL bNewError = LTTRUE);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

	protected : // Protected member variables

		LTObjRef	m_hObject;
		LTBOOL		m_bCanUpdateVisibility;
        LTBOOL		m_bVisibleFromEye;
        LTBOOL		m_bVisibleFromWeapon;
		LTObjRef	m_hVisionBlocker;
		LTVector	m_vTargetDims;
        LTVector	m_vPosition;

		uint32		m_iHit;
		uint32		m_iMiss;
		uint32		m_cHits;
		uint32		m_cMisses;
		LTFLOAT		m_fCurMovementInaccuracy;

		LTFLOAT		m_fTargetTime;
		LTVector	m_vTargetVelocity;
		LTVector	m_vTargetPosition;
        LTVector	m_vVisiblePosition;
		LTFLOAT		m_fTargetDistSqr;
        LTBOOL		m_bAttacking;
		int32		m_nPhase;
		int32		m_nResetPhase;
		LTFLOAT		m_fPhaseStep;

		// RETAIL: Tron's, saved after m_fPhaseStep (Save 0x101df3f0).
		LTFLOAT		m_fTronUnknown98;
		LTVector	m_vTronUnknown9C;

		RelationChangeNotifier m_hRelationNotifier; // Reset before set

		// RETAIL: Tron's registration with its AI's +0x1010 object (Init
		// 0x101df2e0 sets these: owner, callback 0x101df7c0, registry).
		void*		m_pTronUnknownB0;
		void*		m_pTronUnknownB4;
		void*		m_pTronUnknownB8;
};


class CAITarget : public CAIClassAbstract, public AbstractTarget
{
	public : // Public methods

		DECLARE_AI_FACTORY_CLASS(CAITarget);

		CAITarget( );

		void UpdatePush( CCharacter* pChar );

		void SetPushSpeed(LTFLOAT fPushSpeed) { m_fPushSpeed = fPushSpeed; }
		void SetPushMinDist(LTFLOAT fPushMinDist) { m_fPushMinDist = fPushMinDist; m_fPushMinDistSqr = fPushMinDist* fPushMinDist; }

	protected : // Protected methods

		friend class CAI;

		void Init(CAI* pAI);
		virtual CAI* GetAI() { return m_pAI; }

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

	private : // Private member variables

		CAI*		m_pAI;

		LTFLOAT		m_fPushSpeed;
		LTFLOAT		m_fPushMinDist;
		LTFLOAT		m_fPushMinDistSqr;
		LTFLOAT		m_fPushThreshold;
};

// RETAIL: CGenericTarget<CAIGoalAbstract> (RTTI; vtable 0x1029d678:
// OnRelationChange, dtor 0x1020ab60, GetAI 0x1020ab50) is the target a goal
// keeps itself (CAIGoalAttack +0xdc). The owner pointer is at +0xbc; the
// goal sets it in InitGoal before it calls AbstractTarget::Init. The class
// name is retail's; Init's name is ours (it is inline in retail).
template<class T>
class CGenericTarget : public AbstractTarget
{
	public :

		CGenericTarget( ) : m_pOwner( LTNULL ) { }

		void Init(T* pOwner) { m_pOwner = pOwner; AbstractTarget::Init(); }

		void Save(ILTMessage_Write *pMsg) { AbstractTarget::Save(pMsg); }
		void Load(ILTMessage_Read *pMsg) { AbstractTarget::Load(pMsg); }

	protected :

		virtual CAI* GetAI() { return m_pOwner->GetAI(); }

	protected :

		T*			m_pOwner;
};

#endif