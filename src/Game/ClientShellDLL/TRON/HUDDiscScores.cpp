// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDiscScores.cpp
//
// PURPOSE : HUDItem showing the disc games' scores
//
// RETAIL: Tron 2.0's, written from retail CShell.dll 0x10027370..
// 0x10028d70 (HUDDiscScores.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "HUDDiscScores.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "SharedScoring.h"
#include "DiscMatchMgr.h"
#include "ClientUtilities.h"

extern CGameClientShell* g_pGameClientShell;

// RETAIL: set by CTronHUDMgr to its scores (0x1009b9f0); the name is ours
CHUDDiscScores*		g_pDiscScores = LTNULL;

// RETAIL: Tron's string ids (retail pushes them; the names are ours)
#define IDS_TRON_DISCSCORES_PLAYING			3306
#define IDS_TRON_DISCSCORES_WAITMATCH		3307
#define IDS_TRON_DISCSCORES_WAITTOURNEY		3308
#define IDS_TRON_DISCSCORES_WAITROUND		3317
#define IDS_TRON_DISCSCORES_TEAM			3272
#define IDS_TRON_DISCSCORES_TEAMSCORE		3273
#define IDS_TRON_DISCSCORES_NAME			386
#define IDS_TRON_DISCSCORES_FRAGS			3263
#define IDS_TRON_DISCSCORES_SCORE			693
#define IDS_TRON_DISCSCORES_STATUS			3274
#define IDS_TRON_DISCSCORES_PING			387

// RETAIL: a player's frags and client id, sorted by both (ours)
typedef std::pair<int, uint32> DiscScore;
typedef std::list<DiscScore> DiscScoreList;

CHUDDiscScores::CHUDDiscScores()
{
	m_UpdateFlags = 0x8000000;	// RETAIL: Tron's
	m_eLevel = kHUDRenderNone;
	m_fScale = 0.0f;
	m_bShow = false;
	m_bScreen = false;
	m_hPlaying = LTNULL;
	m_hWaitingForMatch = LTNULL;
	m_hWaitingForTournament = LTNULL;
	m_bInited = false;
}

CHUDDiscScores::~CHUDDiscScores()
{
}

LTBOOL CHUDDiscScores::Init()
{
	m_bControlsInited = false;

	m_hPlaying = g_pInterfaceResMgr->GetTexture("interface\\hud\\playing_game.dtx");
	m_hWaitingForMatch = g_pInterfaceResMgr->GetTexture("interface\\hud\\waiting_for_match.dtx");
	m_hWaitingForTournament = g_pInterfaceResMgr->GetTexture("interface\\hud\\waiting_for_tournament.dtx");

	m_bHasPlayers = false;
	m_eGameType = g_knNumGameTypes;

	UpdateLayout();

	m_bInited = true;
	return LTTRUE;
}

void CHUDDiscScores::Term()
{
	if (!m_bInited) return;

	if (m_bControlsInited)
	{
		m_Header.Destroy();
		m_HeaderFrame.Destroy();
		m_Frame.Destroy();
		m_Legends[0].Destroy();
		m_Legends[1].Destroy();
		m_Legends[2].Destroy();

		for (int i = 0; i < kMaxRows; i++)
			m_Rows[i].Destroy();

		m_bControlsInited = false;
	}

	m_bInited = false;
}

// RETAIL: 0x10027680
void CHUDDiscScores::Show(bool bShow, bool bScreen)
{
	if (m_bInited)
	{
		m_bShow = bShow;
		m_bScreen = bScreen;
		Update();
	}
}

