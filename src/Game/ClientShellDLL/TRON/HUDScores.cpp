// ----------------------------------------------------------------------- //
//
// MODULE  : HUDScores.cpp
//
// PURPOSE : Implementation of CHUDScores to display the light cycle game's
//			 scores
//
// RETAIL: Tron 2.0's, written from retail CShell.dll 0x10034800..
// 0x1003a620 (HUDScores.h). Build 69's (NOLF2's) is gone: retail's lays
// out a header (the round, the time left, the round's figure), the
// players' or the teams' scores and a footer (the light cycle match
// manager's figures) in column controls on frames.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDScores.h"
#include "TronHUDMgr.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientInfoMgr.h"
#include "SharedScoring.h"
#include "LightCycleMatchMgr.h"
#include "ClientUtilities.h"
#include <list>

extern CGameClientShell* g_pGameClientShell;

// RETAIL: Tron's string ids of the column titles (retail pushes them; the
// names are ours)
#define IDS_TRON_SCORES_COLUMN1		695
#define IDS_TRON_SCORES_COLUMN2		696
#define IDS_TRON_SCORES_COLUMN3		697
#define IDS_TRON_SCORES_COLUMN4		699
#define IDS_TRON_SCORES_COLUMN5		387
#define IDS_TRON_SCORES_HEADER1		700
#define IDS_TRON_SCORES_HEADER2		710
#define IDS_TRON_SCORES_HEADER3		711
#define IDS_TRON_SCORES_FOOTER1		712
#define IDS_TRON_SCORES_FOOTER2		713
#define IDS_TRON_SCORES_FOOTER3		714

// RETAIL: the layout's section, a pointer of the TU's (0x101e70ec)
static char* const s_szScoresTag = "MultiplayerLightCycleScores";

// RETAIL: out of line (0x10034800); the name is ours
static LTIntPt AddPoints(const LTIntPt& a, const LTIntPt& b)
{
	LTIntPt pt = a;
	pt.y += b.y;
	pt.x += b.x;
	return pt;
}

// RETAIL: a team's players, ordered by their score's +0x2 figure and
// their client ids
typedef std::list< std::pair<int, uint32> > HUDScoreSortList;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildTeamList
//
//	PURPOSE:	The teams with players, each with its players sorted and
//				their totals. RETAIL: 0x10034830; the name is ours
//
// ----------------------------------------------------------------------- //

static void BuildTeamList(HUDScoreTeamArray* pTeams, CTronTeamMgr* pTeamMgr, CClientInfoMgr* pClientInfo)
{
	if (!pTeams || !pTeamMgr || !pClientInfo) return;

	int nNumTeams = pTeamMgr->GetNumTeams();
	pTeams->reserve(nNumTeams);

	for (int i = 0; i < nNumTeams; i++)
	{
		HUDScoreTeam team;

		CTronTeam* pTeam = pTeamMgr->GetTeam(i, true);
		if (!pTeam->GetNumClients()) continue;

		pTeam->GetName(false, team.sName, GetGameType() == eGameTypeMultiplayerLightcycles);

		team.nTronUnknownC = 0;
		team.nTronUnknown10 = 0;
		team.nTronUnknown14 = 0;

		int nFlagged = 0;
		int nNumClients = pTeam->GetNumClients();

		HUDScoreSortList sorted;
		for (int j = 0; j < nNumClients; j++)
		{
			uint32 nClientID = pTeam->GetClient(j);
			CLIENT_INFO* pInfo = pClientInfo->GetClientByID(nClientID, true);
			if (!pInfo) continue;

			sorted.push_back(std::make_pair(pInfo->sScore.GetTronUnknown2(), nClientID));
		}

		sorted.sort();

		for (HUDScoreSortList::reverse_iterator iter = sorted.rbegin(); iter != sorted.rend(); ++iter)
		{
			CLIENT_INFO* pInfo = pClientInfo->GetClientByID((*iter).second, true);
			if (!pInfo) continue;

			HUDScorePlayer player;
			player.sName = pInfo->sName;

			player.nTronUnknownC = pInfo->sScore.GetTronUnknown0();
			team.nTronUnknownC += player.nTronUnknownC;
			player.nTronUnknown10 = pInfo->sScore.GetTronUnknown2();
			team.nTronUnknown10 += player.nTronUnknown10;
			player.nTronUnknown14 = pInfo->sScore.GetTronUnknownC();
			team.nTronUnknown14 += player.nTronUnknown14;
			player.bTronUnknown18 = pInfo->sScore.GetTronUnknownE();
			if (player.bTronUnknown18)
				nFlagged++;

			player.nPing = pInfo->nPing;

			team.Players.push_back(player);
		}

		if (!team.bTronUnknown2C && nFlagged && nFlagged == nNumClients)
			team.bTronUnknown2C = true;

		pTeams->push_back(team);
	}
}

