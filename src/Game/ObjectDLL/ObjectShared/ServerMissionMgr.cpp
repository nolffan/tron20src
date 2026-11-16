// ----------------------------------------------------------------------- //
//
// MODULE  : ServerMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ServerMissionMgr.h"
#include "MsgIds.h"
#include "WinUtil.h"
#include "MissionButeMgr.h"
#include "ServerSaveLoadMgr.h"
#include "PlayerObj.h"
#include "DisplayTimer.h"
#include "Spawner.h"
#include "WorldProperties.h"

CServerMissionMgr* g_pServerMissionMgr = LTNULL;
extern CVarTrack g_vtNetFriendlyFire;

// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CServerMissionMgr::CServerMissionMgr()
{
	// RETAIL: 0x10161890
	m_bCustomLevel = false;
    m_nCurrentMission = -1;
    m_nCurrentLevel = -1;
	m_nCurCampaignIndex = -1;

	m_bTransitionLevels = false;
	m_bNewMission = false;
	m_bExitingLevel = false;
	m_bPlayerTrackerAborted = false;

	m_bUseSkills = false;
	m_bFriendlyFire = false;
	m_nMPDifficulty = 0;
	m_fPlayerDiffFactor = 0.0f;

	m_bTronUnknown4 = true;
	m_bMissionFailed = false;
}

