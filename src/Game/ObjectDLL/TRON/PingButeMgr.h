// ----------------------------------------------------------------------- //
//
// MODULE  : PingButeMgr.h
//
// PURPOSE : The pings: each a name and the character it is for
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10033890..0x10033b70, vtable 0x1029434c). The class name
// and its bute tags are retail's; the module, members and functions are
// ours.
//
// ----------------------------------------------------------------------- //

#ifndef __PINGBUTEMGR_H__
#define __PINGBUTEMGR_H__

#include "GameButeMgr.h"

class CPingButeMgr;
extern CPingButeMgr* g_pPingButeMgr;

class CPingButeMgr : public CGameButeMgr
{
	public :

		CPingButeMgr();
		~CPingButeMgr();

		LTBOOL	Init(const char* szAttributeFile);
		void	Term();

		// The tag of the ping of that name (empty when there is none)
		void	GetPingTag(const char* szName, char* pBuf, int nBufLen);

		// The character of the ping of that tag
		void	GetCharacter(const char* szTag, char* pBuf, uint32 nBufLen);

	protected :

		int		m_nNumPings;
};

#endif // __PINGBUTEMGR_H__
