// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.h
//
// PURPOSE : Projectile class - definition
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "GameBase.h"
#include "DamageTypes.h"
#include "SurfaceMgr.h"
#include "WeaponMgr.h"
#include "Destructible.h"
#include "ModelButeMgr.h"
#include "LTObjRef.h"
#include "ImpactType.h"
#include "WeaponFireInfo.h"

LINKTO_MODULE( Projectile );

struct WeaponFireInfo;
class CCharacter;
class CWeapon;

class RelationData;

// RETAIL: an object a projectile has damaged, and when (the list keeps a
// projectile from damaging an object twice: 0x10121ef0, 0x101220a0,
// 0x10122020).  The names are ours
struct ProjectileDamagedObject
{
	ProjectileDamagedObject( HOBJECT hObj, LTFLOAT fDamageTime )
		: hObject( hObj ), fTime( fDamageTime ) {}

	LTObjRef	hObject;
	LTFLOAT		fTime;
};

typedef std::list<ProjectileDamagedObject> ProjectileDamagedObjectList;

// RETAIL: a touch the new collision code keeps until the projectile's next
// update (0x1011f940 adds one, 0x1011f6f0 finds the nearest, the update
// slot 32 handles them).  0x84 bytes, saved whole.  The names are ours
struct ProjectilePendingTouch
{
	ProjectilePendingTouch() {}
	ProjectilePendingTouch( HOBJECT hObj, IntersectInfo const& iTouchInfo, LTFLOAT fDamage,
		LTFLOAT fDist, LTVector const& vPos, LTVector const& vVel, CollisionInfo const& cInfo )
		: hObject( hObj ), iInfo( iTouchInfo ), fInstDamage( fDamage ), fDistSqr( fDist ),
		  vLastPos( vPos ), vLastVel( vVel ), collInfo( cInfo ) {}

	LTObjRef		hObject;			// 0x00
	IntersectInfo	iInfo;				// 0x10
	LTFLOAT			fInstDamage;		// 0x3c
	LTFLOAT			fDistSqr;			// 0x40 from the last position
	LTVector		vLastPos;			// 0x44
	LTVector		vLastVel;			// 0x50
	CollisionInfo	collInfo;			// 0x5c
};

typedef std::list<ProjectilePendingTouch> ProjectilePendingTouchList;

// RETAIL: Tron's CProjectile differs a lot from NOLF2's.  Its vtable has 46
// slots (vtable 0x10298290; NOLF2's has 39), Save/Load are not virtual
// (EngineMessageFn 0x1011e730 calls them directly: 0x10124320, 0x101246c0),
// its members are bools where NOLF2 has LTBOOLs, and it is 0x910 bytes
// (constructor 0x1011e3c0).  The order of the virtuals below is retail's;
// the names of the new ones are ours.  The members are at retail's offsets
// (the constructor, Setup 0x1011e9e0 and Save 0x10124320 show them).
class CProjectile : public GameBase
{
	// RETAIL: PowerArmor::HandleProjectileImpact reads and sets its damage
	// (Object.lto 0x1013bb99). Ours
	friend class PowerArmor;

	public :

