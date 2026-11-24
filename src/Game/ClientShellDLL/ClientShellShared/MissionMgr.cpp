// ----------------------------------------------------------------------- //
//
// MODULE  : MissionMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ServerOptionMgr.h"
#include "MissionMgr.h"
#include "GameClientShell.h"
#include "VarTrack.h"
#include "MsgIds.h"
#include "WinUtil.h"
#include "MissionButeMgr.h"
#include "PlayerMgr.h"
#include "ClientSaveLoadMgr.h"
#include "clientresshared.h"
#include "ClientMultiplayerMgr.h"
#include "ProfileMgr.h"
#include "ScreenPreload.h"
#include "ClientWeaponMgr.h"

extern VarTrack g_vtScreenFadeInTime;

CMissionMgr* g_pMissionMgr = LTNULL;

// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CMissionMgr::CMissionMgr()
{
	m_bCustomLevel = false;
    m_nCurrentMission = -1;
    m_nCurrentLevel = -1;
	m_bExitingLevel = false;
	m_bExitingMission = false;
	m_eStartGameState = eStartGameUnknown;
	m_nSaveSlot = -1;
	m_bNewMission = true;
	m_bRestoringLevel = false;
	// RETAIL: 0x100eb5d0, 0x100eb6e0: m_bServerWaiting is left alone
    m_nNewMission = -1;
    m_nNewLevel = -1;
	m_bGameOver = false;
}

