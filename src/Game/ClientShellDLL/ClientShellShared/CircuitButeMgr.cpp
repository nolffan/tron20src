// ----------------------------------------------------------------------- //
//
// MODULE  : CircuitButeMgr.cpp
//
// PURPOSE : The light cycle circuits of the single player light cycle game
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron 2.0's, written from retail CShell.dll (TU 0x1000b8e0..
// 0x1000c59a; see CircuitButeMgr.h)

#include "stdafx.h"
#include "CircuitButeMgr.h"
#include "ButeStrings.h"
#include <ctype.h>

// RETAIL: the map names are searched with a char_traits that also matches
// characters without case (toupper; 0x1000c2c0). Ours
struct CircuitMapTraits : public CaselessCharTraits
{
	static bool eq(char c1, char c2)
	{
		return toupper(c1) == toupper(c2);
	}
};

typedef std::basic_string<char, CircuitMapTraits> CircuitMapString;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CircuitWave::CircuitWave
//
//	PURPOSE:	Constructor (RETAIL: empty, folded into 0x100c42f0)
//
// ----------------------------------------------------------------------- //

CircuitWave::CircuitWave()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CircuitWave::Load
//
//	PURPOSE:	Read the wave's ids (RETAIL: 0x1000ba90)
//
// ----------------------------------------------------------------------- //

bool CircuitWave::Load(CButeMgr& buteMgr, const char* pszTag, uint8 nWave)
{
	char szAttr[48];

	sprintf(szAttr, "%s%d", "SpeedScalarId", nWave);
	nSpeedScalarId = buteMgr.GetInt(pszTag, szAttr, nSpeedScalarId);

	sprintf(szAttr, "%s%d", "TrailLengthId", nWave);
	nTrailLengthId = buteMgr.GetInt(pszTag, szAttr, nTrailLengthId);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Circuit::Circuit
//
//	PURPOSE:	Constructor (RETAIL: 0x1000bb10)
//
// ----------------------------------------------------------------------- //

Circuit::Circuit()
:	nNameId(0),
	nBriefingId(0),
	nSuperCycleRestrictionId(0),
	nStandardCycleRestrictionId(0),
	nCompleteId(0),
	sCompleteSound(""),
	bInitiallyLocked(0),
	nInitialLives(0)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Circuit::Load
//
//	PURPOSE:	Read the circuit Circuit<nCircuit> (RETAIL: 0x1000bc40)
//
// ----------------------------------------------------------------------- //

bool Circuit::Load(CButeMgr& buteMgr, int nCircuit)
{
	char szTag[40];
	sprintf(szTag, "Circuit%d", nCircuit);

	if (!buteMgr.Exist(szTag))
		return false;

	GetButeString(buteMgr, szTag, "NameKey", sNameKey);
	GetButeString(buteMgr, szTag, "Image", sImage);
	GetButeString(buteMgr, szTag, "MapName", sMapName);
	GetButeStringList(buteMgr, szTag, "Unlocks", aUnlocks);

	nNameId = buteMgr.GetInt(szTag, "CircuitNameId");
	nBriefingId = buteMgr.GetInt(szTag, "BriefingId");
	nSuperCycleRestrictionId = buteMgr.GetInt(szTag, "SuperCycleRestrictionId");
	nStandardCycleRestrictionId = buteMgr.GetInt(szTag, "StandardCycleRestrictionId");
	nCompleteId = buteMgr.GetInt(szTag, "CircuitCompleteId");
	sCompleteSound = buteMgr.GetString(szTag, "CircuitCompleteSound");
	bInitiallyLocked = buteMgr.GetByte(szTag, "InitiallyLock");
	nInitialLives = buteMgr.GetInt(szTag, "InitialLives");
	GetButeStringList(buteMgr, szTag, "CyclesAllowed", aCyclesAllowed);

	uint8 nWaves = (uint8)buteMgr.GetInt(szTag, "WaveCount", 0);
	aWaves.reserve(nWaves);
	for (int nWave = 0; nWave < nWaves; nWave++)
	{
		CircuitWave wave;
		wave.Load(buteMgr, szTag, nWave);
		aWaves.push_back(wave);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CircuitButeMgr::Init
//
//	PURPOSE:	Read the circuits (RETAIL: 0x1000bef0)
//
// ----------------------------------------------------------------------- //

LTBOOL CircuitButeMgr::Init(const char* szAttributeFile)
{
	if (!szAttributeFile || !Parse(szAttributeFile))
		return LTFALSE;

	int nCircuit = 0;
	while (true)
	{
		Circuit circuit;
		if (!circuit.Load(m_buteMgr, nCircuit))
			break;

		m_lstCircuits.push_back(circuit);
		nCircuit++;
	}

	m_buteMgr.Term();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CircuitButeMgr::IsCircuitMap
//
//	PURPOSE:	Is the map a circuit's? (RETAIL: 0x1000c2c0)
//
// ----------------------------------------------------------------------- //

bool CircuitButeMgr::IsCircuitMap(const std::string& sMap) const
{
	const CircuitMapString& sFind = (const CircuitMapString&)sMap;

	for (CircuitList::const_iterator iter = m_lstCircuits.begin(); iter != m_lstCircuits.end(); ++iter)
	{
		const CircuitMapString& sMapName = (const CircuitMapString&)(*iter).sMapName;

		if (sMapName == sFind)
			return true;

		if (sMapName.find(sFind) != CircuitMapString::npos)
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CircuitButeMgr::GetNumCircuits
//
//	PURPOSE:	The number of circuits (RETAIL: 0x1000c4c0)
//
// ----------------------------------------------------------------------- //

uint8 CircuitButeMgr::GetNumCircuits() const
{
	return m_lstCircuits.size();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CircuitButeMgr::GetCircuit
//
//	PURPOSE:	A circuit (RETAIL: 0x1000c4e0)
//
// ----------------------------------------------------------------------- //

const Circuit& CircuitButeMgr::GetCircuit(uint8 nCircuit) const
{
	if (nCircuit >= (uint8)m_lstCircuits.size())
	{
		// RETAIL: a function static (0x10236ca8)
		static Circuit s_BlankCircuit;
		return s_BlankCircuit;
	}

	CircuitList::const_iterator iter = m_lstCircuits.begin();
	std::advance(iter, nCircuit);
	return *iter;
}
