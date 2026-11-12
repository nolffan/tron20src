// ----------------------------------------------------------------------- //
//
// MODULE  : TronInterfaceMgr.cpp
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MsgIDs.h"
#include "TronInterfaceMgr.h"
#include "TronPlayerMgr.h"
#include "TronTargetMgr.h"
#include "GameClientShell.h"
#include "TronMissionButeMgr.h"
#include "MissionMgr.h"
// RETAIL: retail's TU runs ScreenSubroutines.h's constant initializers
#include "ScreenSubroutines.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::CTronInterfaceMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronInterfaceMgr::CTronInterfaceMgr() : CInterfaceMgr()
{
	// PORT: retail's constructor (0x1009cxxx) is not yet written; ours
	// clears Tron's flags
	m_bTronUnknown28AE8 = false;
	m_bTronUnknown28B14 = false;
	m_nTronUnknown28B18 = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::~CTronInterfaceMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronInterfaceMgr::~CTronInterfaceMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::Init
//
//	PURPOSE:	One-time initialization
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::Init()
{
	m_SubroutineMgr.Init("attributes\\subroutines.txt");
	m_bDisplayProgress = false;

	return CInterfaceMgr::Init();
}

void CTronInterfaceMgr::Term()
{
	CInterfaceMgr::Term();
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnExitWorld()
//
//	PURPOSE:	Handle exiting a world
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnExitWorld()
{
	g_pMissionText->Clear();
	g_pChatInput->OnExitWorld();
	g_pSubtitles->Clear();

	m_bDisplayProgress = false;

	// Now handle the base class
	CInterfaceMgr::OnExitWorld();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnCommandOn()
//
//	PURPOSE:	Handle command on
//
// ----------------------------------------------------------------------- //
LTBOOL CTronInterfaceMgr::OnCommandOn(int command)
{
    if (g_pChatInput->IsVisible()) return LTTRUE;

	// Take appropriate action

	switch (command)
	{
		case COMMAND_ID_MESSAGE :
		{
			if (m_eGameState == GS_PLAYING && !g_pPlayerMgr->IsSpectatorMode())
			{
				if (m_AmmoChooser.IsOpen())
				{
					m_AmmoChooser.Close();
				}
				if (m_WeaponChooser.IsOpen())
				{
					m_WeaponChooser.Close();
				}

                g_pChatInput->Show(true, false);	// PORT: NOLF2's, with the team flag
                
			}

            return LTTRUE;
		}
		break;

		case COMMAND_ID_SUBROUTINE_MENU :
		{
			if (m_eGameState == GS_PLAYING)
			{
				SwitchToScreen(SCREEN_ID_SUBROUTINES);
				g_pSubroutineMgr->PopulateSubroutineScreen();
			}
		}
		break;

		case COMMAND_ID_TOGGLE_PROGRESS :
		{
			if (m_eGameState == GS_PLAYING)
			{
				m_bDisplayProgress = m_bDisplayProgress ? false : true;
			}
		}
		break;

		default :
		{
			return CInterfaceMgr::OnCommandOn(command);
		}
		break;
	}
    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::UpdatePlayerStats()
//
//	PURPOSE:	Handle server messages that affect tron-specific player stats
//
// ----------------------------------------------------------------------- //
void CTronInterfaceMgr::UpdatePlayerStats(uint8 nThing, uint8 nType1, uint8 nType2, LTFLOAT fAmount)
{
	switch (nThing)
	{
		case IC_REGION_CHANGE :
		{
			m_stats.SetRequiredPermissions(nType1);
		}
		break;

		case IC_PSETS_ID : // Permissions from the server
		{
			m_stats.SetPermissions(nType1);
		}
		break;

		case IC_PERFORMANCE_RATING_ID :
		{
			m_stats.SetPerformanceRating((PerformanceRating)nType1,nType2);
		}
		break;

		case IC_BUILD_POINTS_ID : // When the build points change
		{
			uint16 iPoints = (uint16)fAmount;
			m_stats.SetJetVersion(iPoints);
		}
		break;

		default :
		{
			CInterfaceMgr::UpdatePlayerStats(nThing, nType1, nType2, fAmount);
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnMessage
//
//	PURPOSE:	Message from the server
//
// ----------------------------------------------------------------------- //
LTBOOL CTronInterfaceMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	switch(messageID)
	{
		case MID_SUBROUTINE_OBTAINED:
		{
  			char pName[64];
  			char pState[64];
  			char pCondition[64];
  
  			pMsg->ReadString(pName,64);
			pMsg->ReadString(pState,64);
  			pMsg->ReadString(pCondition,64);

			m_SubroutineMgr.GivePlayerSubroutine(pName, pState, pCondition);
            return LTTRUE;
		}

		case MID_ADDITIVE_OBTAINED:
		{
			char buf[1024];
			pMsg->ReadString(buf,1024);
			m_stats.GivePlayerAdditive(buf);
			return LTTRUE;
		}

		case MID_PROCEDURAL_OBTAINED:
		{
			char buf[1024];
			pMsg->ReadString(buf,1024);
			m_SubroutineMgr.GivePlayerProcedural(buf);
			return LTTRUE;
		}

		case MID_PRIMITIVE_OBTAINED:
		{
			char buf[1024];
			pMsg->ReadString(buf,1024);
			m_SubroutineMgr.GivePlayerPrimitive(buf);
			return LTTRUE;
		}

		case MID_QUERY_TARGET_PROPERTIES:
		{
			g_pTronPlayerMgr->GetTronTargetMgr()->HandleTargetPropertiesMessage(pMsg);
		}

		default:
		{
			// Call the base class
			return CInterfaceMgr::OnMessage(messageID,pMsg);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnEnterWorld()
//
//	PURPOSE:	Handle entering new world
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnEnterWorld(LTBOOL bRestoringGame)
{
	// Call down to the base class first
	CInterfaceMgr::OnEnterWorld(bRestoringGame);

	// RETAIL: the HUD items' flag is set first thing (0x1009d938)
	m_bTronUnknown28B14 = true;

	m_bDisplayProgress = false;

	// Now do our TRON specific stuff such as...
	// Telling the subroutine manager about the state
	// of system memory.
	TRONMISSION* pMission = (TRONMISSION*)g_pMissionButeMgr->GetMission(g_pMissionMgr->GetCurrentMission());
	char *szMem;
	
	if(pMission)
	{
		szMem = pMission->szSystemMemory;
	}
	else
	{
		szMem = NULL;
	}

	g_pSubroutineMgr->SetSystemMemoryConfiguration(szMem);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::UpdatePlayingState()
//
//	PURPOSE:	Update playing state
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::UpdatePlayingState()
{
	g_pSubroutineMgr->Update();
	CInterfaceMgr::UpdatePlayingState();
}

// ----------------------------------------------------------------------- //
//
// PORT: NOLF2's shared ClientMultiplayerMgr makes WON's server directory
// (ServerDir.dll, which Tron does not have: it browses and hosts through
// GameSpy). Until Tron's GameSpy code is written from the retail code, there
// is no directory: declared here without SERVERDIR_API, so that the link
// resolves the shared code's import to it.
//
// ----------------------------------------------------------------------- //

class IServerDirectory;
class ILTCSBase;
IServerDirectory *Factory_Create_IServerDirectory_Titan( bool bClientSide, ILTCSBase& ltCSBase, HMODULE hResourceModule )
{
	return NULL;
}
