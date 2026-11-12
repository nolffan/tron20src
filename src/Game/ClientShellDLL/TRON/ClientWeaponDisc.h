// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponDisc.h
//
// PURPOSE : Tron specific client-side version of the disc
//
// CREATED : 4/12/02 (was WeaponModel.h)
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponDisc_h_INCLUDED_
#define  _ClientWeaponDisc_h_INCLUDED_

#include "TronClientWeapon.h"


// forward declarations to reduce header dependancies
struct DISCCLASSDATA;


// RETAIL: derives from CTronClientWeapon (RTTI); its members are at 0x208..
class CClientWeaponDisc : public CTronClientWeapon
{
public:
	CClientWeaponDisc();
	~CClientWeaponDisc();

	// handle messages
	virtual bool  OnMessage( uint8 messageID, ILTMessage_Read *pMsg );

	// callbacks from the animation system
	virtual bool  OnModelKey( HLOCALOBJ hObj, ArgList* pArgs );

	// RETAIL: 0x10019690 / 0x10019710
	virtual void  Load( ILTMessage_Read *pMsg );
	virtual void  Save( ILTMessage_Write *pMsg );

	// override the main update function
	virtual WeaponState   Update( bool bFire, FireType eFireType = FT_NORMAL_FIRE );

protected:
	// update the model state for the disc
	virtual WeaponState   UpdateModelState( bool bFire );

	// Update the defensive capabilities of the disc
	virtual void  UpdateFiring();
	void          UpdateSwatDefense();
	void          UpdateHoldDefense();
	virtual void  UpdateIdle();

	// initialize all the animations for this model
	virtual void  InitAnimations( bool bAllowSelectOverride = false );

	// swat defense animation
	bool          IsSwatDefenseAni( uint32 dwAni );
	uint32        GetSwatDefenseAni() const;

	// Hold defense animation
	bool          IsHoldDefenseAni( uint32 dwAni );
	uint32        GetPreHoldDefenseAni() const;
	uint32        GetHoldDefenseAni() const;
	uint32        GetImpactHoldDefenseAni() const;
	uint32        GetPostHoldDefenseAni() const;

	// play model animations
	virtual bool  PlayFireAnimation( bool bResetAni );
	virtual bool  PlaySwatDefenseAnimation( bool bResetAni );
	virtual bool  PlayHoldDefenseAnimation( bool bResetAni );

	// RETAIL: slots 103/104 (0x1001a1f0, 0x1001a210; names ours): any
	// ammo, and not a full load of it
	virtual bool  TronUnknownVirtual19C();
	virtual bool  TronUnknownVirtual1A0();

	// RETAIL: slot 50 (0x10018150, emitted in ClientWeaponClusterDisc's TU)
	virtual bool  TronUnknownVirtualC8() { return !TronUnknownVirtual1A0(); }

	// helper to handle the projectile messages that come down the pike
	virtual bool  HandleMessageProjectile( uint8 messageID, ILTMessage_Read *pMsg );

	// add disc firing information to the fire message
	virtual void  AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg );

	// return the "type" identifier (1st part of a fire message)
	// NOTE: currently the server handles VECTOR and PROJECTILE the same
	virtual uint8 GetFireMessageType() const { return MWEAPFIRE_DISC; }	// RETAIL: 0x10018160 returns 3 (kTronFire_Disc); our MsgIDs.h and server still say 2

	// send a message to the server telling it a defense is starting
	void          SendDefendMessage( uint8 cBlockMsgType, uint32 nAni ) const;

	// shoot a ray through the camera and return the point it hits
	bool          CalculatePointGuidedTarget( LTVector *pvTargetPoint );

	// get the ray telling where the player is facing
	bool          CalculateControlDirection( LTVector *pvControlDirection );

	// handle the fire key
	virtual void  Fire( bool bFire );
	virtual void  HandleFireKeyDownTransition();  // up to down transition
	virtual void  HandleFireKeyUpTransition();    // down to up transition
	virtual void  HandleFireKeyDown();            // fire held continuously down
	virtual void  HandleFireKeyUp();              // fire key not pressed

	// special cases
	virtual void  DoSpecialEndFire();

	// we got the disc back, add the ammo
	void          IncrementAmmo( int nAmount = 1 );

private:
	// swat defense animations
	HMODELANIM    m_nSwatDefenseAni;           // swat defense

	// hold defense animations
	HMODELANIM    m_nPreHoldDefenseAni;     // Hold defense start
	HMODELANIM    m_nHoldDefenseAni;        // Hold defense hold
	HMODELANIM    m_nImpactHoldDefenseAni;  // Hold defense impact
	HMODELANIM    m_nPostHoldDefenseAni;    // Hold defense end

	// RETAIL: the order below is retail's (0x21c..0x225, from Load/Save);
	// which retail bool is which of ours is a guess.

	// true if we should play the hold impact defenese animation
	// during the update (only works if the defense hold is already
	// playing)
	bool          m_bPlayImpactHoldAnimation;

	// true if the disc is in use
	bool          m_bIsDiscActive;

	// this flag will be sent when the fire message has been
	// sent to ensure we don't send updates for the disc before
	// the server has been notified its been sent
	bool          m_bFireMessageSent;

	// time that the disc is in use (-1.0f when not in use)
	float         m_fTimeDiscActive;

	// true if we should be sending updates to the server
	bool          m_bDiscNeedsUpdates;

	// The disc is a click-to-fire and click-to-return weapon: true when
	// the player clicked to return before the fire key (see b69).
	bool          m_bIgnoreFireKeyframe;
};

#endif //_ClientWeaponDisc_h_INCLUDED_