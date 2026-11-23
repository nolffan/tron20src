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
#include "LightCycleGameData.h"

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
	}
	else
	{
		// RETAIL: the client's current set (CShell.dll's; the server and the
		// client share the process in the single player game)
		CLightCycleGameData* pData = ( CLightCycleGameData* )g_pGameServerShell->GetServerGameOptions( ).GetLightCycleGameData( );
		StringArray& aMaps = pData->m_Custom.aMaps;

		int8 nNextMap = 0;
		StringArray::iterator iter;
		for( iter = aMaps.begin( ); iter != aMaps.end( ); iter++ )
		{
			nNextMap++;
			// RETAIL: the maps are std::strings, but they compare without
			// case (_memicmp, 0x101ccce7): the cast is ours
			if( *( CaselessInlineString* )&( *iter ) == m_sCurrentWorldName.GetBuffer( ))
				break;
		}

		if( nNextMap >= ( int8 )aMaps.size( ))
			nNextMap = -1;

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_TRON_UNKNOWN_195 );
		cMsg.Writeint8( nNextMap );
		g_pLTServer->SendToClient( cMsg.Read( ), LTNULL, MESSAGE_GUARANTEED );
	}

	return true;
}
