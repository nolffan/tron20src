// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostGame.cpp
//
// PURPOSE : Tron's host screen: the settings of the three multiplayer game
//			 types (disc arena, disc tournament, light cycles)
//
// RETAIL: written from retail CShell.dll 0x1005d9e0..0x10060c20. The string
// IDs are Tron's (its CRes.dll): 0xF5A the game type, 0xF51..0xF53 the
// types, 0x35/0x36 on and off; the others are each control's name and help.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostGame.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ServerOptionMgr.h"
#include "NetDefs.h"
#include "InterfaceMgr.h"

namespace
{
	// RETAIL: 0x1026e9a0: the screen, for the team sliders' callback (set
	// by Build, cleared by Term)
	CScreenHostGame *s_pHostGame = LTNULL;

	// RETAIL: Tron's command IDs (this screen's)
	const uint32 kCmdGameTypeChanged	= 0x43B;
	const uint32 kCmdShowPowerups		= 0x43C;
	const uint32 kCmdHidePowerups		= 0x43D;

	// RETAIL: the team sliders' limits on the players (teams times team size)
	const int kMaxDiscPlayers			= 16;
	const int kMaxLCPlayers				= 8;

	// RETAIL: 0x1005da00
	bool TeamSizeCallBack(CLTGUISlider* pSlider, int nNewPos, int nOldPos)
	{
		if (s_pHostGame && pSlider)
			return s_pHostGame->CheckTeamSize(pSlider,nNewPos);
		return false;
	}
}

// RETAIL: 0x1005da30
CScreenHostGame::CScreenHostGame()
{
	m_pGameTypeCycle			= LTNULL;
	m_nGameType					= 0;

	m_pArenaTeams				= LTNULL;
	m_pArenaTeamSize			= LTNULL;
	m_pArenaFriendlyFire		= LTNULL;
	m_pArenaDiscsInSpectator	= LTNULL;
	m_pArenaAllowAutoAim		= LTNULL;
	m_pArenaPointsPerMatch		= LTNULL;
	m_pArenaMatchTimeLimit		= LTNULL;
	m_pArenaWinLimit			= LTNULL;
	m_pArenaTimeLimit			= LTNULL;
	m_pArenaGamesPerMap			= LTNULL;
	m_pArenaPreGameDelay		= LTNULL;
	m_pArenaMaxPing				= LTNULL;
	m_pArenaAirControl			= LTNULL;
	m_nArenaTeams				= 4;
	m_nArenaTeamSize			= 2;
	m_bArenaFriendlyFire		= LTFALSE;
	m_bArenaDiscsInSpectator	= LTTRUE;
	m_bArenaAllowAutoAim		= LTTRUE;
	m_nArenaPointsPerMatch		= 0;
	m_nArenaMatchTimeLimit		= 5;
	m_nArenaWinLimit			= 5;
	m_nArenaTimeLimit			= 20;
	m_nArenaGamesPerMap			= 1;
	m_nArenaPreGameDelay		= 60;
	m_nArenaMaxPing				= 0;
	m_nArenaAirControl			= 50;

	m_pTournTeams				= LTNULL;
	m_pTournTeamSize			= LTNULL;
	m_pTournFriendlyFire		= LTNULL;
	m_pTournDiscsInSpectator	= LTNULL;
	m_pTournAllowAutoAim		= LTNULL;
	m_pTournPointsPerMatch		= LTNULL;
	m_pTournMatchTimeLimit		= LTNULL;
	m_pTournTeamsToStart		= LTNULL;
	m_pTournGamesPerMap			= LTNULL;
	m_pTournPreGameDelay		= LTNULL;
	m_pTournMaxPing				= LTNULL;
	m_pTournAirControl			= LTNULL;
	m_nTournTeams				= 4;
	m_nTournTeamSize			= 2;
	m_bTournFriendlyFire		= LTFALSE;
	m_bTournDiscsInSpectator	= LTTRUE;
	m_bTournAllowAutoAim		= LTTRUE;
	m_nTournPointsPerMatch		= 0;
	m_nTournMatchTimeLimit		= 5;
	m_nTournTeamsToStart		= 2;
	m_nTournMode				= 0;
	m_nTournGamesPerMap			= 1;
	m_nTournPreGameDelay		= 60;
	m_nTournMaxPing				= 0;
	m_nTournAirControl			= 50;

	m_pLCTeams					= LTNULL;
	m_pLCTeamSize				= LTNULL;
	m_pLCTrailLength			= LTNULL;
	m_pLCGameSpeed				= LTNULL;
	m_pLCAllowedCycleType		= LTNULL;
	m_pLCWaitForPlayersTime		= LTNULL;
	m_pLCTotalRaces				= LTNULL;
	m_pLCTotalPoints			= LTNULL;
	m_pLCTotalTime				= LTNULL;
	m_pLCMaxPing				= LTNULL;
	m_pLCShield					= LTNULL;
	m_pLCTurbo					= LTNULL;
	m_pLCTrailSpike				= LTNULL;
	m_pLCMissile				= LTNULL;
	m_pLCTurboCurse				= LTNULL;
	m_pLCGrab					= LTNULL;
	m_pLCTrailReset				= LTNULL;
	m_pLCForceUse				= LTNULL;
	m_pLCWallExtender			= LTNULL;
	m_pPowerups					= LTNULL;
	m_pPowerupList				= LTNULL;
	m_pPowerupsDone				= LTNULL;
	m_pLCKillPointValue			= LTNULL;
	m_pLCSelfKillPointValue		= LTNULL;
	m_pLCWinRacePointValue		= LTNULL;
	m_nLCTeams					= 4;
	m_nLCTeamSize				= 2;
	m_nLCTrailLength			= 1;
	m_nLCGameSpeed				= 1;
	m_nLCAllowedCycleType		= 0;
	m_nLCWaitForPlayersTime		= 0;
	m_nLCTotalRaces				= 0;
	m_nLCTotalPoints			= 0;
	m_nLCTotalTime				= 0;
	m_nLCMaxPing				= 0;
	m_bLCShield					= LTTRUE;
	m_bLCTurbo					= LTTRUE;
	m_bLCTrailSpike				= LTTRUE;
	m_bLCMissile				= LTTRUE;
	m_bLCTurboCurse				= LTTRUE;
	m_bLCGrab					= LTTRUE;
	m_bLCTrailReset				= LTTRUE;
	m_bLCForceUse				= LTTRUE;
	m_bLCWallExtender			= LTTRUE;
	m_nLCKillPointValue			= 1;
	m_nLCSelfKillPointValue		= -1;
	m_nLCWinRacePointValue		= 6;
}

