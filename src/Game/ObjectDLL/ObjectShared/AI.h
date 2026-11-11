// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_H__
#define __AI_H__

#include "AISounds.h"
#include "AITypes.h"
#include "AISensing.h"
#include <vector>
#include <list>

// Forward declarations.
class CAIGoalMgr;
class CAIState;
class CAITarget;
class CAISense;
class TeleportPoint;
class CAISenseRecorderAbstract;
class CAIBrain;
class CAnimationContext;
class CAnimationMgr;
class CAIMovement;
class CAIPathKnowledgeMgr;
class CServerTrackedNodeContext;
struct INVALID_NODE;

class CAttachmentPosition;
class CWeapons;
class CAttachmentsPlugin;

struct CHARCREATESTRUCT;

enum  EnumAIStateType;
enum  EnumTrackedNodeGroup;
enum  EnumAnimProp;

struct DroppedWeapon
{
	DroppedWeapon() { Clear(); }
	void Clear() {hPickupItem = NULL; sAttachString = ""; }
	LTObjRefNotifier	hPickupItem;
	CString     sAttachString;
};


// RETAIL: Tron's 0x1e byte block CAI allocates in its constructor (+0x11b0)
// and fills from "NameId"/"ModelTemplate" and "TriviaId" in ReadProp; Save
// writes every word but the last. Names ours.
struct TronAIInfoWords
{
	TronAIInfoWords() { memset(this, 0, sizeof(*this)); }
	uint16	anValues[6];
};

struct TronAIInfo
{
	TronAIInfo() : nNameId(0), nTriviaId(0), nTronUnknown4(0), nTronUnknown6(0),
		nTronUnknown8(0), nTronUnknownA(0), nTronUnknownC(0), nTronUnknownE(0),
		nTronUnknown1C(0)
	{
	}

	// RETAIL: inlined in CAI::Save and CAI::Load (retail 0x101336ea,
	// 0x101344e4); every word but the last, in this order
	void Save(ILTMessage_Write *pMsg)
	{
		pMsg->Writeuint16( nNameId );
		pMsg->Writeuint16( nTronUnknown8 );
		pMsg->Writeuint16( nTronUnknownA );
		pMsg->Writeuint16( nTronUnknownC );
		pMsg->Writeuint16( nTronUnknownE );
		pMsg->Writeuint16( nTronUnknown4 );
		pMsg->Writeuint16( nTriviaId );
		pMsg->Writeuint16( nTronUnknown6 );
		pMsg->Writeuint16( Words.anValues[1] );
		pMsg->Writeuint16( Words.anValues[0] );
		pMsg->Writeuint16( Words.anValues[3] );
		pMsg->Writeuint16( Words.anValues[2] );
		pMsg->Writeuint16( Words.anValues[5] );
		pMsg->Writeuint16( Words.anValues[4] );
	}

	void Load(ILTMessage_Read *pMsg)
	{
		nNameId = pMsg->Readuint16();
		nTronUnknown8 = pMsg->Readuint16();
		nTronUnknownA = pMsg->Readuint16();
		nTronUnknownC = pMsg->Readuint16();
		nTronUnknownE = pMsg->Readuint16();
		nTronUnknown4 = pMsg->Readuint16();
		nTriviaId = pMsg->Readuint16();
		nTronUnknown6 = pMsg->Readuint16();
		Words.anValues[1] = pMsg->Readuint16();
		Words.anValues[0] = pMsg->Readuint16();
		Words.anValues[3] = pMsg->Readuint16();
		Words.anValues[2] = pMsg->Readuint16();
		Words.anValues[5] = pMsg->Readuint16();
		Words.anValues[4] = pMsg->Readuint16();
	}
	uint16			nNameId;
	uint16			nTriviaId;
	uint16			nTronUnknown4;
	uint16			nTronUnknown6;
	uint16			nTronUnknown8;
	uint16			nTronUnknownA;
	uint16			nTronUnknownC;
	uint16			nTronUnknownE;
	TronAIInfoWords	Words;
	uint16			nTronUnknown1C;
};

// RETAIL: Tron's 8 byte class at CAI+0x1010 (not written yet)
class CAITronUnknown1010;

// Defines

#define AI_MAX_WEAPONS		(4)
#define AI_MAX_OBJECTS		(32)

LINKTO_MODULE( AI );


enum EnumAIAwareness
{
	kAware_Relaxed,		// AIVolume SenseMasks enabled, narrow FOV.
	kAware_Suspicious,	// AIVolume SenseMasks disabled, narrow FOV, searches. 
	kAware_Alert,		// AIVolume SenseMasks disabled, wide FOV, searches. 
};

enum EnumAIWeaponType
{
	kAIWeap_Invalid = 0,
	kAIWeap_Ranged,
	kAIWeap_Melee,
	kAIWeap_Thrown,
};

