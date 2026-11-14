// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScreenLightCycle.cpp
//
// PURPOSE : Base of Tron's single player light cycle screens: the four
//			 tabs (circuit, player config, custom maps, custom game), the
//			 player's name and cycle, and the list of saved game settings
//
// RETAIL: written from retail CShell.dll 0x10008230..0x1000a6f0 (Tron's
// own class: the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseScreenLightCycle.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "MissionButeMgr.h"
#include "ProfileMgr.h"
#include "CheatMgr.h"
#include "LightCycleGameDataMgr.h"
#include "TRON/LightCycleButeMgr.h"
#include "GameClientShell.h"

namespace
{
	// RETAIL: 0x10210ab0: the font size of the settings list (a variable)
	int		s_nListFontSize = 20;

	// RETAIL: the cheat that unlocks every cycle color (retail's cheat
	// 0x37; ours has no such cheat yet: the name is ours)
	const int kCheatTronUnknown37 = 0x37;

	// RETAIL: 0x10008250: the answer to "delete these settings?"
	void DeleteSettingsCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID());
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdLCDeleteSettings);
	}

	// RETAIL: 0x100082a0: the player's new name
	void EditNameCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID());
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_NAME);
	}
}

// RETAIL: CBaseScreenHost's (0x10002f40, 0x10002f90). PORT: here until its
// TU is written, which should take them (and this file lose them)
void NewSettingsCallBack(LTBOOL bReturn, void *pData)
{
	CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID());
	if (bReturn && pThisScreen)
		pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdLCNewSettings);
}

void RenameSettingsCallBack(LTBOOL bReturn, void *pData)
{
	CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID());
	if (bReturn && pThisScreen)
		pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdLCRenameSettings);
}

// RETAIL: 0x100082f0
CBaseScreenLightCycle::CBaseScreenLightCycle()
{
	m_pPlayerConfigTab = LTNULL;
	m_pCircuitTab = LTNULL;
	m_pCustomGameTab = LTNULL;
	m_pCustomMapTab = LTNULL;
	m_bChangingFocus = false;
	m_nTabColor = 0x80000080;
	m_pNewSettings = LTNULL;
	m_pRenameSettings = LTNULL;
	m_pDeleteSettings = LTNULL;
	m_pSettingsList = LTNULL;
	m_pSettingsTitle = LTNULL;
	m_sNewName = "unnamed";
	m_pName = LTNULL;
	m_pCycleName = LTNULL;
}

// RETAIL: 0x10008540
CBaseScreenLightCycle::~CBaseScreenLightCycle()
{
	for (int i = 0; i < 8; i++)
	{
		m_TabLines[i].Term();
	}
}

