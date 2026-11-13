// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeapon.h
//
// PURPOSE : Generic client-side weapon 
//
// CREATED : 9/27/97 (was WeaponModel.h)
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeapon_h_INCLUDED_
#define  _ClientWeapon_h_INCLUDED_

// stl vector
#include <vector>

#include "PVFXMgr.h"
#include "ClientFXMgr.h"
#include "MsgIDs.h"
#include "ClientWeaponBase.h"

// forward definitions to reduce header dependancies
struct CLIENTFX_LINK;
enum SurfaceType;

//
// Defines
//
#define WM_MAX_FIRE_ANIS            WMGR_MAX_WEAPONANI_FIRE
#define WM_MAX_ALTFIRE_ANIS         WM_MAX_FIRE_ANIS    
#define WM_MAX_IDLE_ANIS            WMGR_MAX_WEAPONANI_IDLE
#define WM_MAX_ALTIDLE_ANIS         WM_MAX_IDLE_ANIS
#define WM_MAX_PV_ATTACH_CLIENTFX   WMGR_MAX_PVCLIENTFX_ATTACHMENTS


///
// class CClientWeapon
//
class CClientWeapon : public IClientWeaponBase
{
public:

	CClientWeapon();
	virtual       ~CClientWeapon();

	// handle messages
	virtual bool  OnMessage( uint8 messageID, ILTMessage_Read *pMsg ) { return true; }

	// callbacks from the animation system
	virtual bool  OnModelKey( HLOCALOBJ hObj, ArgList* pArgs );

	// enter/exit world functionality
	virtual void          OnEnterWorld();
	virtual void          OnExitWorld();

	// standard init/term functions
	virtual bool          Init( WEAPON const &pWeapon );
	virtual void          Term();

	// main update function
	virtual WeaponState   Update( bool bFire, FireType eFireType = FT_NORMAL_FIRE );

	// external interfaces to change the ammo
	virtual void          ChangeAmmoWithReload( uint8 nNewAmmoId, bool bForce= false );
	virtual void          ChangeAmmoImmediate( uint8 nNewAmmoId, int nAmmoAmount = -1, bool bForce= false );
	virtual void          ReloadClip( bool bPlayReload = true, int nNewAmmo=-1, bool bForce = false, bool bNotifyServer = false );
	virtual void          DecrementAmmo();

	// set the information about the camera that the weapon needs to know
	virtual void          SetCameraInfo( LTRotation const &rCamRot, LTVector const &vCamPos );

	// Update the weapon model filenames, particularly the skins.
	// This is for cases where the base player model changes and 
	// the playerview model needs to change too (i.e. sleeves
	// look different on new player model).
	virtual void          ResetWeaponFilenames();

	// to update the weapon bob
	virtual void          UpdateBob( LTFLOAT fWidth, LTFLOAT fHeight );

	// dynanic perturb
	// RETAIL: not virtual in Tron (IClientWeaponBase's vtable)
	LTFLOAT       GetDynamicPerturb()  const { return (m_fFiringPerturb > m_fMovementPerturb ? m_fFiringPerturb : m_fMovementPerturb); }

	// Do NOT call these functions directly.  If you want to
	// enable/disable or show/hide the weapons, call the functions
	// in CClientWeaponMgr.
	virtual void          SetVisible( bool bVis = true );
	virtual void          SetDisable( bool bDisable = true );
	
	// Do NOT call these directly.  They are used CClientWeaponMgr.
	virtual void          Select();
	virtual bool          Deselect( ClientWeaponCallBackFn cbFn, void *pData );
	virtual bool          Activate();   // Create the weapon's resources
	virtual void          Deactivate(); // Shut down the weapon (destroy resources)

	// weapon info
	virtual WEAPON const *GetWeapon()       const { return m_pWeapon; }
	virtual int           GetWeaponId()     const { return m_nWeaponId; }

	// ammo info
	virtual AMMO const   *GetAmmo()         const { return m_pAmmo; }
	virtual int           GetAmmoId()       const { return m_nAmmoId; }
	virtual int           GetAmmoInClip()   const { return m_nAmmoInClip; }
	virtual bool          HasAmmo() const;
	virtual bool          CanUseAmmo( uint8 nAmmoId ) const;
	virtual bool          CanChangeToAmmo( uint8 nAmmoId ) const;
	virtual uint8         GetNextAvailableAmmo( uint8 nGivenAmmoId = WMGR_INVALID_ID );
	virtual bool          GetBestAvailableAmmoId( int *nAmmoId ) const;

	// state info
	virtual WeaponState   GetState()        const { return m_eState; }

