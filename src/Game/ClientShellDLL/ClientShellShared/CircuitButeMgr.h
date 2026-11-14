// ----------------------------------------------------------------------- //
//
// MODULE  : CircuitButeMgr.h
//
// PURPOSE : The light cycle circuits of the single player light cycle game
//
// ----------------------------------------------------------------------- //

#ifndef __CIRCUIT_BUTE_MGR_H__
#define __CIRCUIT_BUTE_MGR_H__

// RETAIL: Tron 2.0's, not in any source release; written from retail
// CShell.dll (TU 0x1000b8e0..0x1000c59a, vtable 0x101ea294; the class name
// is from RTTI, the file name and the names of the records and members are
// ours, the attribute names are retail's). CGameClientShell owns it (retail
// +0x8340, created in OnEngineInitialized 0x100e270f by an inline
// constructor; 0x1120 bytes).

#include "GameButeMgr.h"
#include <list>
#include <string>
#include <vector>

#include "TRON/CaselessString.h"

// One wave of a circuit: the string ids of its speed scalar and trail length
// (0x8 bytes). RETAIL: Load 0x1000ba90
struct CircuitWave
{
	// RETAIL: out of line and empty (folded into 0x100c42f0)
	CircuitWave();

	bool Load(CButeMgr& buteMgr, const char* pszTag, uint8 nWave);

	int		nSpeedScalarId;		// RETAIL: +0x00 SpeedScalarId<n>
	int		nTrailLengthId;		// RETAIL: +0x04 TrailLengthId<n>
};

// RETAIL: the circuit's sound name is a string whose length checks for
// NULL inline (0x1000bb93, 0x1000bd46: unlike std::string's and
// CaselessString's). Ours
struct CircuitCharTraits : public std::char_traits<char>
{
	static size_t length(const char* s)
	{
		if (!s) return 0;
		return strlen(s);
	}
};

typedef std::basic_string<char, CircuitCharTraits> CircuitString;

typedef std::vector<CircuitWave> CircuitWaveArray;
typedef std::vector<std::string> CircuitStringArray;

// One circuit, the tag Circuit<n> (0x70 bytes). RETAIL: constructor
// 0x1000bb10, Load 0x1000bc40, destructor 0x1000c180 (implicit)
struct Circuit
{
	Circuit();

	bool Load(CButeMgr& buteMgr, int nCircuit);

	std::string			sNameKey;			// RETAIL: +0x00 NameKey
	std::string			sImage;				// RETAIL: +0x0c Image
	int					nNameId;			// RETAIL: +0x18 CircuitNameId
	int					nBriefingId;		// RETAIL: +0x1c BriefingId
	int					nSuperCycleRestrictionId;		// RETAIL: +0x20
	int					nStandardCycleRestrictionId;	// RETAIL: +0x24
	int					nCompleteId;		// RETAIL: +0x28 CircuitCompleteId
	CircuitString		sCompleteSound;		// RETAIL: +0x2c CircuitCompleteSound
	CircuitStringArray	aUnlocks;			// RETAIL: +0x38 Unlocks<n>
	std::string			sMapName;			// RETAIL: +0x44 MapName
	uint8				bInitiallyLocked;	// RETAIL: +0x50 InitiallyLock
	int					nInitialLives;		// RETAIL: +0x54 InitialLives
	CircuitStringArray	aCyclesAllowed;		// RETAIL: +0x58 CyclesAllowed<n>
	CircuitWaveArray	aWaves;				// RETAIL: +0x64 WaveCount, the waves
};

typedef std::list<Circuit> CircuitList;

class CircuitButeMgr : public CGameButeMgr
{
	public :

		CircuitButeMgr() {}
		virtual ~CircuitButeMgr() {}

		virtual LTBOOL Init(const char* szAttributeFile);		// RETAIL: 0x1000bef0

		// Is the map one of the circuits' (its name equal to or containing
		// it, without case)? (the name is ours) RETAIL: 0x1000c2c0
		bool IsCircuitMap(const std::string& sMap) const;

		int GetNumCircuits() const;						// RETAIL: 0x1000c4c0 (int: its callers use all of eax)
		// A blank circuit when out of range. RETAIL: 0x1000c4e0
		const Circuit& GetCircuit(uint8 nCircuit) const;

	protected :

		CircuitList		m_lstCircuits;		// RETAIL: +0x111c
};

#endif // __CIRCUIT_BUTE_MGR_H__
