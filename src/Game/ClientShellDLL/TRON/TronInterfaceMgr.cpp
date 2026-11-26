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
#include "HUDTronDecision.h"
#include "TronInterfaceMgr.h"
#include "TronPlayerMgr.h"
#include "TronTargetMgr.h"
#include "GameClientShell.h"
#include "TronMissionButeMgr.h"
#include "MissionMgr.h"
#include "LightCycleMatchMgr.h"
// RETAIL: retail's TU runs ScreenSubroutines.h's constant initializers
#include "ScreenSubroutines.h"
#include "VarTrack.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "PlayerLureFX.h"
#include "ClientUtilities.h"
#include "SubroutineMgr.h"
#include "TronPlayerStats.h"
#include "ClientSoundMgr.h"
#include "TronHUDMgr.h"

// RETAIL: 0x1026ee38, set up by Init (the name is ours; nothing reads it)
static VarTrack s_vtIOScene;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::CTronInterfaceMgr
//
//	PURPOSE:	Constructor (RETAIL: 0x1009ca20)
//
// ----------------------------------------------------------------------- //

CTronInterfaceMgr::CTronInterfaceMgr() : CInterfaceMgr()
{
	// RETAIL: layout anchors (see the header): fail to compile when a
	// member's size changes (adjust the pads)
	typedef char TronCheckHUD[offsetof(CTronInterfaceMgr, m_HUD) == 0x10108 ? 1 : -1];
	typedef char TronCheckSubroutineMgr[offsetof(CTronInterfaceMgr, m_SubroutineMgr) == 0x24474 ? 1 : -1];
	typedef char TronCheckWeaponChooser[offsetof(CTronInterfaceMgr, m_WeaponChooser) == 0x26790 ? 1 : -1];
	typedef char TronCheckSize[sizeof(CTronInterfaceMgr) == 0x29490 ? 1 : -1];

	// RETAIL: CInterfaceMgr's loading screen (+0xf8e4) is the member
	// (0x1009cb97)
	SetLoadingScreen(&m_LoadingScreen);

	m_pTurnArrowLeft = LTNULL;
	m_pTurnArrowRight = LTNULL;
	m_hIOCamera = LTNULL;
	m_nTronUnknown28B18 = 0;
	m_bTronUnknown28B1C = false;
	m_pDuplicateNew = LTNULL;
	m_pDuplicateOld = LTNULL;
	m_szDuplicateName[0] = 0;
	m_szDuplicateVersion[0] = 0;
	m_szDuplicateState[0] = 0;
	m_eDuplicateRenderLevel = kHUDRenderFull;

	// RETAIL: 0x1009cb9d..: +0x28ae8, +0x28b14, +0x28b1d and +0x28af0 are
	// left alone here (OnEnterWorld and Update set them)
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::~CTronInterfaceMgr
//
//	PURPOSE:	Destructor (RETAIL: 0x1009ce40)
//
// ----------------------------------------------------------------------- //

CTronInterfaceMgr::~CTronInterfaceMgr()
{
	ClearTronRecords();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::Init
//
//	PURPOSE:	One-time initialization (RETAIL: 0x1009d010)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::Init()
{
	m_SubroutineMgr.Init("attributes\\subroutines.txt");

	LTBOOL bRet = CInterfaceMgr::Init();

	CreateIOCamera();

	m_ServerOptionMgr.Init();

	m_LightCycleGameDataMgr.Init();

	m_pTurnArrowLeft = g_pScreenSpriteMgr->CreateScreenSprite("Interface\\Spr\\TurnArrowLeft.spr", false, SPRITELAYER_DEFAULT);
	// RETAIL: 0x1009d075: the flag at +0x2c (the sprite hides when its
	// life ends), not m_bShow
	m_pTurnArrowLeft->m_bTronUnknown2C = LTTRUE;
	m_pTurnArrowRight = g_pScreenSpriteMgr->CreateScreenSprite("Interface\\Spr\\TurnArrowRight.spr", false, SPRITELAYER_DEFAULT);
	m_pTurnArrowRight->m_bTronUnknown2C = LTTRUE;

	m_DuplicateDlg.Init();

	s_vtIOScene.Init(g_pLTClient, "IOScene", NULL, 1.0f);

	m_pDuplicateNew = LTNULL;
	m_pDuplicateOld = LTNULL;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::Term
//
//	PURPOSE:	Termination (RETAIL: 0x1009d150)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::Term()
{
	if (m_pTurnArrowLeft)
	{
		g_pScreenSpriteMgr->DestroyScreenSprite(m_pTurnArrowLeft);
		m_pTurnArrowLeft = LTNULL;
	}

	if (m_pTurnArrowRight)
	{
		g_pScreenSpriteMgr->DestroyScreenSprite(m_pTurnArrowRight);
		m_pTurnArrowRight = LTNULL;
	}

	if (m_hIOCamera)
	{
		g_pLTClient->RemoveObject(m_hIOCamera);
		m_hIOCamera = LTNULL;
	}

	m_ServerOptionMgr.Term();

	m_LightCycleGameDataMgr.Term();

	CInterfaceMgr::Term();

	m_SubroutineMgr.Term();

	if (m_pDuplicateNew)
	{
		delete m_pDuplicateNew;
		m_pDuplicateNew = LTNULL;
	}

	m_DuplicateDlg.Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnExitWorld()
//
//	PURPOSE:	Handle exiting a world (RETAIL: 0x1009d200)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnExitWorld()
{
	g_pMissionText->Clear();
	g_pChatInput->OnExitWorld();
	g_pSubtitles->Clear(LTNULL);	// RETAIL: 0x1009d222

	m_bTronUnknown28AE8 = false;
	m_nTronUnknown28AF0 = 0;
	m_bDisplayProgress = false;
	m_nTronUnknown28B18 = 0;

	// Now handle the base class
	CInterfaceMgr::OnExitWorld();

	ClearTronRecords();

	m_DuplicateDlg.Hide();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnKeyDown()
//
//	PURPOSE:	Handle a key (RETAIL: 0x1009d260)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::OnKeyDown(int key, int rep)
{
	if (m_DuplicateDlg.IsVisible())
	{
		return LTTRUE;
	}

	// RETAIL: 0x1009d278: in the playing state escape belongs to an open
	// decision window (and closes it when it may) unless the chat input
	// is open (RETAIL: Tron's decision window, 0x1026edec)
	if (m_eGameState == GS_PLAYING && key == VK_ESCAPE && g_pTronDecision->IsVisible() && !g_pChatInput->IsVisible())
	{
		if (g_pTronDecision->GetTronUnknown4d())
		{
			g_pTronDecision->Hide(0xff);
		}
		return LTTRUE;
	}

	return CInterfaceMgr::OnKeyDown(key, rep);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::ShowChatInput()
//
//	PURPOSE:	Open the chat input (RETAIL: 0x1009d2e0; the name is ours)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::ShowChatInput(uint32 nChatType)
{
	if (m_eGameState == GS_PLAYING && !g_pPlayerMgr->IsSpectatorMode())
	{
		if (m_AmmoChooser.IsOpen())
		{
			m_AmmoChooser.Close();
		}
		// RETAIL: through the chooser's vtable (CTronWeaponChooser's Close
		// takes an argument, 1 here)
		if (m_WeaponChooser.IsOpen())
		{
			static_cast<CWeaponChooser*>(&m_WeaponChooser)->Close();
		}

		// RETAIL: the chat type goes to the chat input first (0x1009d32d)
		g_pChatInput->SetChatType(nChatType);
		g_pChatInput->Show(true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnCommandOn()
//
//	PURPOSE:	Handle command on (RETAIL: 0x1009d350)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::OnCommandOn(int command)
{
    if (g_pChatInput->IsVisible()) return LTTRUE;

	// RETAIL: its CHUDTronDecision takes COMMAND_ID_CHOOSE_1..6 first
	if (g_pTronDecision->IsVisible() && command >= COMMAND_ID_CHOOSE_1 && command <= COMMAND_ID_CHOOSE_6)
	{
		g_pTronDecision->Choose(command - COMMAND_ID_CHOOSE_1);
		return LTTRUE;
	}

	// Take appropriate action

	// RETAIL: the cases' code in this order (0x1009d3be..0x1009d43a)
	switch (command)
	{
		// RETAIL: 93 opens the team chat
		case COMMAND_ID_SEND_TEAM_MESSAGE :
		{
			ShowChatInput(2);
            return LTTRUE;
		}
		break;

		case COMMAND_ID_MESSAGE :
		{
			ShowChatInput(1);
            return LTTRUE;
		}
		break;

		case COMMAND_ID_NEXT_WEAPON_1 :
		case COMMAND_ID_NEXT_WEAPON_2 :
		case COMMAND_ID_NEXT_WEAPON_3 :
		case COMMAND_ID_NEXT_WEAPON_4 :
		case COMMAND_ID_NEXT_WEAPON_5 :
		case COMMAND_ID_NEXT_WEAPON_6 :
		{
			// RETAIL: not while Tron's decision window (0x1026edec) shows
			if (!g_pTronDecision->IsVisible())
			{
				NextWeapon(command);
			}
            return LTTRUE;
		}
		break;

		// RETAIL: 0x1009d405: the light cycle camera toggles (the player
		// manager's slot 0x24), and the profile keeps it
		case COMMAND_ID_TOGGLE_LC_CAMERA :
		{
			if (!static_cast<CTronPlayerMgr*>(g_pPlayerMgr)->GetTronUnknown1D9())
			{
				g_pPlayerMgr->TronUnknownVirtual24(!g_pPlayerMgr->GetTronUnknown1D7(), false);
				g_pProfileMgr->GetCurrentProfile()->SetLightCycleCamera();
			}
		}
		break;

		// RETAIL: 0x1009d43a: the manual light cycle camera (slot 0x28)
		case COMMAND_ID_MANUAL_LC_CAM :
		{
			g_pPlayerMgr->TronUnknownVirtual28(!g_pPlayerMgr->GetTronUnknown1D8());
			g_pProfileMgr->GetCurrentProfile()->SetLightCycleCamera();
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
//	ROUTINE:	CTronInterfaceMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off (RETAIL: 0x1009d510)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::OnCommandOff(int command)
{
	if (command == COMMAND_ID_SUBROUTINE_MENU)
	{
		// RETAIL: 0x1009d522: an open decision window keeps the screen shut
		// unless its +0x4d is set (RETAIL: Tron's decision window, 0x1026edec)
		bool bAllow = true;
		if (g_pTronDecision && g_pTronDecision->IsVisible() && !g_pTronDecision->GetTronUnknown4d())
		{
			bAllow = false;
		}

		// RETAIL: nor on a light cycle, in a multiplayer game, or while the
		// player dies
		if (m_eGameState == GS_PLAYING && g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() != PPM_LIGHTCYCLE &&
			!IsMultiplayerGame() && !g_pPlayerMgr->IsPlayerDead() && bAllow)
		{
			if (m_AmmoChooser.IsOpen())
			{
				m_AmmoChooser.Close();
			}
			if (m_WeaponChooser.IsOpen())
			{
				static_cast<CWeaponChooser*>(&m_WeaponChooser)->Close();
			}

			SwitchToScreen(SCREEN_ID_SUBROUTINES);
			g_pSubroutineMgr->PopulateSubroutineScreen();

			// RETAIL: the screen clears the HUD's pings for it
			g_pPlayerStats->RemovePing("subroutines");
			g_pPlayerStats->RemovePing("ratings");

			return LTTRUE;
		}

		// RETAIL: 0x1009d5fb: the team selection in the disc and light
		// cycle games
		GameType eGameType = g_pGameClientShell->GetGameType();
		if ((eGameType == eGameTypeDiscArena || eGameType == eGameTypeDiscTournament ||
			 eGameType == eGameTypeMultiplayerLightcycles) && m_eGameState == GS_PLAYING)
		{
			SwitchToScreen(SCREEN_ID_TEAMSELECT);
		}
		return LTTRUE;
	}

	return CInterfaceMgr::OnCommandOff(command);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::UpdatePlayerStats()
//
//	PURPOSE:	Handle server messages that affect tron-specific player stats (RETAIL: 0x1009d640)
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

		// RETAIL: Tron's energy (retail passes fAmount as it is)
		case IC_MAX_ENERGY_ID :
		{
			m_stats.UpdateMaxEnergy(fAmount);
		}
		break;

		case IC_ENERGY_ID :
		{
			m_stats.UpdateEnergy(fAmount);
		}
		break;

		case IC_RESET_INVENTORY_ID :
		{
			// RETAIL: 0x100948d0 (no arguments), then the stats' virtual at
			// +0x88 with nType1, then the base class
			if (nType2)
			{
				g_pSubroutineMgr->ResetPrimitives();
			}
			GetPlayerStats()->DropInventory((LTBOOL)nType1);
			CInterfaceMgr::UpdatePlayerStats(nThing, nType1, nType2, fAmount);
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
//	ROUTINE:	CTronInterfaceMgr::TronUnknownVirtual7C()
//
//	PURPOSE:	RETAIL: 0x1009d7a0 (slot 0x7c): picks the HUD's status string for a client
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::TronUnknownVirtual7C(CPlayerScore* pScore)
{
	if (GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		uint32 nLocalID = 0;
		g_pLTClient->GetLocalClientID(&nLocalID);
		int nTeam = m_TeamMgr.GetTeamOfClient(nLocalID);

		switch (pScore->m_nTronUnknown18)
		{
			default:
				m_nTronUnknown28B18 = 0;
				g_pHUDMgr->QueueUpdate(0x4000000);
				return;

			case 2:
				if (nTeam == 0x10)
				{
					m_nTronUnknown28B18 = 0xd8b;
					g_pHUDMgr->QueueUpdate(0x4000000);
					return;
				}
				m_nTronUnknown28B18 = 0;
				g_pHUDMgr->QueueUpdate(0x4000000);
				return;
		}
	}

	if (IsDiscGameType())
	{
		uint32 nLocalID = 0;
		g_pLTClient->GetLocalClientID(&nLocalID);

		if (IsDiscGameType() && nLocalID == pScore->m_nClientID)
		{
			int nTeam = m_TeamMgr.GetTeamOfClient(nLocalID);

			switch (pScore->m_nTronUnknown18)
			{
				case 0:
					m_nTronUnknown28B18 = 0;
					break;

				case 1:
					m_nTronUnknown28B18 = 0xd89;
					break;

				// RETAIL: state 3 is "Waiting for next point." and state 2
				// (a player in no match yet, the state a client starts in)
				// the team's string: retail's jump table (0x1009d918) sends
				// case 2 to 0x1009d8ae and case 3 to 0x1009d8a2; the bodies
				// stay in this order
				case 3:
					m_nTronUnknown28B18 = 0xd8a;
					break;

				case 2:
					if (nTeam == 0x10)
					{
						m_nTronUnknown28B18 = 0xd8b;
					}
					else if (GetGameType() == eGameTypeDiscTournament)
					{
						if (nTeam >= 0 && m_TeamMgr.GetTeam(nTeam, false)->GetState() == CTronTeam::kTronTeamState_Timed)
						{
							m_nTronUnknown28B18 = 0xd9b;
						}
						else
						{
							m_nTronUnknown28B18 = 0xd87;
						}
					}
					else
					{
						m_nTronUnknown28B18 = 0xd88;
					}
					break;
			}

			g_pHUDMgr->QueueUpdate(0x4000000);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::Update()
//
//	PURPOSE:	Per-frame update (RETAIL: 0x1009d930)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::Update()
{
	// RETAIL: 0x1009d933
	UpdateConsoleVarRecords();

	m_bTronUnknown28B14 = true;

	// RETAIL: 0x1009d93f: cleared while the player follows a lure that
	// allows no weapon
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX(g_pMoveMgr->GetVehicleMgr()->GetPlayerLureId());
	if (g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LURE && pPlayerLureFX && !pPlayerLureFX->GetAllowWeapon())
	{
		m_bTronUnknown28B14 = false;
	}

	if (IsMultiplayerGame() && g_pGameClientShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		g_pLightCycleMatchMgr->Update();
	}

	return CInterfaceMgr::Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnMessage
//
//	PURPOSE:	Message from the server (PORT: Build 69's; retail's is 0x1009d9b0)
//
// ----------------------------------------------------------------------- //
LTBOOL CTronInterfaceMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	switch(messageID)
	{
		case MID_SUBROUTINE_OBTAINED:
		{
			// RETAIL: 0x1009d9ed: "all" gives every subroutine; a pickup
			// waits for the duplicate subroutine box (0x1009f0c0)
  			char pName[64];
  			char pState[64];
  			char pCondition[64];
  
  			pMsg->ReadString(pName,64);
			pMsg->ReadString(pState,64);
  			pMsg->ReadString(pCondition,64);

			if (!stricmp(pName, "all"))
			{
				int nNumSubroutines = m_SubroutineMgr.GetNumSubroutines();
				for (int i = 0; i < nNumSubroutines; i++)
				{
					m_SubroutineMgr.GivePlayerSubroutine(m_SubroutineMgr.GetSubroutineByIndex(i)->szName, pState, pCondition, true);
				}
			}
			else
			{
				QueueSubroutinePickup(pName, pState, pCondition);
			}
            return LTTRUE;
		}

		// RETAIL: Tron's decision window (0x1009dacc)
		case MID_DECISION:
		{
			g_pTronDecision->Show(pMsg);
			return LTTRUE;
		}

		// RETAIL: Tron has no MID_ADDITIVE_OBTAINED case (CInterfaceMgr
		// gets it)

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

		// RETAIL: 0x1009db6a
		case MID_QUERY_TARGET_PROPERTIES:
		{
			g_pTronPlayerMgr->GetTronTargetMgr()->HandleTargetPropertiesMessage(pMsg);
			return LTTRUE;
		}

		// RETAIL: 0x1009db90: a character's ping on the HUD, added or
		// removed
		case MID_TRON_PING:
		{
			char buf[1024];
			pMsg->ReadString(buf,1024);
			if (pMsg->Readuint8())
			{
				g_pTronPlayerStats->AddPing(buf);
				return LTTRUE;
			}
			g_pTronPlayerStats->RemovePing(buf);
			return LTTRUE;
		}

		// RETAIL: 0x1009dc03: the I/O node camera, on at a position and a
		// rotation with an object, or off
		case MID_TRON_UNKNOWN_180:
		{
			m_bTronUnknown28AE8 = (pMsg->Readuint8() != 0);
			if (m_bTronUnknown28AE8)
			{
				LTVector vPos = pMsg->ReadCompLTVector();
				g_pLTClient->SetObjectPos(m_hIOCamera, &vPos, true);
				LTRotation rRot = pMsg->ReadCompLTRotation();
				g_pLTClient->SetObjectRotation(m_hIOCamera, &rRot);
				m_nTronUnknown28AF0 = (uint32)pMsg->ReadObject();
				return LTTRUE;
			}
			m_nTronUnknown28AF0 = 0;
			return LTTRUE;
		}

		// RETAIL: 0x1009dcdc
		case MID_TRON_UNKNOWN_188:
		{
			m_bDisplayProgress = (pMsg->Readuint8() != 0);
			return LTTRUE;
		}

		// RETAIL: 0x1009dd06: an optimizer opens the subroutines screen
		case MID_OPTIMIZER_BEGIN:
		{
			if (m_eGameState == GS_PLAYING)
			{
				SwitchToScreen(SCREEN_ID_SUBROUTINES, true);
				g_pSubroutineMgr->SetOptimizer(pMsg->ReadObject());
				g_pSubroutineMgr->PopulateSubroutineScreen();
			}
			return LTTRUE;
		}

		// RETAIL: 0x1009dd4b: a popup for the subroutines screen
		case MID_TRON_UNKNOWN_166:
		{
			uint32 nTextId = pMsg->Readuint32();
			uint8 nPopupId = pMsg->Readuint8();
			char szText[128];
			pMsg->ReadString(szText,128);
			CScreenSubroutines *pSubScreen = (CScreenSubroutines *)GetScreenMgr()->GetScreenFromID(SCREEN_ID_SUBROUTINES);
			pSubScreen->SetPopupText(nTextId, nPopupId, szText);
			return LTTRUE;
		}

		// RETAIL: 0x1009ddae: the subroutines screen with its popup 3235
		case MID_TRON_UNKNOWN_163:
		{
			if (m_eGameState == GS_PLAYING)
			{
				CScreenSubroutines *pSubScreen = (CScreenSubroutines *)GetScreenMgr()->GetScreenFromID(SCREEN_ID_SUBROUTINES);
				pSubScreen->SetPopupText(3235, 59, LTNULL);
				SwitchToScreen(SCREEN_ID_SUBROUTINES, true);
				g_pSubroutineMgr->PopulateSubroutineScreen();
			}
			return LTTRUE;
		}

		// RETAIL: 0x1009dde4
		case MID_TRON_UNKNOWN_151:
		{
			g_pSubroutineMgr->InfectRandomSector();
			return LTTRUE;
		}

		// RETAIL: 0x1009de00: the console variable records
		case MID_TRON_UNKNOWN_104:
		{
			uint8 nType = pMsg->Readuint8();
			if (nType == 0)
			{
				HandleConsoleVarFade(pMsg);
				return LTTRUE;
			}
			if (nType == 1)
			{
				HandleConsoleVarOscillate(pMsg);
				return LTTRUE;
			}
			if (nType == 2)
			{
				HandleConsoleVarStop(pMsg, false);
				return LTTRUE;
			}
			if (nType == 3)
			{
				HandleConsoleVarStop(pMsg, true);
				return LTTRUE;
			}
			return LTFALSE;
		}

		// RETAIL: 0x1009de96
		case MID_TRON_UNKNOWN_157:
		{
			float fValue = pMsg->Readfloat();
			g_pSubroutineMgr->SetTronUnknown11A0(fValue);
			return LTTRUE;
		}

		// RETAIL: 0x1009dec8: string 3271 with the energy gained, then (no
		// break in retail: docs/RETAIL_BUGS.md) the MID_TRON_HUD_TEXT case
		// reads on
		case MID_TRON_ENERGY_GAINED:
		{
			uint8 nEnergy = pMsg->Readuint8();
			g_pPickupMsgs->AddMessage(FormatTempString(3271, nEnergy), (const char*)LTNULL);
#ifdef TRON20SRC_MODERN
			// MODERN.md GAME-136: the message ends here
			return LTTRUE;
#endif
		}

		// RETAIL: 0x1009df00: two strings with the "msgfadeout" sound
		case MID_TRON_HUD_TEXT:
		{
			uint32 nTextId = pMsg->Readuint32();
			uint32 nValue = pMsg->Readuint32();
			g_pClientSoundMgr->PlaySoundLocal("msgfadeout", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0, 1.0f);
			if (nTextId)
			{
				g_pAnnouncementMsgs->AddMessage(LoadTempString(nTextId), kMsgScmd);
			}
			if (nValue)
			{
				g_pAnnouncementMsgs->AddMessage(FormatTempString(3822, nValue), kMsgScmd);
			}
			return LTTRUE;
		}

		// RETAIL: 0x1009df94: the jet version as three digits (string 2557)
		case MID_TRON_VERSION:
		{
			uint16 nVersion;
			g_pTronPlayerStats->GetJetVersion(nVersion);
			int nMajor = nVersion / 100;
			nVersion -= nMajor * 100;
			int nMinor = nVersion / 10;
			nVersion -= nMinor * 10;
			g_pAnnouncementMsgs->AddMessage(FormatTempString(2557, nMajor, nMinor, nVersion), kMsgScmd);
			return LTTRUE;
		}

		// RETAIL: 0x1009e03b
		case MID_SET_PERFORMANCE_RATING:
		{
			uint8 nRating = pMsg->Readuint8();
			uint8 nValue = pMsg->Readuint8();
			g_pTronPlayerStats->SetPerformanceRating((PerformanceRating)nRating, nValue);
			return LTTRUE;
		}

		// RETAIL: 0x1009e093
		case MID_SYSTEM_MEMORY_CONFIGURATION:
		{
			char szConfig[128];
			pMsg->ReadString(szConfig,128);
			g_pSubroutineMgr->SetSystemMemoryConfiguration(szConfig);
			return LTTRUE;
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
//	PURPOSE:	Handle entering new world (RETAIL: 0x1009e1d0)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnEnterWorld(LTBOOL bRestoringGame)
{
	// Call down to the base class first
	CInterfaceMgr::OnEnterWorld(bRestoringGame);

	m_bTronUnknown28B1C = true;
	m_bTronUnknown28AE8 = false;
	m_nTronUnknown28AF0 = 0;
	m_bDisplayProgress = false;

	// RETAIL: 0x1009e1f8: a custom level starts with the ratings reset
	if (g_pMissionMgr->IsCustomLevel())
	{
		m_stats.GetRatingMgr()->ResetRatings();
	}

	ClearTronRecords();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::ScreenDimsChanged()
//
//	PURPOSE:	RETAIL: 0x1009e220
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::ScreenDimsChanged()
{
	CInterfaceMgr::ScreenDimsChanged();
	UpdateIOCamera();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::UpdateIOCamera()
//
//	PURPOSE:	Fit the I/O node camera to the screen (RETAIL: 0x1009e240; the name is ours)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::UpdateIOCamera()
{
	float fXRatio = m_InterfaceResMgr.GetXRatio();
	float fYRatio = m_InterfaceResMgr.GetYRatio();

	CTronLayoutMgr* pLayoutMgr = (CTronLayoutMgr*)g_pLayoutMgr;
	m_rcIOCamera = pLayoutMgr->GetIOCameraRect();

	m_rcIOCameraScaled.left = (int)((float)m_rcIOCamera.left * fXRatio);
	m_rcIOCameraScaled.right = (int)((float)m_rcIOCamera.right * fXRatio);
	float fWidth = (float)(m_rcIOCameraScaled.right - m_rcIOCameraScaled.left);
	m_rcIOCameraScaled.top = (int)((float)m_rcIOCamera.top * fYRatio);
	m_rcIOCameraScaled.bottom = (int)((float)m_rcIOCamera.bottom * fYRatio);
	float fHeight = (float)(m_rcIOCameraScaled.bottom - m_rcIOCameraScaled.top);
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: on the centred page
	m_rcIOCameraScaled.left += m_InterfaceResMgr.GetXOffset();
	m_rcIOCameraScaled.right += m_InterfaceResMgr.GetXOffset();
	m_rcIOCameraScaled.top += m_InterfaceResMgr.GetYOffset();
	m_rcIOCameraScaled.bottom += m_InterfaceResMgr.GetYOffset();
#endif

	g_pLTClient->SetCameraRect(m_hIOCamera
, false, m_rcIOCameraScaled.left, m_rcIOCameraScaled.top, m_rcIOCameraScaled.right, m_rcIOCameraScaled.bottom);

	// PORT: retail's constants (0x101e539c, 0x101e7280, 0x101e9054) are
	// read as degrees to radians and the aspect correction
	float fFOVX = pLayoutMgr->GetIOCameraFOVX();
	float fFOVY = pLayoutMgr->GetIOCameraFOVY() * MATH_PI / 180.0f;
	g_pLTClient->SetCameraFOV(m_hIOCamera, fWidth / fHeight * fFOVX * MATH_PI / 180.0f, fFOVY);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::CreateIOCamera()
//
//	PURPOSE:	Create the I/O node camera (RETAIL: 0x1009e390; the name is ours)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::CreateIOCamera()
{
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_CAMERA;

	m_hIOCamera = g_pLTClient->CreateObject(&createStruct);

	UpdateIOCamera();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::RenderIOCamera()
//
//	PURPOSE:	Render the I/O node camera (RETAIL: 0x1009e530; the name is
//				ours), without the player's weapon
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::RenderIOCamera(float fFrameTime)
{
	if (m_eGameState != GS_PLAYING || !m_hIOCamera || !m_nTronUnknown28AF0)
		return;

	int nDrawGuns = 1;
	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("DrawGuns");
	if (hVar)
	{
		nDrawGuns = (int)g_pLTClient->GetVarValueFloat(hVar);
	}

	g_pLTClient->RunConsoleString("DrawGuns 0");
	g_pLTClient->ClearScreen(&m_rcIOCameraScaled, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	g_pLTClient->RenderCamera(m_hIOCamera, fFrameTime);

	char szTmp[32];
	sprintf(szTmp, "DrawGuns %d", nDrawGuns);
	g_pLTClient->RunConsoleString(szTmp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::UpdatePlayingState()
//
//	PURPOSE:	Update playing state (RETAIL: 0x1009e6f0)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::UpdatePlayingState()
{
	g_pSubroutineMgr->Update();

	// RETAIL: one waiting pickup at a time, while the box is down
	if (!m_DuplicateDlg.IsVisible() && !m_PendingSubroutines.empty())
	{
		TronPendingSubroutine& pending = m_PendingSubroutines.back();
		HandleSubroutinePickup(pending.szName, pending.szVersion, pending.szState);
		m_PendingSubroutines.pop_back();
	}

	CInterfaceMgr::UpdatePlayingState();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::PostPlayingState()
//
//	PURPOSE:	RETAIL: 0x1009e750
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::PostPlayingState(GameState eNewState)
{
	// RETAIL: the decision window closes (Hide(0xff), slot 12) unless the
	// new state is GS_POPUP (RETAIL: Tron's decision window, 0x1026edec)
	if (g_pTronDecision->IsVisible() && eNewState != GS_POPUP)
	{
		g_pTronDecision->Hide(0xff);
	}

	return CInterfaceMgr::PostPlayingState(eNewState);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::UpdateCursorState()
//
//	PURPOSE:	Hides or shows the cursor (RETAIL: 0x1009e790)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::UpdateCursorState()
{
	switch (m_eGameState)
	{
		// RETAIL: 0x1009e7ab: the playing case's code comes first
		case GS_EXITINGLEVEL:
		case GS_PLAYING:
		{
			m_CursorMgr.UseCursor(m_MessageBox.IsVisible(),!m_MessageBox.IsVisible());
			if (m_MessageBox.IsVisible())
			{
				m_CursorMgr.UseSprite("Interface\\cursor.dtx");
				m_CursorMgr.UseGlowSprite("Interface\\cursor_glow.dtx");
				m_CursorMgr.SetCenter(32, 32);
			}
		}
		break;

		case GS_LOADINGLEVEL:
		case GS_SPLASHSCREEN:
		case GS_POPUP:
		case GS_PAUSED:
		case GS_DEMOSCREEN:
		case GS_MOVIE:
		case GS_UNDEFINED:
		{
			m_CursorMgr.UseCursor(FALSE, FALSE);
		}
		break;

		case GS_MENU:
		case GS_SCREEN:
		default:
		{
			m_CursorMgr.UseCursor(TRUE, FALSE);
			m_CursorMgr.UseSprite("Interface\\cursor.dtx");
			m_CursorMgr.UseGlowSprite("Interface\\cursor_glow.dtx");
			m_CursorMgr.SetCenter(32, 32);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::UpdateLoadingLevelState()
//
//	PURPOSE:	RETAIL: 0x1009e600
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::UpdateLoadingLevelState()
{
	if (g_pPlayerMgr->IsPlayerInWorld())
	{
		// Make sure the loading screen isn't busy
		if (m_pLoadingScreen)
			m_pLoadingScreen->Pause();
		// Let the game client shell do stuff that's time consuming so we're not stuck on a black screen
		g_pGameClientShell->PostLevelLoadFirstUpdate();
		// Turn off the loading screen
		if (m_pLoadingScreen)
			m_pLoadingScreen->Hide();

		if (!IsMultiplayerGame())
		{
			// RETAIL: a new mission's system memory configuration, then
			// the subroutines compile
			TRONMISSION *pMission = (TRONMISSION*)g_pMissionButeMgr->GetMission(g_pMissionMgr->GetCurrentMission());
			int nSystemNameId = 0;
			if (m_bTronUnknown28B1C && g_pMissionMgr->IsNewMission())
			{
				const char *pszSystemMemory;
				if (pMission)
				{
					nSystemNameId = pMission->nSystemNameId;
					pszSystemMemory = pMission->szSystemMemory;
				}
				else
				{
					pszSystemMemory = LTNULL;
				}
				g_pSubroutineMgr->SetSystemMemoryConfiguration(pszSystemMemory, nSystemNameId);
			}
			g_pSubroutineMgr->Compile();

			if (m_pLoadingScreen && GetConsoleInt("LoadScreenWait",1))
			{
				SwitchToScreen(SCREEN_ID_POSTLOAD);
				CInterfaceMgr::UpdateLoadingLevelState();
				return;
			}

			ChangeState(GS_PLAYING);
		}
	}

	CInterfaceMgr::UpdateLoadingLevelState();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::ShowTurnArrow
//
//	PURPOSE:	RETAIL: 0x1009e850 (the name is ours): shows the left (0)
//				or the right (1) turn arrow above the screen's center
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::ShowTurnArrow(int nDirection)
{
	HideTurnArrows();

	LTIntPt pos;

	switch (nDirection)
	{
		case 0:
		{
			if (m_pTurnArrowLeft)
			{
				pos.x = g_pInterfaceResMgr->GetScreenWidth() / 2 - 256;
				pos.y = (g_pInterfaceResMgr->GetScreenHeight() - 128) / 2;
				m_pTurnArrowLeft->SetPosition(pos);
				m_pTurnArrowLeft->SetLifeTime(2);
				m_pTurnArrowLeft->Show(LTTRUE, LTTRUE);
			}
		}
		break;

		case 1:
		{
			if (m_pTurnArrowRight)
			{
				pos.x = g_pInterfaceResMgr->GetScreenWidth() / 2;
				pos.y = (g_pInterfaceResMgr->GetScreenHeight() - 128) / 2;
				m_pTurnArrowRight->SetPosition(pos);
				m_pTurnArrowRight->SetLifeTime(2);
				m_pTurnArrowRight->Show(LTTRUE, LTTRUE);
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::HideTurnArrows
//
//	PURPOSE:	RETAIL: 0x1009e910 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::HideTurnArrows()
{
	if (m_pTurnArrowLeft)
	{
		m_pTurnArrowLeft->Show(LTFALSE, LTTRUE);
	}

	if (m_pTurnArrowRight)
	{
		m_pTurnArrowRight->Show(LTFALSE, LTTRUE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::UpdateConsoleVarRecords
//
//	PURPOSE:	RETAIL: 0x1009e940 (the name is ours): moves each console
//				variable record's value on by the frame time, and removes
//				one finished fade a frame
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::UpdateConsoleVarRecords()
{
	if (!m_TronRecords.size() || g_pGameClientShell->IsGamePaused())
		return;

	std::list<TronInterfaceRecord*>::iterator iter;
	for (iter = m_TronRecords.begin(); iter != m_TronRecords.end(); iter++)
	{
		TronInterfaceRecord* pRecord = *iter;

		pRecord->m_fTronUnknown4C += g_pGameClientShell->GetFrameTime();
		if (pRecord->m_fTronUnknown4C >= pRecord->m_fTronUnknown44)
		{
			if (pRecord->m_bTronUnknown40)
			{
				do
				{
					pRecord->m_fTronUnknown4C -= pRecord->m_fTronUnknown44;
				}
				while (pRecord->m_fTronUnknown4C >= pRecord->m_fTronUnknown44);
			}
			else
			{
				pRecord->m_bTronUnknown64 = true;
			}
		}

		float fT = pRecord->m_fTronUnknown4C / pRecord->m_fTronUnknown44;
		float fValue;
		if (pRecord->m_bTronUnknown40)
		{
			fValue = (float)sin(fT * MATH_CIRCLE) * pRecord->m_fTronUnknown5C + pRecord->m_fTronUnknown58;
		}
		else if (pRecord->m_bTronUnknown64)
		{
			fValue = pRecord->m_fTronUnknown6C;
		}
		else
		{
			fValue = (pRecord->m_fTronUnknown6C - pRecord->m_fTronUnknown68) * fT + pRecord->m_fTronUnknown68;
		}

		if (g_pLTClient->GetConsoleVar(pRecord->m_szName))
		{
			char szCmd[64];
			sprintf(szCmd, "%s %4.2f", pRecord->m_szName, fValue);
			g_pLTClient->RunConsoleString(szCmd);
		}
	}

	for (iter = m_TronRecords.begin(); iter != m_TronRecords.end(); iter++)
	{
		if (!(*iter)->m_bTronUnknown40 && (*iter)->m_bTronUnknown64)
		{
			debug_delete(*iter);
			m_TronRecords.erase(iter);
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::HandleConsoleVarFade
//
//	PURPOSE:	RETAIL: 0x1009eae0 (the name is ours): MID_TRON_UNKNOWN_104
//				sub-id 0, a console variable moves from a start value (or
//				its current one) to an end value in a time
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::HandleConsoleVarFade(ILTMessage_Read *pMsg)
{
	char szName[64];
	pMsg->ReadString(szName, sizeof(szName));
	uint8 nUseStart = pMsg->Readuint8();
	float fStart = pMsg->Readfloat();
	float fEnd = pMsg->Readfloat();
	float fTime = pMsg->Readfloat();

	std::list<TronInterfaceRecord*>::iterator iter;
	for (iter = m_TronRecords.begin(); iter != m_TronRecords.end(); iter++)
	{
		if (!strcmpi(szName, (*iter)->m_szName))
		{
			debug_delete(*iter);
			m_TronRecords.erase(iter);
			break;
		}
	}

	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(szName);
	if (hVar)
	{
		TronInterfaceRecord* pRecord = debug_new(TronInterfaceRecord);
		SAFE_STRCPY(pRecord->m_szName, szName);
		pRecord->m_bTronUnknown40 = false;
		pRecord->m_bTronUnknown64 = false;
		pRecord->m_fTronUnknown44 = fTime;
		pRecord->m_fTronUnknown4C = 0;
		if (nUseStart)
			pRecord->m_fTronUnknown68 = fStart;
		else
			pRecord->m_fTronUnknown68 = g_pLTClient->GetVarValueFloat(hVar);
		pRecord->m_fTronUnknown6C = fEnd;
		m_TronRecords.push_back(pRecord);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::HandleConsoleVarOscillate
//
//	PURPOSE:	RETAIL: 0x1009ec40 (the name is ours): MID_TRON_UNKNOWN_104
//				sub-id 1, a console variable moves on a sine between two
//				values with a period
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::HandleConsoleVarOscillate(ILTMessage_Read *pMsg)
{
	char szName[64];
	pMsg->ReadString(szName, sizeof(szName));
	float fMin = pMsg->Readfloat();
	float fMax = pMsg->Readfloat();
	float fPeriod = pMsg->Readfloat();

	std::list<TronInterfaceRecord*>::iterator iter;
	for (iter = m_TronRecords.begin(); iter != m_TronRecords.end(); iter++)
	{
		if (!strcmpi(szName, (*iter)->m_szName))
		{
			debug_delete(*iter);
			m_TronRecords.erase(iter);
			break;
		}
	}

	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(szName);
	if (hVar)
	{
		TronInterfaceRecord* pRecord = debug_new(TronInterfaceRecord);
		SAFE_STRCPY(pRecord->m_szName, szName);
		pRecord->m_bTronUnknown40 = true;
		pRecord->m_fTronUnknown44 = fPeriod;
		pRecord->m_fTronUnknown4C = 0;
		pRecord->m_fTronUnknown50 = fMin;
		pRecord->m_fTronUnknown54 = fMax;
		// RETAIL: the center is fMin + fMax / 2, not ( fMin + fMax ) / 2
		// (see docs/RETAIL_BUGS.md)
#ifdef TRON20SRC_MODERN
		// MODERN.md GAME-129: the center is between the two
		pRecord->m_fTronUnknown58 = (fMin + fMax) * 0.5f;
#else
		pRecord->m_fTronUnknown58 = fMin + fMax * 0.5f;
#endif
		pRecord->m_fTronUnknown5C = (fMax - fMin) * 0.5f;
		pRecord->m_fTronUnknown60 = g_pLTClient->GetVarValueFloat(hVar);
		m_TronRecords.push_back(pRecord);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::HandleConsoleVarStop
//
//	PURPOSE:	RETAIL: 0x1009edb0 (the name is ours): MID_TRON_UNKNOWN_104
//				sub-ids 2 and 3, a console variable's record stops (3 sets
//				the value it had before an oscillation again)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::HandleConsoleVarStop(ILTMessage_Read *pMsg, bool bRestore)
{
	if (!m_TronRecords.size())
		return LTFALSE;

	char szName[64];
	pMsg->ReadString(szName, sizeof(szName));
	pMsg->Readfloat();
	pMsg->Readfloat();
	pMsg->Readfloat();

	std::list<TronInterfaceRecord*>::iterator iter;
	for (iter = m_TronRecords.begin(); iter != m_TronRecords.end(); iter++)
	{
		TronInterfaceRecord* pRecord = *iter;
		if (!strcmpi(pRecord->m_szName, szName))
		{
			if (bRestore)
			{
				// RETAIL: "%4.2d" with a double (see docs/RETAIL_BUGS.md)
				char szCmd[64];
#ifdef TRON20SRC_MODERN
				// MODERN.md GAME-130: the value as a float
				sprintf(szCmd, "%s %f", pRecord->m_szName, pRecord->m_fTronUnknown60);
#else
				sprintf(szCmd, "%s %4.2d", pRecord->m_szName, pRecord->m_fTronUnknown60);
#endif
				g_pLTClient->RunConsoleString(szCmd);
			}

			debug_delete(*iter);
			m_TronRecords.erase(iter);
			return LTTRUE;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::Save
//
//	PURPOSE:	Save the interface info (RETAIL: 0x1009eee0)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	pMsg->Writebool(eSaveDataState == eSaveDataStateSwitchLevels ||
					eSaveDataState == eSaveDataStateTransitionLevels ||
					(eSaveDataState == eSaveDataAutoSave && g_pMissionMgr->IsNewMission()));

	CInterfaceMgr::Save(pMsg, eSaveDataState);

	if (eSaveDataState >= eSaveDataSaveGameSlot && eSaveDataState <= eSaveDataAutoSave)
	{
		pMsg->Writeuint32(m_TronRecords.size());

		std::list<TronInterfaceRecord*>::iterator iter;
		for (iter = m_TronRecords.begin(); iter != m_TronRecords.end(); iter++)
		{
			(*iter)->Save(pMsg);
		}
	}
	else
	{
		pMsg->Writeuint32(0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::Load
//
//	PURPOSE:	Load the interface info (RETAIL: 0x1009efb0)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::Load(ILTMessage_Read *pMsg)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	m_bTronUnknown28B1C = pMsg->Readbool();

	CInterfaceMgr::Load(pMsg);

	uint32 nRecords = pMsg->Readuint32();
	ClearTronRecords();

	for (uint32 i = 0; i < nRecords; i++)
	{
		TronInterfaceRecord* pRecord = debug_new(TronInterfaceRecord);
		if (pRecord)
		{
			pRecord->Load(pMsg);
			m_TronRecords.insert(m_TronRecords.end(), pRecord);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::ClearTronRecords
//
//	PURPOSE:	RETAIL: 0x1009f050 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::ClearTronRecords()
{
	std::list<TronInterfaceRecord*>::iterator iter;
	for (iter = m_TronRecords.begin(); iter != m_TronRecords.end(); iter++)
	{
		debug_delete(*iter);
	}
	m_TronRecords.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::QueueSubroutinePickup
//
//	PURPOSE:	RETAIL: 0x1009f0c0 (the name is ours): keeps a picked up
//				subroutine for UpdatePlayingState. An infected one is
//				clean while the gold virus scan is installed.
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::QueueSubroutinePickup(const char* szName, const char* szVersion, const char* szState)
{
	if (!szName || !szName[0] || !szVersion || !szVersion[0] || !szState || !szState[0])
		return;

	if (!strcmpi(szState, "infected"))
	{
		PlayerSubroutine* pScan = g_pSubroutineMgr->GetPlayerSubroutine("virus_scan", LTNULL);
		if (pScan && pScan->eVersion == VERSION_GOLD && pScan->nSlot >= 0 && pScan->nSlot < 100)
		{
			szState = "okay";
		}
	}

	TronPendingSubroutine pending;
	strncpy(pending.szName, szName, sizeof(pending.szName) - 1);
	pending.szName[sizeof(pending.szName) - 1] = 0;
	strncpy(pending.szVersion, szVersion, sizeof(pending.szVersion) - 1);
	pending.szVersion[sizeof(pending.szVersion) - 1] = 0;
	strncpy(pending.szState, szState, sizeof(pending.szState) - 1);
	pending.szState[sizeof(pending.szState) - 1] = 0;

	m_PendingSubroutines.push_back(pending);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::HandleSubroutinePickup
//
//	PURPOSE:	RETAIL: 0x1009f200 (the name is ours): a subroutine the
//				player has (and that is no better, or infected) shows the
//				duplicate subroutine box and pauses the game; any other
//				is given.
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::HandleSubroutinePickup(char* szName, char* szVersion, char* szState)
{
	if (!szName || !szName[0] || !szVersion || !szVersion[0] || !szState || !szState[0])
		return;

	SubState eExclude = SUBSTATE_FOREIGN;
	m_pDuplicateOld = m_SubroutineMgr.GetPlayerSubroutine(szName, &eExclude);

	SubVersion eVersion = VERSION_ALPHA;
	if (m_pDuplicateOld)
	{
		if (!strcmpi(szVersion, "beta"))
			eVersion = VERSION_BETA;
		else if (!strcmpi(szVersion, "gold"))
			eVersion = VERSION_GOLD;

		if (strcmpi(szState, "noncompatible") && strcmpi(szState, "foreign") &&
			(eVersion <= m_pDuplicateOld->eVersion || !strcmpi(szState, "infected")))
		{
			if (m_pDuplicateNew)
			{
				delete m_pDuplicateNew;
			}

			m_pDuplicateNew = debug_new(PlayerSubroutine);
			m_pDuplicateNew->pTronSubroutine = m_SubroutineMgr.GetSubroutine(szName);
			m_pDuplicateNew->eVersion = eVersion;
			m_pDuplicateNew->eState = SUBSTATE_OKAY;
			m_pDuplicateNew->nSlot = -1;
			if (!strcmpi(szState, "infected"))
			{
				m_pDuplicateNew->eState = SUBSTATE_CORRUPT;
				m_pDuplicateNew->nTronUnknown1C = (LAST_VERSION - m_pDuplicateNew->eVersion) * 3;
			}

			strncpy(m_szDuplicateName, szName, sizeof(m_szDuplicateName) - 1);
			m_szDuplicateName[sizeof(m_szDuplicateName) - 1] = 0;
			strncpy(m_szDuplicateVersion, szVersion, sizeof(m_szDuplicateVersion) - 1);
			m_szDuplicateVersion[sizeof(m_szDuplicateVersion) - 1] = 0;
			strncpy(m_szDuplicateState, szState, sizeof(m_szDuplicateState) - 1);
			m_szDuplicateState[sizeof(m_szDuplicateState) - 1] = 0;

			m_DuplicateDlg.Show(m_pDuplicateOld, m_pDuplicateNew);

			m_eDuplicateRenderLevel = GetHUDMgr()->GetRenderLevel();
			GetHUDMgr()->SetRenderLevel(kHUDRenderNone);

			m_CursorMgr.UseCursor(LTTRUE);
			m_CursorMgr.UseSprite("Interface\\cursor.dtx");
			m_CursorMgr.UseGlowSprite("Interface\\cursor_glow.dtx");
			m_CursorMgr.SetCenter(32, 32);

			g_pGameClientShell->PauseGame(true);
			return;
		}
	}

	m_SubroutineMgr.GivePlayerSubroutine(szName, szVersion, szState, true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnDuplicateDlgClick
//
//	PURPOSE:	RETAIL: 0x1009f510 (the name is ours): a KEEP button
//				closes the duplicate subroutine box; keeping the new copy
//				gives it (and so replaces the old one).
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnDuplicateDlgClick(int x, int y)
{
	PlayerSubroutine* pKept = m_DuplicateDlg.GetKeptSubroutine(x, y);
	if (!pKept)
		return;

	m_DuplicateDlg.Hide();

	m_CursorMgr.UseCursor(LTFALSE, LTTRUE);
	// RETAIL: 0x1009f54b: the level is read before the HUD manager
	eHUDRenderLevel eLevel = m_eDuplicateRenderLevel;
	GetHUDMgr()->SetRenderLevel(eLevel);
	g_pHUDMgr->QueueUpdate(kHUDAll);

	if (pKept == m_pDuplicateNew)
	{
		m_SubroutineMgr.GivePlayerSubroutine(m_szDuplicateName, m_szDuplicateVersion, m_szDuplicateState, true);
	}

	delete m_pDuplicateNew;
	m_pDuplicateNew = LTNULL;
	m_pDuplicateOld = LTNULL;

	g_pGameClientShell->PauseGame(false);
	g_pLTClient->ClearInput();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnLButtonUp
//
//	PURPOSE:	RETAIL: 0x1009f470
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnLButtonUp(int x, int y)
{
	if (m_DuplicateDlg.IsVisible())
	{
		OnDuplicateDlgClick(x, y);
	}

	CInterfaceMgr::OnLButtonUp(x, y);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnRButtonUp
//
//	PURPOSE:	RETAIL: 0x1009f4a0
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnRButtonUp(int x, int y)
{
	if (m_DuplicateDlg.IsVisible())
	{
		OnDuplicateDlgClick(x, y);
	}

	CInterfaceMgr::OnRButtonUp(x, y);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::OnMouseMove
//
//	PURPOSE:	RETAIL: 0x1009f4d0
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnMouseMove(int x, int y)
{
	CInterfaceMgr::OnMouseMove(x, y);

	if (m_DuplicateDlg.IsVisible())
	{
		m_DuplicateDlg.OnMouseMove(x, y);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::Draw
//
//	PURPOSE:	RETAIL: 0x1009f5e0
//
// ----------------------------------------------------------------------- //

LTBOOL CTronInterfaceMgr::Draw()
{
	LTBOOL bRet = CInterfaceMgr::Draw();

	if (m_DuplicateDlg.IsVisible())
	{
		m_DuplicateDlg.Draw();
		m_CursorMgr.Update();
		m_ScreenSpriteMgr.Update();
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::TronUnknownVirtual50
//
//	PURPOSE:	RETAIL: 0x1009cc70 (slot 0x50)
//
// ----------------------------------------------------------------------- //

bool CTronInterfaceMgr::TronUnknownVirtual50()
{
	// RETAIL: a computed bool (mov eax,1 / xor eax,eax)
	return !g_pDecision->IsVisible() && !m_DuplicateDlg.IsVisible();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronInterfaceRecord::Save
//
//	PURPOSE:	RETAIL: 0x1009f620
//
// ----------------------------------------------------------------------- //

void TronInterfaceRecord::Save(ILTMessage_Write *pMsg)
{
	pMsg->WriteString(m_szName);
	pMsg->Writebool(m_bTronUnknown40);
	pMsg->Writebool(m_bTronUnknown64);
	pMsg->Writefloat(m_fTronUnknown44);
	pMsg->Writefloat(m_fTronUnknown4C);
	pMsg->Writefloat(m_fTronUnknown50);
	pMsg->Writefloat(m_fTronUnknown54);
	pMsg->Writefloat(m_fTronUnknown58);
	pMsg->Writefloat(m_fTronUnknown5C);
	pMsg->Writefloat(m_fTronUnknown60);
	pMsg->Writefloat(m_fTronUnknown68);
	pMsg->Writefloat(m_fTronUnknown6C);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronInterfaceRecord::Load
//
//	PURPOSE:	RETAIL: 0x1009f710
//
// ----------------------------------------------------------------------- //

void TronInterfaceRecord::Load(ILTMessage_Read *pMsg)
{
	pMsg->ReadString(m_szName, sizeof(m_szName) - 1);
	m_bTronUnknown40 = pMsg->Readbool();
	m_bTronUnknown64 = pMsg->Readbool();
	m_fTronUnknown44 = pMsg->Readfloat();
	m_fTronUnknown4C = pMsg->Readfloat();
	m_fTronUnknown50 = pMsg->Readfloat();
	m_fTronUnknown54 = pMsg->Readfloat();
	m_fTronUnknown58 = pMsg->Readfloat();
	m_fTronUnknown5C = pMsg->Readfloat();
	m_fTronUnknown60 = pMsg->Readfloat();
	m_fTronUnknown68 = pMsg->Readfloat();
	m_fTronUnknown6C = pMsg->Readfloat();
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
