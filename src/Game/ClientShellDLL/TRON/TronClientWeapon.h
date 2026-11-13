// ----------------------------------------------------------------------- //
//
// MODULE  : TronClientWeapon.h
//
// PURPOSE : Tron's common base of its client weapons (retail only: no
//           source of it survives; RTTI .?AVCTronClientWeapon@@, vtable
//           0x101e8884, 100 slots; CShell.dll 0x10097060..).
//
//           Every Tron client weapon derives from it (RTTI): CClientWeaponDisc
//           (and its ClusterDisc/SequencerDisc), Ball, BallLauncher, Blaster,
//           ContinuousFireBase (PRod, EnergyClaw), DrunkenDims, LOL, Prankster
//           and Suffusion.  The names of its members and new virtuals are
//           ours unless said otherwise.
//
// ----------------------------------------------------------------------- //

#ifndef  _TronClientWeapon_h_INCLUDED_
#define  _TronClientWeapon_h_INCLUDED_

#include "ClientWeapon.h"
#include "SubroutineDataMgr.h"

// RETAIL: the fire message types Tron's weapons return from
// GetFireMessageType (slot 70); the names are ours.  MWEAPFIRE_VECTOR (0)
// and MWEAPFIRE_PROJECTILE (1) are as in MsgIDs.h; Tron's disc is 3.
enum TronWeaponFireType
{
	kTronFire_Disc			= 3,	// RETAIL: 0x10018160
	kTronFire_PRod			= 6,	// RETAIL: 0x10099460
	kTronFire_Suffusion		= 7,	// RETAIL: 0x1001bd20
	kTronFire_LOL			= 8,	// RETAIL: 0x1001acd0
	kTronFire_Ball			= 9,	// RETAIL: 0x10017840
	kTronFire_BallLauncher	= 10,	// RETAIL: 0x10017c30
	kTronFire_DrunkenDims	= 11,	// RETAIL: 0x1001a440
	kTronFire_EnergyClaw	= 13,	// RETAIL: 0x1001a610
	kTronFire_Prankster		= 14,	// RETAIL: 0x1001b0d0
};

class CTronClientWeapon : public CClientWeapon
{
public:
	CTronClientWeapon();
	virtual ~CTronClientWeapon();

	// RETAIL: slot 1 (0x100970e0)
	virtual bool  OnMessage( uint8 messageID, ILTMessage_Read *pMsg );

	// RETAIL: slot 7 (0x10097110).  Retail's takes three arguments (ret 0xc;
	// so does CClientWeapon's, 0x10130680); ours keeps NOLF2's two.
	virtual WeaponState   Update( bool bFire, FireType eFireType = FT_NORMAL_FIRE );

	// RETAIL: slot 18 (0x100971e0)
	virtual void  Select();

	// RETAIL: slots 45/46 (0x10097160, 0x100971a0)
	virtual void  Load( ILTMessage_Read *pMsg );
	virtual void  Save( ILTMessage_Write *pMsg );

protected:
	// RETAIL: slot 57 (0x10097a20): firing spends the shot's energy
	virtual void  Fire( bool bFire );

	// RETAIL: slot 89 (0x10098ba0): whether the player's energy covers a shot
	virtual bool  TronUnknownVirtual164();

	//
	// RETAIL: the new virtuals, slots 90..99 (names ours)
	//

	// slot 90 (0x10097200): the energy a shot uses.  The weapons add their
	// subroutine's fEnergyUseMultiplier times GetBaseEnergyUse() to it.
	virtual float GetEnergyUse();
	// slot 91 (0x101c8210, shared "return false")
	virtual bool  TronUnknownVirtual16C() { return false; }
	// slot 92 (0x100976c0)
	virtual bool  TronUnknownVirtual170();
	// slot 93 (0x100978f0): OnMessage hands it MID_PROJECTILE
	virtual bool  HandleMessageProjectile( uint8 messageID, ILTMessage_Read *pMsg );
	// slot 94 (0x10097c30): Load's helper
	virtual void  TronUnknownVirtual178( ILTMessage_Read *pMsg );
	// slot 95 (0x101c9040, shared empty)
	virtual void  TronUnknownVirtual17C() {}
	// slot 96 (0x10098b30)
	virtual void  TronUnknownVirtual180();
	// slot 97 (0x10098be0)
	virtual bool  TronUnknownVirtual184();
	// slot 98 (0x100d1580, shared "return true")
	virtual bool  TronUnknownVirtual188() { return true; }
	// slot 99 (0x10098d10): Update stops the weapon while it is true
	virtual bool  TronUnknownVirtual18C();

	//
	// RETAIL: non-virtual helpers the derived weapons call (names ours)
	//

	// 0x10098c00: the subroutine data key of the named utility subroutine
	SubroutineDataKey GetSubroutineDataKey( const char *szName );
	// 0x10098c90: the weapon's energy use, scaled by a rating
	float         GetBaseEnergyUse();
	// 0x10098b20
	void          TronUnknown98B20();
	// 0x10097f40 (the constructor and Load call it)
	void          TronUnknown97F40();

	float         m_fTronUnknown204;	// RETAIL: +0x204 (0 in the ctor)
};

#endif //_TronClientWeapon_h_INCLUDED_