// RETAIL: 0x1005dd30
CScreenHostGame::~CScreenHostGame()
{
}

// RETAIL: 0x1005de50
void CScreenHostGame::Term()
{
	s_pHostGame = LTNULL;
	CBaseScreenHost::Term();
}

// RETAIL: 0x1005de60
LTBOOL CScreenHostGame::Build()
{
	LTBOOL bRet = CBaseScreenHost::Build();
	std::wstring sText;

	s_pHostGame = this;

	int kColumn0 = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST,"ColumnWidth");

	m_pGameTypeCycle = AddCycle(0xF5A, 0xF5B, kColumn0, &m_nGameType, g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST_GAME,"CommandPos"));
	m_pGameTypeCycle->NotifyOnChange(kCmdGameTypeChanged,this);
	m_pGameTypeCycle->AddString(LoadTempWString(0xF51));
	m_pGameTypeCycle->AddString(LoadTempWString(0xF52));
	m_pGameTypeCycle->AddString(LoadTempWString(0xF53));

	// each game type's controls start at the same place
	LTIntPt startPos = m_nextPos;

	// the disc arena
	m_pArenaTeams = AddSlider(0xF54, 0xF55, kColumn0, 50, -1, &m_nArenaTeams, startPos);
	m_pArenaTeams->SetNumericDisplay(LTTRUE);
	m_pArenaTeams->ShowBar(LTFALSE);
	m_pArenaTeams->SetSliderRange(2,kMaxDiscPlayers);
	m_pArenaTeams->SetSliderIncrement(1);
	m_pArenaTeams->SetRangeCallback(TeamSizeCallBack);
	m_ArenaCtrls.push_back(m_pArenaTeams);

	m_pArenaTeamSize = AddSlider(0xF56, 0xF57, kColumn0, 50, -1, &m_nArenaTeamSize);
	m_pArenaTeamSize->SetNumericDisplay(LTTRUE);
	m_pArenaTeamSize->ShowBar(LTFALSE);
	m_pArenaTeamSize->SetSliderRange(1,4);
	m_pArenaTeamSize->SetSliderIncrement(1);
	m_pArenaTeamSize->SetRangeCallback(TeamSizeCallBack);
	m_ArenaCtrls.push_back(m_pArenaTeamSize);

	m_pArenaFriendlyFire = AddToggle(0xF58, 0xF59, kColumn0, &m_bArenaFriendlyFire);
	m_pArenaFriendlyFire->SetOnString(LoadTempWString(0x35));
	m_pArenaFriendlyFire->SetOffString(LoadTempWString(0x36));
	m_ArenaCtrls.push_back(m_pArenaFriendlyFire);

	m_pArenaDiscsInSpectator = AddToggle(0xF8E, 0xF8F, kColumn0, &m_bArenaDiscsInSpectator);
	m_pArenaDiscsInSpectator->SetOnString(LoadTempWString(0x35));
	m_pArenaDiscsInSpectator->SetOffString(LoadTempWString(0x36));
	m_ArenaCtrls.push_back(m_pArenaDiscsInSpectator);

	m_pArenaAllowAutoAim = AddToggle(0xF95, 0xF96, kColumn0, &m_bArenaAllowAutoAim);
	m_pArenaAllowAutoAim->SetOnString(LoadTempWString(0x35));
	m_pArenaAllowAutoAim->SetOffString(LoadTempWString(0x36));
	m_ArenaCtrls.push_back(m_pArenaAllowAutoAim);

	m_pArenaPointsPerMatch = AddSlider(0xF76, 0xF77, kColumn0, 50, -1, &m_nArenaPointsPerMatch);
	m_pArenaPointsPerMatch->SetNumericDisplay(LTTRUE);
	m_pArenaPointsPerMatch->ShowBar(LTFALSE);
	m_pArenaPointsPerMatch->SetSliderRange(1,5);
	m_pArenaPointsPerMatch->SetSliderIncrement(2);
	m_ArenaCtrls.push_back(m_pArenaPointsPerMatch);

	m_pArenaMatchTimeLimit = AddSlider(0xF78, 0xF79, kColumn0, 50, -1, &m_nArenaMatchTimeLimit);
	m_pArenaMatchTimeLimit->SetNumericDisplay(LTTRUE);
	m_pArenaMatchTimeLimit->ShowBar(LTFALSE);
	m_pArenaMatchTimeLimit->SetSliderRange(1,60);
	m_pArenaMatchTimeLimit->SetSliderIncrement(1);
	m_ArenaCtrls.push_back(m_pArenaMatchTimeLimit);

	m_pArenaWinLimit = AddSlider(0xF7A, 0xF7B, kColumn0, 50, -1, &m_nArenaWinLimit);
	m_pArenaWinLimit->SetNumericDisplay(LTTRUE);
	m_pArenaWinLimit->ShowBar(LTFALSE);
	m_pArenaWinLimit->SetSliderRange(1,100);
	m_pArenaWinLimit->SetSliderIncrement(1);
	m_ArenaCtrls.push_back(m_pArenaWinLimit);

	m_pArenaTimeLimit = AddSlider(0xF7C, 0xF7D, kColumn0, 50, -1, &m_nArenaTimeLimit);
	m_pArenaTimeLimit->SetNumericDisplay(LTTRUE);
	m_pArenaTimeLimit->ShowBar(LTFALSE);
	m_pArenaTimeLimit->SetSliderRange(0,120);
	m_pArenaTimeLimit->SetSliderIncrement(1);
	m_ArenaCtrls.push_back(m_pArenaTimeLimit);

	m_pArenaGamesPerMap = AddSlider(0xF7E, 0xF7F, kColumn0, 50, -1, &m_nArenaGamesPerMap);
	m_pArenaGamesPerMap->SetNumericDisplay(LTTRUE);
	m_pArenaGamesPerMap->ShowBar(LTFALSE);
	m_pArenaGamesPerMap->SetSliderRange(1,20);
	m_pArenaGamesPerMap->SetSliderIncrement(1);
	m_ArenaCtrls.push_back(m_pArenaGamesPerMap);

	m_pArenaPreGameDelay = AddSlider(0xF80, 0xF81, kColumn0, 50, -1, &m_nArenaPreGameDelay);
	m_pArenaPreGameDelay->SetNumericDisplay(LTTRUE);
	m_pArenaPreGameDelay->ShowBar(LTFALSE);
	m_pArenaPreGameDelay->SetSliderRange(0,120);
	m_pArenaPreGameDelay->SetSliderIncrement(5);
	m_ArenaCtrls.push_back(m_pArenaPreGameDelay);

	m_pArenaMaxPing = AddSlider(0x26B4, 0x26B5, kColumn0, 50, -1, &m_nArenaMaxPing);
	m_pArenaMaxPing->SetNumericDisplay(LTTRUE);
	m_pArenaMaxPing->ShowBar(LTFALSE);
	m_pArenaMaxPing->SetSliderRange(0,1000);
	m_pArenaMaxPing->SetSliderIncrement(50);
	m_ArenaCtrls.push_back(m_pArenaMaxPing);

	m_pArenaAirControl = AddSlider(0xF82, 0xF83, kColumn0, 50, -1, &m_nArenaAirControl);
	m_pArenaAirControl->SetNumericDisplay(LTTRUE);
	m_pArenaAirControl->ShowBar(LTFALSE);
	m_pArenaAirControl->SetSliderRange(0,100);
	m_pArenaAirControl->SetSliderIncrement(5);
	m_ArenaCtrls.push_back(m_pArenaAirControl);

	// the disc tournament
	m_pTournTeams = AddSlider(0xF54, 0xF55, kColumn0, 50, -1, &m_nTournTeams, startPos);
	m_pTournTeams->SetNumericDisplay(LTTRUE);
	m_pTournTeams->ShowBar(LTFALSE);
	m_pTournTeams->SetSliderRange(2,kMaxDiscPlayers);
	m_pTournTeams->SetSliderIncrement(1);
	m_pTournTeams->SetRangeCallback(TeamSizeCallBack);
	m_TournCtrls.push_back(m_pTournTeams);

	m_pTournTeamSize = AddSlider(0xF56, 0xF57, kColumn0, 50, -1, &m_nTournTeamSize);
	m_pTournTeamSize->SetNumericDisplay(LTTRUE);
	m_pTournTeamSize->ShowBar(LTFALSE);
	m_pTournTeamSize->SetSliderRange(1,4);
	m_pTournTeamSize->SetSliderIncrement(1);
	m_pTournTeamSize->SetRangeCallback(TeamSizeCallBack);
	m_TournCtrls.push_back(m_pTournTeamSize);

	m_pTournFriendlyFire = AddToggle(0xF58, 0xF59, kColumn0, &m_bTournFriendlyFire);
	m_pTournFriendlyFire->SetOnString(LoadTempWString(0x35));
	m_pTournFriendlyFire->SetOffString(LoadTempWString(0x36));
	m_TournCtrls.push_back(m_pTournFriendlyFire);

	m_pTournDiscsInSpectator = AddToggle(0xF8E, 0xF8F, kColumn0, &m_bTournDiscsInSpectator);
	m_pTournDiscsInSpectator->SetOnString(LoadTempWString(0x35));
	m_pTournDiscsInSpectator->SetOffString(LoadTempWString(0x36));
	m_TournCtrls.push_back(m_pTournDiscsInSpectator);

	m_pTournAllowAutoAim = AddToggle(0xF95, 0xF96, kColumn0, &m_bTournAllowAutoAim);
	m_pTournAllowAutoAim->SetOnString(LoadTempWString(0x35));
	m_pTournAllowAutoAim->SetOffString(LoadTempWString(0x36));
	m_TournCtrls.push_back(m_pTournAllowAutoAim);

	m_pTournPointsPerMatch = AddSlider(0xF76, 0xF77, kColumn0, 50, -1, &m_nTournPointsPerMatch);
	m_pTournPointsPerMatch->SetNumericDisplay(LTTRUE);
	m_pTournPointsPerMatch->ShowBar(LTFALSE);
	m_pTournPointsPerMatch->SetSliderRange(1,5);
	m_pTournPointsPerMatch->SetSliderIncrement(2);
	m_TournCtrls.push_back(m_pTournPointsPerMatch);

	m_pTournMatchTimeLimit = AddSlider(0xF78, 0xF79, kColumn0, 50, -1, &m_nTournMatchTimeLimit);
	m_pTournMatchTimeLimit->SetNumericDisplay(LTTRUE);
	m_pTournMatchTimeLimit->ShowBar(LTFALSE);
	m_pTournMatchTimeLimit->SetSliderRange(1,60);
	m_pTournMatchTimeLimit->SetSliderIncrement(1);
	m_TournCtrls.push_back(m_pTournMatchTimeLimit);

	m_pTournTeamsToStart = AddSlider(0xF8C, 0xF8D, kColumn0, 50, -1, &m_nTournTeamsToStart);
	m_pTournTeamsToStart->SetNumericDisplay(LTTRUE);
	m_pTournTeamsToStart->ShowBar(LTFALSE);
	m_pTournTeamsToStart->SetSliderRange(2,16);
	m_pTournTeamsToStart->SetSliderIncrement(1);
	m_TournCtrls.push_back(m_pTournTeamsToStart);

	m_pTournGamesPerMap = AddSlider(0xF88, 0xF89, kColumn0, 50, -1, &m_nTournGamesPerMap);
	m_pTournGamesPerMap->SetNumericDisplay(LTTRUE);
	m_pTournGamesPerMap->ShowBar(LTFALSE);
	m_pTournGamesPerMap->SetSliderRange(1,20);
	m_pTournGamesPerMap->SetSliderIncrement(1);
	m_TournCtrls.push_back(m_pTournGamesPerMap);

	m_pTournPreGameDelay = AddSlider(0xF8A, 0xF8B, kColumn0, 50, -1, &m_nTournPreGameDelay);
	m_pTournPreGameDelay->SetNumericDisplay(LTTRUE);
	m_pTournPreGameDelay->ShowBar(LTFALSE);
	m_pTournPreGameDelay->SetSliderRange(0,120);
	m_pTournPreGameDelay->SetSliderIncrement(5);
	m_TournCtrls.push_back(m_pTournPreGameDelay);

	m_pTournMaxPing = AddSlider(0x26B4, 0x26B5, kColumn0, 50, -1, &m_nTournMaxPing);
	m_pTournMaxPing->SetNumericDisplay(LTTRUE);
	m_pTournMaxPing->ShowBar(LTFALSE);
	m_pTournMaxPing->SetSliderRange(0,1000);
	m_pTournMaxPing->SetSliderIncrement(50);
	m_TournCtrls.push_back(m_pTournMaxPing);

	m_pTournAirControl = AddSlider(0xF82, 0xF83, kColumn0, 50, -1, &m_nTournAirControl);
	m_pTournAirControl->SetNumericDisplay(LTTRUE);
	m_pTournAirControl->ShowBar(LTFALSE);
	m_pTournAirControl->SetSliderRange(0,100);
	m_pTournAirControl->SetSliderIncrement(5);
	m_TournCtrls.push_back(m_pTournAirControl);

	// the light cycles
	m_pLCTeams = AddSlider(0xF54, 0xF55, kColumn0, 50, -1, &m_nLCTeams, startPos);
	m_pLCTeams->SetNumericDisplay(LTTRUE);
	m_pLCTeams->ShowBar(LTFALSE);
	m_pLCTeams->SetSliderRange(2,kMaxLCPlayers);
	m_pLCTeams->SetSliderIncrement(1);
	m_pLCTeams->SetRangeCallback(TeamSizeCallBack);
	m_LCCtrls.push_back(m_pLCTeams);

	m_pLCTeamSize = AddSlider(0xF56, 0xF57, kColumn0, 50, -1, &m_nLCTeamSize);
	m_pLCTeamSize->SetNumericDisplay(LTTRUE);
	m_pLCTeamSize->ShowBar(LTFALSE);
	m_pLCTeamSize->SetSliderRange(1,4);
	m_pLCTeamSize->SetSliderIncrement(1);
	m_pLCTeamSize->SetRangeCallback(TeamSizeCallBack);
	m_LCCtrls.push_back(m_pLCTeamSize);

	m_pLCAllowedCycleType = AddCycle(0x2598, 0x2599, kColumn0, &m_nLCAllowedCycleType);
	m_pLCAllowedCycleType->AddString(LoadTempWString(0x259A));
	m_pLCAllowedCycleType->AddString(LoadTempWString(0x259B));
	m_pLCAllowedCycleType->AddString(LoadTempWString(0x259C));
	m_LCCtrls.push_back(m_pLCAllowedCycleType);

	m_pLCGameSpeed = AddCycle(0x25AE, 0x25AF, kColumn0, &m_nLCGameSpeed);
	m_pLCGameSpeed->AddString(LoadTempWString(0x25C0));
	m_pLCGameSpeed->AddString(LoadTempWString(0x25C1));
	m_pLCGameSpeed->AddString(LoadTempWString(0x25C2));
	m_pLCGameSpeed->AddString(LoadTempWString(0x25C3));
	m_LCCtrls.push_back(m_pLCGameSpeed);

	m_pLCTrailLength = AddCycle(0x25B0, 0x25B1, kColumn0, &m_nLCTrailLength);
	m_pLCTrailLength->AddString(LoadTempWString(0x25C4));
	m_pLCTrailLength->AddString(LoadTempWString(0x25C5));
	m_pLCTrailLength->AddString(LoadTempWString(0x25C6));
	m_pLCTrailLength->AddString(LoadTempWString(0x25C7));
	m_LCCtrls.push_back(m_pLCTrailLength);

	m_pLCWaitForPlayersTime = AddSlider(0x26AC, 0x26AD, kColumn0, 50, -1, &m_nLCWaitForPlayersTime);
	m_pLCWaitForPlayersTime->SetNumericDisplay(LTTRUE);
	m_pLCWaitForPlayersTime->ShowBar(LTFALSE);
	m_pLCWaitForPlayersTime->SetSliderRange(10,60);
	m_pLCWaitForPlayersTime->SetSliderIncrement(5);
	m_LCCtrls.push_back(m_pLCWaitForPlayersTime);

	m_pLCTotalRaces = AddSlider(0x26AE, 0x26AF, kColumn0, 50, -1, &m_nLCTotalRaces);
	m_pLCTotalRaces->SetNumericDisplay(LTTRUE);
	m_pLCTotalRaces->ShowBar(LTFALSE);
	m_pLCTotalRaces->SetSliderRange(0,50);
	m_pLCTotalRaces->SetSliderIncrement(1);
	m_LCCtrls.push_back(m_pLCTotalRaces);

	m_pLCTotalPoints = AddSlider(0x26B0, 0x26B1, kColumn0, 50, -1, &m_nLCTotalPoints);
	m_pLCTotalPoints->SetNumericDisplay(LTTRUE);
	m_pLCTotalPoints->ShowBar(LTFALSE);
	m_pLCTotalPoints->SetSliderRange(0,996);
	m_pLCTotalPoints->SetSliderIncrement(6);
	m_LCCtrls.push_back(m_pLCTotalPoints);

	m_pLCTotalTime = AddSlider(0x26B2, 0x26B3, kColumn0, 50, -1, &m_nLCTotalTime);
	m_pLCTotalTime->SetNumericDisplay(LTTRUE);
	m_pLCTotalTime->ShowBar(LTFALSE);
	m_pLCTotalTime->SetSliderRange(0,60);
	m_pLCTotalTime->SetSliderIncrement(1);
	m_LCCtrls.push_back(m_pLCTotalTime);

	m_pLCMaxPing = AddSlider(0x26B4, 0x26B5, kColumn0, 50, -1, &m_nLCMaxPing);
	m_pLCMaxPing->SetNumericDisplay(LTTRUE);
	m_pLCMaxPing->ShowBar(LTFALSE);
	m_pLCMaxPing->SetSliderRange(50,1000);
	m_pLCMaxPing->SetSliderIncrement(10);
	m_LCCtrls.push_back(m_pLCMaxPing);

	m_pLCKillPointValue = AddSlider(0x26BA, 0x26BB, kColumn0, 50, -1, &m_nLCKillPointValue);
	m_pLCKillPointValue->SetNumericDisplay(LTTRUE);
	m_pLCKillPointValue->ShowBar(LTFALSE);
	m_pLCKillPointValue->SetSliderRange(0,100);
	m_pLCKillPointValue->SetSliderIncrement(1);
	m_LCCtrls.push_back(m_pLCKillPointValue);

	m_pLCSelfKillPointValue = AddSlider(0x26BC, 0x26BD, kColumn0, 50, -1, &m_nLCSelfKillPointValue);
	m_pLCSelfKillPointValue->SetNumericDisplay(LTTRUE);
	m_pLCSelfKillPointValue->ShowBar(LTFALSE);
	m_pLCSelfKillPointValue->SetSliderRange(-100,0);
	m_pLCSelfKillPointValue->SetSliderIncrement(1);
	m_LCCtrls.push_back(m_pLCSelfKillPointValue);

	m_pLCWinRacePointValue = AddSlider(0x26BE, 0x26BF, kColumn0, 50, -1, &m_nLCWinRacePointValue);
	m_pLCWinRacePointValue->SetNumericDisplay(LTTRUE);
	m_pLCWinRacePointValue->ShowBar(LTFALSE);
	m_pLCWinRacePointValue->SetSliderRange(0,100);
	m_pLCWinRacePointValue->SetSliderIncrement(1);
	m_LCCtrls.push_back(m_pLCWinRacePointValue);

	m_pPowerups = AddTextItem(0x26B6, kCmdShowPowerups, 0x26B7);
	m_LCCtrls.push_back(m_pPowerups);

	// the power-ups: a list over the body, closed by "Done"
	LTRect rcList = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_HOST,"BodyRect");
	rcList.left += 32;
	rcList.top += 32;
	rcList.right -= 32;
	rcList.bottom -= 32;
	int nWidth = rcList.right - rcList.left;
	m_pPowerupList = AddList(LTIntPt(rcList.left,rcList.top),rcList.bottom - rcList.top,LTTRUE,nWidth - 16);
	if (m_pPowerupList)
	{
		m_pPowerupList->SetIndent(g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LCPowerupListIndent"));
		m_pPowerupList->SetFrameTexture(g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\frame_solid.dtx"));
		m_pPowerupList->Show(LTFALSE);

		m_pLCShield = CreateToggle(0x2580, 0x2581, kColumn0, &m_bLCShield);
		m_pLCShield->SetOnString(LoadTempWString(0x35));
		m_pLCShield->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCShield);

		m_pLCTurbo = CreateToggle(0x2582, 0x2583, kColumn0, &m_bLCTurbo);
		m_pLCTurbo->SetOnString(LoadTempWString(0x35));
		m_pLCTurbo->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCTurbo);

		m_pLCTrailSpike = CreateToggle(0x2584, 0x2585, kColumn0, &m_bLCTrailSpike);
		m_pLCTrailSpike->SetOnString(LoadTempWString(0x35));
		m_pLCTrailSpike->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCTrailSpike);

		m_pLCMissile = CreateToggle(0x2586, 0x2587, kColumn0, &m_bLCMissile);
		m_pLCMissile->SetOnString(LoadTempWString(0x35));
		m_pLCMissile->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCMissile);

		m_pLCTurboCurse = CreateToggle(0x2588, 0x2589, kColumn0, &m_bLCTurboCurse);
		m_pLCTurboCurse->SetOnString(LoadTempWString(0x35));
		m_pLCTurboCurse->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCTurboCurse);

		m_pLCGrab = CreateToggle(0x258A, 0x258B, kColumn0, &m_bLCGrab);
		m_pLCGrab->SetOnString(LoadTempWString(0x35));
		m_pLCGrab->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCGrab);

		m_pLCTrailReset = CreateToggle(0x258C, 0x258D, kColumn0, &m_bLCTrailReset);
		m_pLCTrailReset->SetOnString(LoadTempWString(0x35));
		m_pLCTrailReset->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCTrailReset);

		m_pLCForceUse = CreateToggle(0x258E, 0x258F, kColumn0, &m_bLCForceUse);
		m_pLCForceUse->SetOnString(LoadTempWString(0x35));
		m_pLCForceUse->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCForceUse);

		m_pLCWallExtender = CreateToggle(0x2576, 0x2577, kColumn0, &m_bLCWallExtender);
		m_pLCWallExtender->SetOnString(LoadTempWString(0x35));
		m_pLCWallExtender->SetOffString(LoadTempWString(0x36));
		m_pPowerupList->AddControl(m_pLCWallExtender);

		int nX = rcList.left + nWidth / 2;
		int nY = rcList.bottom - 32;
		m_pPowerupsDone = AddTextItem(0x42, kCmdHidePowerups, 0xEB, LTIntPt(nX,nY));
		nX -= m_pPowerupsDone->GetBaseWidth() / 2;
		m_pPowerupsDone->SetBasePos(LTIntPt(nX,nY));
		m_pPowerupsDone->Show(LTFALSE);
	}

	return bRet;
}

