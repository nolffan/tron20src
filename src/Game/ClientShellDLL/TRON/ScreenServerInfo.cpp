// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenServerInfo.cpp
//
// PURPOSE : Tron's server info tab: the players and the game settings of
//			 the server chosen in the server list
//
// RETAIL: written from retail CShell.dll 0x10077ec0..0x1007bcd0 (Tron's
// own class: the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenServerInfo.h"
#include "InterfaceMgr.h"
#include "ScreenMgr.h"
#include "ClientSoundMgr.h"

namespace
{
	// RETAIL: the refresh button's command (the server list's too)
	const uint32 kCmdRefresh = 0x436;

	// RETAIL: 0x101e8598: the refresh button waits this long
	const float kRefreshTime = 8.0f;

	// RETAIL: 0x101e859c, 0x101e85a8: the names of the light cycle types and
	// trail lengths
	const int s_aCycleTypeIDs[3] = { 0x259A, 0x259B, 0x259C };
	const int s_aTrailLengthIDs[4] = { 0x25C4, 0x25C5, 0x25C6, 0x25C7 };
}

// An info row: its name and an empty value, white and not selectable
#define ADD_INFO_CTRL(pCtrl, nStringID, pos) \
	pCtrl = AddColumnCtrl(0,0,pos); \
	pCtrl->AddColumn(LoadTempWString(nStringID),nLeftInfoWidth,LTTRUE); \
	pCtrl->AddColumn(L" ",nRightInfoWidth,LTTRUE); \
	pCtrl->SetColors(0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF); \
	pCtrl->Enable(LTFALSE);

// RETAIL: 0x10077ee0
CScreenServerInfo::CScreenServerInfo()
{
	for (int i = 0; i < 16; i++)
	{
		m_apPlayers[i] = LTNULL;
	}
	m_pPlayerList = LTNULL;
	m_pRefresh = LTNULL;
	m_pName = LTNULL;
	m_xTronUnknownA74 = LTNULL;
}

// RETAIL: 0x10077fc0
CScreenServerInfo::~CScreenServerInfo()
{
}

