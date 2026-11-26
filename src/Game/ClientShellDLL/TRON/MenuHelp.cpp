// ----------------------------------------------------------------------- //
//
// MODULE  : MenuHelp.cpp
//
// PURPOSE : In-game help menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (retail CShell.dll 0x101c7750..0x101c7d5f), written from
// the retail code on CMenuIntel's pattern.

#include "stdafx.h"
#include "MenuHelp.h"
#include "MenuMgr.h"
#include "InterfaceMgr.h"
#include "ClientResShared.h"
#include "TRONPlayerStats.h"

namespace
{
	enum eLocalMenuCmds
	{
		MC_HELPFILE = MC_CUSTOM,
	};
}


// RETAIL: 0x101c7750
LTBOOL CMenuHelp::Init()
{
	m_MenuID = MENU_ID_HELP;

	if (!CBaseMenu::Init()) return LTFALSE;

	// RETAIL: string 20 (0x14)
	SetTitle(20);

	g_pInterfaceMgr->GetMenuMgr()->RegisterCommand(COMMAND_ID_VIEW_HELP,MENU_ID_HELP);

	m_PopupText.Init(false);

	return LTTRUE;
}

// RETAIL: the same code as CMenuBuildNotes' (folded at 0x101c5a30)
void CMenuHelp::Term()
{
	CBaseMenu::Term();

	m_PopupText.Term();
}

// RETAIL: 0x101c77a0
uint32 CMenuHelp::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return 1;
	}

	if (nCommand != MC_HELPFILE)
	{
		return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
	}

	ShowItem((uint16)nParam1, (uint16)nParam2);
	return 1;
}


// RETAIL: 0x101c7c50. Shows the item, and marks it read
void CMenuHelp::ShowItem(uint16 nCtrlIndex, uint16 nItemIndex)
{
	CIntelItemList *pList = g_pPlayerStats->GetIntelList();
	INTEL_ITEM* pItem = pList->Get(nItemIndex);
	if (!pItem) return;

	if (!pItem->bRead)
	{
		pItem->bRead = true;

		CLTGUICtrl* pCtrl = m_List.GetControl(nCtrlIndex);
		if (pCtrl)
		{
			pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		}
	}

	m_PopupText.Show(pItem->nTextId, pItem->nPopupId, pItem->szName);	// RETAIL: with its image (0x1007f630)

	// Disable menu now that popup is visible.

	g_pInterfaceMgr->GetMenuMgr()->EnableMenuBar(false);
}


// RETAIL: 0x101c77f0. The help files and e-mails (items of type 2 and
// up); the unread ones in a colour of their own
void CMenuHelp::OnFocus(LTBOOL bFocus)
{
	if (!m_PopupText.IsVisible())
	{
		ClearSelection();
	}

	if (bFocus)
	{
		// PORT: retail calls CPlayerStats' last slot (0x164: the I/O node
		// pings' RemovePing, 0x100ac180), which our CPlayerStats has not
		g_pTronPlayerStats->RemovePing("helpfile");

		int nFirstCtrl = -1;
		int nFirstItem = -1;
		std::wstring sName;
		uint16 nextItem = 0;
		CIntelItemList *pList = g_pPlayerStats->GetIntelList();
		char szTemp[512] = "";
		uint16 nCtrlIndex = 0;
		for (uint16 i = 0; i < pList->GetCount(); i++)
		{
			INTEL_ITEM* pItem = pList->Get(i);
			if (!pItem || pItem->nType < 2)
			{
				continue;
			}

			uint16 nCtrl = nCtrlIndex++;
			if (nFirstCtrl < 0)
			{
				nFirstCtrl = nCtrl;
				nFirstItem = i;
			}

			GetIntelName(pItem->nTextId,szTemp,sizeof(szTemp));

			char* pName = szTemp;
			if (_strnicmp(szTemp,"<color",6) == 0)
			{
				pName = strchr(szTemp,'>') + 1;
			}

			sName.erase();
			uint32 nLen = strlen(pName);
			for (uint32 n = 0; n < nLen; n++)
			{
				sName += (wchar_t)(uint8)pName[n];
			}

			CLTGUITextCtrl* pCtrl = LTNULL;
			if (nextItem < m_List.GetNumControls())
			{
				pCtrl = (CLTGUITextCtrl*)m_List.GetControl(nextItem);
				if (pCtrl)
				{
					pCtrl->SetString(sName);
					pCtrl->Show(LTTRUE);
					nextItem++;
				}
			}
			else
			{
				uint16 ndx = AddControl(sName,MC_HELPFILE);
				pCtrl = (CLTGUITextCtrl*)m_List.GetControl(ndx);
				nextItem++;
			}

			if (pCtrl)
			{
				pCtrl->SetParam1(nCtrl);
				pCtrl->SetParam2(i);
				pCtrl->SetColors(m_SelectedColor,(pItem->bRead ? m_NonSelectedColor : 0xFFFFCB00),m_DisabledColor);
			}
		}

		for (uint16 nIndex = nextItem; nIndex < m_List.GetNumControls(); nIndex++)
		{
			CLTGUITextCtrl* pCtrl = (CLTGUITextCtrl*)m_List.GetControl(nIndex);
			if (pCtrl)
			{
				pCtrl->Show(LTFALSE);
			}
		}

#ifdef TRON20SRC_MODERN
		// MODERN.md CL-30: every time: the page's margins can change with the
		// same scale (1920x1080 and 2560x1080)
		if (true)
#else
		if (m_fScale != g_pInterfaceResMgr->GetXRatio())
#endif
		{
			SetScale(g_pInterfaceResMgr->GetXRatio());
		}

		SetSelection(GetIndex(&m_List));

		// RETAIL: shows the first file at once when it is new. Retail tests
		// the item at the control's index, not the item's (see
		// docs/RETAIL_BUGS.md)
		if (g_pInterfaceMgr->GetMenuMgr()->GetTronUnknown5CE4() && nFirstCtrl >= 0)
		{
#ifdef TRON20SRC_MODERN
			// MODERN.md GAME-125: the first file's item, by its item index
			INTEL_ITEM* pItem = pList->Get(nFirstItem);
#else
			INTEL_ITEM* pItem = pList->Get(nFirstCtrl);
#endif
			if (pItem && !pItem->bRead)
			{
				ShowItem(nFirstCtrl, nFirstItem);
			}
		}
	}
}


