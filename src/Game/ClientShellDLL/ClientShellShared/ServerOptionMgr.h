// ----------------------------------------------------------------------- //
//
// MODULE  : ServerOptionMgr.h
//
// PURPOSE : Tron's sets of saved host options (ServerData\<name>.txt)
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_OPTION_MGR_H__
#define __SERVER_OPTION_MGR_H__

#include "ProfileUtils.h"
#include "ButeMgr.h"
#include <vector>
#include <string>

// RETAIL: Tron's (no RTTI, no strings naming it: the class and member names
// are ours): CShell.dll 0x100df710 to 0x100e0640, 0x111c bytes,
// CTronInterfaceMgr's +0x268a8, g_pServerOptionMgr (0x10270534). It keeps
// the named sets of host options, each in ServerData\<name>.txt, and the
// profile the name of the set in use.
class CServerOptionMgr
{
public:
	bool		Init();										// 0x100df900
	void		Term();										// 0x100dfad0

	// make the named set current, adding it if asked
	bool		Select(const std::string& sName, bool bCreate);	// 0x100dfb40
	void		SetCurrent(uint16 nIndex);					// 0x100dfbe0
	void		SaveCurrent();								// 0x100dfc90
	void		LoadAll();									// 0x100dfdd0
	ServerGameOptions* Add(const std::string& sName);		// 0x100e00b0
	void		Delete(std::string sName);					// 0x100e02a0
	void		Rename(const std::string& sOldName, const std::string& sNewName);	// 0x100e04f0

	// RETAIL: 0x10047270 (out of line; its twin's, folded with it)
	ServerGameOptions* GetCurrent();

	uint32		GetNumOptions() { return m_Options.size(); }
	ServerGameOptions* GetOptions(uint32 nIndex) { return m_Options[nIndex]; }

private:
	std::vector<ServerGameOptions*>	m_Options;		// +0x00
	uint16		m_nCurrent;							// +0x0c (-1 until selected)
	CButeMgr	m_buteMgr;							// +0x10 the file being read or written
};

extern CServerOptionMgr* g_pServerOptionMgr;

#endif // __SERVER_OPTION_MGR_H__
