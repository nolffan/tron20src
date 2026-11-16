// ----------------------------------------------------------------------- //
//
// MODULE  : MissionButeMgr.cpp
//
// PURPOSE : MissionButeMgr implementation - Controls attributes of all
//			 missions
//
// CREATED : 07/26/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MissionButeMgr.h"
#include "CommonUtilities.h"
#include "WeaponMgr.h"


#define MMGR_MISSION_NAMEID				"NameId"
#define MMGR_MISSION_BRIEFINGID			"BriefingId"
#define MMGR_MISSION_HELPID				"HelpId"
#define MMGR_MISSION_RESETPLAYER		"ResetPlayer"
#define MMGR_MISSION_OBJECTIVEIDS		"ObjectiveIds"
#define MMGR_MISSION_LAYOUT				"LayoutInfo"
#define MMGR_MISSION_BRIEFING_LAYOUT	"BriefingLayout"
#define MMGR_MISSION_WEAPONS_DEFAULT	"DefaultWeapons"
#define MMGR_MISSION_AMMO_DEFAULT		"DefaultAmmo"
#define MMGR_MISSION_MODS_DEFAULT		"DefaultMods"
#define MMGR_MISSION_SELECTED_WEAPON	"SelectedWeapon"
#define MMGR_MISSION_WEAPONS_IDEAL		"IdealWeapons"
#define MMGR_MISSION_SUBROUTINES_IDEAL	"IdealSubroutines"
#define MMGR_MISSION_PROCEDURALS_IDEAL	"IdealProcedurals"
#define MMGR_MISSION_IDEAL_BUILDPOINTS	"IdealBuildPoints"
#define MMGR_MISSION_NAMESEED			"NameSeed"
#define MMGR_MISSION_BUILDNOTES			"BuildNotes"

