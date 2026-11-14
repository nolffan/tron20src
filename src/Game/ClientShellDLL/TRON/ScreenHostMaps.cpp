// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostMaps.cpp
//
// PURPOSE : Tron's host screen: the maps of the game type to play
//
// RETAIL: written from retail CShell.dll 0x10060c20..0x10062ad0, after
// NOLF2's CScreenHostLevels (TO2/ScreenHostLevels.cpp)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostMaps.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ServerOptionMgr.h"
#include "LTGUIListItemCtrl.h"
#include "InterfaceMgr.h"
#include <list>

// RETAIL: the folders of the game types' maps (0x1026e9b8: both disc games;
// 0x1026e9a8)
static std::string s_sDiscMapDir = "Worlds\\RetailMultiPlayer\\DiscArena";
static std::string s_sLightCycleMapDir = "Worlds\\RetailLightCycle";

namespace
{
	// RETAIL: 0x10218f4c
	uint8	s_nListFontSize = 16;

	// RETAIL: the most maps a game takes (NOLF2's MAX_GAME_LEVELS)
	const uint16 kMaxMaps = 50;

	// RETAIL: Tron's command IDs: the selected map down, up the list
	const uint32 kCmdHostMapDown	= 0x43D;
	const uint32 kCmdHostMapUp		= 0x43E;

	// RETAIL: 0x10060e40: the name after the path's last backslash (the
	// function name is ours)
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
}

// RETAIL: 0x10060e70
CScreenHostMaps::CScreenHostMaps()
{
	m_pAvailMissions = LTNULL;
	m_pSelMissions = LTNULL;
	m_pAdd = LTNULL;
	m_pRemove = LTNULL;
	m_pAddAll = LTNULL;
	m_pRemoveAll = LTNULL;
	m_pMoveDown = LTNULL;
	m_pMoveUp = LTNULL;
}

// RETAIL: 0x10060ef0
CScreenHostMaps::~CScreenHostMaps()
{
}

// RETAIL: 0x10060fe0
LTBOOL CScreenHostMaps::Build()
{
	LTBOOL bRet = CBaseScreenHost::Build();

	m_pAdd = AddTextItem(0xE6, CMD_ADD_LEVEL, 0x216, g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"CommandPos"));
	m_pAddAll = AddTextItem(0xE8, CMD_ADD_ALL, 0x217);
	m_pRemove = AddTextItem(0xE7, CMD_REMOVE_LEVEL, 0x218);
	m_pRemoveAll = AddTextItem(0xE9, CMD_REMOVE_ALL, 0x219);
	m_pMoveDown = AddTextItem(0xCA9, kCmdHostMapDown, 0xCAA);
	m_pMoveUp = AddTextItem(0xCAB, kCmdHostMapUp, 0xCAC);

	LTRect rcAvailRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"AvailRect");
	CLTGUITextCtrl *pTitle = AddTextItem(0xCE5, 0, 0, LTIntPt(rcAvailRect.left,rcAvailRect.top - 20));
	pTitle->Enable(LTFALSE);
	m_nAvailWidth = (rcAvailRect.right - rcAvailRect.left) - 32;
	m_pAvailMissions = AddList(LTIntPt(rcAvailRect.left,rcAvailRect.top),rcAvailRect.bottom - rcAvailRect.top,LTTRUE,m_nAvailWidth);
	m_pAvailMissions->SetIndent(LTIntPt(5,5));
	m_pAvailMissions->SetFrameWidth(2);
	// RETAIL: and clears a member of Tron's list control (+0x66c): left
	// out, ours has no such member

	LTRect rcSelRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"SelectRect");
	pTitle = AddTextItem(0xCE6, 0, 0, LTIntPt(rcSelRect.left,rcSelRect.top - 20));
	pTitle->Enable(LTFALSE);
	m_nSelWidth = (rcSelRect.right - rcSelRect.left) - 32;
	m_pSelMissions = AddList(LTIntPt(rcSelRect.left,rcSelRect.top),rcSelRect.bottom - rcSelRect.top,LTTRUE,m_nSelWidth);
	m_pSelMissions->SetIndent(LTIntPt(5,5));
	m_pSelMissions->SetFrameWidth(2);
	// RETAIL: (+0x66c of Tron's list control, as above)

	return bRet;
}

// RETAIL: 0x10061300
void CScreenHostMaps::Escape()
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

	CBaseScreen::Escape();
}