// RETAIL: the same code as CMenuIntel's (folded at 0x101c6440)
void CMenuHelp::GetIntelName(uint32 nTextId, char *pBuf, int nBufSize)
{
	LoadString(nTextId,pBuf,nBufSize);
	if (strchr(pBuf,'@'))
	{
		char* pTok = strtok(pBuf,"@");
	}
	else
	{
		char* pTok = strtok(pBuf,"\n\t,");
		int i = 24;
		int len = strlen(pBuf);
		if (len <= i) i = len;
		for (int j = 0; j < 3; j++)
			pBuf[i+j] = '.';
		pBuf[i+j] = 0;
		
	}
};


// RETAIL: 0x101c7ce0
void CMenuHelp::ClosePopup()
{
	m_PopupText.Close();
	g_pInterfaceMgr->UpdateCursorState();
	g_pLTClient->ClearInput();

	// Re-enable the menu since the popup is now closed

	g_pInterfaceMgr->GetMenuMgr()->EnableMenuBar(true);
}

// RETAIL: 0x101c7d20
void CMenuHelp::Render ( )
{
	if (!IsVisible()) return;

	CBaseMenu::Render();

	// If we're showing the file, render the popup...

	if (m_PopupText.IsVisible())
	{
		// Make sure the cursor isn't visible...
		g_pInterfaceMgr->UseCursor(LTFALSE, LTTRUE);

		m_PopupText.Update();
		m_PopupText.Draw();
	}
}

// RETAIL: the same code as CMenuBuildNotes' (folded at 0x101c5e80..0x101c5f70)
LTBOOL CMenuHelp::OnUp ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE;
	}

	return CBaseMenu::OnUp();
}

LTBOOL CMenuHelp::OnDown ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnDown();
}

LTBOOL CMenuHelp::OnLeft ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE;
	}

	return CBaseMenu::OnLeft();
}

LTBOOL CMenuHelp::OnRight ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnRight();
}

LTBOOL CMenuHelp::OnEnter ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnEnter();
}

LTBOOL CMenuHelp::OnEscape ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnEscape();
}

LTBOOL CMenuHelp::OnLButtonDown(int x, int y)
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnLButtonDown(x, y);
}

LTBOOL CMenuHelp::HandleKeyDown(int key, int rep)
{
	// Close popup if necessary...Use OnEscape
	// to handle escape key...
	if (m_PopupText.IsVisible() && (VK_ESCAPE != key))
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::HandleKeyDown(key, rep);
}