#define MMGR_MISSION_LEVEL				"Level"
#define MMGR_MISSION_REWARD				"Reward"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::CMissionButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMissionButeMgr::CMissionButeMgr()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::~CMissionButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CMissionButeMgr::~CMissionButeMgr()
{
	g_pMissionButeMgr = NULL;

	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CMissionButeMgr::Term()
{
	MissionList::iterator it = m_MissionList.begin( );
	while( it != m_MissionList.end( ))
	{
		DestroyMission( *it );
		it++;
	}
	m_MissionList.clear();

	CGameButeMgr::Term( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::GetMission
//
//	PURPOSE:	Get the specified mission
//
// ----------------------------------------------------------------------- //

MISSION* CMissionButeMgr::GetMission(int nMissionId)
{
    if (nMissionId < 0 || nMissionId > ( int )m_MissionList.size( )) return LTNULL;

	MissionList::iterator it = m_MissionList.begin( );
	for( ; it != m_MissionList.end( ); it++ )
	{
		MISSION* pMission = *it;
		if( !pMission )
			continue;

		if( pMission->nId == nMissionId )
			return pMission;
	}

    return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::GetLevel
//
//	PURPOSE:	Get the specified level
//
// ----------------------------------------------------------------------- //

LEVEL* CMissionButeMgr::GetLevel( int nMissionId, int nLevelId )
{
	MISSION *pMission = GetMission( nMissionId );
	if( !pMission )
		return LTNULL;

	if( nLevelId < 0 || nLevelId > pMission->nNumLevels )
		return LTNULL;

	return &pMission->aLevels[nLevelId];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::IsMissionLevel()
//
//	PURPOSE:	Determine if the passed in string is a level of one of the
//				missions.
//
//	RETURNS:	True if a mission level, else false.  Also nMissionId and
//				nLevel are filled in with the correct values.
//
// ----------------------------------------------------------------------- //

bool CMissionButeMgr::IsMissionLevel(char const* pWorldFile, int & nMissionId, int & nLevel)
{
	if (!pWorldFile) 
		return false;


	//strip path (if there is one) from world name
	char const* pTmp = strrchr(pWorldFile,'\\');
	if (pTmp)
	{
		pTmp++;
		pWorldFile = pTmp;
	}

	MissionList::iterator it = m_MissionList.begin( );
	for( ; it != m_MissionList.end( ); it++)
	{
		MISSION* pMission = *it;
		if( !pMission )
			continue;

		for (int i=0; i < pMission->nNumLevels; i++)
		{

			//strip path (if there is one) from each levels name
			char const* pTmp = strrchr(pMission->aLevels[i].szLevel,'\\');
			if (pTmp)
			{
				pTmp++;
			}
			else
			{
				pTmp = pMission->aLevels[i].szLevel;
			}

			if (stricmp( pTmp, pWorldFile ) == 0)
			{
				nMissionId = pMission->nId;
				nLevel = i;
                return true;
			}
		}
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::MISSION
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

// RETAIL: CShell 0x1013c850.
MISSION::MISSION()
{
	nId					= -1;

	nNameId				= -1;
	bResetPlayer		= true;

	szLayout[0]			= '\0';
	szBriefLayout[0]	= '\0';

	nNumDefaultWeapons		= 0;
	nNumIdealWeapons		= 0;
	nNumIdealSubroutines	= 0;
	nNumIdealProcedurals	= 0;
	nNumDefaultAmmo			= 0;
	nNumDefaultMods			= 0;
	nIdealBuildPoints		= 0;

	nSelectedWeapon		= -1;

	aDefaultAmmo[0]		= WMGR_INVALID_ID;
	aDefaultMods[0]		= WMGR_INVALID_ID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::Init
//
//	PURPOSE:	Build the mission struct
//
// ----------------------------------------------------------------------- //

// RETAIL: CShell 0x1013c950.
LTBOOL MISSION::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	char aAttName[100];

	nNameId				= buteMgr.GetInt(aTagName, MMGR_MISSION_NAMEID);
	nIdealBuildPoints	= buteMgr.GetInt(aTagName, MMGR_MISSION_IDEAL_BUILDPOINTS);
	bResetPlayer		= !!buteMgr.GetInt(aTagName, MMGR_MISSION_RESETPLAYER, 1 );
	nNameSeed			= buteMgr.GetInt(aTagName, MMGR_MISSION_NAMESEED, 0 );

	char szStr[512] ="";
	buteMgr.GetString(aTagName, MMGR_MISSION_OBJECTIVEIDS,szStr,sizeof(szStr));

	buteMgr.GetString(aTagName, MMGR_MISSION_LAYOUT, szLayout, ARRAY_LEN(szLayout));
	buteMgr.GetString(aTagName, MMGR_MISSION_BRIEFING_LAYOUT, szBriefLayout, ARRAY_LEN(szBriefLayout));

	buteMgr.GetString(aTagName, MMGR_MISSION_WEAPONS_DEFAULT,szStr,sizeof(szStr));
	nNumDefaultWeapons = BuildWeaponsList(szStr, aDefaultWeapons, ARRAY_LEN(aDefaultWeapons));

	buteMgr.GetString(aTagName, MMGR_MISSION_WEAPONS_IDEAL,szStr,sizeof(szStr));
	nNumIdealWeapons = BuildWeaponsList(szStr, aIdealWeapons, ARRAY_LEN(aIdealWeapons));

	buteMgr.GetString(aTagName, MMGR_MISSION_SUBROUTINES_IDEAL,szStr,sizeof(szStr));
	nNumIdealSubroutines = BuildSubroutinesList(szStr, aIdealSubroutines, ARRAY_LEN(aIdealSubroutines));

	buteMgr.GetString(aTagName, MMGR_MISSION_PROCEDURALS_IDEAL,szStr,sizeof(szStr));
	nNumIdealProcedurals = BuildProceduralsList(szStr, aIdealProcedurals, ARRAY_LEN(aIdealProcedurals));

	buteMgr.GetString(aTagName, MMGR_MISSION_AMMO_DEFAULT,szStr,sizeof(szStr));
	nNumDefaultAmmo = BuildAmmoList(szStr, aDefaultAmmo, ARRAY_LEN(aDefaultAmmo));

	buteMgr.GetString(aTagName, MMGR_MISSION_MODS_DEFAULT,szStr,sizeof(szStr));
	nNumDefaultMods = BuildModsList(szStr, aDefaultMods, ARRAY_LEN(aDefaultMods));

	szStr[0] = '\0';
	buteMgr.GetString(aTagName, MMGR_MISSION_SELECTED_WEAPON, szStr, sizeof(szStr));
	if (szStr[0])
	{
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(szStr);
		if (pWeapon)
		{
			nSelectedWeapon = pWeapon->nId;
		}
	}

	nNumLevels = 0;

	while( nNumLevels < MMGR_MAX_MISSION_LEVELS )
	{

		sprintf(aAttName, "%s%d", MMGR_MISSION_LEVEL, nNumLevels);
		buteMgr.GetString(aTagName, aAttName, "", aLevels[nNumLevels].szLevel, ARRAY_LEN(aLevels[nNumLevels].szLevel));
		if( !buteMgr.Success( ))
			break;

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_NAMEID );
		aLevels[nNumLevels].nNameId	= buteMgr.GetInt( aTagName, aAttName, -1 );

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_BRIEFINGID );
		aLevels[nNumLevels].nBriefingId	= buteMgr.GetInt( aTagName, aAttName, -1 );

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_HELPID );
		aLevels[nNumLevels].nHelpId	= buteMgr.GetInt( aTagName, aAttName, -1 );

		// Level default weapons...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_WEAPONS_DEFAULT );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumDefaultWeapons = BuildWeaponsList( szStr, aLevels[nNumLevels].aDefaultWeapons,
																   ARRAY_LEN(aLevels[nNumLevels].aDefaultWeapons) );

		// Level ideal weapons...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_WEAPONS_IDEAL );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumIdealWeapons = BuildWeaponsList( szStr, aLevels[nNumLevels].aIdealWeapons,
																 ARRAY_LEN(aLevels[nNumLevels].aIdealWeapons) );

		// Level ideal subroutines...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_SUBROUTINES_IDEAL );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumIdealSubroutines = BuildSubroutinesList( szStr, aLevels[nNumLevels].aIdealSubroutines,
																		 ARRAY_LEN(aLevels[nNumLevels].aIdealSubroutines) );

		// Level ideal procedurals...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_PROCEDURALS_IDEAL );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumIdealProcedurals = BuildProceduralsList( szStr, aLevels[nNumLevels].aIdealProcedurals,
																		 ARRAY_LEN(aLevels[nNumLevels].aIdealProcedurals) );

		// Level default ammo...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_AMMO_DEFAULT );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumDefaultAmmo = BuildAmmoList( szStr, aLevels[nNumLevels].aDefaultAmmo, 
															 ARRAY_LEN(aLevels[nNumLevels].aDefaultAmmo) );

		// Level default mods...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_MODS_DEFAULT );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumDefaultMods = BuildModsList( szStr, aLevels[nNumLevels].aDefaultMods,
															 ARRAY_LEN(aLevels[nNumLevels].aDefaultMods) );

		// RETAIL: the key is "BuildNotes<n>", not "Level<n>BuildNotes".
		sprintf( aAttName, "%s%d", MMGR_MISSION_BUILDNOTES, nNumLevels );
		aLevels[nNumLevels].nBuildNotes = buteMgr.GetInt( aTagName, aAttName, 0 );

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_IDEAL_BUILDPOINTS );
		aLevels[nNumLevels].nIdealBuildPoints = buteMgr.GetInt( aTagName, aAttName, 0 );

		nNumLevels++;
	}

	nNumRewards = 0;
	while( nNumRewards < MMGR_MAX_MISSION_REWARDS)
	{
		sprintf(aAttName, "%s%d", MMGR_MISSION_REWARD, nNumRewards);
		buteMgr.GetString(aTagName, aAttName, "", aRewards[nNumRewards].szName, ARRAY_LEN(aRewards[nNumRewards].szName));
		if( !buteMgr.Success( ))
			break;

		char *pTok = strtok(aRewards[nNumRewards].szName,",");
		pTok = strtok(LTNULL,",");
		if (pTok)
			aRewards[nNumRewards].nVal = (uint32)atol(pTok);

		pTok = strtok(LTNULL,"");
		if (pTok)
			aRewards[nNumRewards].nDescriptionId = (uint32)atol(pTok);

		nNumRewards++;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::Save
//
//	PURPOSE:	Save the mission struct
//
// ----------------------------------------------------------------------- //

LTBOOL MISSION::Save(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	char aAttName[100];


//***********************************************
//*jrg  NOTE:Only saving level info at this point
//***********************************************

	for (int n = 0; n < nNumLevels; n++)
	{
		sprintf(aAttName, "%s%d", MMGR_MISSION_LEVEL, n);
		buteMgr.SetString(aTagName, aAttName,aLevels[n].szLevel);
	}
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildAmmoList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of ammo added to list
//
// ----------------------------------------------------------------------- //

int MISSION::BuildAmmoList(char *pszStr, int* pArray, int nArrayLen)
{
	if (!pArray || !pszStr) return 0;

	int  nNumAmmo = 0;
	char buf[512] = "";

	while (*pszStr == ' ' || *pszStr == '\t')
		pszStr++;


	if (strlen(pszStr))
	{
		SAFE_STRCPY(buf, pszStr);
		char *pAmmoName = strtok(buf,",");
		while (pAmmoName && nNumAmmo < nArrayLen)
		{
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(pAmmoName);
			if (pAmmo)
			{
				pArray[nNumAmmo] = pAmmo->nId;
				nNumAmmo++;
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in %s: Unknown ammo: %s", g_pMissionButeMgr->GetAttributeFile( ),pAmmoName);
#else
                g_pLTServer->CPrint("Error in %s: Unknown ammo: %s", g_pMissionButeMgr->GetAttributeFile( ),pAmmoName);
#endif
			}

			pAmmoName = strtok(NULL,",");
		}
	}

	return nNumAmmo;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildModsList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of mods added to list
//
// ----------------------------------------------------------------------- //

int MISSION::BuildModsList(char *pszStr, int* pArray, int nArrayLen)
{
	if (!pArray || !pszStr) return 0;

	int  nNumMods = 0;
	char buf[512] = "";

	while (*pszStr == ' ' || *pszStr == '\t')
		pszStr++;

	if (strlen(pszStr))
	{
		SAFE_STRCPY(buf, pszStr);
		char *pModName = strtok(buf,",");
		while (pModName && nNumMods < nArrayLen)
		{
			MOD const *pMod = g_pWeaponMgr->GetMod(pModName);
			if (pMod)
			{
				pArray[nNumMods] = pMod->nId;
				nNumMods++;
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in %s: Unknown weapon mod: %s", g_pMissionButeMgr->GetAttributeFile( ),pModName);
#else
                g_pLTServer->CPrint("Error in %s: Unknown weapon mod: %s", g_pMissionButeMgr->GetAttributeFile( ),pModName);
#endif
			}

			pModName = strtok(NULL,",");
		}
	}

	return nNumMods;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildWeaponsList()
//
//	PURPOSE:	Builds a list of weapons from a string of the form
//				"Name alpha normal, Name beta infected, ..."
//
//	RETURNS:	Number of weapons added to list
//
// ----------------------------------------------------------------------- //

// RETAIL: CShell 0x1013d3e0.  The name of the function is ours.
int MISSION::BuildWeaponsList(char* pszStr, MISSIONWEAPON* pArray, int nArrayLen)
{
	if (!pArray || !pszStr) return 0;

	int  nNumWeapons = 0;
	char buf[512] = "";

	while (*pszStr == ' ' || *pszStr == '\t')
		pszStr++;

	if (strlen(pszStr))
	{
		SAFE_STRCPY(buf, pszStr);
		char *pTok = strtok(buf,",");
		while (pTok && nNumWeapons < nArrayLen)
		{
			int i = 0;
			char *p = pTok;
			while (isalnum(*p) || *p == '_')
			{
				pArray[nNumWeapons].szName[i] = *p++;
				i++;
			}
			pArray[nNumWeapons].szName[i] = '\0';

			// RETAIL: the lookup and the error message use the terminating
			// character of the name, not the name (docs/RETAIL_BUGS.md).
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(pArray[nNumWeapons].szName[i]);
			if (pWeapon)
			{
				pArray[nNumWeapons].nId = pWeapon->nId;
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in %s: Unknown weapon: %s", g_pMissionButeMgr->GetAttributeFile( ),pArray[nNumWeapons].szName[i]);
#else
                g_pLTServer->CPrint("Error in %s: Unknown weapon: %s", g_pMissionButeMgr->GetAttributeFile( ),pArray[nNumWeapons].szName[i]);
#endif
            }

			while (*p)
			{
				while (!isalpha(*p))
					p++;

				if (!strnicmp(p, "alpha", 5))
					pArray[nNumWeapons].nVersion = 0;
				if (!strnicmp(p, "beta", 4))
					pArray[nNumWeapons].nVersion = 1;
				if (!strnicmp(p, "gold", 4))
					pArray[nNumWeapons].nVersion = 2;
				if (!strnicmp(p, "normal", 6))
					pArray[nNumWeapons].nCondition = 0;
				if (!strnicmp(p, "infected", 8) || !strnicmp(p, "corrupted", 9))
					pArray[nNumWeapons].nCondition = 1;

				while (isalnum(*p) || *p == '_')
					p++;
			}

			nNumWeapons++;
			pTok = strtok(NULL,",");
		}
	}

	return nNumWeapons;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildSubroutinesList()
//
//	PURPOSE:	Builds a list of subroutines from a string of the form
//				"Name alpha normal, Name beta infected, ..."
//
//	RETURNS:	Number of subroutines added to list
//
// ----------------------------------------------------------------------- //

// RETAIL: CShell 0x1013d630.  The name of the function is ours.
int MISSION::BuildSubroutinesList(char* pszStr, MISSIONSUBROUTINE* pArray, int nArrayLen)
{
	if (!pArray || !pszStr) return 0;

	int  nNumSubroutines = 0;
	char buf[512] = "";

	while (*pszStr == ' ' || *pszStr == '\t')
		pszStr++;

	if (strlen(pszStr))
	{
		SAFE_STRCPY(buf, pszStr);
		char *pTok = strtok(buf,",");
		while (pTok && nNumSubroutines < nArrayLen)
		{
			int i = 0;
			char *p = pTok;
			while (isalnum(*p) || *p == '_')
			{
				pArray[nNumSubroutines].szName[i] = *p++;
				i++;
			}
			pArray[nNumSubroutines].szName[i] = '\0';

			while (*p)
			{
				while (!isalpha(*p))
					p++;

				if (!strnicmp(p, "alpha", 5))
					pArray[nNumSubroutines].nVersion = 0;
				if (!strnicmp(p, "beta", 4))
					pArray[nNumSubroutines].nVersion = 1;
				if (!strnicmp(p, "gold", 4))
					pArray[nNumSubroutines].nVersion = 2;
				if (!strnicmp(p, "normal", 6))
					pArray[nNumSubroutines].nCondition = 0;
				if (!strnicmp(p, "infected", 8) || !strnicmp(p, "corrupted", 9))
					pArray[nNumSubroutines].nCondition = 1;

				while (isalnum(*p) || *p == '_')
					p++;
			}

			nNumSubroutines++;
			pTok = strtok(NULL,",");
		}
	}

	return nNumSubroutines;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildProceduralsList()
//
//	PURPOSE:	Builds a list of procedural names from a string
//
//	RETURNS:	Number of procedurals added to list
//
// ----------------------------------------------------------------------- //

// RETAIL: CShell 0x1013d840.  The name of the function is ours.
int MISSION::BuildProceduralsList(char* pszStr, MISSIONPROCEDURAL* pArray, int nArrayLen)
{
	if (!pArray || !pszStr) return 0;

	int  nNumProcedurals = 0;
	char buf[512] = "";

	while (*pszStr == ' ' || *pszStr == '\t')
		pszStr++;

	if (strlen(pszStr))
	{
		SAFE_STRCPY(buf, pszStr);
		char *pTok = strtok(buf,",");
		while (pTok && nNumProcedurals < nArrayLen)
		{
			SAFE_STRCPY(pArray[nNumProcedurals].szName, pTok);
			nNumProcedurals++;
			pTok = strtok(NULL,",");
		}
	}

	return nNumProcedurals;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LEVEL::LEVEL
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

// RETAIL: CShell 0x1013d920.
LEVEL::LEVEL()
{ 
	nNameId			= -1;
	nBriefingId		= -1;
	nHelpId			= -1;

	szLevel[0] = '\0';

	nNumDefaultWeapons		= 0;
	nNumIdealWeapons		= 0;
	nNumIdealSubroutines	= 0;
	// RETAIL: nNumIdealProcedurals is not set here.
	nNumDefaultAmmo			= 0;
	nNumDefaultMods			= 0;
	nBuildNotes				= 0;
	nIdealBuildPoints		= 0;

	aDefaultAmmo[0]		= WMGR_INVALID_ID;
	aDefaultMods[0]		= WMGR_INVALID_ID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONWEAPON::MISSIONWEAPON etc.
//
//	PURPOSE:	Constructors
//
// ----------------------------------------------------------------------- //

// RETAIL: CShell 0x1013d9d0, 0x1013d9e0, 0x1013d9f0 (out of line, at the end
// of the module).
MISSIONWEAPON::MISSIONWEAPON()
{
	szName[0] = '\0';
	nVersion = 0;
	nCondition = 0;
}

MISSIONSUBROUTINE::MISSIONSUBROUTINE()
{
	szName[0] = '\0';
	nVersion = 0;
	nCondition = 0;
}

MISSIONPROCEDURAL::MISSIONPROCEDURAL()
{
	szName[0] = '\0';
}
