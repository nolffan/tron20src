// ----------------------------------------------------------------------- //
//
// MODULE  : SubroutineDataMgr.h
//
// PURPOSE : Per-version/state/difficulty data of the Tron subroutines
//           (weapons and utilities), read from the subroutine attribute file.
//
// RETAIL: no source of this file survives.  Written from the retail code
//         (CShell.dll 0x10084080..0x10090fae, vtable 0x101e8848 with RTTI
//         CSubroutineDataMgr; Object.lto has the same class, vtable
//         0x10294f78).  Every name here except CSubroutineDataMgr and the
//         attribute strings is ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SUBROUTINE_DATA_MGR_H__
#define __SUBROUTINE_DATA_MGR_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include "ltbasedefs.h"
#include "ButeMgr.h"
#include "CaselessString.h"

#pragma warning( disable : 4786 )
#include <vector>
#include <map>
#include <string>

class CSubroutineDataMgr;
extern CSubroutineDataMgr* g_pSubroutineDataMgr;	// RETAIL: CShell 0x1026ec34

// The three axes the data varies along.  RETAIL: the values are the ones Init
// pushes; the names are the attribute name pieces ("Alpha", "Corrupt", "Easy"...).
enum SubDataVersion
{
	kSubData_Alpha = 0,
	kSubData_Beta,
	kSubData_Gold,
};

enum SubDataState
{
	kSubData_Okay = 0,
	kSubData_Corrupt,
};

enum SubDataDifficulty
{
	kSubData_Easy = 0,
	kSubData_Normal,
	kSubData_Hard,
	kSubData_VeryHard,
};

struct SubroutineDataKey
{
	SubroutineDataKey(int nVersion, int nState, int nDifficulty)
	{
		m_nVersion = nVersion;
		m_nState = nState;
		m_nDifficulty = nDifficulty;
	}

	int		m_nVersion;
	int		m_nState;
	int		m_nDifficulty;
};

// The per-subroutine data, in attribute file order.