// RETAIL: 0x10008690
LTBOOL CBaseScreenLightCycle::Build()
{
	CreateTitle(0x25B4);

	LTRect rcTab = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_BASE_LIGHTCYCLE,"TabRect");
	int nTabWidth = (rcTab.right - rcTab.left) / 4;

	LTVector vColor = g_pLayoutMgr->GetScreenCustomVector(SCREEN_ID_HOST,"TabColor");
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;
	uint8 nA = (uint8)(g_pLayoutMgr->GetScreenCustomFloat(SCREEN_ID_HOST,"TabAlpha") * 255.0f);
	m_nTabColor = SET_ARGB(nA,nR,nG,nB);

	LTIntPt pos(rcTab.left,rcTab.top);
	m_pCircuitTab = AddTextItem(0x259D,kCmdLCCircuitTab,0,pos);
	m_pCircuitTab->SetFixedWidth(nTabWidth);
	pos.x += nTabWidth;
	m_pPlayerConfigTab = AddTextItem(0x25CC,kCmdLCPlayerConfigTab,0,pos);
	m_pPlayerConfigTab->SetFixedWidth(nTabWidth);
	pos.x += nTabWidth;
	m_pCustomMapTab = AddTextItem(0x259F,kCmdLCCustomMapTab,0,pos);
	m_pCustomMapTab->SetFixedWidth(nTabWidth);
	pos.x += nTabWidth;
	m_pCustomGameTab = AddTextItem(0x259E,kCmdLCCustomGameTab,0,pos);
	m_pCustomGameTab->SetFixedWidth(nTabWidth);

	// the tabs' order: circuit, player config, custom maps, custom game
	m_TabOrder[SCREEN_ID_LIGHTCYCLE_CIRCUIT] = std::pair<int,int>(SCREEN_ID_LIGHTCYCLE_CUSTOMGAME,SCREEN_ID_LIGHTCYCLE_PLAYERCONFIG);
	m_TabOrder[SCREEN_ID_LIGHTCYCLE_PLAYERCONFIG] = std::pair<int,int>(SCREEN_ID_LIGHTCYCLE_CIRCUIT,SCREEN_ID_LIGHTCYCLE_CUSTOMMAP);
	m_TabOrder[SCREEN_ID_LIGHTCYCLE_CUSTOMMAP] = std::make_pair(SCREEN_ID_LIGHTCYCLE_PLAYERCONFIG,SCREEN_ID_LIGHTCYCLE_CUSTOMGAME);
	m_TabOrder[SCREEN_ID_LIGHTCYCLE_CUSTOMGAME] = std::make_pair(SCREEN_ID_LIGHTCYCLE_CUSTOMMAP,SCREEN_ID_LIGHTCYCLE_CIRCUIT);

	LTIntPt namePos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_BASE_LIGHTCYCLE,"NamePos");
	int nNameColumn = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_BASE_LIGHTCYCLE,"NameColumn");
	int nNameWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_BASE_LIGHTCYCLE,"NameWidth");

	m_pName = AddColumnCtrl(CMD_EDIT_NAME,0xDA3,namePos);
	m_pName->AddColumn(LoadTempWString(0xDA4),nNameColumn);
	m_pName->AddColumn(std::wstring(L"<player name>"),nNameWidth,LTTRUE);
	m_pName->SetColors(argbWhite,argbWhite,argbWhite);

	m_pCycleName = AddColumnCtrl(0,0xDA5,g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_BASE_LIGHTCYCLE,"CycleNamePos"));
	m_pCycleName->AddColumn(LoadTempWString(0x2620),nNameColumn);
	m_pCycleName->AddColumn(std::wstring(L"<cycle name>"),nNameWidth,LTTRUE);
	m_pCycleName->SetColors(argbWhite,argbWhite,argbWhite);

	char szName[128];
	FormatString(0xA04,szName,sizeof(szName));
	m_sNewName = szName;

	return CBaseScreen::Build();
}

// RETAIL: 0x10008d10
void CBaseScreenLightCycle::CreateSettingsList()
{
	LTRect rcAvail = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_BASE_LIGHTCYCLE,"AvailRect");
	m_pSettingsList = AddList(LTIntPt(rcAvail.left,rcAvail.top),rcAvail.bottom - rcAvail.top,LTTRUE,rcAvail.right - rcAvail.left - 32);
	m_pSettingsList->SetIndent(LTIntPt(5,5));
	// RETAIL: here Tron's list control takes a frame texture
	// ("interface\menu\sprtex\frame_solid.dtx", CLTGUIListCtrl 0x100b98e0),
	// and a member is set (+0x670 = 1, after the Enable): left out, our
	// LTGUIMgr's list control has neither yet
	m_pSettingsList->Enable(LTTRUE);

	m_pSettingsTitle = AddTextItem(0x25B7,0,0,g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_BASE_LIGHTCYCLE,"ConfigTitlePos"),LTTRUE);
	m_pNewSettings = AddTextItem(0x25B8,kCmdLCNewSettings,0x25B9,g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_BASE_LIGHTCYCLE,"CommandPos"));
	m_pRenameSettings = AddTextItem(0x25BA,kCmdLCRenameSettings,0x25BB);
	m_pDeleteSettings = AddTextItem(0x25BC,kCmdLCDeleteSettings,0x25BD);
}

