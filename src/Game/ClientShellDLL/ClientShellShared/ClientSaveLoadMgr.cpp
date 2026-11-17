// ----------------------------------------------------------------------- //
//
// MODULE  : SaveLoadMgr.cpp
//
// PURPOSE : Manages the Saving and Loading of games..
//
// CREATED : 12/06/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "ClientSaveLoadMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "MsgIds.h"
#include "ClientMultiplayerMgr.h"
#include "CMoveMgr.h"
#include "ClientResShared.h"
#include "HUDDecision.h"
#include "VehicleMgr.h"

// 
// Globals...
//

CClientSaveLoadMgr *g_pClientSaveLoadMgr = LTNULL;
	
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::CClientSaveLoadMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CClientSaveLoadMgr::CClientSaveLoadMgr( )
{
	ASSERT( g_pClientSaveLoadMgr == LTNULL );

	g_pClientSaveLoadMgr = this;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::~CClientSaveLoadMgr
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CClientSaveLoadMgr::~CClientSaveLoadMgr( )
{
	Term( );

	ASSERT( g_pClientSaveLoadMgr != LTNULL );

	g_pClientSaveLoadMgr = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::Init
//
//  PURPOSE:	Initializes the object.
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::Init( char const* pszProfileName, bool bUseMultiplayerFolders )
{
	if( !CSaveLoadMgr::Init( pszProfileName, bUseMultiplayerFolders ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::Term
//
//  PURPOSE:	Terminates the object.
//
// ----------------------------------------------------------------------- //

void CClientSaveLoadMgr::Term( )
{
	CSaveLoadMgr::Term( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::OnCommandOn
//
//  PURPOSE:	Handle a save/load command
//
// ----------------------------------------------------------------------- //



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::QuickSave
//
//  PURPOSE:	Handle quick saving of the game...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::QuickSave( )
{
	// Make sure it's valid to save.  RETAIL: 0x10144598: Tron says why not
	// (one string for the light cycle games) with a sound
	if( !CanSaveGame() )
	{
		GameType eGameType = GetGameType();
		const char* pszMsg = LoadTempString((eGameType > eGameTypeSingle && eGameType <= eGameTypeSingleLightCycleCustom) ? 0x455 : 0x3fa);
		g_pChatMsgs->AddMessage(pszMsg);
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\Nosave.wav");
		return false;
	}

	g_pGameClientShell->CSPrint( LoadTempString( 0x3f4 ));	// RETAIL: Tron's string

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_GAME );
	cMsg.Writeuint8( 0 );
    g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::QuickLoad
//
//  PURPOSE:	Handle the loading of the quick save file...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::QuickLoad( )
{
	if( !CanLoadGame( ) || !QuickSaveExists( ))
		return false;

	g_pPlayerMgr->TurnOffAlternativeCamera(CT_FULLSCREEN);

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeQuick );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::SaveGameSlot
//
//  PURPOSE:	Save a named game to a specific slot...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::SaveGameSlot( uint32 nSlot, char const* pszSaveName )
{
	// Are we trying to save to the Quick save slot?

	if( nSlot == 0 ) 
		return QuickSave();

	if( !pszSaveName )
		return false;

	// Make sure it's valid to save.
	if( !CanSaveGame( ))
	{
		// RETAIL: 0x1014482d: Tron says no with a sound
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\Nosave.wav");
		return false;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_GAME );
	cMsg.Writeuint8( nSlot );
	cMsg.WriteString( pszSaveName );
    g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::LoadGameSlot
//
//  PURPOSE:	Load a game from a specific slot...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::LoadGameSlot( uint32 nSlot )
{
	if( !CanLoadGame( ) || !SlotSaveExists( nSlot )) 
		return false;

	g_pPlayerMgr->TurnOffAlternativeCamera(CT_FULLSCREEN);

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeSlot );
	cMsg.Writeuint8( nSlot );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::ReloadLevel
//
//  PURPOSE:	Reload the current level...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::ReloadLevel( )
{
	if( !CanLoadGame( ) || !ReloadSaveExists() ) 
		return false;

	g_pPlayerMgr->TurnOffAlternativeCamera(CT_FULLSCREEN);

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeReload );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::ContinueGame
//
//  PURPOSE:	Load the last saved game...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::ContinueGame()
{
	// Are we allowed to continue?
	if( !CanLoadGame( ) || !CanContinueGame() ) 
		return false;

	g_pPlayerMgr->TurnOffAlternativeCamera(CT_FULLSCREEN);

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeContinue );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::CanSaveGame
//
//  PURPOSE:	Test if we are allowed to save...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::CanSaveGame()	const
{ 
	// RETAIL: 0x10144c70: Tron's tests.  Not between worlds, nor in the
	// single player light cycle games, nor while an open decision window
	// keeps the player (unless its +0x4d is set), nor on a vehicle
	if (g_pMissionMgr->IsExitingLevel())
		return false;

	bool bDecisionAllows = true;
	if (g_pDecision && g_pDecision->IsVisible() && !g_pDecision->GetTronUnknown4d())
	{
		bDecisionAllows = false;
	}

	// Handle single player.
	if( !IsMultiplayerGame( ))
	{
		if (GetGameType() == eGameTypeSingleLightCycleCircuit || GetGameType() == eGameTypeSingleLightCycleCustom)
			return false;

		return (g_pPlayerMgr->IsPlayerInWorld() && 
			!g_pPlayerMgr->IsPlayerDead() && 
			!g_pPlayerMgr->IsUsingExternalCamera() &&
			!g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() &&
			bDecisionAllows);
	}
	// Handle multiplayer.
	else
	{
		// Remote clients can't do saves.
		if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			return false;

		bool bCanSave = (g_pGameClientShell->GetGameType( ) == eGameTypeCooperative);
		return bCanSave;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::CanLoadGame
//
//  PURPOSE:	Test if we are allowed to Load...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::CanLoadGame( ) const
{ 
	// RETAIL: 0x10144d50: Tron's tests
	if (g_pMissionMgr->IsExitingLevel())
		return false;

	// Handle single player.
	if( !IsMultiplayerGame( ))
	{
		if (GetGameType() == eGameTypeSingleLightCycleCircuit || GetGameType() == eGameTypeSingleLightCycleCustom)
			return false;

		return true;
	}
	// Handle multiplayer.
	else
	{
		// Remote clients can't load.
		if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			return false;

		bool bCanSave = (g_pGameClientShell->GetGameType( ) == eGameTypeCooperative);
		return bCanSave;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSaveLoadMgr::OnMessage
//
//	PURPOSE:	Handle messages.
//
// --------------------------------------------------------------------------- //

bool CClientSaveLoadMgr::OnMessage( uint8 messageID, ILTMessage_Read& msg )
{
	switch (messageID)
	{
		case MID_SAVE_DATA:		HandleSaveData			( msg );	return true; break;
		// RETAIL: Tron's handles no MID_SAVE_GAME (0x10144dc0)
		default:
			break;
	}

	return false;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSaveLoadMgr::HandleSaveData
//
//	PURPOSE:	Handle savedata message from server.
//
// --------------------------------------------------------------------------- //

bool CClientSaveLoadMgr::HandleSaveData( ILTMessage_Read& msg )
{
	SaveDataState eSaveDataState = static_cast<SaveDataState>(msg.Readuint8());

	// If we're not the host, then let the user know a save is happening.
	// Don't show autosaves.
	if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) && eSaveDataState != eSaveDataAutoSave )
		g_pGameClientShell->CSPrint( LoadTempString( 0x3f9 ));	// RETAIL: Tron's string
	
	// Give the server our save data.  RETAIL: 0x10144e7e: Tron's has no
	// jump test (nor its flag)

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_DATA) ;
	g_pGameClientShell->BuildClientSaveMsg( cMsg, eSaveDataState );


	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSaveLoadMgr::HandleSaveGameMsg
//
//	PURPOSE:	Handle savedata message from server.
//
// --------------------------------------------------------------------------- //

bool CClientSaveLoadMgr::HandleSaveGameMsg( ILTMessage_Read& msg )
{
	bool bSucceed = msg.Readbool();

	if (!bSucceed)
	{
		// Can't quicksave now...
		g_pGameClientShell->CSPrint(LoadTempString(IDS_CANTQUICKSAVE));
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\Nosave.wav");
	}

	return true;
}