// ----------------------------------------------------------------------- //
//	CHUDScores
// ----------------------------------------------------------------------- //

CHUDScores::CHUDScores()
{
	m_UpdateFlags = 0x01000000;	// RETAIL: Tron's (not NOLF2's kHUDScores)
	m_eLevel = kHUDRenderNone;

	m_bInited = false;
	m_fScale = 0.0f;
}

CHUDScores::~CHUDScores()
{
}

LTBOOL CHUDScores::Init()
{
	m_bControlsInited = false;

	char* pTag = s_szScoresTag;

	int nScoreColumn1 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn1", 0);
	int nScoreColumn2 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn2", 0);
	int nScoreColumn3 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn3", 0);
	int nScoreColumn4 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn4", 0);
	int nScoreColumn5 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn5", 0);
	int nInfoColumn1 = g_pLayoutMgr->GetInt(pTag, "InfoColumn1", 0);
	int nInfoColumn2 = g_pLayoutMgr->GetInt(pTag, "InfoColumn2", 0);

	int nScoreWidth = nScoreColumn1 + nScoreColumn2 + nScoreColumn3 + nScoreColumn4 + nScoreColumn5;

	int nInfoColumn3 = g_pLayoutMgr->GetInt(pTag, "InfoColumn3", 0);
	int nInfoWidth = nInfoColumn1 + nInfoColumn2 + nInfoColumn3;

	m_nFrameWidth = (nScoreWidth > nInfoWidth) ? nScoreWidth : nInfoWidth;

	m_fScale = 0.0f;
	m_bDraw = false;
	m_bScreen = false;
	m_fDrawTime = 0.0f;

	UpdateLayout();

	m_bInited = true;
	return LTTRUE;
}

void CHUDScores::Term()
{
	if (!m_bInited || m_bControlsInited) return;

	m_ScoreTitles.Destroy();
	m_ScoreFrame.Destroy();
	m_HeaderFrame.Destroy();
	m_FooterFrame.Destroy();
}

void CHUDScores::Show(bool bDraw, bool bScreen)
{
	m_bDraw = bDraw;
	m_bScreen = bScreen;
	m_fDrawTime = 0.0f;

	Update();
}

void CHUDScores::Render()
{
	if (!IsMultiplayerGame()) return;
	if (!IsEnabled()) return;

	m_bDraw = (bool)g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_PROGRESS);

	if (m_fDrawTime >= 0.0f)
		m_fDrawTime -= g_pLTClient->GetFrameTime();

	if (m_bDraw && m_bScreen && m_fDrawTime < 0.0f)
		m_bScreen = false;

	m_bDraw = (m_bDraw || m_bScreen);

	// RETAIL: the crosshair is hidden while the scores are drawn
	g_pCrosshair->Enable(!m_bDraw);

	if (!m_bDraw || !m_bControlsInited) return;

	Update();

	if (g_pLightCycleMatchMgr->GetTimeLeft() > 0.0f)
	{
		std::wstring sTime;
		char szTime[256];

		int nTime = (int)g_pLightCycleMatchMgr->GetTimeLeft();
		if (nTime < 0)
			nTime = 0;

		int nSeconds = nTime % 60;
		int nMinutes = (nTime - nSeconds) / 60;
		sprintf(szTime, "%d:%02d", nMinutes, nSeconds);
		LTGUIWiden(sTime, szTime);

		m_HeaderValues.GetColumn(1)->SetString(sTime);
	}
	else
	{
		m_HeaderValues.GetColumn(1)->SetString(L"--:--");
	}

	m_Frame.Render();
	m_ScoreFrame.Render();
	m_ScoreTitles.Render();
	m_HeaderFrame.Render();
	m_HeaderTitles.Render();
	m_HeaderValues.Render();
	m_FooterFrame.Render();
	m_FooterTitles.Render();
	m_FooterValues.Render();

	for (int i = 0; i < kMaxRows; i++)
	{
		m_Scores[i].Show(LTTRUE);
		m_Scores[i].Render();
	}
}

