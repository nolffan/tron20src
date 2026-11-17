// ----------------------------------------------------------------------- //
//
// MODULE  : TronGameClientShell.cpp
//
// PURPOSE : Game Client Shell - Implementation
//
// CREATED : 11/5/01
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronGameClientShell.h"

#include "msgids.h"

#include "clientmultiplayermgr.h"
#include "iserverdir.h"
#include "TexturedFrame.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "PlayerCamera.h"
#include "HUDMgr.h"
#include "HUDChatInput.h"
#include "CommandIDs.h"
#include "SubroutineMgr.h"
#include "TRONPlayerStats.h"

// Sample rate
int g_nSampleRate = 44100;

// RETAIL: 0x1026eda0, set by the constructor and cleared by the destructor
// (0x10099b12, 0x10099e34); the base shell's g_pGameClientShell is another
// global. The name is ours.
CTronGameClientShell* g_pTronGameClientShell = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LCFn
//
//	PURPOSE:	RETAIL: 0x10099810: the "LC" console program (the
//				function's name is ours): "LC color <name>" sets the
//				player's light cycle colour
//
// ----------------------------------------------------------------------- //

static void LCFn(int argc, char **argv)
{
	if (argc < 2)
	{
		g_pLTClient->CPrint("LC <color> [color]");
		return;
	}

	if (_stricmp(argv[0], "color") == 0)
	{
		g_pPlayerMgr->TronUnknownVirtual90(argv[1], true, true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FrameFn
//
//	PURPOSE:	RETAIL: 0x10099860: the "Frame" console program (the
//				function's name is ours): moves an edge of a piece of the
//				textured frame last initialised
//
// ----------------------------------------------------------------------- //

static void FrameFn(int argc, char **argv)
{
	if (argc != 3)
	{
		g_pLTClient->CPrint("Frame <piece> <edge> <value>");
		return;
	}

	int nPiece = atoi(argv[0]);
	int nValue = atoi(argv[2]);
	int nEdge;
	if (_strcmpi(argv[1], "left") == 0)
		nEdge = 0;
	else if (_strcmpi(argv[1], "top") == 0)
		nEdge = 1;
	else if (_strcmpi(argv[1], "right") == 0)
		nEdge = 2;
	else if (_strcmpi(argv[1], "bottom") == 0)
		nEdge = 3;
	else
		return;

	if (g_pTexturedFrame)
	{
		g_pTexturedFrame->AdjustPiece(nPiece, nEdge, nValue);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FrameUVFn
//
//	PURPOSE:	RETAIL: 0x10099920: the "FrameUV" console program (the
//				function's name is ours): as "Frame", for the piece's
//				texture coordinates
//
// ----------------------------------------------------------------------- //

static void FrameUVFn(int argc, char **argv)
{
	if (argc != 3)
	{
		g_pLTClient->CPrint("FrameUV <piece> <edge> <value>");
		return;
	}

	int nPiece = atoi(argv[0]);
	int nValue = atoi(argv[2]);
	int nEdge;
	if (_strcmpi(argv[1], "left") == 0)
		nEdge = 0;
	else if (_strcmpi(argv[1], "top") == 0)
		nEdge = 1;
	else if (_strcmpi(argv[1], "right") == 0)
		nEdge = 2;
	else if (_strcmpi(argv[1], "bottom") == 0)
		nEdge = 3;
	else
		return;

	if (g_pTexturedFrame)
	{
		g_pTexturedFrame->AdjustPieceUV(nPiece, nEdge, nValue);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::CTronGameClientShell()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CTronGameClientShell::CTronGameClientShell() : CGameClientShell()
{
	// RETAIL: 0x10099b12: the mission manager is a member (Tron's base
	// shell does not make one)
	g_pTronGameClientShell = this;
	m_MissionMgr.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::~CTronGameClientShell()
//
//	PURPOSE:	Destruction
//
// ----------------------------------------------------------------------- //

CTronGameClientShell::~CTronGameClientShell()
{
	g_pTronGameClientShell = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::OnEnterWorld
//
//	PURPOSE:	RETAIL: 0x10099fa0: no light cycles are left from the last
//				world, the player's cycle colour is sent again
//
// ----------------------------------------------------------------------- //

void CTronGameClientShell::OnEnterWorld()
{
	m_LightCycleMgr.RemoveAllCyclists();
	g_pPlayerMgr->TronUnknownVirtual90(LTNULL, true, true);

	if (IsMultiplayerGame() && GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		g_pGameClientShell->m_bTronUnknown833A = true;
	}

	CGameClientShell::OnEnterWorld();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::OnExitWorld
//
//	PURPOSE:	RETAIL: 0x10099ff0: resets the match managers
//
// ----------------------------------------------------------------------- //

void CTronGameClientShell::OnExitWorld()
{
	m_DiscMatchMgr.Reset();
	m_LightCycleMatchMgr.Reset();
	CGameClientShell::OnExitWorld();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::OnKeyDown
//
//	PURPOSE:	RETAIL: 0x1009ac40
//
// ----------------------------------------------------------------------- //

void CTronGameClientShell::OnKeyDown(int key, int rep)
{
	if (IsMultiplayerGame() && GetGameType() == eGameTypeMultiplayerLightcycles &&
		g_pLightCycleMatchMgr->OnKeyDown(key, rep))
	{
		return;
	}

	CGameClientShell::OnKeyDown(key, rep);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::OnEngineInitialized
//
//	PURPOSE:	Wrapper for OnEngineInitialized so we can do something extra
//
// ----------------------------------------------------------------------- //

uint32 CTronGameClientShell::OnEngineInitialized(RMode *pMode, LTGUID *pAppGuid)
{
	uint32 nResult = CGameClientShell::OnEngineInitialized(pMode, pAppGuid);

	// RETAIL: 0x1009a020 then inits the light cycle manager and the match
	// managers, and registers the "LC", "Frame" and "FrameUV" console
	// programs
	if (nResult == LT_OK)
	{
		m_LightCycleMgr.Init();
		m_DiscMatchMgr.Init();
		m_LightCycleMatchMgr.Init();

		g_pLTClient->RegisterConsoleProgram("LC", LCFn);
		g_pLTClient->RegisterConsoleProgram("Frame", FrameFn);
		g_pLTClient->RegisterConsoleProgram("FrameUV", FrameUVFn);
	}

	return nResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::ProcessPacket
//
//	PURPOSE:	Handling for connectionless networking packets
//
// ----------------------------------------------------------------------- //

LTRESULT CTronGameClientShell::ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort)
{
	// Gotta have some data...
	if (pMsg->Size() < 16)
		return LT_OK;

	// Skip the engine-side portion of the message header
	CLTMsgRef_Read cSubMsg(pMsg->SubMsg(8));

	// Remember what address this came from
	g_pClientMultiplayerMgr->SetCurMessageSource(senderAddr, senderPort);

	// Hand it off as if it were a message
	// Note : This has to go through the parent's OnMessage so we handle the message source properly
	CGameClientShell::OnMessage(cSubMsg);

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::OnMessage
//
//	PURPOSE:	Message from the server
//
// ----------------------------------------------------------------------- //
void CTronGameClientShell::OnMessage(ILTMessage_Read *pMsg)
{
	// Store the server's address as where this message came from
	uint8 aAddrBuffer[4];
	uint16 nPort;
	g_pLTClient->GetServerIPAddress(aAddrBuffer, &nPort);
	g_pClientMultiplayerMgr->SetCurMessageSource(aAddrBuffer, nPort);

	CGameClientShell::OnMessage(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::PauseGame()
//
//	PURPOSE:	Pauses/Unpauses the server
//
// ----------------------------------------------------------------------- //

void CTronGameClientShell::PauseGame(bool bPause, bool bPauseSound)
{
	// start with the base "pause" function
	CGameClientShell::PauseGame(bPause, bPauseSound);

	// RETAIL: Tron's (0x1009a1a0) checks for the managers first
	if (g_pSubroutineMgr)
		g_pSubroutineMgr->Pause(bPause);
	if (g_pTronPlayerMgr)
		g_pTronPlayerMgr->Pause(bPause);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::RenderCamera()
//
//	PURPOSE:	RETAIL: 0x1009a1e0: sets up the client and renders the
//				camera: on a light cycle, the turn around command looks
//				back at the cycle from the console's radius and angles;
//				the I/O node camera's view goes over the world's
//
// ----------------------------------------------------------------------- //

void CTronGameClientShell::RenderCamera(bool bDrawInterface)
{
	if (!m_bMainWindowFocus)
	{
		if (IsMultiplayerGame())
		{
			UpdateInterfaceFX();
		}
		return;
	}

	if (!GetPlayerMgr()->PreRender()) return;

	m_sfxMgr.UpdateSpecialFX();
	m_DamageFXMgr.Update();

	HLOCALOBJ hCamera = GetPlayerMgr()->GetCamera();

	if (g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE &&
		g_pLTClient->IsCommandOn(COMMAND_ID_TURNAROUND) &&
		!g_pPlayerMgr->TronUnknownVirtual3C(false) &&
		!g_pChatInput->IsVisible())
	{
		float fTheta = 266.0f;
		float fPhi = 45.0f;
		float fRadius = 800.0f;

		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCReverseRadius");
		if (hVar)
		{
			fRadius = g_pLTClient->GetVarValueFloat(hVar);
		}

		hVar = g_pLTClient->GetConsoleVar("LCReverseTheta");
		if (hVar)
		{
			fTheta = g_pLTClient->GetVarValueFloat(hVar);
		}

		hVar = g_pLTClient->GetConsoleVar("LCReversePhi");
		if (hVar)
		{
			fPhi = g_pLTClient->GetVarValueFloat(hVar);
		}

		fPhi = fPhi * MATH_PI / 180.0f;

		LTVector vPos;
		g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPos);

		fTheta = fTheta * MATH_PI / 180.0f + g_pPlayerMgr->GetPlayerCamera()->TronUnknownGetOrbitYaw();

		vPos.x += (float)sin(fPhi) * (float)cos(fTheta) * fRadius;
		vPos.y += (float)cos(fPhi) * fRadius;
		vPos.z += (float)sin(fPhi) * (float)sin(fTheta) * fRadius;

		g_pLTClient->SetObjectPos(hCamera, &vPos, true);

		LTRotation rRot(MATH_HALFPI - fPhi, -fTheta - MATH_HALFPI, 0.0f);
		g_pLTClient->SetObjectRotation(hCamera, &rRot);
	}

	//handle updating the paused status
	m_ClientFXMgr.Pause(m_bServerPaused);

	HCONSOLEVAR hDetail = g_pLTClient->GetConsoleVar("ClientFXDetailLevel");
	if (hDetail)
	{
		m_ClientFXMgr.SetDetailLevel((int)g_pLTClient->GetVarValueFloat(hDetail));
	}

	m_ClientFXMgr.UpdateAllActiveFX( IsMultiplayerGame() ? true : m_bMainWindowFocus );

	float fFrameTime = m_bServerPaused ? 0.0 : m_fFrameTime;

	g_pLTClient->Start3D();

	if (!GetPlayerMgr()->IsDeathFXDone())
	{
		g_pLTClient->RenderCamera(hCamera, fFrameTime);
	}

	// Render the the dynamic FX.
	m_sfxMgr.RenderFX(hCamera);

	if (m_InterfaceMgr.GetTronUnknown28AE8())
	{
		g_pDrawPrim->SaveViewport();
		m_InterfaceMgr.RenderIOCamera(fFrameTime);
		g_pDrawPrim->SetCamera(hCamera);
		g_pDrawPrim->RestoreViewport();
	}

	// Render the effects
	m_ClientFXMgr.RenderAllActiveFX(m_bMainWindowFocus);

	g_pPlayerMgr->TronUnknownVirtual5C();

	g_pLTClient->StartOptimized2D();

	GetInterfaceMgr()->Draw();

	// Display any necessary debugging info...
	RenderDebugStrings();

	g_pLTClient->EndOptimized2D();
	g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::UpdateInterfaceFX()
//
//	PURPOSE:	RETAIL: 0x1009a670: keep the fx running while the interface
//				draws instead of the world
//
// ----------------------------------------------------------------------- //

void CTronGameClientShell::UpdateInterfaceFX()
{
	GetPlayerMgr()->PreRender();

	m_sfxMgr.UpdateSpecialFX();

	m_ClientFXMgr.Pause(m_bServerPaused);

	HCONSOLEVAR hDetail = g_pLTClient->GetConsoleVar("ClientFXDetailLevel");
	if (hDetail)
	{
		m_ClientFXMgr.SetDetailLevel((int)g_pLTClient->GetVarValueFloat(hDetail));
	}

	m_ClientFXMgr.UpdateAllActiveFX( IsMultiplayerGame() ? true : m_bMainWindowFocus );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::TronUnknownVirtual94
//
//	PURPOSE:	RETAIL: 0x1009a700 (slot 0x94): Tron's own cheats
//				(CCheatMgr gives them here, retail 0x100f2541)
//
// ----------------------------------------------------------------------- //

void CTronGameClientShell::TronUnknownVirtual94(int nCheat)
{
	switch (nCheat)
	{
		case CHEAT_TRON_ENERGY:
		{
			g_pChatMsgs->AddMessage(0xa00, kMsgCheatConfirm);

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_CHEAT);
			cMsg.Writeuint8((uint8)nCheat);
			cMsg.Writeuint32(1);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}
		break;

		case CHEAT_TRON_PSET:
		case CHEAT_TRON_CLEARPSET:
		break;

		case CHEAT_TRON_PROCEDURALS:
		{
			g_pSubroutineMgr->GivePlayerProcedural("port");
			g_pSubroutineMgr->GivePlayerProcedural("defrag");
			g_pSubroutineMgr->GivePlayerProcedural("disinfect");
			g_pLTClient->CPrint("All procedurals acquired!");
		}
		break;

		case CHEAT_TRON_SUBALPHA:
		{
			g_pLTClient->CPrint("All ALPHA Subroutines acquired!");
			g_pSubroutineMgr->GivePlayerProcedural("port");
			g_pSubroutineMgr->GivePlayerProcedural("defrag");
			g_pSubroutineMgr->GivePlayerProcedural("disinfect");

			uint16 nNumSubroutines = g_pSubroutineMgr->GetNumSubroutines();
			for (int i = 0; i < nNumSubroutines; i++)
			{
				g_pSubroutineMgr->GivePlayerSubroutine(g_pSubroutineMgr->GetSubroutineByIndex(i)->szName, "alpha", "okay", true);
			}
		}
		break;

		case CHEAT_TRON_SUBBETA:
		{
			g_pLTClient->CPrint("All BETA Subroutines acquired!");
			g_pSubroutineMgr->GivePlayerProcedural("port");
			g_pSubroutineMgr->GivePlayerProcedural("defrag");
			g_pSubroutineMgr->GivePlayerProcedural("disinfect");

			uint16 nNumSubroutines = g_pSubroutineMgr->GetNumSubroutines();
			for (int i = 0; i < nNumSubroutines; i++)
			{
				g_pSubroutineMgr->GivePlayerSubroutine(g_pSubroutineMgr->GetSubroutineByIndex(i)->szName, "beta", "okay", true);
			}
		}
		break;

		case CHEAT_TRON_SUBGOLD:
		{
			g_pLTClient->CPrint("All GOLD Subroutines acquired!");
			g_pSubroutineMgr->GivePlayerProcedural("port");
			g_pSubroutineMgr->GivePlayerProcedural("defrag");
			g_pSubroutineMgr->GivePlayerProcedural("disinfect");

			uint16 nNumSubroutines = g_pSubroutineMgr->GetNumSubroutines();
			for (int i = 0; i < nNumSubroutines; i++)
			{
				g_pSubroutineMgr->GivePlayerSubroutine(g_pSubroutineMgr->GetSubroutineByIndex(i)->szName, "gold", "okay", true);
			}
		}
		break;

		case CHEAT_TRON_CLEARMEMORY:
		{
			g_pSubroutineMgr->Reset();
			g_pLTClient->CPrint("All subroutines removed!");
		}
		break;

		case CHEAT_TRON_CORRUPTION:
		{
			for (int i = 0; i < 200; i++)
			{
				g_pSubroutineMgr->Update();
			}
			g_pLTClient->CPrint("Corruption!");
		}
		break;

		case CHEAT_TRON_MAXPERFORMANCE:
		{
			for (int i = 0; i < 5; i++)
			{
				g_pTronPlayerStats->SetPerformanceRating((PerformanceRating)i, 100);
			}
			g_pLTClient->CPrint("Maxed performance ratings!");
		}
		break;

		case CHEAT_TRON_CLEARPERFORMANCE:
		{
			for (int i = 0; i < 5; i++)
			{
				g_pTronPlayerStats->SetPerformanceRating((PerformanceRating)i, 0);
			}
			g_pLTClient->CPrint("Cleared performance ratings!");
		}
		break;

		case CHEAT_TRON_IDEALWEAPONS:
		case CHEAT_TRON_IDEALSUBROUTINES:
		case CHEAT_TRON_IDEALPROCEDURALS:
		case CHEAT_TRON_IDEALPOINTS:
		case CHEAT_TRON_IDEAL:
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_CHEAT);
			cMsg.Writeuint8((uint8)nCheat);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}
		break;
	}
}
