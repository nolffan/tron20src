// ----------------------------------------------------------------------- //
//
// MODULE  : TronLoadingScreen.cpp
//
// PURPOSE : Tron's loading screen
//
// ----------------------------------------------------------------------- //

// RETAIL: written from retail CShell.dll (0x100a25f0..0x100a2d18). The
// single player game uses the mission's strings (CLoadingScreen's); the
// light cycle and disc games read them from the world's layout, or use
// their default layout.

#include "stdafx.h"
#include "TronLoadingScreen.h"
#include "InterfaceMgr.h"
#include "MissionMgr.h"
#include "LayoutMgr.h"
#include "ScreenMgr.h"
#include "ScreenPostload.h"
#include "ClientUtilities.h"

// RETAIL: 0x100a25f0
bool CTronLoadingScreen::ReadWorldLayout()
{
	const char* pszWorld = g_pMissionMgr->GetNewWorldName();
	if (!pszWorld)
		pszWorld = g_pMissionMgr->GetCurrentWorldName();
	if (pszWorld)
	{
		char szTag[MAX_PATH] = "";
		_splitpath(pszWorld, NULL, NULL, szTag, NULL);
		if (g_pLayoutMgr->Exist(szTag))
		{
			m_layout = szTag;

			int nId = g_pLayoutMgr->GetInt(szTag, "NameId", 0);
			if (!nId)
				m_missionname = "";
			else
				m_missionname = LoadTempString(nId);

			nId = g_pLayoutMgr->GetInt(szTag, "LevelNameId", 0);
			if (!nId)
				m_levelname = "";
			else
				m_levelname = LoadTempString(nId);

			nId = g_pLayoutMgr->GetInt(szTag, "BriefingId", 0);
			if (!nId)
				m_briefing = "";
			else
				m_briefing = LoadTempString(nId);

			nId = g_pLayoutMgr->GetInt(szTag, "HelpId", 0);
			if (!nId)
				m_help = "";
			else
				m_help = LoadTempString(nId);

			return true;
		}
	}

	return false;
}

// RETAIL: 0x100a29a0
void CTronLoadingScreen::InitMissionInfo()
{
	switch (GetGameType())
	{
		case eGameTypeSingle:
			CLoadingScreen::InitMissionInfo();
			break;

		case eGameTypeSingleLightCycleCircuit:
		case eGameTypeSingleLightCycleCustom:
			if (!ReadWorldLayout())
			{
				m_layout = "LightCycleDefault";
				m_briefing = "";
				m_help = "";
				m_missionname = "";
				m_levelname = "";
			}
			break;

		case eGameTypeDiscArena:
			m_layout = "DiscArenaDefault";
			m_briefing = "";
			m_help = "";
			m_missionname = "";
			m_levelname = "";
			break;

		case eGameTypeDiscTournament:
			m_layout = "DiscTournamentDefault";
			m_briefing = "";
			m_help = "";
			m_missionname = "";
			m_levelname = "";
			break;

		case eGameTypeMultiplayerLightcycles:
			m_layout = "MultiplayerLightcycleDefault";
			m_briefing = "";
			m_help = "";
			m_missionname = "";
			m_levelname = "";
			break;
	}

	// the post-load screen shows them too
	CScreenPostload* pPostload = (CScreenPostload*)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_POSTLOAD);
	if (pPostload)
	{
		pPostload->SetLayout(m_layout.c_str());
		pPostload->m_briefing = m_briefing;
		pPostload->m_help = m_help;
	}
}
