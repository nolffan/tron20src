// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMultiplayerMissionMgr.cpp
//
// PURPOSE : The mission manager of the multiplayer light cycle game type.
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x101cc860..0x101ccb5a). It follows CCoopMissionMgr.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleMultiplayerMissionMgr.h"
#include "MusicMgr.h"
#include "PlayerObj.h"
#include "MsgIds.h"

extern CVarTrack g_vtNetFriendlyFire;
extern CVarTrack g_vtDifficultyFactorPlayerIncrease;

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMultiplayerMissionMgr::StartGame
//
//	PURPOSE:	Start the game with the settings that follow MID_START_GAME.
//
// --------------------------------------------------------------------------- //

bool CLightCycleMultiplayerMissionMgr::StartGame( ILTMessage_Read& msg )
{
	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	if( !g_vtNetFriendlyFire.IsInitted( ))
		g_vtNetFriendlyFire.Init( g_pLTServer, "NetFriendlyFire", LTNULL, 0.0f );
	if( !g_vtDifficultyFactorPlayerIncrease.IsInitted( ))
		g_vtDifficultyFactorPlayerIncrease.Init( g_pLTServer, "DifficultyFactorPlayerIncrease", LTNULL, 0.1f );

	m_bUseSkills = msg.Readbool( );
	m_bFriendlyFire = msg.Readbool( );
	g_vtNetFriendlyFire.SetFloat( m_bFriendlyFire ? 1.0f : 0.0f );

	// RETAIL: 0x101cc9f1: friendly fire is then always on in this game type.
	g_vtNetFriendlyFire.SetFloat( 1.0f );

	m_nMPDifficulty = msg.Readuint8( );
	g_pGameServerShell->SetDifficulty(( GameDifficulty )m_nMPDifficulty );

	m_fPlayerDiffFactor = msg.Readfloat( );
	g_vtDifficultyFactorPlayerIncrease.SetFloat( m_fPlayerDiffFactor );

	g_pMusicMgr->Enable( );
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMultiplayerMissionMgr::EndGame
//
//	PURPOSE:	Called when the game has finished the last mission.
//
// --------------------------------------------------------------------------- //

bool CLightCycleMultiplayerMissionMgr::EndGame( )
{
	// Can't have custom level or no campaign.
	if( m_bCustomLevel || m_Campaign.size( ) == 0 )
	{
		ASSERT( !"CLightCycleMultiplayerMissionMgr::EndGame:  Can't have custom level or no campaign." );
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
