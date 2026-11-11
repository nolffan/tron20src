// ----------------------------------------------------------------------- //
//
// MODULE  : MultiplayerLevelInfoMgr.h
//
// PURPOSE : A multiplayer level's info: system memory, the subroutines a
//			 player starts with or may use, default primitives and
//			 weapons, the team size
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1002e870..0x1002f550, vtable 0x1029412c). The class name
// and its bute tags are retail's; the module, members and functions are
// ours.
//
// ----------------------------------------------------------------------- //

#ifndef __MULTIPLAYERLEVELINFOMGR_H__
#define __MULTIPLAYERLEVELINFOMGR_H__

#include "GameButeMgr.h"
#include <vector>
#include <string>

class CMultiplayerLevelInfoMgr;
extern CMultiplayerLevelInfoMgr* g_pMultiplayerLevelInfoMgr;

// A subroutine of the level: its string id and its version
struct MPLevelSubroutine
{
	int		nNameId;
	uint8	nVersion;	// alpha 0, beta 1, gold 2
};

typedef std::vector<MPLevelSubroutine> MPLevelSubroutineList;
typedef std::vector<std::string> MPLevelStringList;

class CMultiplayerLevelInfoMgr : public CGameButeMgr
{
	public :

		CMultiplayerLevelInfoMgr();
		~CMultiplayerLevelInfoMgr() {}

		LTBOOL	Init(const char* szAttributeFile);
		void	Term();

	protected :

		bool	ReadSubroutines();
		bool	ParseSubroutine(const CString& sValue, MPLevelSubroutine& sub);
		bool	ReadDefaults();
		bool	ReadMaxTeamSize();

		std::string				m_sSystemMemory;
		MPLevelSubroutineList	m_lstDefaultSubs;
		MPLevelSubroutineList	m_lstAvailableSubs;	// With the random ones picked
		MPLevelStringList		m_lstDefaultPrims;
		MPLevelStringList		m_lstDefaultWeaps;
		int						m_nMaxTeamSize;
};

#endif // __MULTIPLAYERLEVELINFOMGR_H__
