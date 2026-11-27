/*
 * gamecommands.cpp -- the modern build's console commands of the game
 * client: Map and UnlockCycles (docs/MODERN.md CL-50, CL-51;
 * docs/CONSOLE.md). #included once, by TronGameClientShell.cpp, in a
 * TRON20SRC_MODERN block (CShell.dll's project file is the retail one, as
 * for xinput/clientpad.cpp).
 *
 * Map [name]: start a world by its name, in single player only. The worlds
 * are the .dat files under Worlds\ in the resources (the engine's file
 * tree: the .rez archives and the directories), with the names of the
 * mission file (Attributes\Missions.txt) and the circuit file
 * (Attributes\Circuits.txt) where those name them. A name is the world's
 * file name (m05-01, LC_02s03, intro), or its path with or without
 * "Worlds\" (RetailLightCycle\Zone\Map0); without case; ".dat" may be
 * given. With no name: the list of the worlds. The world starts as the
 * game starts a level from its menus and from +runworld: the mission
 * manager's StartGameFromLevel on a single-player server, with the mission
 * file of single player. So a level of a mission gets that mission's data
 * (its name on the loading screen, its default weapons, its briefing and
 * subroutine screens where the level has them), and saving works as in a
 * level that the campaign started. A light-cycle arena that is a circuit's
 * map starts as the circuit screen starts it (that circuit, with its waves,
 * lives and unlocks, the profile's cycle); any other world starts as a
 * custom level (no mission). Each start is a new game: the player has only
 * the defaults of the mission and the level (see docs/CONSOLE.md).
 *
 * UnlockCycles: the current profile gets every light-cycle color (each
 * shade's regular and super cycle, Attributes\LightCycles.txt), every map
 * of the custom light-cycle game that a circuit locks, and every power-up,
 * and is saved. The circuits themselves are never locked: retail's
 * CScreenLightCycleCircuit::IsCircuitLocked (0x10067660) returns false,
 * and no circuit has InitiallyLock TRUE.
 */

#include "LightCycleGameDataMgr.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"
#include "ClientMultiplayerMgr.h"
#include "GameClientShell.h"
#include "CircuitButeMgr.h"
#include "TRON/LightCycleButeMgr.h"
#include "ProfileMgr.h"
#include "MissionButeMgr.h"

// A world's name without its folders
static const char *ModernWorldBaseName(const char *pszWorld)
{
	const char *pszSlash = strrchr(pszWorld, '\\');
	return pszSlash ? pszSlash + 1 : pszWorld;
}

// The name without ".dat", with backslashes
static std::string ModernWorldName(const char *pszName)
{
	std::string sName(pszName);
	for (uint32 i = 0; i < sName.size(); i++)
	{
		if (sName[i] == '/')
			sName[i] = '\\';
	}
	if (sName.size() > 4 && !stricmp(sName.c_str() + sName.size() - 4, ".dat"))
		sName.erase(sName.size() - 4);
	return sName;
}

static void ModernAddWorld(StringArray &aWorlds, const std::string &sWorld)
{
	for (StringArray::iterator it = aWorlds.begin(); it != aWorlds.end(); ++it)
	{
		if (!stricmp(it->c_str(), sWorld.c_str()))
			return;
	}
	aWorlds.push_back(sWorld);
}

// Every .dat file under the folder, and under its folders
static void ModernFindWorldFiles(const char *pszDir, StringArray &aWorlds, int nDepth)
{
	if (nDepth > 8)
		return;

	FileEntry *pFiles = g_pLTClient->GetFileList(pszDir);
	for (FileEntry *pFile = pFiles; pFile; pFile = pFile->m_pNext)
	{
		if (pFile->m_Type == TYPE_DIRECTORY)
		{
			ModernFindWorldFiles(pFile->m_pFullFilename, aWorlds, nDepth + 1);
		}
		else if (pFile->m_Type == TYPE_FILE)
		{
			uint32 nLen = strlen(pFile->m_pFullFilename);
			if (nLen > 4 && !stricmp(pFile->m_pFullFilename + nLen - 4, ".dat"))
				ModernAddWorld(aWorlds, ModernWorldName(pFile->m_pFullFilename));
		}
	}
	g_pLTClient->FreeFileList(pFiles);
}

