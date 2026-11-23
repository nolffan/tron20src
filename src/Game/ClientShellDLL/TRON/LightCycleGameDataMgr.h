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
// 0x100486a0 (the sets, CLightCycleGameData, are Shared/TRON/
// LightCycleGameData.h), CTronInterfaceMgr's +0x279c4 (0x111c bytes),
// g_pLightCycleGameDataMgr (0x1026dd34). The twin of CServerOptionMgr: it
// keeps the named sets of settings of the single player light cycle games
// (a custom game's maps and options, the circuit played), each in
// LightCycleGameData\<name>.txt.

#include "ButeMgr.h"
#include "LightCycleGameData.h"
#include "CommonUtilities.h"
#include <vector>
#include <string>

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
