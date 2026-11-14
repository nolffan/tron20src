// ----------------------------------------------------------------------- //
//
// MODULE  : TronCDKey.h
//
// PURPOSE : The CD key, as the installer left it in the registry
//
// RETAIL: Tron 2.0's, retail-only (no RTTI or strings name the class: its
// name and its members' are ours). A 0xc-byte member of
// CTronGameClientShell (+0x31a74, its slot 0x98 returns it): the
// constructor is retail CShell.dll 0x1000b640, GetCDKey 0x1000b7d0
// (ClientMultiplayerMgr::HandleMsgHandshake, 0x10096ac0, answers the
// server's GameSpy CD key challenge with it).
//
// ----------------------------------------------------------------------- //

#ifndef __TRONCDKEY_H__
#define __TRONCDKEY_H__

#include <string>

class CTronCDKey
{
	public:

		CTronCDKey();						// RETAIL: 0x1000b640

		std::string GetCDKey() const;		// RETAIL: 0x1000b7d0

	private:

		std::string	m_sCDKey;		// +0x0 (upper case)
};

#endif // __TRONCDKEY_H__
