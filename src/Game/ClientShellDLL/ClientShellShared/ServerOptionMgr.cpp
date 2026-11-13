// ----------------------------------------------------------------------- //
//
// MODULE  : ServerOptionMgr.cpp
//
// PURPOSE : Tron's sets of saved host options (ServerData\<name>.txt)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerOptionMgr.h"
#include "ProfileMgr.h"
#include "WinUtil.h"
#include <io.h>

// RETAIL: the folder and extension of the option files (0x10270528,
// 0x10270518; initialised at 0x100df710 and 0x100df810)
static std::string s_sServerDataDir = "ServerData\\";
static std::string s_sServerDataExt = ".txt";

CServerOptionMgr* g_pServerOptionMgr = LTNULL;

// RETAIL: string 1500, the default set's name
#define IDS_SERVEROPTIONS_DEFAULT	1500
// RETAIL: strings 1501 (the session name) and 1502 (the passwords) of a new set
#define IDS_SERVEROPTIONS_SESSION	1501
#define IDS_SERVEROPTIONS_PASSWORD	1502

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::Init
//
//	PURPOSE:	Read the option sets and select the profile's (RETAIL:
//				0x100df900)
//
// ----------------------------------------------------------------------- //

bool CServerOptionMgr::Init()
{
	g_pServerOptionMgr = this;

	if (!CWinUtil::DirExist("ServerData"))
	{
		CWinUtil::CreateDir("ServerData");
	}

	m_nCurrent = (uint16)-1;
	LoadAll();

	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	HSTRING hDefault = g_pLTClient->FormatString(IDS_SERVEROPTIONS_DEFAULT);

	const char* pszName;
	if (!pProfile->m_sServerOptions.empty())
		pszName = pProfile->m_sServerOptions.c_str();
	else
		pszName = g_pLTClient->GetStringData(hDefault);

	bool bFailed = !Select(std::string(pszName), false);
	if (bFailed)
	{
		Select(std::string(g_pLTClient->GetStringData(hDefault)), true);
	}

	g_pLTClient->FreeString(hDefault);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::Term
//
//	PURPOSE:	Free the option sets (RETAIL: 0x100dfad0)
//
// ----------------------------------------------------------------------- //

void CServerOptionMgr::Term()
{
	for (uint16 i = 0; i < m_Options.size(); i++)
	{
		debug_delete(m_Options[i]);
	}
	m_Options.clear();

	g_pServerOptionMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::Select
//
//	PURPOSE:	Make the named set current, adding it if bCreate (RETAIL:
//				0x100dfb40)
//
// ----------------------------------------------------------------------- //

bool CServerOptionMgr::Select(const std::string& sName, bool bCreate)
{
	if (sName.empty())
		return false;

	uint16 nIndex;
	for (nIndex = 0; nIndex < m_Options.size(); nIndex++)
	{
		if (stricmp(m_Options[nIndex]->m_sOptionsName.c_str(), sName.c_str()) == 0)
			break;
	}

	if (nIndex >= m_Options.size())
	{
		if (bCreate)
		{
			Add(sName);
			nIndex = m_Options.size() - 1;
		}
		else
		{
			return false;
		}
	}

	SetCurrent(nIndex);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::SetCurrent
//
//	PURPOSE:	Save the current set and make another current; the profile
//				keeps its name (RETAIL: 0x100dfbe0)
//
// ----------------------------------------------------------------------- //

void CServerOptionMgr::SetCurrent(uint16 nIndex)
{
	SaveCurrent();

	if (nIndex >= m_Options.size())
		return;

	m_nCurrent = nIndex;

	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_sServerOptions = GetCurrent()->m_sOptionsName;
	pProfile->Save();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::SaveCurrent
//
//	PURPOSE:	Write the current set to its file (RETAIL: 0x100dfc90)
//
// ----------------------------------------------------------------------- //

void CServerOptionMgr::SaveCurrent()
{
	if (m_nCurrent >= m_Options.size())
		return;

	ServerGameOptions* pOptions = GetCurrent();
	pOptions->SaveToBute(m_buteMgr);

	std::string sFile = s_sServerDataDir + pOptions->m_sOptionsName + s_sServerDataExt;
	m_buteMgr.Save(sFile.c_str());
	m_buteMgr.Term();
	m_buteMgr.Init();
	m_buteMgr.Parse(sFile.c_str());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::LoadAll
//
//	PURPOSE:	Read every set in ServerData (a default one when there is
//				none) (RETAIL: 0x100dfdd0)
//
// ----------------------------------------------------------------------- //

void CServerOptionMgr::LoadAll()
{
	for (uint32 i = 0; i < m_Options.size(); i++)
	{
		debug_delete(m_Options[i]);
	}
	m_Options.erase(m_Options.begin(), m_Options.end());

	std::string sFind = s_sServerDataDir + "*" + s_sServerDataExt;

	struct _finddata_t file;
	long hFile = _findfirst(sFind.c_str(), &file);
	if (hFile != -1)
	{
		do
		{
			std::string sFile = s_sServerDataDir + file.name;

			m_buteMgr.Term();
			m_buteMgr.Init();
			if (m_buteMgr.Parse(sFile.c_str()))
			{
				ServerGameOptions* pOptions = debug_new(ServerGameOptions);

				char* pszName = strtok(file.name, ".");
				pOptions->m_sOptionsName.assign(pszName, pszName + strlen(pszName));
				pOptions->LoadFromBute(m_buteMgr);

				m_Options.push_back(pOptions);
			}
		}
		while (_findnext(hFile, &file) == 0);
	}

	_findclose(hFile);

	if (m_Options.size() == 0)
	{
		HSTRING hDefault = g_pLTClient->FormatString(IDS_SERVEROPTIONS_DEFAULT);
		Select(std::string(g_pLTClient->GetStringData(hDefault)), true);
		g_pLTClient->FreeString(hDefault);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::Add
//
//	PURPOSE:	A new set with the default session name and passwords
//				(RETAIL: 0x100e00b0)
//
// ----------------------------------------------------------------------- //

ServerGameOptions* CServerOptionMgr::Add(const std::string& sName)
{
	ServerGameOptions* pOptions = debug_new(ServerGameOptions);

	pOptions->m_sOptionsName = sName;
	pOptions->m_sSessionName = LoadTempString(IDS_SERVEROPTIONS_SESSION);
	pOptions->m_sPassword = LoadTempString(IDS_SERVEROPTIONS_PASSWORD);
	pOptions->m_sScmdPassword = LoadTempString(IDS_SERVEROPTIONS_PASSWORD);

	m_Options.push_back(pOptions);

	pOptions->SaveToBute(m_buteMgr);

	return pOptions;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::Delete
//
//	PURPOSE:	Remove a set and its file; select another if it was current
//				(RETAIL: 0x100e02a0)
//
// ----------------------------------------------------------------------- //

void CServerOptionMgr::Delete(std::string sName)
{
	bool bFound = false;
	std::vector<ServerGameOptions*>::iterator iter = m_Options.begin();
	while (iter != m_Options.end() && !bFound)
	{
		bFound = (stricmp((*iter)->m_sOptionsName.c_str(), sName.c_str()) == 0);
		if (!bFound)
			iter++;
	}

	if (!bFound)
		return;

	ServerGameOptions* pCurrent = GetCurrent();
	bool bCurrent = (*iter == pCurrent);
	std::string sCurrent = pCurrent->m_sOptionsName;

	ServerGameOptions* pOptions = *iter;
	m_Options.erase(iter);
	debug_delete(pOptions);

	std::string sFile = s_sServerDataDir + sName + s_sServerDataExt;
	remove(sFile.c_str());

	if (bCurrent)
	{
		if (m_Options.size() > 0)
		{
			SetCurrent(0);
		}
		else
		{
			HSTRING hDefault = g_pLTClient->FormatString(IDS_SERVEROPTIONS_DEFAULT);
			Select(std::string(g_pLTClient->GetStringData(hDefault)), true);
			g_pLTClient->FreeString(hDefault);
		}
	}
	else
	{
		Select(sCurrent, true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::Rename
//
//	PURPOSE:	Rename a set and its file (RETAIL: 0x100e04f0)
//
// ----------------------------------------------------------------------- //

void CServerOptionMgr::Rename(const std::string& sOldName, const std::string& sNewName)
{
	std::string sOldFile = s_sServerDataDir + sOldName + s_sServerDataExt;
	std::string sNewFile = s_sServerDataDir + sNewName + s_sServerDataExt;
	rename(sOldFile.c_str(), sNewFile.c_str());

	uint16 nIndex;
	for (nIndex = 0; nIndex < m_Options.size(); nIndex++)
	{
		if (stricmp(m_Options[nIndex]->m_sOptionsName.c_str(), sOldName.c_str()) == 0)
			break;
	}

	if (nIndex < m_Options.size())
	{
		m_Options[nIndex]->m_sOptionsName = sNewName;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::GetCurrent
//
//	PURPOSE:	The current set, or NULL (RETAIL: 0x10047270)
//
// ----------------------------------------------------------------------- //

ServerGameOptions* CServerOptionMgr::GetCurrent()
{
	if (m_nCurrent >= m_Options.size())
		return LTNULL;

	return m_Options[m_nCurrent];
}
