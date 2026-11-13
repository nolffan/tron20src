// ----------------------------------------------------------------------- //
//
// MODULE  : SkillsButeMgr.h
//
// PURPOSE : Definition of bute mgr to manage skill related data
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SKILLS_BUTE_MGR_
#define _SKILLS_BUTE_MGR_

#include "GameButeMgr.h"
#include "ltbasetypes.h"

#define SMGR_DEFAULT_FILE		"Attributes\\Skills.txt"

enum eSkill
{
	SKL_STEALTH,
	SKL_STAMINA,
	SKL_AIM,
	SKL_CARRY,
	SKL_ARMOR,
	SKL_WEAPON,
	SKL_GADGET,
	SKL_ENERGY,	// RETAIL: Tron's (retail Object.lto's skill name table, 0x10302a94)
	SKL_SEARCH,
	kNumSkills
};

enum eSkillLevel
{
	SKL_NOVICE,
	SKL_AMATEUR,
	SKL_SKILLED,
	SKL_EXPERT,
	SKL_MASTER,
	kNumSkillLevels
};

#ifdef _CLIENTBUILD
const char* GetSkillName(eSkill skl);
const char* GetSkillDescription(eSkill skl);
const char* GetSkillLevelName(eSkillLevel lvl);
int			GetSkillNameId(eSkill skl);
int			GetSkillDescriptionId(eSkill skl);
#endif

#define SMGR_MAX_NUM_RANKS 10
struct RANK
{
	int		nNameId;
	uint16	nMinScore;
};


const uint8 kMaxModifiers = 5;

// RETAIL: Tron's modifier tables are sized by their own modifier counts
// (12, 20, 12, 8, 8, 8, 8, 4 and 4 bytes a level from +0x124c: the
// constructors 0x10168ee0 (client) and 0x10185f10 (server)), as build 51's
// structs were, each constructor a chained assignment (the stores run from
// the last modifier down). NOLF2 gave them all kMaxModifiers floats in a
// common base. The modifier names are NOLF2's enums; Tron adds Energy's
// one and keeps one for Search (retail CSkillsButeMgr::Init 0x10169110).
#define SKILL_MODIFIERS_1(cls, a) \
	cls() { fModifier[a] = 1.0f; }
#define SKILL_MODIFIERS_2(cls, a, b) \
	cls() { fModifier[a] = fModifier[b] = 1.0f; }
#define SKILL_MODIFIERS_3(cls, a, b, c) \
	cls() { fModifier[a] = fModifier[b] = fModifier[c] = 1.0f; }
#define SKILL_MODIFIERS_5(cls, a, b, c, d, e) \
	cls() { fModifier[a] = fModifier[b] = fModifier[c] = fModifier[d] = fModifier[e] = 1.0f; }

class StealthModifiers
{
public:
	enum eMods
	{
		eRadius,		//	Sound radii for footsteps, weapon reload, etc.
		eHideTime,		//	How quickly player disappears into shadows
		eEvasion,		//	How effectively player ditches pursuers
		kNumModifiers
	};
	SKILL_MODIFIERS_3(StealthModifiers, eRadius, eHideTime, eEvasion)
	float	fModifier[kNumModifiers];
};

class StaminaModifiers
{
public:
	enum eMods
	{
		eMaxHealth,			//	The size of player health meter
		eMoveDamage,		//	How much player movement is affected by damage
		eResistance,		//	Resistance to certain status effects (i.e. poison, burn damage, bleeding, etc.)
		eHealthPickup,		//	Multiplier for how many health points the player gets from a helth pickup
		eDamage,			//	Multiplier for how much damage the player takes
		kNumModifiers
	};
	SKILL_MODIFIERS_5(StaminaModifiers, eMaxHealth, eMoveDamage, eResistance, eHealthPickup, eDamage)
	float	fModifier[kNumModifiers];
};

class AimModifiers
{
public:
	enum eMods
	{
		eAccuracy,		//	The amount of weapon perturb
		eZoomSway,		//	How much player's view swims when zoomed in
		eCorrection,	//	How quickly player's perturb recovers after moving.
		kNumModifiers
	};
	SKILL_MODIFIERS_3(AimModifiers, eAccuracy, eZoomSway, eCorrection)
	float	fModifier[kNumModifiers];
};

class CarryModifiers
{
public:
	enum eMods
	{
		eMaxAmmo,				//	How much ammunition the player can carry
		eCarryBodyMovement,		//	RETAIL: read as "HeavyWeaponMovement" (build 51's name)
		kNumModifiers
	};
	SKILL_MODIFIERS_2(CarryModifiers, eMaxAmmo, eCarryBodyMovement)
	float	fModifier[kNumModifiers];
};

class ArmorModifiers
{
public:
	enum eMods
	{
		eMaxArmor,		//	The size of the player armor meter
		eArmorPickup,	//	Multiplier for how many armor points the player gets from an armor pickup
		kNumModifiers
	};
	SKILL_MODIFIERS_2(ArmorModifiers, eMaxArmor, eArmorPickup)
	float	fModifier[kNumModifiers];
};

