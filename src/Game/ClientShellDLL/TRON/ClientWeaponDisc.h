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
	virtual WeaponState   Update( bool bFire, bool bAltFire, FireType eFireType = FT_NORMAL_FIRE );

protected:
	// update the model state for the disc
	virtual WeaponState   UpdateModelState( bool bFire, bool bAltFire );

	// Update the defensive capabilities of the disc
	virtual void  UpdateFiring();
	void          UpdateSwatDefense();
	void          UpdateHoldDefense();
	// RETAIL: slot 53 (0x10019000): CTronClientWeapon's with the disc shown
	// again in the states but firing, and the defenses
	virtual void  UpdateNonFiring();
	// RETAIL: Tron's disc declares its two defense animations here, slots
	// 100 and 101 (0x10019920, 0x10019990): retail's UpdateFiring,
	// UpdateSwatDefense, UpdateHoldDefense and UpdateNonFiring call slot
	// 100 in W_SWAT_DEFENSE and slot 101 in W_HOLD_DEFENSE
	virtual bool  PlaySwatDefenseAnimation( bool bResetAni );
	virtual bool  PlayHoldDefenseAnimation( bool bResetAni );
	// RETAIL: slot 102 (0x10019e00; the name is ours): sends
	// MID_DISC_HOLD_DEFENSE with the fire position and direction
	virtual void  TronUnknownVirtual198();

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
	// RETAIL: slot 62 (0x10019f50): the disc returns
	virtual void  TronUnknownVirtualF8();
	virtual void  HandleFireKeyDown();            // fire held continuously down
	virtual void  HandleFireKeyUp();              // fire key not pressed

	// special cases
	virtual void  DoSpecialEndFire();

	// we got the disc back, add the ammo
	void          IncrementAmmo( int nAmount = 1 );

	// RETAIL: 0x1001a240: the player view FX of a block (2 blocked, 3
	// deadly blocked; the name is ours)
	void          CreateDefendFX( int nDefendType );
	// RETAIL: 0x1001a320: shows the disc piece again (the name is ours)
	void          ShowDisc();
	// RETAIL: 0x1001a370: MID_TRON_DISC_FIRED (the name is ours)
	void          SendDiscFired();
	// RETAIL: 0x10019ff0, 0x1001a0f0: MPROJ_RETURN and Tron's message 14
	// (the fire key is up) to our player (the names are ours)
	void          SendDiscReturn();
	void          SendDiscRelease();

private:
	// swat defense animations
	HMODELANIM    m_nSwatDefenseAni;           // swat defense

	// hold defense animations
	HMODELANIM    m_nPreHoldDefenseAni;     // Hold defense start
	HMODELANIM    m_nHoldDefenseAni;        // Hold defense hold
	HMODELANIM    m_nImpactHoldDefenseAni;  // Hold defense impact
	HMODELANIM    m_nPostHoldDefenseAni;    // Hold defense end

	// RETAIL: the order below is retail's (0x21c..0x225, from Load/Save).
	// AddExtraFireMessageInfo (0x10019bb0) sets 0x21d; Fire (0x10019dc0)
	// sets 0x21e from the class data's Controlable and 0x220 to -100.  The
	// names are ours.

	// true if we should play the hold impact defenese animation
	// during the update (only works if the defense hold is already
	// playing)
	bool          m_bPlayImpactHoldAnimation;	// 0x21c

	// true when the fire message has been sent (cleared by the fire key)
	bool          m_bFireMessageSent;			// 0x21d

	// true while the player controls the thrown disc (the fire key is
	// held): Update sends the control line to the server
	bool          m_bControllingDisc;			// 0x21e

	// the time the control line was last sent
	float         m_fLastControlTime;			// 0x220

	// true if we should be sending updates to the server
	bool          m_bDiscNeedsUpdates;

	// The disc is a click-to-fire and click-to-return weapon: true when
	// the player clicked to return before the fire key (see b69).
	bool          m_bIgnoreFireKeyframe;
};

#endif //_ClientWeaponDisc_h_INCLUDED_