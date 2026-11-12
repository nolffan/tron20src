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

LINKTO_MODULE( Projectile );

struct WeaponFireInfo;
class CCharacter;
class CWeapon;

class RelationData;

// RETAIL: Tron's CProjectile differs a lot from NOLF2's.  Its vtable has 46
// slots (vtable 0x10298290; NOLF2's has 39), Save/Load are not virtual
// (EngineMessageFn 0x1011e730 calls them directly: 0x10124320, 0x101246c0),
// its members are bools where NOLF2 has LTBOOLs, and it is 0x910 bytes
// (constructor 0x1011e3c0).  The order of the virtuals below is retail's;
// the names of the new ones are ours.  The members are at retail's offsets
// (the constructor and Setup 0x1011e9e0 show them); where a NOLF2 member's
// place is not proven it is put at the most likely free one (PORT: noted).
class CProjectile : public GameBase
{
	public :

		CProjectile();
		virtual ~CProjectile();		// RETAIL: out of line (0x1011e260)

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);	// slot 25
        void			SetLifeTime(LTFLOAT fTime) { m_fLifeTime = fTime; }

		void			AdjustDamage(LTFLOAT fModifier);

		HOBJECT          GetFiredFrom() const   { return m_hFiredFrom; }
		const LTVector&  GetFirePos() const     { return m_vFirePos; }
		const LTVector&  GetFireDir() const     { return m_vDir; }

		void			SetTeamId( uint8 nTeamId ) { m_nTeamId = nTeamId; }
		uint8			GetTeamId( ) const { return m_nTeamId; }
		bool			IsMyTeam( HOBJECT hPlayer ) const;

		// RETAIL: slot 26 (0x10033c80, folded): returns the bool at 0x6a.
		virtual bool	TronUnknownVirtual26() const { return m_bCanHitSameProjectileKind; }

		virtual LTBOOL	CreateDangerStimulus(HOBJECT hFiredFrom) { return LTFALSE; }	// slot 27

	protected :

		// message receiving functions
		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		// RETAIL: slot 28 (0x1011e970): sets m_fVelocity from the fire info
		// or the ammo's projectile FX.  The name is ours.
		virtual void TronUnknownVirtual28(WeaponFireInfo const& info);

		// handles any time a projectile touches something
		virtual void HandleTouch(HOBJECT hObj);		// slot 29 (0x1011fe60)

		// RETAIL: slots 30..33 (0x1011fd10, 0x1011f380, 0x1011f760,
		// 0x1011f820): not yet identified.  Slot 30 is called on touch
		// instead of HandleTouch in one case (0x1011e842).
		virtual void TronUnknownVirtual30(HOBJECT hObj);
		virtual void TronUnknownVirtual31(void *pTronUnknown1, void *pTronUnknown2);
		virtual void TronUnknownVirtual32();
		virtual void TronUnknownVirtual33(void *pTronUnknown);

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

		// RETAIL: slots 40..42 (0x10123dc0, 0x10123f80, 0x10124170): three
		// similar functions of two arguments that build a damage message.
		virtual void TronUnknownVirtual40(HOBJECT hTronUnknown1, void *pTronUnknown2);
		virtual void TronUnknownVirtual41(HOBJECT hTronUnknown1, void *pTronUnknown2);
		virtual void TronUnknownVirtual42(HOBJECT hTronUnknown1, void *pTronUnknown2);

		virtual void HandleTouchSky(HOBJECT hObj);		// slot 43

				void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
				void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		virtual LTBOOL CanSetLastFireInfo() { return LTTRUE; }	// slot 44
		// RETAIL: slot 45 (0x10120750) takes a second argument (HandleImpact
		// 0x10120730 passes 0).  Its name is ours.
		virtual void Detonate(HOBJECT hObj, bool bTronUnknown = false);

		// Check if we got deflected.
		bool	Deflect( HOBJECT hTarget, const LTVector &vFirePos, const LTVector &vImpactPos, const LTVector &vNormal );

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
		bool			m_bRemoveFromWorld;     // 0x69 Should we be removed?

		// touch considerations
		bool			m_bCanHitSameProjectileKind;  // 0x6a PORT: place guessed

		// damage
		DamageType      m_eInstDamageType;      // 0x6c Instant damage type - DT_XXX
		DamageType      m_eProgDamageType;      // 0x70 Progressive damage type - DT_XXX
		LTFLOAT         m_fInstDamage;          // 0x74 How much instant damage do we do
		LTFLOAT         m_fProgDamage;          // 0x78 How much progressive damage do we do

		bool			m_bCanTouchFiredFromObj;	// 0x7c PORT: place guessed
		uint32			m_xTronUnknown80;

		LTFLOAT         m_fRadius;              // 0x84 PORT: place guessed
		LTFLOAT         m_fStartTime;           // 0x88 PORT: place guessed
		LTFLOAT         m_fLifeTime;            // 0x8c How long do we stay around
		LTFLOAT         m_fVelocity;            // 0x90 What is our velocity
		LTFLOAT         m_fMass;                // 0x94 What is our mass
		LTFLOAT         m_fRange;               // 0x98 Vector weapon range

		LTObjRef		m_hFiredFrom;			// 0x9c Who fired us

		uint32			m_xTronUnknownAC;		// RETAIL: set from the weapon in Setup
		LTFLOAT			m_xTronUnknownB0;		// RETAIL: a time (CTronProjectile 0x10068e33)

		uint8			m_nTeamId;				// 0xb4 PORT: place guessed

		// for future optimization?
		LTBOOL          m_bNumCallsToAddImpact;	// 0xb8 PORT: place guessed


		// These are used to move through invisible brushes...

		LTBOOL          m_bProcessInvImpact;	// 0xbc PORT: place guessed
		LTVector        m_vInvisVel;			// 0xc0 PORT: place guessed
		LTVector        m_vInvisNewPos;			// 0xcc PORT: place guessed

		uint32			m_xTronUnknownD8;
		uint32			m_xTronUnknownDC;
		uint32			m_xTronUnknownE0;

		// the number of times this projectile has ricocheted
		int             m_nTotalRicochets;		// 0xe4 PORT: place guessed

		// MID_TOUCHNOTIFY can come several times a frame, this is one
		// way to detect this and avoid doing calculations multiple times
		int             m_nUpdateNum;			// 0xe8 (MID_UPDATE increments it)
		int             m_nLastRicochetUpdateNum;	// 0xec (-1)

		bool			m_xTronUnknownF0;
		// RETAIL: 0xc4 bytes the constructor does not set
		uint32			m_xTronUnknownF4;
		bool			m_bTronUnknownF8;
		bool			m_bTronUnknownF9;		// RETAIL: CProjectileBallLauncher::Setup 0x10035594
		bool			m_bTronUnknownFA;
		bool			m_bTronUnknownFB;		// RETAIL: CProjectileDrunkenDims::Setup 0x1003d914
		uint32			m_xTronUnknownFC[3];
		LTFLOAT			m_fTronUnknown108;		// RETAIL: a scale when 0xf9 is set (0x1003559e)
		uint32			m_xTronUnknown10C[0x2b];
		uint32			m_xTronUnknown1B8;		// RETAIL: an STL list (node 0x1c)
		uint32			m_xTronUnknown1BC;		// RETAIL: an STL list (node 0x8c)
		bool			m_xTronUnknown1C0;		// RETAIL: tested on touch (0x1011e82c)
		bool			m_xTronUnknown1C1;
		bool			m_xTronUnknown1C2;
		uint32			m_xTronUnknown1C4[6];

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

		RelationData	*m_pTronUnknownRelationData;	// 0x8f8 RETAIL: new'd in the constructor
		bool			m_xTronUnknown8FC;
		bool			m_xTronUnknown8FD;
		uint32			m_xTronUnknown900;
		uint32			m_xTronUnknown904;
		uint32			m_xTronUnknown908;

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
