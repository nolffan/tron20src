// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypesEnum.h
//
// PURPOSE : Enums and string constants for damage types.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, the body of a const char* string list, or as a constructor
// for a DTINFO struct.
//

#undef ADD_DT

#if defined(INCLUDE_AS_ENUM)
	#define ADD_DT(id,tag,altname) DT_##id,
#elif defined(INCLUDE_AS_STRING)
	#define ADD_DT(id,bute,altname) #bute,
#elif defined(INCLUDE_AS_STRUCT)
	#define ADD_DT(id,bute,altname) DTINFO(DT_##id,#altname),	// RETAIL: Tron keeps the alternate name only
#else
	#error	To use this include file, first define INCLUDE_AS_ENUM, INCLUDE_AS_STRING or INCLUDE_AS_STRUCT, to include the goals as enums, bute string constants, or struct constructors.
#endif

// NOTE: The order of these are IMPORTANT!!
//		 The numbers to the right of the damage type should match up identiclly to the 
//		 numbers representing the damage types found in the Ammo section of weapons.txt.
//		 Additionally, there is a habbit of sending any of the first 8 damage types' flags
//		 as a byte, so if they are moved outside that range there will be instances in
//		 the code that will send blanks rather than relevant flags. 


// RETAIL: Tron 2.0's damage types, in its order (Object.lto: the bute names
// pDTNames, e.g. .data 0x102e8e7c; the alternate names DTInfoArray's
// initializer pushes, 0x10125050). Tron gives every type an alternate name
// (NOLF2's "NULL" ones are the upper case name) and has neither NOLF2's
// tracker gadget nor its DoomsDay burn. The ids of Tron's own types are ours.

ADD_DT(UNSPECIFIED,Unspecified,UNSPECIFIED)				// 0
ADD_DT(BLEEDING,Bleeding,BLEEDING)						// 1
ADD_DT(BULLET,Bullet,BULLET)								// 2
ADD_DT(BURN,Burn,BURN)									// 3
ADD_DT(CHOKE,Choke,CHOKE)								// 4
ADD_DT(CRUSH,Crush,CRUSH)								// 5
ADD_DT(ELECTROCUTE,Electrocute,ELECTROCUTE)				// 6
ADD_DT(EXPLODE,Explode,EXPLODE)							// 7
ADD_DT(FREEZE,Freeze,FREEZE)								// 8
ADD_DT(POISON,Poison,POISON)								// 9
ADD_DT(ENDLESS_FALL,EndlessFall,ENDLESS FALL)			// 10
ADD_DT(SLEEPING,Sleeping,SLEEPING)						// 11
ADD_DT(STUN,Stun,STUN)									// 12
ADD_DT(MELEE,Melee,MELEE)								// 13
ADD_DT(CAMERA_DISABLER,CameraDisabler,CAMERA DISABLER)	// 14
ADD_DT(GLUE,Glue,GLUE)									// 15
ADD_DT(BEAR_TRAP,BearTrap,BEAR TRAP)						// 16
ADD_DT(LAUGHING,Laughing,LAUGHING)						// 17
ADD_DT(ASSS,AntiSuperSoldierSerum,ANTI-SUPER SOLDIER SERUM)	// 18
ADD_DT(GADGET_CODE_DECIPHERER,CodeDecipherer,GADGET CODE DECIPHERER)	// 19
ADD_DT(GADGET_POODLE,Poodle,GADGET POODLE)				// 20
ADD_DT(GADGET_LOCK_PICK,LockPick,GADGET LOCK PICK)		// 21
ADD_DT(GADGET_WELDER,Welder,GADGET WELDER)				// 22
ADD_DT(GADGET_LIGHTER,Lighter,GADGET LIGHTER)			// 23
ADD_DT(GADGET_CAMERA,Camera,GADGET CAMERA)				// 24
ADD_DT(WORLDONLY,WorldOnly,WORLD ONLY)					// 25
ADD_DT(GADGET_INFRA_RED,InfraRed,GADGET INFRA RED)		// 26
ADD_DT(GADGET_DECAYPOWDER,DecayPowder,GADGET DECAY POWDER)	// 27
ADD_DT(GADGET_TIME_BOMB,TimeBomb,GADGET TIME BOMB)		// 28
ADD_DT(GADGET_INK_REAGENT,InkReagent,GADGET INK REAGENT)	// 29
ADD_DT(GADGET_EAVESDROPBUG,EavesDropBug,GADGET EAVESDROP BUG)	// 30
ADD_DT(SLIPPERY,Slippery,SLIPPERY)						// 31
ADD_DT(SWORD,Sword,SWORD)								// 32
ADD_DT(PROD,PRod,PROD)									// 33
ADD_DT(CORRODE,Corrode,CORRODE)							// 34
ADD_DT(ENERGYCLAW,EnergyClaw,ENERGY CLAW)				// 35
ADD_DT(KNUCKLESANDWICH,KnuckleSandwich,KNUCKLESANDWICH)	// 36
ADD_DT(DISC,Disc,DISC)									// 37
ADD_DT(SEQUENCER,Sequencer,SEQUENCER)					// 38
ADD_DT(CLUSTER,Cluster,CLUSTER)							// 39
ADD_DT(SUFFUSION,Suffusion,SUFFUSION)					// 40
ADD_DT(LOL,LOL,LOL)										// 41
ADD_DT(BALL,Ball,BALL)									// 42
ADD_DT(BALLLAUNCHER,BallLauncher,BALLLAUNCHER)			// 43
ADD_DT(DRUNKENDIMS,DrunkenDims,DRUNKENDIMS)				// 44
ADD_DT(BLASTER,Blaster,BLASTER)							// 45
ADD_DT(PRANKSTERBIT,PranksterBit,PRANKSTERBIT)			// 46
ADD_DT(CLUSTERSHARDAMMO,ClusterShardAmmo,CLUSTERSHARDAMMO)	// 47
ADD_DT(ENEMYDISC,EnemyDisc,ENEMYDISC)					// 48
ADD_DT(CORRUPTICDIMS,CorrupticDims,CORRUPTICDIMS)		// 49
ADD_DT(CLUSTERAUTOBURST,ClusterAutoBurst,CLUSTERAUTOBURST)	// 50
ADD_DT(RIOTSHIELDAMMO,RiotShieldAmmo,RIOTSHIELDAMMO)		// 51
ADD_DT(OPTIMIZEREXPLOSION,OptimizerExplosion,OPTIMIZEREXPLOSION)	// 52
ADD_DT(SWATBLOCKMELEEATTACK,SwatBlockMeleeAttack,SWATBLOCKMELEEATTACK)	// 53
ADD_DT(FINDERWEAPON,FinderWeapon,FINDERWEAPON)			// 54
ADD_DT(PRANKSTERBITIMPLODE,PranksterBitImplode,PRANKSTERBITIMPLODE)	// 55
ADD_DT(POWERBLOCK,PowerBlock,POWERBLOCK)					// 56
ADD_DT(BALLRADIUS,BallRadius,BALLRADIUS)					// 57
ADD_DT(BALLLAUNCHERRADIUS,BallLauncherRadius,BALLLAUNCHERRADIUS)	// 58
ADD_DT(DRUNKENDIMSRADIUS,DrunkenDimsRadius,DRUNKENDIMSRADIUS)	// 59
ADD_DT(PLAYERONLY,PlayerOnly,PLAYERONLY)					// 60
ADD_DT(SUFFUSIONGIB,SuffusionGib,SUFFUSIONGIB)			// 61
ADD_DT(TANK,Tank,TANK)									// 62
