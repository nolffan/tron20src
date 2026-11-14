//-------------------------------------------------------------------------
//
// MODULE  : Cheatdefs.h
//
// PURPOSE : Cheat code defines
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __CHEATDEFS_H
#define __CHEATDEFS_H

// RETAIL: Tron's cheats, in the order of CCheatMgr's table (CShell.dll
// 0x1022b870: 57 entries, all the "mp" names; the plain names are in the
// comments). The CHEAT_TRON_* names are ours; the others keep NOLF2's
// names for the same cheat.
enum CheatCode
{
	CHEAT_NONE = -1,

	CHEAT_GOD,					// mpgod (0x00)
	CHEAT_AMMO,					// mpammo (0x01)
	CHEAT_ARMOR,				// mparmor (0x02)
	CHEAT_HEALTH,				// mphealth (0x03)
	CHEAT_CLIP,					// mpclip (0x04)
	CHEAT_INVISIBLE,			// mppoltergeist (0x05)
	CHEAT_TELEPORT,				// mpteleport (0x06)
	CHEAT_POS,					// mppos (0x07)
	CHEAT_KFA,					// mpkfa (0x08)
	CHEAT_FULL_WEAPONS,			// mpguns (0x09)
	CHEAT_POSWEAPON,			// mpwpos (0x0a)
	CHEAT_POSWEAPON_MUZZLE,		// mpwmpos (0x0b)
	CHEAT_POS1STCAM,			// mpcamera (0x0c)
	CHEAT_LIGHTSCALE,			// mplightscale (0x0d)
	CHEAT_LIGHTADD,				// mplightadd (0x0e)
	CHEAT_FOV,					// mpfov (0x0f)
	CHEAT_TRON_POLY,			// mppoly (0x10)
	CHEAT_TEARS,				// mptears (0x11)
	CHEAT_REMOVEAI,				// mpboyisuck (0x12)
	CHEAT_TRON_CAM,				// mpcam (0x13)
	CHEAT_SNOWMOBILE,			// mprosebud (0x14)
	CHEAT_MODSQUAD,				// mpmods (0x15)
	CHEAT_FULL_GEAR,			// mpgear (0x16)
	CHEAT_CHASETOGGLE,			// mpasscam (0x17)
	CHEAT_CAM_POSROT,			// mpcampos (0x18)
	CHEAT_EXITLEVEL,			// mpmaphole (0x19)
	CHEAT_NEXTMISSION,			// mpnextmission (0x1a)
	CHEAT_BOOT,					// mpboot (0x1b)
	CHEAT_VERSION,				// mpbuild (0x1c)
	CHEAT_TRON_EXORBITANTAMOUNTS,	// mpexorbitantamounts (0x1d)
	CHEAT_GIMMEGUN,				// mpgimmegun (0x1e)
	CHEAT_GIMMEMOD,				// mpgimmemod (0x1f)
	CHEAT_GIMMEGEAR,			// mpgimmegear (0x20)
	CHEAT_GIMMEAMMO,			// mpgimmeammo (0x21)
	CHEAT_TRON_GIMMEPRIM,		// mpgimmeprim (0x22)
	CHEAT_CONSOLE,				// mpobtuse (0x23)
	CHEAT_SKILLZ,				// mpskillz (0x24)
	CHEAT_WEAPON_BREACHOFFSET,	// mpbreachoffset (0x25)
	CHEAT_TRON_ENERGY,			// mpenergy (0x26)
	CHEAT_TRON_PSET,			// mppset (0x27)
	CHEAT_TRON_CLEARPSET,		// mpclearpset (0x28)
	CHEAT_TRON_PROCEDURALS,		// mpprocedurals (0x29)
	CHEAT_TRON_SUBALPHA,		// mpsubalpha (0x2a)
	CHEAT_TRON_SUBBETA,			// mpsubbeta (0x2b)
	CHEAT_TRON_SUBGOLD,			// mpsubgold (0x2c)
	CHEAT_TRON_CLEARMEMORY,		// mpclearmemory (0x2d)
	CHEAT_TRON_CORRUPTION,		// mpcorruption (0x2e)
	CHEAT_TRON_MAXPERFORMANCE,	// mpmaxperformance (0x2f)
	CHEAT_TRON_CLEARPERFORMANCE,	// mpclearperformance (0x30)
	CHEAT_TRON_IDEALWEAPONS,	// mpidealweapons (0x31)
	CHEAT_TRON_IDEALSUBROUTINES,	// mpidealsubroutines (0x32)
	CHEAT_TRON_IDEALPROCEDURALS,	// mpidealprocedurals (0x33)
	CHEAT_TRON_IDEALPOINTS,		// mpidealpoints (0x34)
	CHEAT_TRON_IDEAL,			// mpideal (0x35)
	CHEAT_TRON_UNLOCKGRIDS,		// mpunlockgrids (0x36)
	CHEAT_TRON_UNLOCKCYCLES,	// mpunlockcycles (0x37)
	CHEAT_TRON_UNLOCKPOWERUPS,	// mpunlockpowerups (0x38)

	CHEAT_MAX,

	// PORT: NOLF2's, which Tron has not: kept out of range for the NOLF2
	// code still standing in for Tron's
	CHEAT_BUILDGUID = CHEAT_MAX + 1,
	CHEAT_ENDGAME,
	CHEAT_BODYGOLFING,
	CHEAT_TRIGGERBOX,
};

#endif
