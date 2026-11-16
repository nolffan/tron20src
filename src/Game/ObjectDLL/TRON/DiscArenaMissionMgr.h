// ----------------------------------------------------------------------- //
//
// MODULE  : DiscArenaMissionMgr.h
//
// PURPOSE : The mission manager of the disc arena game type.
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x101cc7c0..0x101cc7f0, RTTI .?AVCDiscArenaMissionMgr@@,
// vtable 0x1029793c). The class name is retail's.
//
// ----------------------------------------------------------------------- //

#ifndef __DISCARENAMISSIONMGR_H__
#define __DISCARENAMISSIONMGR_H__

#include "ServerMissionMgr.h"

class CDiscArenaMissionMgr : public CServerMissionMgr
{
	protected:

		virtual bool	StartGame( ILTMessage_Read& msg );	// RETAIL: 0x101cc7c0
		virtual bool	EndGame( );							// RETAIL: 0x101cc7f0
};

#endif // __DISCARENAMISSIONMGR_H__