// RETAIL: 0x10008ec0: the body's frame, opened where the selected tab is
void CBaseScreenLightCycle::SetTabFrame(CLTGUICtrl *pTab)
{
	if (!pTab) return;

	LTIntPt tabPos = pTab->GetPos();
	float fTabLeft = (float)tabPos.x - 10.0f;
	float fTabTop = (float)tabPos.y - 5.0f;
	float fTabRight = (float)pTab->GetWidth() + fTabLeft;

	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	LTRect rcBody = GetBodyRect();
	float fLeft = (float)rcBody.left * fXRatio;
	float fWidth = (float)(rcBody.right - rcBody.left) * fXRatio;
	float fTop = (float)(rcBody.top - 5) * fYRatio;
	float fHeight = (float)(rcBody.bottom - rcBody.top) * fYRatio;

	g_pDrawPrim->SetXYWH(&m_TabPoly[0],fLeft,fTop,fWidth,fHeight);
	g_pDrawPrim->SetXYWH(&m_TabPoly[1],fTabLeft,fTabTop,fTabRight - fTabLeft,fTop - fTabTop);

	float fBottom = fTop + fHeight;
	m_TabLines[0].Create(fLeft,fTop,fLeft,fBottom);
	float fRight = fLeft + fWidth;
	m_TabLines[1].Create(fLeft,fBottom,fRight,fBottom);
	m_TabLines[2].Create(fRight,fTop,fRight,fBottom);
	m_TabLines[3].Create(fLeft,fTop,fTabLeft,fTop);
	m_TabLines[4].Create(fTabRight,fTop,fRight,fTop);
	m_TabLines[5].Create(fTabLeft,fTabTop + 2.0f,fTabLeft,fTop);
	m_TabLines[6].Create(fTabLeft,fTabTop,fTabRight,fTabTop);
	m_TabLines[7].Create(fTabRight,fTabTop + 2.0f,fTabRight,fTop);

	g_pDrawPrim->SetRGBA(&m_TabPoly[0],m_nTabColor);
	g_pDrawPrim->SetRGBA(&m_TabPoly[1],m_nTabColor);
}

// RETAIL: 0x10009110
uint32 CBaseScreenLightCycle::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}

	switch (dwCommand)
	{
	case CMD_EDIT_NAME:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditNameCallBack;
			mb.pString = m_sPlayerName.c_str();
			mb.nMaxChars = 15;
			g_pInterfaceMgr->ShowMessageBox(0x1F8,&mb);
			return 1;
		}
	case kCmdLCPlayerConfigTab:
		if (m_nScreenID != SCREEN_ID_LIGHTCYCLE_PLAYERCONFIG)
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_PLAYERCONFIG,false);
			return 1;
		}
		break;
	case kCmdLCCircuitTab:
		if (m_nScreenID != SCREEN_ID_LIGHTCYCLE_CIRCUIT)
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_CIRCUIT,false);
			return 1;
		}
		break;
	case kCmdLCCustomGameTab:
		if (m_nScreenID != SCREEN_ID_LIGHTCYCLE_CUSTOMGAME)
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_CUSTOMGAME,false);
			return 1;
		}
		break;
	case kCmdLCCustomMapTab:
		if (m_nScreenID != SCREEN_ID_LIGHTCYCLE_CUSTOMMAP)
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_CUSTOMMAP,false);
			return 1;
		}
		break;
	case kCmdLCNewSettings:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = NewSettingsCallBack;
			mb.pString = m_sNewName;
			mb.nMaxChars = 16;
			mb.eInput = CLTGUIEditCtrl::kInputFileFriendly;
			g_pInterfaceMgr->ShowMessageBox(0x25BE,&mb);
			return 1;
		}
	case kCmdLCRenameSettings:
		{
			CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = RenameSettingsCallBack;
			mb.pString = pData->m_sName.c_str();
			mb.nMaxChars = 16;
			mb.eInput = CLTGUIEditCtrl::kInputFileFriendly;
			g_pInterfaceMgr->ShowMessageBox(0x25BA,&mb);
			return 1;
		}
	case kCmdLCDeleteSettings:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = DeleteSettingsCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(0x25BF,&mb);
			return 1;
		}
	case kCmdLCSelectSettings:
		if (!m_bChangingFocus)
		{
			CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSettingsList->GetControl(m_pSettingsList->GetSelectedIndex());
			if (pCtrl)
			{
				const std::wstring &sText = pCtrl->GetString()->GetText();
				char szName[128] = {0};
				for (int i = 0; i < sText.size() && i < 128; i++)
					szName[i] = (char)sText[i];

				SaveSettings();
				g_pLightCycleGameDataMgr->Select(std::string(szName),false);
				LoadSettings();
			}
		}
		break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// RETAIL: 0x10009510
