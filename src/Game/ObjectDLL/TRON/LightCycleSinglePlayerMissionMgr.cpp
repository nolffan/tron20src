// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleSinglePlayerMissionMgr.cpp
//
// PURPOSE : The mission manager of the single player light cycle game
//           types (circuit and custom game).
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x101ccb60..0x101ccdde).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleSinglePlayerMissionMgr.h"
#include "GameServerShell.h"
#include "MsgIDs.h"
#include "CaselessString.h"

// RETAIL: the client's light cycle game data (CShell.dll's
// CLightCycleGameData, ClientShellDLL/TRON/LightCycleGameDataMgr.h) as the
// server reads it: ServerGameOptions::GetTronUnknownCC gives its address.
// Only the custom game's maps are read here. The names are ours.
struct LightCycleServerGameData
{
	std::string							m_sName;			// +0x00
	std::vector<CaselessInlineString>	m_aCustomMaps;		// +0x0c
};

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleSinglePlayerMissionMgr::CLightCycleSinglePlayerMissionMgr
//
//	PURPOSE:	Constructor.
//
// --------------------------------------------------------------------------- //

CLightCycleSinglePlayerMissionMgr::CLightCycleSinglePlayerMissionMgr( )
{
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleSinglePlayerMissionMgr::~CLightCycleSinglePlayerMissionMgr
//
//	PURPOSE:	Destructor.
//
// --------------------------------------------------------------------------- //

CLightCycleSinglePlayerMissionMgr::~CLightCycleSinglePlayerMissionMgr( )
{
	Term( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleSinglePlayerMissionMgr::ExitLevelSwitch
//
//	PURPOSE:	An exit switch ends the race: the client chooses what
//				follows.
//
// --------------------------------------------------------------------------- //

bool CLightCycleSinglePlayerMissionMgr::ExitLevelSwitch( )
{
	return SendExitLevelMessage( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleSinglePlayerMissionMgr::SendExitLevelMessage
//
//	PURPOSE:	Tell the client that the level is done.  The circuit's
//				client gets no data; the custom game's gets the index of
//				the map after this one (-1 after the last).
//
// --------------------------------------------------------------------------- //

bool CLightCycleSinglePlayerMissionMgr::SendExitLevelMessage( )
{
	if( g_pGameServerShell->GetGameType( ) == eGameTypeSingleLightCycleCircuit )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_TRON_UNKNOWN_194 );
		g_pLTServer->SendToClient( cMsg.Read( ), LTNULL, MESSAGE_GUARANTEED );
		return true;
	}

	LightCycleServerGameData* pData = ( LightCycleServerGameData* )g_pGameServerShell->GetServerGameOptions( ).GetTronUnknownCC( );
	std::vector<CaselessInlineString>& aMaps = pData->m_aCustomMaps;

	int8 nNextMap = 0;
	std::vector<CaselessInlineString>::iterator iter;
	for( iter = aMaps.begin( ); iter != aMaps.end( ); iter++ )
	{
		nNextMap++;
		if( *iter == m_sCurrentWorldName.GetBuffer( ))
			break;
	}

	if( nNextMap >= ( int8 )aMaps.size( ))
		nNextMap = -1;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_TRON_UNKNOWN_195 );
	cMsg.Writeint8( nNextMap );
	g_pLTServer->SendToClient( cMsg.Read( ), LTNULL, MESSAGE_GUARANTEED );
	return true;
}
