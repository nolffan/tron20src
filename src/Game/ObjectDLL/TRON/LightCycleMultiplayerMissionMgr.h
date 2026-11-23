// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMultiplayerMissionMgr.h
//
// PURPOSE : The mission manager of the multiplayer light cycle game type.
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x101cc860..0x101ccb5a, RTTI
// .?AVCLightCycleMultiplayerMissionMgr@@, vtable 0x10297970). The class
// name is retail's.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEMULTIPLAYERMISSIONMGR_H__
#define __LIGHTCYCLEMULTIPLAYERMISSIONMGR_H__

#include "ServerMissionMgr.h"
#include "TRON/LightCycleGameData.h"

class CLightCycleMultiplayerMissionMgr : public CServerMissionMgr
{
	protected:

		virtual bool	StartGame( ILTMessage_Read& msg );	// RETAIL: 0x101cc860
		virtual bool	EndGame( );							// RETAIL: 0x101ccac0

		// RETAIL: +0x48, a set of light cycle game settings (0xa4 bytes;
		// the class is 0xec bytes: debug_new 0x100e5140 constructs it with
		// CLightCycleGameData's constructor, 0x10029cb0; the inline
		// destructor 0x100e5250 destroys it). No code of the class reads it.
		// The name is ours
		CLightCycleGameData	m_GameData;
};

#endif // __LIGHTCYCLEMULTIPLAYERMISSIONMGR_H__