// RETAIL: 0x100780e0
LTBOOL CScreenServerInfo::Build()
{
	LTBOOL bRet = CBaseScreenJoin::Build();

	HTEXTURE hNormal = g_pInterfaceResMgr->GetTexture("interface\\icons\\refresh_normal.dtx");
	HTEXTURE hSelected = g_pInterfaceResMgr->GetTexture("interface\\icons\\refresh_selected.dtx");
	HTEXTURE hDisabled = g_pInterfaceResMgr->GetTexture("interface\\icons\\refresh_disabled.dtx");
	m_pRefresh = debug_new(CLTGUIButton);
	m_pRefresh->Create(kCmdRefresh,0xF66,hNormal,hSelected,hDisabled);
	m_pRefresh->SetCommandHandler(this);
	LTIntPt refreshPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"RefreshPos");
	m_pRefresh->SetBasePos(refreshPos);
	AddControl(m_pRefresh);
	m_fRefreshTimer = 0.0f;

	int nLeftPlayerWidth = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"LeftPlayerWidth");
	int nRightPlayerWidth = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"RightPlayerWidth");
	int nLeftInfoWidth = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"LeftInfoWidth");
	int nRightInfoWidth = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"RightInfoWidth");
	LTIntPt leftPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LeftColumnPos");
	LTIntPt rightPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"RightColumnPos");

	m_pNoInfo = AddTextItem(0xCA8,0,0,leftPos);
	m_pNoInfo->Enable(LTFALSE);

	// the players
	m_pPlayersHeader = AddColumnCtrl(0,0,leftPos);
	m_pPlayersHeader->AddColumn(LoadTempWString(0xF6D),nLeftPlayerWidth,LTTRUE);
	m_pPlayersHeader->AddColumn(L" ",nRightPlayerWidth,LTTRUE);
	m_pPlayersHeader->SetColors(0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF);
	m_pPlayersHeader->Enable(LTFALSE);

	m_pPlayerList = AddList(m_nextPos,rightPos.x - leftPos.x - 16);
	m_pPlayerList->SetFrameWidth(0);

	for (int i = 0; i < 16; i++)
	{
		m_apPlayers[i] = CreateColumnCtrl(0,0);
		m_apPlayers[i]->AddColumn(L" ",nLeftPlayerWidth,LTTRUE);
		m_apPlayers[i]->AddColumn(L" ",nRightPlayerWidth,LTTRUE);
		m_apPlayers[i]->SetColors(0xFF44FF44,0xFF44FF44,0xFF44FF44);
		m_apPlayers[i]->Enable(LTFALSE);
		m_pPlayerList->AddControl(m_apPlayers[i]);
	}

	// the server
	ADD_INFO_CTRL(m_pName,0x181,rightPos);
	ADD_INFO_CTRL(m_pMap,0xF6F,kDefaultPos);
	ADD_INFO_CTRL(m_pPassword,0xF4A,kDefaultPos);
	ADD_INFO_CTRL(m_pDedicated,0xF46,kDefaultPos);
	ADD_INFO_CTRL(m_pGameTypeName,0xF5A,kDefaultPos);

	// the game type's settings, one set at a time in the same place
	LTIntPt settingsPos = m_nextPos;

	ADD_INFO_CTRL(m_pTournamentTeamSize,0xF56,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentTeamSize);
	ADD_INFO_CTRL(m_pTournamentFriendlyFire,0xF58,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentFriendlyFire);
	ADD_INFO_CTRL(m_pTournamentWaitWeapons,0xF8E,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentWaitWeapons);
	ADD_INFO_CTRL(m_pTournamentAutoAim,0xF95,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentAutoAim);
	ADD_INFO_CTRL(m_pTournamentMatchPoints,0xF76,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentMatchPoints);
	ADD_INFO_CTRL(m_pTournamentMatchTime,0xF78,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentMatchTime);
	ADD_INFO_CTRL(m_pTournamentGamesPerMap,0xF7E,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentGamesPerMap);
	ADD_INFO_CTRL(m_pTournamentMaxPing,0x26B4,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentMaxPing);
	ADD_INFO_CTRL(m_pTournamentAirControl,0xF82,kDefaultPos);
	m_TournamentCtrls.push_back(m_pTournamentAirControl);

	ADD_INFO_CTRL(m_pArenaTeamSize,0xF56,settingsPos);
	m_ArenaCtrls.push_back(m_pArenaTeamSize);
	ADD_INFO_CTRL(m_pArenaFriendlyFire,0xF58,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaFriendlyFire);
	ADD_INFO_CTRL(m_pArenaWaitWeapons,0xF8E,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaWaitWeapons);
	ADD_INFO_CTRL(m_pArenaAutoAim,0xF95,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaAutoAim);
	ADD_INFO_CTRL(m_pArenaMatchPoints,0xF76,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaMatchPoints);
	ADD_INFO_CTRL(m_pArenaFragLimit,0xF7A,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaFragLimit);
	ADD_INFO_CTRL(m_pArenaTimeLimit,0xF7C,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaTimeLimit);
	ADD_INFO_CTRL(m_pArenaMatchTime,0xF78,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaMatchTime);
	ADD_INFO_CTRL(m_pArenaGamesPerMap,0xF7E,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaGamesPerMap);
	ADD_INFO_CTRL(m_pArenaMaxPing,0x26B4,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaMaxPing);
	ADD_INFO_CTRL(m_pArenaAirControl,0xF82,kDefaultPos);
	m_ArenaCtrls.push_back(m_pArenaAirControl);

	ADD_INFO_CTRL(m_pCycleTeamSize,0xF56,settingsPos);
	m_CycleCtrls.push_back(m_pCycleTeamSize);
	ADD_INFO_CTRL(m_pCycleType,0x2598,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleType);
	ADD_INFO_CTRL(m_pCycleTrailLength,0x25B0,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleTrailLength);
	ADD_INFO_CTRL(m_pCycleGamesPerMap,0x26AE,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleGamesPerMap);
	ADD_INFO_CTRL(m_pCycleFragLimit,0x26B0,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleFragLimit);
	ADD_INFO_CTRL(m_pCycleTimeLimit,0x26B2,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleTimeLimit);
	ADD_INFO_CTRL(m_pCycleMaxPing,0x26B4,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleMaxPing);
	ADD_INFO_CTRL(m_pCycleKillPoints,0x26BA,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleKillPoints);
	ADD_INFO_CTRL(m_pCycleDeathPoints,0x26BC,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleDeathPoints);
	ADD_INFO_CTRL(m_pCycleWinPoints,0x26BE,kDefaultPos);
	m_CycleCtrls.push_back(m_pCycleWinPoints);

	return bRet;
}