// RETAIL: 0x100600a0
uint32 CScreenHostGame::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		ShowPowerups(false);
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}

	switch(dwCommand)
	{
	case kCmdGameTypeChanged:
		{
			m_pGameTypeCycle->UpdateData(LTTRUE);

			char szGameType[128];
			FormatString(0xF50,szGameType,sizeof(szGameType),LoadTempString(0xF51 + m_nGameType));
			std::wstring sGameType;
			LTGUIWiden(sGameType,szGameType);
			m_pGameType->SetString(sGameType);

			UpdateGameType();
		} break;
	case CMD_BACK:
		{
			m_pScreenMgr->EscapeCurrentScreen();
		} break;
	case kCmdShowPowerups:
		{
			ShowPowerups(true);
		} break;
	case kCmdHidePowerups:
		{
			ShowPowerups(false);
		} break;
	default:
		return CBaseScreenHost::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// RETAIL: 0x10060340
void CScreenHostGame::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		LoadOptions();
		SetTabFrame(m_pGameTab);
		SetSelection(GetIndex(m_pGameTab));
		m_pPowerupList->Show(LTFALSE);
	}
	CBaseScreenHost::OnFocus(bFocus);
}

// RETAIL: 0x100603a0
void CScreenHostGame::ScreenDimsChanged()
{
	CBaseScreenHost::ScreenDimsChanged();
	SetTabFrame(m_pGameTab);
}