CServerMissionMgr::~CServerMissionMgr()
{
	Term( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CServerMissionMgr::Init()
{
	g_pServerMissionMgr = this;
	m_bTransitionLevels = false;
	m_bCustomLevel  = false;
	m_nCurrentMission = -1;
	m_nCurCampaignIndex = -1;
	m_nCurrentLevel = -1;
	m_sCurrentWorldName.Empty( );
	m_sCampaignFile.Empty( );
	m_bNewMission = false;
	m_bExitingLevel = false;
	m_bPlayerTrackerAborted = false;
	m_bMissionFailed = false;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CServerMissionMgr::Term()
{
	g_pServerMissionMgr = LTNULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::GetLevelFromMission
//
//	PURPOSE:	Get the level name from mission info.
//
// --------------------------------------------------------------------------- //

char const* CServerMissionMgr::GetLevelFromMission( int nMission, int nLevel )
{

	MISSION* pMission = g_pMissionButeMgr->GetMission( nMission );
	if( !pMission )
	{
		ASSERT( !"CServerMissionMgr::GetLevelFromMission: Invalid mission." );
		return NULL;
	}

	if( nLevel < 0 || nLevel > pMission->nNumLevels )
	{
		ASSERT( !"CServerMissionMgr::GetLevelFromMission: Invalid mission info." );
		return NULL;
	}

	return pMission->aLevels[nLevel].szLevel;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetMissionBasedOnLevel
//
//	PURPOSE:	Sets mission info based on level.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::SetMissionBasedOnLevel( char const* pszFilename )
{
	// Check inputs.
	if( !pszFilename || !pszFilename[0] )
	{
		ASSERT( !"CServerMissionMgr::SetMissionBasedOnLevel: Invalid filename." );
		return false;
	}

	// See if the loaded world is a custom level or not...
	int nMissionId, nLevel;
	if( g_pMissionButeMgr->IsMissionLevel(( char* )pszFilename, nMissionId, nLevel ))
	{
		m_nCurrentMission	= nMissionId;
		m_nCurrentLevel		= nLevel;
		m_bCustomLevel		= false;
	}
	else
	{
		m_nCurrentMission	= -1;
		m_nCurrentLevel		= -1;
        m_bCustomLevel		= true;
	}

	// Save our world name.
	m_sCurrentWorldName = pszFilename;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::FindNextCampaignIndex
//
//	PURPOSE:	Finds the next occurrance of the mission in the campaign.
//
// --------------------------------------------------------------------------- //

int CServerMissionMgr::FindNextCampaignIndex( int nStartingCampaignIndex, int nMissionIndex )
{
	int nCampaignIndex = -1;

	// Check inputs.
	if( nMissionIndex < 0 || nMissionIndex >= g_pMissionButeMgr->GetNumMissions( ))
		return -1;

	// Starting from a campaign index, find the next occurance of the mission.
	for( int nIndex = nStartingCampaignIndex + 1; nIndex < ( int )m_Campaign.size( ); nIndex++ )
	{
		if( m_Campaign[nIndex] == nMissionIndex )
		{
			return nIndex;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Save()
//
//	PURPOSE:	Saves state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::Save( ILTMessage_Write& msg, uint32 dwSaveFlags )
{
	// RETAIL: 0x10161ba0
	msg.Writebool( m_bTronUnknown4 );
	msg.WriteString( m_sCampaignFile );

	msg.Writeuint8( m_Campaign.size( ));
	for( Campaign::iterator iter = m_Campaign.begin( ); iter != m_Campaign.end( ); iter++ )
	{
		msg.Writeuint8( *iter );
	}
	msg.Writeuint8( m_nCurCampaignIndex );

	msg.Writebool( m_bUseSkills );
	msg.Writebool( m_bFriendlyFire );
	msg.Writeuint8( m_nMPDifficulty );
	msg.Writefloat( m_fPlayerDiffFactor );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Load()
//
//	PURPOSE:	Loads state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::Load( ILTMessage_Read& msg, uint32 dwSaveFlags )
{
	// RETAIL: 0x10161c70
	m_bTronUnknown4 = msg.Readbool( );
	msg.ReadString( m_sCampaignFile.GetBuffer( MAX_PATH ), MAX_PATH );
	m_sCampaignFile.ReleaseBuffer( );

	m_Campaign.clear( );
	int nNumCampaignEntries = msg.Readuint8( );
	for( int nCampaignIndex = 0; nCampaignIndex < nNumCampaignEntries; ++nCampaignIndex )
	{
		m_Campaign.push_back( msg.Readuint8( ));
	}
	m_nCurCampaignIndex = msg.Readuint8( );

	m_bUseSkills = msg.Readbool( );
	m_bFriendlyFire = msg.Readbool( );
	m_nMPDifficulty = msg.Readuint8( );
	m_fPlayerDiffFactor = msg.Readfloat( );

	g_vtNetFriendlyFire.SetFloat( m_bFriendlyFire ? 1.0f : 0.0f );

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::OnMessage
//
//	PURPOSE:	Handle missionmgr messages.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::OnMessage( HCLIENT hSender, ILTMessage_Read& msg )
{
	msg.SeekTo(0);
	uint8 messageID = msg.Readuint8();

	switch (messageID)
	{
		case MID_START_GAME:			HandleStartGame			( hSender, msg );	return true;
		case MID_START_LEVEL:			HandleStartLevel		( hSender, msg );	return true;
		case MID_EXIT_LEVEL:			HandleExitLevel			( hSender, msg );	return true;

		default:
			break;
	}

	return false;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleStartGame
//
//	PURPOSE:	Handle start game message.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::HandleStartGame( HCLIENT hSender, ILTMessage_Read& msg )
{
	// RETAIL: 0x10161e70
	m_bTronUnknown4 = msg.Readbool( );

	// Initialize the bute file the client wants.
	char szMissionFile[MAX_PATH];
	msg.ReadString( szMissionFile, ARRAY_LEN( szMissionFile ));
	if( !g_pMissionButeMgr->Init( szMissionFile ))
		return false;

	m_nMPDifficulty = (uint8)g_pGameServerShell->GetDifficulty();

	// Start the game.
	if( !StartGame( msg ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleStartLevel
//
//	PURPOSE:	Start a level from scratch.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::HandleStartLevel( HCLIENT hSender, ILTMessage_Read& msg )
{
	char szStartLevel[MAX_PATH];

	msg.ReadString( szStartLevel, ARRAY_LEN( szStartLevel ));

	// Starting level from scratch, so this is a new mission for us.
	m_bNewMission = true;

	// Setup mission based on level.
	if( !SetMissionBasedOnLevel( szStartLevel ))
		return false;

	// Setup the campaign index.
	if( !m_bCustomLevel )
		m_nCurCampaignIndex = FindNextCampaignIndex( -1, m_nCurrentMission );

	// Load the level.
	if( !g_pServerSaveLoadMgr->LoadNewLevel( szStartLevel ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevelSwitch()
//
//	PURPOSE:	Called when it's time to go to the next level in the mission.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevelSwitch( )
{
	TRACE( "CServerMissionMgr::ExitLevelSwitch\n" );

	// Check if we're already exiting a level.
	if( m_bExitingLevel )
		return true;

	// If this was a custom level, then the game is over.
	if( m_bCustomLevel )
	{
		// Let the endgame code figure out what we should do.
		if( !EndGame( ))
			return false;

		return true;
	}

	// Get the mission.
	MISSION* pMission = g_pMissionButeMgr->GetMission(m_nCurrentMission);
	if( !pMission )
	{
        ASSERT( !"CServerMissionMgr::ExitLevelSwitch:  Invalid mission." );
		return false;
	}

	// Go to the next level.
	m_nCurrentLevel++;

	// Default to not a new mission.
	m_bNewMission = false;

	// See if we finished a mission.
	if( m_nCurrentLevel >= pMission->nNumLevels )
	{
		// Start level index over and advance the mission index.
		m_nCurrentLevel = 0;
		m_nCurCampaignIndex++;

		// See if we reached the end of our campaign.
		if( m_nCurCampaignIndex >= ( int )m_Campaign.size( ))
		{
			// Let the endgame code figure out what we should do.
			if( !EndGame( ))
				return false;

			return true;
		}

		// Get the missionid from the campaign.
		m_nCurrentMission = m_Campaign[m_nCurCampaignIndex];

		// Flag that this is a new mission so we can tell everyone else.
		m_bNewMission = true;
	}

	// Get the worldname from the mission/level indices.
	char const* pszFilename = GetLevelFromMission( m_nCurrentMission, m_nCurrentLevel );
	if( !pszFilename )
		return false;

	// Do the exit level.
	if( !ExitLevel( pszFilename, false ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::NextMission()
//
//	PURPOSE:	Called to force transition to the next mission.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::NextMission( )
{
	TRACE( "CServerMissionMgr::NextMission\n" );

	// Check if we're already exiting a level.
	if( m_bExitingLevel )
		return true;

	// If this was a custom level, then the game is over.
	if( m_bCustomLevel )
	{
		// Let the endgame code figure out what we should do.
		if( !EndGame( ))
			return false;

		return true;
	}

	// Get the mission.
	MISSION* pMission = g_pMissionButeMgr->GetMission(m_nCurrentMission);
	if( !pMission )
	{
        ASSERT( !"CServerMissionMgr::NextMission:  Invalid mission." );
		return false;
	}

	// Start level index over and advance the mission index.
	m_nCurrentLevel = 0;
	m_nCurCampaignIndex++;

	// See if we reached the end of our campaign.
	if( m_nCurCampaignIndex >= ( int )m_Campaign.size( ))
	{
		// Let the endgame code figure out what we should do.
		if( !EndGame( ))
			return false;

		return true;
	}

	// Get the missionid from the campaign.
	m_nCurrentMission = m_Campaign[m_nCurCampaignIndex];

	// Flag that this is a new mission so we can tell everyone else.
	m_bNewMission = true;

	// Get the worldname from the mission/level indices.
	char const* pszFilename = GetLevelFromMission( m_nCurrentMission, m_nCurrentLevel );
	if( !pszFilename )
		return false;

	// Do the exit level.
	if( !ExitLevel( pszFilename, false ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevelToLevel()
//
//	PURPOSE:	Called when you want to go to any arbitrary level.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevelToLevel( char const* pszNewLevel )
{
	TRACE( "CServerMissionMgr::ExitLevelToLevel\n" );

	// Check if we're already exiting a level.
	if( m_bExitingLevel )
		return true;

	// Verify inputs.
	if( !pszNewLevel )
	{
		ASSERT( !"CServerMissionMgr::ExitLevelToLevel: Invalid inputs." );
		return false;
	}

	// Default to this not being a mission change.
	m_bNewMission = false;

	// Check if the new level is a new mission.
	int nCurMission = m_nCurrentMission;
	if( !SetMissionBasedOnLevel( pszNewLevel ))
		return false;

	// If we switch to a custom level, then consider it a new mission.
	if( m_bCustomLevel )
	{
		m_bNewMission = true;
		m_nCurCampaignIndex = -1;
	}
	// If it's not a custom world, check if our mission changed.
	else
	{
		m_bNewMission = ( nCurMission != m_nCurrentMission );

		// If we had a mission switch, advance our campaign index.
		if( m_bNewMission )
		{
			// See if we can find the campaign index from our previous position.
			m_nCurCampaignIndex = FindNextCampaignIndex( m_nCurCampaignIndex, m_nCurrentMission );
		}
	}

	// Do the exit level.
	if( !ExitLevel( pszNewLevel, false ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevelTransition()
//
//	PURPOSE:	Called when level transition is needed.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevelTransition( int nNewLevel )
{
	TRACE( "CServerMissionMgr::ExitLevelTransition\n" );

	// Check if we're already exiting a level.
	if( m_bExitingLevel )
		return true;
	
	// Default to this not being a mission change.
	m_bNewMission = false;

	// Get the next level name.
	char const* pszFilename = GetLevelFromMission( m_nCurrentMission, nNewLevel );
	if( !pszFilename )
		return false;

	// Do the exit level.
	if( !ExitLevel( pszFilename, true ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevel()
//
//	PURPOSE:	Called by exitlevelXXX routines to go to a new level.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevel( char const* pszNewLevel, bool bTransitionToLevel )
{
	// RETAIL: 0x101621b0: Tron does not look for the worldproperties
	// object and has no start time.

	// Verify inputs.
	if( !pszNewLevel )
	{
		ASSERT( !"CServerMissionMgr::ExitLevel: Invalid inputs." );
		return false;
	}

	// Check if we're already exiting a level.
	if( m_bExitingLevel )
		return true;

	// Record that we are transitioning.
	m_bTransitionLevels = bTransitionToLevel;

	// Make sure mission information is setup for the new level.  The
	// campaign index should have already been updated.
	if( !SetMissionBasedOnLevel( pszNewLevel ))
		return false;

	// Initialize the PlayerTracker to the current set of players.
	m_bPlayerTrackerAborted = false;
	if( !m_PlayerTracker.Init( *this ))
	{
		// No players, just go to the next level.
		return FinishExitLevel( );
	}

	// Tell the client.
	if( !SendExitLevelMessage( ))
		return false;

	// Pause the server while we switch levels.
	g_pGameServerShell->PauseGame( LTTRUE );

	// We're now waiting for exitlevel messages back from client.
	m_bExitingLevel = true;

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleExitLevel
//
//	PURPOSE:	Handle the client's response to exit level.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::HandleExitLevel( HCLIENT hSender, ILTMessage_Read& msg )
{
	TRACE( "CServerMissionMgr::HandleExitLevel: hSender(0x%X)\n", hSender );

	// Get the playerobj for this client.
    void *pData = g_pLTServer->GetClientUserData(hSender);
	CPlayerObj* pPlayerObjSender = (CPlayerObj*)pData;
	if( !pPlayerObjSender )
	{
		ASSERT( !"CServerMissionMgr::HandleExitLevel: Invalid playerobj sender." );
		return false;
	}

	// Remove this player from the tracker.
	m_PlayerTracker.RemoveClient( hSender );

	// Check if we are still waiting for some clients to finish their exiting procedures.
	if( !m_PlayerTracker.IsEmpty( ))
		return true;

	return FinishExitLevel( );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::OnPlayerTrackerAbort
//
//	PURPOSE:	Handle when we have to abort a playertracker.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::OnPlayerTrackerAbort( )
{
	// Tell update to handle the playertrackerabort.  We can't continue
	// with the savedata right now because the player is still in the
	// process of getting deleted.
	m_bPlayerTrackerAborted = true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Update
//
//	PURPOSE:	Frame update.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::Update( )
{
	// RETAIL: 0x101622a0: Tron has no time limit here.

	// Update the playertracker.
	m_PlayerTracker.Update( );

	// Since all the players dropped out, we can just finish what we
	// were doing.
	if( m_bPlayerTrackerAborted )
	{
		// Clear the abort signal.
		m_bPlayerTrackerAborted = false;
		FinishExitLevel( );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::FinishExitLevel
//
//	PURPOSE:	Finish the exitlevel we started.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::FinishExitLevel( )
{
	// RETAIL: 0x101622c0

	// No longer waiting for exitlevel messages back from client.
	m_bExitingLevel = false;

	// Clear out the playertracker.
	m_PlayerTracker.Term( );

	// Tell the players to handle an exit level.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		pPlayerObj->HandleExit( m_bNewMission );
		iter++;
	}

	if( m_bNewMission )
	{
		// Clear out our objectives...
		g_pGameServerShell->ResetObjectives();
	}

	m_bNewMission = false;

	// Default to failure.
	bool bRet = false;

	// Do transition.
	if( m_bTransitionLevels )
	{
		m_bTransitionLevels = false;
		bRet = g_pServerSaveLoadMgr->TransitionLevels( m_sCurrentWorldName );
	}
	// Do switch.
	else
	{
		// PORT: Tron's SwitchLevels decides on the keep alive save itself
		bRet = g_pServerSaveLoadMgr->SwitchLevels( m_sCurrentWorldName );
	}

	// Unpause the server now that we're done switching levels.
	g_pGameServerShell->PauseGame( LTFALSE );	

	return bRet;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SendExitLevelMessage
//
//	PURPOSE:	Tell client to exit level.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::SendExitLevelMessage( )
{
	// RETAIL: 0x10162350

	// Tell the players to prepare to exit a level.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		pPlayerObj->HandlePreExit();
		iter++;
	}

	// See if we have already visited the level we are going to.
	CString sLoadGameFile = g_pServerSaveLoadMgr->GetWorldSaveFile( m_sCurrentWorldName );
	bool bRestoringLevel = !!CWinUtil::FileExist( sLoadGameFile );

	// Tell client's we're exiting.  Wait for their response.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_EXIT_LEVEL );
	cMsg.WriteString( m_sCurrentWorldName );
	cMsg.Writebool( m_bNewMission );
	cMsg.Writebool( bRestoringLevel );
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::EndGame
//
//	PURPOSE:	The game has ended.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::EndGame( )
{
	// RETAIL: 0x1000a4d0 (a shared "return true" body): Tron does not look
	// for the worldproperties object.
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::StartGame
//
//	PURPOSE:	Default start game.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::StartGame(ILTMessage_Read& msg )
{
	// Setup the campaign from the campaign file.
	SetupCampaign( m_sCampaignFile );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetupCampaign
//
//	PURPOSE:	Setup the campaign list of missions.  The campaign
//				defines which missions we will play.  Having a blank
//				campaign file means that all missions will be used.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::SetupCampaign( char const* pszCampaignName )
{
	// Clear all missions from our campaign.
	m_Campaign.clear( );

	// RETAIL: 0x10162590: Tron reads the campaign from the given file.
	char const* pszCampaignFile = pszCampaignName;

	// If the campaignfile is blank, then just use all missions.
	if( !pszCampaignFile || !pszCampaignFile[0] )
	{
		for( int nMissionId = 0; nMissionId < g_pMissionButeMgr->GetNumMissions( ); nMissionId++ )
		{
			m_Campaign.push_back( nMissionId );
		}

		// Default to not looping.
		m_bLoopCampaign = false;

		return true;
	}

	// Read the missionid's for this campaign.
	char szMissionId[4];
	char szCampaignIndexKeyName[32];
	int nCampaignIndex = 0;
	while( 1 )
	{
		// Create the key name for this entry.
		sprintf( szCampaignIndexKeyName, "Mission%d", nCampaignIndex );

		// Get the missionid for this entry.
		CWinUtil::WinGetPrivateProfileString( "MissionList", szCampaignIndexKeyName, "", szMissionId, 
			ARRAY_LEN( szMissionId ), pszCampaignFile );

		// If there wasn't a missionid, then we reached the end.
		if( !szMissionId[0] )
			break;

		// Get the mission id.
		int nMissionId = atoi( szMissionId );

		// Make sure it's a valid mission.
		if( nMissionId < g_pMissionButeMgr->GetNumMissions( ))
			m_Campaign.push_back( nMissionId );

		nCampaignIndex++;
	}

	// Get the looping flag.
	char szLoop[4];
	CWinUtil::WinGetPrivateProfileString( "MissionList", "LoopMissions", "0", szLoop, 
		ARRAY_LEN( szLoop ), pszCampaignFile );
	m_bLoopCampaign = !!atoi( szLoop );

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetMissionFailed()
//
//	PURPOSE:	Set or clear mission failed state
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::SetMissionFailed(bool bFail)
{ 
	// RETAIL: 0x10162730: Tron sends no message to the server app.
	if (bFail != m_bMissionFailed)
	{
		g_pGameServerShell->PauseGame(bFail);
	}
	m_bMissionFailed = bFail; 
}
