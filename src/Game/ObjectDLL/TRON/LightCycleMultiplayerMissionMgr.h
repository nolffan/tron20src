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
#include "ProfileUtils.h"

class CLightCycleMultiplayerMissionMgr : public CServerMissionMgr
{
	protected:

		virtual bool	StartGame( ILTMessage_Read& msg );	// RETAIL: 0x101cc860
		virtual bool	EndGame( );							// RETAIL: 0x101ccac0

		// RETAIL: Tron's (+0x48: the class is 0xec bytes; its inline
		// constructor and destructor, Object.lto 0x100e5140, 0x100e5250).
		// Not yet identified
		DMGameOptions	m_xTronUnknown48;
};

#endif // __LIGHTCYCLEMULTIPLAYERMISSIONMGR_H__
