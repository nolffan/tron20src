// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDiscMatches.cpp
//
// PURPOSE : HUDItem listing the disc games' matches
//
// RETAIL: Tron 2.0's, written from retail CShell.dll 0x100257a0..
// 0x10027370 (HUDDiscMatches.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "HUDDiscMatches.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "SharedScoring.h"
#include "DiscMatchMgr.h"
#include "ClientUtilities.h"

extern CGameClientShell* g_pGameClientShell;

// RETAIL: set by CTronHUDMgr to its matches (0x1009b9fb)
CHUDDiscMatches*	g_pDiscMatches = LTNULL;

// RETAIL: Tron's string ids of the titles (retail pushes them; the names
// are ours)
#define IDS_TRON_DISCMATCHES_TIME		3279
#define IDS_TRON_DISCMATCHES_ROUND		3280
#define IDS_TRON_DISCMATCHES_MATCH		3281
#define IDS_TRON_DISCMATCHES_TIMELEFT	3278
#define IDS_TRON_DISCMATCHES_VERSUS		3282
#define IDS_TRON_DISCMATCHES_SCORE		3283
#define IDS_TRON_DISCMATCHES_VS			3303

static char* s_szDiscMatchesTag = "DiscMatches";

CHUDDiscMatches::CHUDDiscMatches()
{
	m_eLevel = kHUDRenderNone;
	m_fScale = 0.0f;
	m_bDraw = false;
	m_bScreen = false;
	m_hEmpty = LTNULL;
	m_hFull = LTNULL;
	m_bInited = false;
	m_UpdateFlags = 0x10000000;	// RETAIL: Tron's
}

CHUDDiscMatches::~CHUDDiscMatches()
{
}

LTBOOL CHUDDiscMatches::Init()
{
	m_bControlsInited = false;

	m_hEmpty = g_pInterfaceResMgr->GetTexture("Interface\\hud\\circle_empty.dtx");
	m_hFull = g_pInterfaceResMgr->GetTexture("Interface\\hud\\circle_full.dtx");

	int i, j;
	for (i = 0; i < kMaxMatches; i++)
	{
		for (j = 0; j < kMaxRounds * 2; j++)
		{
			g_pDrawPrim->SetUVWH(&m_Circles[i][j], 0.0f, 0.0f, 1.0f, 1.0f);
			g_pDrawPrim->SetRGBA(&m_Circles[i][j], 0xFFFFFFFF);
		}
	}

	for (i = 0; i < kMaxMatches; i++)
	{
		for (j = 0; j < 2; j++)
			g_pDrawPrim->SetRGBA(&m_Boxes[i][j], 0x80404040);
	}

	m_nNumMatches = 0;
	m_eGameType = g_knNumGameTypes;

	UpdateLayout();

	m_bInited = true;
	return LTTRUE;
}

void CHUDDiscMatches::Term()
{
	if (!m_bInited || !m_bControlsInited) return;

	m_HeaderTitles.Destroy();
	m_HeaderValues.Destroy();
	m_Titles.Destroy();

	for (int i = 0; i < kMaxMatches; i++)
		m_Rows[i].Destroy();
}

void CHUDDiscMatches::Show(bool bDraw, bool bScreen)
{
	m_bDraw = bDraw;
	m_bScreen = bScreen;

	Update();
}

void CHUDDiscMatches::Render()
{
	if (!IsDiscGameType()) return;
	if (!IsEnabled()) return;

	m_bDraw = (g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_MATCHES) && !g_pChatInput->IsVisible());

	if (m_bDraw)
	{
		if (m_bScreen)
			m_bScreen = false;
	}
	m_bDraw = (m_bDraw || m_bScreen);

	// RETAIL: the crosshair is hidden while the matches or the scores are
	// drawn
	bool bProgress = !!g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_PROGRESS);
	g_pCrosshair->Enable(!m_bDraw && !bProgress);

	if (!m_bDraw) return;

	GameType eGameType = g_pGameClientShell->GetGameType();
	std::wstring sText;
	char szTmp[256];

	if (!m_bScreen && eGameType == eGameTypeDiscArena)
	{
		int nTime;
		float fTime = g_pDiscMatchMgr->GetTronUnknownC();
		if (fTime > 0.0f)
			nTime = (int)(fTime - g_pLTClient->GetTime());
		else
			nTime = -1;

		int nSeconds = nTime % 60;
		int nMinutes = (nTime - nSeconds) / 60;
		if (nTime >= 0)
		{
			sprintf(szTmp, "%d:%02d", nMinutes, nSeconds);
			LTGUIWiden(sText, szTmp);
		}
		else
		{
			sText = L"--";
		}

		m_HeaderValues.GetPolyString(0)->SetText(sText);
	}

	if (m_bScreen) return;
	if (!m_nNumMatches) return;

	m_Frame.Render();
	if (eGameType == eGameTypeDiscArena)
	{
		m_HeaderTitles.Render();
		m_HeaderValues.Render();
	}
	m_Titles.Render();

	for (int i = 0; i < m_nNumMatches; i++)
	{
		g_pDrawPrim->DrawPrim(m_Boxes[i], 2);

		DiscMatch* pMatch = g_pDiscMatchMgr->GetMatch(i);
		int nTime = (int)(pMatch->fTronUnknown14 - g_pLTClient->GetTime());
		if (nTime < 0)
			nTime = 0;

		int nSeconds = nTime % 60;
		int nMinutes = (nTime - nSeconds) / 60;
		sprintf(szTmp, "%d:%02d", nMinutes, nSeconds);
		LTGUIWiden(sText, szTmp);

		m_Rows[i].GetPolyString(1)->SetText(sText);
		m_Rows[i].Render();

		for (int j = 0; j < m_nRounds[i]; j++)
		{
			g_pDrawPrim->SetTexture(m_nWins[i][0] > j ? m_hFull : m_hEmpty);
			g_pDrawPrim->DrawPrim(&m_Circles[i][j * 2], 1);

			g_pDrawPrim->SetTexture(m_nWins[i][1] > j ? m_hFull : m_hEmpty);
			g_pDrawPrim->DrawPrim(&m_Circles[i][j * 2 + 1], 1);
		}
	}
}

