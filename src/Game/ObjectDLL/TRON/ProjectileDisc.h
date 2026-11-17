// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileDisc.h
//
// PURPOSE : TRON Disc class - definition
//
// CREATED : 12/10/01
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
//
// RETAIL: Tron's CDisc is 0xa28 bytes (constructor 0x100371a0) with 55
// virtuals (vtable 0x102946dc).  The members after 0x97c are Tron's; their
// offsets come from the constructor and from Save (0x100380f0).  Names that
// the retail code does not show are ours.
//
// ----------------------------------------------------------------------- //


#ifndef  _ProjectileDisc_INCLUDED_
#define  _ProjectileDisc_INCLUDED_

//
// Includes
//
#include "TronProjectile.h"


// forward define to reduce header dependancies
struct WeaponFireInfo;


///
// class CDisc
//
class CDisc : public CTronProjectile
{
public:
	CDisc();
	~CDisc();


protected:
	// setup the disc (called externally)
	virtual LTBOOL      Setup( CWeapon const*, WeaponFireInfo const& );

	// message functions
	virtual uint32      EngineMessageFn( uint32, void*, LTFLOAT );
	virtual uint32      ObjectMessageFn( HOBJECT, ILTMessage_Read* );

	// RETAIL: slot 28 (0x100374a0)
	virtual void		TronUnknownVirtual28( WeaponFireInfo const& info );

	// RETAIL: slot 31 (0x10038f60): a disc in state 3 takes any touch
	virtual bool		TronUnknownVirtual31( HOBJECT hAttacker, HOBJECT hVictim );

	// RETAIL: slot 32 (0x1003ae60)
	virtual void		TronUnknownVirtual32();

	// RETAIL: slot 33 (0x1003b010): the disc's handling of a kept touch
	virtual bool		TronUnknownVirtual33( ProjectilePendingTouch *pTouch );

	// RETAIL: slot 34 (0x10038010): no impact on an object the disc has
	// damaged already
	virtual void		AddImpact( HOBJECT hObj,
	                               const LTVector &vFirePos,
	                               const LTVector &vImpactPos,
	                               const LTVector &vNormal,
	                               SurfaceType eSurfaceType = ST_UNKNOWN,
	                               IMPACT_TYPE eImpactType = IMPACT_TYPE_IMPACT,
	                               bool bDeflected = false );

	virtual void        HandleImpact( HOBJECT );
	virtual void        HandleTouch( HOBJECT );

	virtual void		HandleTouchSky( HOBJECT hObj );

	virtual void RemoveObject();

	// RETAIL: slot 45 (0x100e0e10) is an empty function
	virtual void        Detonate( HOBJECT, CollisionInfo* pCollInfo = LTNULL ) {}

	// RETAIL: slot 48 (0x10039060)
	virtual void		HandleOwnerDeath();

	// RETAIL: slot 49 (0x10038f80)
	virtual bool        UpdateDisc();

	// code to handle state changing information on ReturnMode
	// RETAIL: slot 50 (0x100399b0).  The argument is the object the disc
	// hit (0 when there is none)
	virtual void        EnterReturnMode( HOBJECT hObj );

	// RETAIL: slot 51 (0x1003a0a0): deflects the disc off the object
	virtual void		HandleDeflect();

	// RETAIL: slots 52 and 53 (0x1003a350, 0x1003a3c0) turn the blockable
	// flag and its AI stimulus on and off
	virtual void		StartBlockable();
	virtual void		StopBlockable();

	// RETAIL: slot 54 (0x100356d0): returns the flag that slots 52 and 53
	// set
	virtual bool		IsBlockable() const { return m_bBlockable; }

	// RETAIL: 0x10037180 (from the constructor and RemoveObject): an empty
	// function
	void				ClearDebugInfo();

	// RETAIL: 0x1026abb0 (from 0x1003a3f0): an empty function
	void				ShowDebugTarget( HOBJECT hTarget );