// RETAIL: 0x100603c0
void CScreenHostGame::UpdateGameType()
{
	ControlArray::iterator iter;

	LTBOOL bShow = (m_nGameType == 0);
	for (iter = m_ArenaCtrls.begin(); iter != m_ArenaCtrls.end(); iter++)
		(*iter)->Show(bShow);

	bShow = (m_nGameType == 1);
	for (iter = m_TournCtrls.begin(); iter != m_TournCtrls.end(); iter++)
		(*iter)->Show(bShow);

	bShow = (m_nGameType == 2);
	for (iter = m_LCCtrls.begin(); iter != m_LCCtrls.end(); iter++)
		(*iter)->Show(bShow);
}

// RETAIL: 0x10060470
LTBOOL CScreenHostGame::OnLeft()
{
	return CBaseScreenHost::OnLeft();
}

// RETAIL: 0x10060480
LTBOOL CScreenHostGame::OnRight()
{
	return CBaseScreenHost::OnRight();
}

// RETAIL: 0x10060490: Escape closes the power-up list first
void CScreenHostGame::Escape()
{
	if (m_pPowerupList->IsVisible())
	{
		ShowPowerups(false);
		PlayInterfaceSound(IS_SELECT);
	}
	else
		CBaseScreenHost::Escape();
}