CMissionMgr::~CMissionMgr()
{
	Term( );

	g_pMissionMgr = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::Init()
{
	// Start fresh.
	Term( );

	g_pMissionMgr = this;

	m_bCustomLevel = false;
    m_nCurrentMission = -1;
    m_nCurrentLevel = -1;
	m_bExitingLevel = false;
	m_bExitingMission = false;
	m_bNewMission = true;
	m_bRestoringLevel = false;
    m_nNewMission = -1;
    m_nNewLevel = -1;

	m_sCurrentWorldName.Empty( );
	m_sNewWorldName.Empty( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CMissionMgr::Term()
{
	g_pMissionMgr = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameNew
//
//	PURPOSE:	Start the game from the beginning.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameNew( )
{
	// RETAIL: 0x100eb740: Tron's single player game only; the client weapons
	// are reset after the level is set
	if( IsMultiplayerGame( ))
		return false;

	// Get the level name for the first mission/level.
	char const* pszLevelFilename = GetLevelFromMission( 0, 0 );
	if( !pszLevelFilename )
		return false;

	// Start from the first level.
	if( !StartGameFromLevel( pszLevelFilename ))
		return false;

	CClientWeaponMgr* pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	if( pClientWeaponMgr )
		pClientWeaponMgr->ResetWeapons();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromLevel
//
//	PURPOSE:	Start the game from a level
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromLevel( char const* pszFilename )
{
	// Check inputs.
	if( !pszFilename || !pszFilename[0] )
	{
		ASSERT( !"CMissionMgr::StartGameFromLevel: Invalid filename." );
		return false;
	}

	// RETAIL: 0x100eb790: no StartingNewGame or server waiting in Tron

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	ClearMissionInfo( );

	m_eStartGameState = eStartGameFromLevel;
	// Set our new level name.
	if( !SetNewLevel( pszFilename ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartPerformanceLevel
//
//	PURPOSE:	Start the game from the performance level
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartPerformanceLevel()
{
	// RETAIL: 0x100eb7d0: no StartingNewGame or server waiting in Tron

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	ClearMissionInfo( );

	// [KLS 8/30/02] Make sure the server info is setup correctly...
	g_pClientMultiplayerMgr->SetupServerSinglePlayer();

	m_eStartGameState = eStartGameFromLevel;
	// Set our new level name.
	if( !SetNewLevel("worlds\\RetailSinglePlayer\\performance" ))
		return false;

	return true;
}



bool CMissionMgr::FinishStartGameFromLevel()
{
	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	if( !g_pClientMultiplayerMgr->StartClientServer( ))
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	// Send the start game message.
	if( !SendStartGameMessage( ))
		return false;

	// Tell the server to start with this level.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_START_LEVEL );
	cMsg.WriteString( m_sCurrentWorldName );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromQuickSave
//
//	PURPOSE:	Start the game from quicksave.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromQuickSave( )
{
	// RETAIL: 0x100eb940: no StartingNewGame in Tron; without a quick save
	// the mission info is cleared and the loading level set before the
	// load fails
	if( !g_pClientSaveLoadMgr->QuickSaveExists( ))
	{
		ClearMissionInfo( );
		SetLoadingLevel( );
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( QUICKSAVE_INIKEY, NULL, 0, szWorldName, ARRAY_LEN( szWorldName ),
		NULL ))
	{
		ClearMissionInfo( );
		SetLoadingLevel( );
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	ClearMissionInfo( );

	// Set this as the level we'll be playing.
	if( !SetNewLevel( szWorldName ))
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}
	m_eStartGameState = eStartGameFromQuickSave;

	return true;
}


// RETAIL: 0x100eb9f0: Tron's finish functions set m_bServerWaiting, and
// clear it when the load fails
bool CMissionMgr::FinishStartGameFromQuickSave( )
{
	bool bOk = true;

	m_bServerWaiting = true;
	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	bOk = bOk && g_pClientMultiplayerMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the quickload.
	bOk = bOk && g_pClientSaveLoadMgr->QuickLoad( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		m_bServerWaiting = false;
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromSaveSlot
//
//	PURPOSE:	Start the game from save slot.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromSaveSlot( int nSlot )
{
	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->SlotSaveExists( nSlot ))
		return false;

	// RETAIL: 0x100eba50: no StartingNewGame in Tron

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( g_pClientSaveLoadMgr->GetSlotSaveKey( nSlot ), NULL, 0, 
		szWorldName, ARRAY_LEN( szWorldName ),	NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	ClearMissionInfo( );
	m_bServerWaiting = true;

	// Set this as the level we'll be playing.
	if( !SetNewLevel( szWorldName ))
	{
		g_pInterfaceMgr->LoadFailed( );
		m_bServerWaiting = false;
		return false;
	}

	m_nSaveSlot = nSlot;
	m_eStartGameState = eStartGameFromSaveSlot;

	return true;
}


bool CMissionMgr::FinishStartGameFromSaveSlot( )
{
	bool bOk = true;

	m_bServerWaiting = true;
	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	bOk = bOk && g_pClientMultiplayerMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the load.
	bOk = bOk && g_pClientSaveLoadMgr->LoadGameSlot( m_nSaveSlot );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		m_bServerWaiting = false;
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromReload
//
//	PURPOSE:	Start the game from the reload save.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromReload( )
{
	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->ReloadSaveExists( ))
		return false;

	// RETAIL: 0x100ebb80: no StartingNewGame in Tron

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( RELOADLEVEL_INIKEY, NULL, 0, 
		szWorldName, ARRAY_LEN( szWorldName ),	NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	ClearMissionInfo( );
	m_bServerWaiting = true;

	// Set this as the level we'll be playing.
	if( !SetNewLevel( szWorldName ))
	{
		g_pInterfaceMgr->LoadFailed( );
		m_bServerWaiting = false;
		return false;
	}

	m_eStartGameState = eStartGameFromReload;

	return true;
}

bool CMissionMgr::FinishStartGameFromReload( )
{
	m_eStartGameState = eStartGameStarted;
	m_bServerWaiting = true;

	bool bOk = true;

	// Make sure we have a server started.
	bOk = bOk && g_pClientMultiplayerMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the reload.
	bOk = bOk && g_pClientSaveLoadMgr->ReloadLevel( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		m_bServerWaiting = false;
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromContinue
//
//	PURPOSE:	Start the game from the continue save.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromContinue( )
{
	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->CanContinueGame( ))
		return false;

	// RETAIL: 0x100ebc80: no StartingNewGame in Tron

	// Get the savekey to use.
	char szSaveKey[SLMGR_MAX_INIKEY_LEN];
	if( !g_pClientSaveLoadMgr->ReadContinueINI( szSaveKey, ARRAY_LEN( szSaveKey ), NULL, 0, NULL, 0 ))
		return false;

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( szSaveKey, NULL, 0, 
		szWorldName, ARRAY_LEN( szWorldName ),	NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	ClearMissionInfo( );
	m_bServerWaiting = true;

	// Set this as the level we'll be playing.
	if( !SetNewLevel( szWorldName ))
	{
		g_pInterfaceMgr->LoadFailed( );
		m_bServerWaiting = false;
		return false;
	}

	m_eStartGameState = eStartGameFromContinue;

	return true;
}

bool CMissionMgr::FinishStartGameFromContinue( )
{
	m_eStartGameState = eStartGameStarted;
	m_bServerWaiting = true;

	bool bOk = true;

	// Make sure we have a server started.
	bOk = bOk && g_pClientMultiplayerMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the continue.
	bOk = bOk && g_pClientSaveLoadMgr->ContinueGame( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		m_bServerWaiting = false;
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameAsClient
//
//	PURPOSE:	Start the game as a remote client.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameAsClient( )
{
	// RETAIL: 0x100ebdb0: no server waiting or StartingNewGame in Tron

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	ClearMissionInfo( );

	// Go to the loading state.
	if( !SetLoadingLevel( ))
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	m_eStartGameState = eStartGameAsClient;

	return true;
}


bool CMissionMgr::FinishStartGameAsClient( )
{
	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	if( !g_pClientMultiplayerMgr->StartClientServer( ))
		return false;

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

bool CMissionMgr::OnMessage( uint8 messageID, ILTMessage_Read & msg )
{

	// RETAIL: 0x100ebe10: Tron's messages 194 and 195 go to the virtuals
	switch(messageID)
	{
		case MID_EXIT_LEVEL:		HandleExitLevel			( msg );	return true;
		case MID_TRON_UNKNOWN_194:	TronUnknownVirtual4		( msg );	return true;
		case MID_TRON_UNKNOWN_195:	TronUnknownVirtual8		( msg );	return true;
		case MID_END_GAME:			HandleEndGame			( msg );	return true;
	}

	return false;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleMissionFailed
//
//	PURPOSE:	Handle mission failure
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleMissionFailed()
{
	g_pPlayerMgr->ClearPlayerModes();
	// [KLS 7/28/02] - Interface mgr handles forcing screen fading in
	// if necessary, doing it here may cause problems...
	///g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
	// RETAIL: 0x100ebea0: Tron's string ids (CRes.dll): 595 "Fatal
	// compression error!", 1011 "END OF LINE."
	int nStringId = (g_pPlayerMgr->GetTronUnknown1E0() == 10 ? 595 : 1011);
	g_pInterfaceMgr->MissionFailed(nStringId);

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleExitLevel
//
//	PURPOSE:	Handle the server telling us to prepare for exit level.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleExitLevel( ILTMessage_Read& msg  )
{
	// RETAIL: 0x100ebee0: no server waiting flag in Tron's message; the
	// loading screen is hidden before exiting
	msg.ReadString( m_sNewWorldName.GetBuffer( MAX_PATH ), MAX_PATH );
	m_sNewWorldName.ReleaseBuffer( );
	m_bExitingMission = msg.Readbool( );
	m_bRestoringLevel = msg.Readbool( );

	m_bNewMission = m_bExitingMission;

	int nMissionId, nLevel;

	// Check if this is a valid mission level.
	if (g_pMissionButeMgr->IsMissionLevel( m_sNewWorldName, nMissionId, nLevel))
	{
		m_nNewMission = nMissionId;
		m_nNewLevel = nLevel;
	}
	else
	{
		m_nNewMission = -1;
		m_nNewLevel = -1;
	}

	// Check if we didn't get a new world.
	if( m_sNewWorldName.IsEmpty( ))
	{
		// Just go back to the main menu.
		g_pInterfaceMgr->ChangeState( GS_SCREEN );
		return true;
	}

	// Consider ourselves to be exiting.
	m_bExitingLevel = true;

	g_pInterfaceMgr->HideLoadScreen( );

	// Change to the exiting level state.
	g_pInterfaceMgr->ChangeState(GS_EXITINGLEVEL);

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleEndGame
//
//	PURPOSE:	Handle the server telling us to prepare for exit level.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleEndGame( ILTMessage_Read& msg  )
{
	// Go to the main menu.
	g_pInterfaceMgr->ChangeState(GS_SCREEN);

	m_bGameOver = true;

	if (g_pGameClientShell->IsRunningPerformanceTest())
	{
		//since our history was cleared by loading the level... rebuild it
		g_pInterfaceMgr->GetScreenMgr()->AddScreenToHistory( SCREEN_ID_MAIN );
		g_pInterfaceMgr->GetScreenMgr()->AddScreenToHistory( SCREEN_ID_OPTIONS );
	
		//we're done performance testing go back to performance screen
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PERFORMANCE);
	}
	// RETAIL: 0x100ebfb0: Tron goes back to the main screen after a single
	// player game (no end mission screen, no EndGame registry key) and
	// nowhere after a multiplayer one
	else if (!IsMultiplayerGame())
	{
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::FinishExitLevel
//
//	PURPOSE:	Called when interfacemgr is finished doing exit level tasks.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::FinishExitLevel( )
{
	// We already handled the exiting level.
	if( !m_bExitingLevel )
		return true;

	m_bExitingLevel = false;
	m_bExitingMission = false;

	// Tell the server we're done exiting.
	// RETAIL: 0x100ec030: always
	SendEmptyServerMsg( MID_EXIT_LEVEL, MESSAGE_GUARANTEED );

	// Set our new level information.
	if( !SetNewLevel( m_sNewWorldName ))
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::FinishStartGame
//
//	PURPOSE:	Called when interfacemgr is finished doing start game tasks.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::FinishStartGame( )
{
	g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);
	
	// RETAIL: 0x100ec080: no message histories or transmission here in Tron
	
	switch (m_eStartGameState)
	{
	case eStartGameFromLevel:
		return FinishStartGameFromLevel( );
	case eStartGameFromQuickSave:
		return FinishStartGameFromQuickSave( );
	case eStartGameFromSaveSlot:
		return FinishStartGameFromSaveSlot();
	case eStartGameFromReload:
		return FinishStartGameFromReload( );
	case eStartGameFromContinue:
		return FinishStartGameFromContinue( );
	case eStartGameAsClient:
		return FinishStartGameAsClient( );
	case eStartGameStarted:
		return true;
	default:
		g_pInterfaceMgr->LoadFailed( );
		return false;

	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::SetNewLevel
//
//	PURPOSE:	Initialize new level information.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::SetNewLevel( char const* pszWorldName )
{
	// Check inputs.
	if( !pszWorldName || !pszWorldName[0] )
	{
		ASSERT( !"CMissionMgr::SetNewLevel:  Invalid world name." );
		return false;
	}

	m_sCurrentWorldName = pszWorldName;

	// See if the loaded world is a custom level or not...

	int nMissionId, nLevel;

	// Check if this is a valid mission level.
	if (g_pMissionButeMgr->IsMissionLevel( pszWorldName, nMissionId, nLevel))
	{
		// Check if we're switching missions.
		if( m_bNewMission )
		{
			// Starting new mission.
			ClearMissionInfo( );
		}

		m_nCurrentMission	= nMissionId;
		m_nCurrentLevel		= nLevel;
		m_nNewMission	= nMissionId;
		m_nNewLevel		= nLevel;
        m_bCustomLevel    = false;
	}
	// This is a custom level.
	else
	{
        m_bCustomLevel = true;
		m_nCurrentMission = -1;
		m_nCurrentLevel = -1;
		m_bNewMission = true;
		m_nNewMission = -1;
		m_nNewLevel = -1;
	}

	// Go to the loading state.
	if( !SetLoadingLevel( ))
		return false;


	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::SetLoadingLevel
//
//	PURPOSE:	Go to loading level state.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::SetLoadingLevel( )
{
	// Clear the input buffer.
	g_pLTClient->ClearInput();
	
	// Toss any cameras.
	g_pPlayerMgr->TurnOffAlternativeCamera( CT_FULLSCREEN );

	g_pGameClientShell->GetPlayerMgr()->ResetCamera();

	// Consider the world not loaded.
	g_pGameClientShell->SetWorldNotLoaded( );

	// Clear the disconnect flags.
	g_pInterfaceMgr->SetIntentionalDisconnect( false );

	if (GS_LOADINGLEVEL == g_pInterfaceMgr->GetGameState())
	{
		g_pInterfaceMgr->UpdateLoadScreenInfo();
		return true;
	}

	// RETAIL: 0x100ec1c0: Tron goes to the preload screen; no server
	// waiting or skip pre-load checks
	{
		CScreenPreload *pPreload = (CScreenPreload *) (g_pInterfaceMgr->GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
  		if (pPreload)
  		{
  			pPreload->SetWaitingToExit(false);
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);
  		}
  		else
  			ASSERT(!"No Preload screen available.");
		
	}

	return true;
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::GetLevelFromMission
//
//	PURPOSE:	Get the level name from mission info.
//
// --------------------------------------------------------------------------- //

char const* CMissionMgr::GetLevelFromMission( int nMission, int nLevel )
{
	MISSION* pMission = g_pMissionButeMgr->GetMission( nMission );
	if( !pMission )
	{
		ASSERT( !"CMissionMgr::GetLevelFromMission: Invalid mission." );
		return NULL;
	}

	if( nLevel < 0 || nLevel > pMission->nNumLevels )
	{
		ASSERT( !"CMissionMgr::GetLevelFromMission: Invalid mission info." );
		return NULL;
	}

	return pMission->aLevels[nLevel].szLevel;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::PreLoadWorld
//
//	PURPOSE:	Handle pre-load world.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::PreLoadWorld( char const* pszNewWorldName )
{
	// Check inputs.
	if( !pszNewWorldName )
	{
		ASSERT( !"CMissionMgr::PreLoadWorld: Invalid inputs." );
		return false;
	}

	char szTmp[256];
	SAFE_STRCPY(szTmp,pszNewWorldName);
	strtok(szTmp,".");

	// Check if we already set ourselves up for this world.
//	if( !m_sCurrentWorldName.IsEmpty( ) && m_sCurrentWorldName.CompareNoCase( szTmp ) == 0 )
//		return true;

	// If we just joined a remote server, this is the first time we
	// know the new world name.
	if( !SetNewLevel( szTmp ))
		return false;

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::ClearMissionInfo
//
//	PURPOSE:	Clear the mission information when a new mission is started.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::ClearMissionInfo( )
{
	// RETAIL: 0x100ec300: Tron's CPlayerStats has no ClearMissionInfo; the
	// mission damage and stats are reset and the objective lists cleared
	// here
	g_pPlayerStats->ClearMissionDamage();
	g_pPlayerStats->ResetMissionStats();
	g_pPlayerStats->GetObjectives()->Clear();
	g_pPlayerStats->GetOptionalObjectives()->Clear();
	g_pPlayerStats->GetCompletedObjectives()->Clear();
	g_pPlayerStats->GetParameters()->Clear();

	m_bCustomLevel = false;
    m_nCurrentMission = -1;
    m_nCurrentLevel = -1;
    m_nNewMission = -1;
    m_nNewLevel = -1;
	m_bServerWaiting = false;
	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::SendStartGameMessage
//
//	PURPOSE:	Clear the mission information when a new mission is started.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::SendStartGameMessage( )
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	m_bGameOver = false;

	// Tell the server to start with the specified bute.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_START_GAME );
	// RETAIL: 0x100ec3e0: Tron adds the profile's m_bBodiesAllowed (+0x73)
	// and the mission bute file
	cMsg.Writebool( pProfile->m_bBodiesAllowed );
	cMsg.WriteString( g_pMissionButeMgr->GetAttributeFile( ));

	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::GetCurrentWorldName
//
//	PURPOSE:	The current world's name (RETAIL: 0x100ec4e0)
//
// --------------------------------------------------------------------------- //
char const* CMissionMgr::GetCurrentWorldName() const
{
	// RETAIL: an empty string without a buffer
	char const* pszWorldName = m_sCurrentWorldName;
	return pszWorldName ? pszWorldName : "";
}
