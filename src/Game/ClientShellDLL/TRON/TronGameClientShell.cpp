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

// Sample rate
int g_nSampleRate = 44100;

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

	g_pSubroutineMgr->Pause(bPause);	
	g_pTronPlayerMgr->Pause(bPause);
}