void CBaseScreenLightCycle::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		if (g_pLTClient->IsConnected())
			g_pLTClient->Disconnect();

		if (g_pInterfaceResMgr->GetXRatio() != m_fLastScale)
			ScreenDimsChanged();

		LoadSettings();
		m_bChangingFocus = true;

		if (!g_pMissionButeMgr->Init("Attributes\\nomissions.txt"))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.","Attributes\\nomissions.txt");
			return;
		}

		if (g_pProfileMgr)
		{
			LIGHTCYCLECOLOR *pColor = LTNULL;
			uint8 nStyle = 0;
			uint8 nShade = 0;
			GetProfileCycleColor(&pColor,&nStyle,&nShade);
			LIGHTCYCLESHADE *pShade = g_pLightCycleButeMgr->GetShade(nShade);

			int nStyleId = nStyle ? 0x259B : 0x259A;
			char szShade[128] = "";
			char szStyle[128] = "";
			LoadString(IsCycleColorLocked(pColor->szName) ? pShade->nLockedNameId : pShade->nNameId,szShade,sizeof(szShade));
			LoadString(nStyleId,szStyle,sizeof(szStyle));

			char szCycle[256] = "";
			FormatString(0x2621,szCycle,sizeof(szCycle),szStyle,szShade);

			std::wstring sCycle;
			LTGUIWiden(sCycle,szCycle);
			m_pCycleName->GetColumn(1)->SetString(sCycle);
		}

		UpdateSettingsList();
		m_pScreenMgr->AddScreenToHistory(SCREEN_ID_SINGLE);
	}
	else
	{
		SaveSettings();
	}

	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
		SetTabFrame(GetTab());

	m_bChangingFocus = false;
}

// RETAIL: 0x100098e0
LTBOOL CBaseScreenLightCycle::Render(HSURFACE hDestSurf)
{
	SetPolyRenderState();
	g_pDrawPrim->DrawPrim(m_TabPoly,2);
	for (int i = 0; i < 8; i++)
	{
		m_TabLines[i].Render();
	}

	return CBaseScreen::Render(hDestSurf);
}

// RETAIL: 0x10009930: a tab's screen is gone to if its tab may be chosen
LTBOOL CBaseScreenLightCycle::CycleTab(LTBOOL bNext)
{
	std::map<eScreenID, std::pair<int,int> >::iterator it = m_TabOrder.find((eScreenID)m_nScreenID);
	if (it != m_TabOrder.end())
	{
		int nScreen = (bNext == LTTRUE) ? (*it).second.second : (*it).second.first;
		CLTGUICtrl *pTab;
		switch (nScreen)
		{
		case SCREEN_ID_LIGHTCYCLE_CUSTOMGAME:
			pTab = m_pCustomGameTab;
			break;
		case SCREEN_ID_LIGHTCYCLE_CIRCUIT:
			pTab = m_pCircuitTab;
			break;
		case SCREEN_ID_LIGHTCYCLE_PLAYERCONFIG:
			pTab = m_pPlayerConfigTab;
			break;
		case SCREEN_ID_LIGHTCYCLE_CUSTOMMAP:
			pTab = m_pCustomMapTab;
			break;
		default:
			g_pInterfaceMgr->SwitchToScreen((eScreenID)nScreen,false);
			return LTTRUE;
		}

		if (!pTab->IsSelected() && !pTab->IsEnabled())
			return LTFALSE;

		g_pInterfaceMgr->SwitchToScreen((eScreenID)nScreen,false);
	}
	return LTTRUE;
}

