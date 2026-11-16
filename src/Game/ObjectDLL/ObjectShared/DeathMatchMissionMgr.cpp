// ----------------------------------------------------------------------- //
//
// MODULE  : DeathMatchMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "DeathMatchMissionMgr.h"
#include "PlayerObj.h"
#include "ServerSaveLoadMgr.h"
#include "MissionButeMgr.h"


// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CDeathMatchMissionMgr::CDeathMatchMissionMgr()
{
}

CDeathMatchMissionMgr::~CDeathMatchMissionMgr()
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CDeathMatchMissionMgr::Init()
{
	if( !CServerMissionMgr::Init( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CDeathMatchMissionMgr::Term()
{
	CServerMissionMgr::Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Save()
//
//	PURPOSE:	Saves state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::Save( ILTMessage_Write& msg, uint32 dwSaveFlags )
{
	if( !CServerMissionMgr::Save( msg, dwSaveFlags ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Load()
//
//	PURPOSE:	Loads state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::Load( ILTMessage_Read& msg, uint32 dwSaveFlags )
{
	if( !CServerMissionMgr::Load( msg, dwSaveFlags ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Update
//
//	PURPOSE:	Frame update.
//
// --------------------------------------------------------------------------- //

void CDeathMatchMissionMgr::Update( )
{
	// RETAIL: 0x101cc680: Tron has no score limit check here.
	CServerMissionMgr::Update( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::FinishExitLevel
//
//	PURPOSE:	Finish the exitlevel we started.
//
// --------------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::FinishExitLevel( )
{
	TRACE( "CDeathMatchMissionMgr::FinishExitLevel:\n" );

	// No longer waiting for exitlevel messages back from client.
	m_bExitingLevel = false;

	// Clear out the playertracker.
	m_PlayerTracker.Term( );

	// Tell the players to handle an exit level.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		pPlayerObj->HandleExit( true );
		iter++;
	}

	// Load new level with no restoring of keepalives or save games.
	// RETAIL: 0x101cc690: Tron does not unpause the server here.
	if( !g_pServerSaveLoadMgr->LoadNewLevel( m_sCurrentWorldName ))
		return false;

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::StartGame
//
//	PURPOSE:	Start the game.
//
// --------------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::StartGame(ILTMessage_Read& msg )
{
	// RETAIL: 0x101cc6e0: Tron has no deathmatch settings here and its
	// mission bute file comes with MID_START_GAME.
	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::EndGame
//
//	PURPOSE:	End the game.
//
// --------------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::EndGame( )
{
	// RETAIL: 0x101cc700: Tron does not call the base EndGame here.

	// Can't have custom level or no campaign in deathmatch.
	if( m_bCustomLevel || m_Campaign.size( ) == 0 )
	{
		ASSERT( !"CDeathMatchMissionMgr::EndGame:  Can't have custom level or no campaign in deathmatch." );
		return false;
	}

	// Start from the beginning.
	m_nCurrentLevel = 0;
	m_nCurCampaignIndex = 0;
	m_nCurrentMission = m_Campaign[m_nCurCampaignIndex];
	m_bNewMission = true;

	// Get the worldname from the mission/level indices.
	char const* pszFilename = GetLevelFromMission( m_nCurrentMission, m_nCurrentLevel );
	if( !pszFilename )
		return false;

	// Exit to the level.
	if( !ExitLevel( pszFilename, false ))
		return false;

	return true;
}
