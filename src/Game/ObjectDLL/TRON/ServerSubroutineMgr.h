// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSubroutineMgr.h
//
// PURPOSE : The server's list of subroutines, procedurals and primitives
//			 (their names and string ids)
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (Init and GetNameId at 0x100cec50..0x100cf170, vtable
// 0x10295950; its inline constructor and destructor are emitted at
// 0x10089180, 0x10089280). The class name and its bute tags are retail's;
// the module, members and functions are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERSUBROUTINEMGR_H__
#define __SERVERSUBROUTINEMGR_H__

#include "GameButeMgr.h"
#include <vector>

#define SSMGR_DEFAULT_FILE		"Attributes\\subroutines.txt"

class CServerSubroutineMgr;
extern CServerSubroutineMgr* g_pServerSubroutineMgr;
// RETAIL: the server's one (0x1035bbe0; its constructor runs in a TU of
// its own, 0x10089180..0x100893f0); the name is ours
extern CServerSubroutineMgr g_ServerSubroutineMgr;

// What GetNameId looks in
enum ServerSubroutineType
{
	kSST_Subroutine = 0,
	kSST_Procedural,
	kSST_Primitive,
};

struct ServerSubroutineData
{
	char	szName[64];
	bool	bFunction;		// Subroutines only: Function is 2
	int		nNameId;
};

typedef std::vector<ServerSubroutineData> ServerSubroutineList;

class CServerSubroutineMgr : public CGameButeMgr
{
	public :

		CServerSubroutineMgr() {}
		~CServerSubroutineMgr() { Term(); }

		LTBOOL	Init(const char* szAttributeFile = SSMGR_DEFAULT_FILE);

		// The string id of the subroutine, procedural or primitive of that
		// name (0 when there is none)
		int		GetNameId(const char* szName, int nType);

	protected :

		ServerSubroutineList	m_lstSubroutines;
		ServerSubroutineList	m_lstProcedurals;
		ServerSubroutineList	m_lstPrimitives;
};

#endif // __SERVERSUBROUTINEMGR_H__
