// ----------------------------------------------------------------------- //
//
// MODULE  : DiscArenaMissionMgr.cpp
//
// PURPOSE : The mission manager of the disc arena game type.
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x101cc7c0..0x101cc7f0).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DiscArenaMissionMgr.h"
#include "MusicMgr.h"

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscArenaMissionMgr::StartGame
//
//	PURPOSE:	Start the game.
//
// --------------------------------------------------------------------------- //

bool CDiscArenaMissionMgr::StartGame( ILTMessage_Read& msg )
{
	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	g_pMusicMgr->Enable( );
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscArenaMissionMgr::EndGame
//
//	PURPOSE:	Called when the game has finished the last mission.
//
// --------------------------------------------------------------------------- //

bool CDiscArenaMissionMgr::EndGame( )
{
	// RETAIL: 0x101cc7f0: the disc arena game never ends this way.
	return false;
}