void CHUDScores::Update()
{
	float fScale = g_pInterfaceResMgr->GetXRatio();
	if (fScale != m_fScale)
	{
		m_fScale = fScale;

		m_Frame.SetScale(fScale);
		m_ScoreTitles.SetScale(fScale);
		m_ScoreFrame.SetScale(fScale);
		m_HeaderFrame.SetScale(fScale);
		m_HeaderTitles.SetScale(fScale);
		m_HeaderValues.SetScale(fScale);
		m_FooterFrame.SetScale(fScale);
		m_FooterTitles.SetScale(fScale);
		m_FooterValues.SetScale(fScale);

		for (int i = 0; i < kMaxRows; i++)
			m_Scores[i].SetScale(fScale);
	}

	m_Frame.SetSize(m_nFrameWidth, 32);

	UpdateScores();
}

void CHUDScores::UpdateLayout()
{
	char* pTag = s_szScoresTag;

	m_BasePos = g_pLayoutMgr->GetPoint(pTag, "BasePos");
	m_BasePos.x = (640 - m_nFrameWidth) / 2;

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag, "Font", 0);
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(pTag, "FontSize", 0);

	LTVector vCol = g_pLayoutMgr->GetVector(pTag, "TextColor");
	m_nTextColor = SET_ARGB(0xFF, (uint8)vCol.x, (uint8)vCol.y, (uint8)vCol.z);

	vCol = g_pLayoutMgr->GetVector(pTag, "ElimTextColor");
	m_nElimTextColor = SET_ARGB(0xFF, (uint8)vCol.x, (uint8)vCol.y, (uint8)vCol.z);

	int nScoreColumn1 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn1", 0);
	int nScoreColumn2 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn2", 0);
	int nScoreColumn3 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn3", 0);
	int nScoreColumn4 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn4", 0);
	int nScoreColumn5 = g_pLayoutMgr->GetInt(pTag, "ScoreColumn5", 0);
	int nInfoColumn1 = g_pLayoutMgr->GetInt(pTag, "InfoColumn1", 0);
	int nInfoColumn2 = g_pLayoutMgr->GetInt(pTag, "InfoColumn2", 0);
	int nInfoColumn3 = g_pLayoutMgr->GetInt(pTag, "InfoColumn3", 0);

	if (!m_bControlsInited)
	{
		vCol = g_pLayoutMgr->GetVector(pTag, "FrameColor");
		float fAlpha = g_pLayoutMgr->GetFloat(pTag, "FrameAlpha", 0.0f);
		uint32 nFrameColor = SET_ARGB((uint8)(fAlpha * 255.0f), (uint8)vCol.x, (uint8)vCol.y, (uint8)vCol.z);

		// RETAIL: retail's attribute names are misspelt so
		vCol = g_pLayoutMgr->GetVector(pTag, "BackroundFrameColor");
		fAlpha = g_pLayoutMgr->GetFloat(pTag, "BackroundFrameAlpha", 0.0f);
		uint32 nBackColor = SET_ARGB((uint8)(fAlpha * 255.0f), (uint8)vCol.x, (uint8)vCol.y, (uint8)vCol.z);

		vCol = g_pLayoutMgr->GetVector(pTag, "BackroundBorderFrameColor");
		fAlpha = g_pLayoutMgr->GetFloat(pTag, "BackroundBorderFrameAlpha", 0.0f);
		uint32 nBorderColor = SET_ARGB((uint8)(fAlpha * 255.0f), (uint8)vCol.x, (uint8)vCol.y, (uint8)vCol.z);

		int nBorder = g_pLayoutMgr->GetInt(pTag, "BackroundBorderFrameSize", 0);

		m_Frame.Create(nFrameColor, m_nFrameWidth, 32);
		m_Frame.SetBorder((uint8)nBorder, nBorderColor);

		m_ScoreTitles.Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);
		m_ScoreFrame.Create(nBackColor, m_nFrameWidth, 32);
		m_HeaderFrame.Create(nBackColor, m_nFrameWidth, 32);
		m_HeaderTitles.Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);
		m_HeaderValues.Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);
		m_FooterFrame.Create(nBackColor, m_nFrameWidth, 32);
		m_FooterTitles.Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);
		m_FooterValues.Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);

		for (int i = 0; i < kMaxRows; i++)
			m_Scores[i].Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);

		m_bControlsInited = true;
	}

	m_nFrameHBorder = g_pLayoutMgr->GetInt(s_szScoresTag, "FrameHBorder", 0);
	m_nFrameVBorder = g_pLayoutMgr->GetInt(s_szScoresTag, "FrameVBorder", 0);

	LTIntPt pos = m_BasePos;
	pos.y += m_nFrameVBorder;

	m_ScoreTitles.RemoveAllColumns();
	m_ScoreTitles.SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
	m_ScoreTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_COLUMN1), nScoreColumn1, LTFALSE);
	m_ScoreTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_COLUMN2), nScoreColumn2, LTFALSE);
	m_ScoreTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_COLUMN3), nScoreColumn3, LTFALSE);
	m_ScoreTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_COLUMN4), nScoreColumn4, LTFALSE);
	m_ScoreTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_COLUMN5), nScoreColumn5, LTFALSE);

	int nY = pos.y + 2 * m_nBaseFontSize;

	m_Frame.SetBasePos(m_BasePos);

	for (int i = 0; i < kMaxRows; i++)
	{
		m_Scores[i].RemoveAllColumns();
		m_Scores[i].SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
		m_Scores[i].AddColumn(std::wstring(L" "), nScoreColumn1, LTFALSE);
		m_Scores[i].AddColumn(std::wstring(L" "), nScoreColumn2, LTFALSE);
		m_Scores[i].AddColumn(std::wstring(L" "), nScoreColumn3, LTFALSE);
		m_Scores[i].AddColumn(std::wstring(L" "), nScoreColumn4, LTFALSE);
		m_Scores[i].AddColumn(std::wstring(L" "), nScoreColumn5, LTFALSE);
		m_Scores[i].Show(LTFALSE);

		nY += m_nBaseFontSize;
	}

	m_HeaderTitles.RemoveAllColumns();
	m_HeaderTitles.SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
	m_HeaderTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_HEADER1), nInfoColumn1, LTTRUE);
	m_HeaderTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_HEADER2), nInfoColumn2, LTTRUE);
	m_HeaderTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_HEADER3), nInfoColumn3, LTTRUE);

	m_HeaderValues.RemoveAllColumns();
	m_HeaderValues.SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
	m_HeaderValues.AddColumn(std::wstring(L" "), nInfoColumn1, LTTRUE);
	m_HeaderValues.AddColumn(std::wstring(L" "), nInfoColumn2, LTTRUE);
	m_HeaderValues.AddColumn(std::wstring(L" "), nInfoColumn3, LTTRUE);

	m_FooterTitles.RemoveAllColumns();
	m_FooterTitles.SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
	m_FooterTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_FOOTER1), nInfoColumn1, LTTRUE);
	m_FooterTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_FOOTER2), nInfoColumn2, LTTRUE);
	m_FooterTitles.AddColumn(LoadTempWString(IDS_TRON_SCORES_FOOTER3), nInfoColumn3, LTTRUE);

	m_FooterValues.RemoveAllColumns();
	m_FooterValues.SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
	m_FooterValues.AddColumn(std::wstring(L" "), nInfoColumn1, LTTRUE);
	m_FooterValues.AddColumn(std::wstring(L" "), nInfoColumn2, LTTRUE);
	m_FooterValues.AddColumn(std::wstring(L" "), nInfoColumn3, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDScores::UpdateScores
//
//	PURPOSE:	The teams' scores for teams of more than one player, else
//				the players'
//
// ----------------------------------------------------------------------- //

void CHUDScores::UpdateScores()
{
	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	if (!pTeamMgr) return;

	if (pTeamMgr->GetTeamSize() != 1)
		UpdateTeamScores();
	else
		UpdatePlayerScores();
}

void CHUDScores::UpdateTeamScores()
{
	if (!m_bControlsInited) return;

	LTIntPt pos = m_BasePos;
	pos = UpdateHeader(pos);
	pos = UpdateTeams(pos);
	pos = UpdateFooter(pos);

	int nHeight = pos.y - m_BasePos.y;
	m_Frame.SetSize(m_nFrameWidth, nHeight);
}

void CHUDScores::UpdatePlayerScores()
{
	if (!m_bControlsInited) return;

	LTIntPt pos = m_BasePos;
	pos = UpdateHeader(pos);
	pos = UpdatePlayers(pos);
	pos = UpdateFooter(pos);

	int nHeight = pos.y - m_BasePos.y;
	m_Frame.SetSize(m_nFrameWidth, nHeight);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDScores::UpdatePlayers
//
//	PURPOSE:	A line for each team's first player
//
// ----------------------------------------------------------------------- //

LTIntPt CHUDScores::UpdatePlayers(const LTIntPt& pos)
{
	LTIntPt curPos = pos;

	m_ScoreTitles.SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));
	// RETAIL: by the header titles' height
	curPos.y += (int)((float)m_HeaderTitles.GetHeight() / g_pInterfaceResMgr->GetXRatio());

	CClientInfoMgr* pClientInfo = g_pGameClientShell->GetInterfaceMgr()->GetClientInfoMgr();
	CTronTeamMgr* pTeamMgr;
	if (!pClientInfo || !(pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr()))
		return curPos;

	std::wstring sText;
	int nNumTeams = pTeamMgr->GetNumTeams();

	// RETAIL: both read and not used
	int nVBorder = g_pLayoutMgr->GetInt(s_szScoresTag, "FrameVBorder", 0);
	uint16 nHeight = m_ScoreTitles.GetHeight();

	HUDScorePlayerArray players;
	players.reserve(nNumTeams);

	for (int i = 0; i < nNumTeams; i++)
	{
		HUDScorePlayer player;

		CTronTeam* pTeam = pTeamMgr->GetTeam(i, true);
		if (!pTeam->GetNumClients()) continue;

		CLIENT_INFO* pInfo = pClientInfo->GetClientByID(pTeam->GetClient(0), true);
		if (!pInfo) continue;

		pTeam->GetName(false, player.sName, GetGameType() == eGameTypeMultiplayerLightcycles);
		player.nTronUnknownC = pInfo->sScore.GetTronUnknown0();
		player.nTronUnknown10 = pInfo->sScore.GetTronUnknown2();
		player.nTronUnknown14 = pInfo->sScore.GetTronUnknownC();
		player.bTronUnknown18 = pInfo->sScore.GetTronUnknownE();
		player.nPing = pInfo->nPing;

		players.push_back(player);
	}

	char szTmp[64];
	int nRow = 0;
	for (HUDScorePlayerArray::iterator iter = players.begin(); iter != players.end(); iter++, nRow++)
	{
		CLTGUIColumnCtrl* pCtrl = &m_Scores[nRow];

		LTGUIWiden(sText, iter->sName.c_str());
		pCtrl->GetPolyString(0)->SetText(sText);

		sprintf(szTmp, "%d", iter->nTronUnknownC);
		LTGUIWiden(sText, szTmp);
		pCtrl->GetPolyString(1)->SetText(sText);

		sprintf(szTmp, "%d", iter->nTronUnknown10);
		LTGUIWiden(sText, szTmp);
		pCtrl->GetPolyString(2)->SetText(sText);

		sprintf(szTmp, "%d", iter->nTronUnknown14);
		LTGUIWiden(sText, szTmp);
		pCtrl->GetPolyString(3)->SetText(sText);

		sprintf(szTmp, "%d", iter->nPing);
		LTGUIWiden(sText, szTmp);
		pCtrl->GetPolyString(4)->SetText(sText);

		pCtrl->SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));
		curPos.y += (int)((float)pCtrl->GetHeight() / g_pInterfaceResMgr->GetXRatio());

		uint32 nColor = iter->bTronUnknown18 ? m_nElimTextColor : m_nTextColor;
		pCtrl->SetColors(nColor, nColor, nColor);
	}

	for (; nRow < kMaxRows; nRow++)
	{
		CLTGUIColumnCtrl* pCtrl = &m_Scores[nRow];
		pCtrl->GetPolyString(0)->SetText(std::wstring(L" "));
		pCtrl->GetPolyString(1)->SetText(std::wstring(L" "));
		pCtrl->GetPolyString(2)->SetText(std::wstring(L" "));
		pCtrl->GetPolyString(3)->SetText(std::wstring(L" "));
		pCtrl->GetPolyString(4)->SetText(std::wstring(L" "));
	}

	curPos.y += m_nBaseFontSize;

	m_ScoreFrame.SetBasePos(pos);
	m_ScoreFrame.SetSize(m_nFrameWidth, (int)((float)m_ScoreTitles.GetHeight() / g_pInterfaceResMgr->GetXRatio()));

	return curPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDScores::UpdateTeams