void CHUDDiscScores::Render()
{
	if (!IsDiscGameType()) return;
	if (!m_bEnabled) return;

	m_bShow = (g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_PROGRESS) && !g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_MATCHES));

	if (m_bShow)
	{
		if (m_bScreen)
			m_bScreen = false;
	}
	m_bShow = (m_bShow || m_bScreen);

	// RETAIL: the crosshair is hidden while the scores are drawn
	g_pCrosshair->Enable(!m_bShow);

	if (!m_bShow) return;

	if (m_bControlsInited && m_bHasPlayers)
	{
		m_Frame.Render();
		m_HeaderFrame.Render();
		m_Header.Render();

		for (int i = 0; i < kMaxRows; i++)
			m_Rows[i].Render();
	}

	SetRenderState();

	g_pDrawPrim->SetTexture(m_hPlaying);
	g_pDrawPrim->DrawPrim(&m_LegendIcons[0], 1);
	g_pDrawPrim->SetTexture(m_hWaitingForMatch);
	g_pDrawPrim->DrawPrim(&m_LegendIcons[1], 1);
	g_pDrawPrim->SetTexture(m_hWaitingForTournament);
	g_pDrawPrim->DrawPrim(&m_LegendIcons[2], 1);

	m_Legends[0].Render();
	m_Legends[1].Render();
	m_Legends[2].Render();
}

void CHUDDiscScores::Update()
{
	if (!IsDiscGameType()) return;

	if (!m_bInited)
		Init();

	m_bHasPlayers = false;

	GameType eGameType = g_pGameClientShell->GetGameType();
	GameType eOldGameType = m_eGameType;
	m_eGameType = eGameType;
	if (eGameType != eOldGameType)
		UpdateLayout();

	float fScale = g_pInterfaceResMgr->GetXRatio();
	if (fScale != m_fScale)
	{
		m_fScale = fScale;
		m_Frame.SetScale(fScale);
		m_HeaderFrame.SetScale(fScale);
		m_Header.SetScale(fScale);

		for (int i = 0; i < kMaxRows; i++)
			m_Rows[i].SetScale(fScale);
	}

	CClientInfoMgr* pCIMgr = g_pGameClientShell->GetInterfaceMgr()->GetClientInfoMgr();
	if (!pCIMgr) return;

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	if (!pTeamMgr) return;

	std::wstring sText;
	char szTmp[64] = "";
	int nRow = 0;
	uint16 nHeight = m_Header.GetHeight();

	// RETAIL: read and not used
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	int nNumTeams = pTeamMgr->GetNumTeams();
	for (int nTeam = 0; nTeam < nNumTeams; nTeam++)
	{
		CTronTeam* pTeam = pTeamMgr->GetTeam(nTeam, true);
		int nNumClients = pTeam->GetNumClients();
		if (!nNumClients) continue;

		m_bHasPlayers = true;

		DiscScoreList Scores;
		int i;
		for (i = 0; i < nNumClients; i++)
		{
			uint32 nID = pTeam->GetClient(i);
			CLIENT_INFO* pCI = pCIMgr->GetClientByID(nID, true);
			if (pCI)
				Scores.push_back(std::make_pair(pCI->sScore.GetFrags(), nID));
		}

		Scores.sort();

		std::string sName;
		if (pTeam->GetTeamSize() > 1)
			pTeam->GetName(false, sName, false);
		else
			sName = "  -";

		// the team's name and score on its first player's line
		LTGUIWiden(sText, sName.c_str());
		CLTGUIColumnCtrl* pTeamRow = &m_Rows[nRow];
		pTeamRow->GetPolyString(0)->SetText(sText);

		char szScore[128];
		sprintf(szScore, "%d / %d", pTeam->GetTronUnknownC(), g_pDiscMatchMgr->GetTronUnknown8());
		LTGUIWiden(sText, szScore);
		pTeamRow->GetPolyString(1)->SetText(sText);

		for (DiscScoreList::reverse_iterator iter = Scores.rbegin(); iter != Scores.rend(); ++iter)
		{
			m_Rows[nRow].AddIconColumn(LTNULL, 0);

			CLIENT_INFO* pCI = pCIMgr->GetClientByID(iter->second, true);
			if (pCI)
			{
				if (iter != Scores.rbegin())
				{
					m_Rows[nRow].GetPolyString(0)->SetText(L" ");
					m_Rows[nRow].GetPolyString(1)->SetText(L" ");
				}

				LTGUIWiden(sText, pCI->sName.c_str());
				m_Rows[nRow].GetPolyString(2)->SetText(sText);

				sprintf(szTmp, "%d", pCI->sScore.GetFrags());
				LTGUIWiden(sText, szTmp);
				m_Rows[nRow].GetPolyString(3)->SetText(sText);

				sprintf(szTmp, "%d", pCI->sScore.GetScore());
				LTGUIWiden(sText, szTmp);
				m_Rows[nRow].GetPolyString(4)->SetText(sText);

				// RETAIL: the player's state's icon
				HTEXTURE hIcon = LTNULL;
				switch (pCI->sScore.GetTronUnknown18())
				{
					case 0:
					case 3:
						hIcon = m_hPlaying;
						break;
					case 1:
						hIcon = m_hWaitingForMatch;
						break;
					case 2:
						hIcon = m_hWaitingForTournament;
						break;
				}
				if (hIcon)
					m_Rows[nRow].AddIconColumn(hIcon, (uint16)(g_pInterfaceResMgr->GetXRatio() * 465.0f));

				sprintf(szTmp, "%d", pCI->nPing);
				LTGUIWiden(sText, szTmp);
				m_Rows[nRow].GetPolyString(6)->SetText(sText);

				uint32 nColor = (pCI->sScore.GetTronUnknown18() == 0) ? m_nTextColor : m_nElimTextColor;
				m_Rows[nRow].SetColors(nColor, nColor, nColor);
			}

			m_Rows[nRow].Show(LTTRUE);
			nHeight += m_Rows[nRow].GetHeight();
			nRow++;
		}

		if (pTeam->GetState() == CTronTeam::kTronTeamState_TronUnknown0)
		{
			pTeamRow->GetColumn(0)->SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
			pTeamRow->GetColumn(1)->SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
		}
		else
		{
			pTeamRow->GetColumn(0)->SetColors(m_nElimTextColor, m_nElimTextColor, m_nElimTextColor);
			pTeamRow->GetColumn(1)->SetColors(m_nElimTextColor, m_nElimTextColor, m_nElimTextColor);
		}
	}

	m_Frame.SetSize(m_nFrameWidth, (uint16)((float)nHeight / fScale) + 32);

	for (; nRow < kMaxRows; nRow++)
		m_Rows[nRow].Show(LTFALSE);
}

