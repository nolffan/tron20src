// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleGameDataMgr.h
//
// PURPOSE : Tron's sets of saved single player light cycle game settings
//			 (LightCycleGameData\<name>.txt)
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHT_CYCLE_GAME_DATA_MGR_H__
#define __LIGHT_CYCLE_GAME_DATA_MGR_H__

// RETAIL: Tron's (no RTTI: the class, file and member names are ours, the
// folder and the bute keys are retail's): CShell.dll TU 0x10046e30..
// 0x10048e50, CTronInterfaceMgr's +0x279c4 (0x111c bytes),
// g_pLightCycleGameDataMgr (0x1026dd34). The twin of CServerOptionMgr: it
// keeps the named sets of settings of the single player light cycle games
// (a custom game's maps and options, the circuit played), each in
// LightCycleGameData\<name>.txt.

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

	std::string					m_sName;		// RETAIL: +0x00 the file's name
	LightCycleCustomSettings	m_Custom;		// RETAIL: +0x0c CustomSettings
	Circuit						m_Circuit;		// RETAIL: +0x30 CircuitSettings (its Map: the first string)
	// RETAIL: +0xa0, +0xa1: the profile's cycle style and shade, copied as
	// bytes by the circuit screen's launch (0x100669a0)
	uint8						m_bTronUnknownA0;	// RETAIL: +0xa0
	uint8						m_bTronUnknownA1;	// RETAIL: +0xa1
};

class CLightCycleGameDataMgr
{
public:
	bool		Init();										// RETAIL: 0x10047030
	void		Term();										// RETAIL: 0x10047200

	// RETAIL: 0x10047270 (folded with CServerOptionMgr::GetCurrent)
	CLightCycleGameData* GetCurrent();

	// make the named set current, adding it if asked. RETAIL: 0x10047290
	bool		Select(const std::string& sName, bool bCreate);
	void		SetCurrent(uint16 nIndex);					// RETAIL: 0x10047330
	void		SaveCurrent();								// RETAIL: 0x10047400
	void		LoadAll();									// RETAIL: 0x10047540
	CLightCycleGameData* Add(const std::string& sName);		// RETAIL: 0x10047820
	// the names of the sets. RETAIL: 0x100478e0
	void		GetNames(StringList& lstNames);
	void		Delete(std::string sName);					// RETAIL: 0x10047970
	void		Rename(const std::string& sOldName, const std::string& sNewName);	// RETAIL: 0x10047bc0

	uint32		GetNumData() { return m_Data.size(); }
	CLightCycleGameData* GetData(uint32 nIndex) { return m_Data[nIndex]; }

private:
	std::vector<CLightCycleGameData*>	m_Data;		// RETAIL: +0x00
	uint16		m_nCurrent;							// RETAIL: +0x0c (-1 until selected)
	CButeMgr	m_buteMgr;							// RETAIL: +0x10 the file being read or written
};

extern CLightCycleGameDataMgr* g_pLightCycleGameDataMgr;

#endif // __LIGHT_CYCLE_GAME_DATA_MGR_H__
