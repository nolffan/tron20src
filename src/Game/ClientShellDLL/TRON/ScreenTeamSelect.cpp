// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenTeamSelect.cpp
//
// PURPOSE : Tron's multiplayer team selection screen: the teams and their
//			 players, the spectators, and moving the local player
//
// RETAIL: written from retail CShell.dll 0x10082510..0x10084080 (the
// function and member names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenTeamSelect.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "ClientInfoMgr.h"
#include "SharedScoring.h"
#include "GameClientShell.h"

namespace
{
	// RETAIL: Tron's command IDs (the names are ours): a team's rows, the
	// spectators' rows
	const uint32 kCmdTeamRow		= 0x433;
	const uint32 kCmdSpectatorRow	= 0x447;

	const int kMaxRows = 16;

	// the layout's screen (SCREEN_ID_TEAMSELECT)
	const eScreenID kScreenID = (eScreenID)0x28;
}

// RETAIL: 0x10082510
CScreenTeamSelect::CScreenTeamSelect()
{
}

// RETAIL: 0x10082550
CScreenTeamSelect::~CScreenTeamSelect()
{
}

// RETAIL: 0x10082560
LTBOOL CScreenTeamSelect::Build()
{
	CreateTitle(0xCD5);

	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	LTVector vColor = g_pLayoutMgr->GetScreenCustomVector(kScreenID,"FrameColor");
	uint32 argbFrameColor = SET_ARGB((uint8)(g_pLayoutMgr->GetScreenCustomFloat(kScreenID,"FrameAlpha") * 255.0f),
		(uint8)vColor.x, (uint8)vColor.y, (uint8)vColor.z);

	vColor = g_pLayoutMgr->GetScreenCustomVector(kScreenID,"TextColor");
	m_nTextColor = SET_ARGB(0xFF, (uint8)vColor.x, (uint8)vColor.y, (uint8)vColor.z);

	vColor = g_pLayoutMgr->GetScreenCustomVector(kScreenID,"PlayerTextColor");
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;
	m_nPlayerTextColor = SET_ARGB(0xFF, nR, nG, nB);

	LTRect rcTeam = g_pLayoutMgr->GetScreenCustomRect(kScreenID,"TeamRect");
	LTRect rcSpec = g_pLayoutMgr->GetScreenCustomRect(kScreenID,"SpectatorRect");

	m_nHeaderFontSize = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"HeaderFontSize");
	m_nListFontSize = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"ListFontSize");

	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\frame_solid.dtx");

	int nTeamWidth = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"TeamColumn");
	int nMatchesWonWidth = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"MatchesWonColumn");
	int nPlayerWidth = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"PlayerColumn");
	int nGamesWonWidth = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"GamesWonColumn");
	int nPingWidth = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"PingColumn");
	int nSpectatorWidth = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"SpectatorColumn");
	int nSpectatorPingWidth = g_pLayoutMgr->GetScreenCustomInt(kScreenID,"SpectatorPingColumn");

	// the teams' frame and header
	m_pTeamFrame = debug_new(CLTGUIFrame);
	m_pTeamFrame->Create(hFrame,10,10);
	m_pTeamFrame->SetBasePos(LTIntPt(rcTeam.left - 4, rcTeam.top - 4));

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight((int)(g_pInterfaceResMgr->GetXRatio() * 20.0f));

	m_pTeamHeader = CreateColumnCtrl(LTNULL,LTNULL,LTIntPt(rcTeam.left,rcTeam.top));
	m_pTeamHeader->SetFont(pFont,(uint8)m_nHeaderFontSize);
	m_pTeamHeader->AddColumn(LoadTempWString(0xCC8),nTeamWidth,LTTRUE);
	m_pTeamHeader->AddColumn(LoadTempWString(0xCC9),nMatchesWonWidth,LTTRUE);
	m_pTeamHeader->AddColumn(LoadTempWString(0x182),nPlayerWidth,LTTRUE);
	m_pTeamHeader->AddColumn(LoadTempWString(0x2B5),nGamesWonWidth,LTTRUE);
	m_pTeamHeader->AddColumn(LoadTempWString(0x183),nPingWidth,LTTRUE);

	m_pTeamHeaderBar = debug_new(CLTGUIFrame);
	CLTGUICtrl* pHeader = m_pTeamHeader;
	m_pTeamHeaderBar->Create(argbFrameColor,(uint16)((float)pHeader->GetWidth() / fXRatio),(uint16)((float)pHeader->GetHeight() / fYRatio));
	m_pTeamHeaderBar->SetBasePos(LTIntPt(rcTeam.left,rcTeam.top));

	AddControl(m_pTeamFrame);
	AddControl(m_pTeamHeaderBar);
	AddControl(m_pTeamHeader);

	// the spectators' frame and header
	m_pSpecFrame = debug_new(CLTGUIFrame);
	m_pSpecFrame->Create(hFrame,10,10);
	m_pSpecFrame->SetBasePos(LTIntPt(rcSpec.left - 4, rcSpec.top - 4));

	m_pSpecHeader = CreateColumnCtrl(LTNULL,LTNULL,LTIntPt(rcSpec.left,rcSpec.top));
	m_pSpecHeader->SetFont(pFont,(uint8)m_nHeaderFontSize);
	m_pSpecHeader->AddColumn(LoadTempWString(0xCD6),nSpectatorWidth,LTTRUE);
	m_pSpecHeader->AddColumn(LoadTempWString(0x183),nSpectatorPingWidth,LTTRUE);

	m_pSpecHeaderBar = debug_new(CLTGUIFrame);
	pHeader = m_pSpecHeader;
	m_pSpecHeaderBar->Create(argbFrameColor,(uint16)((float)pHeader->GetWidth() / fXRatio),(uint16)((float)pHeader->GetHeight() / fYRatio));
	m_pSpecHeaderBar->SetBasePos(LTIntPt(rcSpec.left,rcSpec.top));

	AddControl(m_pSpecFrame);
	AddControl(m_pSpecHeaderBar);
	AddControl(m_pSpecHeader);

	// the rows, below the headers
	int nTeamY = rcTeam.top + (int)((float)m_pTeamHeader->GetHeight() / g_pInterfaceResMgr->GetYRatio());
	int nSpecY = rcSpec.top + (int)((float)m_pSpecHeader->GetHeight() / g_pInterfaceResMgr->GetYRatio());

	CUIFont* pListFont = g_pInterfaceResMgr->GetFontForHeight(16);

	for (int i = 0; i < kMaxRows; i++)
	{
		m_pTeamRows[i] = AddColumnCtrl(kCmdTeamRow + i,0xCF1,LTIntPt(rcTeam.left,nTeamY));
		m_pTeamRows[i]->SetFont(pListFont,(uint8)m_nListFontSize);
		m_pTeamRows[i]->AddColumn(std::wstring(L"-"),nTeamWidth);
		m_pTeamRows[i]->AddColumn(std::wstring(L"-"),nMatchesWonWidth);
		m_pTeamRows[i]->AddColumn(std::wstring(L"-"),nPlayerWidth);
		m_pTeamRows[i]->AddColumn(std::wstring(L"-"),nGamesWonWidth);
		m_pTeamRows[i]->AddColumn(std::wstring(L"-"),nPingWidth);
		nTeamY += (int)((float)m_pTeamRows[i]->GetHeight() / g_pInterfaceResMgr->GetYRatio());

		m_pSpecRows[i] = AddColumnCtrl(kCmdSpectatorRow,0xCF2,LTIntPt(rcSpec.left,nSpecY));
		m_pSpecRows[i]->SetFont(pListFont,(uint8)m_nListFontSize);
		m_pSpecRows[i]->AddColumn(std::wstring(L"-"),nSpectatorWidth);
		m_pSpecRows[i]->AddColumn(std::wstring(L"-"),nSpectatorPingWidth);
		nSpecY += (int)((float)m_pSpecRows[i]->GetHeight() / g_pInterfaceResMgr->GetYRatio());
	}

	m_pTeamFrame->SetSize((uint16)((float)m_pTeamHeaderBar->GetWidth() / fXRatio) + 8,nTeamY - rcTeam.top + 8);
	m_pSpecFrame->SetSize((uint16)((float)m_pSpecHeaderBar->GetWidth() / fXRatio) + 8,nSpecY - rcSpec.top + 8);

	ScreenDimsChanged();
	m_bNeedUpdate = false;

	return CBaseScreen::Build();
}