// The worlds, in the sequence of the campaign, then the circuits, then
// the other files. The names of the mission and circuit files are kept:
// the mission manager finds a level of a mission by its whole name
static void ModernFindWorlds(StringArray &aWorlds)
{
	aWorlds.clear();

	if (g_pMissionButeMgr)
	{
		int nMissions = g_pMissionButeMgr->GetNumMissions();
		for (int nMission = 0; nMission < nMissions; nMission++)
		{
			MISSION *pMission = g_pMissionButeMgr->GetMission(nMission);
			if (!pMission)
				continue;
			for (int nLevel = 0; nLevel < pMission->nNumLevels; nLevel++)
			{
				if (pMission->aLevels[nLevel].szLevel[0])
					ModernAddWorld(aWorlds, ModernWorldName(pMission->aLevels[nLevel].szLevel));
			}
		}
	}

	CircuitButeMgr *pCircuits = g_pGameClientShell ? g_pGameClientShell->GetCircuitButeMgr() : LTNULL;
	if (pCircuits)
	{
		int nCircuits = pCircuits->GetNumCircuits();
		for (int nCircuit = 0; nCircuit < nCircuits; nCircuit++)
		{
			const std::string &sMap = pCircuits->GetCircuit(nCircuit).sMapName;
			if (!sMap.empty())
				ModernAddWorld(aWorlds, ModernWorldName(sMap.c_str()));
		}
	}

	// The files: only those that are there (the mission file could name a
	// world that the resources do not have)
	StringArray aFiles;
	ModernFindWorldFiles("Worlds", aFiles, 0);

	StringArray aKnown;
	aKnown.swap(aWorlds);
	StringArray::iterator it;
	for (it = aKnown.begin(); it != aKnown.end(); ++it)
	{
		for (StringArray::iterator itFile = aFiles.begin(); itFile != aFiles.end(); ++itFile)
		{
			if (!stricmp(it->c_str(), itFile->c_str()))
			{
				aWorlds.push_back(*it);
				break;
			}
		}
	}
	for (it = aFiles.begin(); it != aFiles.end(); ++it)
		ModernAddWorld(aWorlds, *it);
}

// The worlds whose name, path, or path after "Worlds\" is the name
static void ModernMatchWorlds(const StringArray &aWorlds, const char *pszName, StringArray &aMatches)
{
	std::string sName = ModernWorldName(pszName);
	std::string sPath = "Worlds\\" + sName;

	aMatches.clear();
	for (StringArray::const_iterator it = aWorlds.begin(); it != aWorlds.end(); ++it)
	{
		if (!stricmp(it->c_str(), sName.c_str()) || !stricmp(it->c_str(), sPath.c_str()) ||
			!stricmp(ModernWorldBaseName(it->c_str()), sName.c_str()))
		{
			aMatches.push_back(*it);
		}
	}
}

// The list: each folder, then its worlds' names, some to a line
static void ModernPrintWorlds(const StringArray &aWorlds)
{
	StringArray aDirs;
	StringArray::const_iterator it;
	for (it = aWorlds.begin(); it != aWorlds.end(); ++it)
	{
		const char *pszBase = ModernWorldBaseName(it->c_str());
		ModernAddWorld(aDirs, std::string(it->c_str(), pszBase - it->c_str()));
	}

	for (StringArray::iterator itDir = aDirs.begin(); itDir != aDirs.end(); ++itDir)
	{
		g_pLTClient->CPrint("%s", itDir->empty() ? "(no folder)" : itDir->c_str());

		char szLine[128] = "";
		for (it = aWorlds.begin(); it != aWorlds.end(); ++it)
		{
			const char *pszBase = ModernWorldBaseName(it->c_str());
			if (stricmp(std::string(it->c_str(), pszBase - it->c_str()).c_str(), itDir->c_str()))
				continue;

			if (szLine[0] && strlen(szLine) + 2 + strlen(pszBase) > 72)
			{
				g_pLTClient->CPrint("  %s", szLine);
				szLine[0] = '\0';
			}
			if (szLine[0])
				strcat(szLine, "  ");
			strncat(szLine, pszBase, sizeof(szLine) - strlen(szLine) - 1);
		}
		if (szLine[0])
			g_pLTClient->CPrint("  %s", szLine);
	}
	g_pLTClient->CPrint("%d worlds. Map <name> starts one (single player)", aWorlds.size());
}