	// misc functions
	virtual bool          IsMeleeWeapon() const { return ( m_pAmmo && ( DT_MELEE == m_pAmmo->eInstDamageType ) ); }
	virtual bool          IsFireButtonDown() const { return m_bFireKeyDownLastUpdate; }
	virtual bool          WeaponNeedsCrosshair() const { return true; }

	virtual void          GetShellEjectPos( LTVector *vOriginalPos );

	// used with the cheat codes to adjust the proper position of the weapon
	virtual LTVector      GetMuzzleOffset() const               
	{ 
		if (!m_pWeapon) return LTVector();
		return m_pWeapon->vMuzzlePos;		
	}

	virtual void          SetMuzzleOffset( LTVector const &v )  
	{
		if (!m_pWeapon) return;
		const_cast<WEAPON *>(m_pWeapon)->vMuzzlePos = v;
	}
	
	virtual LTVector      GetBreachOffset() const               
	{ 
		if (!m_pWeapon) return LTVector();
		return m_pWeapon->vBreachOffset;		
	}

	virtual void          SetBreachOffset( LTVector const &v )  
	{
		if (!m_pWeapon) return;
		const_cast<WEAPON *>(m_pWeapon)->vBreachOffset = v;
	}
	
	virtual LTVector      GetWeaponOffset() const               
	{ 
		if (!m_pWeapon) return LTVector();
		return m_pWeapon->vPos;		
	}
	
	virtual void          SetWeaponOffset( LTVector const &v )  
	{ 
		if (!m_pWeapon) return;
		const_cast<WEAPON *>(m_pWeapon)->vPos = v;
	}

	// This interface is only used for the camera shake.  This
	// is privledged information, and the code to shake the
	// camera should eventually be moved to this class.  Please
	// don't make use of this function, and if you do comment its
	// use here.
	virtual HLOCALOBJ     GetHandle() const { return m_hObject; }

	// This needs to be exposed for the PlayerStats.  Eventually
	// this should be moved so creation/deletion of the mods
	// are transparent to the outside system (not all games
	// need mods).
	virtual void          CreateMods();

	// load/save functionality
	virtual void          Load( ILTMessage_Read *pMsg );
	virtual void          Save( ILTMessage_Write *pMsg );

	virtual void		  ResetWeapon();

	virtual void		SetPaused( bool bPaused );
	
	virtual void		ClearFiring();


protected:

	//
	//
	// Protected interfaces
	//
	//

	// Create the weapon model and 
	bool          CreateWeaponModel();
	void          RemoveWeaponModel();

	// general model create (used for weapon, mods, etc)
	HOBJECT       CreateModelObject( HOBJECT hOldObj, ObjectCreateStruct *createStruct );

	WeaponState   UpdateAmmoFromFire( bool bDecrementAmmo = true );
	virtual WeaponState   UpdateModelState( bool bFire );
	void          UpdateWeaponPosition( LTVector const &vOffset );  // offset is the additional offset

	// Dynamic perturb updating
	void          UpdateMovementPerturb();
	void          UpdateFiringPerturb(LTBOOL bFiredWeapon);

	// Update the weapon based on current state and
	// animation and determines what animation to play next.
	virtual void  UpdateFiring();

	// [KLS 5/8/02] - changed the name of this function from UpdateIdle *back* to 
	// UpdateNonFiring which is more accurate since the weapon may be reloading, 
	// selecting, deselecting, or idle...but definitiely not firing ;)
	virtual void  UpdateNonFiring(); 

	// initialize all the animations for this model
	virtual void  InitAnimations( bool bAllowSelectOverride = false );
	void          SetAmmoOverrideAnis( bool bAllowSelectOverride );

	// play model animations
	void          PlayAnimation( uint32 dwAni, bool bReset = true,
	                             float fRate = 1.0f,  bool bLooping = false);
	bool          PlaySelectAnimation();
	bool          PlayDeselectAnimation();
	bool          PlayReloadAnimation();
	bool          PlayIdleAnimation();
	virtual bool  PlayFireAnimation( bool bResetAni );
	// RETAIL: Tron's slot 0xe0, two arguments (the first compared with -1;
	// retail 0x10133890). Not yet named
	virtual bool  TronUnknownVirtualE0( int nArg1, int nArg2 );

	// selet animation
	bool          IsSelectAni( uint32 dwAni ) const;
	uint32        GetSelectAni() const;

	// deselect animation
	bool          IsDeselectAni( uint32 dwAni ) const;
	uint32        GetDeselectAni() const;

	// reload animation
	bool          IsReloadAni( uint32 dwAni ) const;
	uint32        GetReloadAni() const;

