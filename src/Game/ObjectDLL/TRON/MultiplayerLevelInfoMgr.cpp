// ----------------------------------------------------------------------- //
//
// MODULE  : MultiplayerLevelInfoMgr.cpp
//
// PURPOSE : A multiplayer level's info
//
// RETAIL: Tron 2.0's, written from retail Object.lto
// (MultiplayerLevelInfoMgr.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MultiplayerLevelInfoMgr.h"
#include "ServerSubroutineMgr.h"
#include "ButeTools.h"
#include <malloc.h>

#define MLIMGR_TAG					"Level"
#define MLIMGR_SYSTEMMEMORY			"SystemMemory"
#define MLIMGR_DEFAULTSUB			"DefaultSub%d"
#define MLIMGR_AVAILABLESUB			"AvailableSub%d"
#define MLIMGR_RANDOMSUB			"RandomSub%d"
#define MLIMGR_NUMRANDOMSUBS		"NumRandomSubs"
#define MLIMGR_DEFAULTPRIM			"DefaultPrim%d"
#define MLIMGR_DEFAULTWEAP			"DefaultWeap%d"
#define MLIMGR_MAXTEAMSIZE			"MaxTeamSize"

CMultiplayerLevelInfoMgr* g_pMultiplayerLevelInfoMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerLevelInfoMgr::CMultiplayerLevelInfoMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMultiplayerLevelInfoMgr::CMultiplayerLevelInfoMgr()
{
	Term();

	g_pMultiplayerLevelInfoMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerLevelInfoMgr::Init()
//
//	PURPOSE:	Read a level's info
//
// ----------------------------------------------------------------------- //

LTBOOL CMultiplayerLevelInfoMgr::Init(const char* szAttributeFile)
{
	Term();

	if (!szAttributeFile) return LTFALSE;
	if (!Parse(szAttributeFile)) return LTFALSE;

	if (!ReadSubroutines())
	{
		Term();
		return LTFALSE;
	}

	if (!ReadDefaults())
	{
		Term();
		return LTFALSE;
	}

	if (!ReadMaxTeamSize())
	{
		Term();
		return LTFALSE;
	}

	m_buteMgr.Term();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerLevelInfoMgr::Term()
//
//	PURPOSE:	Forget the level's info
//
// ----------------------------------------------------------------------- //

void CMultiplayerLevelInfoMgr::Term()
{
	m_sSystemMemory = "";
	m_lstDefaultSubs.clear();
	m_lstAvailableSubs.clear();
	m_lstDefaultPrims.clear();
	m_lstDefaultWeaps.clear();
	m_nMaxTeamSize = 1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerLevelInfoMgr::ReadSubroutines()
//
//	PURPOSE:	The system memory, the default and available subroutines
//				and NumRandomSubs of the random ones
//
// ----------------------------------------------------------------------- //

bool CMultiplayerLevelInfoMgr::ReadSubroutines()
{
	CString sValue;
	MPLevelSubroutineList lstRandomSubs;
	char szTag[] = MLIMGR_TAG;

	char szSystemMemory[32];
	CButeTools::GetValidatedString(m_buteMgr, szTag, MLIMGR_SYSTEMMEMORY, szSystemMemory, sizeof(szSystemMemory));
	m_sSystemMemory = szSystemMemory;

	char szLabel[128];
	MPLevelSubroutine sub;

	int i = 0;
	for (;;)
	{
		sprintf(szLabel, MLIMGR_DEFAULTSUB, i++);
		if (!m_buteMgr.Exist(szTag, szLabel)) break;

		sValue = m_buteMgr.GetString(szTag, szLabel);
		if (!ParseSubroutine(sValue, sub)) return false;

		m_lstDefaultSubs.push_back(sub);
	}

	i = 0;
	for (;;)
	{
		sprintf(szLabel, MLIMGR_AVAILABLESUB, i++);
		if (!m_buteMgr.Exist(szTag, szLabel)) break;

		sValue = m_buteMgr.GetString(szTag, szLabel);
		if (!ParseSubroutine(sValue, sub)) return false;

		m_lstAvailableSubs.push_back(sub);
	}

	i = 0;
	for (;;)
	{
		sprintf(szLabel, MLIMGR_RANDOMSUB, i++);
		if (!m_buteMgr.Exist(szTag, szLabel)) break;

		sValue = m_buteMgr.GetString(szTag, szLabel);
		if (!ParseSubroutine(sValue, sub)) return false;

		lstRandomSubs.push_back(sub);
	}

	int nNumRandom = CButeTools::GetValidatedInt(m_buteMgr, szTag, MLIMGR_NUMRANDOMSUBS, 0);
	if (nNumRandom < 0) return false;

	int nNumRandomSubs = lstRandomSubs.size();
	if (nNumRandom > nNumRandomSubs) return false;

	if (nNumRandom)
	{
		// Shuffle the random ones and make the first available
		int* aIndices = (int*)alloca(nNumRandomSubs * sizeof(int));

		int j;
		for (j = 0; j < nNumRandomSubs; j++)
		{
			aIndices[j] = j;
		}

		for (j = 0; j < nNumRandomSubs; j++)
		{
			int k = rand() % nNumRandomSubs;
			int nTemp = aIndices[j];
			aIndices[j] = aIndices[k];
			aIndices[k] = nTemp;
		}

		for (j = 0; j < nNumRandom; j++)
		{
			m_lstAvailableSubs.push_back(lstRandomSubs[aIndices[j]]);
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerLevelInfoMgr::ParseSubroutine()
//
//	PURPOSE:	A subroutine: "<name> [alpha|beta|gold]"
//
// ----------------------------------------------------------------------- //

bool CMultiplayerLevelInfoMgr::ParseSubroutine(const CString& sValue, MPLevelSubroutine& sub)
{
	char szValue[128];
	if (sValue.GetLength() > sizeof(szValue) - 1) return false;

	strcpy(szValue, sValue);

	char* pName = strtok(szValue, " ");
	if (!pName) return false;

	int nNameId = g_pServerSubroutineMgr->GetNameId(pName, kSST_Subroutine);
	if (!nNameId) return false;

	char* pVersion = strtok(LTNULL, " ");
	if (pVersion)
	{
		if (stricmp(pVersion, "alpha") == 0)
		{
			sub.nVersion = 0;
		}
		else if (stricmp(pVersion, "beta") == 0)
		{
			sub.nVersion = 1;
		}
		else if (stricmp(pVersion, "gold") == 0)
		{
			sub.nVersion = 2;
		}
		else
		{
			return false;
		}
	}

	sub.nNameId = nNameId;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerLevelInfoMgr::ReadDefaults()
//
//	PURPOSE:	The default primitives and weapons
//
// ----------------------------------------------------------------------- //

bool CMultiplayerLevelInfoMgr::ReadDefaults()
{
	CString sValue;
	char szTag[] = MLIMGR_TAG;
	char szLabel[128];

	int i = 0;
	for (;;)
	{
		sprintf(szLabel, MLIMGR_DEFAULTPRIM, i++);
		if (!m_buteMgr.Exist(szTag, szLabel)) break;

		sValue = m_buteMgr.GetString(szTag, szLabel);
		m_lstDefaultPrims.push_back(std::string(sValue));
	}

	i = 0;
	for (;;)
	{
		sprintf(szLabel, MLIMGR_DEFAULTWEAP, i++);
		if (!m_buteMgr.Exist(szTag, szLabel)) break;

		sValue = m_buteMgr.GetString(szTag, szLabel);
		m_lstDefaultWeaps.push_back(std::string(sValue));
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerLevelInfoMgr::ReadMaxTeamSize()
//
//	PURPOSE:	The team size (1..8)
//
// ----------------------------------------------------------------------- //

bool CMultiplayerLevelInfoMgr::ReadMaxTeamSize()
{
	char szTag[] = MLIMGR_TAG;

	m_nMaxTeamSize = CButeTools::GetValidatedInt(m_buteMgr, szTag, MLIMGR_MAXTEAMSIZE, 1);
	if (m_nMaxTeamSize < 1 || m_nMaxTeamSize > 8) return false;

	return true;
}
