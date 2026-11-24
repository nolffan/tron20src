// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AI.h"
#include "AITarget.h"
#include "AISenseRecorderAbstract.h"
#include "AIUtils.h"
#include "AIBrain.h"
#include "ObjectRelationMgr.h" 
#include "CharacterMgr.h" 
#include "WeaponFireInfo.h"
#include "Weapon.h"

// RETAIL: retail's unit starts with the two target commands' factory
// entries (0x101dec80, 0x101dee30) and the +0x1010 object's code.
DEFINE_AI_FACTORY_CLASS(RefreshCacheIfSame);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AbstractTarget::TakeCache
//
//	PURPOSE:	RETAIL: inline in RefreshCacheIfSame::Execute
//				(0x101dece0). Name ours.
//
// ----------------------------------------------------------------------- //

void AbstractTarget::TakeCache(AbstractTarget* pTarget)
{
	m_hObject = pTarget->m_hObject;
	m_bCanUpdateVisibility = pTarget->m_bCanUpdateVisibility;
	m_hVisionBlocker = pTarget->m_hVisionBlocker;
	m_bVisibleFromEye = pTarget->m_bVisibleFromEye;
	m_bVisibleFromWeapon = pTarget->m_bVisibleFromWeapon;
	m_fTargetTime = pTarget->m_fTargetTime;
	m_fCurMovementInaccuracy = pTarget->m_fCurMovementInaccuracy;
	m_vTargetVelocity = pTarget->m_vTargetVelocity;
	m_vTargetPosition = pTarget->m_vTargetPosition;
	m_fTargetDistSqr = pTarget->m_fTargetDistSqr;
	m_vVisiblePosition = pTarget->m_vVisiblePosition;
	m_hVisionBlocker = pTarget->m_hVisionBlocker;
	m_bVisibleFromEye = pTarget->m_bVisibleFromEye;
	m_bVisibleFromWeapon = pTarget->m_bVisibleFromWeapon;
	m_nPhase = pTarget->m_nPhase;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefreshCacheIfSame::Execute
//
//	PURPOSE:	RETAIL: 0x101dece0: takes the cache of pTarget.
//
// ----------------------------------------------------------------------- //

int RefreshCacheIfSame::Execute(AbstractTarget* pTarget)
{
	if( !pTarget->m_bCanUpdateVisibility &&
		m_pTarget->m_bCanUpdateVisibility &&
		( m_pTarget->m_hObject == pTarget->m_hObject ) &&
		( pTarget != m_pTarget ) )
	{
		m_pTarget->TakeCache( pTarget );
	}

	return 0;
}

DEFINE_AI_FACTORY_CLASS(SetAllowUpdate);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITronUnknown1010::Flush
//
//	PURPOSE:	RETAIL: 0x101dee80: sends each posted command to every
//				receiver, then gives it back to the factory.
//
// ----------------------------------------------------------------------- //

void CAITronUnknown1010::Flush()
{
	std::list<ITargetCommand*> lstCommands( m_lstTronUnknown0 );
	m_lstTronUnknown0.clear();

	while( !lstCommands.empty() )
	{
		ITargetCommand* pCommand = lstCommands.front();
		lstCommands.pop_front();

		std::for_each( m_lstTronUnknown4.begin(), m_lstTronUnknown4.end(),
			std::bind2nd( std::mem_fun( &CTargetCommandReceiver<AbstractTarget>::Receive ), pCommand ) );

		AI_FACTORY_DELETE( pCommand );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITronUnknown1010::PostCommand
//
//	PURPOSE:	RETAIL: 0x101df000: keeps a copy of the command for Flush.
//
// ----------------------------------------------------------------------- //

void CAITronUnknown1010::PostCommand(const ITargetCommand& Command)
{
	ITargetCommand* pCommand = Command.Clone();
	m_lstTronUnknown0.push_back( pCommand );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITronUnknown1010::RefreshTargets
//
//	PURPOSE:	RETAIL: 0x101df040: a target that must update its
//				visibility asks the other targets for their cache first.
//
// ----------------------------------------------------------------------- //

void CAITronUnknown1010::RefreshTargets(AbstractTarget* pTarget)
{
	if( pTarget->m_bCanUpdateVisibility )
	{
		RefreshCacheIfSame Command( pTarget );
		std::for_each( m_lstTronUnknown4.begin(), m_lstTronUnknown4.end(),
			std::bind2nd( std::mem_fun( &CTargetCommandReceiver<AbstractTarget>::Receive ), &Command ) );
	}
}



// RETAIL: Tron's phase limits are variables in the data section
// (0x1030dd10, 0x1030dd14), not NOLF2's MAX_PHASE. The names are ours.
static int		s_nMaxPhase = 7;
static float	s_fPhaseHeightTrim = 0.3f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Constructor/Destructor
//
//	PURPOSE:	Factory con/destructor
//
// ----------------------------------------------------------------------- //

AbstractTarget::AbstractTarget() :
	m_hObject( LTNULL ),
	m_bCanUpdateVisibility( LTTRUE ),
	m_bVisibleFromEye( LTFALSE ),
	m_bVisibleFromWeapon( LTFALSE ),
	m_hVisionBlocker( LTNULL ),
	m_iHit( 0 ),
	m_iMiss( 0 ),
	m_cHits( 0 ),
	m_cMisses( 0 ),
	m_fCurMovementInaccuracy( 0.f ),
	m_fTargetTime( 0.f ),
	m_fTargetDistSqr( 0.f ),
	m_bAttacking( LTFALSE ),
	m_nPhase( 2 ),
	m_nResetPhase( 2 ),
	m_fPhaseStep( 0.f ),
	m_fTronUnknown98( 0.f )
{
	VEC_INIT(m_vTargetVelocity);
	VEC_INIT(m_vTargetPosition);
	VEC_INIT(m_vVisiblePosition);

	m_hRelationNotifier.SetObserver( this );
}

// RETAIL: 0x101df1e0 (the receiver leaves its registry's list).
AbstractTarget::~AbstractTarget()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Init
//
//	PURPOSE:	Intialized targeting for an AI.
//
// ----------------------------------------------------------------------- //

void AbstractTarget::Init()
{
	// RETAIL: Tron's asks its AI through IAISensing (0x101df2e9): the slot
	// that returns the relation manager (see CAI::GetSenseRelationSet).
	m_hRelationNotifier.SetSubject( (CObjectRelationMgr*)&GetAI()->GetSenseRelationSet() );

	// RETAIL: Tron's (0x101df2e0).
	m_Receiver.SetOwner( this );
	m_Receiver.SetCallback( &AbstractTarget::ReceiveCommand );
	m_Receiver.SetRegistry( GetAI()->GetTronUnknown1010() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Save
//
//	PURPOSE:	Saves our data
//
// ----------------------------------------------------------------------- //

void AbstractTarget::Save(ILTMessage_Write *pMsg)
{

	SAVE_BOOL(m_bCanUpdateVisibility);
	SAVE_BOOL(m_bVisibleFromEye);
	SAVE_BOOL(m_bVisibleFromWeapon);
	SAVE_HOBJECT(m_hObject);
	SAVE_HOBJECT(m_hVisionBlocker);

	SAVE_DWORD(m_iHit);
	SAVE_DWORD(m_iMiss);
	SAVE_DWORD(m_cHits);
	SAVE_DWORD(m_cMisses);
	SAVE_FLOAT(m_fCurMovementInaccuracy);

	SAVE_TIME(m_fTargetTime);
	SAVE_VECTOR(m_vTargetVelocity);
	SAVE_VECTOR(m_vTargetPosition);
	SAVE_VECTOR(m_vVisiblePosition);
	SAVE_VECTOR(m_vPosition);
	SAVE_FLOAT(m_fTargetDistSqr);
	// RETAIL: written inline as data (0x101df4fc), unlike the vectors above.
	pMsg->WriteType(m_vTargetDims);
	SAVE_BOOL(m_bAttacking);
	SAVE_INT(m_nPhase);
	SAVE_INT(m_nResetPhase);
	SAVE_FLOAT(m_fPhaseStep);
	SAVE_FLOAT(m_fTronUnknown98);
	pMsg->WriteType(m_vTronUnknown9C);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Load
//
//	PURPOSE:	Loads our data
//
// ----------------------------------------------------------------------- //

void AbstractTarget::Load(ILTMessage_Read *pMsg)
{

	LOAD_BOOL(m_bCanUpdateVisibility);
	LOAD_BOOL(m_bVisibleFromEye);
	LOAD_BOOL(m_bVisibleFromWeapon);
	LOAD_HOBJECT(m_hObject);
	LOAD_HOBJECT(m_hVisionBlocker);

	LOAD_DWORD(m_iHit);
	LOAD_DWORD(m_iMiss);
	LOAD_DWORD(m_cHits);
	LOAD_DWORD(m_cMisses);
	LOAD_FLOAT(m_fCurMovementInaccuracy);

	LOAD_TIME(m_fTargetTime);
	LOAD_VECTOR(m_vTargetVelocity);
	LOAD_VECTOR(m_vTargetPosition);
	LOAD_VECTOR(m_vVisiblePosition);
	LOAD_VECTOR(m_vPosition);
	LOAD_FLOAT(m_fTargetDistSqr);
	LOAD_VECTOR(m_vTargetDims);
	LOAD_BOOL(m_bAttacking);
	LOAD_INT(m_nPhase);
	LOAD_INT(m_nResetPhase);
	LOAD_FLOAT(m_fPhaseStep);
	LOAD_FLOAT(m_fTronUnknown98);
	LOAD_VECTOR(m_vTronUnknown9C);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AbstractTarget::ReceiveCommand
//
//	PURPOSE:	RETAIL: 0x101df7c0: the receiver's callback.
//
// ----------------------------------------------------------------------- //

int AbstractTarget::ReceiveCommand(ITargetCommand* pCommand)
{
	pCommand->Execute( this );
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::SetObject
//
//	PURPOSE:	Set target object.
//
// ----------------------------------------------------------------------- //

void AbstractTarget::SetObject(HOBJECT hObject)
{
	// HACK:  for TO2 to make AI shutup after killing someone one co-op.

	if( IsPlayer( m_hObject ) )
	{	
		CCharacter* pPlayer = (CCharacter*)g_pLTServer->HandleToObject( m_hObject );
		if( pPlayer->IsDead() )
		{
			GetAI()->MuteAISounds( LTTRUE );
		}
	}

	// END HACK

	m_hObject = hObject;
	if( m_hObject )
	{
		if( IsPlayer( m_hObject ) )
		{
			CCharacter* pPlayer = (CCharacter*)g_pLTServer->HandleToObject( m_hObject );
			if( !pPlayer->IsDead() )
			{
				GetAI()->MuteAISounds( LTFALSE );
			}
		}

		// Re-initialize target tracking data.

		m_fTargetTime = g_pLTServer->GetTime();
		g_pLTServer->GetObjectPos( m_hObject, &m_vTargetPosition );
		m_fCurMovementInaccuracy = GetAI()->GetMaxMovementAccuracyPerturb();
		m_vTargetVelocity = LTVector( 0.f, 0.f, 0.f );

		g_pPhysicsLT->GetObjectDims( m_hObject, &m_vTargetDims );
		m_fPhaseStep = ( ( 2.f - s_fPhaseHeightTrim ) * m_vTargetDims.y ) / ( s_nMaxPhase + 1 );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::GetShootPosition
//
//	PURPOSE:	Get shoot position, including some random
//				inaccuracy due to target movement.
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (0x101df910) does not test the pointer and returns the
// position. It refreshes the target first, and it keeps the last position
// (m_vTronUnknown9C) for a tenth of a second (m_fTronUnknown98).

const LTVector& AbstractTarget::GetShootPosition(LTVector* pvShootPos)
{
	Refresh();

	// Initially aim for the target's visible position.

	*pvShootPos = m_vVisiblePosition;

	// If Target is within the FullAccuracy radius, we are done.

	if( m_fTargetDistSqr < GetAI()->GetFullAccuracyRadiusSqr() )
	{
		m_vTronUnknown9C = *pvShootPos;
		return *pvShootPos;
	}

	if( g_pLTServer->GetTime() < m_fTronUnknown98 )
	{
		*pvShootPos = m_vTronUnknown9C;
		return *pvShootPos;
	}

	m_fTronUnknown98 = g_pLTServer->GetTime() + 0.1f;


	// The following code forces the AI to intenionally miss every x
	// number of shots, depending on their accuracy. This gives players
	// the excitement of getting shot at without killing them too fast.

	// For example, if accuracy = 0.5 there will be a guaranteed sequence
	// of HIT, MISS, HIT, MISS, ...
	// If accuracy = 0.25, then HIT, MISS, MISS, MISS, HIT, MISS, MISS, MISS, etc.
	// If accuracy = 0.75, then HIT, HIT, HIT, MISS, HIT, HIT, HIT, MISS, etc.

	// Calculate the ratio of hits to misses based on the current 
	// accuracy.  This needs to be recalculated for every shot, 
	// because accuracy may change at any time.

	LTFLOAT fAccuracy = GetAI()->GetAccuracy();
	if( fAccuracy <= 0.f )
	{
		m_cMisses = 1;
		m_cHits = 0;
	}
	else if( fAccuracy >= 1.f )
	{
		m_cMisses = 0;
		m_cHits = 1;
	}
	else if( fAccuracy < 0.5f )
	{
		m_cMisses = (uint32)( ( ( 1.f - fAccuracy ) / fAccuracy ) + 0.5f );
		m_cHits = 1;
	}
	else {
		m_cMisses = 1;
		m_cHits = (uint32)( ( fAccuracy / ( 1.f - fAccuracy ) ) + 0.5f );
	}

	// If we have met or exceeded the required number of misses, 
	// reset the counters.

	if( m_iMiss >= m_cMisses )
	{
		m_iHit = 0;
		m_iMiss = 0;
	}

	//
	// First take care of hits, then take care of misses.
	//

	// Hit.

	if( m_iHit < m_cHits )
	{
		++m_iHit;

		// If target has started moving or change directions recently,
		// factor in some inaccuracy.

		LTFLOAT fInnaccuracy = Max( 0.f, m_fCurMovementInaccuracy );
		if( fInnaccuracy > 0.f )
		{
			LTVector vShootOffset = LTVector(	GetRandom( -fInnaccuracy, fInnaccuracy ),
												GetRandom( -fInnaccuracy * 0.5f, fInnaccuracy * 0.5f ),
												GetRandom( -fInnaccuracy, fInnaccuracy ) );
			vShootOffset.Normalize();

			*pvShootPos += vShootOffset * 100.0f;
		}
	}

	// Miss.

	else {
		++m_iMiss;

		// Calculate a position to the right or left of the target.

		LTVector vDir = m_vVisiblePosition - GetAI()->GetPosition();
		vDir.Normalize();

		LTVector vRight = vDir.Cross( LTVector( 0.f, 1.f, 0.f ) );

		if( GetRandom( -1.f, 1.f ) > 0.f )
		{
			vRight *= GetAI()->GetAccuracyMissPerturb();
		}
		else {
			vRight *= -GetAI()->GetAccuracyMissPerturb();
		}
		vRight.y = GetRandom( -5.f, 15.f );

		// Apply the offset to miss the target.

		*pvShootPos += vRight;
	}

	m_vTronUnknown9C = *pvShootPos;
	return *pvShootPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AbstractTarget::Refresh
//
//	PURPOSE:	RETAIL: 0x101dfd00 (Tron's; name ours).
//
// ----------------------------------------------------------------------- //

void AbstractTarget::Refresh()
{
	if( GetAI() && m_hObject )
	{
		GetAI()->GetTronUnknown1010()->RefreshTargets( this );

		if( m_bCanUpdateVisibility )
		{
			UpdateVisibility();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdateVisibility
//
//	PURPOSE:	Updates the target's visibility
//
// ----------------------------------------------------------------------- //

void AbstractTarget::UpdateVisibility()
{
	_ASSERT(IsValid());

	// Do not update visibility again until something external says it's OK.
	// AI sets this to TRUE when it has had a sense update.

	if( !m_bCanUpdateVisibility )
	{
		return;
	}
	m_bCanUpdateVisibility = LTFALSE;

	// Do not see dead targets.
	// This is important for AI behavior in multiplayer, where a 
	// dead players stick around so the HOBJECTs do not get NULLed.

	if( IsCharacter( m_hObject ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( m_hObject );
		if( pChar->IsDead() )
		{
			// HACK:  for TO2 to make AI shutup after killing someone one co-op.

			if( IsPlayer( m_hObject ) )
			{
				GetAI()->MuteAISounds( LTTRUE );
			}
			
			// END HACK

			// Invalidate a dead target.

			m_hObject = LTNULL;
			m_hVisionBlocker = LTNULL;
			SetVisibleFromEye( LTFALSE );
			SetVisibleFromWeapon( LTFALSE );
			return;
		}
	}

	LTVector vPosition;
	g_pLTServer->GetObjectPos( m_hObject, &vPosition );

	//
	// Predict the target's next position based on target's observed velocity.
	//

	LTFLOAT fCurTime = g_pLTServer->GetTime();
	LTFLOAT fTimeDelta = fCurTime - m_fTargetTime;
	m_fTargetTime = fCurTime;

	if( fTimeDelta > 0.f )
	{
		LTVector vPredictedPos = m_vTargetPosition + ( m_vTargetVelocity * fTimeDelta);
		LTVector vDiff = vPosition - vPredictedPos;
		
		LTFLOAT fInaccuracyDist = Min( vDiff.Mag(), GetAI()->GetMaxMovementAccuracyPerturb() );
		m_fCurMovementInaccuracy = Max( fInaccuracyDist, m_fCurMovementInaccuracy - ( GetAI()->GetMovementAccuracyPerturbDecay() * fTimeDelta ) );

		m_vTargetVelocity = ( vPosition - m_vTargetPosition ) / fTimeDelta;
	}

	m_vTargetPosition = vPosition;

	// Keep track of the distance to the target.

	m_fTargetDistSqr = GetAI()->GetPosition().DistSqr( m_vTargetPosition );

	// Incrementally scan the target vertically until he can be seen.

	if ( m_nPhase == m_nResetPhase )
	{
		m_vVisiblePosition = vPosition;
	}

	vPosition.y += ( m_vTargetDims.y - ( LTFLOAT(m_nPhase) * m_fPhaseStep ) );

    LTFLOAT fSeeEnemyDistanceSqr = GetAI()->GetSenseRecorder()->GetSenseDistanceSqr(kSense_SeeEnemy);

	LTBOOL bTargetVisibleFromEye = ( GetAI()->GetSenseRecorder()->HasFullStimulation(kSense_SeeEnemy) || 
									 GetAI()->GetSenseRecorder()->HasFullStimulation(kSense_SeeEnemyLean) );
	LTBOOL bTargetVisibleFromWeapon = LTFALSE;
	HOBJECT hVisionBlocker = LTNULL;

	if( bTargetVisibleFromEye )
	{
		// Check visibility from a position dependent on the current weapon.
		// Primary weapon is on the right side, secondary on the left.
		// If no weapon, check from the eyes.

		// RETAIL: Tron's AI gives the position (CAI::GetWeaponCheckPosition,
		// 0x101311f0); it keeps its weapon choice while the target stays
		// visible from the weapon.

		LTVector vCheckPos = GetAI()->GetWeaponCheckPosition( m_bVisibleFromWeapon );

		// Check visibility.

		if ( !GetAI()->CanShootThrough() )
		{
			if ( GetAI()->IsObjectPositionVisible(CAI::DefaultFilterFn, NULL, vCheckPos, m_hObject, vPosition, fSeeEnemyDistanceSqr, !GetAI()->IsAlert(), LTFALSE, &hVisionBlocker ) )
			{
				bTargetVisibleFromWeapon = LTTRUE;
			}
		}
		else
		{
			if( GetAI()->IsObjectPositionVisible(CAI::ShootThroughFilterFn, CAI::ShootThroughPolyFilterFn, vCheckPos, m_hObject, vPosition, fSeeEnemyDistanceSqr, !GetAI()->IsAlert(), LTTRUE, &hVisionBlocker ) )
			{
				CAI* pTargetAI = LTNULL;
				if( IsAI( m_hObject ) )
				{
					pTargetAI = (CAI*)g_pLTServer->HandleToObject( m_hObject );
				}

				CAI* pVisionBlocker = LTNULL;
				if( !g_pCharacterMgr->RayIntersectAI( vCheckPos, vPosition, GetAI(), pTargetAI, &pVisionBlocker ) )
				{
					bTargetVisibleFromWeapon = LTTRUE;
				}
				else if( pVisionBlocker )
				{
					hVisionBlocker = pVisionBlocker->m_hObject;
				}
			}

			// Ignore AI that are not currently client-solid.
			// (e.g. AI that are knocked out).

			else if( IsAI( hVisionBlocker ) )
			{
				uint32 dwFlags;
				g_pCommonLT->GetObjectFlags( hVisionBlocker, OFT_User, dwFlags);
				if( ! ( dwFlags & USRFLG_AI_CLIENT_SOLID ) )
				{
					hVisionBlocker = LTNULL;
				}
			}
		}
	}

	if( bTargetVisibleFromWeapon )
	{
		m_vVisiblePosition = vPosition;
	
		m_hVisionBlocker = LTNULL;
		SetVisibleFromEye( bTargetVisibleFromEye );
		SetVisibleFromWeapon( bTargetVisibleFromWeapon );
	}
	else {
		SetVisibleFromEye( bTargetVisibleFromEye );
		SetVisibleFromWeapon( bTargetVisibleFromWeapon );
		m_hVisionBlocker = hVisionBlocker;

		// Only change the phase if target is not visible.

		m_nPhase = (m_nPhase+1) & s_nMaxPhase;
	}
}

/*virtual*/ int AbstractTarget::OnRelationChange(HOBJECT hObj)
{
	if( hObj == m_hObject.operator HOBJECT() )
		m_hObject = LTNULL;

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AbstractTarget::GetCharacter
//
//	PURPOSE:	RETAIL: 0x101e02c0 (inline in NOLF2).
//
// ----------------------------------------------------------------------- //

CCharacter* AbstractTarget::GetCharacter() const
{
	return dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject( m_hObject ) );
}

// RETAIL: CAITarget's part of the unit follows AbstractTarget's (its factory
// entry 0x101e0300, constructor 0x101e03b0).
DEFINE_AI_FACTORY_CLASS(CAITarget);

CAITarget::CAITarget() :
	m_pAI( LTNULL ),
	m_fPushSpeed( 0.f ),
	m_fPushMinDist( 0.f ),
	m_fPushMinDistSqr( 0.f ),
	m_fPushThreshold( 0.f )
{
}

// RETAIL: 0x101e04c0.
void CAITarget::Save(ILTMessage_Write *pMsg)
{
	AbstractTarget::Save(pMsg);

	SAVE_FLOAT(m_fPushSpeed);
	SAVE_FLOAT(m_fPushMinDist);
	SAVE_FLOAT(m_fPushMinDistSqr);
	SAVE_FLOAT(m_fPushThreshold);
}

// RETAIL: 0x101e0530.
void CAITarget::Load(ILTMessage_Read *pMsg)
{
	AbstractTarget::Load(pMsg);

	LOAD_FLOAT(m_fPushSpeed);
	LOAD_FLOAT(m_fPushMinDist);
	LOAD_FLOAT(m_fPushMinDistSqr);
	LOAD_FLOAT(m_fPushThreshold);
}

// RETAIL: 0x101e05a0.
void CAITarget::Init(CAI* pAI)
{
	m_pAI = pAI;

	AbstractTarget::Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdatePush
//
//	PURPOSE:	Push the target back if it is too close.
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (0x101e05c0): the AI pushes only players, and its Tron
// weapon (CAI+0xd60) fires at the target.
void CAITarget::UpdatePush( CCharacter* pChar )
{
	if( !pChar )
	{
		return;
	}

	Refresh();

	if( m_fPushSpeed > 0.f )
	{
		// Handle collisions by pushing the target back.

		LTVector vPos = GetAI()->GetPosition();
		if( vPos.DistSqr( GetVisiblePosition() ) < m_fPushMinDistSqr )
		{
			if( IsPlayer( pChar->GetUserObject() ) )
			{
				pChar->PushCharacter(vPos, m_fPushMinDist + m_fPushThreshold, 0.f, GetAI()->GetSenseUpdateRate(), m_fPushSpeed );
			}

			if( GetAI()->GetTronWeapon() )
			{
				WeaponFireInfo weaponFireInfo;
				weaponFireInfo.hFiredFrom = m_pAI->m_hObject;
				weaponFireInfo.hTestObj = g_pLTServer->ObjectToHandle( pChar );

				LTVector vTargetPos;
				g_pLTServer->GetObjectPos( weaponFireInfo.hTestObj, &vTargetPos );

				weaponFireInfo.vPath = vTargetPos - vPos;
				weaponFireInfo.vPath.Normalize();
				weaponFireInfo.vFirePos = vPos;
				weaponFireInfo.vFlashPos = vPos;

				GetAI()->GetTronWeapon()->UpdateWeapon( weaponFireInfo, LTTRUE );
			}
		}
	}
}


