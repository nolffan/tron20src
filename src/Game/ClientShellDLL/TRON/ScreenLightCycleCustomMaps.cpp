// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCycleCustomMaps.cpp
//
// PURPOSE : Tron's single player light cycle screen: the maps of a custom
//			 game, and launching it
//
// RETAIL: written from retail CShell.dll 0x10069890..0x1006bf80, after
// CScreenHostMaps (ScreenHostMaps.cpp), whose code it shares
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenLightCycleCustomMaps.h"
#include "ScreenSubroutines.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ProfileMgr.h"
#include "CheatMgr.h"
#include "MissionMgr.h"
#include "ClientMultiplayerMgr.h"
#include "CircuitButeMgr.h"
#include "LightCycleGameDataMgr.h"
#include "TRON/LightCycleButeMgr.h"
#include "GameClientShell.h"
#include "TronGameClientShell.h"
#include <list>

// RETAIL: 0x1026ea08: the folder of the light cycle maps (a caseless
// string: its initializer 0x10069910 tests the literal for NULL)
static CaselessString s_sMapDir = "Worlds\\RetailLightCycle\\";

namespace
{
	// RETAIL: 0x102199ac: the message when the profile's cycle is locked
	// (a variable)
	int		s_nCycleLockedId = 0x2613;

	// RETAIL: 0x102199b0
	uint8	s_nListFontSize = 16;

	// RETAIL: the most maps a game takes (NOLF2's MAX_GAME_LEVELS)
	const uint16 kMaxMaps = 50;

	// RETAIL: Tron's command IDs: launch (CBaseScreenHost's kCmdHostLaunch),
	// the selected map up, down the list
	const uint32 kCmdLaunch		= 0x2C;
	const uint32 kCmdMapUp		= 0x43D;
	const uint32 kCmdMapDown	= 0x43E;

	// RETAIL: the cheats that unlock every map and every cycle color (ours:
	// see CBaseScreenLightCycle)
	const int kCheatTronUnknown36 = 0x36;
	const int kCheatTronUnknown37 = 0x37;

	// RETAIL: 0x10060e40 (folded with CScreenHostMaps'): the name after the
	// path's last backslash (the function name is ours)
	const char* GetFileName(const char *pszPath)
	{
		if (!pszPath)
			return LTNULL;

		const char *p = pszPath + strlen(pszPath);
		while (p > pszPath)
		{
			if (*p == '\\')
				return p + 1;
			p--;
		}
		return p;
	}

	// RETAIL: the maps are sorted by their file names (the comparison's
	// list merge is 0x10062980, which both map screens call; names ours)
	struct FileNameLess
	{
		bool operator()(const std::string & sA, const std::string & sB) const
		{
			const char *pszA = GetFileName(sA.c_str());
			return stricmp(pszA, GetFileName(sB.c_str())) < 0;
		}
	};

	// RETAIL: 0x1006b510: a circuit's map is locked until the profile has
	// unlocked it. The circuit manager comes through Tron's game client
	// shell (g_pTronGameClientShell, 0x1026eda0)
	bool IsMapLocked(std::string sMap)
	{
		if (!sMap[0])
			return true;

		if (g_pCheatMgr->IsCheatActive(kCheatTronUnknown36))
			return false;

		if (g_pTronGameClientShell->GetCircuitButeMgr()->IsCircuitMap(sMap))
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			StringArray::iterator it;
			StringArray::iterator itEnd = pProfile->m_UnlockedMaps.end();
			for (it = pProfile->m_UnlockedMaps.begin(); it != itEnd; ++it)
			{
				if (LCNameEquals(*it,sMap))
					break;
			}
			if (it == pProfile->m_UnlockedMaps.end())
				return true;
		}
		return false;
	}
}

// RETAIL: 0x10069a10 (called from CTronScreenMgr::AddScreen's new)
CScreenLightCycleCustomMaps::CScreenLightCycleCustomMaps()
{
	m_pAdd = LTNULL;
	m_pRemove = LTNULL;
	m_pAddAll = LTNULL;
	m_pRemoveAll = LTNULL;
	m_pMoveUp = LTNULL;
	m_pMoveDown = LTNULL;
	m_pAvailMissions = LTNULL;
	m_pSelMissions = LTNULL;
	m_nAvailWidth = 0;
	m_nSelWidth = 0;
}