// RETAIL: 0x100099f0: up and down go to the next and the previous tab
LTBOOL CBaseScreenLightCycle::OnUp()
{
	if (IsTabSelected())
		return CycleTab(LTTRUE);
	return CBaseScreen::OnUp();
}

// RETAIL: 0x10009a10
LTBOOL CBaseScreenLightCycle::OnDown()
{
	if (IsTabSelected())
		return CycleTab(LTFALSE);
	return CBaseScreen::OnDown();
}

// RETAIL: 0x10009a30: overrides that only call CBaseScreen's
LTBOOL CBaseScreenLightCycle::OnLeft()
{
	return CBaseScreen::OnLeft();
}

// RETAIL: 0x10009a40
LTBOOL CBaseScreenLightCycle::OnRight()
{
	return CBaseScreen::OnRight();
}

// RETAIL: 0x10009a50
void CBaseScreenLightCycle::ScreenDimsChanged()
{
	CBaseScreen::ScreenDimsChanged();
	if (IsTabSelected())
		SetTabFrame(GetControl(m_nSelection));
}

// RETAIL: 0x10009a80
void CBaseScreenLightCycle::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch (dwParam2)
	{
	case kCmdLCNewSettings:
		{
			char szName[128];
			{
				std::wstring sText((wchar_t *)dwParam1);
				memset(szName,0,sizeof(szName));
				for (uint32 i = 0; i < sText.size() && i < 128; i++)
					szName[i] = (char)sText[i];
			}
			if (!szName[0])
				return;

			SaveSettings();
			g_pLightCycleGameDataMgr->Select(std::string(szName),true);
			UpdateSettingsList();
		}
		break;
	case kCmdLCRenameSettings:
		{
			char szName[128];
			{
				std::wstring sText((wchar_t *)dwParam1);
				memset(szName,0,sizeof(szName));
				for (uint32 i = 0; i < sText.size() && i < 128; i++)
					szName[i] = (char)sText[i];
			}
			if (!szName[0])
				return;

			CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
			g_pLightCycleGameDataMgr->Rename(pData->m_sName,std::string(szName));
			UpdateSettingsList();
		}
		break;
	case kCmdLCDeleteSettings:
		{
			CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
			g_pLightCycleGameDataMgr->Delete(pData->m_sName);
			UpdateSettingsList();
			LoadSettings();
		}
		break;
	case CMD_EDIT_NAME:
		{
			char szName[16];
			{
				std::wstring sText((wchar_t *)dwParam1);
				memset(szName,0,sizeof(szName));
				for (uint32 i = 0; i < sText.size() && i < 16; i++)
					szName[i] = (char)sText[i];
			}
			m_sPlayerName = szName;
			m_pName->SetString(1,(wchar_t *)dwParam1);
		}
		break;
	}

	UpdateData(LTTRUE);
}

// RETAIL: 0x10009e90: tab and shift+tab go to the next and the previous tab
LTBOOL CBaseScreenLightCycle::HandleKeyDown(int key, int rep)
{
	if (key == VK_TAB)
	{
		if (GetAsyncKeyState(VK_SHIFT) & 0x80000000)
			CycleTab(LTFALSE);
		else
			CycleTab(LTTRUE);
	}
	return CBaseScreen::HandleKeyDown(key,rep);
}

