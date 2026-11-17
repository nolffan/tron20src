// ----------------------------------------------------------------------- //
//
// MODULE  : MenuIntel.cpp
//
// PURPOSE : In-game intel item menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuIntel.h"
#include "InterfaceMgr.h"
#include "ClientResShared.h"
#include "MissionMgr.h"

namespace
{
	enum eLocalMenuCmds
	{
		MC_INTEL = MC_CUSTOM,
	};
}



// RETAIL: Tron's e-mails menu, CMenuEmails (0x101c5fa0)
LTBOOL CMenuIntel::Init()
{
	m_MenuID = MENU_ID_EMAILS;

	if (!CBaseMenu::Init()) return LTFALSE;

	// RETAIL: string 109 (0x6d)
	SetTitle(109);

	g_pInterfaceMgr->GetMenuMgr()->RegisterCommand(COMMAND_ID_VIEW_EMAIL,MENU_ID_EMAILS);

	m_PopupText.Init(false);

	return LTTRUE;
}

void CMenuIntel::Term()
{
	CBaseMenu::Term();

	m_PopupText.Term();
}

uint32 CMenuIntel::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return 1;
	}

	switch (nCommand)
	{
		case MC_INTEL:
		{
			// RETAIL: Tron's deselects the list's control while the popup
			// shows (0x101c601a)
			CLTGUICtrl* pSel = m_List.GetControl(m_List.GetSelectedIndex());
			if (pSel)
			{
				pSel->Select(LTFALSE);
			}

			uint16 nID = (uint16) nParam1;
			CIntelItemList *pList = g_pPlayerStats->GetIntelList();
			INTEL_ITEM* pItem = pList->Get(nID);

			if (pItem)
			{
				m_PopupText.Show(pItem->nTextId, pItem->nPopupId, pItem->szName);	// RETAIL: with its image (0x1007f630)

				// Disable menu now that popup is visible.

				g_pInterfaceMgr->GetMenuMgr()->EnableMenuBar(false);
			}
		}

		default:
		{
			return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
		}
	}

	return 1;
}


// This is called when the screen gets or loses focus
// TRON: the list holds the items of type 0 only (not the build notes), in
// the order found, in one color; a leading "<color...>" tag is skipped and the name is widened
// into a string of its own (RETAIL: 0x101c6090)
void CMenuIntel::OnFocus(LTBOOL bFocus)
{
	if (!m_PopupText.IsVisible())
	{
		ClearSelection();
	}

	if (bFocus)
	{
		std::wstring sName;
		uint16 nextItem = 0;
		CIntelItemList *pList = g_pPlayerStats->GetIntelList();
		char szTemp[512] = "";
		for (uint16 i = 0; i < pList->GetCount(); i++)
		{
			INTEL_ITEM* pItem = pList->Get(i);
			if (!pItem || pItem->nType)
			{
				continue;
			}

			GetIntelName(pItem->nTextId,szTemp,sizeof(szTemp));

			char* pName = szTemp;
			if (!_strnicmp(szTemp,"<color",6))
				pName = strchr(szTemp,'>') + 1;
			LTGUIWiden(sName, pName);

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
				uint16 ndx = AddControl(sName,MC_INTEL);
				pCtrl = (CLTGUITextCtrl*)m_List.GetControl(ndx);
				nextItem++;
			}

			if (pCtrl)
			{
				pCtrl->SetParam1(i);
				pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
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

		if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		{
			SetScale(g_pInterfaceResMgr->GetXRatio());
		}

		SetSelection(GetIndex(&m_List));
	}
}


void CMenuIntel::GetIntelName(uint32 nTextId, char *pBuf, int nBufSize)
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



// [KLS 6/23/02] - The following were overridden to support the m_PopupText data member...

void CMenuIntel::ClosePopup()
{
	m_PopupText.Close();
	// RETAIL: Tron's (0x101c64c0)
	g_pInterfaceMgr->UpdateCursorState();
	g_pLTClient->ClearInput();

	// Re-enable the menu since the popup is now closed

	g_pInterfaceMgr->GetMenuMgr()->EnableMenuBar(true);

	// RETAIL: and selects the list's control again
	CLTGUICtrl* pSel = m_List.GetControl(m_List.GetSelectedIndex());
	if (pSel)
	{
		pSel->Select(LTTRUE);
	}
}

void CMenuIntel::Render ( )
{
	if (!IsVisible()) return;

	CBaseMenu::Render();

	// If we're showing the intel item, render the popup...

	if (m_PopupText.IsVisible())
	{
		// Make sure the cursor isn't visible...
		g_pInterfaceMgr->UseCursor(LTFALSE, LTTRUE);

		m_PopupText.Update();
		m_PopupText.Draw();
	}
}

LTBOOL CMenuIntel::OnUp ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE;
	}

	return CBaseMenu::OnUp();
}

LTBOOL CMenuIntel::OnDown ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnDown();
}

LTBOOL CMenuIntel::OnLeft ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE;
	}

	return CBaseMenu::OnLeft();
}

LTBOOL CMenuIntel::OnRight ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnRight();
}

LTBOOL CMenuIntel::OnEnter ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnEnter();
}

LTBOOL CMenuIntel::OnEscape ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnEscape();
}

LTBOOL CMenuIntel::OnLButtonDown(int x, int y)
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnLButtonDown(x, y);
}

LTBOOL CMenuIntel::HandleKeyDown(int key, int rep)
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

