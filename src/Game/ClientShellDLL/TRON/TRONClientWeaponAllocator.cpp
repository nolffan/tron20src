
#include "stdafx.h"

#include "StandardCreator.h"
#include "TronClientWeaponAllocator.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponNone.h"
#include "TronClientWeapon.h"
#include "ClientWeaponDisc.h"
#include "ClientWeaponSequencerDisc.h"
#include "ClientWeaponClusterDisc.h"
#include "ClientWeaponPRod.h"
#include "ClientWeaponSuffusion.h"
#include "ClientWeaponLOL.h"
#include "ClientWeaponBall.h"
#include "ClientWeaponBallLauncher.h"
#include "ClientWeaponDrunkenDims.h"
#include "ClientWeaponBlaster.h"
#include "ClientWeaponEnergyClaw.h"
#include "ClientWeaponPrankster.h"

//
// Typedefs
//

// These are the various types of client weapons available
// in this game.  Be VERY VERY careful about changing these,
// or changing their order.  There are data files that rely
// on this ordering to create the proper weapon.
//
// RETAIL: Tron's 15 types (retail CShell.dll 0x10098fe0 checks against
// 15; the table at 0x1021c3f4). They are the fire message types the
// weapons send (TronWeaponFireType); the names after CLUSTERDISC are ours.
typedef enum _CLIENT_WEAPON_TYPE
{
	CLIENT_WEAPON_TYPE_NONE,  // this is invalid
	CLIENT_WEAPON_TYPE_VECTOR,
	CLIENT_WEAPON_TYPE_PROJECTILE,
	CLIENT_WEAPON_TYPE_DISC,
	CLIENT_WEAPON_TYPE_SEQUENCERDISC,
	CLIENT_WEAPON_TYPE_CLUSTERDISC,
	CLIENT_WEAPON_TYPE_PROD,
	CLIENT_WEAPON_TYPE_SUFFUSION,
	CLIENT_WEAPON_TYPE_LOL,
	CLIENT_WEAPON_TYPE_BALL,
	CLIENT_WEAPON_TYPE_BALLLAUNCHER,
	CLIENT_WEAPON_TYPE_DRUNKENDIMS,
	CLIENT_WEAPON_TYPE_BLASTER,
	CLIENT_WEAPON_TYPE_ENERGYCLAW,
	CLIENT_WEAPON_TYPE_PRANKSTER,

	NUM_CLIENT_WEAPON_TYPES  // always last!
} CLIENT_WEAPON_TYPE;


// Here is a list of the templated classes that will
// be used to create the specific ClientWeapon.
// Template parameters are < BaseClass, DerivedClass >
//
// RETAIL: in this order (their initializers 0x10098e30..0x10098fd0, the
// creators' vtables 0x101e8a18..0x101e8a80 and their New 0x10099020..)
CStandardCreator< IClientWeaponBase, CClientWeaponNone > ClientWeaponNoneCreator;
CStandardCreator< IClientWeaponBase, CTronClientWeapon > ClientWeaponCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponDisc > ClientWeaponDiscCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponSequencerDisc > ClientWeaponSequencerDiscCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponClusterDisc > ClientWeaponClusterDiscCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponPRod > ClientWeaponPRodCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponSuffusion > ClientWeaponSuffusionCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponLOL > ClientWeaponLOLCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponBall > ClientWeaponBallCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponBallLauncher > ClientWeaponBallLauncherCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponDrunkenDims > ClientWeaponDrunkenDimsCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponBlaster > ClientWeaponBlasterCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponEnergyClaw > ClientWeaponEnergyClawCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponPrankster > ClientWeaponPranksterCreator;


// This table matches the enumeration from above to a
// class that will create the proper client weapon
// (also defined above).  Be VERY VERY careful about
// changeing the order of these, or the corresponding
// enumeration.    There are data files that rely
// on this ordering to create the proper weapon.
CCreator< IClientWeaponBase > *aWeaponClasses[] =
{
	&ClientWeaponNoneCreator,           // CLIENT_WEAPON_TYPE_NONE,
	&ClientWeaponCreator,               // CLIENT_WEAPON_TYPE_VECTOR,
	&ClientWeaponCreator,               // CLIENT_WEAPON_TYPE_PROJECTILE,
	&ClientWeaponDiscCreator,           // CLIENT_WEAPON_TYPE_DISC,
	&ClientWeaponSequencerDiscCreator,  // CLIENT_WEAPON_TYPE_SEQUENCERDISC,
	&ClientWeaponClusterDiscCreator,    // CLIENT_WEAPON_TYPE_CLUSTERDISC,
	&ClientWeaponPRodCreator,           // CLIENT_WEAPON_TYPE_PROD,
	&ClientWeaponSuffusionCreator,      // CLIENT_WEAPON_TYPE_SUFFUSION,
	&ClientWeaponLOLCreator,            // CLIENT_WEAPON_TYPE_LOL,
	&ClientWeaponBallCreator,           // CLIENT_WEAPON_TYPE_BALL,
	&ClientWeaponBallLauncherCreator,   // CLIENT_WEAPON_TYPE_BALLLAUNCHER,
	&ClientWeaponDrunkenDimsCreator,    // CLIENT_WEAPON_TYPE_DRUNKENDIMS,
	&ClientWeaponBlasterCreator,        // CLIENT_WEAPON_TYPE_BLASTER,
	&ClientWeaponEnergyClawCreator,     // CLIENT_WEAPON_TYPE_ENERGYCLAW,
	&ClientWeaponPranksterCreator,      // CLIENT_WEAPON_TYPE_PRANKSTER,
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronClientWeaponAllocator::New
//
//	PURPOSE:	Allocate the specified CClientWepaon.
//
// ----------------------------------------------------------------------- //

IClientWeaponBase *CTronClientWeaponAllocator::New( int nClientWeaponType ) const
{
	// check the specified client weapon type (do NOT include type NONE)
	ASSERT( ( CLIENT_WEAPON_TYPE_NONE <= nClientWeaponType ) &&
	        ( NUM_CLIENT_WEAPON_TYPES > nClientWeaponType ) );

	// RETAIL: 0x10098fe0: an invalid type, and the vector and projectile
	// types (which Tron's weapons don't use), get no weapon (NOLF2 returns 0)
	if ( !( ( CLIENT_WEAPON_TYPE_NONE <= nClientWeaponType ) &&
	        ( NUM_CLIENT_WEAPON_TYPES > nClientWeaponType ) ) ||
		 ( CLIENT_WEAPON_TYPE_VECTOR == nClientWeaponType ) ||
		 ( CLIENT_WEAPON_TYPE_PROJECTILE == nClientWeaponType ) )
	{
		return aWeaponClasses[ CLIENT_WEAPON_TYPE_NONE ]->New();
	}

	// return a new instance of the requested type of client weapon 
	return aWeaponClasses[ nClientWeaponType ]->New();
}
