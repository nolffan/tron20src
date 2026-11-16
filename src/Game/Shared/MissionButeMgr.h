// ----------------------------------------------------------------------- //
//
// MODULE  : MissionButeMgr.h
//
// PURPOSE : MissionButeMgr definition - Controls attributes of all Missions
//
// CREATED : 07/26/99
//
// ----------------------------------------------------------------------- //

#ifndef __MISSION_BUTE_MGR_H__
#define __MISSION_BUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "TemplateList.h"

#define MISSION_DEFAULT_FILE		"Attributes\\Missions.txt"
#define MISSION_COOP_FILE			"Attributes\\CoopMissions.txt"
#define MISSION_DM_FILE 			"DMMissions.txt"
//#define MISSION_TDM_FILE 			"TDMMissions.txt"
#define MISSION_DD_FILE 			"DDMissions.txt"

#define MMGR_INVALID_ID			255	// Invalid id
#define MISSION_DEFAULT_ID		0	// First Level in Mission attribute file

#define MMGR_MAX_FILE_PATH			64
#define MMGR_MAX_LEVEL_FILE_PATH	64
#define MMGR_MAX_REWARD_NAME		64
#define MMGR_MAX_MISSION_LEVELS		10
#define MMGR_MAX_MISSION_REWARDS	20
#define MMGR_MAX_WEAPONS			10
#define MMGR_MAX_GADGETS			10
#define MMGR_MAX_AMMO				10
#define MMGR_MAX_MODS				10

#define MMGR_MISSION_TAG				"Mission"
#define MMGR_GENERAL_TAG				"General"


// RETAIL: Tron's lists of named weapons, subroutines and procedurals.  The
// record layouts come from the constructors at CShell 0x1013d9d0, 0x1013d9e0,
// 0x1013d9f0 and the list builders at 0x1013d3e0, 0x1013d630, 0x1013d840.
// The names of the structs and their members are ours.
#define MMGR_MAX_ITEM_NAME			64
#define MMGR_MAX_MISSION_WEAPONS	16
#define MMGR_MAX_SUBROUTINES		18
#define MMGR_MAX_PROCEDURALS		3

// 0x4c bytes.
struct MISSIONWEAPON
{
	MISSIONWEAPON();

	char	szName[MMGR_MAX_ITEM_NAME];
	int		nId;			// +0x40
	int		nVersion;		// +0x44 "alpha" 0, "beta" 1, "gold" 2
	int		nCondition;		// +0x48 "normal" 0, "infected"/"corrupted" 1
};

// 0x48 bytes.
struct MISSIONSUBROUTINE
{
	MISSIONSUBROUTINE();

	char	szName[MMGR_MAX_ITEM_NAME];
	int		nVersion;		// +0x40
	int		nCondition;		// +0x44
};

// 0x40 bytes.
struct MISSIONPROCEDURAL
{
	MISSIONPROCEDURAL();

	char	szName[MMGR_MAX_ITEM_NAME];
};

// RETAIL: 0x100c bytes (LEVEL::LEVEL, CShell 0x1013d920).
struct LEVEL
{
	LEVEL();

	// Name for the loading screen.
	int		nNameId;
	int		nBriefingId;
	int		nHelpId;

	char	szLevel[MMGR_MAX_LEVEL_FILE_PATH];		// +0xc

	int					nNumDefaultWeapons;			// +0x4c
	MISSIONWEAPON		aDefaultWeapons[MMGR_MAX_MISSION_WEAPONS];

	int					nNumIdealWeapons;			// +0x510
	MISSIONWEAPON		aIdealWeapons[MMGR_MAX_MISSION_WEAPONS];

	int					nNumIdealSubroutines;		// +0x9d4
	MISSIONSUBROUTINE	aIdealSubroutines[MMGR_MAX_SUBROUTINES];

	int					nNumIdealProcedurals;		// +0xee8
	MISSIONPROCEDURAL	aIdealProcedurals[MMGR_MAX_PROCEDURALS];

	int		nNumDefaultAmmo;						// +0xfac
	int		aDefaultAmmo[MMGR_MAX_AMMO];