	// RETAIL: 0x1003a3f0 (from UpdateStateFlying 0x1003c2a3)
	bool				UpdateMagneticHoming( LTVector *pvDir, LTVector const *pvTronUnknown );

	// RETAIL: 0x1003a780 (from UpdateStateFlying 0x1003bd99): a player's
	// disc homes in on the nearest target in the auto aim radius.  The
	// name is ours
	bool				AcquireAutoAimTarget();

	// RETAIL: 0x1003aea0 (from slot 33): a kept touch of a deadly
	// deflected disc.  The name is ours
	bool				HandleDeadlyDeflectTouch( ProjectilePendingTouch *pTouch );

	// RETAIL: 0x1003c570 (from UpdateStateFlying): the object's position
	// up by a part of its height.  The name is ours
	LTVector			GetControlPosition( HOBJECT hObj );

	// RETAIL: 0x1003c5f0 (from UpdateStateFlying): the point of the target
	// the disc flies at.  The name is ours
	LTVector			GetTargetPosition();

	// RETAIL: 0x1003d1b0: a deadly deflected disc hits an object.  The
	// name is ours
	void				SmiteObject( HOBJECT hObj );

	// RETAIL: 0x1003d460: the disc is destroyed where it is (it does not
	// return)
	void				DestroyInFlight();

	// RETAIL: 0x1003d490: plays the class data's DestroyedInFlightFXName
	void				CreateDestroyedInFlightFX();

	// RETAIL: 0x1003cf50: tells the owner that the disc returned
	void				SendReturnedMessage();

	LTVector			GetReturnToPosition(const LTVector& vOffset);

	// RETAIL: 0x1003d410: the offset from the deflect position at a time
	void				GetDeflectOffset( LTVector *pvOffset,
										  float fInitialRadius,
										  float fFinalRadius,
										  float fDistance,
										  float fAngularSpeed,
										  float fTime,
										  float fTotalTime );

	// RETAIL: inline (0x1003c790 computes it twice): the time since the
	// deflection started, up to its end time when it has one.  The name
	// is ours
	LTFLOAT				GetDeflectElapsedTime()
	{
		return ( ( m_fDeflectEndTime > 0.0f ) ? m_fDeflectEndTime : g_pLTServer->GetTime() ) - m_fDeflectStartTime;
	}

	// specific update routines
	bool                UpdateStateFlying();
	bool                UpdateStateDeflecting();
	bool                UpdateStateReturning();

	// State query methods:
	bool				IsDiscReturning();
	// RETAIL: 0x1003cf30
	bool				IsDiscDeflecting();

private:
	void				Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
	void				Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

protected:

	// current state...what is the disc doing?
	// NOTE: these are enumerated states, the enumerations
	// defined in the .cpp file
	int                 m_eDiscState;			// 0x918
	int                 m_eTrackingMode;		// 0x91c
	int                 m_eTrackingStyle;		// 0x920

	// TRUE between the time we receive a new target point
	// and the disc recomputes its flight based on the new info.
	bool                m_bNewTarget;			// 0x924

	// The target the disc is trying to hit (it will use one or the
	// other, not both, depending on the tracking state).
	LTVector            m_vPointTarget;			// 0x928
	LTObjRef            m_hObjectTarget;		// 0x934

	// variables for the control line method of tracking
	LTVector            m_vControlDirection;	// 0x944
	LTVector            m_vControlPosition;		// 0x950

	// Socket the Disc should return to, valid if the valid
	// is not INVALID_MODEL_SOCKET
	HMODELSOCKET		m_hReturnToSocket;		// 0x95c

	// Store the ID of the Catch enabling stimulus so that it can
	// be turned off when the disc is caught.  Currently set if the
	// value is not kStimID_Unset
	EnumAIStimulusID	m_eDiscCatchableStimID;	// 0x960

	// Store the ID of the Block enabling stimulus so that it can
	// be turned off when the disc enters return mode.  Currently
	// set if the value is not kStimID_Unset
	EnumAIStimulusID	m_eDiscBlockableStimID;	// 0x964