// RETAIL: 0x10069ab0
CScreenLightCycleCustomMaps::~CScreenLightCycleCustomMaps()
{
}

// RETAIL: 0x10069ba0
LTBOOL CScreenLightCycleCustomMaps::Build()
{
	LTBOOL bRet = CBaseScreenLightCycle::Build();
	CreateSettingsList();

	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);

	m_pAdd = AddTextItem(0x25A0, CMD_ADD_LEVEL, 0x25A1, g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"CommandPos"));
	m_pAddAll = AddTextItem(0x25A2, CMD_ADD_ALL, 0x25A3);
	m_pRemove = AddTextItem(0x25A4, CMD_REMOVE_LEVEL, 0x25A5);
	m_pRemoveAll = AddTextItem(0x25A6, CMD_REMOVE_ALL, 0x25A7);
	m_pMoveUp = AddTextItem(0x25A8, kCmdMapUp, 0x25A9);
	m_pMoveDown = AddTextItem(0x25AA, kCmdMapDown, 0x25AB);

	LTRect rcRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"AvailRect");
	CLTGUITextCtrl *pTitle = AddTextItem(0x25AC, 0, 0, LTIntPt(rcRect.left,rcRect.top - 20));
	pTitle->Enable(LTFALSE);
	m_nAvailWidth = (rcRect.right - rcRect.left) - 32;
	m_pAvailMissions = AddList(LTIntPt(rcRect.left,rcRect.top),rcRect.bottom - rcRect.top,LTTRUE,m_nAvailWidth);
	m_pAvailMissions->SetIndent(LTIntPt(5,5));
	m_pAvailMissions->SetFrameWidth(2);
	m_pAvailMissions->SetMouseMoveSelects(LTFALSE);

	rcRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"SelectRect");
	pTitle = AddTextItem(0x25AD, 0, 0, LTIntPt(rcRect.left,rcRect.top - 20));
	pTitle->Enable(LTFALSE);
	uint16 nHeight = rcRect.bottom - rcRect.top;
	m_nSelWidth = (rcRect.right - rcRect.left) - 32;
	m_pSelMissions = AddList(LTIntPt(rcRect.left,rcRect.top),nHeight,LTTRUE,m_nSelWidth);
	m_pSelMissions->SetIndent(LTIntPt(5,5));
	m_pSelMissions->SetFrameWidth(2);
	m_pSelMissions->SetMouseMoveSelects(LTFALSE);

	LTIntPt launchPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LaunchPos");
	CLTGUIButton *pLaunch = debug_new(CLTGUIButton);

	char szTex[128];
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,"LaunchTex",szTex,sizeof(szTex));
	HTEXTURE hNormal = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,"LaunchTexH",szTex,sizeof(szTex));
	HTEXTURE hHot = g_pInterfaceResMgr->GetTexture(szTex);

	pLaunch->Create(kCmdLaunch,0x25B6,hNormal,hHot);
	pLaunch->SetFont(g_pInterfaceResMgr->GetFontForHeight(nFontSize),nFontSize);
	pLaunch->SetText(LoadTempWString(0x25B5),LTTRUE);
	pLaunch->SetBasePos(launchPos);
	pLaunch->SetScale(g_pInterfaceResMgr->GetXRatio());
	pLaunch->SetCommandHandler(this);
	pLaunch->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_controlArray.push_back(pLaunch);

	return bRet;
}

// RETAIL: 0x1006a080
void CScreenLightCycleCustomMaps::Escape()
{
	if (m_pAvailMissions->IsEnabled())
	{
		m_pAdd->Enable(LTTRUE);
		SetSelection(GetIndex(m_pAdd));
	}
	else if (m_pSelMissions->IsEnabled())
	{
		m_pRemove->Enable(LTTRUE);
		SetSelection(GetIndex(m_pRemove));
	}

	CBaseScreenLightCycle::Escape();
}

