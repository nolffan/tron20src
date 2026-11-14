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
#include "LightCycleMatchMgr.h"
// RETAIL: retail's TU runs ScreenSubroutines.h's constant initializers
#include "ScreenSubroutines.h"

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
	typedef char TronCheckWeaponChooser[offsetof(CTronInterfaceMgr, m_WeaponChooser) == 0x26790 ? 1 : -1];
	typedef char TronCheckSize[sizeof(CTronInterfaceMgr) == 0x29490 ? 1 : -1];

	// PORT: retail points CInterfaceMgr's loading screen (+0xf8e4) at its
	// CTronLoadingScreen (+0x28b24), not yet written

	m_pTurnArrowLeft = LTNULL;
	m_pTurnArrowRight = LTNULL;
	m_hIOCamera = LTNULL;
	m_nTronUnknown28B18 = 0;
	m_bTronUnknown28B1C = false;
	m_pTronUnknown293BC = LTNULL;
	m_nTronUnknown293B8 = 0;
	m_szTronUnknown293C0[0] = 0;
	m_szTronUnknown29400[0] = 0;
	m_szTronUnknown29440[0] = 0;
	m_nTronUnknown29480 = 2;

	// PORT: retail leaves these to the members' constructors (not yet
	// written); ours clears them
	m_bTronUnknown28AE8 = false;
	m_bTronUnknown28B14 = false;
	m_bTronUnknown28C51 = false;
	m_bDisplayProgress = false;
	m_nTronUnknown28AF0 = 0;
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
	m_pTurnArrowLeft->m_bShow = LTTRUE;
	m_pTurnArrowRight = g_pScreenSpriteMgr->CreateScreenSprite("Interface\\Spr\\TurnArrowRight.spr", false, SPRITELAYER_DEFAULT);
	m_pTurnArrowRight->m_bShow = LTTRUE;

	// PORT: retail initializes its +0x28c48 object (0x1001cd20) and finds
	// (or creates) the "IOScene" console variable here; not yet written

	m_pTronUnknown293BC = LTNULL;
	m_nTronUnknown293B8 = 0;

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

	if (m_pTronUnknown293BC)
	{
		delete m_pTronUnknown293BC;
		m_pTronUnknown293BC = LTNULL;
	}

	// PORT: retail terms its +0x28c48 object here (0x1001d640)
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
	g_pSubtitles->Clear();

	m_bTronUnknown28AE8 = false;
	m_nTronUnknown28AF0 = 0;
	m_bDisplayProgress = false;
	m_nTronUnknown28B18 = 0;

	// Now handle the base class
	CInterfaceMgr::OnExitWorld();

	ClearTronRecords();

	// PORT: retail tells its +0x28c48 object here (0x1001e780)
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
	if (m_bTronUnknown28C51)
	{
		return LTTRUE;
	}

	// PORT: in the playing state retail lets escape close its
	// CHUDTronDecision (retail global 0x1026edec, not yet written) here

	return CInterfaceMgr::OnKeyDown(key, rep);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronInterfaceMgr::ShowChatInput()
