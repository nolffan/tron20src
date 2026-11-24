// ----------------------------------------------------------------------- //
//
// MODULE  : MenuSystem.cpp
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuSystem.h"
#include "InterfaceMgr.h"
#include "ClientResShared.h"
#include "ClientMultiplayerMgr.h"
#include "ClientSaveLoadMgr.h"
#include "VehicleMgr.h"
#include "CMoveMgr.h"
namespace
{
	enum eMenuCmds
	{
		// RETAIL: Tron's commands (CMenuSystem::OnCommand's jump table,
		// 0x101c80c8, and Init, 0x101c7df0): no host options, team before
		// player
		MC_LOAD = MC_CUSTOM,
		MC_SAVE,
		MC_OPTIONS,
		MC_TEAM,
		MC_PLAYER,
		MC_MAIN,
		MC_ABORT,
		MC_QUIT,
		MC_EXIT,
	};

	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CMenuSystem *pThisMenu = (CMenuSystem *)pData;
		if (bReturn && pThisMenu)
			pThisMenu->SendCommand(MC_EXIT,0,0);
	};

	void AbortCallBack(LTBOOL bReturn, void *pData)
	{
		CMenuSystem* pThisMenu = (CMenuSystem *)pData;
		if (bReturn && pThisMenu)
			pThisMenu->SendCommand(MC_MAIN,0,0);
	}


}

CMenuSystem::CMenuSystem( )
{
	m_pSaveCtrl = NULL;
	m_pLoadCtrl = NULL;
	m_pPlayerCtrl = NULL;
	m_pTeamCtrl = NULL;
}

LTBOOL CMenuSystem::Init()
{
	m_MenuID = MENU_ID_SYSTEM;

	if (!CBaseMenu::Init()) return LTFALSE;

	// RETAIL: Tron's string IDs (0x101c7e09..0x101c7eab)
	SetTitle(0x28);

	uint16 nLoadCtrlId = AddControl(IDS_LOADGAME,MC_LOAD);
	m_pLoadCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nLoadCtrlId );
	uint16 nSaveCtrlId = AddControl(0x52,MC_SAVE);
	m_pSaveCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nSaveCtrlId );

	AddControl(0x4d,MC_OPTIONS);

	// RETAIL: 0x101c7df0: no host options control in Tron
	uint16 nTeamCtrlId = AddControl(0x2eb,MC_TEAM);
	m_pTeamCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nTeamCtrlId );

	uint16 nPlayerCtrlId = AddControl(0x3f0,MC_PLAYER);
	m_pPlayerCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nPlayerCtrlId );
	
	AddControl(0x578,MC_ABORT);
	AddControl(0x4f,MC_QUIT);



	g_pInterfaceMgr->GetMenuMgr()->RegisterHotKey(VK_ESCAPE,MENU_ID_SYSTEM);

	return LTTRUE;
}

uint32 CMenuSystem::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	switch (nCommand)
	{
	case MC_LOAD:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LOAD);
		break;
	case MC_SAVE:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_SAVE);
		break;
	case MC_OPTIONS:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_OPTIONS);
		break;
	case MC_PLAYER:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PLAYER);
		break;
	// RETAIL: 0x101c7ed0: the team command goes to Tron's team selection
	// screen
	case MC_TEAM:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_TEAMSELECT);
		break;
	case MC_MAIN:
		{
			
			// Make sure we're disconnected from server.
			if(g_pLTClient->IsConnected())
			{
				g_pInterfaceMgr->SetIntentionalDisconnect( true );
				g_pClientMultiplayerMgr->ForceDisconnect();
			}

			// RETAIL: Tron's stops the music (0x101c7fa9)
			CMusic* pMusic = g_pGameClientShell->GetMusic();
			if (pMusic)
			{
				pMusic->Stop(LTDMEnactDefault);
			}

			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);

		}
		break;
	case MC_QUIT:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = QuitCallBack;
			mb.pData = this;
			// RETAIL: Tron's string IDs (0x101c7fe4, 0x101c8045, 0x101c8070)
			g_pInterfaceMgr->ShowMessageBox(0x297,&mb);
			break;
		}
	case MC_ABORT:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AbortCallBack;
			mb.pData = this;

			int nMsgID = 0x298;
			if (IsMultiplayerGame())
			{
				// Since we know we're starting a new server, ignore any disconnect messages.
				bool bIsLocalClient = false;
				g_pLTClient->IsLocalToServer(&bIsLocalClient);
				if( bIsLocalClient)
					nMsgID = 0x395;
				
			}
			g_pInterfaceMgr->ShowMessageBox(nMsgID,&mb);

			break;
		}
	case MC_EXIT:
		{
            g_pLTClient->Shutdown();
			break;
		}
	default:
		return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
	}
	return 1;
}

void CMenuSystem::OnFocus(LTBOOL bFocus)
{
	// RETAIL: Tron's (0x101c80f0) hides save and load in multiplayer and
	// disables them on a light cycle
	if( bFocus )
	{
		if (IsMultiplayerGame())
		{
			m_pSaveCtrl->Show( LTFALSE );
			m_pLoadCtrl->Show( LTFALSE );
			m_pTeamCtrl->Show( LTTRUE );
			m_pPlayerCtrl->Show( LTTRUE );
		}
		else
		{
			// RETAIL: the load control is not shown again (docs/RETAIL_BUGS.md)
			m_pSaveCtrl->Show( LTTRUE );
#ifdef TRON20SRC_MODERN
			m_pLoadCtrl->Show( LTTRUE );
#endif
			m_pSaveCtrl->Enable( g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() != PPM_LIGHTCYCLE );
			m_pLoadCtrl->Enable( g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() != PPM_LIGHTCYCLE );
			m_pTeamCtrl->Show( LTFALSE );
			m_pPlayerCtrl->Show( LTFALSE );
		}

		if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		{
			SetScale(g_pInterfaceResMgr->GetXRatio());
		}

		SetSelection(GetIndex(&m_List));
		m_List.ClearSelection();
		m_List.NextSelection();
	}
}