// RETAIL: 0x10083050: a row moves the local player to its team
uint32 CScreenTeamSelect::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	if (!pTeamMgr) return 1;

	if (dwCommand == kCmdSpectatorRow)
	{
		pTeamMgr->MoveClient(nLocalID,TRON_UNASSIGNED_TEAM);
		return 1;
	}

	// RETAIL: the test takes one command more than there are rows (0x443,
	// docs/RETAIL_BUGS.md)
	if (dwCommand >= kCmdTeamRow && dwCommand <= kCmdTeamRow + kMaxRows)
	{
		pTeamMgr->MoveClient(nLocalID,m_anRowTeam[dwCommand - kCmdTeamRow]);
		return 1;
	}

	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
}

// RETAIL: 0x100830f0: the light cycle games count other wins
void CScreenTeamSelect::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		if (GetGameType() == eGameTypeMultiplayerLightcycles)
		{
			m_pTeamHeader->GetColumn(1)->SetString(LoadTempWString(0xCE0));
			m_pTeamHeader->GetColumn(3)->SetString(LoadTempWString(0xCC0));
		}
		else
		{
			m_pTeamHeader->GetColumn(1)->SetString(LoadTempWString(0xCC9));
			m_pTeamHeader->GetColumn(3)->SetString(LoadTempWString(0x2B5));
		}

		if (g_pInterfaceResMgr->GetXRatio() != m_fLastScale)
			ScreenDimsChanged();

		CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight((int)((float)m_nHeaderFontSize * g_pInterfaceResMgr->GetXRatio()));
		m_pTeamHeader->SetFont(pFont,(uint8)m_nHeaderFontSize);
		m_pSpecHeader->SetFont(pFont,(uint8)m_nHeaderFontSize);

		pFont = g_pInterfaceResMgr->GetFontForHeight((int)((float)m_nListFontSize * g_pInterfaceResMgr->GetXRatio()));
		for (int i = 0; i < kMaxRows; i++)
		{
			m_pTeamRows[i]->SetFont(pFont,(uint8)m_nListFontSize);
			m_pSpecRows[i]->SetFont(pFont,(uint8)m_nListFontSize);
		}

		Update();
		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData(LTTRUE);
	}

	CBaseScreen::OnFocus(bFocus);
}

