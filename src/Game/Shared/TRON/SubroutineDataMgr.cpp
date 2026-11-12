// ----------------------------------------------------------------------- //
//
// MODULE  : SubroutineDataMgr.cpp
//
// PURPOSE : Per-version/state/difficulty data of the Tron subroutines.
//
// RETAIL: no source of this file survives.  Written from the retail code,
//         CShell.dll 0x10084080..0x10090fae (see SubroutineDataMgr.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SubroutineDataMgr.h"
#include "WeaponMgr.h"

CSubroutineDataMgr* g_pSubroutineDataMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineDataMgr::FindTag
//
//	PURPOSE:	Find the "<szTagBase><n>" tag whose szAttrib is szValue
//
//	RETAIL:		CShell 0x10084230.
//
// ----------------------------------------------------------------------- //

char* CSubroutineDataMgr::FindTag(const char* szTagBase, const char* szAttrib, const char* szValue,
								  char* szResult, uint32 nResultLen)
{
	CButeMgr* pButeMgr = m_pButeMgr;

	char szTag[128];
	char szBuffer[128];

	int i = 0;
	while (true)
	{
		sprintf(szTag, "%s%d", szTagBase, i);
		i++;
		if (!pButeMgr->Exist(szTag))
		{
			LTStrCpy(szResult, "", nResultLen);
			return LTNULL;
		}

		m_pButeMgr->GetString(szTag, szAttrib, szBuffer, sizeof(szBuffer));
		if (!m_pButeMgr->Success())
		{
#ifdef _CLIENTBUILD
			g_pLTClient->CPrint(" Cannot find Tag: %s in Label: %s\n", szTag, szAttrib);
#else
			g_pLTServer->CPrint(" Cannot find Tag: %s in Label: %s\n", szTag, szAttrib);
#endif
			continue;
		}

		if (strcmp(szValue, szBuffer) == 0)
			break;
	}

	LTStrCpy(szResult, szTag, nResultLen);
	return szResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineDataMgr::Init
//
//	PURPOSE:	Read all the subroutine data
//
//	RETAIL:		CShell 0x10084370.
//
// ----------------------------------------------------------------------- //

bool CSubroutineDataMgr::Init(CButeMgr* pButeMgr)
{
	m_pButeMgr = pButeMgr;
	g_pSubroutineDataMgr = this;

	m_Versions.push_back(kSubData_Alpha);
	m_Versions.push_back(kSubData_Beta);
	m_Versions.push_back(kSubData_Gold);

	m_States.push_back(kSubData_Okay);
	m_States.push_back(kSubData_Corrupt);

	m_Difficulties.push_back(kSubData_Easy);
	m_Difficulties.push_back(kSubData_Normal);
	m_Difficulties.push_back(kSubData_Hard);
	m_Difficulties.push_back(kSubData_VeryHard);

	m_nNumVersions = m_Versions.size();
	m_nNumStates = m_States.size();
	m_nNumDifficulties = m_Difficulties.size();
	m_nNumKeys = m_nNumVersions * m_nNumStates * m_nNumDifficulties;

	// Map the subroutine names to their string ids.

	char szTag[128];
	int i = 0;
	while (true)
	{
		sprintf(szTag, "%s%d", "Subroutine", i);
		i++;
		const char* szName = m_pButeMgr->GetString(szTag, "Name");
		if (!m_pButeMgr->Success() || !szName)
			break;

		uint16 nNameId = (uint16)m_pButeMgr->GetInt(szTag, "NameId", 0);
		m_mapNameIds[CaselessString(szName)] = nNameId;
	}

	LoadData(m_vecSequencer, "Sequencer");
	LoadData(m_vecCluster, "Cluster");
	LoadData(m_vecSuffusion, "Suffusion");
	LoadData(m_vecLOL, "LOL");
	LoadData(m_vecBallLauncher, "Ball_launcher");
	LoadData(m_vecDrunkenDims, "Drunken_dims");
	LoadData(m_vecEnergyClaw, "Energy_claw");
	LoadData(m_vecPranksterBit, "Prankster_bit");
	LoadData(m_vecFuzzySignature, "Fuzzy_signature");
	LoadData(m_vecShieldPrism, "Shield_prism");
	LoadData(m_vecMegaHurtz, "MegaHurtz");
	LoadData(m_vecCorrode, "Corrode");
	LoadData(m_vecTriangulate, "Triangulate");
	LoadData(m_vecYAmp, "YAmp");
	LoadData(m_vecPrimitiveCharge, "Primitive_charge");
	LoadData(m_vecProfiler, "Profiler");
	LoadData(m_vecVirusScan, "Virus_scan");

	m_pButeMgr = LTNULL;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineDataMgr::Read
//
//	PURPOSE:	Read one subroutine's data for one key.
//
//	RETAIL:		CShell 0x10084950 (Sequencer) through 0x10088660 (Virus_scan),
//				in this order.
//
// ----------------------------------------------------------------------- //

void CSubroutineDataMgr::Read(const char* szTag, SequencerData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fDiscResponsivenessMultiplier = reader.GetFloat("DiscResponsivenessMultiplier");
	pData->nMaxAmmoOverride = reader.GetFloat("MaxAmmoOverride");
	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, ClusterData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	reader.GetString("BurstEffectName", pData->szBurstEffectName, sizeof(pData->szBurstEffectName));

	char szAmmo[256];
	reader.GetString("BurstExplosionAmmoName", szAmmo, sizeof(szAmmo));
	AMMO const* pAmmo = g_pWeaponMgr->GetAmmo(szAmmo);
	if (pAmmo)
	{
		pData->nBurstExplosionAmmoId = pAmmo->nId;
	}
	else
	{
		pData->nBurstExplosionAmmoId = WMGR_INVALID_ID;
	}

	pData->fDiscResponsivenessMultiplier = reader.GetFloat("DiscResponsivenessMultiplier");
	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fExplosionDamageMultiplier = reader.GetFloat("ExplosionDamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, SuffusionData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->nVectorsPerRound = reader.GetFloat("VectorsPerRound");
	pData->fPerturbMultiplier = reader.GetFloat("PerturbMultiplier");
	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, LOLData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fFireAnimationRateMultiplier = reader.GetFloat("FireAnimationRateMultiplier");
	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, BallLauncherData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fRandomnessMultiplier = reader.GetFloat("RandomnessMultiplier");
	pData->fFireAnimationRateMultiplier = reader.GetFloat("FireAnimationRateMultiplier");
	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fExplosionDamageMultiplier = reader.GetFloat("ExplosionDamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, DrunkenDimsData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fRandomnessMultiplier = reader.GetFloat("RandomnessMultiplier");
	pData->nVectorsPerRound = (int)reader.GetFloat("VectorsPerRound");
	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fExplosionDamageMultiplier = reader.GetFloat("ExplosionDamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, EnergyClawData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fRange = reader.GetFloat("Range");
	pData->nInRangePVAttachFX = (int)reader.GetFloat("InRangePVAttachFX");
	pData->fRechargeRate = reader.GetFloat("RechargeRate");
	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, PranksterBitData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	reader.GetString("ClassDataName", pData->szClassDataName, sizeof(pData->szClassDataName));
	pData->fLifetime = reader.GetFloat("Lifetime");
	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, FuzzySignatureData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fLoudChance = reader.GetFloat("LoudChance");
	pData->fAISoundVolumeScalar = reader.GetFloat("AISoundVolumeScalar");
	pData->fAIAlternateVolume = reader.GetFloat("AIAlternateVolume");
}

void CSubroutineDataMgr::Read(const char* szTag, ShieldPrismData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fDamageMin = reader.GetFloat("DamageMin");
	pData->fDamageMax = reader.GetFloat("DamageMax");
	pData->fVelocity = reader.GetFloat("Velocity");
}

void CSubroutineDataMgr::Read(const char* szTag, MegaHurtzData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, CorrodeData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fCorrodeDuration = reader.GetFloat("CorrodeDuration");
	pData->fCorrodeAmount = reader.GetFloat("CorrodeAmount");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, TriangulateData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fSwayAmountMultiplier = reader.GetFloat("SwayAmountMultiplier");
	pData->fSwaySpeedMultiplier = reader.GetFloat("SwaySpeedMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, YAmpData* pData, const SubroutineDataKey & key)
{
	// RETAIL: 0x10087fc0 sets the reader up and reads nothing.
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);
}

void CSubroutineDataMgr::Read(const char* szTag, PrimitiveChargeData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fDamageMultiplier = reader.GetFloat("DamageMultiplier");
	pData->fEnergyUseMultiplier = reader.GetFloat("EnergyUseMultiplier");
}

void CSubroutineDataMgr::Read(const char* szTag, ProfilerData* pData, const SubroutineDataKey & key)
{
	// RETAIL: 0x100884c0 sets the reader up and reads nothing.
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);
}

void CSubroutineDataMgr::Read(const char* szTag, VirusScanData* pData, const SubroutineDataKey & key)
{
	CSubroutineDataReader reader(m_pButeMgr, szTag, key);

	pData->fFailureChance = reader.GetFloat("FailureChance");
	pData->fChanceToCorrupt = reader.GetFloat("ChanceToCorrupt");
}
