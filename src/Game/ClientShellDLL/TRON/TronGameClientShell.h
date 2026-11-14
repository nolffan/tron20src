// ----------------------------------------------------------------------- //
//
// MODULE  : TronGameClientShell.h
//
// PURPOSE : Game Client Shell - Definition
//
// CREATED : 11/5/01
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRONGAME_CLIENT_SHELL_H__
#define __TRONGAME_CLIENT_SHELL_H__

#include "GameClientShell.h"
#include "TronInterfaceMgr.h"
#include "TronVersionMgr.h"
#include "TronPlayerMgr.h"
#include "TronClientWeaponAllocator.h"
#include "TronMissionButeMgr.h"
#include "ClientLightCycleMgr.h"
#include "DiscMatchMgr.h"
#include "LightCycleMatchMgr.h"

class CTronGameClientShell : public CGameClientShell
{
	public:
		declare_interface(CTronGameClientShell);

		CTronGameClientShell();
		~CTronGameClientShell();

		virtual CInterfaceMgr * GetInterfaceMgr() { return &m_InterfaceMgr; }
		virtual CPlayerMgr*		GetPlayerMgr() { return &m_PlayerMgr; }
		virtual CClientWeaponAllocator const *GetClientWeaponAllocator() const
				{ return &m_ClientWeaponAllocator; }

		virtual uint32 OnEngineInitialized(RMode *pMode, LTGUID *pAppGuid);
	    virtual LTRESULT ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort);
		virtual void OnMessage(ILTMessage_Read *pMsg);

		virtual void PauseGame(bool bPause, bool bPauseSound = false);

		// RETAIL: slot 0x8 (0x10099ff0): resets the match managers
		virtual void OnExitWorld();

		// RETAIL: slot 0x18 (0x1009ac40): the light cycle match manager
		// sees the keys first in the light cycle game type
		virtual void OnKeyDown(int key, int rep);

	private:
		CTronInterfaceMgr			m_InterfaceMgr;		// Interface manager
		CTronVersionMgr				m_VersionMgr;		// Same as g_pVersionMgr
		CTronPlayerMgr				m_PlayerMgr;		// Player manager
		CTronMissionButeMgr			m_MissionButeMgr;	// Same as g_pMissionButeMgr
		CClientLightCycleMgr		m_LightCycleMgr;	// Same as g_pLightCycleMgr (RETAIL: +0x32e00)
		CDiscMatchMgr				m_DiscMatchMgr;		// Same as g_pDiscMatchMgr (RETAIL: +0x3695c)
		CLightCycleMatchMgr			m_LightCycleMatchMgr;	// Same as g_pLightCycleMatchMgr (RETAIL: +0x3696c)

		// allocator for client weapons
		CTronClientWeaponAllocator	m_ClientWeaponAllocator;
};

#endif  // __TRON_GAME_CLIENT_SHELL_H__
