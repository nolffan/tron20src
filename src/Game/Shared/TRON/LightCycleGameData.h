// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleGameData.h
//
// PURPOSE : One set of single player light cycle game settings
//			 (LightCycleGameData\<name>.txt)
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHT_CYCLE_GAME_DATA_H__
#define __LIGHT_CYCLE_GAME_DATA_H__

// RETAIL: Tron's (no RTTI: the class, file and member names are ours, the
// bute keys are retail's). CShell.dll 0x100486b0..0x10048e5a (after
// CLightCycleGameDataMgr's TU, which reads and writes the sets) and
// Object.lto 0x10029c80..0x10029ec1: the client gives the server the
// current set (ServerGameOptions::GetLightCycleGameData), and the server's
// light cycle mission manager keeps one (CLightCycleMultiplayerMissionMgr
// +0x48).

#include "ButeMgr.h"
#include "CircuitButeMgr.h"
#include "CommonUtilities.h"
#include <vector>
#include <string>

// The custom game's settings (0x24 bytes; the names are ours, the keys of
// CustomSettings are retail's)
struct LightCycleCustomSettings
{
	void	Reset();							// RETAIL: 0x100486b0

	StringArray	aMaps;							// RETAIL: +0x00 CustomMaps Map%02d
	uint8		nMaxAICyclists;					// RETAIL: +0x0c MaxAICyclists
	int			nOpponentSkill;					// RETAIL: +0x10 OpponentSkill
	int			nOpponentCycleType;				// RETAIL: +0x14 OpponentCycleType
	int			nGameSpeed;						// RETAIL: +0x18 GameSpeed
	int			nTrailLength;					// RETAIL: +0x1c TrailLength
	// RETAIL: +0x20 the allowed power-ups, a bit field (PowerupAllow*)
	union
	{
		uint32	nPowerups;
		struct
		{
			uint32	bShield : 1;
			uint32	bTurbo : 1;
			uint32	bTrailSpike : 1;
			uint32	bMissile : 1;
			uint32	bWallExtender : 1;
			uint32	bTurboCurse : 1;
			uint32	bGrab : 1;
			uint32	bTrailReset : 1;
			uint32	bForceUse : 1;
		};
	};
};

// One set of settings (0xa4 bytes). RETAIL: constructor 0x100486e0,
// destructor 0x100482e0 (implicit)
class CLightCycleGameData
{
public:
	CLightCycleGameData();

	void	Reset();							// RETAIL: 0x10048760
	// RETAIL: 0x10048780: Save and Load call it and ignore the result
	bool	IsValid();
	void	Save(CButeMgr& buteMgr);			// RETAIL: 0x10048790
	void	Load(CButeMgr& buteMgr);			// RETAIL: 0x10048990
	// The maps of the game type (the custom game's; none for the others).
	// RETAIL: 0x10048e00
	StringArray* GetMaps(int nGameType);
	// The circuit's waves (the names are ours). RETAIL: Object.lto
	// 0x10029e30, 0x10029eb0 (the server's light cycle circuit; CShell.dll
	// has no calls to them)
	const CircuitWave& GetWave(uint8 nWave);
	uint8	GetNumWaves();

	std::string					m_sName;		// RETAIL: +0x00 the file's name
	LightCycleCustomSettings	m_Custom;		// RETAIL: +0x0c CustomSettings
	Circuit						m_Circuit;		// RETAIL: +0x30 CircuitSettings (its Map: the first string)
	// RETAIL: +0xa0, +0xa1: the profile's cycle style and shade, copied as
	// bytes by the circuit screen's launch (0x100669a0)
	uint8						m_bTronUnknownA0;	// RETAIL: +0xa0
	uint8						m_bTronUnknownA1;	// RETAIL: +0xa1
};

#endif // __LIGHT_CYCLE_GAME_DATA_H__