// RETAIL: 0x100604c0
void CScreenHostGame::LoadOptions()
{
	CBaseScreenHost::LoadOptions();

	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();

	m_nGameType = pOptions->m_eGameType - eGameTypeDiscArena;

	m_nArenaTeams				= pOptions->m_nArenaMaxPlayers / pOptions->m_nArenaMaxTeamSize;
	m_nArenaTeamSize			= pOptions->m_nArenaMaxTeamSize;
	m_bArenaFriendlyFire		= pOptions->m_bArenaFriendlyFire;
	m_bArenaDiscsInSpectator	= pOptions->m_bArenaDiscsInSpectator;
	m_bArenaAllowAutoAim		= pOptions->m_bArenaAllowAutoAim;
	m_nArenaPointsPerMatch		= pOptions->m_nArenaPointsPerMatch;
	m_nArenaMatchTimeLimit		= pOptions->m_nArenaMatchTimeLimit;
	m_nArenaWinLimit			= pOptions->m_nArenaWinLimit;
	m_nArenaTimeLimit			= pOptions->m_nArenaTimeLimit;
	m_nArenaGamesPerMap			= pOptions->m_nArenaGamesPerMap;
	m_nArenaPreGameDelay		= pOptions->m_nArenaPreGameDelay;
	m_nArenaMaxPing				= pOptions->m_nArenaMaxPing;
	m_nArenaAirControl			= pOptions->m_nArenaAirControl;

	m_nTournTeams				= pOptions->m_nTournMaxPlayers / pOptions->m_nTournMaxTeamSize;
	m_nTournTeamSize			= pOptions->m_nTournMaxTeamSize;
	m_bTournFriendlyFire		= pOptions->m_bTournFriendlyFire;
	m_bTournDiscsInSpectator	= pOptions->m_bTournDiscsInSpectator;
	m_bTournAllowAutoAim		= pOptions->m_bTournAllowAutoAim;
	m_nTournPointsPerMatch		= pOptions->m_nTournPointsPerMatch;
	m_nTournMatchTimeLimit		= pOptions->m_nTournMatchTimeLimit;
	m_nTournTeamsToStart		= pOptions->m_nTournTeamsToStart;
	m_nTournMode				= (uint8)pOptions->m_nTournMode;
	m_nTournGamesPerMap			= pOptions->m_nTournGamesPerMap;
	m_nTournPreGameDelay		= pOptions->m_nTournPreGameDelay;
	m_nTournMaxPing				= pOptions->m_nTournMaxPing;
	m_nTournAirControl			= pOptions->m_nTournAirControl;

	m_nLCTeams					= pOptions->m_nLCMaxPlayers / pOptions->m_nLCMaxTeamSize;
	m_nLCTeamSize				= pOptions->m_nLCMaxTeamSize;
	m_nLCTrailLength			= (uint8)pOptions->m_nLCTrailLength;
	m_nLCGameSpeed				= (uint8)pOptions->m_nLCGameSpeed;
	m_nLCAllowedCycleType		= (uint8)pOptions->m_nLCAllowedCycleType;
	m_nLCWaitForPlayersTime		= pOptions->m_nLCWaitForPlayersTime;
	m_nLCTotalRaces				= pOptions->m_nLCTotalRaces;
	m_nLCTotalPoints			= pOptions->m_nLCTotalPoints;
	m_nLCTotalTime				= pOptions->m_nLCTotalTime;
	m_nLCMaxPing				= pOptions->m_nLCMaxPing;
	m_bLCShield					= pOptions->m_nLCPowerups & 1;
	m_bLCTurbo					= (pOptions->m_nLCPowerups >> 1) & 1;
	m_bLCTrailSpike				= (pOptions->m_nLCPowerups >> 2) & 1;
	m_bLCMissile				= (pOptions->m_nLCPowerups >> 3) & 1;
	m_bLCTurboCurse				= (pOptions->m_nLCPowerups >> 5) & 1;
	m_bLCGrab					= (pOptions->m_nLCPowerups >> 6) & 1;
	m_bLCTrailReset				= (pOptions->m_nLCPowerups >> 7) & 1;
	m_bLCForceUse				= (pOptions->m_nLCPowerups >> 8) & 1;
	m_bLCWallExtender			= (pOptions->m_nLCPowerups >> 4) & 1;
	m_nLCKillPointValue			= pOptions->m_nLCKillPointValue;
	m_nLCSelfKillPointValue		= -pOptions->m_nLCSelfKillPointValue;
	m_nLCWinRacePointValue		= pOptions->m_nLCWinRacePointValue;

	UpdateGameType();
	UpdateData(LTFALSE);
}