// RETAIL: 0x1006a100
uint32 CScreenLightCycleCustomMaps::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}

	switch(dwCommand)
	{
	case CMD_BACK:
		{
			m_pScreenMgr->EscapeCurrentScreen();
		} break;
	case CMD_ADD_LEVEL:
		{
			if (m_pSelMissions->GetNumControls() < kMaxMaps)
			{
				if (m_pAvailMissions->GetSelectedControl())
				{
					AddMapToList(m_pAvailMissions->GetSelectedControl()->GetParam1());
					m_pSelMissions->ClearSelection();
				}
			}
			UpdateButtons();
		} break;
	case CMD_REMOVE_LEVEL:
		{
			int nIndex = m_pSelMissions->GetSelectedIndex();
			if (nIndex >= 0)
			{
				m_pSelMissions->RemoveControl(nIndex);
			}
			UpdateButtons();
		} break;
	case kCmdLaunch:
		{
			if (IsCycleUnlocked())
				LaunchGame();
			else
				ShowCycleLocked();
		} break;
	case CMD_ADD_ALL:
		{
			if (m_pAvailMissions->GetNumControls() != 0)
			{
				for (int i = 0; i < m_pAvailMissions->GetNumControls(); i++)
				{
					if (m_pSelMissions->GetNumControls() < kMaxMaps && m_pAvailMissions->GetControl(i))
						AddMapToList(m_pAvailMissions->GetControl(i)->GetParam1());
				}
			}
			m_pSelMissions->ClearSelection();
			UpdateButtons();
		} break;
	case kCmdMapDown:
		{
			int nIndex = m_pSelMissions->GetSelectedIndex();
			if (nIndex < m_pSelMissions->GetNumControls() - 1)
			{
				m_pSelMissions->MoveControlDown(nIndex);
				m_pSelMissions->SetSelection(nIndex + 1);
			}
		} break;
	case kCmdMapUp:
		{
			int nIndex = m_pSelMissions->GetSelectedIndex();
			if (nIndex > 0)
			{
				m_pSelMissions->MoveControlUp(nIndex);
				m_pSelMissions->SetSelection(nIndex - 1);
			}
		} break;
	case CMD_REMOVE_ALL:
		{
			if (m_pSelMissions->GetNumControls() > 0)
			{
				m_pSelMissions->ClearSelection();
				m_pSelMissions->RemoveAll();
			}
			UpdateButtons();
		} break;
	default:
		return CBaseScreenLightCycle::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// RETAIL: 0x1006a410: the cheat unlocks every cycle color
bool CScreenLightCycleCustomMaps::IsCycleUnlocked()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return false;

	LIGHTCYCLECOLOR *pColor = g_pLightCycleButeMgr->GetShadeCycleColor(pProfile->m_nLightCycleStyle,pProfile->m_nLightCycleShade);

	if (g_pCheatMgr->IsCheatActive(kCheatTronUnknown37))
		return true;

	pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return false;

	StringArray::iterator itColor;
	StringArray::iterator itColorEnd = pProfile->m_UnlockedCycleColors.end();
	for (itColor = pProfile->m_UnlockedCycleColors.begin(); itColor != itColorEnd; ++itColor)
	{
		if (LCNameEquals(*itColor,pColor->szName))
			break;
	}
	return (itColorEnd != itColor);
}

// RETAIL: 0x1006a4b0
void CScreenLightCycleCustomMaps::ShowCycleLocked()
{
	char szMsg[1024] = "";
	LoadString(s_nCycleLockedId,szMsg,sizeof(szMsg));

	std::wstring sMsg;
	LTGUIWiden(sMsg,szMsg);

	MBCreate mb;
	g_pInterfaceMgr->ShowMessageBox(sMsg,&mb);
}

// RETAIL: 0x1006a660
void CScreenLightCycleCustomMaps::LaunchGame()
{
	SaveSettings();

	CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
	if (!pData) return;

	StringArray *pMaps = pData->GetMaps(eGameTypeSingleLightCycleCustom);
	if (!pMaps->size()) return;

	g_pGameClientShell->SetGameType(eGameTypeSingleLightCycleCustom);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return;

	pData->m_bTronUnknownA0 = pProfile->m_nLightCycleStyle;
	pData->m_bTronUnknownA1 = pProfile->m_nLightCycleShade;
	SaveSettings();

	g_pClientMultiplayerMgr->SetupServerSinglePlayer();
	g_pMissionMgr->StartGameFromLevel((*pMaps)[0].c_str());
}

// RETAIL: 0x1006a700
void CScreenLightCycleCustomMaps::OnFocus(LTBOOL bFocus)
{
	CBaseScreenLightCycle::OnFocus(bFocus);

	if (!bFocus)
	{
		m_pAvailMissions->RemoveAll();
		m_pSelMissions->RemoveAll();
	}
}