class WeaponModifiers
{
public:
	enum eMods
	{
		eDamage,		//	The amount of damage caused by a weapon
		eReload,		//	Weapon reload and select times
		kNumModifiers
	};
	SKILL_MODIFIERS_2(WeaponModifiers, eDamage, eReload)
	float	fModifier[kNumModifiers];
};

class GadgetModifiers
{
public:
	enum eMods
	{
		eEffect,		//	Effectiveness of the gadget
		eSelect,		//	Gadget select times
		kNumModifiers
	};
	SKILL_MODIFIERS_2(GadgetModifiers, eEffect, eSelect)
	float	fModifier[kNumModifiers];
};

// RETAIL: Tron's (build 51's EnergyModifiers: fMaxEnergy); Init reads none
class EnergyModifiers
{
public:
	enum eMods
	{
		eMaxEnergy,		//	Maximum amount of energy
		kNumModifiers
	};
	SKILL_MODIFIERS_1(EnergyModifiers, eMaxEnergy)
	float	fModifier[kNumModifiers];
};

class SearchModifiers
{
public:
	enum eMods
	{
		eSearchSpeed,	//	How quickly player searches things
		kNumModifiers	//	RETAIL: Tron has no FindJunk
	};
	SKILL_MODIFIERS_1(SearchModifiers, eSearchSpeed)
	float	fModifier[kNumModifiers];
};

class CSkillsButeMgr : public CGameButeMgr
{
public :

	CSkillsButeMgr();
	~CSkillsButeMgr();

    LTBOOL      Init(const char* szAttributeFile=SMGR_DEFAULT_FILE);
	void		Term();

    void        Reload() { Term(); Init(); }

	const RANK*	GetRank(uint32 nScore);

	// RETAIL: Tron reads no intel bonus (Init 0x10169110 has no "Bonus"
	// tag, the manager no member for it); NOLF2's CSkills still asks
	uint32		GetIntelSkillBonus() {return 0; }

	uint32		GetCostToUpgrade(eSkill skl, eSkillLevel tgtLevel) {return m_nCosts[skl][tgtLevel];}

	bool		IsAvailable(eSkill skl) {return (m_nCosts[skl][SKL_AMATEUR] > 0);}

	const float*			GetModifiers(eSkill skl, eSkillLevel lvl);
	float					GetModifier(eSkill skl, eSkillLevel lvl, uint8 nMod);
	uint8					GetNumModifiers(eSkill skl);

#ifdef _CLIENTBUILD
	uint16					GetModifierNameId(eSkill skl, uint8 nMod);
	uint16					GetModifierDescriptionId(eSkill skl, uint8 nMod);
#endif

	// RETAIL: build 51's per-skill accessors, which Tron's CTronPlayerStats
	// inlines (retail 0x100a9410..0x100a94f0; Tron's modifier tables are
	// 12, 20, 12, 8, 8, 8, 8 and 4 bytes a level, from +0x124c)
	const StealthModifiers*	GetStealth(eSkillLevel lvl)	{ return &m_Stealth[lvl]; }
	const StaminaModifiers*	GetStamina(eSkillLevel lvl)	{ return &m_Stamina[lvl]; }
	const AimModifiers*		GetAim(eSkillLevel lvl)		{ return &m_Aim[lvl]; }
	const CarryModifiers*	GetCarry(eSkillLevel lvl)	{ return &m_Carry[lvl]; }
	const ArmorModifiers*	GetArmor(eSkillLevel lvl)	{ return &m_Armor[lvl]; }
	const WeaponModifiers*	GetWeapon(eSkillLevel lvl)	{ return &m_Weapon[lvl]; }
	const GadgetModifiers*	GetGadget(eSkillLevel lvl)	{ return &m_Gadget[lvl]; }
	const EnergyModifiers*	GetEnergy(eSkillLevel lvl)	{ return &m_Energy[lvl]; }
	const SearchModifiers*	GetSearch(eSkillLevel lvl)	{ return &m_Search[lvl]; }

	uint32		GetMultiplayerPool() {return m_nMPPool;}
	eSkillLevel GetDefaultMultiplayerLevel(eSkill skl) {return m_MPSkillLevel[skl];}


private:

	int			m_nNumRanks;
	RANK		m_Ranks[SMGR_MAX_NUM_RANKS];

	uint32		m_nCosts[kNumSkills][kNumSkillLevels];

	uint32		m_nMPPool;
	eSkillLevel m_MPSkillLevel[kNumSkills];

	StealthModifiers	m_Stealth[kNumSkillLevels];
	StaminaModifiers	m_Stamina[kNumSkillLevels];
	AimModifiers		m_Aim[kNumSkillLevels];
	CarryModifiers		m_Carry[kNumSkillLevels];
	ArmorModifiers		m_Armor[kNumSkillLevels];
	WeaponModifiers		m_Weapon[kNumSkillLevels];
	GadgetModifiers		m_Gadget[kNumSkillLevels];
	EnergyModifiers		m_Energy[kNumSkillLevels];
	SearchModifiers		m_Search[kNumSkillLevels];
};

extern CSkillsButeMgr* g_pSkillsButeMgr;	



#endif