//
//	PURPOSE:	A line for each team, then one for each of its players
//
// ----------------------------------------------------------------------- //

LTIntPt CHUDScores::UpdateTeams(const LTIntPt& pos)
{
	LTIntPt curPos = pos;

	m_ScoreTitles.SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));
	// RETAIL: by the header titles' height
	curPos.y += (int)((float)m_HeaderTitles.GetHeight() / g_pInterfaceResMgr->GetXRatio());

	CClientInfoMgr* pClientInfo = g_pGameClientShell->GetInterfaceMgr()->GetClientInfoMgr();
	CTronTeamMgr* pTeamMgr;
	if (!pClientInfo || !(pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr()))
		return curPos;

	HUDScoreTeamArray teams;
	BuildTeamList(&teams, pTeamMgr, pClientInfo);

	int nRow = 0;
	std::wstring sText;
	char szTmp[32];

	for (HUDScoreTeamArray::iterator iter = teams.begin(); iter != teams.end(); iter++)
	{
		curPos.y += m_nBaseFontSize;

		LTGUIWiden(sText, iter->sName.c_str());
		CLTGUIColumnCtrl* pCtrl = &m_Scores[nRow];
		pCtrl->GetPolyString(0)->SetText(sText);

		sprintf(szTmp, "%d", iter->nTronUnknownC);
		LTGUIWiden(sText, szTmp);
		pCtrl->GetPolyString(1)->SetText(sText);

		sprintf(szTmp, "%d", iter->nTronUnknown10);
		LTGUIWiden(sText, szTmp);
		pCtrl->GetPolyString(2)->SetText(sText);

		sprintf(szTmp, "%d", iter->nTronUnknown14);
		LTGUIWiden(sText, szTmp);
		pCtrl->GetPolyString(3)->SetText(sText);

		pCtrl->SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));
		curPos.y += (int)((float)pCtrl->GetHeight() / g_pInterfaceResMgr->GetXRatio());

		LTGUIWiden(sText, "-");
		pCtrl->GetPolyString(4)->SetText(sText);

		if (iter->bTronUnknown2C)
			pCtrl->SetColors(m_nElimTextColor, m_nElimTextColor, m_nElimTextColor);
		else
			pCtrl->SetColors(m_nTextColor, m_nTextColor, m_nTextColor);

		nRow++;

		for (HUDScorePlayerArray::iterator pIter = iter->Players.begin(); pIter != iter->Players.end(); pIter++, nRow++)
		{
			pCtrl = &m_Scores[nRow];

			LTGUIWiden(sText, (std::string("     ") + pIter->sName).c_str());
			pCtrl->GetPolyString(0)->SetText(sText);

			sprintf(szTmp, "%d", pIter->nTronUnknownC);
			LTGUIWiden(sText, szTmp);
			pCtrl->GetPolyString(1)->SetText(sText);

			sprintf(szTmp, "%d", pIter->nTronUnknown10);
			LTGUIWiden(sText, szTmp);
			pCtrl->GetPolyString(2)->SetText(sText);

			sprintf(szTmp, "%d", pIter->nTronUnknown14);
			LTGUIWiden(sText, szTmp);
			pCtrl->GetPolyString(3)->SetText(sText);

			pCtrl->SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));
			curPos.y += (int)((float)pCtrl->GetHeight() / g_pInterfaceResMgr->GetXRatio());

			sprintf(szTmp, "%d", pIter->nPing);
			LTGUIWiden(sText, szTmp);
			pCtrl->GetPolyString(4)->SetText(sText);

			uint32 nColor = pIter->bTronUnknown18 ? m_nElimTextColor : m_nTextColor;
			pCtrl->SetColors(nColor, nColor, nColor);
		}
	}

	for (; nRow < kMaxRows; nRow++)
	{
		CLTGUIColumnCtrl* pCtrl = &m_Scores[nRow];
		pCtrl->GetPolyString(0)->SetText(std::wstring(L" "));
		pCtrl->GetPolyString(1)->SetText(std::wstring(L" "));
		pCtrl->GetPolyString(2)->SetText(std::wstring(L" "));
		pCtrl->GetPolyString(3)->SetText(std::wstring(L" "));
		pCtrl->GetPolyString(4)->SetText(std::wstring(L" "));
	}

	curPos.y += m_nBaseFontSize;

	m_ScoreFrame.SetBasePos(pos);
	m_ScoreFrame.SetSize(m_nFrameWidth, (int)((float)m_ScoreTitles.GetHeight() / g_pInterfaceResMgr->GetXRatio()));

	return curPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDScores::UpdateHeader