		CProjectile();
		virtual ~CProjectile();		// RETAIL: out of line (0x1011e260)

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);	// slot 25
        void			SetLifeTime(LTFLOAT fTime) { m_fLifeTime = fTime; }

		void			AdjustDamage(LTFLOAT fModifier);

		// RETAIL: Tron's damage scales by the projectile's modifier flags
		// (0x10121df0, 0x10121ea0, 0x10121ec0; PowerArmor's damage check
		// calls them). The names are ours
		void			ScaleTronDamage(LTFLOAT* pfDamage);
		void			ScaleTronDamage101(LTFLOAT* pfDamage);
		void			ScaleTronDamage105(LTFLOAT* pfDamage);

		HOBJECT          GetFiredFrom() const   { return m_hFiredFrom; }

		// PORT: Tron's CProjectile keeps no team id; NOLF2's projectile types
		// (ProjectileTypes.cpp, where these are) still use one, so these
		// take it from the shooter
		uint8			GetTeamId() const;
		bool			IsMyTeam( HOBJECT hPlayer ) const;
		const LTVector&  GetFirePos() const     { return m_vFirePos; }
		const LTVector&  GetFireDir() const     { return m_vDir; }

		// RETAIL: slot 26 (0x10033c80, folded): returns the bool at 0x6a.
		virtual bool	TronUnknownVirtual26() const { return m_bCanTouchFiredFromObj; }

		virtual LTBOOL	CreateDangerStimulus(HOBJECT hFiredFrom) { return LTFALSE; }	// slot 27

		// RETAIL: the new collision code's intersect filter (0x1011edf0; the
		// filter function 0x1011e240 calls it).  The name is ours
		bool			TouchFilter(HOBJECT hObj);

	protected :

		// message receiving functions
		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		// RETAIL: slot 28 (0x1011e970): sets m_fVelocity from the fire info
		// or the ammo's projectile FX.  The name is ours.
		virtual void TronUnknownVirtual28(WeaponFireInfo const& info);

		// handles any time a projectile touches something
		virtual void HandleTouch(HOBJECT hObj);		// slot 29 (0x1011fe60)

		// RETAIL: slots 30..33 are Tron's new collision code (the cvar
		// "ProjectileNewCollision" chooses it).  Slot 30 (0x1011fd10) keeps
		// a touch for the next update, slot 31 (0x1011f380) is
		// CanCharacterHitCharacter, slot 32 (0x1011f760) handles the kept
		// touches in the update, slot 33 (0x1011f820) handles one of them.
		// The names are ours.
		virtual void TronUnknownVirtual30(HOBJECT hObj);
		virtual bool TronUnknownVirtual31(HOBJECT hAttacker, HOBJECT hVictim);
		virtual void TronUnknownVirtual32();
		virtual bool TronUnknownVirtual33(ProjectilePendingTouch *pTouch);

		virtual void AddImpact( HOBJECT hObj,				// slot 34
		                        const LTVector &vFirePos, 
		                        const LTVector &vImpactPos, 
		                        const LTVector &vNormal, 
		                        SurfaceType eSurfaceType=ST_UNKNOWN,
		                        IMPACT_TYPE eImpactType = IMPACT_TYPE_IMPACT,
								bool bDeflected = false);
		virtual void AddExplosion(const LTVector &vPos, const LTVector &vNormal);	// slot 35

		virtual void AddSpecialFX();				// slot 36
		virtual void RemoveObject();				// slot 37
		
				void CreateSpecialFX( );

		virtual void HandleImpact(HOBJECT hObj);		// slot 38
		virtual void ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj);	// slot 39

		// RETAIL: slots 40..42 (0x10123dc0, 0x10123f80, 0x10124170): the
		// instant, the progressive and the modifier 9 damage that
		// ImpactDamageObject does.  The names are ours.
		virtual void TronUnknownVirtual40(HOBJECT hDamager, HOBJECT hObj);
		virtual void TronUnknownVirtual41(HOBJECT hDamager, HOBJECT hObj);
		virtual void TronUnknownVirtual42(HOBJECT hDamager, HOBJECT hObj);

		virtual void HandleTouchSky(HOBJECT hObj);		// slot 43

				void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
				void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		virtual LTBOOL CanSetLastFireInfo() { return LTTRUE; }	// slot 44
		// RETAIL: slot 45 (0x10120750) takes the collision to use (the kept
		// touch's, 0x1011f923; HandleImpact 0x10120730 passes none, and
		// Detonate then asks the engine for the last one).
		virtual void Detonate(HOBJECT hObj, CollisionInfo* pCollInfo = LTNULL);

		// RETAIL: Tron moved these from CDisc to here (0x10123a80, 0x10123b90)
		// handles everything necessary to send request to the owner
		// to stop or start sending projectile messages
		void	RequestStartSendingMessages();
		void	RequestStopSendingMessages();

		// RETAIL: Tron's helpers (the names are ours)
		bool	IsIgnoringProjectiles(HOBJECT hObj);					// 0x1011f310
		bool	IsFiredFromHitBox(HOBJECT hObj);						// 0x1011f3a0
		bool	IsSameKindFromSameShooter(HOBJECT hObj);				// 0x1011f400
		bool	IsNonSolidTouch(HOBJECT hObj);							// 0x1011f470
		bool	GetHitBoxImpact(HOBJECT hObj, HOBJECT *phObj, IntersectInfo *pInfo);	// 0x1011f4e0
		bool	TestTouchSegment(HOBJECT hObj, IntersectInfo *pInfo);	// 0x1011f5f0
		ProjectilePendingTouch*	GetNearestPendingTouch(LTFLOAT *pfDistSqr);	// 0x1011f6f0
		bool	AddPendingTouch(HOBJECT hObj, IntersectInfo *pInfo);	// 0x1011f940
		void	LoadPendingTouches(ILTMessage_Read *pMsg);				// 0x1011fb50
		void	SavePendingTouches(ILTMessage_Write *pMsg);				// 0x1011fca0
		void	CopyModifiers(WeaponFireInfo const& info);				// 0x10121950
		void	LoadModifiers(ILTMessage_Read *pMsg);					// 0x10121b00
		void	SaveModifiers(ILTMessage_Write *pMsg);					// 0x10121c70
		void	AddDamagedObject(HOBJECT hObj);							// 0x10121ef0
		void	UpdateDamagedObjects();									// 0x10122020
		bool	IsDamagedObject(HOBJECT hObj);							// 0x101220a0
		void	LoadDamagedObjects(ILTMessage_Read *pMsg);				// 0x10122120
		void	SaveDamagedObjects(ILTMessage_Write *pMsg);				// 0x10122240
		void	UpdatePendingPos();										// 0x101222b0
		bool	DoVectorTrace(LTVector vFrom, LTVector vTo);			// 0x101222e0

		LTVector        m_vFlashPos;            // 0x38 Where the fired from special fx should be created
		LTVector        m_vFirePos;             // 0x44 Where were we fired from
		LTVector        m_vDir;                 // 0x50 What direction our we moving

		// special conditions
		bool			m_bSilenced;            // 0x5c This weapon silenced?

		// state of object
		bool			m_bObjectRemoved;       // 0x5d Have we been removed?
		bool			m_bDetonated;           // 0x5e Have we detonated yet?

		// weapon/ammo ids
		int             m_nWeaponId;            // 0x60 Type of weapon
		int             m_nAmmoId;              // 0x64 Type of ammo

		bool			m_bSetup;				// 0x68 Has this projectile been setup properly?

		// touch considerations
		bool			m_bCanHitSameProjectileKind;	// 0x69
		bool			m_bCanTouchFiredFromObj;		// 0x6a

		// damage
		DamageType      m_eInstDamageType;      // 0x6c Instant damage type - DT_XXX
		DamageType      m_eProgDamageType;      // 0x70 Progressive damage type - DT_XXX
		LTFLOAT         m_fInstDamage;          // 0x74 How much instant damage do we do
		LTFLOAT         m_fProgDamage;          // 0x78 How much progressive damage do we do

		// RETAIL: modifier 9's damage (type 0x22: TronUnknownVirtual42
		// 0x10124170), its amount and its duration (Setup 0x1011ec90)
		bool			m_bTronUnknown7C;		// 0x7c
		LTFLOAT			m_fTronUnknown80;		// 0x80
		LTFLOAT			m_fTronUnknown84;		// 0x84

		LTFLOAT         m_fStartTime;           // 0x88
		LTFLOAT         m_fLifeTime;            // 0x8c How long do we stay around
		LTFLOAT         m_fVelocity;            // 0x90 What is our velocity
		LTFLOAT         m_fMass;                // 0x94 What is our mass
		LTFLOAT         m_fRange;               // 0x98 Vector weapon range

		LTObjRef		m_hFiredFrom;			// 0x9c Who fired us

		// for future optimization?
		LTBOOL          m_bNumCallsToAddImpact;	// 0xac

		LTFLOAT			m_xTronUnknownB0;		// 0xb0 RETAIL: the time of the last update (0x1011f29b)

		// These are used to move through invisible brushes...

		bool			m_bProcessInvImpact;	// 0xb4
		LTVector        m_vInvisVel;			// 0xb8
		LTVector        m_vInvisNewPos;			// 0xc4

		LTVector		m_vTronUnknownD0;		// 0xd0 RETAIL: where Detonate was (0x10120dc3)

		// RETAIL: Tron copies the ricochet limits from the projectile FX
		// (Setup 0x1011ec6f)
		LTFLOAT			m_fMaxRicochetAngle;	// 0xdc
		int				m_nMaxRicochets;		// 0xe0

		// the number of times this projectile has ricocheted
		int             m_nTotalRicochets;		// 0xe4

		// MID_TOUCHNOTIFY can come several times a frame, this is one
		// way to detect this and avoid doing calculations multiple times
		int             m_nUpdateNum;			// 0xe8 (MID_UPDATE increments it)
		int             m_nLastRicochetUpdateNum;	// 0xec (-1)

		// keep track if we've requested messages or not
		// (to cut down no message traffic)
		bool			m_bReceivingMessages;	// 0xf0 RETAIL: Tron moved it here from CDisc

		// RETAIL: the weapon modifiers, copied from the fire info (see
		// WeaponFireInfo.h); the constructor does not set them
		TronWeaponModifierFlags	m_aModifiers[TRON_NUM_WEAPON_MODIFIERS];	// 0xf1
		TronWeaponModifierData	m_ModifierData;		// 0x108
		TronWeaponModifier8Data	m_Modifier8;		// 0x19c
		LTVector				m_vModifier9;		// 0x1a4
		TronWeaponModifier8Data	m_Modifier10;		// 0x1b0

		ProjectileDamagedObjectList	m_lstDamagedObjects;	// 0x1b8
		ProjectilePendingTouchList	m_lstPendingTouches;	// 0x1bc

		// RETAIL: the new collision code's state (Update 0x1011f1f0): set
		// while the kept touches are handled, and two "touch not taken"
		// flags (0x1011eeaf, 0x1011fe44)
		bool			m_bTronUnknown1C0;		// 0x1c0
		bool			m_bTronUnknown1C1;		// 0x1c1
		bool			m_bTronUnknown1C2;		// 0x1c2

		// RETAIL: the position and the velocity at the end of the last update
		LTVector		m_vLastPos;				// 0x1c4
		LTVector		m_vLastVel;				// 0x1d0

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		// Member Variables

		CDestructible   m_damage;               // 0x1e0 Handle damage

		LTVector        m_vDims;                // 0x8c8 Model dimensions

		uint32          m_dwFlags;              // 0x8d4 Model flags
		DamageFlags     m_nCantDamageFlags;    // 0x8d8 What type of damage can't damage us...

		WEAPON const   *m_pWeaponData;			// 0x8e0
		AMMO const     *m_pAmmoData;			// 0x8e4

		// Client-side hit detection support
		LTVector		m_vClientObjImpactPos;	// 0x8e8
		HOBJECT			m_hClientObjImpact;		// 0x8f4

		// RETAIL: the shooter's relation data (new'd in the constructor, set
		// in Setup 0x1011eaa2) and whether the fire info had one
		RelationData	*m_pRelationData;		// 0x8f8
		bool			m_bHasRelationData;		// 0x8fc

		// RETAIL: a position to move to on the next update (0x101222b0)
		bool			m_bTronUnknown8FD;		// 0x8fd
		LTVector		m_vTronUnknown900;		// 0x900

	private :

		// update functions
		void InitialUpdate(int nInfo);
		void Update();

		// 
		void DoProjectile();
		void DoVector();

		LTBOOL TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType);

		LTBOOL HandleVectorImpact(IntersectInfo & iInfo, LTVector & vFrom, LTVector & vTo);
		LTBOOL HandlePotentialHitBoxImpact(IntersectInfo & iInfo, LTVector & vFrom);

		LTBOOL UpdateDoVectorValues(SURFACE & surf, LTFLOAT fThickness, LTVector vImpactPos,
		                            LTVector & vFrom, LTVector & vTo);

		LTBOOL CalcInvisibleImpact(IntersectInfo & iInfo, SurfaceType & eSurfType);

		LTBOOL HandleClientVectorImpact();
};


#endif //  __PROJECTILE_H__