void CHUDDiscScores::UpdateLayout()
{
	char *pTag = "DiscScores";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag, "BasePos");
	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag, "Font", 0);
	m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(pTag, "FontSize", 0);

	LTVector vCol = g_pLayoutMgr->GetVector(pTag, "TextColor");
	m_nTextColor = SET_ARGB(0xFF, (uint8)vCol.x, (uint8)vCol.y, (uint8)vCol.z);

	vCol = g_pLayoutMgr->GetVector(pTag, "ElimTextColor");
	m_nElimTextColor = SET_ARGB(0xFF, (uint8)vCol.x, (uint8)vCol.y, (uint8)vCol.z);

	vCol = g_pLayoutMgr->GetVector(pTag, "FrameColor");
	uint8 nR, nG, nB, nA;
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	nA = (uint8)(g_pLayoutMgr->GetFloat(pTag, "FrameAlpha", 0.0f) * 255.0f);
	uint32 argbFrameColor = SET_ARGB(nA, nR, nG, nB);

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_nFrameWidth = 581;

	LTIntPt pos = m_BasePos;

	if (!m_bControlsInited)
	{
		// RETAIL: retail's attribute names are spelt so
		vCol = g_pLayoutMgr->GetVector(pTag, "BackroundFrameColor");
		nR = (uint8)vCol.x;
		nG = (uint8)vCol.y;
		nB = (uint8)vCol.z;
		nA = (uint8)(g_pLayoutMgr->GetFloat(pTag, "BackroundFrameAlpha", 0.0f) * 255.0f);
		uint32 argbBackColor = SET_ARGB(nA, nR, nG, nB);

		vCol = g_pLayoutMgr->GetVector(pTag, "BackroundBorderFrameColor");
		nR = (uint8)vCol.x;
		nG = (uint8)vCol.y;
		nB = (uint8)vCol.z;
		nA = (uint8)(g_pLayoutMgr->GetFloat(pTag, "BackroundBorderFrameAlpha", 0.0f) * 255.0f);
		uint32 argbBorderColor = SET_ARGB(nA, nR, nG, nB);

		int nBorderSize = g_pLayoutMgr->GetInt(pTag, "BackroundBorderFrameSize", 0);

		m_Frame.Create(argbBackColor, m_nFrameWidth, 32);
		m_Frame.SetBorder(nBorderSize, argbBorderColor);

		m_HeaderFrame.Create(argbFrameColor, m_nFrameWidth, 32);

		m_Header.Create(0, 0, pFont, m_nBaseFontSize + 2, LTNULL, 0, 0);

		for (int i = 0; i < kMaxRows; i++)
			m_Rows[i].Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);

		m_Legends[0].Create(LoadTempWString(IDS_TRON_DISCSCORES_PLAYING), 0, 0, pFont, 20, LTNULL, 0, 0);
		m_Legends[1].Create(LoadTempWString(IDS_TRON_DISCSCORES_WAITMATCH), 0, 0, pFont, 20, LTNULL, 0, 0);
		if (g_pGameClientShell->GetGameType() == eGameTypeDiscTournament)
			m_Legends[2].Create(LoadTempWString(IDS_TRON_DISCSCORES_WAITTOURNEY), 0, 0, pFont, 20, LTNULL, 0, 0);
		else
			m_Legends[2].Create(LoadTempWString(IDS_TRON_DISCSCORES_WAITROUND), 0, 0, pFont, 20, LTNULL, 0, 0);

		m_Legends[0].SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
		m_Legends[1].SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
		m_Legends[2].SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

		m_bControlsInited = true;
	}

	m_Frame.SetBasePos(LTIntPt(pos.x - 8, pos.y));

	m_Header.RemoveAllColumns();
	m_Header.SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
	m_Header.SetBasePos(pos);
	m_Header.AddColumn(LoadTempWString(IDS_TRON_DISCSCORES_TEAM), 100, LTFALSE);
	m_Header.AddColumn(LoadTempWString(IDS_TRON_DISCSCORES_TEAMSCORE), 90, LTFALSE);
	m_Header.AddColumn(LoadTempWString(IDS_TRON_DISCSCORES_NAME), 145, LTFALSE);
	m_Header.AddColumn(LoadTempWString(IDS_TRON_DISCSCORES_FRAGS), 70, LTFALSE);
	m_Header.AddColumn(LoadTempWString(IDS_TRON_DISCSCORES_SCORE), 60, LTFALSE);
	m_Header.AddColumn(LoadTempWString(IDS_TRON_DISCSCORES_STATUS), 50, LTFALSE);
	m_Header.AddColumn(LoadTempWString(IDS_TRON_DISCSCORES_PING), 50, LTFALSE);

	m_HeaderFrame.SetBasePos(LTIntPt(pos.x - 6, pos.y + 2));
	m_HeaderFrame.SetSize(m_nFrameWidth - 4, m_Header.GetHeight() - 2);

	pos.y += m_nBaseFontSize * 2 + 8;

	int i;
	for (i = 0; i < kMaxRows; i++)
	{
		m_Rows[i].RemoveAllColumns();
		m_Rows[i].SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
		m_Rows[i].SetBasePos(pos);
		m_Rows[i].AddColumn(std::wstring(L" "), 100, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L" "), 90, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L" "), 145, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L" "), 70, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L" "), 60, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L" "), 50, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L" "), 50, LTFALSE);
		m_Rows[i].Show(LTFALSE);

		pos.y += m_nBaseFontSize;
	}

	// RETAIL: the legend, its icons left of its texts
	float fScale = g_pInterfaceResMgr->GetXRatio();
	pos.x = m_BasePos.x;
	pos.y = 400;
	float fX = (float)pos.x * fScale - 24.0f;
	for (i = 0; i < kNumLegends; i++)
	{
		g_pDrawPrim->SetRGBA(&m_LegendIcons[i], 0xFFFFFFFF);
		g_pDrawPrim->SetUVWH(&m_LegendIcons[i], 0.0f, 0.0f, 1.0f, 1.0f);

		m_Legends[i].SetBasePos(pos);
		m_Legends[i].SetScale(fScale);

		g_pDrawPrim->SetXYWH(&m_LegendIcons[i], fX, (float)(pos.y + 2) * fScale, 16.0f, 16.0f);

		pos.y += 20;
	}
}

// RETAIL: 0x10028d50
bool CHUDDiscScores::IsVisible()
{
	if (m_bEnabled && m_bShow)
		return true;
	return false;
}
