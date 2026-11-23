
#ifndef  _WeaponFireInfo_h_INCLUDED_
#define  _WeaponFireInfo_h_INCLUDED_


#include "MsgIDs.h"

class RelationData;


// RETAIL: Tron's weapons carry eleven modifiers (subroutine effects) from
// the fire information into the projectile (CProjectile 0x10121950 copies
// them).  Each has an enable flag and a second flag; the modifier's data
// is in a union (one data block for the first eight, then three blocks of
// their own).  The sizes are retail's (the projectile's Save 0x10121c70
// writes each block whole); the names are ours.
#define TRON_NUM_WEAPON_MODIFIERS		11

struct TronWeaponModifierFlags
{
	// RETAIL: inline (CProjectile's 0x10121b00 and 0x10121c70 use them)
	void	Load( ILTMessage_Read *pMsg )
	{
		bEnabled = pMsg->Readbool();
		bTronUnknown1 = pMsg->Readbool();
	}

	void	Save( ILTMessage_Write *pMsg )
	{
		pMsg->Writebool( bEnabled );
		pMsg->Writebool( bTronUnknown1 );
	}

	bool	bEnabled;
	bool	bTronUnknown1;
};

// RETAIL: modifier 0 and 2: four floats (+8 scales the damage, 0x10121dfe)
struct TronWeaponModifier0Data
{
	float	fTronUnknown[4];
};

// RETAIL: modifier 1: 0x94 bytes (+0x88 scales the damage, 0x10121e12)
struct TronWeaponModifier1Data
{
	float	fTronUnknown[37];
};

// RETAIL: modifier 3: three floats (+4 scales the damage, 0x10121e3a)
struct TronWeaponModifier3Data
{
	float	fTronUnknown[3];
};

// RETAIL: modifiers 4 and 5: five floats (+0 scales the ball launcher's
// and the drunken dims' spread, 0x1003559e; +8 the damage, 0x10121e4e)
struct TronWeaponModifier4Data
{
	float	fScale;
	float	fTronUnknown4;
	float	fDamageScale;
	float	fTronUnknownC;
	float	fTronUnknown10;
};

// RETAIL: modifier 6: five floats (+0xc scales the damage, 0x10121e76)
struct TronWeaponModifier6Data
{
	float	fTronUnknown[5];
};

// RETAIL: modifier 7 (the prankster): the name of its projectile class data
// (CProjectilePrankster::Setup), a life time and a damage scale (0x10121e8a)
struct TronWeaponModifier7Data
{
	char	szClassData[0x80];
	float	fLifeTime;
	float	fDamageScale;
	float	fTronUnknown88;
};

union TronWeaponModifierData
{
	TronWeaponModifier0Data	m0;
	TronWeaponModifier1Data	m1;
	TronWeaponModifier3Data	m3;
	TronWeaponModifier4Data	m4;
	TronWeaponModifier6Data	m6;
	TronWeaponModifier7Data	m7;
};

// RETAIL: modifiers 8 and 10: two floats (the first scales the damage,
// 0x10121eae, 0x10121ede); saved as one 64 bit value (0x10125010)
struct TronWeaponModifier8Data
{
	float	fDamageScale;
	float	fTronUnknown4;
};


// RETAIL: Tron's WeaponFireInfo is 0x16c bytes; its constructor is
// 0x1010fea0.  The offsets are retail's.
struct WeaponFireInfo
{
	WeaponFireInfo()
		: hFiredFrom( LTNULL )
		, hTestObj( LTNULL )
		, vPath()
		, vFirePos()
		, vFlashPos()
		, nSeed( 0 )
		, bAltFire( LTFALSE )
		, fPerturbU( 1.0f )	// 0.0 - 1.0
		, fPerturbR( 1.0f )	// 0.0 - 1.0
		, bOverrideVelocity( LTFALSE )
		, fOverrideVelocity( 0.0f )
		, bOverrideRange( LTFALSE )
		, fOverrideRange( 0.0f )
		, fFireTime( 0.0f )
		, pRelationData( LTNULL )
		, hClientObjImpact( INVALID_HOBJECT )
		, nTronUnknown64( 0 )
		, nDiscTrackingType( 0 )
		, vPointTarget()
		, vControlDirection()
		, vControlPosition()
		, hObjectTarget( 0 )
		, hSocket( INVALID_MODEL_SOCKET )
		, fDiscReleaseAngle( 0.0f )
		, bDiscAutoAim( false )
		, fDamageMultiplier( 1.0f )
	{
		for ( int i = 0; i < TRON_NUM_WEAPON_MODIFIERS; ++i )
		{
			aModifiers[i].bEnabled = false;
			aModifiers[i].bTronUnknown1 = false;
		}
	}