// RETAIL: 0x1007a3f0
void CScreenServerInfo::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		SetTabFrame(m_pServerInfoTab);
		UpdateInfo(false);
		m_fRefreshTimer = kRefreshTime;
		m_pRefresh->Enable(LTFALSE);
	}

	CBaseScreenJoin::OnFocus(bFocus);
}

// RETAIL: 0x1007a440
void CScreenServerInfo::ScreenDimsChanged()
{
	CBaseScreenJoin::ScreenDimsChanged();
	SetTabFrame(m_pServerInfoTab);
}

// RETAIL: 0x1007a460
uint32 CScreenServerInfo::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == kCmdRefresh)
	{
		Refresh();
		return 1;
	}

	return CBaseScreenJoin::OnCommand(dwCommand,dwParam1,dwParam2);
}

// RETAIL: 0x1007a490
LTBOOL CScreenServerInfo::Render(HSURFACE hDestSurf)
{
	if (m_fRefreshTimer > 0.0f)
	{
		m_fRefreshTimer -= g_pLTClient->GetFrameTime();
		if (m_fRefreshTimer <= 0.0f)
		{
			g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\newping.wav");
			m_pRefresh->Enable(LTTRUE);
		}
	}

	return CBaseScreenJoin::Render(hDestSurf);
}

// RETAIL: 0x1007ba80: a row's value
static void SetInfoValue(CLTGUIColumnCtrl *pCtrl, int nValue);