// Classes
// RETAIL: Tron's: the 12 byte elements of CAI's vector at +0xddc, one for
// each of the brain's AttackLeft/Right/CenterMaxDist AIData (CAI::
// InitialUpdate and CAI::Load, retail 0x1012b987, 0x1013467f). Load maps
// the chosen side to m_iCurrentWeapon (left 1, right 0, center 2). Names
// ours.
enum EnumAIAttackSide
{
	kAIAttack_None = 0,
	kAIAttack_Left,
	kAIAttack_Right,
	kAIAttack_Center,
};

struct AIWeaponChoice
{
	AIWeaponChoice()
	{
		m_eSide = kAIAttack_None;
		m_fMaxDist = 0.0f;
		m_bEnabled = true;
	}
	AIWeaponChoice(EnumAIAttackSide eSide, float fMaxDist, bool bEnabled)
	{
		m_eSide = eSide;
		m_fMaxDist = fMaxDist;
		m_bEnabled = bEnabled;
	}
	EnumAIAttackSide	m_eSide;
	float				m_fMaxDist;
	bool				m_bEnabled;
};
typedef std::vector<AIWeaponChoice> AIWeaponChoiceList;

// IAISensing is an interface for sensing objects.

class CAI : public CCharacter, public IAISensing
{
	typedef CCharacter super;

	public : // Public methods

		// Ctors/Dtors/etc

		CAI();
		virtual ~CAI();

		LTBOOL IsFirstUpdate() const { return m_bFirstUpdate; }

		// IAISensing members.

		virtual HOBJECT	GetSensingObject() { return m_hObject; }
		virtual LTBOOL	IsSensing() { return !IsDead(); }
		virtual LTBOOL	IsAlert();

		virtual LTFLOAT GetSenseUpdateRate() const { return m_fSenseUpdateRate; }
		virtual LTFLOAT GetNextSenseUpdate() const { return m_fNextSenseUpdate; }
		virtual void	SetNextSenseUpdate(LTFLOAT fTime) { m_fNextSenseUpdate = fTime; }

		virtual	void	UpdateSensingMembers();

		virtual	LTBOOL	GetDoneProcessingStimuli() const;
		virtual	void	SetDoneProcessingStimuli(LTBOOL bDone);
		virtual void	ClearProcessedStimuli();
		virtual LTBOOL	ProcessStimulus(CAIStimulusRecord* pRecord);

		virtual int		GetIntersectSegmentCount() const;
		virtual void	ClearIntersectSegmentCount();
		virtual void	IncrementIntersectSegmentCount();