// RETAIL: 0x10060780
void CScreenHostGame::SaveOptions()
{
	UpdateData(LTTRUE);

	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();

	pOptions->m_eGameType				= m_nGameType + eGameTypeDiscArena;

	pOptions->m_nArenaMaxPlayers		= m_nArenaTeams * m_nArenaTeamSize;
	pOptions->m_nArenaMaxTeamSize		= m_nArenaTeamSize;
	pOptions->m_bArenaFriendlyFire		= !!m_bArenaFriendlyFire;
	pOptions->m_bArenaDiscsInSpectator	= !!m_bArenaDiscsInSpectator;
	pOptions->m_bArenaAllowAutoAim		= !!m_bArenaAllowAutoAim;
	pOptions->m_nArenaPointsPerMatch	= m_nArenaPointsPerMatch;
	pOptions->m_nArenaMatchTimeLimit	= m_nArenaMatchTimeLimit;
	pOptions->m_nArenaWinLimit			= m_nArenaWinLimit;
	pOptions->m_nArenaTimeLimit			= m_nArenaTimeLimit;
	pOptions->m_nArenaGamesPerMap		= m_nArenaGamesPerMap;
	pOptions->m_nArenaPreGameDelay		= m_nArenaPreGameDelay;
	pOptions->m_nArenaMaxPing			= m_nArenaMaxPing;
	pOptions->m_nArenaAirControl		= m_nArenaAirControl;

	pOptions->m_nTournMaxPlayers		= m_nTournTeams * m_nTournTeamSize;
	pOptions->m_nTournMaxTeamSize		= m_nTournTeamSize;
	pOptions->m_bTournFriendlyFire		= !!m_bTournFriendlyFire;
	pOptions->m_bTournDiscsInSpectator	= !!m_bTournDiscsInSpectator;
	pOptions->m_bTournAllowAutoAim		= !!m_bTournAllowAutoAim;
	pOptions->m_nTournPointsPerMatch	= m_nTournPointsPerMatch;
	pOptions->m_nTournMatchTimeLimit	= m_nTournMatchTimeLimit;
	pOptions->m_nTournTeamsToStart		= m_nTournTeamsToStart;
	pOptions->m_nTournMode				= m_nTournMode;
	pOptions->m_nTournGamesPerMap		= m_nTournGamesPerMap;
	pOptions->m_nTournPreGameDelay		= m_nTournPreGameDelay;
	pOptions->m_nTournMaxPing			= m_nTournMaxPing;
	pOptions->m_nTournAirControl		= m_nTournAirControl;

	pOptions->m_nLCMaxPlayers			= m_nLCTeams * m_nLCTeamSize;
	pOptions->m_nLCMaxTeamSize			= m_nLCTeamSize;
	pOptions->m_nLCTrailLength			= m_nLCTrailLength;
	pOptions->m_nLCGameSpeed			= m_nLCGameSpeed;
	pOptions->m_nLCAllowedCycleType		= m_nLCAllowedCycleType;
	pOptions->m_nLCWaitForPlayersTime	= m_nLCWaitForPlayersTime;
	pOptions->m_nLCTotalRaces			= m_nLCTotalRaces;
	pOptions->m_nLCTotalPoints			= m_nLCTotalPoints;
	pOptions->m_nLCTotalTime			= m_nLCTotalTime;
	pOptions->m_nLCMaxPing				= m_nLCMaxPing;
	pOptions->m_nLCPowerups = 0;
	if (m_bLCShield)		pOptions->m_nLCPowerups |= 0x001;
	if (m_bLCTurbo)			pOptions->m_nLCPowerups |= 0x002;
	if (m_bLCTrailSpike)	pOptions->m_nLCPowerups |= 0x004;
	if (m_bLCMissile)		pOptions->m_nLCPowerups |= 0x008;
	if (m_bLCTurboCurse)	pOptions->m_nLCPowerups |= 0x020;
	if (m_bLCGrab)			pOptions->m_nLCPowerups |= 0x040;
	if (m_bLCTrailReset)	pOptions->m_nLCPowerups |= 0x080;
	if (m_bLCForceUse)		pOptions->m_nLCPowerups |= 0x100;
	if (m_bLCWallExtender)	pOptions->m_nLCPowerups |= 0x010;
	pOptions->m_nLCKillPointValue		= m_nLCKillPointValue;
	pOptions->m_nLCSelfKillPointValue	= -m_nLCSelfKillPointValue;
	pOptions->m_nLCWinRacePointValue	= m_nLCWinRacePointValue;

	g_pServerOptionMgr->SaveCurrent();
}