// RETAIL: 0x1006a740: CBaseScreen's, but the lists stay selected
uint16 CScreenLightCycleCustomMaps::SetSelection(uint16 select, LTBOOL bFindSelectable)
{
	if (select == m_nSelection)
	{
		CLTGUICtrl *pCtrl = GetControl(select);
		if (pCtrl != m_pAvailMissions && pCtrl != m_pSelMissions)
			return select;
	}

	int nOldSelect=m_nSelection;

	if (select == kNoSelection)
	{
		if (nOldSelect != kNoSelection)
		{
			GetControl(nOldSelect)->Select(LTFALSE);
			OnSelectionChange();
		}
		m_nOldSelection = m_nSelection;
		m_nSelection = kNoSelection;
		UpdateHelpText();
		return kNoSelection;
	}

	CLTGUICtrl *pSelCtrl;

	if (select >= m_controlArray.size())
		select = m_controlArray.size()-1;

	pSelCtrl = GetControl(select);
	if (!pSelCtrl)
	{
		UpdateHelpText();
		return nOldSelect;
	}
	//check to see if we can select this item
	if (!pSelCtrl->IsEnabled())
	{
		//if we don't need to find a selectable item return
		if (!bFindSelectable)
		{
			UpdateHelpText();
			return nOldSelect;
		}

		//keep looking until we run out of on screen items or find a selectable one
		while (pSelCtrl && !pSelCtrl->IsEnabled())
		{
			select++;
			pSelCtrl = GetControl(select);
		}
		if (!pSelCtrl || !pSelCtrl->IsEnabled())
		{
			UpdateHelpText();
			return nOldSelect;
		}
	}

	if (nOldSelect != kNoSelection)
	{
		CLTGUICtrl *pOldCtrl = GetControl(nOldSelect);
		if (pOldCtrl != m_pAvailMissions && pOldCtrl != m_pSelMissions)
			GetControl(nOldSelect)->Select(LTFALSE);
	}

	m_nOldSelection = m_nSelection;
	m_nSelection = select;

	if (m_nSelection == kNoSelection)
	{
		UpdateHelpText();
		return nOldSelect;
	}

	LTIntPt pos = pSelCtrl->GetPos();
	if (m_bSelectFXCenter)
		pos.x += (pSelCtrl->GetWidth() / 2);
	pos.y += (pSelCtrl->GetHeight() / 2);
	g_pInterfaceMgr->ShowSelectFX(pos);
	pSelCtrl->Select(LTTRUE);

	PlayInterfaceSound(IS_CHANGE);

	UpdateHelpText();
	OnSelectionChange();
	return m_nSelection;
}

// RETAIL: 0x1006a980
LTBOOL CScreenLightCycleCustomMaps::FillAvailList()
{
	if (!m_pAvailMissions) return LTFALSE;

	m_saMaps.clear();

	FindMaps(s_sMapDir,IsMapLocked);

	// sort the maps by name
	StringList lstMaps;
	StringArray::iterator iter;
	for (iter = m_saMaps.begin(); iter != m_saMaps.end(); ++iter)
		lstMaps.push_back(*iter);
	lstMaps.sort(FileNameLess());
	m_saMaps.clear();
	StringList::iterator iterList;
	for (iterList = lstMaps.begin(); iterList != lstMaps.end(); ++iterList)
		m_saMaps.push_back(*iterList);

	m_pAvailMissions->ClearSelection();
	m_pAvailMissions->RemoveAll();

	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight(s_nListFontSize);
	uint32 nIndex = 0;
	std::wstring sText;
	for (iter = m_saMaps.begin(); iter != m_saMaps.end(); ++iter, ++nIndex)
	{
		const char *pszName = GetFileName(iter->c_str());
		LTGUIWiden(sText,pszName);

		CLTGUIListItemCtrl *pItem = CreateListItem(sText,CMD_ADD_LEVEL,0);
		pItem->SetFont(pFont,s_nListFontSize);
		pItem->SetParam1(nIndex);
		pItem->SetFixedWidth(m_nAvailWidth,LTTRUE);
		m_pAvailMissions->AddControl(pItem);
	}

	return LTTRUE;
}

