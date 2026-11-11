// ----------------------------------------------------------------------- //
//
// MODULE  : PingButeMgr.cpp
//
// PURPOSE : The pings: each a name and the character it is for
//
// RETAIL: Tron 2.0's, written from retail Object.lto (PingButeMgr.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PingButeMgr.h"

#define PBMGR_PING_TAG				"Ping"
#define PBMGR_PING_NAME				"Name"
#define PBMGR_PING_CHARACTER		"Character"

namespace
{
	char s_aName[100];
	char s_aTagName[100];
}

CPingButeMgr* g_pPingButeMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPingButeMgr::CPingButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPingButeMgr::CPingButeMgr()
{
	m_nNumPings = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPingButeMgr::~CPingButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPingButeMgr::~CPingButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPingButeMgr::Init()
//
//	PURPOSE:	Init mgr: count the pings
//
// ----------------------------------------------------------------------- //

LTBOOL CPingButeMgr::Init(const char* szAttributeFile)
{
	if (g_pPingButeMgr || !szAttributeFile) return LTFALSE;
	if (!Parse(szAttributeFile)) return LTFALSE;

	g_pPingButeMgr = this;

	sprintf(s_aTagName, "%s%d", PBMGR_PING_TAG, m_nNumPings);
	while (m_buteMgr.Exist(s_aTagName))
	{
		sprintf(s_aTagName, "%s%d", PBMGR_PING_TAG, ++m_nNumPings);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPingButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CPingButeMgr::Term()
{
	m_buteMgr.Term();
	g_pPingButeMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPingButeMgr::GetPingTag()
//
//	PURPOSE:	The tag of the ping of that name
//
// ----------------------------------------------------------------------- //

void CPingButeMgr::GetPingTag(const char* szName, char* pBuf, int nBufLen)
{
	for (int i = 0; i < m_nNumPings; i++)
	{
		sprintf(s_aTagName, "%s%d", PBMGR_PING_TAG, i);
		m_buteMgr.GetString(s_aTagName, PBMGR_PING_NAME, s_aName, 30);

		if (stricmp(szName, s_aName) == 0)
		{
			sprintf(pBuf, "%s%d", PBMGR_PING_TAG, i);
			pBuf[nBufLen - 1] = '\0';
			return;
		}
	}

	pBuf[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPingButeMgr::GetCharacter()
//
//	PURPOSE:	The character of the ping of that tag
//
// ----------------------------------------------------------------------- //

void CPingButeMgr::GetCharacter(const char* szTag, char* pBuf, uint32 nBufLen)
{
	m_buteMgr.GetString(szTag, PBMGR_PING_CHARACTER, pBuf, nBufLen);
}
