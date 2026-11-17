// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_TARGET_H__
#define __AI_TARGET_H__

#include "AIClassFactory.h"
#include "LTObjRef.h"
#include "RelationChangeObserver.h"
#include <list>
#include <algorithm>

class CAI;
class CCharacter;
class AbstractTarget;
class CAITronUnknown1010;

// RETAIL: ITargetCommand (RTTI; no vtable of its own in retail): a command
// that the AI's +0x1010 object (CAITronUnknown1010, AI.h) sends to each of
// its AI's targets. Execute is slot 3, Clone slot 4 (0x101dedf0,
// 0x1012fe90). Slot 2 returns 0 in RefreshCacheIfSame and 1 in
// SetAllowUpdate; nothing in this unit calls it. Names other than the
// class names are ours.

struct ITargetCommand : public CAIClassAbstract
{
	virtual int TronUnknownVirtual2() = 0;
	virtual int Execute(AbstractTarget* pTarget) = 0;
	virtual ITargetCommand* Clone() const = 0;
};

// RETAIL: a target's registration with its AI's +0x1010 object (at
// AbstractTarget+0xb0): the owner, the owner's callback and the registry.
// It has no RTTI; the names are ours. The destructor is inline in
// ~AbstractTarget (0x101df1e0), SetRegistry in AbstractTarget::Init
// (0x101df2e0), Receive is 0x101e08b0. The registry's class is complete
// only where the template is used (AITarget.cpp includes AI.h).

template<class T>
class CTargetCommandReceiver
{
	public :

		typedef int (T::*Callback)(ITargetCommand* pCommand);

		CTargetCommandReceiver() :
			m_pOwner( LTNULL ),
			m_pfnCallback( LTNULL ),
			m_pRegistry( LTNULL )
		{
		}

		~CTargetCommandReceiver()
		{
			if( m_pRegistry )
			{
				std::list<CTargetCommandReceiver<T>*>& lstReceivers = m_pRegistry->m_lstTronUnknown4;
				CTargetCommandReceiver<T>* pThis = this;
				if( std::find( lstReceivers.begin(), lstReceivers.end(), pThis ) != lstReceivers.end() )
				{
					lstReceivers.remove( pThis );
				}
			}
		}

		void SetOwner(T* pOwner)
		{
			if( pOwner && !m_pOwner )
			{
				m_pOwner = pOwner;
			}
		}

		void SetCallback(Callback pfnCallback)
		{
			if( pfnCallback )
			{
				m_pfnCallback = pfnCallback;
			}
		}

		void SetRegistry(CAITronUnknown1010* pRegistry)
		{
			if( !m_pRegistry && pRegistry )
			{
				m_pRegistry = pRegistry;

				std::list<CTargetCommandReceiver<T>*>& lstReceivers = pRegistry->m_lstTronUnknown4;
				CTargetCommandReceiver<T>* pThis = this;
				if( lstReceivers.end() == std::find( lstReceivers.begin(), lstReceivers.end(), pThis ) )
				{
					lstReceivers.push_back( pThis );
				}
			}
		}

		int Receive(ITargetCommand* pCommand)
		{
			if( m_pOwner )
			{
				(m_pOwner->*m_pfnCallback)( pCommand );
			}

			return 0;
		}

	protected :

		T*					m_pOwner;
		Callback			m_pfnCallback;
		CAITronUnknown1010*	m_pRegistry;
};


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

        LTBOOL IsVisibleCompletely() { Refresh(); return IsVisibleFromEye() && IsVisibleFromWeapon(); }
        LTBOOL IsVisiblePartially() { Refresh(); return IsVisibleFromEye() || IsVisibleFromWeapon(); }
        LTBOOL IsVisibleFromEye() { Refresh(); return m_bVisibleFromEye; }
        LTBOOL IsVisibleFromWeapon() { Refresh(); return m_bVisibleFromWeapon; }

		// RETAIL: Tron's accessors first call Refresh (0x101dfd00).
		HOBJECT GetVisionBlocker() { Refresh(); return m_hVisionBlocker; }

        LTBOOL IsAttacking() const { return m_bAttacking; }

		void UpdateVisibility();

		HOBJECT GetObject() const { return m_hObject; }
		// RETAIL: out of line (0x101e02c0), a dynamic_cast.
        CCharacter* GetCharacter() const;

        const LTVector& GetVisiblePosition() { Refresh(); return m_vVisiblePosition; }
        void GetShootPosition(LTVector* pvShootPos);

		virtual int OnRelationChange(HOBJECT);

		// RETAIL: Tron's (0x101dfd00): first takes the cache of a target of
		// the same AI with the same object, then updates the visibility if
		// that is still necessary. Name ours.
		void Refresh();

	protected : // Protected methods

		friend class CAI;
		friend class CAITronUnknown1010;
		friend struct RefreshCacheIfSame;
		friend struct SetAllowUpdate;

		// RETAIL: Tron's (0x101df7c0): the receiver's callback. Name ours.
		int ReceiveCommand(ITargetCommand* pCommand);

		// RETAIL: inline in RefreshCacheIfSame::Execute. Name ours.
		inline void TakeCache(AbstractTarget* pTarget);

		virtual CAI* GetAI() = 0;

		void Init();

        LTBOOL IsValid() { Refresh(); return ( m_hObject != NULL ); }
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
		// 0x101df2e0 sets it: owner, callback 0x101df7c0, registry).
		CTargetCommandReceiver<AbstractTarget> m_Receiver;
};

// RETAIL: RefreshCacheIfSame (RTTI; vtable 0x1029b398, 8 bytes). Execute
// (0x101dece0) gives the target that refreshes (m_pTarget, which must still
// update its visibility) the cache of another target with the same object
// that has updated already.

struct RefreshCacheIfSame : public ITargetCommand
{
	DECLARE_AI_FACTORY_CLASS(RefreshCacheIfSame);

	RefreshCacheIfSame() { }
	RefreshCacheIfSame(AbstractTarget* pTarget) : m_pTarget( pTarget ) { }

	virtual int TronUnknownVirtual2() { return 0; }

	virtual int Execute(AbstractTarget* pTarget);

	virtual ITargetCommand* Clone() const
	{
		RefreshCacheIfSame* pClone = AI_FACTORY_NEW(RefreshCacheIfSame);
		pClone->m_pTarget = m_pTarget;
		return pClone;
	}

	AbstractTarget*	m_pTarget;
};

// RETAIL: SetAllowUpdate (RTTI; vtable 0x1029869c, 8 bytes): sets each
// target's m_bCanUpdateVisibility (Execute 0x1012fe70). CAI posts one in
// 0x1012fdf0.

struct SetAllowUpdate : public ITargetCommand
{
	DECLARE_AI_FACTORY_CLASS(SetAllowUpdate);

	SetAllowUpdate() { }
	SetAllowUpdate(bool bAllow) : m_bAllow( bAllow ) { }

	virtual int TronUnknownVirtual2() { return 1; }

	virtual int Execute(AbstractTarget* pTarget)
	{
		pTarget->m_bCanUpdateVisibility = m_bAllow;
		return 0;
	}

	virtual ITargetCommand* Clone() const
	{
		SetAllowUpdate* pClone = AI_FACTORY_NEW(SetAllowUpdate);
		pClone->m_bAllow = m_bAllow;
		return pClone;
	}

	bool			m_bAllow;
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