void CHUDDiscMatches::Update()
{
	if (!IsDiscGameType()) return;

	if (!m_bInited)
		Init();

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
		m_HeaderTitles.SetScale(fScale);
		m_HeaderValues.SetScale(fScale);
		m_Titles.SetScale(fScale);

		for (int i = 0; i < kMaxMatches; i++)
			m_Rows[i].SetScale(fScale);
	}

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	if (!pTeamMgr) return;

	uint16 nHeight;
	if (m_eGameType == eGameTypeDiscArena)
		nHeight = m_HeaderValues.GetHeight() + m_Titles.GetHeight() + m_HeaderTitles.GetHeight();
	else
		nHeight = m_Titles.GetHeight();

	m_nNumMatches = g_pDiscMatchMgr->GetNumMatches();
	if (!m_nNumMatches) return;

	std::wstring sText;
	char szTmp[32];

	sprintf(szTmp, "%d", g_pDiscMatchMgr->GetTronUnknown8());
	LTGUIWiden(sText, szTmp);
	m_HeaderValues.GetPolyString(1)->SetText(sText);

	int i;
	for (i = 0; i < m_nNumMatches; i++)
	{
		std::string sName;
		DiscMatch* pMatch = g_pDiscMatchMgr->GetMatch(i);

		sprintf(szTmp, "%d", pMatch->nOrder + 1);
		LTGUIWiden(sText, szTmp);
		m_Rows[i].GetPolyString(0)->SetText(sText);

		pTeamMgr->GetTeam(pMatch->nTeam[0], false)->GetName(false, sName, false);
		LTGUIWiden(sText, sName.c_str());
		m_Rows[i].GetPolyString(2)->SetText(sText);

		pTeamMgr->GetTeam(pMatch->nTeam[1], false)->GetName(false, sName, false);
		LTGUIWiden(sText, sName.c_str());
		m_Rows[i].GetPolyString(4)->SetText(sText);

		// RETAIL: a box behind each team's name, the rounds' circles below
		int32 x, y, x2, y2, w, h;
		m_Rows[i].GetPolyString(2)->GetPosition(&x, &y);
		m_Rows[i].GetPolyString(2)->GetDims(&w, &h);
		g_pDrawPrim->SetXYWH(&m_Boxes[i][0], (float)x, (float)y, (float)w, (float)h + 16.0f);

		m_Rows[i].GetPolyString(4)->GetPosition(&x2, &y2);
		m_Rows[i].GetPolyString(4)->GetDims(&w, &h);
		g_pDrawPrim->SetXYWH(&m_Boxes[i][1], (float)x2, (float)y2, (float)w, (float)h + 16.0f);

		y += h;
		for (int j = 0; j < kMaxRounds; j++)
		{
			float fX = (float)j * 16.0f;
			g_pDrawPrim->SetXYWH(&m_Circles[i][j * 2], (float)x + fX, (float)y, 16.0f, 16.0f);
			g_pDrawPrim->SetXYWH(&m_Circles[i][j * 2 + 1], (float)x2 + fX, (float)y, 16.0f, 16.0f);
		}

		if (m_eGameType == eGameTypeDiscTournament)
		{
			sprintf(szTmp, "(%d/%d)", pMatch->nTronUnknownC, pMatch->nTronUnknown10);
			LTGUIWiden(sText, szTmp);
			m_Rows[i].GetPolyString(5)->SetText(sText);
		}

		nHeight += m_Rows[i].GetHeight();

		m_nRounds[i] = (pMatch->nTronUnknown5 + 1) / 2;
		m_nWins[i][0] = pMatch->nWins[0];
		m_nWins[i][1] = pMatch->nWins[1];
	}

	m_Frame.SetSize((int)((float)m_Titles.GetWidth() / fScale) + 20, (int)((float)nHeight / fScale) + (i + 2) * 16);
}