	// the disc's return velocity
	LTFLOAT             m_fReturnVelocity;		// 0x968

	// if the disc is not returning to a node, it will return
	// to the owner's position plus this offset in the Y
	// direction
	LTFLOAT             m_fReturnHeightOffset;	// 0x96c

	// RETAIL: the class data's ReturnToOwner
	LTBOOL				m_bReturnToOwner;		// 0x970

	// rate (in RADIANS/SEC) at which the disc can turn
	LTFLOAT             m_fTurnRate;			// 0x974

	// angle (in RADIANS) the disc will attempt in aim for the control line
	LTFLOAT             m_fIncidentAngleToControlLine;	// 0x978

	// the decay (in RADIANS/SEC) of the previously mentioned angle
	LTFLOAT             m_fIncidentAngleToControlLineDecay;	// 0x97c

	// RETAIL: the cosine of the class data's TargetAngleTrackThreshold
	LTFLOAT				m_fTargetAngleTrackThreshold;	// 0x980
	LTFLOAT				m_fLostTargetFlightTime;		// 0x984

	//	Temp demo hack!!  This is here because the discs RemoveObject
	//	can be called from TestInsideObject, meaning that the thrower
	//	never recieves the RETURNED message, and is then stuck
	//	discless.
	// RETAIL: Tron's is a bool
	bool				m_bSentReturnedMsg;		// 0x988

	bool				m_bBlockable;			// 0x989

	// the deflection
	LTFLOAT				m_fDeflectStartTime;	// 0x98c (a time)
	LTFLOAT				m_fTronUnknown990;		// 0x990
	LTFLOAT				m_fDeflectEndTime;		// 0x994 (a time)
	bool				m_bTronUnknown998;		// 0x998
	LTVector			m_vDeflectPos;			// 0x99c
	LTVector			m_vTronUnknown9A8;		// 0x9a8
	LTFLOAT				m_fDeflectTotalTime;	// 0x9b4
	LTFLOAT				m_fDeflectVelocity;		// 0x9b8
	LTFLOAT				m_fDeflectDistance;		// 0x9bc
	LTFLOAT				m_fDeflectInitialRadius;	// 0x9c0
	LTFLOAT				m_fDeflectFinalRadius;	// 0x9c4
	LTFLOAT				m_fDeflectAngularSpeed;	// 0x9c8
	LTRotation			m_rDeflectRot;			// 0x9cc
	bool				m_bDeflectAdjust;		// 0x9dc
	int					m_nDeflectUpdateNum;	// 0x9e0 (an m_nUpdateNum)

	// RETAIL: the last bool of the fire info (WeaponFireInfo 0x164)
	bool				m_bTronUnknown9E4;		// 0x9e4

	// the class data's magnetic values
	LTFLOAT				m_fMagneticRadius;			// 0x9e8
	LTFLOAT				m_fMagneticMiddleInfluence;	// 0x9ec
	LTFLOAT				m_fMagneticEdgeInfluence;	// 0x9f0
	LTFLOAT				m_fMagneticAngleMax;		// 0x9f4

	// RETAIL: the ammo id (Setup sets "DiscEnergy"'s for a world model
	// in a disc game)
	uint8				m_nDiscAmmoId;			// 0x9f8
	bool				m_bTronUnknown9F9;		// 0x9f9
	LTVector			m_vTronUnknown9FC;		// 0x9fc
	HOBJECT				m_hLastTouched;			// 0xa08
	LTFLOAT				m_fTronUnknownA0C;		// 0xa0c
	LTFLOAT				m_fMultiplayerLeashLength;	// 0xa10
	LTVector			m_vTronUnknownA14;		// 0xa14
	uint32				m_dwFlagsAtStart;		// 0xa20 (MID_INITIALUPDATE)

	// last time the projectile was updated
	LTFLOAT             m_fLastUpdateTime;		// 0xa24
};


#endif //_ProjectileDisc_INCLUDED_