struct SequencerData
{
	float	fDiscResponsivenessMultiplier;
	int		nMaxAmmoOverride;
	float	fDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct ClusterData
{
	char	szBurstEffectName[128];
	int		nBurstExplosionAmmoId;
	float	fDiscResponsivenessMultiplier;
	float	fDamageMultiplier;
	float	fExplosionDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct SuffusionData
{
	int		nVectorsPerRound;
	float	fPerturbMultiplier;
	float	fDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct LOLData
{
	float	fFireAnimationRateMultiplier;
	float	fDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct BallLauncherData
{
	float	fRandomnessMultiplier;
	float	fFireAnimationRateMultiplier;
	float	fDamageMultiplier;
	float	fExplosionDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct DrunkenDimsData
{
	float	fRandomnessMultiplier;
	int		nVectorsPerRound;
	float	fDamageMultiplier;
	float	fExplosionDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct EnergyClawData
{
	float	fRange;
	int		nInRangePVAttachFX;
	float	fRechargeRate;
	float	fDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct PranksterBitData
{
	char	szClassDataName[128];
	float	fLifetime;
	float	fDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct FuzzySignatureData
{
	float	fLoudChance;
	float	fAISoundVolumeScalar;
	float	fAIAlternateVolume;
};

struct ShieldPrismData
{
	float	fDamageMin;
	float	fDamageMax;
	float	fVelocity;
};

struct MegaHurtzData
{
	float	fDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct CorrodeData
{
	float	fCorrodeDuration;
	float	fCorrodeAmount;
	float	fEnergyUseMultiplier;
};

struct TriangulateData
{
	float	fSwayAmountMultiplier;
	float	fSwaySpeedMultiplier;
};

struct YAmpData
{
};

struct PrimitiveChargeData
{
	float	fDamageMultiplier;
	float	fEnergyUseMultiplier;
};

struct ProfilerData
{
};

struct VirusScanData
{
	float	fFailureChance;
	float	fChanceToCorrupt;
};

// Reads the attributes of one subroutine for one key: the attribute names are
// the base name followed by the state and difficulty ("DamageMultiplierCorruptHard"),
// the vector's x/y/z are the Alpha/Beta/Gold values.
class CSubroutineDataReader
{
public:

	CSubroutineDataReader(CButeMgr* pButeMgr, const char* szTag, const SubroutineDataKey & key)
		: m_pButeMgr(pButeMgr), m_Key(key)
	{
		SAFE_STRCPY(m_szTag, szTag);

		char szVersion[16];
		char szState[16];
		char szDifficulty[16];

		switch (key.m_nVersion)
		{
			case kSubData_Alpha:	strcpy(szVersion, "Alpha");	break;
			case kSubData_Beta:		strcpy(szVersion, "Beta");	break;
			case kSubData_Gold:		strcpy(szVersion, "Gold");	break;
			default:				strcpy(szVersion, "");		break;
		}

		switch (key.m_nState)
		{
			case kSubData_Okay:		strcpy(szState, "");		break;
			case kSubData_Corrupt:	strcpy(szState, "Corrupt");	break;
			default:				strcpy(szState, "");		break;
		}

		switch (key.m_nDifficulty)
		{
			case kSubData_Easy:		strcpy(szDifficulty, "Easy");		break;
			case kSubData_Normal:	strcpy(szDifficulty, "Normal");		break;
			case kSubData_Hard:		strcpy(szDifficulty, "Hard");		break;
			case kSubData_VeryHard:	strcpy(szDifficulty, "VeryHard");	break;
			default:				strcpy(szDifficulty, "");			break;
		}

		sprintf(m_szFullSuffix, "%s%s%s", szVersion, szState, szDifficulty);
		sprintf(m_szSuffix, "%s%s", szState, szDifficulty);
	}

	char* GetAttribName(const char* szAttrib, char* szBuffer)
	{
		strcpy(szBuffer, szAttrib);
		strcat(szBuffer, m_szSuffix);
		return szBuffer;
	}

	char* GetFullAttribName(const char* szAttrib, char* szBuffer)
	{
		strcpy(szBuffer, szAttrib);
		strcat(szBuffer, m_szFullSuffix);
		return szBuffer;
	}

	void CheckSuccess(const char* szTag, const char* szAttrib)
	{
		if (!m_pButeMgr->Success())
		{
#ifdef _CLIENTBUILD
			g_pLTClient->CPrint(" Cannot find : %s in: %s\n", szTag, szAttrib);
#else
			g_pLTServer->CPrint(" Cannot find : %s in: %s\n", szTag, szAttrib);
#endif
		}
	}

	float GetFloat(const char* szAttrib)
	{
		char szAttribName[80];
		LTVector vValue = m_pButeMgr->GetVector(m_szTag, GetAttribName(szAttrib, szAttribName));
		CheckSuccess(m_szTag, szAttribName);
		return (&vValue.x)[m_Key.m_nVersion];
	}

	void GetString(const char* szAttrib, char* szResult, uint32 nResultLen)
	{
		char szAttribName[64];
		// RETAIL: strings are per version too, so they take the full suffix.
		m_pButeMgr->GetString(m_szTag, GetFullAttribName(szAttrib, szAttribName), szResult, nResultLen);
		CheckSuccess(m_szTag, szAttribName);
	}

protected:

	CButeMgr*			m_pButeMgr;
	char				m_szTag[80];
	char				m_szSuffix[64];
	char				m_szFullSuffix[64];
	SubroutineDataKey	m_Key;
};

typedef std::vector<int> SubDataAxis;
typedef std::map<CaselessString, uint16> SubNameIdMap;

class CSubroutineDataMgr
{
public:

	CSubroutineDataMgr()
	{
		m_pButeMgr = LTNULL;
	}

	virtual bool Init(CButeMgr* pButeMgr);

	int GetIndex(const SubroutineDataKey & key) const
	{
		int nIndex = (key.m_nVersion * m_nNumStates + key.m_nState) * m_nNumDifficulties + key.m_nDifficulty;
		if (nIndex < 0 || nIndex >= m_nNumKeys)
			nIndex = 0;
		return nIndex;
	}

	// RETAIL: the client weapons' reads of their subroutine's data (inlined
	// in CShell 0x10017e90.., names ours)
	SequencerData		GetSequencerData( SubroutineDataKey key ) const		{ return m_vecSequencer.at( GetIndex( key ) ); }
	ClusterData			GetClusterData( SubroutineDataKey key ) const		{ return m_vecCluster.at( GetIndex( key ) ); }
	SuffusionData		GetSuffusionData( SubroutineDataKey key ) const		{ return m_vecSuffusion.at( GetIndex( key ) ); }
	LOLData				GetLOLData( SubroutineDataKey key ) const			{ return m_vecLOL.at( GetIndex( key ) ); }
	BallLauncherData	GetBallLauncherData( SubroutineDataKey key ) const	{ return m_vecBallLauncher.at( GetIndex( key ) ); }
	DrunkenDimsData		GetDrunkenDimsData( SubroutineDataKey key ) const	{ return m_vecDrunkenDims.at( GetIndex( key ) ); }
	EnergyClawData		GetEnergyClawData( SubroutineDataKey key ) const	{ return m_vecEnergyClaw.at( GetIndex( key ) ); }
	PranksterBitData	GetPranksterBitData( SubroutineDataKey key ) const	{ return m_vecPranksterBit.at( GetIndex( key ) ); }

protected:

	char* FindTag(const char* szTagBase, const char* szAttrib, const char* szValue, char* szResult, uint32 nResultLen);

	template <class T>
	void LoadData(std::vector<T> & vecData, const char* szName)
	{
		vecData.reserve(m_nNumKeys);

		char szTag[128];
		FindTag("Subroutine", "Name", szName, szTag, sizeof(szTag));

		for (SubDataAxis::iterator itVersion = m_Versions.begin(); itVersion != m_Versions.end(); ++itVersion)
		{
			for (SubDataAxis::iterator itState = m_States.begin(); itState != m_States.end(); ++itState)
			{
				for (SubDataAxis::iterator itDiff = m_Difficulties.begin(); itDiff != m_Difficulties.end(); ++itDiff)
				{
					SubroutineDataKey key(*itVersion, *itState, *itDiff);
					T data;
					Read(szTag, &data, key);
					vecData.push_back(data);
				}
			}
		}
	}

	void Read(const char* szTag, SequencerData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, ClusterData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, SuffusionData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, LOLData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, BallLauncherData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, DrunkenDimsData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, EnergyClawData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, PranksterBitData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, FuzzySignatureData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, ShieldPrismData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, MegaHurtzData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, CorrodeData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, TriangulateData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, YAmpData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, PrimitiveChargeData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, ProfilerData* pData, const SubroutineDataKey & key);
	void Read(const char* szTag, VirusScanData* pData, const SubroutineDataKey & key);

public:

	CButeMgr*		m_pButeMgr;			// only valid during Init

	SubDataAxis		m_Versions;
	SubDataAxis		m_States;
	SubDataAxis		m_Difficulties;
	int				m_nNumVersions;
	int				m_nNumStates;
	int				m_nNumDifficulties;
	int				m_nNumKeys;

	std::vector<SequencerData>			m_vecSequencer;
	std::vector<ClusterData>			m_vecCluster;
	std::vector<SuffusionData>			m_vecSuffusion;
	std::vector<LOLData>				m_vecLOL;
	std::vector<BallLauncherData>		m_vecBallLauncher;
	std::vector<DrunkenDimsData>		m_vecDrunkenDims;
	std::vector<EnergyClawData>			m_vecEnergyClaw;
	std::vector<PranksterBitData>		m_vecPranksterBit;
	std::vector<FuzzySignatureData>		m_vecFuzzySignature;
	std::vector<ShieldPrismData>		m_vecShieldPrism;
	std::vector<MegaHurtzData>			m_vecMegaHurtz;
	std::vector<CorrodeData>			m_vecCorrode;
	std::vector<TriangulateData>		m_vecTriangulate;
	std::vector<YAmpData>				m_vecYAmp;
	std::vector<PrimitiveChargeData>	m_vecPrimitiveCharge;
	std::vector<ProfilerData>			m_vecProfiler;
	std::vector<VirusScanData>			m_vecVirusScan;

	SubNameIdMap	m_mapNameIds;		// subroutine Name -> NameId
};

#endif // __SUBROUTINE_DATA_MGR_H__
