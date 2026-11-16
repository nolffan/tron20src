// ----------------------------------------------------------------------- //
//
// MODULE  : TeamDeathMatchMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle team deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "TeamDeathMatchMissionMgr.h"
	#include "TeamMgr.h"
	#include "MissionButeMgr.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::CTeamDeathMatchMissionMgr
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTeamDeathMatchMissionMgr::CTeamDeathMatchMissionMgr()
:	CDeathMatchMissionMgr	()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::~CTeamDeathMatchMissionMgr
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTeamDeathMatchMissionMgr::~CTeamDeathMatchMissionMgr()
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

bool CTeamDeathMatchMissionMgr::Init()
{
	if( !CDeathMatchMissionMgr::Init() )
		return false;

	// Add the teams to the team mgr...

	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );

	uint8 nNumTeams = sgo.GetTeamDeathmatch().m_nNumTeams;

	for( uint8 i = 0; i < nNumTeams; ++i )
	{
		CTeamMgr::Instance().AddTeam( sgo.GetTeamDeathmatch().m_sTeamName[i].c_str(),
									  sgo.GetTeamDeathmatch().m_nTeamModel[i] );
	}

	return !!CTeamMgr::Instance().GetNumTeams();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::Update
//
//	PURPOSE:	Frame update.  Check for end level condition...
//
// ----------------------------------------------------------------------- //

void CTeamDeathMatchMissionMgr::Update()
{
	// PORT: retail Tron has no team deathmatch mission manager (no RTTI for
	// it) and its CServerMissionMgr has no score limit or rounds; NOLF2's
	// class stays for the game type switch only.
	CServerMissionMgr::Update( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::FinishExitLevel
//
//	PURPOSE:	Finish the exitlevel we started....
//
// ----------------------------------------------------------------------- //

bool CTeamDeathMatchMissionMgr::FinishExitLevel()
{
	if( !CDeathMatchMissionMgr::FinishExitLevel() )
		return false;

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::StartGame
//
//	PURPOSE:	Start the game.
//
// --------------------------------------------------------------------------- //

bool CTeamDeathMatchMissionMgr::StartGame(ILTMessage_Read& msg )
{
	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	m_bUseSkills = false;
	m_bFriendlyFire = sgo.GetTeamDeathmatch().m_bFriendlyFire;

	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::LevelStarted()
//
//	PURPOSE:	Called when the clients have finished loading and have started
//				playing a level
//
// ----------------------------------------------------------------------- //

void CTeamDeathMatchMissionMgr::LevelStarted()
{
	CServerMissionMgr::LevelStarted( );

	// Check if this is a new level, or just another round.
	if( m_bNewMission )
	{
		CTeamMgr::Instance( ).NewLevel( );
	}
	else
	{
		CTeamMgr::Instance( ).NewRound( );
	}
}