// Start the world as the game's menus start it
static bool ModernStartWorld(const std::string &sWorld)
{
	if (!g_pMissionButeMgr->Init(MISSION_DEFAULT_FILE))
		return false;

	g_pClientSaveLoadMgr->SetUseMultiplayerFolders(false);

	// A circuit's map: as the circuit screen launches it (LaunchCircuit,
	// 0x100669a0)
	CircuitButeMgr *pCircuits = g_pGameClientShell->GetCircuitButeMgr();
	CLightCycleGameData *pData = g_pLightCycleGameDataMgr ? g_pLightCycleGameDataMgr->GetCurrent() : LTNULL;
	CUserProfile *pProfile = g_pProfileMgr ? g_pProfileMgr->GetCurrentProfile() : LTNULL;
	int nCircuits = pCircuits ? pCircuits->GetNumCircuits() : 0;
	int nCircuit;
	for (nCircuit = 0; nCircuit < nCircuits; nCircuit++)
	{
		if (!stricmp(ModernWorldName(pCircuits->GetCircuit(nCircuit).sMapName.c_str()).c_str(), sWorld.c_str()))
			break;
	}

	if (nCircuit < nCircuits && pData && pProfile)
	{
		const Circuit &circuit = pCircuits->GetCircuit(nCircuit);
		g_pGameClientShell->SetGameType(eGameTypeSingleLightCycleCircuit);
		pData->m_Circuit = circuit;
		pData->m_bTronUnknownA0 = (uint8)pProfile->m_nLightCycleStyle;
		pData->m_bTronUnknownA1 = (uint8)pProfile->m_nLightCycleShade;
		g_pLightCycleGameDataMgr->SaveCurrent();

		if (!g_pClientMultiplayerMgr->SetupServerSinglePlayer())
			return false;
		return g_pMissionMgr->StartGameFromLevel(circuit.sMapName.c_str());
	}

	g_pGameClientShell->SetGameType(eGameTypeSingle);
	if (!g_pClientMultiplayerMgr->SetupServerSinglePlayer())
		return false;
	return g_pMissionMgr->StartGameFromLevel(sWorld.c_str());
}

void ModernMapFn(int argc, char **argv)
{
	StringArray aWorlds;
	ModernFindWorlds(aWorlds);

	if (argc < 1)
	{
		ModernPrintWorlds(aWorlds);
		return;
	}

	if (IsMultiplayerGame())
	{
		g_pLTClient->CPrint("Map: only in single player");
		return;
	}

	StringArray aMatches;
	ModernMatchWorlds(aWorlds, argv[0], aMatches);
	if (aMatches.empty())
	{
		g_pLTClient->CPrint("Map: no world %s (Map alone lists them)", argv[0]);
		return;
	}
	if (aMatches.size() > 1)
	{
		g_pLTClient->CPrint("Map: %s is more than one world; give its folder:", argv[0]);
		for (StringArray::iterator it = aMatches.begin(); it != aMatches.end(); ++it)
			g_pLTClient->CPrint("  %s", it->c_str());
		return;
	}

	g_pLTClient->CPrint("Map: %s", aMatches[0].c_str());
	if (!ModernStartWorld(aMatches[0]))
	{
		g_pLTClient->CPrint("Map: %s did not start", aMatches[0].c_str());
		return;
	}

	// The console goes away, so that the loading screen gets the keys. A
	// new client shell puts it away (from the menus), but a level that
	// follows a level keeps the shell. (The game never turns the console
	// off, so it is turned on again at once.)
	g_pLTClient->SetConsoleEnable(false);
	g_pLTClient->SetConsoleEnable(true);
}

