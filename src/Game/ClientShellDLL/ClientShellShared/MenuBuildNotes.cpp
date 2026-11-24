// ----------------------------------------------------------------------- //
//
// MODULE  : MenuBuildNotes.cpp
//
// PURPOSE : In-game build notes menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (retail CShell.dll 0x101c59f0..0x101c5f9b), written from
// the retail code on CMenuIntel's pattern.

#include "stdafx.h"
#include "MenuBuildNotes.h"
#include "InterfaceMgr.h"
#include "ClientResShared.h"

namespace
{
	enum eLocalMenuCmds
	{
		MC_BUILDNOTE = MC_CUSTOM,
	};
}


// RETAIL: 0x101c59f0
LTBOOL CMenuBuildNotes::Init()
{
	m_MenuID = MENU_ID_BUILDNOTES;

	if (!CBaseMenu::Init()) return LTFALSE;

	// RETAIL: string 37 (0x25)
	SetTitle(37);

	m_PopupText.Init(false);

	return LTTRUE;
}

// RETAIL: 0x101c5a30
void CMenuBuildNotes::Term()
{
	CBaseMenu::Term();

	m_PopupText.Term();
}

// RETAIL: 0x101c5a50
uint32 CMenuBuildNotes::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return 1;
	}

	switch (nCommand)
	{
		case MC_BUILDNOTE:
		{
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


// RETAIL: 0x101c5ad0. The build notes (items of type 1), with the
// "<color...>" prefix of their names skipped
void CMenuBuildNotes::OnFocus(LTBOOL bFocus)
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
			if (!pItem || pItem->nType != 1)
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
				uint16 ndx = AddControl(sName,MC_BUILDNOTE);
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


// RETAIL: the same code as CMenuIntel's (folded at 0x101c6440)
void CMenuBuildNotes::GetIntelName(uint32 nTextId, char *pBuf, int nBufSize)
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


// RETAIL: the same code as CMenuHelp's (folded at 0x101c7ce0)
void CMenuBuildNotes::ClosePopup()
{
	m_PopupText.Close();
	g_pInterfaceMgr->UpdateCursorState();
	g_pLTClient->ClearInput();

	// Re-enable the menu since the popup is now closed

	g_pInterfaceMgr->GetMenuMgr()->EnableMenuBar(true);
}

// RETAIL: the same code as CMenuHelp's (folded at 0x101c7d20)
void CMenuBuildNotes::Render ( )
{
	if (!IsVisible()) return;

	CBaseMenu::Render();

	// If we're showing the note, render the popup...

	if (m_PopupText.IsVisible())
	{
		// Make sure the cursor isn't visible...
		g_pInterfaceMgr->UseCursor(LTFALSE, LTTRUE);

		m_PopupText.Update();
		m_PopupText.Draw();
	}
}

// RETAIL: 0x101c5e80..0x101c5f70 (CMenuHelp's fold into them)
LTBOOL CMenuBuildNotes::OnUp ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE;
	}

	return CBaseMenu::OnUp();
}

LTBOOL CMenuBuildNotes::OnDown ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnDown();
}

LTBOOL CMenuBuildNotes::OnLeft ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE;
	}

	return CBaseMenu::OnLeft();
}

LTBOOL CMenuBuildNotes::OnRight ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnRight();
}

LTBOOL CMenuBuildNotes::OnEnter ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnEnter();
}

LTBOOL CMenuBuildNotes::OnEscape ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnEscape();
}

LTBOOL CMenuBuildNotes::OnLButtonDown(int x, int y)
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnLButtonDown(x, y);
}

LTBOOL CMenuBuildNotes::HandleKeyDown(int key, int rep)
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
