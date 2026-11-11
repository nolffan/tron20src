// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSubroutineMgr.cpp
//
// PURPOSE : The server's list of subroutines, procedurals and primitives
//
// RETAIL: Tron 2.0's, written from retail Object.lto
// (ServerSubroutineMgr.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerSubroutineMgr.h"

#define SSMGR_SUBROUTINE_TAG		"Subroutine"
#define SSMGR_PROCEDURAL_TAG		"Procedural"
#define SSMGR_PRIMITIVE_TAG			"Primitive"

#define SSMGR_NAME					"Name"
#define SSMGR_NAMEID				"NameId"
#define SSMGR_FUNCTION				"Function"

namespace
{
	char s_aTagName[100];
}

CServerSubroutineMgr* g_pServerSubroutineMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSubroutineMgr::Init()
//
//	PURPOSE:	Init mgr: read the lists, then drop the bute file
//
// ----------------------------------------------------------------------- //

LTBOOL CServerSubroutineMgr::Init(const char* szAttributeFile)
{
	if (g_pServerSubroutineMgr || !szAttributeFile) return LTFALSE;

	if (!Parse(szAttributeFile))
	{
		// RETAIL: formatted, but never shown
		char szError[512];
		_snprintf(szError, sizeof(szError), "CServerSubroutineMgr::Init: Failed to parse %s", szAttributeFile);
		return LTFALSE;
	}

	g_pServerSubroutineMgr = this;

	ServerSubroutineData data;

	int i = 0;
	_snprintf(s_aTagName, sizeof(s_aTagName), "%s%d", SSMGR_SUBROUTINE_TAG, i);
	s_aTagName[sizeof(s_aTagName) - 1] = '\0';
	while (m_buteMgr.Exist(s_aTagName))
	{
		const char* pszName = m_buteMgr.GetString(s_aTagName, SSMGR_NAME, "");
		strncpy(data.szName, pszName, sizeof(data.szName) - 1);
		data.szName[sizeof(data.szName) - 1] = '\0';
		data.nNameId = m_buteMgr.GetInt(s_aTagName, SSMGR_NAMEID);
		data.bFunction = (m_buteMgr.GetInt(s_aTagName, SSMGR_FUNCTION) == 2);
		m_lstSubroutines.push_back(data);

		_snprintf(s_aTagName, sizeof(s_aTagName), "%s%d", SSMGR_SUBROUTINE_TAG, ++i);
		s_aTagName[sizeof(s_aTagName) - 1] = '\0';
	}

	i = 0;
	_snprintf(s_aTagName, sizeof(s_aTagName), "%s%d", SSMGR_PROCEDURAL_TAG, i);
	s_aTagName[sizeof(s_aTagName) - 1] = '\0';
	while (m_buteMgr.Exist(s_aTagName))
	{
		const char* pszName = m_buteMgr.GetString(s_aTagName, SSMGR_NAME, "");
		strncpy(data.szName, pszName, sizeof(data.szName) - 1);
		data.szName[sizeof(data.szName) - 1] = '\0';
		data.nNameId = m_buteMgr.GetInt(s_aTagName, SSMGR_NAMEID);
		m_lstProcedurals.push_back(data);

		_snprintf(s_aTagName, sizeof(s_aTagName), "%s%d", SSMGR_PROCEDURAL_TAG, ++i);
		s_aTagName[sizeof(s_aTagName) - 1] = '\0';
	}

	i = 0;
	_snprintf(s_aTagName, sizeof(s_aTagName), "%s%d", SSMGR_PRIMITIVE_TAG, i);
	s_aTagName[sizeof(s_aTagName) - 1] = '\0';
	while (m_buteMgr.Exist(s_aTagName))
	{
		const char* pszName = m_buteMgr.GetString(s_aTagName, SSMGR_NAME, "");
		strncpy(data.szName, pszName, sizeof(data.szName) - 1);
		data.szName[sizeof(data.szName) - 1] = '\0';
		data.nNameId = m_buteMgr.GetInt(s_aTagName, SSMGR_NAMEID);
		m_lstPrimitives.push_back(data);

		_snprintf(s_aTagName, sizeof(s_aTagName), "%s%d", SSMGR_PRIMITIVE_TAG, ++i);
		s_aTagName[sizeof(s_aTagName) - 1] = '\0';
	}

	m_buteMgr.Term();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSubroutineMgr::GetNameId()
//
//	PURPOSE:	The string id of the one of that name
//
// ----------------------------------------------------------------------- //

int CServerSubroutineMgr::GetNameId(const char* szName, int nType)
{
	int nNameId = 0;

	if (nType == kSST_Subroutine)
	{
		uint16 nCount = m_lstSubroutines.size();
		for (int i = 0; i < nCount; i++)
		{
			if (!_strnicmp(szName, m_lstSubroutines[i].szName, sizeof(m_lstSubroutines[i].szName)))
			{
				nNameId = m_lstSubroutines[i].nNameId;
				break;
			}
		}
	}
	else if (nType == kSST_Procedural)
	{
		uint16 nCount = m_lstProcedurals.size();
		for (int i = 0; i < nCount; i++)
		{
			if (!_strnicmp(szName, m_lstProcedurals[i].szName, sizeof(m_lstProcedurals[i].szName)))
			{
				nNameId = m_lstProcedurals[i].nNameId;
				break;
			}
		}
	}
	else if (nType == kSST_Primitive)
	{
		uint16 nCount = m_lstPrimitives.size();
		for (int i = 0; i < nCount; i++)
		{
			if (!_strnicmp(szName, m_lstPrimitives[i].szName, sizeof(m_lstPrimitives[i].szName)))
			{
				nNameId = m_lstPrimitives[i].nNameId;
				break;
			}
		}
	}

	return nNameId;
}