		virtual LTBOOL	HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle);
		virtual	void	HandleSenses(uint32 nCycle);
		virtual void	HandleSenseTrigger(AISenseRecord* pSenseRecord);
		virtual LTFLOAT	GetSenseDistance(EnumAISenseType eSenseType);

		virtual uint32  GetCurSenseFlags() const { return ( m_bSensesOn ) ? m_flagsCurSenses : kSense_None; }

		virtual	const RelationSet& GetSenseRelationSet() const;

		virtual const LTVector& GetSensingPosition() const { return m_vPos; }

		virtual CRange<int>& GetSightGridRangeX() { return m_rngSightGridX; }
		virtual CRange<int>& GetSightGridRangeY() { return m_rngSightGridY; }

		// Object info

		HOBJECT	GetObject() const { return m_hObject; }

        const char* GetName() const { return ToString(m_hObject); }

		uint32 GetDebugLevel() const { return m_nDebugLevel; }

		virtual const LTVector& GetPosition() const { return m_vPos; }
		void SetPathingPosition( const LTVector& vOffset );
        LTVector GetPathingPosition() const;
		virtual float GetVerticalThreshold() const;
		virtual float GetInfoVerticalThreshold() const;
        
        virtual LTVector GetWeaponPosition(CWeapon *pWeapon);
		HMODELSOCKET	GetWeaponSocket(CWeapon* pWeapon);

        const LTVector& GetEyePosition();
        const LTVector& GetEyeForward();

		const LTVector& GetTorsoRight();
        const LTVector& GetTorsoForward();

        const LTVector& GetDims() const { return m_vDims; }
        virtual LTFLOAT GetRadius() { return m_fRadius; }

        const LTRotation& GetRotation() const { return m_rRot; }

        virtual LTFLOAT GetRotationInterpolation(LTFLOAT fTime) { return fTime; }

        const LTVector & GetUpVector() const { return m_vUp; }
        const LTVector & GetRightVector() const { return m_vRight; }
        const LTVector & GetForwardVector() const { return m_vForward; }

		// Visiblity. Note: these functions can fill in useful values (distance, dot product, direction) of the point/object in question,
		// in order to save you from having to recompute them yourself (since they can be expensive). HOWEVER, you cannot use the values
		// if the point/object is NOT visible (since it may have skipped out of the function before comuting some of the values). ALSO,
		// in the case of checking to see if an object is visible, there is a degenerate case of the Source Position being INSIDE the
		// object, which does return TRUE, although the values will be somewhat meaningless. They will be fudged to a distance of MATH_EPSILON,
		// a dot product of 1, and a direction of the forward vector of the AI.

        virtual LTBOOL IsObjectVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject = LTNULL, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsObjectVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject = LTNULL, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsObjectVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObject, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject = LTNULL, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);

        virtual LTBOOL IsObjectPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject = LTNULL, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsObjectPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject = LTNULL, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsObjectPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObject, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject = LTNULL, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);

        virtual LTBOOL IsPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vFrom, const LTVector& vTo, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);

		// Query.

		virtual LTBOOL CanSearch();
		LTBOOL RequestCrouch(HOBJECT hRequestingAI);
		LTBOOL RequestDodge(HOBJECT hRequestingAI);

		// State methods

        void ChangeState(const char* szNextStateMessage, ...);
		CAIState* GetState() { return m_pState; }
		virtual void SetState(EnumAIStateType eState, LTBOOL bUnlockAnimation = LTTRUE) {}
		virtual void ClearState() {}
		virtual void ClearAndSetState(EnumAIStateType eState, LTBOOL bUnlockAnimation = LTTRUE) {}

		// Goal methods.

		CAIGoalMgr* GetGoalMgr() { return	m_pGoalMgr;	}

		// Senses functions

		void	SetCurSenseFlags(const uint32 flags) {	m_flagsCurSenses = m_flagsBaseSenses & flags; }
		void	ResetBaseSenseFlags() { m_flagsCurSenses = m_flagsBaseSenses; }
		LTBOOL IsSuspicious();
		CAISenseRecorderAbstract* GetSenseRecorder() const { return m_pAISenseRecorder; }

		// Brain

		virtual CAIBrain* GetBrain() { return m_pBrain; }

		// Holster

		LTBOOL		HasHolsterString() const { return !!m_hstrHolster; }
		void		SetHolsterString(const char* szWeapon) { FREE_HSTRING(m_hstrHolster); m_hstrHolster = g_pLTServer->CreateString(const_cast<char*>(szWeapon)); }
		const char* GetHolsterString() const { return g_pLTServer->GetStringData(m_hstrHolster); }
		void		ClearHolsterString() { FREE_HSTRING(m_hstrHolster); m_hstrHolster = LTNULL; }
		LTBOOL		HasBackupHolsterString() const { return !!m_hstrHolsterBackup; }
		const char* GetBackupHolsterString() const { return g_pLTServer->GetStringData(m_hstrHolsterBackup); }
		void		GetRightAndLeftHolsterStrings(char* szBufferRight, char* szBufferLeft, uint32 nBufferSize);
		EnumAIWeaponType GetHolsterWeaponType();

		// Enemy Target stuff

		void Target(HOBJECT hObject);
        LTBOOL FaceTarget();
		CAITarget* GetTarget();
        LTBOOL HasTarget();
		
		// Relation Mgr

		virtual void   UpdateRelationMgr();

		// Awareness

		void			SetAwareness(EnumAIAwareness eAwareness);
		EnumAIAwareness GetAwareness() { return m_eAwareness; }
		void			IncrementAlarmLevel(uint32 nIncr);
		uint32			GetAlarmLevel() const { return m_nAlarmLevel; }
		void			SetAlarmLevel(uint32 nAlarmLevel) { m_nAlarmLevel = nAlarmLevel; }
		LTBOOL			IsMajorlyAlarmed();
		LTBOOL			IsImmediatelyAlarmed();
		void			SetLastStimulusTime(LTFLOAT fTime) { m_fLastStimulusTime = fTime; }
		LTFLOAT			GetLastStimulusTime() { return m_fLastStimulusTime; }
		LTFLOAT			GetLastRelaxedTime() { return m_fLastRelaxedTime; }

		// Accuracy methods

		void UpdateAccuracy();

        void SetAccuracyModifier(LTFLOAT fModifier, LTFLOAT fTime);
        LTFLOAT GetAccuracy();
		LTFLOAT	GetFullAccuracyRadiusSqr() const { return m_fFullAccuracyRadiusSqr; }
		LTFLOAT	GetAccuracyMissPerturb();
		LTFLOAT	GetMaxMovementAccuracyPerturb();
		LTFLOAT	GetMovementAccuracyPerturbDecay() const { return m_fMovementAccuracyPerturbDecay; }

		// Activation

		void UpdateUserFlagCanActivate();

		// Static filter functions for intersect segments

        static bool DefaultFilterFn(HOBJECT hObj, void *pUserData);
        static bool BodyFilterFn(HOBJECT hObj, void *pUserData);
        static bool ShootThroughFilterFn(HOBJECT hObj, void *pUserData);
        static bool ShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData);
        static bool SeeThroughFilterFn(HOBJECT hObj, void *pUserData);
        static bool SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData);

        LTBOOL CanSeeThrough() { return m_bSeeThrough; }
        LTBOOL CanShootThrough() { return m_bShootThrough; }

		// Orientation/movement methods

        LTBOOL FaceObject(HOBJECT hObj);
        LTBOOL FaceDir(const LTVector& vTargetDir);
		LTBOOL FacePosMoving(const LTVector& vTargetPos);
        LTBOOL FacePos(const LTVector& vTargetPos);
		LTBOOL FacePos(const LTVector& vTargetPos, LTFLOAT fRotationSpeed);
		void   FaceTargetRotImmediately();
		void   SetCheapMovement(LTBOOL bCheap) { m_bCheapMovement = bCheap; }

		// Sound methods

		virtual void PlayCombatSound(EnumAISoundType eSound);
		virtual void PlaySearchSound(EnumAISoundType eSound);
		virtual void PlaySound(EnumAISoundType eSound, LTBOOL bInterrupt);
		virtual void PlaySound(char *pSound) { super::PlaySound(pSound); }
		virtual char* GetDeathSound();
		virtual char* GetDeathSilentSound();
		virtual char* GetDamageSound(DamageType eType);
		virtual void PlayDamageSound(DamageType eType) {}
        virtual LTBOOL CanLipSync();
		void MuteAISounds(LTBOOL bMute) { m_bMuteAISounds = bMute; }

		// Weapon methods

		CWeapon* GetCurrentWeapon() { return m_apWeapons[m_iCurrentWeapon]; }
		CWeapon* GetPrimaryWeapon() { if( m_iPrimaryWeapon == -1 ) return LTNULL; return m_apWeapons[m_iPrimaryWeapon]; }
		EnumAIWeaponType GetPrimaryWeaponType();
		EnumAIWeaponType GetCurrentWeaponType();
		EnumAnimProp GetWeaponProp(WEAPON const* pWeaponData);
		EnumAnimProp GetWeaponProp(EnumAIWeaponType eWeaponType);
		LTBOOL SetCurrentWeapon(EnumAIWeaponType eWeaponType);
		int GetNumWeapons() { return m_cWeapons; }
		CWeapon* GetWeapon(int iWeapon);
		CWeapon* GetWeapon(EnumAIWeaponType eWeaponType);
		CAttachmentPosition* GetWeaponPosition(int iWeapon) { _ASSERT(iWeapon >= 0 && iWeapon < m_cWeapons); return m_apWeaponPositions[iWeapon]; }
        LTBOOL HasWeapon(EnumAIWeaponType eWeaponType);
        LTBOOL HasDangerousWeapon();

		virtual void SetDeflecting( float fDuration );

		// Object methods (attachments)

		BaseClass* GetCurrentObject() { return m_apObjects[0]; }
		int GetNumObjects() { return m_cObjects; }
		BaseClass* GetObject(int iObject) { _ASSERT(iObject >= 0 && iObject < m_cObjects); return m_apObjects[iObject]; }
		CAttachmentPosition* GetObjectPosition(int iObject) { _ASSERT(iObject >= 0 && iObject < m_cObjects); return m_apObjectPositions[iObject]; }

		// Trigger methods.

		void QueueTriggerMsg(const CParsedMsg &cMsg);
		void QueueTriggerMsg(const char* szMsg);
		void Activate();
		void Cineract(const char* szAnim, LTBOOL bLoop);
		virtual void HideCharacter(LTBOOL bHide);

		// Commands.

		HSTRING GetCommandProximityGoal() const { return m_hstrCmdProximityGoal; }

		// Damage stuff

        void SetInvincible(LTBOOL bInvincible) { m_damage.SetCanDamage(!bInvincible); }
        LTFLOAT ComputeDamageModifier(ModelNode eModelNode);
		virtual LTBOOL CanBeDamagedAsAttachment();
		virtual	HMODELANIM	GetAlternateDeathAnimation();

		// Animation

		CAnimationContext* GetAnimationContext() { return m_pAnimationContext; }
		void SetAnimObject( HOBJECT hAnimObject ) { m_hAnimObject = hAnimObject; }
		HOBJECT GetAnimObject() { return m_hAnimObject; }

		// Head and Torso Tracking.

		void	SetNodeTrackingTarget(EnumTrackedNodeGroup eGroup, const LTVector& vPosition);
		void	SetNodeTrackingTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, const char* pszNodeName);
		void	SetNodeTrackingTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel);
		LTBOOL	EnableNodeTracking(EnumTrackedNodeGroup eGroup, HOBJECT hFace);
		void	DisableNodeTracking();
		EnumTrackedNodeGroup GetActiveNodeTrackingGroup() const;
		LTBOOL	IsNodeTrackingEnabled() const;
		LTBOOL	IsNodeTrackingAtLimit() const;
		EnumTrackedNodeGroup GetTriggerNodeTrackingGroup() const { return m_eTriggerNodeTrackingGroup; }

		// RETAIL: Tron's look-at tracking, told to ourselves as the
		// "TrackLookAt"/"TrackNone" commands (retail 0x101304f0; name ours)
		void	SetLookAtTracking(bool bTrack);

		// Attributes

		// PORT: Tron has no sentry distance members; NOLF2's sentry states
		// read the template's values through these (AI.cpp)
		float GetSentryChallengeScanDistMax();
		float GetSentryChallengeDistMax();
		float GetSentryMarkDistMax();


		LTFLOAT GetWalkSpeed() { return m_fWalkVel; }
		void SetWalkSpeed(LTFLOAT fWalkSpeed) { m_fWalkVel = fWalkSpeed; }
		LTFLOAT GetRunSpeed() { return m_fRunVel; }
		void SetRunSpeed(LTFLOAT fRunSpeed) { m_fRunVel = fRunSpeed; }
		LTFLOAT GetJumpSpeed() { return m_fJumpVel; }
		void SetJumpSpeed(LTFLOAT fJumpSpeed) { m_fJumpVel = fJumpSpeed; }
		LTFLOAT GetJumpOverSpeed() { return m_fJumpOverVel; }
		void SetJumpOverSpeed(LTFLOAT fJumpOverSpeed) { m_fJumpOverVel = fJumpOverSpeed; }
		LTFLOAT GetFallSpeed() { return m_fFallVel; }
		void SetFallSpeed(LTFLOAT fFallSpeed) { m_fFallVel = fFallSpeed; }
		LTFLOAT GetSwimSpeed() { return m_fSwimVel; }
		void SetSwimSpeed(LTFLOAT fSwimSpeed) { m_fSwimVel = fSwimSpeed; }

		void SetHoverAcceleration(float flHoverAcceleration ) { m_flHoverAccelerationRate = flHoverAcceleration; }
		float GetHoverAcceleration() { return m_flHoverAccelerationRate; }
		void SetHoverSpeed(float fFallSpeed) { m_fFallVel = fFallSpeed; }
		float GetHoverSpeed();
		bool HoverIsDrifting();

		// Movement

		uint32 GetCurValidVolumeMask() { return m_dwCurValidVolumeTypes; } 
		void   SetCurValidVolumeMask(const uint32 flags) {	m_dwCurValidVolumeTypes = m_dwBaseValidVolumeTypes & flags; }
		void   ResetBaseValidVolumeMask() { m_dwCurValidVolumeTypes = m_dwBaseValidVolumeTypes; }

		LTFLOAT	GetSpeed();
        void SetSpeed(LTFLOAT fSpeed) { m_fSpeed = fSpeed; }
		void Walk() { m_fSpeed = GetWalkSpeed(); }
		void Run() { m_fSpeed = GetRunSpeed(); }
		void Jump() { m_fSpeed = GetJumpSpeed(); }
		void JumpOver() { m_fSpeed = GetJumpOverSpeed(); }
		void Fall() { m_fSpeed = GetFallSpeed(); }
		void Swim() { m_fSpeed = GetSwimSpeed(); }
		void Hover() { m_fSpeed = GetHoverSpeed(); }
		void Stop() { m_fSpeed = 0.0f; }

		void Move(const LTVector& vPos);

		LTBOOL FindFloorHeight(const LTVector& vPos, LTFLOAT* pfFloorHeight);

		CAIMovement* GetAIMovement() { return m_pAIMovement; }

		LTransform& GetLastHintTransform() { return m_tfLastMovementHint; }
		void ClearLastHintTransform() { m_bClearMovementHint = LTTRUE; }
		HMODELANIM GetLastHintAnim() const { return m_hHintAnim; }

		LTBOOL IsWalkingOrRunning();
		LTBOOL IsStanding();

		// Path knowledge

		CAIPathKnowledgeMgr* GetPathKnowledgeMgr() const { return m_pPathKnowledgeMgr; }

		// Dialogue object

		LTBOOL IsControlledByDialogue() const { return !!m_hDialogueObject; }
		void StopDialogue()
		{
            SendTriggerMsgToObject(this, m_hDialogueObject, LTFALSE, "OFF");
			m_hDialogueObject = LTNULL;
		}

		void LinkDialogueObject(HOBJECT hDialogueObject);
		void UnlinkDialogueObject(HOBJECT hDialogueObject);

		// Client solid

		void SetClientSolid(LTBOOL bClientSolid) 
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, bClientSolid ? USRFLG_AI_CLIENT_SOLID : 0, USRFLG_AI_CLIENT_SOLID);
		}

		// World node managing functions.

		void			UpdateInvalidNodeList();
		INVALID_NODE*	AddNewInvalidNode();
		std::vector<INVALID_NODE*>* GetInvalidNodeList() { return &m_InvalidNodeList; }

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		virtual void StartDeath();

		virtual void SetUnconscious(bool bUnconscious);
		bool IsUnconscious() {return m_bUnconscious;}

		// defense

		virtual bool	IsDefending() const { return ( LTTRUE == m_bDefenseOn ); }

		// range: 0.0f - 1.0f; Get the "amount" of the defense...the
		// amount of damage that is suppressed by the defense.
		//
		// Example: if hit for 10 points of damage and the percentage
		// is 0.7, the character will take 3 points of damage.
		virtual float	GetDefensePercentage( LTVector const *pIncomingProjectilePosition = 0 ) const;

		// damage filtering 
		bool			FilterDamage( DamageStruct *pDamageStruct );

		void			SetDefending( bool bDefenseOn ) { m_bDefenseOn = static_cast< LTBOOL >( bDefenseOn != false ); }

		// Carrying

		virtual void SetCanCarry( bool bCarry );
		virtual void SetBeingCarried( bool bBeingCarried ) { m_bBeingCarried = bBeingCarried; }
		virtual bool BeingCarried() { return m_bBeingCarried; }
		// PORT: Tron's CAI has no waking (not in its vtable); a no-op for
		// NOLF2's AIGoalSpecialDamage
		void SetCanWake( bool ) {}

		// Get the complete list of all PlayerObj's.
		typedef std::vector< CAI* > AIList;
		static AIList const& GetAIList( ) { return m_lstAIs; }

	protected : // Protected methods

		// Update methods

		virtual void PreUpdate();
		virtual void Update();
		virtual void PostUpdate();

		virtual void UpdateAnimation();
		virtual void UpdateOnGround();
		virtual void UpdateTarget();
		virtual void UpdateMovement() {}
		virtual void UpdatePosition();
		virtual void UpdateNodes() {}
		virtual void UpdateState();
		virtual void UpdateCharacterFx() {}
		virtual void UpdateMusic();
		
		void UpdateInfo();

		// Handler methods

		virtual void HandleTeleport(const LTVector& vPos);
		virtual void HandleTeleport(TeleportPoint* pTeleportPoint);
		virtual void HandleDamage(const DamageStruct& damage);
		virtual void HandleTouch(HOBJECT hObj);
		virtual void HandleModelString(ArgList* pArgList);
		virtual void HandleTargetDied() {}
        virtual bool HandleCommand(const CParsedMsg &cMsg);
		void HandleCommandParameters(const CParsedMsg &cMsg);
		void HandleAttach();
		void HandleDetach();
		virtual void HandleVolumeEnter(AIVolume* pVolume);
		virtual void HandleVolumeExit(AIVolume* pVolume);

		// Parent methods
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);
		virtual void	PreCreateSpecialFX(CHARCREATESTRUCT& cs);

		// Engine methods

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32  ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		void	PreReadProp(ObjectCreateStruct* pStruct);
        virtual LTBOOL  ReadProp(ObjectCreateStruct* pStruct);
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);

		virtual void	InitialUpdate();
        virtual void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		virtual void	InitAttachments();

		// Brain methods

		virtual bool	SetBrain( char const* pszBrain );
		void			SetDamageMask( char const* pszMask );

		// Sets the AIs position

		void SetPosition(const LTVector& vPos, LTBOOL bFindFloorHeight);

		const AIBM_Template* const GetAITemplate() const;
		void SetAITemplate(const char* pszName);
		// RETAIL: Tron's, inlined in InitialUpdate and Load (name ours)
		inline void InitWeaponChoices();
		// RETAIL: Tron's (retail 0x1012f9e0, name ours)
		void InitTronWeapons();

	protected : // Member Variables

		HSTRING		m_hstrBodySkinExtension;

		// Misc

        LTBOOL      m_bCheapMovement;           // Are we using cheap movement?
        LTBOOL      m_bFirstUpdate;             // Is this before/during our first update?
        CString		m_sQueuedCommands;			// Our queued commands
		LTBOOL		m_bInitializedAttachments;
		bool		m_bIsCinematicAI;			// Are an AI in a cinematic

		// Sound

		LTFLOAT		m_fNextCombatSoundTime;		// Next time AI can utter combat dialog.
		LTBOOL		m_bMuteAISounds;			

		// Holster

		HSTRING		m_hstrHolster;				// Our holstered weapon
		HSTRING		m_hstrHolsterBackup;		// A backup copy of our weapon, in case stolen.
		LTFLOAT		m_fRestoreBackupWeaponTime;	// When can we restore our weapon.

		// Weapons

		CWeapon*				m_apWeapons[AI_MAX_WEAPONS];			// An array of pointers to our weapons
		CAttachmentPosition*	m_apWeaponPositions[AI_MAX_WEAPONS];	// An array of pointers to the attachemnts for the weapons
		// RETAIL: Tron's (zeroed by the constructor, not saved; set in
		// InitTronWeapons, retail 0x1012f9e0). Names ours.
		CWeapon*				m_pTronWeapon;
		CWeapons*				m_pTronWeapons;
		int						m_cWeapons;								// How many weapons we have
		int32					m_iCurrentWeapon;						// Index of current weapon.
		int32					m_iPrimaryWeapon;						// Index of Primary weapon.
		EnumAnimProp			m_ePrimaryWeaponProp;					// Anim prop for primary weapon.
		int						m_cDroppedWeapons;
		DroppedWeapon			m_aDroppedWeapons[AI_MAX_WEAPONS];		// Array of weapons we've dropped.

		// RETAIL: Tron's (constructor 0x10128830, Save 0x10132d80). The
		// choices are filled from the brain's AIData (InitWeaponChoices);
		// with SelectWeaponBasedOnVisibility InitialUpdate takes the second
		// and Load picks m_iCurrentWeapon from the current choice. Names
		// ours.
		AIWeaponChoiceList		m_lstWeaponChoices;
		AIWeaponChoice			m_CurWeaponChoice;	// copied whole from a choice
		bool					m_bSelectWeaponBasedOnVisibility;

		// Objects

		BaseClass*				m_apObjects[AI_MAX_OBJECTS];			// An array of pointers to our Objects
		CAttachmentPosition*	m_apObjectPositions[AI_MAX_OBJECTS];	// An array of pointers to the attachemnts for the Objects
		int						m_cObjects;								// How many Objects we have

		// Brain

		CString			m_sBrain;				// Name of our brain
		CAIBrain*		m_pBrain;				// Our brain (attributes and whatnot)

		// Goals

		CAIGoalMgr*	m_pGoalMgr;				// Manages goals.

		// State

        CAIState*    m_pState;                   // Current state
        HSTRING      m_hstrNextStateMessage;     // Next state message
        LTFLOAT      m_fNextStateTime;           // Next state change time

		// Object information

        LTVector	m_vPos;                     // Current pos this frame
        LTRotation  m_rRot;                     // Object's rotation
        LTVector	m_vRight;                   // Object's right vector
        LTVector	m_vUp;                      // Object's up vector
        LTVector	m_vForward;                 // Object's forward vector

		LTVector	m_vTorsoRight;				// Torso's right vector
		LTVector	m_vTorsoForward;			// Torso's forward vector

        LTVector	m_vEyePos;                  // The position of our eye
        LTVector	m_vEyeForward;              // Our eye's forward vector

        LTVector	m_vDims;                    // Object's dims
        LTFLOAT     m_fRadius;                  // Radius of circle circumscribed by our bounding box

        LTRotation	m_rTargetRot;               // Our target rotation
        LTRotation	m_rStartRot;                // Our starting rotation
        LTVector	m_vTargetRight;             // Object's target right vector
        LTVector	m_vTargetUp;                // Object's target up vector
        LTVector	m_vTargetForward;           // Object's target forward vector
        LTBOOL		m_bRotating;                // Are we rotating?
        LTFLOAT		m_fRotationTime;            // Time for our rotation interpolation
        LTFLOAT		m_fRotationTimer;           // Timer for our rotation interpolation

		LTBOOL		m_bUpdateNodes;				// Do the nodes need updating?

		// Senses

		LTFLOAT						m_fSenseUpdateRate;		// How quickly we update our senses.
		LTFLOAT						m_fNextSenseUpdate;
		uint32						m_flagsCurSenses;
		uint32						m_flagsBaseSenses;
		LTBOOL						m_bSensesOn;
		CAISenseRecorderAbstract*	m_pAISenseRecorder;
		CRange<int>					m_rngSightGridX;
		CRange<int>					m_rngSightGridY;

		// RETAIL: Tron's: an 8 byte object (a list of LTObjRefNotifier and
		// another member), created in the constructor and saved after the
		// keys (retail 0x101dee80). Not written yet. Name ours.
		CAITronUnknown1010*			m_pTronUnknown1010;

		// Target

        CAITarget*  m_pTarget;                  // Enemy Target
        LTBOOL      m_bShootThrough;            // Do we shoot through shoot-throughables at our target?
        LTBOOL      m_bSeeThrough;              // Do we see through see-throughables at any stimuli? (VERY EXPENSIVE!!!)

		// Accuracy/lag modififers

        LTFLOAT     m_fAccuracy;
        LTFLOAT     m_fAccuracyIncreaseRate;
        LTFLOAT     m_fAccuracyDecreaseRate;
        LTFLOAT     m_fAccuracyModifier;
        LTFLOAT     m_fAccuracyModifierTime;
        LTFLOAT     m_fAccuracyModifierTimer;
		LTFLOAT		m_fFullAccuracyRadiusSqr;
		LTFLOAT		m_fAccuracyMissPerturb;
		LTFLOAT		m_fMaxMovementAccuracyPerturb;
		LTFLOAT		m_fMovementAccuracyPerturbDecay;

		// Awareness

		EnumAIAwareness		m_eAwareness;			// Relaxed, Suspicious, or Alert.
		LTFLOAT				m_fFOVBias;				// Bias to our FOV
		uint32				m_nAlarmLevel;			// How alarmed the AI is based on observations.
		EnumAIStimulusID	m_eAlarmStimID;			// ID of registered stimulus for suspicion due to alarm.
		LTFLOAT				m_fLastStimulusTime;	// Last stimulus time.
		LTFLOAT				m_fLastRelaxedTime;		// Last time retruned to relaxed awareness.

		// Attributes

		const AIBM_Template* m_pAITemplate;

		// Commands

		HSTRING		m_hstrCmdInitial;			// Initial command (one-shot)
		HSTRING		m_hstrCmdActivateOn;		// ActivateOn command
		HSTRING		m_hstrCmdActivateOff;		// ActivateOff command
		// RETAIL: no OnMarking command in Tron (ReadProp 0x1012a640)
		HSTRING		m_hstrCmdProximityGoal;		// Command run from proximity goal.

		// Activation

		bool		m_bCanTalk;					// Are we allowed to be activated? If false, the AI can't be activated regardless of other settings
        LTBOOL      m_bActivated;               // Is our activation ON or OFF
		LTBOOL		m_bAlwaysActivate;			// Do we always activate regardless of our state
		LTBOOL		m_bPreserveActiveCmds;		// Should we not delete our active cmd's when used?
		bool		m_bUnconscious;

		// Animation stuff

		CAnimationMgr*		m_pAnimationMgr;
		CAnimationContext*	m_pAnimationContext;	// Our animation context
		LTObjRef			m_hAnimObject;			// Object to interact with while animating.

		// RETAIL: Tron's list of pointers (CAI::Activate adds unless
		// present; retail 0x1012ce40). Element type and name ours.
		std::list<void*>	m_lstTronUnknown10A0;

		// Node tracking.

		CServerTrackedNodeContext*	m_pTrackedNodeContext;
		EnumTrackedNodeGroup		m_eTriggerNodeTrackingGroup;

		// Movement

		LTFLOAT		m_fJumpOverVel;				// Speed for jumping over things.

		LTVector	m_vMovePos;					// Position to move to
		LTBOOL		m_bMove;					// Are we moving or not this frame
		LTFLOAT		m_fSpeed;					// Current movement speed

		CAIMovement* m_pAIMovement;
		LTransform	m_tfLastMovementHint;
		LTBOOL		m_bClearMovementHint;
		HMODELANIM	m_hHintAnim;
		LTBOOL		m_bUseMovementEncoding;
		LTBOOL		m_bTimeToUpdate;
		uint32		m_dwBaseValidVolumeTypes;
		uint32		m_dwCurValidVolumeTypes;

		// Path knowledge

		CAIPathKnowledgeMgr*	m_pPathKnowledgeMgr;	// Path knowledge.

		// Dialogue controller

		LTObjRef	m_hDialogueObject;				// Handle to our controlling Dialogue object

		// World node use:

		std::vector<INVALID_NODE*> m_InvalidNodeList;

		// Debug stuff

		uint32		m_nDebugLevel;				// Debug level

		CString		m_cstrCurrentInfo;

		CString		m_cstrNameInfo;
		CString		m_cstrStateInfo;
		CString		m_cstrGoalInfo;

		//	Stores the pathing position for the AI.  Certain AIs, such as
		//	floating AIs, use this offset to keep consistant with ground
		//	dwellers.
		LTVector	m_vPathingPosition;

		// The distance up and down the AI checks for volumes
		float		m_flVerticalThreshold;

		float		m_flHoverAccelerationRate;
		float		m_flLastHoverTime;
		float		m_flCurrentHoverSpeed;

		// RETAIL: no sentry distances in Tron (Save 0x10132d80)

		// defense variables
		LTBOOL		m_bDefenseOn;

		bool		m_bCanCarry;
		bool		m_bBeingCarried;
		// RETAIL: no tracking dart or waking in Tron; Tron's tail follows
		// (ReadProp 0x1012a640, Save 0x10132d80). Names ours.
		uint8		m_nKeys;					// bit n set by property Key<n+1>
		char		m_szAlignment[80];			// the template's alignment or "Alignment"
		TronAIInfo*	m_pTronAIInfo;				// "NameId", "ModelTemplate", "TriviaId"...
		bool		m_bTronUnknown11B4;			// set in UpdateState
		bool		m_bTronUnknown11B5;			// tested by node tracking
		bool		m_bSeeker;					// brain is "Seeker" (InitialUpdate)

		// List of all AI objects.
		static AIList	m_lstAIs;

};

#ifndef __PSX2
class CAIPlugin : public CCharacterPlugin
{
	public:
		CAIPlugin();
		virtual ~CAIPlugin();

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
		virtual CAttachmentsPlugin* GetAttachmentsPlugin();

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;
		CAttachmentsPlugin*	m_pAttachmentsPlugin;
		CModelButeMgrPlugin m_ModelButeMgrPlugin;
};
#endif

#endif // __AI_H__
