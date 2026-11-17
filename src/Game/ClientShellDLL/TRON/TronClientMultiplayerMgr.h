// ----------------------------------------------------------------------- //
//
// MODULE  : TronClientMultiplayerMgr.h
//
// PURPOSE : Tron's client multiplayer manager
//
// ----------------------------------------------------------------------- //

// RETAIL: written from retail CShell.dll (RTTI TronClientMultiplayerMgr,
// vtable 0x101e8e64: SetDisconnectCode, then its HandleMsgHandshake at
// 0x10096ac0). CTronGameClientShell holds it (+0x35644).

#ifndef __TRON_CLIENT_MULTIPLAYER_MGR_H__
#define __TRON_CLIENT_MULTIPLAYER_MGR_H__

#include "ClientMultiplayerMgr.h"

class TronClientMultiplayerMgr : public ClientMultiplayerMgr
{
protected:
	virtual bool	HandleMsgHandshake( ILTMessage_Read& msg );
};

#endif // __TRON_CLIENT_MULTIPLAYER_MGR_H__
