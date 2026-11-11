/****************************************************************************
;
;	MODULE:			TronPlayerObj.h
;
;	PURPOSE:		Tron-specific player object
;
;	HISTORY:		1/28/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/


#ifndef _TRONPLAYEROBJ_H_
#define _TRONPLAYEROBJ_H_

#include "PlayerObj.h"
#include "Ratings.h"
#include "Weapons.h"
#include <slist>
#include <string>

class CTronPlayerObj : public CPlayerObj
{
	public :

		// Constructor/destructor
        CTronPlayerObj();
		virtual ~CTronPlayerObj();

		// Performance Ratings
		// RETAIL: virtual in Tron (retail CTronPlayerObj's slots 132 to 134,
		// 0x100629b0 to 0x100629e0); a rating surges at its own threshold
		virtual int		GetPerformanceRating(PerformanceRating p) const { ASSERT((p >= 0) && (p < NUM_RATINGS)); return m_iPerformanceRatings[p]; }
		virtual void	SetPerformanceRating(PerformanceRating p, uint8 i) { ASSERT((p >= 0) && (p < NUM_RATINGS)); m_iPerformanceRatings[p] = i; }
		virtual LTBOOL	IsSurgePerformanceRating(PerformanceRating p) const { ASSERT((p >= 0) && (p < NUM_RATINGS)); return (m_iPerformanceRatings[p] >= m_iSurgePerformanceRatings[p]); }

		// RETAIL: another virtual of Tron's (slot 135, retail 0x10062a00)
		virtual float	TronUnknownVirtual135() { return m_fTronUnknown31f4; }

		// MSVC6 does NOT support templated member functions.  This function
		// belongs here but cannot due to this limitation.  This comment is
		// here to remind anyone looking for performance ratings that this
		// function exitst and is ready to go.  See the bottom of this file
		// for the implementation.
		//
		// template <typename T>
		// T InterpolatePerformanceRating( PerformanceRating ePerformanceRating,
		//                                 T tMinValue, 
		//                                 T tMaxValue,
		//                                 T tSurgeValue )

		// Permission sets
		// RETAIL: Tron keeps them in CCharacter (0xc20, retail
		// CTronPlayerObj's constructor clears it)
		uint8	GetPSets() { return m_iPermissionSet; }
		void	SetPSets(uint8 byPSets) { m_iPermissionSet = byPSets; }
		void	AddPSets(uint8 byPSets) { m_iPermissionSet |= byPSets; }
		void	RemovePSets(uint8 byPSets) { m_iPermissionSet &= ~byPSets; }

		// Specific messages from the client
		void HandleCompile(ILTMessage_Read *pMsg);

		// Defense
		virtual bool	IsDefending() const;
		virtual float	GetDefensePercentage(  LTVector const *pIncomingProjectilePosition = 0 ) const;

		// damage filtering
		virtual bool	FilterDamage( DamageStruct *pDamageStruct );

		// RETAIL: no HandleDamage override in Tron (its slot 106 is
		// CPlayerObj's)

		// RETAIL: Tron's overrides of CPlayerObj's new virtuals
		virtual void	Respawn(uint8 nServerLoadGameFlags=LOAD_NEW_LEVEL, bool bGetStartPt=true,
			GameStartPoint* pStartPoint=LTNULL, GameBase* pSpawnAt=LTNULL);
		virtual void	RemoveVehicleModel();
		virtual void	GimmeGunCheat( uint8 nId );
		virtual void	TronUnknownVirtual105( LTBOOL bOn );
		virtual void	AcquireWeapon(char* pWeaponName, bool bTronUnknown = false);
		virtual bool	TronUnknownVirtual128( WEAPON const* pWeapon );
		virtual bool	TronUnknownVirtual129( uint8 nWeaponId );

	protected:

		// engine hook for object to object messages
        virtual uint32 ObjectMessageFn(HOBJECT, ILTMessage_Read *);

		bool HandleMessageProjectile( HOBJECT hSender,
		                              ILTMessage_Read *pMsg );

		// Load/Save
		virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// get game specific info out of the fire message
		virtual void GetExtraFireMessageInfo(
					uint8 nWeaponType,
					ILTMessage_Read *pMsg,
					WeaponFireInfo *pInfo
				);

		// Update functions
        virtual LTBOOL Update();
		virtual void UpdateInterface( bool bForceUpdate ); // Send client interface info.

		// Message functions
        virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		// Verify functions
		bool CheckSubState(char const* pState);
		bool CheckSubCondition(char const* pCondition);

		// Inventory acquire functions
		bool AcquireSubroutine(char const* pName, char const* pState, char const* pCondition);
		bool AcquireAdditive(char const* pName);
		bool AcquireProcedural(char const* pName);
		bool AcquirePrimitive(char const* pName);

		// Light cycle functions
		bool StartLightCycle( bool bSuper, bool bTronUnknown );	// RETAIL: two arguments (0x10067420)
		bool EndLightCycle();
		void SetVehiclePhysicsModel(PlayerPhysicsModel eModel);
		// RETAIL: no SetNormalPhysicsModel override in Tron (its slot 124 is
		// CPlayerObj's)

		// invalidates the vars that keep of the block
		void ResetDefend();

		// RETAIL: retail 0x10068180 (InitialUpdate calls it). The name is ours
		void CreateSwatWeapon();
		virtual LTBOOL InitialUpdate(int nInfo);

		// RETAIL: retail 0x100675a0 (Load calls it with false). The names are ours
		void AddPing(char const* pszPing, bool bTronUnknown);

		// Member vars
		// RETAIL: laid out from retail CTronPlayerObj's constructor (0x100626d0),
		// Save and Load (0x10063030, 0x100631c0); 0x3300 bytes. Names not in
		// Build 69 are ours
		uint8	m_byOldPSets;					// 0x31e8
		uint16	m_nBuildPoints;					// 0x31ea
		uint16	m_nOldBuildPoints;				// 0x31ec
		float	m_fTronUnknown31f0;				// RETAIL: scales energy (Update)
		float	m_fTronUnknown31f4;				// RETAIL: caps energy (Update)
		float	m_fTronUnknown31f8;
		float	m_fTronUnknown31fc;
		float	m_fOldMaxHitPoints;				// RETAIL: -1 until sent (UpdateInterface)
		float	m_fOldMaxEnergy;
		float	m_afChanceToBeCorruptedByInfectedAmmo[4];	// RETAIL: one per difficulty (InventoryChanceToBeCorruptedByInfectedAmmo*)

		// for defending
		int			m_nDefendClientTimeStarted;	// 0x3218
		int			m_nDefendServerTimeStarted;
		int			m_nDefendDuration;
		int8		m_cDefendType;
		uint8		m_cDefendAmmoId;

		typedef std::slist<std::string> PingList;
		PingList	m_lstPings;		// RETAIL: 0x3228 ("AddPing", 0x100675c5)
		LTObjRef	m_hLightCycle;					// RETAIL: 0x322c (RemoveVehicleModel removes it)
		char		m_szLightCycleModel[32];		// RETAIL: 0x323c ("LightCycleBlue")
		bool		m_bTronUnknown325c;
		uint8		m_iPerformanceRatings[NUM_RATINGS];		// 0x325d
		uint8		m_iSurgePerformanceRatings[NUM_RATINGS];	// 0x3262
		char		m_szTronUnknown3267[64];
		char		m_szTronUnknown32a7[64];
		CWeapon*	m_pSwatWeapon;				// RETAIL: the "SwatWeapon" the player blocks with
		CWeapons*	m_pSwatWeapons;				// (0x10068180). The names are ours
		int			m_nLastWeaponId;			// RETAIL: Respawn keeps the current weapon's ids
		int			m_nLastAmmoId;				// (0x10067264). The names are ours
		bool		m_bTronUnknown32f8;
		bool		m_bTronUnknown32f9;
		bool		m_bTronUnknown32fa;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InterpolatePerformanceRating()
//
//	PURPOSE:	Using the specificed rating, map the value to a given range.
//				If the rating is in Surge mode, return the surge value.
//				Otherwise, interpolate between the min and max values.
//
//				NOTE: this belongs in the CTronPlayerObj class, but MSVC6
//				doesn't support member templates so this function must be
//				placed externally.  I'll place in here to keep it close to
//				where it is supposed to go.
//
// ----------------------------------------------------------------------- //

template <typename T>
T InterpolatePerformanceRating( PerformanceRating ePerformanceRating,
                                T tMinValue, 
                                T tMaxValue,
                                T tSurgeValue )
{
	// get the player object
	CTronPlayerObj const *pPlayer =
		dynamic_cast< CTronPlayerObj* >( g_pCharacterMgr->FindPlayer() );

	if ( pPlayer->IsSurgePerformanceRating( ePerformanceRating ) )
	{
		//
		// rating is in surge mode
		//
		
		// return the surge value
		return tSurgeValue;
	}
	else
	{
		//
		// rating is NOT in surge mode
		//

		// get the current rating
		uint8 nCurrentRating = pPlayer->GetPerformanceRating( ePerformanceRating );
		ASSERT( ( 0 <= nCurrentRating ) && ( 100 >= nCurrentRating ) );

		// return the interpolated value 
		return tMinValue + ( tMaxValue - tMinValue ) * nCurrentRating / 100;
	}
}


#endif  // _TRONPLAYEROBJ_H_
