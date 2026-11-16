// ----------------------------------------------------------------------- //
//
// MODULE  : CoopMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle coop missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "CoopMissionMgr.h"
#include "MusicMgr.h"
#include "PlayerObj.h"
#include "MsgIds.h"
#include "ServerSaveLoadMgr.h"
#include "MissionButeMgr.h"

extern CVarTrack g_vtNetFriendlyFire;
extern CVarTrack g_vtDifficultyFactorPlayerIncrease;

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCoopMissionMgr::StartGame
//
//	PURPOSE:	Default start game.
//
// --------------------------------------------------------------------------- //

bool CCoopMissionMgr::StartGame(ILTMessage_Read& msg )
{
	// RETAIL: 0x101cc240: Tron's mission bute file comes with
	// MID_START_GAME, and the coop settings follow it in the message.
	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	if( !g_vtNetFriendlyFire.IsInitted( ))
		g_vtNetFriendlyFire.Init( g_pLTServer, "NetFriendlyFire", LTNULL, 0.0f );
	if( !g_vtDifficultyFactorPlayerIncrease.IsInitted( ))
		g_vtDifficultyFactorPlayerIncrease.Init( g_pLTServer, "DifficultyFactorPlayerIncrease", LTNULL, 0.1f );

	m_bUseSkills = msg.Readbool( );
	m_bFriendlyFire = msg.Readbool( );
	g_vtNetFriendlyFire.SetFloat( m_bFriendlyFire ? 1.0f : 0.0f );

	m_nMPDifficulty = msg.Readuint8( );
	g_pGameServerShell->SetDifficulty(( GameDifficulty )m_nMPDifficulty );
	g_pLTServer->CPrint( "CCoopMissionMgr::StartGame() setting difficulty to %d", m_nMPDifficulty );

	m_fPlayerDiffFactor = msg.Readfloat( );
	g_vtDifficultyFactorPlayerIncrease.SetFloat( m_fPlayerDiffFactor );

	g_pMusicMgr->Enable( );
	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCoopMissionMgr::EndGame
//
//	PURPOSE:	End the game.
//
// --------------------------------------------------------------------------- //

bool CCoopMissionMgr::EndGame( )
{
	// RETAIL: 0x101cc480: Tron does not call the base EndGame here.

	// Can't have custom level or no campaign in coop.
	if( m_bCustomLevel || m_Campaign.size( ) == 0 )
	{
		ASSERT( !"CCoopMissionMgr::EndGame:  Can't have custom level or no campaign in coop." );
		return false;
	}

	if( !m_bLoopCampaign )
	{
		// Tell the players to prepare to exit a level.
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj* pPlayerObj = *iter;
			pPlayerObj->HandlePreExit();
			iter++;
		}

		// Pause the server while we switch levels.
		g_pGameServerShell->PauseGame( LTTRUE );	


		// Tell the clients the game is over.
		SendEmptyClientMsg( MID_END_GAME, NULL, MESSAGE_GUARANTEED );
		return true;
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