void CHUDDiscMatches::UpdateLayout()
{
	m_BasePos = g_pLayoutMgr->GetPoint(s_szDiscMatchesTag, "BasePos");
	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(s_szDiscMatchesTag, "Font", 0);
	m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(s_szDiscMatchesTag, "FontSize", 0);

	LTVector vCol = g_pLayoutMgr->GetVector(s_szDiscMatchesTag, "TextColor");
	m_nTextColor = SET_ARGB(0xFF, (uint8)vCol.x, (uint8)vCol.y, (uint8)vCol.z);

	// RETAIL: read and not used
	vCol = g_pLayoutMgr->GetVector(s_szDiscMatchesTag, "FrameColor");
	g_pLayoutMgr->GetFloat(s_szDiscMatchesTag, "FrameAlpha", 0.0f);

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_nFrameWidth = 421;
	if (m_eGameType == eGameTypeDiscTournament)
		m_nFrameWidth = 471;

	LTIntPt pos = m_BasePos;

	if (!m_bControlsInited)
	{
		m_Frame.Create(g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\frame_solid.dtx"), m_nFrameWidth, 32, LTFALSE);
		m_Frame.SetAlpha(0xBF);

		m_HeaderTitles.Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);
		m_HeaderValues.Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);
		m_Titles.Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);

		for (int i = 0; i < kMaxMatches; i++)
			m_Rows[i].Create(0, 0, pFont, m_nBaseFontSize, LTNULL, 0, 0);

		m_bControlsInited = true;
	}

	m_HeaderTitles.RemoveAllColumns();
	m_HeaderTitles.SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	m_HeaderTitles.SetBasePos(pos);
	m_HeaderTitles.AddColumn(LoadTempWString(IDS_TRON_DISCMATCHES_TIME), 200, LTFALSE);
	m_HeaderTitles.AddColumn(LoadTempWString(IDS_TRON_DISCMATCHES_ROUND), 200, LTFALSE);
	pos.y += m_nBaseFontSize;

	m_HeaderValues.RemoveAllColumns();
	m_HeaderValues.SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
	m_HeaderValues.SetBasePos(pos);
	m_HeaderValues.AddColumn(std::wstring(L"0:00"), 200, LTFALSE);
	m_HeaderValues.AddColumn(std::wstring(L"N/A"), 200, LTFALSE);
	pos.y += m_nBaseFontSize;

	m_Titles.RemoveAllColumns();
	m_Titles.SetColors(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	m_Titles.SetBasePos(pos);
	m_Titles.AddColumn(LoadTempWString(IDS_TRON_DISCMATCHES_MATCH), 75, LTFALSE);
	m_Titles.AddColumn(LoadTempWString(IDS_TRON_DISCMATCHES_TIMELEFT), 90, LTFALSE);
	m_Titles.AddColumn(std::wstring(L" "), 100, LTFALSE);
	m_Titles.AddColumn(LoadTempWString(IDS_TRON_DISCMATCHES_VERSUS), 40, LTFALSE);
	m_Titles.AddColumn(std::wstring(L" "), 100, LTFALSE);

	LTIntPt framePos(m_BasePos.x - 8, m_BasePos.y - 4);
	if (m_eGameType == eGameTypeDiscTournament)
	{
		m_Titles.AddColumn(LoadTempWString(IDS_TRON_DISCMATCHES_SCORE), 50, LTFALSE);
		framePos.y = pos.y - 4;
	}
	m_Frame.SetBasePos(framePos);

	pos.y += m_nBaseFontSize;

	for (int i = 0; i < kMaxMatches; i++)
	{
		m_Rows[i].RemoveAllColumns();
		m_Rows[i].SetColors(m_nTextColor, m_nTextColor, m_nTextColor);
		m_Rows[i].SetBasePos(pos);
		m_Rows[i].AddColumn(std::wstring(L"-"), 75, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L"0:00"), 90, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L"A"), 100, LTFALSE);
		m_Rows[i].AddColumn(LoadTempWString(IDS_TRON_DISCMATCHES_VS), 40, LTFALSE);
		m_Rows[i].AddColumn(std::wstring(L"B"), 100, LTFALSE);
		if (m_eGameType == eGameTypeDiscTournament)
			m_Rows[i].AddColumn(std::wstring(L"(1/1)"), 50, LTFALSE);

		pos.y += m_nBaseFontSize + 16;
	}
}
