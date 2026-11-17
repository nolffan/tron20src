// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleSinglePlayerMissionMgr.h
//
// PURPOSE : The mission manager of the single player light cycle game
//           types (circuit and custom game).
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x101ccb60..0x101ccdde, RTTI
// .?AVCLightCycleSinglePlayerMissionMgr@@, vtable 0x1029aee4). The class
// name is retail's.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLESINGLEPLAYERMISSIONMGR_H__
#define __LIGHTCYCLESINGLEPLAYERMISSIONMGR_H__

#include "ServerMissionMgr.h"

class CLightCycleSinglePlayerMissionMgr : public CServerMissionMgr
{
	public:

		CLightCycleSinglePlayerMissionMgr( );				// RETAIL: 0x101ccb60
		virtual ~CLightCycleSinglePlayerMissionMgr( );		// RETAIL: 0x101ccba0

		virtual bool	ExitLevelSwitch( );					// RETAIL: 0x101ccbf0

	protected:

		virtual bool	SendExitLevelMessage( );			// RETAIL: 0x101ccc00
};

#endif // __LIGHTCYCLESINGLEPLAYERMISSIONMGR_H__