	// idle animation
	bool          IsIdleAni( uint32 dwAni ) const;
	uint32        GetIdleAni() const;
	uint32        GetSubtleIdleAni() const;
	LTFLOAT       GetNextIdleTime() const;

	// pre fire animation
	bool          IsPreFireAni( uint32 dwAni ) const;
	uint32        GetPreFireAni() const;

	// fire animation
	bool          IsFireAni( uint32 dwAni , bool bCheckNormalOnly = false) const;
	uint32        GetFireAni( FireType eFireType=FT_NORMAL_FIRE ) const;

	// post fire
	bool          IsPostFireAni( uint32 dwAni ) const;
	uint32        GetPostFireAni() const;

	// are we using alt fire animations
	bool          CanUseAltFireAnis() const;

	// get the model position/rotation info
	void          GetModelPos( LTVector *vPos ) const;
	void          GetModelRot( LTRotation *rRot ) const;

	// mod helpers
	void          UpdateMods();
	void          RemoveMods();
	void          SetVisibleMods( bool bVis = true );

	//TODO: pull these out of the base code
	// silencer mod
	void          CreateSilencer();
	void          UpdateSilencer();
	void          RemoveSilencer();

	//TODO: pull these out of the base code
	// scope mod
	void          CreateScope();
	void          UpdateScope();
	void          RemoveScope();

	// muzzle flash helper routines
	void          CreateMuzzleFlash();
	void          RemoveMuzzleFlash();
	void          StartMuzzleFlash();
	void          UpdateMuzzleFlash( WeaponState eState, LTVector const &vOffset );

	// player-view fx helpers
	void          CreatePVAttachClientFX();
	void          ShowPVAttachClientFX();
	void          HidePVAttachClientFX();
	void          RemovePVAttachClientFX();

	// weapon fire helpers
	virtual void  Fire( bool bFire );
	virtual void  HandleFireKeyDownTransition();  // up to down transition
	virtual void  HandleFireKeyUpTransition();    // down to up transition
	virtual void  HandleFireKeyDown() {}          // fire held continuously down
	virtual void  HandleFireKeyUp() {}            // fire key not pressed

	// RETAIL: Tron's slots 0xf8..0x104, empty here (the discs override the
	// first). Not yet named
	virtual void  TronUnknownVirtualF8() {}
	virtual void  TronUnknownVirtualFC() {}
	virtual void  TronUnknownVirtual100() {}
	virtual void  TronUnknownVirtual104() {}

	// send all relevant fire information to the server
	virtual void  SendFireMessage( bool bFire,
	                               LTFLOAT fPerturb,
	                               LTVector const &vFirePos,
	                               LTVector const &vDir,
								   LTVector const &vImpactPos,
	                               HOBJECT hObject );