	HOBJECT     hFiredFrom;				// 0x00
	HOBJECT     hTestObj;				// 0x04
	LTVector    vPath;					// 0x08
	LTVector    vFirePos;				// 0x14
	LTVector    vFlashPos;				// 0x20
	uint8       nSeed;					// 0x2c
	LTBOOL      bAltFire;				// 0x30
	LTFLOAT     fPerturbU;				// 0x34
	LTFLOAT     fPerturbR;				// 0x38
	LTBOOL      bOverrideVelocity;		// 0x3c (CProjectile 0x1011e977)
	LTFLOAT     fOverrideVelocity;		// 0x40

	// RETAIL: Tron's: a range that replaces the weapon's (CProjectile::Setup
	// 0x1011ebc8).  The names are ours
	LTBOOL      bOverrideRange;			// 0x44
	LTFLOAT     fOverrideRange;			// 0x48

	// RETAIL: Tron keeps the fire time as a float (CWeapon::UpdateWeapon
	// 0x10157528; NOLF2 has a uint32 nFireTimestamp in milliseconds)
	LTFLOAT     fFireTime;				// 0x4c

	// RETAIL: Tron's: the relation data of the shooter, copied into the
	// projectile (CProjectile::Setup 0x1011eaa2)
	RelationData *pRelationData;		// 0x50

	LTVector	vClientObjImpactPos;	// 0x54
	HOBJECT		hClientObjImpact;		// 0x60

	// RETAIL: tested by CWeapon::Fire (0x101576c0).  Not yet identified
	uint32		nTronUnknown64;			// 0x64

	// RETAIL: Tron's weapon modifiers (see above)
	TronWeaponModifierFlags	aModifiers[TRON_NUM_WEAPON_MODIFIERS];	// 0x68
	TronWeaponModifierData	ModifierData;	// 0x80
	TronWeaponModifier8Data	Modifier8;		// 0x114
	LTVector				vModifier9;		// 0x11c
	TronWeaponModifier8Data	Modifier10;		// 0x128

	//
	// Disc Information
	//

	// type of disc tracking
	uint8       nDiscTrackingType;		// 0x130

	// point needed if the disc is homing in on a point
	LTVector    vPointTarget;			// 0x134

	// control vector to help with controlling the disc
	LTVector    vControlDirection;		// 0x140

	// control location to help with controlling the disc
	LTVector    vControlPosition;		// 0x14c

	// an object needed if the disc is homing in on an object
	HOBJECT     hObjectTarget;			// 0x158

	// Optional socket for the disc to return to.
	HMODELSOCKET hSocket;				// 0x15c

	// When the disc is released, instead of sending
	// it straight forward, throw it off center at this
	// angle (specified in RADIANS).  Positive values
	// mean "right", negative values mean "left".
	// NOTE: Only used by the player.
	LTFLOAT     fDiscReleaseAngle;		// 0x160

	// RETAIL: Tron's: the player's "DiscAutoAim" setting, or in a disc
	// game the server's option (CTronPlayerObj::GetExtraFireMessageInfo
	// 0x100633f7); CDisc::Setup (0x100375eb) keeps it.  NOLF2 has the
	// cluster disc's shard values here.  The name is ours
	bool		bDiscAutoAim;			// 0x164

	// RETAIL: Tron's: scales the projectile's instant damage (CProjectile::
	// Setup 0x1011eb74).  The name is ours
	LTFLOAT		fDamageMultiplier;		// 0x168
};



#endif //_WeaponFireInfo_h_INCLUDED_