	int		nNumDefaultMods;						// +0xfd8
	int		aDefaultMods[MMGR_MAX_MODS];

	int		nBuildNotes;							// +0x1004 "BuildNotes<n>"
	int		nIdealBuildPoints;						// +0x1008
};

struct REWARD
{
	REWARD() { szName[0] = '\0'; nVal = 0; nDescriptionId = 0; }

	char	szName[MMGR_MAX_REWARD_NAME];
	uint32	nVal;
	uint32  nDescriptionId;
};


// RETAIL: 0xb674 bytes (MISSION::MISSION, CShell 0x1013c850; the size
// from debug_new_fn at 0x1013dab0).  Tron has no DescId, NameStr or Photo.
struct MISSION
{
	MISSION();
    virtual LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
    virtual LTBOOL   Save(CButeMgr & buteMgr, char* aTagName);

	int		nId;
	int		nNameId;

	char	szLayout[MMGR_MAX_FILE_PATH];			// +0xc
	char	szBriefLayout[MMGR_MAX_FILE_PATH];		// +0x4c

	int					nNumDefaultWeapons;			// +0x8c
	MISSIONWEAPON		aDefaultWeapons[MMGR_MAX_MISSION_WEAPONS];

	int					nNumIdealWeapons;			// +0x550
	MISSIONWEAPON		aIdealWeapons[MMGR_MAX_MISSION_WEAPONS];

	int					nNumIdealSubroutines;		// +0xa14
	MISSIONSUBROUTINE	aIdealSubroutines[MMGR_MAX_SUBROUTINES];

	int					nNumIdealProcedurals;		// +0xf28
	MISSIONPROCEDURAL	aIdealProcedurals[MMGR_MAX_PROCEDURALS];

	int		nSelectedWeapon;						// +0xfec

	int		nNumDefaultAmmo;						// +0xff0
	int		aDefaultAmmo[MMGR_MAX_AMMO];

	int		nNumDefaultMods;						// +0x101c
	int		aDefaultMods[MMGR_MAX_MODS];

	int		nNumLevels;								// +0x1048
	LEVEL	aLevels[MMGR_MAX_MISSION_LEVELS];		// +0x104c

	int		nNumRewards;							// +0xb0c4
	REWARD	aRewards[MMGR_MAX_MISSION_REWARDS];		// +0xb0c8

	int		nIdealBuildPoints;						// +0xb668
	bool	bResetPlayer;							// +0xb66c
	int		nNameSeed;								// +0xb670

protected :
	int		BuildAmmoList(char *pszStr, int* pArray, int nArrayLen);
	int		BuildModsList(char *pszStr, int* pArray, int nArrayLen);
	int		BuildWeaponsList(char *pszStr, MISSIONWEAPON* pArray, int nArrayLen);
	int		BuildSubroutinesList(char *pszStr, MISSIONSUBROUTINE* pArray, int nArrayLen);
	int		BuildProceduralsList(char *pszStr, MISSIONPROCEDURAL* pArray, int nArrayLen);
};



class CMissionButeMgr : public CGameButeMgr
{
	public :

		CMissionButeMgr();
		virtual ~CMissionButeMgr();

        virtual LTBOOL	Init(const char* szAttributeFile=MISSION_DEFAULT_FILE) = 0;
		virtual void	Term();

        void            Reload() { Term(); Init(); }

		int				GetNumMissions() const { return m_MissionList.size(); }
		MISSION*		GetMission(int nMissionId);
		LEVEL*			GetLevel( int nMissionId, int nLevelId );

        bool			IsMissionLevel( char const* pWorldFile, int & nMissionId, int & nLevel);


	protected :

		virtual	MISSION*	CreateMission( ) { return debug_new(MISSION); }
		virtual void		DestroyMission( MISSION* pMission ) { debug_delete( pMission ); }

		typedef std::vector< MISSION* > MissionList;
		MissionList		m_MissionList;

		char			m_aTagName[30];
};

extern CMissionButeMgr *g_pMissionButeMgr;

#endif // __MISSION_BUTE_MGR_H__