	// add any extra firing information (primarily for derived classes)
	virtual void  AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg );

	// RETAIL: Tron's slots 0x110, 0x114: one argument, empty here (the discs
	// override the first). Not yet named
	virtual void  TronUnknownVirtual110( int nArg ) {}
	virtual void  TronUnknownVirtual114( int nArg ) {}

	// return the "type" identifier (1st part of a fire message)
	// NOTE: currently the server handles VECTOR and PROJECTILE the same
	virtual uint8 GetFireMessageType() const { return MWEAPFIRE_VECTOR; }

	// get the last type of fire sound played
	uint8         GetLastSndFireType() const;

	// Get information to use to fire the weapon, such
	// as weapon position and the forward vector.
	bool          GetFireVectors( LTVector *vU, LTVector *vR, LTVector *vF,
	                              LTVector *vFirePos ) const;

	// set the model's state, do any special setup necessary
	// to enter particular states
	WeaponState   SetState( WeaponState eNewState );

	// clientside firing considerations
	void          ClientFire( LTVector const &vPath, LTVector const &vFirePos, LTVector *pObjectImpactPos, HOBJECT *pObjectImpact );
	void          DoProjectile( LTVector const &vPath, LTVector const &vFirePos );
	void          DoVector( LTVector const &vPath, LTVector const &vFirePos, LTVector *pObjectImpactPos, HOBJECT *pObjectImpact );
	void          HandleVectorImpact( LTVector const &vPath, IntersectQuery *qInfo, IntersectInfo *iInfo, LTVector *pObjectImpactPos, HOBJECT *pObjectImpact );
	void          DoGadget( LTVector const &vPath, LTVector const &vFirePos );
	void          HandleGadgetImpact( HOBJECT hObj, LTVector vImpactPoint );

	// special cases
	void          DoSpecialFire();
	virtual void  DoSpecialEndFire();
	// RETAIL: Tron's slot 0x120 (retail 0x10135450). Not yet named
	virtual void  TronUnknownVirtual120();
	void          DoSpecialCreateModel();
	void          SpecialShowPieces( bool bShow = true, bool bForce = false );
	bool          SpecialOverrideFire();

	// handle client side impact effects
	void          AddImpact( HLOCALOBJ hObj, LTVector const &vImpactPoint,
	                         LTVector const &vNormal,  LTVector const &vPath,
	                         SurfaceType eType );

	// generic routine for placing this weapon's
	// information in the create struct
	void          PopulateCreateStruct( ObjectCreateStruct *pOCS ) const;
	
	void          KillLoopSound();

	// reset data before creating the weapon's resources
	void          ResetData();

	// keyframe handlers
	// RETAIL: Tron's OnModelKey hands every key to one of these (slots
	// 0x124..0x15c, in this order); the names other than HandleFireKey's
	// and HandleFXKey's are ours
	virtual bool  HandleFireKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleSoundKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleLoopSoundKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleFXKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleFireFXKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleHidePieceKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleShowPieceKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleFlashlightKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleDeflectKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleRenderStyleKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleShellCasingKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleHidePVAttachFXKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleShowPVAttachFXKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleHidePVAttachmentKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool  HandleShowPVAttachmentKey( HLOCALOBJ hObj, ArgList* pArgs );

	// RETAIL: Tron's last two slots: 0x160 takes one argument and returns
	// false here; 0x164 none (retail 0x1001a1f0). Not yet named
	virtual bool  TronUnknownVirtual160( int nArg ) { return false; }
	virtual bool  TronUnknownVirtual164();

	// remove all keyframed FXEd effects that have expired
	bool          RemoveFinishedKeyframedFX();

	//
	//
	// Protected Data
	//
	//

	ClientWeaponCallBackFn  m_cbDeselect;
	void         *m_pcbData;

	HOBJECT       m_hObject;          // Handle of WeaponModel model

	HMODELSOCKET  m_hBreachSocket;    // Handle of breach socket

	// mod models
	HOBJECT       m_hSilencerModel;   // Handle of silencer mod model
	HOBJECT       m_hScopeModel;      // Handle of scope mod model

	// mod sockets
	HMODELSOCKET  m_hSilencerSocket;  // Handle of silencer mod socket
	HMODELSOCKET  m_hScopeSocket;     // Handle of scope mod socket

	// mod posessions
	bool          m_bHaveSilencer;    // Do we have a silencer mod
	bool          m_bHaveScope;       // Do we have a scope mod

	int           m_nWeaponId;
	int           m_nAmmoId;

	WEAPON const *m_pWeapon;
	AMMO const   *m_pAmmo;

	LTVector      m_vFlashPos;
	LTVector      m_vFlashOffset;
	LTFLOAT       m_fFlashStartTime;       // When did flash start

	LTFLOAT       m_fBobHeight;
	LTFLOAT       m_fBobWidth;

	LTFLOAT       m_fMovementPerturb;
	LTFLOAT		  m_fFiringPerturb;

	FireType      m_eLastFireType;    // How did we last fire
	bool          m_bCanSetLastFire;  // Can we set m_eLastFireType

	LTFLOAT       m_fNextIdleTime;
	bool          m_bFire;

	// local copy of ammo
	int           m_nAmmoInClip;
	int           m_nNewAmmoInClip;

	// What are we currently doing
	WeaponState   m_eState;

	HMODELANIM    m_nSelectAni;       // Select weapon
	HMODELANIM    m_nDeselectAni;     // Deselect weapon
	HMODELANIM    m_nReloadAni;       // Reload weapon

	HMODELANIM    m_nIdleAnis[WM_MAX_IDLE_ANIS];  // Idle animations
	HMODELANIM    m_nFireAnis[WM_MAX_FIRE_ANIS];  // Fire animations

	HMODELANIM    m_nAltSelectAni;        // Alt-Fire Select weapon
	HMODELANIM    m_nAltDeselectAni;      // Alt-Fire Deselect weapon (back to normal weapon)
	HMODELANIM    m_nAltDeselect2Ani;     // Alt-Fire Deselect weapon (to new weapon)
	HMODELANIM    m_nAltReloadAni;        // Alt-Fire Reload weapon

	HMODELANIM    m_nAltIdleAnis[ WM_MAX_ALTIDLE_ANIS ];    // Alt-Fire Idle animations
	HMODELANIM    m_nAltFireAnis[ WM_MAX_ALTFIRE_ANIS ];    // Alt-Fire animations

	HMODELANIM    m_nPreFireAni;          // Optional animation to play before the actual fire ani
	HMODELANIM    m_nPostFireAni;         // Optional animation to play after the actual fire ani

	bool          m_bUsingAltFireAnis;        // Use the m_nAltXXX anis?
	bool          m_bFireKeyDownLastUpdate;   // Was the fire key down last update?
	
    LTFLOAT		  m_fLastPitch;	// Holds last player pitch value
    LTFLOAT		  m_fLastYaw;	// Holds last player yaw value

	uint16        m_wIgnoreFX;    // FX to ignore for current vector/projectile

	bool          m_bWeaponDeselected;    // Did we just deselect the weapon

	bool          m_bDisabled;    // Is the weapon disabled
	bool          m_bVisible;     // Is the weapon visible (should it be)

	bool		  m_bControllingFlashLight; // Does this weapon control the pv flash light?

	LTRotation    m_rCamRot;
	LTVector      m_vCamPos;

	HLTSOUND      m_hLoopSound;
	uint8         m_nLoopSoundId;

	bool          m_bFirstSelection; // Is this the first time we are selecting this weapon?

	// used to determine if we should make a tracer
	int           m_nTracerNumber;

	CLIENTFX_LINK m_PVAttachClientFX[ WM_MAX_PV_ATTACH_CLIENTFX ];  // FXEd attachments
	bool		  m_bPVAttachClientFXHidden[ WM_MAX_PV_ATTACH_CLIENTFX ]; // Was the FX visible when we tried to hide it?

	CLIENTFX_LINK m_MuzzleFlashFX;  // An instance of a FxED created FX 

	// auto switch weapons
	bool          m_bAutoSwitchEnabled;
	bool          m_bAutoSwitch;

	// keyframed ClientFX
	CLIENTFX_LINK_NODE	m_KeyframedClientFX;
	
	// Is the weapon supposed to be paused...
	bool			m_bPaused;

	// RETAIL: Tron's CClientWeapon is 0x204 bytes (CTronClientWeapon's ctor,
	// 0x10097090, puts its first member at 0x204); ours is 0x158 up to here.
	// The members before differ too (retail's 0xC..0x138 per the ctor at
	// 0x1012ec30), so this tail only brings the size -- and the members of
	// the classes derived from it -- to retail's.  The members Tron's
	// weapons use from 0x158 on are at retail's offsets; shrink the padding
	// when the members above are fixed.
	char			m_xTronUnknown155[3];
	union
	{
		float		m_fTronUnknown158;		// RETAIL: +0x158 (BallLauncher's fire rate)
		int			m_nTronUnknown158;		// RETAIL: +0x158 (DrunkenDims' vectors)
	};
	float			m_fTronUnknown15C;		// RETAIL: +0x15c (LOL energy)
	float			m_fTronUnknown160;		// RETAIL: +0x160 (Sequencer energy)
	float			m_fTronUnknown164;		// RETAIL: +0x164 (BallLauncher, DrunkenDims)
	// RETAIL: +0x13c, eleven pairs of bools (CTronClientWeapon 0x10097f40
	// clears them): [n][0] is whether the shot's subroutine n is active
	// (0..7 the weapons' own, 8 Megahurtz, 9 Corrode, 10 Primitive Charge;
	// 0x10097ab0/0x10097c30 save and load them).  Not at retail's offset
	// until the members above are fixed; here it sits in retail's +0x154
	// union of the weapons' subroutine data (the floats around it).
	bool			m_abTronUnknown13C[11][2];
	char			m_xTronUnknown17E[0x5e];
	float			m_fTronUnknown1DC;		// RETAIL: +0x1dc (Prankster)
	float			m_fTronUnknown1E0;
	float			m_fTronUnknown1E4;		// RETAIL: +0x1e4 (Cluster)
	// RETAIL: +0x1e8, +0x1f0, +0x1fc: the shot's MegaHurtzData,
	// CorrodeData and PrimitiveChargeData (SubroutineDataMgr.h)
	float			m_fTronMegaHurtzDamage;			// RETAIL: +0x1e8
	float			m_fTronMegaHurtzEnergyUse;		// RETAIL: +0x1ec
	float			m_fTronCorrodeDuration;			// RETAIL: +0x1f0
	float			m_fTronCorrodeAmount;			// RETAIL: +0x1f4
	float			m_fTronCorrodeEnergyUse;		// RETAIL: +0x1f8
	float			m_fTronPrimitiveChargeDamage;	// RETAIL: +0x1fc
	float			m_fTronPrimitiveChargeEnergyUse;	// RETAIL: +0x200
};


#endif //_ClientWeapon_h_INCLUDED_