// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleGameDataMgr.cpp
//
// PURPOSE : Tron's sets of saved single player light cycle game settings
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's, written from retail CShell.dll (TU 0x10046e30..
// 0x10048e50; see LightCycleGameDataMgr.h)

#include "stdafx.h"
#include "LightCycleGameDataMgr.h"
#include "ServerOptionMgr.h"
#include "ProfileMgr.h"
#include "WinUtil.h"
#include <io.h>

// RETAIL: the folder and extension of the settings files (0x1026dd28,
// 0x1026dd18; initialised at 0x10046e40 and 0x10046f40)
static std::string s_sGameDataDir = "LightCycleGameData\\";
static std::string s_sGameDataExt = ".txt";

CLightCycleGameDataMgr* g_pLightCycleGameDataMgr = LTNULL;

// RETAIL: string 1500, the default set's name (as CServerOptionMgr's)
#define IDS_LIGHTCYCLEGAMEDATA_DEFAULT	1500

// The custom maps a set keeps (Map00..Map31)
#define LCGD_MAX_CUSTOM_MAPS		32

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::Init
//
//	PURPOSE:	Read the sets and select the profile's (RETAIL: 0x10047030)
//
// ----------------------------------------------------------------------- //

bool CLightCycleGameDataMgr::Init()
{
	g_pLightCycleGameDataMgr = this;

	if (!CWinUtil::DirExist("LightCycleGameData"))
	{
		CWinUtil::CreateDir("LightCycleGameData");
	}

	m_nCurrent = (uint16)-1;
	LoadAll();

	// PORT: retail selects the set named by a string of the current profile
	// (retail CUserProfile +0x1f4, 0x10047084; not yet in our profile's
	// layout) when it is not empty, else the default set; the default set
	// stands in
	HSTRING hDefault = g_pLTClient->FormatString(IDS_LIGHTCYCLEGAMEDATA_DEFAULT);
	const char* pszName = g_pLTClient->GetStringData(hDefault);

	bool bFailed = !Select(std::string(pszName), false);
	if (bFailed)
	{
		Select(std::string(g_pLTClient->GetStringData(hDefault)), true);
	}

	g_pLTClient->FreeString(hDefault);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::Term
//
//	PURPOSE:	Free the sets (RETAIL: 0x10047200)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameDataMgr::Term()
{
	for (uint16 i = 0; i < m_Data.size(); i++)
	{
		debug_delete(m_Data[i]);
	}
	m_Data.clear();

	g_pLightCycleGameDataMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::GetCurrent
//
//	PURPOSE:	The set in use (RETAIL: 0x10047270)
//
// ----------------------------------------------------------------------- //

CLightCycleGameData* CLightCycleGameDataMgr::GetCurrent()
{
	if (m_nCurrent >= m_Data.size())
		return LTNULL;

	return m_Data[m_nCurrent];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::Select
//
//	PURPOSE:	Make the named set current (RETAIL: 0x10047290)
//
// ----------------------------------------------------------------------- //

bool CLightCycleGameDataMgr::Select(const std::string& sName, bool bCreate)
{
	if (sName.empty())
		return false;

	uint16 nIndex;
	for (nIndex = 0; nIndex < m_Data.size(); nIndex++)
	{
		if (stricmp(m_Data[nIndex]->m_sName.c_str(), sName.c_str()) == 0)
			break;
	}

	if (nIndex >= m_Data.size())
	{
		if (bCreate)
		{
			Add(sName);
			nIndex = m_Data.size() - 1;
		}
		else
		{
			return false;
		}
	}

	SetCurrent(nIndex);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::SetCurrent
//
//	PURPOSE:	Save the current set and make another current (RETAIL:
//				0x10047330)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameDataMgr::SetCurrent(uint16 nIndex)
{
	SaveCurrent();

	if (nIndex >= m_Data.size())
		return;

	m_nCurrent = nIndex;

	// PORT: retail keeps the set's name in a string of the current profile
	// (retail CUserProfile +0x1f4, 0x10047379; not yet in our profile's
	// layout) before saving the profile
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->Save();

	// RETAIL: the current host options keep the current set (0x100473d4)
	g_pServerOptionMgr->GetCurrent()->SetTronUnknownCC((uint32)GetCurrent());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::SaveCurrent
//
//	PURPOSE:	Write the current set to its file (RETAIL: 0x10047400)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameDataMgr::SaveCurrent()
{
	if (m_nCurrent >= m_Data.size())
		return;

	CLightCycleGameData* pData = GetCurrent();
	pData->Save(m_buteMgr);

	std::string sFile = s_sGameDataDir + pData->m_sName + s_sGameDataExt;
	m_buteMgr.Save(sFile.c_str());
	m_buteMgr.Term();
	m_buteMgr.Init();
	m_buteMgr.Parse(sFile.c_str());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::LoadAll
//
//	PURPOSE:	Read every set of the folder (RETAIL: 0x10047540)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameDataMgr::LoadAll()
{
	for (uint32 i = 0; i < m_Data.size(); i++)
	{
		debug_delete(m_Data[i]);
	}
	m_Data.clear();

	std::string sFind = s_sGameDataDir + "*" + s_sGameDataExt;

	struct _finddata_t file;
	long hFile = _findfirst(sFind.c_str(), &file);
	if (hFile != -1)
	{
		do
		{
			std::string sFile = s_sGameDataDir + file.name;
			m_buteMgr.Term();
			m_buteMgr.Init();
			if (m_buteMgr.Parse(sFile.c_str()))
			{
				CLightCycleGameData* pData = debug_new(CLightCycleGameData);
				char* pszName = strtok(file.name, ".");
				pData->m_sName.assign(pszName, pszName + strlen(pszName));
				pData->Load(m_buteMgr);
				m_Data.push_back(pData);
			}
		}
		while (_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);

	if (m_Data.size() == 0)
	{
		HSTRING hDefault = g_pLTClient->FormatString(IDS_LIGHTCYCLEGAMEDATA_DEFAULT);
		Select(std::string(g_pLTClient->GetStringData(hDefault)), true);
		g_pLTClient->FreeString(hDefault);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::Add
//
//	PURPOSE:	Add a set (RETAIL: 0x10047820)
//
// ----------------------------------------------------------------------- //

CLightCycleGameData* CLightCycleGameDataMgr::Add(const std::string& sName)
{
	CLightCycleGameData* pData = debug_new(CLightCycleGameData);
	pData->m_sName = sName;
	m_Data.push_back(pData);
	pData->Save(m_buteMgr);
	return pData;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::GetNames
//
//	PURPOSE:	The names of the sets (RETAIL: 0x100478e0)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameDataMgr::GetNames(StringList& lstNames)
{
	lstNames.clear();

	for (uint32 i = 0; i < m_Data.size(); i++)
	{
		if (!m_Data[i]->m_sName.empty())
		{
			lstNames.push_back(m_Data[i]->m_sName);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::Delete
//
//	PURPOSE:	Delete a set and its file (RETAIL: 0x10047970)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameDataMgr::Delete(std::string sName)
{
	bool bFound = false;
	std::vector<CLightCycleGameData*>::iterator iter = m_Data.begin();
	while (iter != m_Data.end() && !bFound)
	{
		bFound = (stricmp((*iter)->m_sName.c_str(), sName.c_str()) == 0);
		if (!bFound)
			iter++;
	}

	if (!bFound)
		return;

	CLightCycleGameData* pCurrent = GetCurrent();
	bool bCurrent = (*iter == pCurrent);
	std::string sCurrent = pCurrent->m_sName;

	CLightCycleGameData* pData = *iter;
	m_Data.erase(iter);
	debug_delete(pData);

	std::string sFile = s_sGameDataDir + sName + s_sGameDataExt;
	remove(sFile.c_str());

	if (bCurrent)
	{
		if (m_Data.size() > 0)
		{
			SetCurrent(0);
		}
		else
		{
			HSTRING hDefault = g_pLTClient->FormatString(IDS_LIGHTCYCLEGAMEDATA_DEFAULT);
			Select(std::string(g_pLTClient->GetStringData(hDefault)), true);
			g_pLTClient->FreeString(hDefault);
		}
	}
	else
	{
		Select(sCurrent, true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameDataMgr::Rename
//
//	PURPOSE:	Rename a set and its file (RETAIL: 0x10047bc0)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameDataMgr::Rename(const std::string& sOldName, const std::string& sNewName)
{
	std::string sOldFile = s_sGameDataDir + sOldName + s_sGameDataExt;
	std::string sNewFile = s_sGameDataDir + sNewName + s_sGameDataExt;
	rename(sOldFile.c_str(), sNewFile.c_str());

	uint16 nIndex;
	for (nIndex = 0; nIndex < m_Data.size(); nIndex++)
	{
		if (stricmp(m_Data[nIndex]->m_sName.c_str(), sOldName.c_str()) == 0)
			break;
	}

	if (nIndex < m_Data.size())
	{
		m_Data[nIndex]->m_sName = sNewName;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleCustomSettings::Reset
//
//	PURPOSE:	The defaults (RETAIL: 0x100486b0)
//
// ----------------------------------------------------------------------- //

void LightCycleCustomSettings::Reset()
{
	nMaxAICyclists = 4;
	nOpponentSkill = 2;
	nOpponentCycleType = 0;
	nPowerups = 0xffffffff;
	nGameSpeed = 1;
	nTrailLength = 1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameData::CLightCycleGameData
//
//	PURPOSE:	Constructor (RETAIL: 0x100486e0)
//
// ----------------------------------------------------------------------- //

CLightCycleGameData::CLightCycleGameData()
{
	Reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameData::Reset
//
//	PURPOSE:	The defaults (RETAIL: 0x10048760)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameData::Reset()
{
	m_Custom.Reset();
	m_bTronUnknownA0 = false;
	m_bTronUnknownA1 = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameData::IsValid
//
//	PURPOSE:	(RETAIL: 0x10048780; the name is ours)
//
// ----------------------------------------------------------------------- //

bool CLightCycleGameData::IsValid()
{
	if (m_Custom.nMaxAICyclists > 8) return false; return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameData::Save
//
//	PURPOSE:	Write the set (but the default's) (RETAIL: 0x10048790)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameData::Save(CButeMgr& buteMgr)
{
	if (stricmp(m_sName.c_str(), ".Default") == 0)
		return;

	IsValid();

	buteMgr.SetInt("CustomSettings", "MaxAICyclists", m_Custom.nMaxAICyclists);
	buteMgr.SetInt("CustomSettings", "OpponentSkill", m_Custom.nOpponentSkill);
	buteMgr.SetInt("CustomSettings", "OpponentCycleType", m_Custom.nOpponentCycleType);
	buteMgr.SetInt("CustomSettings", "GameSpeed", m_Custom.nGameSpeed);
	buteMgr.SetInt("CustomSettings", "TrailLength", m_Custom.nTrailLength);
	buteMgr.SetBool("CustomSettings", "PowerupAllowShield", m_Custom.bShield);
	buteMgr.SetBool("CustomSettings", "PowerupAllowTurbo", m_Custom.bTurbo);
	buteMgr.SetBool("CustomSettings", "PowerupAllowTrailSpike", m_Custom.bTrailSpike);
	buteMgr.SetBool("CustomSettings", "PowerupAllowMissile", m_Custom.bMissile);
	buteMgr.SetBool("CustomSettings", "PowerupAllowTurboCurse", m_Custom.bTurboCurse);
	buteMgr.SetBool("CustomSettings", "PowerupAllowGrab", m_Custom.bGrab);
	buteMgr.SetBool("CustomSettings", "PowerupAllowTrailReset", m_Custom.bTrailReset);
	buteMgr.SetBool("CustomSettings", "PowerupAllowForceUse", m_Custom.bForceUse);
	buteMgr.SetBool("CustomSettings", "PowerupAllowWallExtender", m_Custom.bWallExtender);

	buteMgr.SetString("CircuitSettings", "Map", m_Circuit.sNameKey.c_str());

	char szAttr[80];
	StringArray::iterator iter = m_Custom.aMaps.begin();
	for (int nMap = 0; nMap < LCGD_MAX_CUSTOM_MAPS; nMap++)
	{
		sprintf(szAttr, "Map%02d", nMap);
		if (iter != m_Custom.aMaps.end())
		{
			buteMgr.SetString("CustomMaps", szAttr, (*iter).c_str());
			iter++;
		}
		else
		{
			buteMgr.SetString("CustomMaps", szAttr, "");
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameData::Load
//
//	PURPOSE:	Read the set (RETAIL: 0x10048990)
//
// ----------------------------------------------------------------------- //

void CLightCycleGameData::Load(CButeMgr& buteMgr)
{
	m_Custom.nMaxAICyclists = buteMgr.GetInt("CustomSettings", "MaxAICyclists", m_Custom.nMaxAICyclists);
	m_Custom.nOpponentSkill = buteMgr.GetInt("CustomSettings", "OpponentSkill", m_Custom.nOpponentSkill);
	m_Custom.nOpponentCycleType = buteMgr.GetInt("CustomSettings", "OpponentCycleType", m_Custom.nOpponentCycleType);
	m_Custom.nGameSpeed = buteMgr.GetInt("CustomSettings", "GameSpeed", m_Custom.nGameSpeed);
	m_Custom.nTrailLength = buteMgr.GetInt("CustomSettings", "TrailLength", m_Custom.nTrailLength);

	m_Custom.nPowerups = 0;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowShield", m_Custom.bShield))
		m_Custom.bShield = true;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowTurbo", m_Custom.bTurbo))
		m_Custom.bTurbo = true;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowTrailSpike", m_Custom.bTrailSpike))
		m_Custom.bTrailSpike = true;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowMissile", m_Custom.bMissile))
		m_Custom.bMissile = true;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowTurboCurse", m_Custom.bTurboCurse))
		m_Custom.bTurboCurse = true;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowGrab", m_Custom.bGrab))
		m_Custom.bGrab = true;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowTrailReset", m_Custom.bTrailReset))
		m_Custom.bTrailReset = true;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowForceUse", m_Custom.bForceUse))
		m_Custom.bForceUse = true;
	if (buteMgr.GetBool("CustomSettings", "PowerupAllowWallExtender", m_Custom.bWallExtender))
		m_Custom.bWallExtender = true;

	char szMap[48];
	buteMgr.GetString("CircuitSettings", "Map", szMap, sizeof(szMap));
	m_Circuit.sNameKey = szMap;

	char szAttr[64];
	int nMap = 0;
	sprintf(szAttr, "Map%02d", nMap);
	m_Custom.aMaps.clear();
	while (buteMgr.Exist("CustomMaps", szAttr))
	{
		std::string sMap = buteMgr.GetString("CustomMaps", szAttr, "");
		if (!sMap.empty())
		{
			m_Custom.aMaps.push_back(sMap);
		}

		nMap++;
		sprintf(szAttr, "Map%02d", nMap);
		if (nMap >= LCGD_MAX_CUSTOM_MAPS)
			break;
	}

	IsValid();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameData::GetMaps
//
//	PURPOSE:	The maps of a game type (RETAIL: 0x10048e00)
//
// ----------------------------------------------------------------------- //

StringArray* CLightCycleGameData::GetMaps(int nGameType)
{
	switch (nGameType)
	{
		case eGameTypeSingleLightCycleCustom:
			return &m_Custom.aMaps;
	}

	// RETAIL: a function static (0x1026dd48)
	static StringArray s_aNoMaps;
	return &s_aNoMaps;
}
