// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleGameData.cpp
//
// PURPOSE : One set of single player light cycle game settings
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's, written from retail CShell.dll (0x100486b0..0x10048e5a)
// and Object.lto (0x10029c80..0x10029ec1). The two images have the same
// functions; each keeps only the functions it calls (/OPT:REF). See
// LightCycleGameData.h.

#include "stdafx.h"
#include "LightCycleGameData.h"
#include "NetDefs.h"

// The custom maps a set keeps (Map00..Map31)
#define LCGD_MAX_CUSTOM_MAPS		32

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleCustomSettings::Reset
//
//	PURPOSE:	The defaults (RETAIL: CShell.dll 0x100486b0,
//				Object.lto 0x10029c80)
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
//	PURPOSE:	Constructor (RETAIL: CShell.dll 0x100486e0, Object.lto
//				0x10029cb0)
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
//	PURPOSE:	The defaults (RETAIL: CShell.dll 0x10048760,
//				Object.lto 0x10029e10)
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
//	PURPOSE:	(RETAIL: CShell.dll 0x10048780; the name is ours)
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
//	PURPOSE:	Write the set (but the default's) (RETAIL: CShell.dll 0x10048790)
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
//	PURPOSE:	Read the set (RETAIL: CShell.dll 0x10048990)
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
//	PURPOSE:	The maps of a game type (RETAIL: CShell.dll 0x10048e00)
//
// ----------------------------------------------------------------------- //

StringArray* CLightCycleGameData::GetMaps(int nGameType)
{
	switch (nGameType)
	{
		case eGameTypeSingleLightCycleCustom:
			return &m_Custom.aMaps;
	}

	// RETAIL: a function static (CShell.dll 0x1026dd48)
	static StringArray s_aNoMaps;
	return &s_aNoMaps;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameData::GetWave
//
//	PURPOSE:	A wave of the circuit; a blank wave when out of range
//				(RETAIL: Object.lto 0x10029e30; the name is ours)
//
// ----------------------------------------------------------------------- //

const CircuitWave& CLightCycleGameData::GetWave(uint8 nWave)
{
	if (nWave >= m_Circuit.aWaves.size())
	{
		// RETAIL: a function static (Object.lto 0x10358c48)
		static CircuitWave s_BlankWave;
		return s_BlankWave;
	}

	return m_Circuit.aWaves[nWave];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleGameData::GetNumWaves
//
//	PURPOSE:	The number of waves of the circuit (RETAIL: Object.lto
//				0x10029eb0; the name is ours)
//
// ----------------------------------------------------------------------- //

uint8 CLightCycleGameData::GetNumWaves()
{
	return m_Circuit.aWaves.size();
}