// Add the name if the list does not have it; true if added
static bool ModernUnlock(StringArray &aNames, const char *pszName)
{
	if (!pszName || !pszName[0])
		return false;

	for (StringArray::iterator it = aNames.begin(); it != aNames.end(); ++it)
	{
		if (!stricmp(it->c_str(), pszName))
			return false;
	}
	aNames.push_back(std::string(pszName));
	return true;
}

void ModernUnlockCyclesFn(int argc, char **argv)
{
	CUserProfile *pProfile = g_pProfileMgr ? g_pProfileMgr->GetCurrentProfile() : LTNULL;
	CircuitButeMgr *pCircuits = g_pGameClientShell ? g_pGameClientShell->GetCircuitButeMgr() : LTNULL;
	if (!pProfile || !pCircuits || !g_pLightCycleButeMgr)
	{
		g_pLTClient->CPrint("UnlockCycles: no profile");
		return;
	}

	int nColors = 0, nMaps = 0, nPowerups = 0;
	uint32 i;

	// The colors: each shade's regular and super cycle
	uint32 nShades = g_pLightCycleButeMgr->GetNumShades();
	for (i = 0; i < nShades; i++)
	{
		LIGHTCYCLESHADE *pShade = g_pLightCycleButeMgr->GetShade(i);
		if (!pShade)
			continue;
		for (int nStyle = 0; nStyle < kNumLCShadeStyles; nStyle++)
		{
			if (ModernUnlock(pProfile->m_UnlockedCycleColors, pShade->szCycleColors[nStyle]))
				nColors++;
		}
	}

	// The maps: the circuits' unlocks that are maps (as TronMissionMgr
	// adds them), and each map of the custom game's folder that a circuit
	// locks (the custom map screen's test, IsMapLocked, 0x1006b510, on the
	// file name)
	int nCircuits = pCircuits->GetNumCircuits();
	for (int nCircuit = 0; nCircuit < nCircuits; nCircuit++)
	{
		const CircuitStringArray &aUnlocks = pCircuits->GetCircuit(nCircuit).aUnlocks;
		for (CircuitStringArray::const_iterator it = aUnlocks.begin(); it != aUnlocks.end(); ++it)
		{
			if (pCircuits->IsCircuitMap(*it) && ModernUnlock(pProfile->m_UnlockedMaps, it->c_str()))
				nMaps++;
		}
	}
	FileEntry *pFiles = g_pLTClient->GetFileList("Worlds\\RetailLightCycle\\");
	for (FileEntry *pFile = pFiles; pFile; pFile = pFile->m_pNext)
	{
		if (pFile->m_Type != TYPE_FILE)
			continue;
		uint32 nLen = strlen(pFile->m_pBaseFilename);
		if (nLen > 4 && !stricmp(pFile->m_pBaseFilename + nLen - 4, ".dat") &&
			pCircuits->IsCircuitMap(std::string(pFile->m_pBaseFilename)) &&
			ModernUnlock(pProfile->m_UnlockedMaps, pFile->m_pBaseFilename))
		{
			nMaps++;
		}
	}
	g_pLTClient->FreeFileList(pFiles);

	// The power-ups
	for (i = 0; i < kNumLCPowerupTypes; i++)
	{
		if (ModernUnlock(pProfile->m_UnlockedPowerups, g_aszLCPowerupNames[i]))
			nPowerups++;
	}

	pProfile->Save();

	g_pLTClient->CPrint("UnlockCycles: %s: %d colors, %d maps and %d power-ups unlocked (now %d, %d, %d)",
		pProfile->m_sName.c_str(), nColors, nMaps, nPowerups,
		pProfile->m_UnlockedCycleColors.size(), pProfile->m_UnlockedMaps.size(),
		pProfile->m_UnlockedPowerups.size());
}