// RETAIL: 0x10060a60
void CScreenHostGame::ShowPowerups(bool bShow)
{
	m_pPowerupList->Show(bShow);
	if (bShow)
	{
		AddTronUnknownControl(m_pPowerupList);
		AddTronUnknownControl(m_pPowerupsDone);
		SetSelection(GetIndex(m_pPowerupList));
	}
	else
	{
		ClearTronUnknownControls();
		SetSelection(kNoSelection);
	}
	m_pPowerupsDone->Show(bShow);
}

// RETAIL: 0x10060af0
bool CScreenHostGame::CheckTeamSize(CLTGUISlider *pSlider, int nNewPos)
{
	CLTGUISlider *pOther;
	int nMax = kMaxDiscPlayers;

	if (pSlider == m_pArenaTeams)
		pOther = m_pArenaTeamSize;
	else if (pSlider == m_pArenaTeamSize)
		pOther = m_pArenaTeams;
	else if (pSlider == m_pTournTeams)
		pOther = m_pTournTeamSize;
	else if (pSlider == m_pTournTeamSize)
		pOther = m_pTournTeams;
	else if (pSlider == m_pLCTeams)
	{
		pOther = m_pLCTeamSize;
		nMax = kMaxLCPlayers;
	}
	else if (pSlider == m_pLCTeamSize)
	{
		pOther = m_pLCTeams;
		nMax = kMaxLCPlayers;
	}
	else
		return false;

	if (pOther->GetSliderPos() * nNewPos > nMax)
	{
		pOther->SetSliderPos(nMax / nNewPos);
		pOther->UpdateData(LTTRUE);
	}
	return true;
}

// RETAIL: 0x100618b0
void CScreenHostGame::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	HandleEditResult(dwParam1,dwParam2);
}