// RETAIL: 0x10083300
LTBOOL CScreenTeamSelect::Render(HSURFACE hDestSurf)
{
	if (m_bNeedUpdate)
		Update();

	return CBaseScreen::Render(hDestSurf);
}

// RETAIL: 0x10083330
void CScreenTeamSelect::Update()
{
	m_bNeedUpdate = false;

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	if (!pTeamMgr) return;

	CClientInfoMgr* pCIMgr = g_pGameClientShell->GetInterfaceMgr()->GetClientInfoMgr();
	if (!pCIMgr) return;

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	std::wstring sText0;
	std::wstring sText1;
	std::wstring sText2;
	char szTmp[64];

	int i;
	for (i = 0; i < kMaxRows; i++)
	{
		m_pSpecRows[i]->Enable(LTFALSE);
		m_pSpecRows[i]->Show(LTFALSE);
		m_pTeamRows[i]->Enable(LTFALSE);
		m_pTeamRows[i]->Show(LTFALSE);
	}

	int nNumTeams = pTeamMgr->GetNumTeams();
	int nTeamRow = 0;
	int nSpecRow = 0;

	LTRect rcTeam = g_pLayoutMgr->GetScreenCustomRect(kScreenID,"TeamRect");
	int nTeamX = rcTeam.left;
	int nTeamY = rcTeam.top + (int)((float)m_pTeamHeader->GetHeight() / g_pInterfaceResMgr->GetYRatio());

	LTRect rcSpec = g_pLayoutMgr->GetScreenCustomRect(kScreenID,"SpectatorRect");
	int nSpecX = rcSpec.left;
	// RETAIL: the spectators' frame is sized to the header's bottom (nSpecY),
	// not to the rows' (nSpecRowY)
	int nSpecY = rcSpec.top + (int)((float)m_pSpecHeader->GetHeight() / g_pInterfaceResMgr->GetYRatio());
	int nSpecRowY = nSpecY;

	for (int nTeam = 0; nTeam <= nNumTeams; nTeam++)
	{
		// the spectators come after the teams
		if (nTeam == nNumTeams)
			nTeam = TRON_UNASSIGNED_TEAM;

		CTronTeam* pTeam = pTeamMgr->GetTeam(nTeam,false);
		uint32 nNumClients = pTeam->GetNumClients();
		uint32 nMaxClients = pTeam->GetMaxClients();
		uint32 nID = pTeam->GetID();

		if (nID == TRON_UNASSIGNED_TEAM)
		{
			CLTGUIColumnCtrl** ppRow = &m_pSpecRows[nSpecRow];
			nSpecRow += nMaxClients;

			for (uint32 nRow = 0; nRow < nMaxClients; nRow++, ppRow++)
			{
				CLTGUIColumnCtrl* pRow = *ppRow;
				pRow->SetColors(m_nTextColor,m_nTextColor,m_nTextColor);
				pRow->Enable(LTTRUE);
				pRow->Show(LTTRUE);
				pRow->SetBasePos(LTIntPt(nSpecX,nSpecRowY));
				nSpecRowY += (int)((float)pRow->GetHeight() / g_pInterfaceResMgr->GetYRatio());

				sText0 = L"- - -";
				sText1 = L" ";

				if (nRow < nNumClients)
				{
					CLIENT_INFO* pCI = pCIMgr->GetClientByID(pTeam->GetClient(nRow),true);
					if (pCI)
					{
						pRow->Enable(LTFALSE);
						LTGUIWiden(sText0,pCI->sName.c_str());
						sprintf(szTmp,"%d",pCI->nPing);
						LTGUIWiden(sText1,szTmp);
						if (nLocalID == pCI->nID)
							pRow->SetColors(m_nPlayerTextColor,m_nPlayerTextColor,m_nPlayerTextColor);
					}
				}

				pRow->GetPolyString(0)->SetText(sText0);
				pRow->GetPolyString(1)->SetText(sText1);
			}
		}
		else
		{
			// a space between the teams of more than one player
			if (pTeamMgr->GetTeamSize() > 1)
				nTeamY += m_nListFontSize;

			std::string sName;

			for (uint32 nRow = 0; nRow < nMaxClients; nRow++)
			{
				CLTGUIColumnCtrl* pRow = m_pTeamRows[nTeamRow];

				// the rows of single player teams alternate their backgrounds
				uint32 argbBack;
				if (pTeamMgr->GetTeamSize() == 1)
					argbBack = (nTeam & 1) ? 0x80404040 : 0;
				else
					argbBack = 0x80404040;
				pRow->SetBackgroundColor(argbBack);

				pRow->SetColors(m_nTextColor,m_nTextColor,m_nTextColor);
				pRow->Enable(LTTRUE);
				pRow->Show(LTTRUE);
				pRow->SetBasePos(LTIntPt(nTeamX,nTeamY));
				nTeamY += (int)((float)pRow->GetHeight() / g_pInterfaceResMgr->GetYRatio());
				m_anRowTeam[nTeamRow] = nID;

				// the team's name and wins on its first row
				if (nRow)
				{
					sText0 = L" ";
					sText1 = L" ";
				}
				else
				{
					if (pTeam->GetTeamSize() > 1)
						pTeam->GetName(false,sName,GetGameType() == eGameTypeMultiplayerLightcycles);
					else
						sName = "  -";
					LTGUIWiden(sText0,sName.c_str());

					sprintf(szTmp,"%d",(GetGameType() == eGameTypeMultiplayerLightcycles) ? pTeam->GetTronUnknown12() : pTeam->GetTronUnknownC());
					LTGUIWiden(sText1,szTmp);
				}

				pRow->GetPolyString(0)->SetText(sText0);
				pRow->GetPolyString(1)->SetText(sText1);

				sText0 = L"- - - -";
				sText1 = L"-";
				sText2 = L"-";

				if (nRow < nNumClients)
				{
					CLIENT_INFO* pCI = pCIMgr->GetClientByID(pTeam->GetClient(nRow),true);
					if (pCI)
					{
						pRow->Enable(LTFALSE);
						LTGUIWiden(sText0,pCI->sName.c_str());

						sprintf(szTmp,"%d",(GetGameType() == eGameTypeMultiplayerLightcycles) ? pCI->sScore.GetTronUnknownC() : pCI->sScore.GetScore());
						LTGUIWiden(sText1,szTmp);

						sprintf(szTmp,"%d",pCI->nPing);
						LTGUIWiden(sText2,szTmp);

						if (nLocalID == pCI->nID)
							pRow->SetColors(m_nPlayerTextColor,m_nPlayerTextColor,m_nPlayerTextColor);
					}
				}

				pRow->GetPolyString(2)->SetText(sText0);
				pRow->GetPolyString(3)->SetText(sText1);
				pRow->GetPolyString(4)->SetText(sText2);

				nTeamRow++;
			}
		}
	}

	// both frames take the taller list
	int nHeight = LTMAX(nTeamY - rcTeam.top + 8, nSpecY - rcSpec.top + 8);
	m_pTeamFrame->SetSize((uint16)((float)m_pTeamHeaderBar->GetWidth() / g_pInterfaceResMgr->GetXRatio()) + 8,nHeight);
	m_pSpecFrame->SetSize((uint16)((float)m_pSpecHeaderBar->GetWidth() / g_pInterfaceResMgr->GetYRatio()) + 8,nHeight);
}

// RETAIL: 0x10084050: F1 closes the screen
LTBOOL CScreenTeamSelect::HandleKeyDown(int key, int rep)
{
	if (key == VK_F1)
	{
		Escape();
		return LTTRUE;
	}

	return CBaseScreen::HandleKeyDown(key,rep);
}