//
//	PURPOSE:	The round (its +0x19 of +0x18), the time left (Render's)
//				and the match manager's +0x1a
//
// ----------------------------------------------------------------------- //

LTIntPt CHUDScores::UpdateHeader(const LTIntPt& pos)
{
	LTIntPt curPos = pos;

	m_HeaderTitles.SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));
	curPos.y += (int)((float)m_HeaderTitles.GetHeight() / g_pInterfaceResMgr->GetXRatio());

	m_HeaderFrame.SetBasePos(pos);
	m_HeaderFrame.SetSize(m_nFrameWidth, (int)((float)m_HeaderTitles.GetHeight() / g_pInterfaceResMgr->GetXRatio()));

	std::wstring sText;

	if (g_pLightCycleMatchMgr->GetTronUnknown18())
	{
		char szTmp[64] = "";
		sprintf(szTmp, "%d/%d", g_pLightCycleMatchMgr->GetTronUnknown19(), g_pLightCycleMatchMgr->GetTronUnknown18());
		LTGUIWiden(sText, szTmp);
	}
	else
	{
		sText = L"--";
	}
	m_HeaderValues.GetColumn(0)->SetString(sText);

	if (g_pLightCycleMatchMgr->GetTronUnknown1A())
	{
		char szTmp[64] = "";
		sprintf(szTmp, "%d", g_pLightCycleMatchMgr->GetTronUnknown1A());
		LTGUIWiden(sText, szTmp);
	}
	else
	{
		sText = L"--";
	}
	m_HeaderValues.GetColumn(2)->SetString(sText);

	m_HeaderValues.SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));
	curPos.y += (int)((float)m_HeaderValues.GetHeight() / g_pInterfaceResMgr->GetXRatio()) + m_nBaseFontSize;

	return curPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDScores::UpdateFooter