//
//	PURPOSE:	Open the chat input (RETAIL: 0x1009d2e0; the name is ours)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::ShowChatInput(uint8 nChatType)
{
	if (m_eGameState == GS_PLAYING && !g_pPlayerMgr->IsSpectatorMode())
	{
		if (m_AmmoChooser.IsOpen())
		{
			m_AmmoChooser.Close();
		}
		// RETAIL: through the chooser's vtable (CTronWeaponChooser's Close
		// takes an argument, 1 here)
		CWeaponChooser* pWeaponChooser = &m_WeaponChooser;
		if (pWeaponChooser->IsOpen())
		{
			pWeaponChooser->Close();
		}

		// PORT: retail passes nChatType to the chat input (0x101660b0)
		g_pChatInput->Show(true, nChatType == 2);
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

	// PORT: retail lets its CHUDTronDecision (retail global 0x1026edec, not
	// yet written) take COMMAND_ID_CHOOSE_1..6 here

	// Take appropriate action

	switch (command)
	{
		case COMMAND_ID_MESSAGE :
		{
			ShowChatInput(1);
            return LTTRUE;
		}
		break;

		// RETAIL: 93 opens the team chat
		case COMMAND_ID_LASTWEAPON :
		{
			ShowChatInput(2);
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
			// PORT: retail asks its CHUDTronDecision first
			NextWeapon(command);
            return LTTRUE;
		}
		break;

		// PORT: retail's 70 and 192 toggle two of the client shell's flags
		// (+0x1d8, +0x1d7) through its virtuals, not yet identified

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
		// PORT: retail does not open the screen while its CHUDTronDecision
		// shows (unless its +0x4d is set), nor in some of the player's
		// states (the player manager's +0x330, +0x18 == 3) and the client
		// shell's (+0x3c == 2 or 3)
		if (m_eGameState == GS_PLAYING && !IsMultiplayerGame())
		{
			if (m_AmmoChooser.IsOpen())
			{
				m_AmmoChooser.Close();
			}
			CWeaponChooser* pWeaponChooser = &m_WeaponChooser;
			if (pWeaponChooser->IsOpen())
			{
				pWeaponChooser->Close();
			}

			SwitchToScreen(SCREEN_ID_SUBROUTINES);
			g_pSubroutineMgr->PopulateSubroutineScreen();

			// PORT: retail then asks its global 0x1026ef70 (vtable +0x164)
			// for "subroutines" and "ratings"

			return LTTRUE;
		}

		// PORT: retail switches to its screen 0x28 in game types 5..7 while
		// playing
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
			m_stats.UpdateMaxEnergy((uint32)fAmount);
		}
		break;

		case IC_ENERGY_ID :
		{
			m_stats.UpdateEnergy((uint32)fAmount);
		}
		break;

		case IC_RESET_INVENTORY_ID :
		{
			// RETAIL: 0x100948d0 (no arguments), then the stats' virtual at
			// +0x88 with nType1, then the base class
			if (nType2)
			{
				g_pSubroutineMgr->ClearPlayerSubroutines();
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

void CTronInterfaceMgr::TronUnknownVirtual7C(void* pTronUnknown)
{
	// PORT: retail reads the record's +0x10 (a client id: the local
	// client's?) and +0x18 (a state, 0..3) and its team (m_ClientInfo,
	// 0x100fba30); for some game types it sets m_nTronUnknown28B18 to the
	// strings 0xd87..0xd8b and 0xd9b, then updates the HUD (0x4000000).
	// Not yet written.
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
	// PORT: retail first calls 0x1009e940 (not yet written)

	m_bTronUnknown28B14 = true;

	// PORT: retail clears m_bTronUnknown28B14 while the player's lure
	// (PlayerLureFX::GetPlayerLureFX of the player manager's +0x330,
	// +0x2080) is not active

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

	// PORT: retail calls the stats' rating manager (0x10051640) when
	// g_pMissionMgr's +0x4 is set; unidentified

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

	g_pLTClient->SetCameraRect(m_hIOCamera, false, m_rcIOCameraScaled.left, m_rcIOCameraScaled.top, m_rcIOCameraScaled.right, m_rcIOCameraScaled.bottom);

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
//	ROUTINE:	CTronInterfaceMgr::UpdatePlayingState()
//
//	PURPOSE:	Update playing state (RETAIL: 0x1009e6f0)
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::UpdatePlayingState()
{
	g_pSubroutineMgr->Update();

	// PORT: unless the +0x28c48 object is active, retail hands the last of
	// its +0x29484 records to 0x1009f200 (its three 0x80-byte strings) and
	// pops it

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
	// PORT: retail closes its CHUDTronDecision (retail global 0x1026edec,
	// Choose(0xff)) unless the new state is GS_POPUP

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
	// PORT: retail's own version (0x1009e600) is not yet written
	CInterfaceMgr::UpdateLoadingLevelState();
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
//	ROUTINE:	CTronInterfaceMgr::OnLButtonUp
//
//	PURPOSE:	RETAIL: 0x1009f470
//
// ----------------------------------------------------------------------- //

void CTronInterfaceMgr::OnLButtonUp(int x, int y)
{
	// PORT: while the +0x28c48 object is active retail first calls
	// 0x1009f510 with (x, y), not yet written
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
	// PORT: see OnLButtonUp
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

	// PORT: while the +0x28c48 object is active retail passes (x, y) to it
	// (0x1001e9b0)
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

	if (m_bTronUnknown28C51)
	{
		// PORT: retail draws its +0x28c48 object first (0x1001e790)
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
	if (g_pDecision->IsVisible() || m_bTronUnknown28C51)
	{
		return false;
	}
	return true;
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