// RETAIL: 0x1006adb0: the folder's .dat files, without the extension, that
// are not locked
void CScreenLightCycleCustomMaps::FindMaps(const CaselessString &sDir, bool (*pfnIsLocked)(std::string sMap))
{
	if (sDir.empty()) return;

	FileEntry *pFiles = g_pLTClient->GetFileList(sDir.c_str());
	if (!pFiles) return;

	FileEntry *pFile = pFiles;
	while (pFile)
	{
		if (pFile->m_Type == TYPE_FILE && pfnIsLocked &&
			!pfnIsLocked(std::string(pFile->m_pBaseFilename)) &&
			strnicmp(pFile->m_pBaseFilename + strlen(pFile->m_pBaseFilename) - 4,".dat",4) == 0)
		{
			char szName[256];
			strcpy(szName,pFile->m_pFullFilename);
			int nLen = strlen(szName);
			if (nLen > 4)
				szName[nLen - 4] = '\0';
			m_saMaps.push_back(std::string(szName));
		}
		pFile = pFile->m_pNext;
	}

	g_pLTClient->FreeFileList(pFiles);
}

// RETAIL: 0x1006b040
void CScreenLightCycleCustomMaps::UpdateButtons()
{
	if (!m_pSelMissions->GetNumControls())
	{
		m_pSelMissions->Enable(LTFALSE);
		m_pRemove->Enable(LTFALSE);
		m_pRemoveAll->Enable(LTFALSE);
		SetSelection(GetIndex(m_pAvailMissions));
	}
	else
	{
		m_pSelMissions->Enable(LTTRUE);
		m_pRemove->Enable(LTTRUE);
		m_pRemoveAll->Enable(LTTRUE);
	}
}

// RETAIL: 0x1006b0e0
void CScreenLightCycleCustomMaps::AddMapToList(uint32 nMap)
{
	if (!m_pSelMissions) return;
	if (m_pSelMissions->GetNumControls() == kMaxMaps) return;
	if (nMap >= m_saMaps.size()) return;

	const char *pszName = GetFileName(m_saMaps[nMap].c_str());
	std::wstring sText;
	LTGUIWiden(sText,pszName);

	CLTGUIListItemCtrl *pItem = CreateListItem(sText,CMD_REMOVE_LEVEL,0);
	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight(s_nListFontSize);
	pItem->SetFont(pFont,s_nListFontSize);
	pItem->SetParam1(nMap);
	pItem->SetFixedWidth(m_nSelWidth,LTTRUE);
	m_pSelMissions->AddControl(pItem);
}

// RETAIL: 0x1006b2f0: the current settings' custom maps
void CScreenLightCycleCustomMaps::LoadSettings()
{
	CBaseScreenLightCycle::LoadSettings();

	CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
	FillAvailList();
	m_pSelMissions->RemoveAll();

	StringArray *pMaps = pData->GetMaps(eGameTypeSingleLightCycleCustom);
	for (StringArray::iterator iter = pMaps->begin(); iter != pMaps->end(); ++iter)
	{
		StringArray::iterator iterFound;
		StringArray::iterator iterEnd = m_saMaps.end();
		for (iterFound = m_saMaps.begin(); iterFound != iterEnd; ++iterFound)
		{
			if (LCNameEquals(*iterFound,*iter))
				break;
		}
		if (iterFound != m_saMaps.end())
			AddMapToList(iterFound - m_saMaps.begin());
	}

	UpdateButtons();
	UpdateData(LTFALSE);
	SetSelection(GetIndex(GetTab()));
}

// RETAIL: 0x1006b3e0
void CScreenLightCycleCustomMaps::SaveSettings()
{
	UpdateData(LTTRUE);

	if (!m_pSelMissions) return;

	CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
	StringArray *pMaps = pData->GetMaps(eGameTypeSingleLightCycleCustom);
	pMaps->erase(pMaps->begin(),pMaps->end());

	for (int n = 0; n < m_pSelMissions->GetNumControls(); n++)
	{
		CLTGUICtrl *pCtrl = m_pSelMissions->GetControl(n);
		if (pCtrl && pCtrl->GetParam1() < m_saMaps.size())
			pMaps->push_back(m_saMaps[pCtrl->GetParam1()]);
	}

	g_pLightCycleGameDataMgr->SaveCurrent();
	CBaseScreenLightCycle::SaveSettings();
}

// RETAIL: 0x1006d860 (slot 39: folded with CScreenLightCycleGame's)
LTRect CScreenLightCycleCustomMaps::GetBodyRect()
{
	return g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"TabBodyRect");
}