//
//	PURPOSE:	The light cycle match manager's +0x1c, +0x1d (negated) and
//				+0x1e
//
// ----------------------------------------------------------------------- //

LTIntPt CHUDScores::UpdateFooter(const LTIntPt& pos)
{
	LTIntPt curPos = pos;
	std::wstring sText;

	m_FooterTitles.SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));
	curPos.y += (int)((float)m_FooterTitles.GetHeight() / g_pInterfaceResMgr->GetXRatio());

	m_FooterFrame.SetBasePos(pos);
	m_FooterFrame.SetSize(m_nFrameWidth, (int)((float)m_FooterTitles.GetHeight() / g_pInterfaceResMgr->GetXRatio()));

	m_FooterValues.SetBasePos(AddPoints(curPos, LTIntPt(m_nFrameHBorder, 0)));

	char szTmp[64] = "";

	sprintf(szTmp, "%d", (int8)g_pLightCycleMatchMgr->GetTronUnknown1C());
	LTGUIWiden(sText, szTmp);
	m_FooterValues.GetColumn(0)->SetString(sText);

	sprintf(szTmp, "%d", -(int8)g_pLightCycleMatchMgr->GetTronUnknown1D());
	LTGUIWiden(sText, szTmp);
	m_FooterValues.GetColumn(1)->SetString(sText);

	sprintf(szTmp, "%d", (int8)g_pLightCycleMatchMgr->GetTronUnknown1E());
	LTGUIWiden(sText, szTmp);
	m_FooterValues.GetColumn(2)->SetString(sText);

	curPos.y += (int)((float)m_FooterValues.GetHeight() / g_pInterfaceResMgr->GetXRatio()) + m_nBaseFontSize;

	return curPos;
}