// RETAIL: 0x10061380
uint32 CScreenHostMaps::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
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
				CLTGUICtrl *pCtrl = m_pAvailMissions->GetSelectedControl();
				if (pCtrl)
				{
					AddMapToList(pCtrl->GetParam1());
					m_pSelMissions->ClearSelection();
				}
			}
			UpdateButtons();
		} break;
	case CMD_ADD_ALL:
		{
			if (m_pAvailMissions->GetNumControls())
			{
				for (int i = 0; i < m_pAvailMissions->GetNumControls() && (m_pSelMissions->GetNumControls() < kMaxMaps); i++)
				{
					CLTGUICtrl *pCtrl = m_pAvailMissions->GetControl(i);
					if (pCtrl)
					{
						AddMapToList(pCtrl->GetParam1());
					}
				}
			}
			m_pSelMissions->ClearSelection();
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
	case CMD_REMOVE_ALL:
		{
			if (m_pSelMissions->GetNumControls() > 0)
			{
				m_pSelMissions->ClearSelection();
				m_pSelMissions->RemoveAll();
			}
			UpdateButtons();
		} break;
	case kCmdHostMapDown:
		{
			int nIndex = m_pSelMissions->GetSelectedIndex();
			if (nIndex < m_pSelMissions->GetNumControls() - 1)
			{
				// PORT: here Tron's list control swaps the map with the next
				// (CLTGUIListCtrl 0x100b8cb0): left out, ours cannot yet
				m_pSelMissions->SetSelection(nIndex + 1);
			}
		} break;
	case kCmdHostMapUp:
		{
			int nIndex = m_pSelMissions->GetSelectedIndex();
			if (nIndex > 0)
			{
				// PORT: here Tron's list control swaps the map with the one
				// before (CLTGUIListCtrl 0x100b8c60): left out, as above
				m_pSelMissions->SetSelection(nIndex - 1);
			}
		} break;
	default:
		return CBaseScreenHost::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// RETAIL: 0x10061600
void CScreenHostMaps::OnFocus(LTBOOL bFocus)
{
	CBaseScreenHost::OnFocus(bFocus);

	if (bFocus)
	{
		SetTabFrame(m_pMapsTab);
		LoadOptions();
	}
	else
	{
		m_pAvailMissions->RemoveAll();
		m_pSelMissions->RemoveAll();
	}
}

// RETAIL: 0x10061650
void CScreenHostMaps::ScreenDimsChanged()
{
	CBaseScreen::ScreenDimsChanged();
	SetTabFrame(m_pMapsTab);
}

// RETAIL: 0x10061670: CBaseScreen's, but the lists stay selected
uint16 CScreenHostMaps::SetSelection(uint16 select, LTBOOL bFindSelectable)
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

// RETAIL: 0x100618b0
void CScreenHostMaps::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	HandleEditResult(dwParam1,dwParam2);
}

// RETAIL: 0x100618d0
LTBOOL CScreenHostMaps::FillAvailList()
{
	if (!m_pAvailMissions) return LTFALSE;

	m_saMaps.clear();

	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
	std::string sDir;
	switch (pOptions->m_eGameType)
	{
	case eGameTypeDiscArena:
	case eGameTypeDiscTournament:
		sDir = s_sDiscMapDir;
		break;
	case eGameTypeMultiplayerLightcycles:
		sDir = s_sLightCycleMapDir;
		break;
	default:
		return LTFALSE;
	}

	FindMaps(sDir);

	// sort the maps by name
	StringList lstMaps;
	StringArray::iterator iter;
	for (iter = m_saMaps.begin(); iter != m_saMaps.end(); ++iter)
		lstMaps.push_back(*iter);
	lstMaps.sort();
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
		sText.resize(0);
		uint32 nLen = strlen(pszName);
		for (uint32 i = 0; i < nLen; i++)
			sText += (wchar_t)(uint8)pszName[i];

		CLTGUIListItemCtrl *pItem = CreateListItem(sText,CMD_ADD_LEVEL,0);
		pItem->SetFont(pFont,s_nListFontSize);
		pItem->SetParam1(nIndex);
		pItem->SetFixedWidth(m_nAvailWidth,LTTRUE);
		m_pAvailMissions->AddControl(pItem);
	}

	return LTTRUE;
}

// RETAIL: 0x10061d90: the folder's .dat files, without the extension
void CScreenHostMaps::FindMaps(const std::string &sDir)
{
	if (sDir.empty()) return;

	FileEntry *pFiles = g_pLTClient->GetFileList(sDir.c_str());
	if (!pFiles) return;

	FileEntry *pFile = pFiles;
	while (pFile)
	{
		if (pFile->m_Type == TYPE_FILE &&
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

// RETAIL: 0x10061fa0
void CScreenHostMaps::UpdateButtons()
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

// RETAIL: 0x10062040
void CScreenHostMaps::AddMapToList(uint32 nMap)
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

// RETAIL: 0x10062250
void CScreenHostMaps::LoadOptions()
{
	CBaseScreenHost::LoadOptions();

	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
	FillAvailList();
	m_pSelMissions->RemoveAll();

	StringArray *pMaps = pOptions->GetMapList();
	for (StringArray::iterator iter = pMaps->begin(); iter != pMaps->end(); ++iter)
	{
		StringArray::iterator iterFound = std::find(m_saMaps.begin(),m_saMaps.end(),*iter);
		if (iterFound != m_saMaps.end())
			AddMapToList(iterFound - m_saMaps.begin());
	}

	UpdateButtons();
	UpdateData(LTFALSE);
	SetSelection(GetIndex(m_pMapsTab));
}

// RETAIL: 0x10062350
void CScreenHostMaps::SaveOptions()
{
	UpdateData(LTTRUE);

	if (!m_pSelMissions) return;

	StringArray *pMaps = g_pServerOptionMgr->GetCurrent()->GetMapList();
	pMaps->erase(pMaps->begin(),pMaps->end());

	for (int n = 0; n < m_pSelMissions->GetNumControls(); n++)
	{
		CLTGUICtrl *pCtrl = m_pSelMissions->GetControl(n);
		if (pCtrl && pCtrl->GetParam1() < m_saMaps.size())
			pMaps->push_back(m_saMaps[pCtrl->GetParam1()]);
	}

	g_pServerOptionMgr->SaveCurrent();
}