// RETAIL: 0x1007a500
void CScreenServerInfo::UpdateInfo(bool bRefresh)
{
	char szTmp[1024];
	std::wstring sText;

	m_pPlayersHeader->Show(LTFALSE);
	m_pName->Show(LTFALSE);
	m_pMap->Show(LTFALSE);
	m_pPassword->Show(LTFALSE);
	m_pDedicated->Show(LTFALSE);
	m_pGameTypeName->Show(LTFALSE);
	m_pNoInfo->Show(LTTRUE);

	int i;
	for (i = 0; i < 16; i++)
	{
		m_apPlayers[i]->Show(LTFALSE);
	}

	CtrlArray::iterator iter;
	for (iter = m_TournamentCtrls.begin(); iter != m_TournamentCtrls.end(); iter++)
	{
		(*iter)->Show(LTFALSE);
	}
	for (iter = m_ArenaCtrls.begin(); iter != m_ArenaCtrls.end(); iter++)
	{
		(*iter)->Show(LTFALSE);
	}
	for (iter = m_CycleCtrls.begin(); iter != m_CycleCtrls.end(); iter++)
	{
		(*iter)->Show(LTFALSE);
	}

	if (!g_hJoinServer || !g_bServerBrowser)
		return;

	if (bRefresh || !SBServerHasFullKeys(g_hJoinServer))
	{
		if (ServerBrowserAuxUpdateServer(g_hServerBrowser,g_hJoinServer,SBFalse,SBTrue) != sbe_noerror)
			return;
	}

	m_pPlayersHeader->Show(LTTRUE);
	m_pName->Show(LTTRUE);
	m_pMap->Show(LTTRUE);
	m_pPassword->Show(LTTRUE);
	m_pDedicated->Show(LTTRUE);
	m_pGameTypeName->Show(LTTRUE);
	m_pNoInfo->Show(LTFALSE);

	for (i = 0; i < 16; i++)
	{
		m_apPlayers[i]->Show(LTFALSE);
	}

	const char *pszGameType = SBServerGetStringValue(g_hJoinServer,"gametype","");
	if (!_stricmp(pszGameType,GameTypeToString(eGameTypeDiscArena)))
		m_nGameType = eGameTypeDiscArena;
	else if (!_stricmp(pszGameType,GameTypeToString(eGameTypeDiscTournament)))
		m_nGameType = eGameTypeDiscTournament;
	else if (!_stricmp(pszGameType,GameTypeToString(eGameTypeMultiplayerLightcycles)))
		m_nGameType = eGameTypeMultiplayerLightcycles;
	else
		return;

	// the players
	int nPlayers = SBServerGetIntValue(g_hJoinServer,"numplayers",0);
	int nMaxPlayers = SBServerGetIntValue(g_hJoinServer,"maxplayers",0);
	sprintf(szTmp,"(%d/%d)",nPlayers,nMaxPlayers);
	LTGUIWiden(sText,szTmp);
	m_pPlayersHeader->SetString(1,sText);

	for (i = 0; i < nPlayers; i++)
	{
		const char *pszName = SBServerGetPlayerStringValue(g_hJoinServer,i,"player","");
		int nScore = SBServerGetPlayerIntValue(g_hJoinServer,i,"score",0);
		LTGUIWiden(sText,pszName);
		m_apPlayers[i]->SetString(0,sText);
		SetInfoValue(m_apPlayers[i],nScore);
		m_apPlayers[i]->Show(LTTRUE);
	}

	// the game type's settings
	if (m_nGameType == eGameTypeDiscTournament)
	{
		for (iter = m_TournamentCtrls.begin(); iter != m_TournamentCtrls.end(); iter++)
		{
			(*iter)->Show(LTTRUE);
		}
	}
	if (m_nGameType == eGameTypeDiscArena)
	{
		for (iter = m_ArenaCtrls.begin(); iter != m_ArenaCtrls.end(); iter++)
		{
			(*iter)->Show(LTTRUE);
		}
	}
	if (m_nGameType == eGameTypeMultiplayerLightcycles)
	{
		for (iter = m_CycleCtrls.begin(); iter != m_CycleCtrls.end(); iter++)
		{
			(*iter)->Show(LTTRUE);
		}
	}

	// the server
	LTGUIWiden(sText,SBServerGetStringValue(g_hJoinServer,"hostname","- none -"));
	m_pName->SetString(1,sText);

	LTGUIWiden(sText,SBServerGetStringValue(g_hJoinServer,"mapname","- none - "));
	m_pMap->SetString(1,sText);

	m_pPassword->SetString(1,SBServerGetIntValue(g_hJoinServer,"password",0) ? LoadTempWString(0x35) : LoadTempWString(0x36));
	m_pDedicated->SetString(1,SBServerGetIntValue(g_hJoinServer,"dedicated",0) ? LoadTempWString(0x35) : LoadTempWString(0x36));

	int nGameTypeID = 0x3D;
	switch (m_nGameType)
	{
	case eGameTypeDiscArena:
		nGameTypeID = 0xF51;
		break;
	case eGameTypeDiscTournament:
		nGameTypeID = 0xF52;
		break;
	case eGameTypeMultiplayerLightcycles:
		nGameTypeID = 0xF53;
		break;
	}
	m_pGameTypeName->SetString(1,LoadTempWString(nGameTypeID));

	int nTeamSize;
	switch (m_nGameType)
	{
	case eGameTypeDiscArena:
		{
			nTeamSize = SBServerGetIntValue(g_hJoinServer,"teamplay",0);
			if (!nTeamSize)
				nTeamSize = 1;
			SetInfoValue(m_pArenaTeamSize,nTeamSize);

			m_pArenaFriendlyFire->SetString(1,SBServerGetIntValue(g_hJoinServer,"friendlyfire",0) ? LoadTempWString(0x35) : LoadTempWString(0x36));
			m_pArenaWaitWeapons->SetString(1,SBServerGetIntValue(g_hJoinServer,"waitweaps",0) ? LoadTempWString(0x35) : LoadTempWString(0x36));
			m_pArenaAutoAim->SetString(1,SBServerGetIntValue(g_hJoinServer,"autoaim",0) ? LoadTempWString(0x35) : LoadTempWString(0x36));

			SetInfoValue(m_pArenaMatchPoints,SBServerGetIntValue(g_hJoinServer,"matchpoints",0));
			SetInfoValue(m_pArenaFragLimit,SBServerGetIntValue(g_hJoinServer,"fraglimit",0));
			SetInfoValue(m_pArenaTimeLimit,SBServerGetIntValue(g_hJoinServer,"timelimit",0));
			SetInfoValue(m_pArenaMatchTime,SBServerGetIntValue(g_hJoinServer,"matchtime",0));
			SetInfoValue(m_pArenaGamesPerMap,SBServerGetIntValue(g_hJoinServer,"gamespermap",0));
			SetInfoValue(m_pArenaMaxPing,SBServerGetIntValue(g_hJoinServer,"maxping",0));
			SetInfoValue(m_pArenaAirControl,SBServerGetIntValue(g_hJoinServer,"aircontrol",0));
		}
		break;
	case eGameTypeDiscTournament:
		{
			nTeamSize = SBServerGetIntValue(g_hJoinServer,"teamplay",0);
			if (!nTeamSize)
				nTeamSize = 1;
			SetInfoValue(m_pTournamentTeamSize,nTeamSize);

			m_pTournamentFriendlyFire->SetString(1,SBServerGetIntValue(g_hJoinServer,"friendlyfire",0) ? LoadTempWString(0x35) : LoadTempWString(0x36));
			m_pTournamentWaitWeapons->SetString(1,SBServerGetIntValue(g_hJoinServer,"waitweaps",0) ? LoadTempWString(0x35) : LoadTempWString(0x36));
			m_pTournamentAutoAim->SetString(1,SBServerGetIntValue(g_hJoinServer,"autoaim",0) ? LoadTempWString(0x35) : LoadTempWString(0x36));

			SetInfoValue(m_pTournamentMatchPoints,SBServerGetIntValue(g_hJoinServer,"matchpoints",0));
			SetInfoValue(m_pTournamentMatchTime,SBServerGetIntValue(g_hJoinServer,"matchtime",0));
			SetInfoValue(m_pTournamentGamesPerMap,SBServerGetIntValue(g_hJoinServer,"gamespermap",0));
			SetInfoValue(m_pTournamentMaxPing,SBServerGetIntValue(g_hJoinServer,"maxping",0));
			SetInfoValue(m_pTournamentAirControl,SBServerGetIntValue(g_hJoinServer,"aircontrol",0));
		}
		break;

	case eGameTypeMultiplayerLightcycles:
		{
			nTeamSize = SBServerGetIntValue(g_hJoinServer,"teamplay",0);
			if (!nTeamSize)
				nTeamSize = 1;
			SetInfoValue(m_pCycleTeamSize,nTeamSize);

			int nCycleType = SBServerGetIntValue(g_hJoinServer,"cycletype",0);
			if (nCycleType < 0 || nCycleType >= 3)
				nCycleType = 0;
			m_pCycleType->SetString(1,LoadTempWString(s_aCycleTypeIDs[nCycleType]));

			int nTrailLength = SBServerGetIntValue(g_hJoinServer,"traillength",0);
			if (nTrailLength < 0 || nTrailLength >= 4)
				nTrailLength = 0;
			m_pCycleTrailLength->SetString(1,LoadTempWString(s_aTrailLengthIDs[nTrailLength]));

			SetInfoValue(m_pCycleGamesPerMap,SBServerGetIntValue(g_hJoinServer,"gamespermap",0));
			SetInfoValue(m_pCycleFragLimit,SBServerGetIntValue(g_hJoinServer,"fraglimit",0));
			SetInfoValue(m_pCycleTimeLimit,SBServerGetIntValue(g_hJoinServer,"timelimit",0));
			SetInfoValue(m_pCycleMaxPing,SBServerGetIntValue(g_hJoinServer,"maxping",0));
			SetInfoValue(m_pCycleKillPoints,SBServerGetIntValue(g_hJoinServer,"killpoints",0));
			SetInfoValue(m_pCycleDeathPoints,-SBServerGetIntValue(g_hJoinServer,"deathpoints",0));
			SetInfoValue(m_pCycleWinPoints,SBServerGetIntValue(g_hJoinServer,"winpoints",0));

			// RETAIL: read but not shown: the tab has no row for it (a retail
			// bug, to list in docs/RETAIL_BUGS.md once this is identical)
			SBServerGetIntValue(g_hJoinServer,"powerups",0);
		}
		break;

	}
}

// RETAIL: 0x1007ba80
static void SetInfoValue(CLTGUIColumnCtrl *pCtrl, int nValue)
{
	char szTmp[16];
	sprintf(szTmp,"%d",nValue);

	std::wstring sText;
	LTGUIWiden(sText,szTmp);
	pCtrl->SetString(1,sText);
}

// RETAIL: 0x1007bc40
void CScreenServerInfo::Refresh()
{
	if (m_fRefreshTimer > 0.0f)
	{
		MBCreate mb;
		g_pInterfaceMgr->ShowMessageBox(0xCA4,&mb);
		return;
	}

	m_fRefreshTimer = kRefreshTime;
	m_pRefresh->Enable(LTFALSE);

	if (g_hJoinServer)
	{
		UpdateInfo(true);
	}
}