// RETAIL: 0x10009ed0
CLTGUIListItemCtrl* CBaseScreenLightCycle::CreateListItem(std::wstring sString, uint32 commandID, int helpID, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUIListItemCtrl* pCtrl=debug_new(CLTGUIListItemCtrl);

	if (nFont < 0)
	{
		nFont = g_pLayoutMgr->GetScreenFontFace((eScreenID)m_nScreenID);
	}

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);

	if (!pCtrl->Create(sString, 0, commandID, helpID, pFont, nFontSize, this))
	{
		debug_delete(pCtrl);
		return LTNULL;
	}

	if (pos.x < 0 && pos.y < 0)
	{
		pos = m_nextPos;
	}
	else
	{
		m_nextPos = pos;
	}
	pCtrl->SetBasePos(pos);

	m_nextPos.y += (pCtrl->GetHeight() + m_nItemSpacing);

	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());

	if (bFixed)
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pCtrl;
}

// RETAIL: 0x1000a0f0
bool CBaseScreenLightCycle::IsTabSelected()
{
	CLTGUICtrl *pCtrl = GetControl(m_nSelection);
	if (pCtrl == m_pPlayerConfigTab || pCtrl == m_pCircuitTab ||
		pCtrl == m_pCustomGameTab || pCtrl == m_pCustomMapTab)
		return true;
	return false;
}

// RETAIL: 0x1000a130
LTRect CBaseScreenLightCycle::GetBodyRect()
{
	return g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_BASE_LIGHTCYCLE,"BodyRect");
}

// RETAIL: 0x1000a170
void CBaseScreenLightCycle::UpdateSettingsList()
{
	if (!m_pSettingsList) return;

	CLightCycleGameData *pCurrent = g_pLightCycleGameDataMgr->GetCurrent();
	std::wstring sName;
	m_pSettingsList->RemoveAll();

	StringList lstNames;
	g_pLightCycleGameDataMgr->GetNames(lstNames);

	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight((uint8)s_nListFontSize);

	int nIndex = 0;
	for (StringList::iterator it = lstNames.begin(); it != lstNames.end(); ++it, ++nIndex)
	{
		LTGUIWiden(sName,(*it).c_str());
		CLTGUIListItemCtrl *pItem = CreateListItem(sName,kCmdLCSelectSettings,0);
		pItem->SetFont(pFont,(uint8)s_nListFontSize);
		pItem->SetParam1(nIndex);
		m_pSettingsList->AddControl(pItem);

		if ((*it) == pCurrent->m_sName || !nIndex)
			m_pSettingsList->SetSelection(nIndex);
	}
}

// RETAIL: 0x1000a3a0: the light cycle player name
void CBaseScreenLightCycle::LoadSettings()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return;

	m_sPlayerName = pProfile->m_sLightCycleName;

	std::wstring sName;
	LTGUIWiden(sName,m_sPlayerName.c_str());
	m_pName->SetString(1,sName);
}

// RETAIL: 0x1000a560
void CBaseScreenLightCycle::SaveSettings()
{
	UpdateData(LTTRUE);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return;

	pProfile->m_sLightCycleName = m_sPlayerName;
	g_pLightCycleGameDataMgr->SaveCurrent();
}

// RETAIL: 0x1000a600
bool CBaseScreenLightCycle::GetProfileCycleColor(LIGHTCYCLECOLOR **ppColor, uint8 *pnStyle, uint8 *pnShade)
{
	if (!pnStyle || !ppColor) return false;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return false;

	*ppColor = g_pLightCycleButeMgr->GetShadeCycleColor(pProfile->m_nLightCycleStyle,pProfile->m_nLightCycleShade);
	*pnStyle = pProfile->m_nLightCycleStyle;
	*pnShade = pProfile->m_nLightCycleShade;
	return true;
}

// RETAIL: 0x1000a660
bool CBaseScreenLightCycle::IsCycleColorLocked(const char *pszColor)
{
	if (g_pCheatMgr->IsCheatActive(kCheatTronUnknown37))
		return false;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	StringArray::iterator it;
	StringArray::iterator itEnd = pProfile->m_UnlockedCycleColors.end();
	for (it = pProfile->m_UnlockedCycleColors.begin(); it != itEnd; ++it)
	{
		if (LCNameEquals(*it,pszColor))
			break;
	}
	return (it == pProfile->m_UnlockedCycleColors.end());
}
